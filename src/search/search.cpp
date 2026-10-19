#include "search.h"
#include "nnue.h"
#include "eval.h"
int nodes_searched = 0;
int qnodes_searched = 0;

TTEntry tt[TT_SIZE];

// qsearch has no time argument, so the deadline is shared. Once it trips the
// whole iteration is discarded at the root, so partial results are harmless --
// but they must not reach the transposition table.
static TimePoint search_deadline = TimePoint::max();
static bool search_timed_out = false;
long long scouts = 0, researches = 0, first_move_best = 0, nodes_with_moves = 0;
constexpr int QSEARCH_MAX_PLY = 64;

// Quiet-move ordering. Captures are already ranked by MVV-LVA; these two give
// the quiets an order, which is what makes the rest of the pruning work.
//   killers: quiets that caused a beta cutoff at this same ply elsewhere
//   history: how often a (side, piece, destination) has caused a cutoff
static bool from_null[MAX_DEPTH];   // this ply was reached by a null move
static Move killers[MAX_DEPTH][2];
static int history[64][64];   // [from][to]

static void clear_ordering_tables() {
    memset(from_null, 0, sizeof from_null);
    memset(killers, 0, sizeof killers);
    memset(history, 0, sizeof history);
}

static inline bool is_quiet(Board &b, Move m) {
    return b.pieceOn(m.getToSq()) == EMPTY_SQ
        && m.getMoveFlag() != ENPASSANT && !m.isPromo();
}

static void record_cutoff(Board &b, Move m, int depth, int ply) {
    if (ply < MAX_DEPTH && !(killers[ply][0] == m)) {
        killers[ply][1] = killers[ply][0];
        killers[ply][0] = m;
    }
    int &h = history[m.getFromSq()][m.getToSq()];
    h += depth * depth;
    if (h > ORDER_HIST_MAX) {        // rare: halve everything, ordering survives
        for (int f = 0; f < 64; f++)
            for (int t = 0; t < 64; t++) history[f][t] /= 2;
    }
}

