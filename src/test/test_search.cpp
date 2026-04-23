#include "doctest.h"
#include "../representation/board.h"
#include "../movegen/movegen.h"
#include "../initialisation/init.h"
#include "../search/search.h"
#include <chrono>

struct MateTest {
    const char* fen;
    const char* best_move;
    int mate_in;
};

static const MateTest mate_tests[] = {
    { "2r3k1/p4p2/3Rp2p/1p2P1pK/8/1P4P1/P3Q2P/1q6 b - - 0 1",    "b1g6", 3 },
    {"1k5r/pP3ppp/3p2b1/1BN1n3/1Q2P3/P1B5/KP3P1P/7q w - - 1 0", "c5a6", 3},
    {"3r4/pR2N3/2pkb3/5p2/8/2B5/qP3PPP/4R1K1 w - - 1 0", "c3e5",3},
    {"4r1k1/5bpp/2p5/3pr3/8/1B3pPq/PPR2P2/2R2QK1 b - - 0 1", "e5e1",3},
    {"8/k2r4/p7/2b1Bp2/P3p3/qp4R1/4QP2/1K6 b - - 0 1", "d7d1", 4},
    {"7R/r1p1q1pp/3k4/1p1n1Q2/3N4/8/1PP2PPP/2B3K1 w - - 1 0", "h8d8", 4}
};

DOCTEST_TEST_SUITE("search correctness") {
    TEST_CASE("Mate finder") {
        for (auto& t : mate_tests) {
            Board b(t.fen);
            int search_depth = t.mate_in * 2 - 1;

            auto start = chrono::high_resolution_clock::now();
            Move best = get_best_move(b, search_depth);
            double ms = chrono::duration<double, milli>(
                chrono::high_resolution_clock::now() - start).count();

            string found = best.getName();
            cout <<found<<"\n";
            bool correct = (found == t.best_move);
            cout << "mate in " << t.mate_in
                 << " | expected " << t.best_move
                 << " | got " << found
                 << " | " << (correct ? "PASS" : "FAIL")
                 << " | " << ms << "ms\n";
            CHECK(correct);
        }
    }
}
