#include "nnue.h"
#include "../misc/defines.h"
#include <cstdio>

namespace {

constexpr int NUM_FEATURES = 40960;
constexpr int L0 = 256, L1 = 512, L2 = 32, L3 = 32;
constexpr int FT_SCALE = 127, S_L1 = 128, S_L2 = 64, S_OUT = 64, N2S = 128;
constexpr int SHIFT_L1 = 7, SHIFT_L2 = 6;   // log2 of the weight scales
constexpr int QONE = 127;                   // clipped relu upper bound
constexpr int MAX_ACTIVE = 30;              // HalfKP excludes kings
constexpr uint32_t FILE_VERSION = 1;

struct Header {
    char magic[4];
    uint32_t version, num_features, l0, l1, l2, l3;
    uint32_t ft_scale, s_l1, s_l2, s_out, nnue2score;
};
static_assert(sizeof(Header) == 48, "header must be exactly 48 bytes");

alignas(64) int16_t ft_weight[(size_t)NUM_FEATURES * L0];
alignas(64) int16_t ft_bias[L0];
alignas(64) int8_t  l1_weight[L2 * L1];
alignas(64) int8_t  l2_weight[L3 * L2];
alignas(64) int8_t  out_weight[L3];
int32_t l1_bias[L2], l2_bias[L3], out_bias[1];

bool loaded = false;
string loaded_from;

bool fail(FILE *f, const string &why) {
    if (f) fclose(f);
    cout << "info string nnue: " << why << "\n" << flush;
    cerr << "nnue: " << why << "\n" << flush;
    return false;
}

// Black's perspective is a vertical flip, matching HalfKP::orient() as actually
// compiled in nnue-pytorch. (halfkp-extractor.patch shows a 180 rotation; the
// built loader does not do that, and the loader is what trained the net.)
inline int orient(Colour pov, int sq) { return pov == WHITE ? sq : (sq ^ 56); }

// index = orient(sq) + p_idx*64 + orient(ksq)*640, kings excluded.
// The colour bit is relative to the perspective, not absolute.
int active_features(Board &b, Colour pov, int *out) {
    uint64_t kbb = b.getKingBitboard(pov);
    const int king_offset = orient(pov, get_lsb(kbb)) * 640;

    uint64_t occ = b.get_occupancy()
                 & ~(b.getKingBitboard(WHITE) | b.getKingBitboard(BLACK));
    int n = 0;
    while (occ && n < MAX_ACTIVE) {
        const uint8_t sq = pop_lsb(&occ);
        const uint8_t p = b.pieceOn(sq);
        const int p_idx = type_of(p) * 2 + (colour_of(p) != pov);
        out[n++] = orient(pov, sq) + p_idx * 64 + king_offset;
    }
    return n;
}

void accumulate(const int *feats, int n, int16_t *acc) {
    for (int j = 0; j < L0; j++) acc[j] = ft_bias[j];
    for (int i = 0; i < n; i++) {
        const int16_t *w = &ft_weight[(size_t)feats[i] * L0];
        for (int j = 0; j < L0; j++) acc[j] += w[j];
    }
}

inline int32_t crelu(int32_t v) { return v < 0 ? 0 : (v > QONE ? QONE : v); }

// 512 -> 32 -> 32 -> 1. us/them are the two accumulators, side to move first.
int32_t forward(const int16_t *us, const int16_t *them) {
    int32_t in[L1];
    for (int j = 0; j < L0; j++) {
        in[j] = crelu(us[j]);
        in[L0 + j] = crelu(them[j]);
    }

    int32_t h1[L2];
    for (int o = 0; o < L2; o++) {
        int32_t s = l1_bias[o];
        const int8_t *w = &l1_weight[o * L1];
        for (int i = 0; i < L1; i++) s += in[i] * w[i];
        h1[o] = crelu(s >> SHIFT_L1);
    }

    int32_t h2[L3];
    for (int o = 0; o < L3; o++) {
        int32_t s = l2_bias[o];
        const int8_t *w = &l2_weight[o * L2];
        for (int i = 0; i < L2; i++) s += h1[i] * w[i];
        h2[o] = crelu(s >> SHIFT_L2);
    }

    int32_t out = out_bias[0];
    for (int i = 0; i < L3; i++) out += h2[i] * out_weight[i];
    return out;
}

inline int feature_index(Colour pov, uint8_t p, int sq, int king_offset) {
    const int p_idx = type_of(p) * 2 + (colour_of(p) != pov);
    return orient(pov, sq) + p_idx * 64 + king_offset;
}

// ---- lazy accumulator stack ------------------------------------------------
// A move is recorded when it is made, but the accumulator is only materialised
// when evaluate() actually asks for it. Most nodes in this engine never call
// evaluate (it is only reached in qsearch), so updating eagerly would cost more
// than it saves.

constexpr int MAX_ACC_PLY = 1024;

struct DirtyPiece {
    int n = 0;
    uint8_t pc[3];
    int8_t from[3];      // -1: the piece appears (promotion)
    int8_t to[3];        // -1: the piece disappears (capture)
    bool king_moved[2];  // that side's features all shift -> full refresh