// Rank the quiets update_movelist_evals left at 0: killers first, then history.
static void order_quiets(MoveList &ml, int ply) {
    for (int i = 0; i < ml.size; i++) {
        if (ml.evals[i] != 0) continue;   // TT move and tacticals keep their score
        Move &m = ml.moves[i];
        if (ply < MAX_DEPTH && killers[ply][0] == m)      ml.evals[i] = ORDER_KILLER_1;
        else if (ply < MAX_DEPTH && killers[ply][1] == m) ml.evals[i] = ORDER_KILLER_2;
        else ml.evals[i] = history[m.getFromSq()][m.getToSq()];
    }
}
int negamax(Board &b, int depth, int ply, int alpha, int beta, TimePoint time_limit, PVLine &pv) {
    pv.length = 0;
    if (nodes_searched++ % 2048 == 0) {
        if (Clock::now() >= time_limit) {
            search_timed_out = true;
            return OUTOFTIME;
        }
    }

    // repetition or 50-move rule scores as a draw
    if (b.is_repetition() || b.getHalfMoveClock() >= 100) {
        return 0;
    }

    if (depth == 0) {
        return qsearch(b, alpha, beta, 0);
    }

    // Mate distance pruning. Scores here use remaining depth, so the worst case
    // is being mated now and the best is mating on the next ply. Clamping to
    // that range lets a mate already proven nearer the root cut this node off,
    // and stops the search proving a longer mate than one it already has.
    alpha = max(alpha, -(CHECKMATE + depth));
    beta  = min(beta,    CHECKMATE + depth - 1);
    if (alpha >= beta) return alpha;

    const bool in_check = b.in_check();
    const Colour stm = b.isWhiteTurn() ? WHITE : BLACK;

    // Null move pruning: hand the opponent a free move and search shallow. If
    // the position still beats beta, the real move will too. Skipped in check,
    // after another null, and without non-pawn material (zugzwang territory,
    // where passing is genuinely better than any legal move).
    if (!in_check && depth >= 3 && ply > 0 && ply < MAX_DEPTH && !from_null[ply]
        && beta < MATE_SCORE && b.has_non_pawn_material(stm)) {
        const int R = 2 + depth / 6;
        PVLine null_pv;
        from_null[ply + 1] = true;
        nnue_push_null();
        b.make_null_move();
        int res = negamax(b, max(depth - 1 - R, 0), ply + 1, -beta, -beta + 1,
                          time_limit, null_pv);
        b.unmake_null_move();
        nnue_pop();
        from_null[ply + 1] = false;

        if (res == OUTOFTIME) return OUTOFTIME;
        if (-res >= beta) return beta;
    }

    MoveList possible_moves = generate_moves<ALL_MOVES>(b);

    if (possible_moves.size == 0) {
        return in_check ? -(CHECKMATE + depth) : 0;
    }

    uint64_t key = b.getHash();
    TTEntry* tte = probe(key);
    if (tte->key == key && tte->depth >= depth) {
        int value = tte->value;
        if (value > MATE_SCORE) value = value + depth;
        if (value < -MATE_SCORE) value = value - depth;
        if (tte->bound == EXACT) return value;
        // tree is too good to be true, it crashed because it found a move value greater than beta 
        //   will crash again if ran 
        if (tte->bound == LOWER && value >= beta) return value;
        // we know that every value searched in this tree is worse than alpha (didn't improve alpha_original)
        //   will not find anything again if failed like this previously
        if (tte->bound == UPPER && value <= alpha) return value;
    } 

    int alpha_orig = alpha;
    int value = -CHECKMATE - 1;
    update_movelist_evals(b, possible_moves, tte->key == key ? tte->move : MOVE_NONE); // For move ordering
    order_quiets(possible_moves, ply);
    int it = 0;
    int searched = 0;
    while (it < possible_moves.size) { // get it'th best move according to move ordering
        Move m = possible_moves.get_next_move(it++);
        PVLine child_pv;
        nnue_push(b, m);
        b.make_move(m);

        // Principal variation search. The ordering above means the first move is
        // usually best, so the rest only need a null window to prove they are
        // not. A scout that beats alpha is re-searched properly.
        int res;
        if (searched == 0) {
            res = negamax(b, depth - 1, ply + 1, -beta, -alpha, time_limit, child_pv);
        } else {
            scouts++;
            res = negamax(b, depth - 1, ply + 1, -alpha - 1, -alpha, time_limit, child_pv);
            if (res != OUTOFTIME && -res > alpha && -res < beta) {
                researches++;
                res = negamax(b, depth - 1, ply + 1, -beta, -alpha, time_limit, child_pv);
            }
        }

        b.unmake_move(m);
        nnue_pop();

        if (res == OUTOFTIME) return OUTOFTIME;
        searched++;
        int score = -res;

        if (score > value) {
            value = score;
            pv.moves[0] = m;
            memcpy(pv.moves + 1, child_pv.moves, child_pv.length * sizeof(Move));
            pv.length = child_pv.length + 1;
        }

        alpha = max(alpha, score);
        if (alpha >= beta) {
            if (is_quiet(b, m)) record_cutoff(b, m, depth, ply);
            nodes_with_moves++;
            if (searched == 1) first_move_best++;   // cutoff on the first move tried
            break;
        }
    }

    // update the big tt
    int tt_value = value;
    if (value > MATE_SCORE)         tt_value -= depth;
    else if (value < -MATE_SCORE)   tt_value += depth;

    if (search_timed_out) return value;   // partial result: do not poison the TT

    tte->key   = key;
    tte->value = tt_value;
    tte->move  = pv.moves[0];
    tte->depth = depth;
    tte->bound = (value <= alpha_orig) ? UPPER 
               : (value >= beta)       ? LOWER 
                                       : EXACT;

    return value;
}

