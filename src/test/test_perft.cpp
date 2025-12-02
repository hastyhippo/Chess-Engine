#include "doctest.h"
#include "../representation/board.h"
#include "../misc/defines.h"
#include <chrono>

void timedPerft(Board &b, int depth) {
    auto start = chrono::high_resolution_clock::now();
    uint64_t nodes = perft(b, depth);
    auto end = chrono::high_resolution_clock::now();
    double seconds = chrono::duration<double>(end - start).count();
    double nps = nodes/seconds;
    cout << "perft : " << depth <<" | " << nodes << " nodes in " << seconds << " seconds | " << nps/1e6 << "m nodes/sec\n"; 
}

DOCTEST_TEST_SUITE("perft") {
    TEST_CASE("Starting position - perft(1-6)") {
        Board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        CHECK(perft(b, 1) == 20);
        CHECK(perft(b, 2) == 400);
        CHECK(perft(b, 3) == 8902);
        CHECK(perft(b, 4) == 197281);
        CHECK(perft(b, 5) == 4865609);
        // CHECK(perft(b, 6) == 119060324);
    }

    TEST_CASE("Kiwipete - perft(1-5)") {
        Board b("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
        CHECK(perft(b, 1) == 48);
        CHECK(perft(b, 2) == 2039);
        CHECK(perft(b, 3) == 97862);
        CHECK(perft(b, 4) == 4085603);
        // CHECK(perft(b, 5) == 193690690);
    }

    TEST_CASE("perft Position 3 - perft(1-7)") {
        Board b("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
        CHECK(perft(b, 1) == 14);
        CHECK(perft(b, 2) == 191);
        CHECK(perft(b, 3) == 2812);
        CHECK(perft(b, 4) == 43238);
        CHECK(perft(b, 5) == 674624);
        // CHECK(perft(b, 6) == 11030083);
        // CHECK(perft(b, 7) == 178633661);
    }

    // TEST_CASE("TalkChess Position - perft(1-5)") {
    //     Board b("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
    //     CHECK(perft(b, 1) == 44);
    //     CHECK(perft(b, 2) == 1486);
    //     CHECK(perft(b, 3) == 62379);
    //     CHECK(perft(b, 4) == 2103487);
    //     // CHECK(perft(b, 5) == 89941194);
    // }

    
    TEST_CASE("perft Timed") {
        Board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        for (int i = 1; i <= 5; i++) {
            timedPerft(b, i);
        }
    }
}