    void add(uint8_t p, int f, int t) {
        pc[n] = p;
        from[n] = (int8_t)f;
        to[n] = (int8_t)t;
        n++;
    }
};

struct AccEntry {
    alignas(64) int16_t v[2][L0];
    bool computed[2];
    uint64_t key;        // zobrist of the position these values belong to
    DirtyPiece dirty;    // the move that led into this entry
};

AccEntry stack_[MAX_ACC_PLY];
int ply = 0;

void refresh(Board &b, Colour pov, int16_t *acc) {
    int feats[MAX_ACTIVE];
    accumulate(feats, active_features(b, pov, feats), acc);
}

// Replay one move's piece changes onto an existing accumulator.
void apply(const DirtyPiece &d, Colour pov, int king_offset, int16_t *acc) {
    for (int i = 0; i < d.n; i++) {
        if (type_of(d.pc[i]) == KING) continue;  // kings are not features
        if (d.from[i] >= 0) {
            const int16_t *w = &ft_weight[(size_t)feature_index(pov, d.pc[i], d.from[i], king_offset) * L0];
            for (int j = 0; j < L0; j++) acc[j] -= w[j];
        }
        if (d.to[i] >= 0) {
            const int16_t *w = &ft_weight[(size_t)feature_index(pov, d.pc[i], d.to[i], king_offset) * L0];
            for (int j = 0; j < L0; j++) acc[j] += w[j];
        }
    }
}

// Make stack_[ply].v[pov] valid, replaying from the nearest usable ancestor.
void ensure(Board &b, Colour pov) {
    AccEntry &cur = stack_[ply];
    if (cur.computed[pov]) return;

    // Walk back to a computed ancestor. A king move for this perspective
    // invalidates every feature, so the chain cannot cross one.
    int base = ply;
    while (base > 0 && !stack_[base].computed[pov] && !stack_[base].dirty.king_moved[pov])
        base--;

    if (!stack_[base].computed[pov]) {
        refresh(b, pov, cur.v[pov]);
        cur.computed[pov] = true;
        return;
    }

    // The king has not moved along this chain, so its square -- and therefore
    // the feature offset -- is the same at every step.
    uint64_t kbb = b.getKingBitboard(pov);
    const int king_offset = orient(pov, get_lsb(kbb)) * 640;

    if (base != ply) memcpy(cur.v[pov], stack_[base].v[pov], sizeof cur.v[pov]);
    for (int i = base + 1; i <= ply; i++)
        apply(stack_[i].dirty, pov, king_offset, cur.v[pov]);
    cur.computed[pov] = true;
}

}  // namespace


bool nnue_ready() { return loaded; }
const char *nnue_file() { return loaded_from.c_str(); }

bool nnue_load(const string &path) {
    loaded = false;
    FILE *f = fopen(path.c_str(), "rb");
    if (!f) return fail(nullptr, "cannot open " + path);

    Header h;
    if (fread(&h, sizeof h, 1, f) != 1) return fail(f, "truncated header");
    if (memcmp(h.magic, "NNUE", 4) != 0) return fail(f, "bad magic in " + path);

    // The scales live in the file so a net exported under a different scheme is
    // rejected here rather than silently evaluated as nonsense.
    const pair<const char *, pair<uint32_t, uint32_t>> checks[] = {
        {"version", {h.version, FILE_VERSION}},
        {"num_features", {h.num_features, NUM_FEATURES}},
        {"l0", {h.l0, L0}}, {"l1", {h.l1, L1}},
        {"l2", {h.l2, L2}}, {"l3", {h.l3, L3}},
        {"ft_scale", {h.ft_scale, FT_SCALE}},
        {"weight_scale_l1", {h.s_l1, S_L1}},
        {"weight_scale_l2", {h.s_l2, S_L2}},
        {"weight_scale_out", {h.s_out, S_OUT}},
        {"nnue2score", {h.nnue2score, N2S}},
    };
    for (const auto &c : checks) {
        if (c.second.first != c.second.second) {
            return fail(f, string(c.first) + " is " + to_string(c.second.first)
                             + ", engine expects " + to_string(c.second.second));
        }
    }

    const pair<void *, size_t> blocks[] = {
        {ft_weight, sizeof ft_weight}, {ft_bias, sizeof ft_bias},
        {l1_weight, sizeof l1_weight}, {l1_bias, sizeof l1_bias},
        {l2_weight, sizeof l2_weight}, {l2_bias, sizeof l2_bias},
        {out_weight, sizeof out_weight}, {out_bias, sizeof out_bias},
    };
    for (const auto &blk : blocks) {
        if (fread(blk.first, 1, blk.second, f) != blk.second)
            return fail(f, "truncated weight data in " + path);
    }

    char extra;
    if (fread(&extra, 1, 1, f) != 0) return fail(f, "trailing bytes in " + path);
    fclose(f);

    loaded = true;
    loaded_from = path;
    nnue_reset();   // accumulators built with the previous net are now wrong
    cout << "info string nnue: loaded " << path << "\n" << flush;
    return true;
}