int piece_value(uint8_t pc) {
    // pc is a piece code (colour*6 + type); PIECE_VALUES is indexed by type.
    return pc == EMPTY_SQ ? 0 : PIECE_VALUES[type_of(pc)];
}

// Swap algorithm: simulate the capture sequence on to_sq, always recapturing with the least valuable attacker.
int SEE(Board &b, Move &m) {
    static const int val[6] = {100, 300, 300, 500, 900, 20000};
    uint8_t from_sq = m.getFromSq(), to_sq = m.getToSq();
    Colour white_to_move = b.isWhiteTurn() ? WHITE : BLACK;
    uint64_t occ = b.get_occupancy() ^ (1ULL << from_sq);
    int gain[TOTAL_PIECES];

    if (m.getMoveFlag() == ENPASSANT) {
        gain[0] = val[PAWN];
        occ ^= 1ULL << (white_to_move == WHITE ? to_sq - 8 : to_sq + 8);
    } else {
        gain[0] = piece_value(b.pieceOn(to_sq));
    }

    int attacker = type_of(b.pieceOn(from_sq));  // piece now standing on to_sq
    Colour side = ~white_to_move;

    int capt_cnt = 1;
    // Continually perform recaptures with the least valuable piece for the to_sq
    for (; capt_cnt < TOTAL_PIECES; capt_cnt++) {
        uint64_t atk = attackers_of(b, side, to_sq, occ);
        if (!atk) break;

        int t = PAWN;
        uint64_t least = 0;
        for (; t <= KING; t++) {
            least = atk & typed_bb(b, side, t);
            if (least) break;
        }
        // breakpoint: king is last to recapture a piece but can't
        if (t == KING && attackers_of(b, ~side, to_sq, occ)) break;

        gain[capt_cnt] = val[attacker] - gain[capt_cnt - 1];  // score if this recapture happens
        attacker = t;
        occ ^= least & -least;  // remove the used attacker, may reveal an x-ray
        side = ~side;
    }

    // each side may decline to recapture: negamax the swap list backwards
    for (int i = capt_cnt - 1; i > 0; i--) { 
        gain[i - 1] = -max(-gain[i - 1], gain[i]);
    }
    return gain[0];
}

