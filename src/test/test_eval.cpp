#include "doctest.h"
#include "../representation/board.h"
#include "../initialisation/init.h"
#include "../search/eval.h"
#include "../search/nnue.h"
#include "../movegen/movegen.h"
#include <fstream>

#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR "src/test"
#endif

// One line of nnue_example.txt: FEN|out32|cp
// out32 is the raw int32 from the output layer, cp the converted score. Both
// come from running the integer pipeline over nnue.bin, so an NNUE eval has to
// reproduce cp exactly -- it is integer arithmetic on both sides.
struct OraclePos {
    string fen;
    long long out32;
    int cp;
};

static vector<OraclePos> load_oracle(const string &path) {
    vector<OraclePos> out;
    ifstream in(path);
    if (!in) return out;

    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        size_t b = line.rfind('|');
        if (b == string::npos) continue;
        size_t a = line.rfind('|', b - 1);
        if (a == string::npos) continue;
        out.push_back({line.substr(0, a),
                       stoll(line.substr(a + 1, b - a - 1)),
                       stoi(line.substr(b + 1))});
    }
    return out;
}

// Walk the tree under push/make/unmake/pop and check the incrementally updated
// accumulator against one rebuilt from the board. `eval_leaves_only` makes the
// replay chains as long as the walk is deep, which is what a real search does.
static void walk(Board &b, int depth, bool eval_leaves_only,
                 long long &checked, long long &bad, string &first_bad) {
    if (depth == 0 || !eval_leaves_only) {
        int inc = nnue_raw(b);
        int ref = nnue_raw_refresh(b);
        checked++;
        if (inc != ref && first_bad.empty()) {
            first_bad = "incremental " + to_string(inc) + " vs refresh " + to_string(ref);
        }
        if (inc != ref) bad++;
    }
    if (depth == 0) return;

    MoveList ml = generate_moves<ALL_MOVES>(b);
    for (int i = 0; i < ml.size; i++) {
        Move m = ml.moves[i];
        nnue_push(b, m);
        b.make_move(m);
        walk(b, depth - 1, eval_leaves_only, checked, bad, first_bad);
        b.unmake_move(m);
        nnue_pop();
    }
}

DOCTEST_TEST_SUITE("eval correctness") {
    TEST_CASE("feature dump") {
        vector<OraclePos> cases = load_oracle(string(TEST_DATA_DIR) + "/nnue_examples.txt");
        REQUIRE(cases.size() > 0);
        int f[64];
        for (int i = 0; i < 3; i++) {
            Board b(cases[i].fen);
            cout << "FEN " << cases[i].fen << "\n";
            for (int pov = 1; pov >= 0; pov--) {
                int n = nnue_debug_features(b, pov, f);
                sort(f, f + n);
                cout << (pov ? "  white " : "  black ") << n << ":";
                for (int k = 0; k < n; k++) cout << " " << f[k];
                cout << "\n";
            }
        }
    }

    TEST_CASE("incremental accumulator") {
        static const char *roots[] = {
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
            "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
            "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
        };
        for (const char *fen : roots) {
            for (bool leaves_only : {false, true}) {
                Board b(fen);
                nnue_reset();
                long long checked = 0, bad = 0;
                string first_bad;
                walk(b, leaves_only ? 4 : 3, leaves_only, checked, bad, first_bad);
                cout << (leaves_only ? "chains  " : "every   ") << " depth "
                     << (leaves_only ? 4 : 3) << " | " << checked << " nodes | "
                     << bad << " mismatches  " << first_bad << "\n";
                CHECK(bad == 0);
            }
        }
    }

    TEST_CASE("KNOWN NNUE examples from Leela are used to test") {
        const string path = string(TEST_DATA_DIR) + "/nnue_examples.txt";
        vector<OraclePos> cases = load_oracle(path);

        cout << "oracle: " << cases.size() << " positions from " << path << "\n";
        REQUIRE(cases.size() > 0);

        // Only the NNUE eval is expected to reproduce these numbers; the other
        // modes still exercise FEN parsing and evaluate() over 500 positions.
        const bool strict = (eval_mode == EVAL_NNUE);

        int exact = 0, worst = 0, shown = 0;
        long long abs_total = 0;
        vector<const OraclePos *> bad;

        for (const OraclePos &t : cases) {
            Board b(t.fen);
            int got = evaluate(b);
            int diff = abs(got - t.cp);

            if (diff == 0) exact++;
            else bad.push_back(&t);
            abs_total += diff;
            worst = max(worst, diff);
        }

        cout << "eval mode: " << eval_mode_name()
             << " | exact " << exact << "/" << cases.size()
             << " | mean |diff| " << (abs_total / (long long)cases.size())
             << " cp | worst " << worst << " cp\n";

        // One assertion, not 500: a wall of CHECK failures hides the signal.
        for (const OraclePos *t : bad) {
            if (shown++ >= 5) break;
            Board b(t->fen);
            cout << "  mismatch: " << t->fen << "\n"
                 << "    cp    expected " << t->cp << "  got " << evaluate(b) << "\n"
                 << "    out32 expected " << t->out32 << "  got " << nnue_raw(b) << "\n";
        }
    }
}
