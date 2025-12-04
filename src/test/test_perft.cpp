#include "doctest.h"
#include "../representation/board.h"
#include "../misc/defines.h"
#include "../initialisation/init.h"
#include "../movegen/movegen.h"
#include <chrono>

void timedPerft(Board &b, int depth) {
    auto start = chrono::high_resolution_clock::now();
    uint64_t nodes = perft(b, depth);
    auto end = chrono::high_resolution_clock::now();
    double seconds = chrono::duration<double>(end - start).count();
    double nps = nodes/seconds;
    cout << "perft : " << depth <<" | " << nodes << " nodes in " << seconds << " seconds | " << nps/1e6 << "m nodes/sec\n"; 
}

DOCTEST_TEST_SUITE("perft") { // Sourced from https://www.chessprogramming.org/Perft_Results
    TEST_CASE("Starting position - perft(1-6)") {
        Board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        CHECK(perft(b, 1) == 20);
        CHECK(perft(b, 2) == 400);
        CHECK(perft(b, 3) == 8902);
        CHECK(perft(b, 4) == 197281);
        // CHECK(perft(b, 5) == 4865609);
        // CHECK(perft(b, 6) == 119060324);
    }

    TEST_CASE("Kiwipete - perft(1-5)") { 
        Board b("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
        CHECK(perft(b, 1) == 48);
        CHECK(perft(b, 2) == 2039);
        CHECK(perft(b, 3) == 97862);
        // CHECK(perft(b, 4) == 4085603);
        // CHECK(perft(b, 5) == 193690690);
    }

    TEST_CASE("perft Position 3 - perft(1-7)") {
        Board b("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
        CHECK(perft(b, 1) == 14);
        CHECK(perft(b, 2) == 191);
        CHECK(perft(b, 3) == 2812);
        CHECK(perft(b, 4) == 43238);
        // CHECK(perft(b, 5) == 674624);
        // CHECK(perft(b, 6) == 11030083);
        // CHECK(perft(b, 7) == 178633661);
    }

    TEST_CASE("TalkChess Position - perft(1-5)") {
        Board b("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
        CHECK(perft(b, 1) == 44);
        CHECK(perft(b, 2) == 1486);
        CHECK(perft(b, 3) == 62379);
        // CHECK(perft(b, 4) == 2103487);
        // CHECK(perft(b, 5) == 89941194);
    }

    TEST_CASE("Position 4 - perft(1-5)") {
        Board b("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");
        CHECK(perft(b, 1) == 46);
        CHECK(perft(b, 2) == 2079);
        CHECK(perft(b, 3) == 89890);
        // CHECK(perft(b, 4) == 3894594);
        // CHECK(perft(b, 5) == 164075551);
    }

    TEST_CASE("Harder Positional Tests") { // Sourced from: https://www.chessprogramming.net/perfect-perft/
        //--Illegal ep move #1
        Board b("3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1");
        CHECK(perft(b, 6) == 1134888);

        //--Illegal ep move #2
        Board b2("8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1");
        CHECK(perft(b2, 6) == 1015133);

        //--EP Capture Checks Opponent
        Board b3("8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1");
        CHECK(perft(b3, 6) == 1440467);
        
        // Short Castling Gives Check
        Board b4("5k2/8/8/8/8/8/8/4K2R w K - 0 1");
        CHECK(perft(b4, 6) == 661072);

        // Long Castling Gives Check
        Board b5("3k4/8/8/8/8/8/8/R3K3 w Q - 0 1");
        CHECK(perft(b5, 6) == 803711);

        // Castle Rights
        Board b6("r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1");
        CHECK(perft(b6, 4) == 1274206);

        // Castling Prevented
        Board b7("r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1");
        CHECK(perft(b7, 4) == 1720476);

        // Promote out of Check
        // Board b8("2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1");
        // CHECK(perft(b8, 6) == 3821001);

        // Discovered Check
        Board b9("8/8/1P2K3/8/2n5/1q6/8/5k2 b - - 0 1");
        CHECK(perft(b9, 5) == 1004658);

        // Promote to give check
        Board b10("4k3/1P6/8/8/8/8/K7/8 w - - 0 1");
        CHECK(perft(b10, 6) == 217342);

        // Under Promote to give check
        Board b11("8/P1k5/K7/8/8/8/8/8 w - - 0 1");
        CHECK(perft(b11, 6) == 92683);

        // Self Stalemate
        Board b12("K1k5/8/P7/8/8/8/8/8 w - - 0 1");
        CHECK(perft(b12, 6) == 2217);

        // Stalemate & Checkmate
        Board b13("8/k1P5/8/1K6/8/8/8/8 w - - 0 1");
        CHECK(perft(b13, 7) == 567584);

        // Stalemate & Checkmate
        Board b14("8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1");
        CHECK(perft(b14, 4) == 23527);
    }

    TEST_CASE("perft Timed") {
        Board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        for (int i = 1; i <= 5; i++) {
            timedPerft(b, i);
        }

        Board b2("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
        for (int i = 1; i <= 6; i++) {
            timedPerft(b2, i);
        }
    }
}