int qsearch(Board &b, int alpha, int beta, int qdepth) {
    qnodes_searched++;
    if ((qnodes_searched & 2047) == 0 && Clock::now() >= search_deadline)
        search_timed_out = true;

    bool check = b.in_check();
    if (search_timed_out || qdepth >= QSEARCH_MAX_PLY) {
        // alpha is a safe fail-low; standing pat while in check is not legal
        return check ? alpha : evaluate(b);
    }
    int static_eval = 0;

    if (!check) {
        static_eval = evaluate(b);
        if (static_eval >= beta) return beta;
        if (static_eval > alpha) alpha = static_eval;
    }

    // in check: every evasion must be searched, captures alone can't answer a check
    MoveList moves = check ? generate_moves<ALL_MOVES>(b) : generate_moves<CAPTURES>(b);
    if (check && moves.size == 0) {
        return -CHECKMATE + qdepth;
    }

    update_movelist_evals(b, moves);
    int it = 0;
    while (it < moves.size) {
        Move m = moves.get_next_move(it++);

        if (!check) {
            if (SEE(b, m) < 0) continue;
        }

        nnue_push(b, m);
        b.make_move(m);
        int score = -qsearch(b, -beta, -alpha, qdepth + 1);
        b.unmake_move(m);
        nnue_pop();

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}

static string score_string(int value, int depth) {
    if (value > MATE_SCORE) {
        int moves_to_mate = (CHECKMATE + depth - value + 1) / 2;
        return "mate " + to_string(moves_to_mate);
    } else if (value < -MATE_SCORE) {
        int moves_to_mate = (CHECKMATE + depth + value + 1) / 2;
        return "mate -" + to_string(moves_to_mate);
    }
    return "cp " + to_string(value);
}

TTEntry* probe(uint64_t key) {
    // only works when TT_SIZE is a power of 2
    return &tt[key & (TT_SIZE - 1)];
}

Move get_best_move(Board &b, int depth, int time_limit_ms) {
    MoveList root_moves = generate_moves<ALL_MOVES>(b);
    if (root_moves.size == 0) return Move();
    if (eval_mode == EVAL_NNUE && !nnue_ready()) {
        cerr << "\nFATAL: Eval=NNUE but no net is loaded.\n"
             << "  Build one with:  cd training && .venv/bin/python serialize.py <ckpt.pt>\n"
             << "  Or point at one: setoption name EvalFile value <path>\n"
             << "  Or switch eval:  setoption name Eval value pesto\n" << flush;
        cout << "info string FATAL: Eval=NNUE but no net loaded; refusing to search\n" << flush;
        exit(1);
    }
    nnue_reset();   // the board moved outside the search (UCI position, new game)

    // Reserve a margin for move output, but never let it exceed the budget --
    // a negative deadline used to time out before the first node and return a
    // default-constructed (illegal) move.
    const int margin = min(50, time_limit_ms / 4);
    auto time_limit = time_limit_ms > 0
        ? Clock::now() + std::chrono::milliseconds(time_limit_ms - margin)
        : TimePoint::max();

    search_deadline = time_limit;
    search_timed_out = false;
    clear_ordering_tables();
    scouts = researches = first_move_best = nodes_with_moves = 0;

    Move best_move = root_moves.moves[0];   // always a legal move to fall back on
    nodes_searched = 0;

    for (int d = 1; d <= depth; d++) {
        PVLine pv;
        auto start = Clock::now();

        int best_value = -CHECKMATE - 1;
        MoveList moves = generate_moves<ALL_MOVES>(b);
        update_movelist_evals(b, moves, d > 1 ? best_move : MOVE_NONE);

        Move iter_best = moves[0];
        bool out_of_time = false;
        int completed = 0;

        int it = 0;
        while (it < moves.size) {
            Move m = moves.get_next_move(it++);
            PVLine child_pv;
            nnue_push(b, m);
            b.make_move(m);
            int res = negamax(b, d - 1, 1, -INF, INF, time_limit, child_pv);
            b.unmake_move(m);
            nnue_pop();

            if (res == OUTOFTIME) { out_of_time = true; break; }
            completed++;
            int value = -res;

            if (value > best_value) {
                best_value = value;
                iter_best = m;
                pv.moves[0] = m;
                memcpy(pv.moves + 1, child_pv.moves, child_pv.length * sizeof(Move));
                pv.length = child_pv.length + 1;
            }

            if (value >= MATE_SCORE) {
                break;  // finds a forced mate
            }
        }

        if (out_of_time) {
            // Moves are ordered best-first, so a partially searched iteration
            // still improves on the previous one if any move finished.
            if (completed > 0) best_move = iter_best;
            break;
        }

        best_move = iter_best;
        double elapsed = duration<double, milli>(Clock::now() - start).count();
        cout << "info depth " << d
                << " score " << score_string(best_value, d)
                << " nodes " << nodes_searched
                << " time " << (int)elapsed
                << " pv";
        for (int i = 0; i < pv.length; i++)
            cout << " " << pv.moves[i].getName();
        cout << "\n";
        if (nodes_with_moves)
            cout << "info string ordering: cutoff-nodes " << nodes_with_moves
                 << " first-move " << first_move_best
                 << " | scouts " << scouts << " researches " << researches << "\n";
        cout.flush();
        
        if (best_value >= MATE_SCORE) {
            break;
        }
    }

    return best_move;
}

Move get_best_move(Board &b, int time_limit_ms) {
    // PVLine caps at 64 plies
    return get_best_move(b, 64, time_limit_ms);
}