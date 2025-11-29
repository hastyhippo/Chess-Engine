#include "doctest.h"
#include "../representation/board.h"
#include "../movegen/move.h"
#include "../movegen/movegen.h"
#include <chrono>

uint64_t Perft(Board &b, int depth) {
    if (depth == 0) {
        return (uint64_t)1;
    }
    
    uint64_t n_moves = 0;
    vector<Move> move_list = generateMoves(b);

    for (Move m : move_list) {
        b.makeMove(m);
        n_moves += Perft(b, depth - 1);
        b.unmakeMove();
    }
    return n_moves;
}

void timedPerft(Board &b, int depth) {
    auto start = chrono::high_resolution_clock::now();
    uint64_t nodes = Perft(b, depth);
    auto end = chrono::high_resolution_clock::now();
    double seconds = chrono::duration<double>(end - start).count();
    double nps = nodes/seconds;
    cout << "Perft : " << depth <<" | " << nodes << " nodes in " << seconds << " seconds | " << nps << " nodes/sec\n"; 

}
DOCTEST_TEST_SUITE("perft") {
    TEST_CASE("Starting position - perft(1-6)") {
        Board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        CHECK(Perft(b, 1) == 20);
        CHECK(Perft(b, 2) == 400);
        CHECK(Perft(b, 3) == 8902);
        CHECK(Perft(b, 4) == 197281);
        CHECK(Perft(b, 5) == 4865609);
        CHECK(Perft(b, 6) == 119060324);
    }



    TEST_CASE("Kiwipete - perft(1-5)") {
        Board b("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
        CHECK(Perft(b, 1) == 48);
        CHECK(Perft(b, 2) == 2039);
        CHECK(Perft(b, 3) == 97862);
        CHECK(Perft(b, 4) == 4085603);
        CHECK(Perft(b, 5) == 193690690);
    }

    TEST_CASE("Perft Position 3 - perft(1-7)") {
        Board b("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
        CHECK(Perft(b, 1) == 14);
        CHECK(Perft(b, 2) == 191);
        CHECK(Perft(b, 3) == 2812);
        CHECK(Perft(b, 4) == 43238);
        CHECK(Perft(b, 5) == 674624);
        CHECK(Perft(b, 6) == 11030083);
        CHECK(Perft(b, 7) == 178633661);
    }

    TEST_CASE("TalkChess Position - perft(1-5)") {
        Board b("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
        CHECK(Perft(b, 1) == 44);
        CHECK(Perft(b, 2) == 1486);
        CHECK(Perft(b, 3) == 62379);
        CHECK(Perft(b, 4) == 2103487);
        CHECK(Perft(b, 5) == 89941194);
    }

    TEST_CASE("Perft Timed") {
        Board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        for (int i = 1; i <= 7; i++) {
            timedPerft(b, i);
        }
    }
}