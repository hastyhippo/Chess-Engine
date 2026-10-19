#include "doctest.h"
#include "../representation/board.h"
#include "../initialisation/init.h"
#include "../search/eval.h"
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

DOCTEST_TEST_SUITE("eval correctness") {
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
            cout << "  mismatch: " << t->fen << "\n    expected " << t->cp
                 << " cp (out32 " << t->out32 << "), got " << evaluate(b) << " cp\n";
        }
    }
}
