

#include "doctest.h"
#include "../representation/board.h"
#include "../movegen/movegen.h"
#include "../initialisation/init.h"
#include <chrono>

void timedPerft(Board &b, int depth) {
    auto start = chrono::high_resolution_clock::now();
    uint64_t nodes = perft(b, depth);
    auto end = chrono::high_resolution_clock::now();
    double seconds = chrono::duration<double>(end - start).count();
    cout << "single thread  perft " << depth << " | " << nodes << " nodes in " << seconds << "s | " << nodes/seconds/1e6 << "m nps\n";
}

void timedPerftMT(Board &b, int depth) {
    auto start = chrono::high_resolution_clock::now();
    uint64_t nodes = perft_mt(b, depth);
    auto end = chrono::high_resolution_clock::now();
    double seconds = chrono::duration<double>(end - start).count();
    cout << "multi thread  perft " << depth << " | " << nodes << " nodes in " << seconds << "s | " << nodes/seconds/1e6 << "m nps\n";
}

DOCTEST_TEST_SUITE("movegen speed") {
    TEST_CASE("perft Timed") {
        Board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        cout << "--------------------------| single-threading |---------------------------------"<<"\n";
        for (int i = 1; i <= 6; i++) {
            timedPerft(b, i);
        }
        cout << "\n--------------------------| multi-threading |---------------------------------"<<"\n";
        for (int i = 1; i <= 7; i++)  {
            timedPerftMT(b, i);
        }
    }
}