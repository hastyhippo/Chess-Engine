#include "doctest.h"
#include "../representation/board.h"
#include "../movegen/movegen.h"
#include "../search/search.h"
#include <chrono>
#include <cstdio>

extern int nodes_searched;

struct BenchPos {
    const char* name;
    const char* fen;
};

// drawn from the perft/search correctness suites
static const BenchPos bench_positions[] = {
    { "startpos",   "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" },
    { "kiwipete",   "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1" },
    { "endgame",    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1" },
    { "talkchess",  "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8" },
    { "middlegame", "2r3k1/p4p2/3Rp2p/1p2P1pK/8/1P4P1/P3Q2P/1q6 b - - 0 1" },
};

DOCTEST_TEST_SUITE("search speed") {
    TEST_CASE("Branching factor and speed at depth 5") {
        const int DEPTH = 5;
        double ebf_sum = 0;
        int ebf_count = 0;

        for (auto& p : bench_positions) {
            Board b(p.fen);
            printf("\n%s | %s\n", p.name, p.fen);
            printf("  depth |      nodes |  time ms |   mnps |  EBF\n");

            long long prev_nodes = 0;
            for (int d = 1; d <= DEPTH; d++) {
                nodes_searched = 0;
                PVLine pv;
                auto t0 = chrono::high_resolution_clock::now();
                int score = negamax(b, d, -INF, INF, TimePoint::max(), pv);
                double ms = chrono::duration<double, milli>(
                    chrono::high_resolution_clock::now() - t0).count();

                long long nodes = nodes_searched;
                double ebf = prev_nodes ? (double)nodes / prev_nodes : 0;
                double mnps = ms > 0 ? nodes / ms / 1000.0 : 0;
                printf("  %5d | %10lld | %8.1f | %6.2f | %5.2f\n",
                       d, nodes, ms, mnps, ebf);

                CHECK(nodes > 0);
                if (d == DEPTH) { ebf_sum += ebf; ebf_count++; }
                prev_nodes = nodes;
                (void)score;
            }
        }
        printf("\nmean EBF at depth %d across %d positions: %.2f\n",
               DEPTH, ebf_count, ebf_sum / ebf_count);
    }
}