void nnue_reset() {
    ply = 0;
    stack_[0].computed[0] = stack_[0].computed[1] = false;
    stack_[0].key = 0;
}

void nnue_push(Board &b, Move m) {
    // Read the pre-move board: this must be called before b.make_move(m).
    DirtyPiece d;
    d.n = 0;
    d.king_moved[WHITE] = d.king_moved[BLACK] = false;

    const uint8_t from = m.getFromSq(), to = m.getToSq();
    const uint8_t flag = m.getMoveFlag();
    const uint8_t moving = b.pieceOn(from);
    const Colour side = colour_of(moving);

    if (flag == ENPASSANT) {
        const uint8_t csq = (side == WHITE) ? to - 8 : to + 8;
        d.add(b.pieceOn(csq), csq, -1);
    } else {
        const uint8_t cap = b.pieceOn(to);
        if (cap != EMPTY_SQ) d.add(cap, to, -1);
    }

    if (m.isPromo()) {
        d.add(moving, from, -1);
        d.add(piece(side, m.promoPiece()), -1, to);
    } else {
        d.add(moving, from, to);
    }
    if (type_of(moving) == KING) d.king_moved[side] = true;

    if (flag == CASTLE) {
        const int cs = to > from ? KINGSIDE : QUEENSIDE;
        const uint8_t rf = castling_rook_from_sq[side][cs];
        d.add(b.pieceOn(rf), rf, castling_rook_to_sq[side][cs]);
    }

    if (++ply >= MAX_ACC_PLY) return;   // overflow: ensure() falls back to refresh
    stack_[ply].dirty = d;
    stack_[ply].computed[0] = stack_[ply].computed[1] = false;
    stack_[ply].key = 0;
}

void nnue_pop() {
    if (ply > 0) ply--;
}

int nnue_raw(Board &b) {
    if (ply >= MAX_ACC_PLY) ply = MAX_ACC_PLY - 1;   // shouldn't happen; stay safe
    AccEntry &cur = stack_[ply];

    // Guards against being called outside the search's push/pop discipline --
    // tests and UCI build a fresh Board without ever pushing.
    if (cur.key != b.getHash()) {
        cur.key = b.getHash();
        cur.computed[0] = cur.computed[1] = false;
    }
    ensure(b, WHITE);
    ensure(b, BLACK);

    // Side to move occupies the first half, matching the trainer's [us, them].
    const int16_t *us = b.isWhiteTurn() ? cur.v[WHITE] : cur.v[BLACK];
    const int16_t *them = b.isWhiteTurn() ? cur.v[BLACK] : cur.v[WHITE];

    return forward(us, them);
}

// Reference path: ignores the stack entirely and rebuilds from the board.
// Only used by tests, to check the incremental path against.
int nnue_raw_refresh(Board &b) {
    int16_t acc_w[L0], acc_b[L0];
    refresh(b, WHITE, acc_w);
    refresh(b, BLACK, acc_b);
    return forward(b.isWhiteTurn() ? acc_w : acc_b,
                   b.isWhiteTurn() ? acc_b : acc_w);
}

int nnue_evaluate(Board &b) {
    if (!loaded) return 0;
    // Round half away from zero.  '/' truncating toward zero.
    const int den = S_OUT * QONE;
    const int num = nnue_raw(b) * N2S;
    return num >= 0 ? (num + den / 2) / den : (num - den / 2) / den;
}

// debug helper: fills `out` with the active features for one perspective
int nnue_debug_features(Board &b, int white_pov, int *out) {
    return active_features(b, white_pov ? WHITE : BLACK, out);
}
