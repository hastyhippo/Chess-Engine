#include "doctest.h"
#include "../representation/board.h"
#include "../search/search.h"
#include "../initialisation/init.h"
#include <chrono>

using namespace std;

void timedSearch(Board &b, Colour side, int depth) {
    auto start = chrono::high_resolution_clock::now();
    int score = negamax(b, side, depth);
    auto end = chrono::high_resolution_clock::now();
    double seconds = chrono::duration<double>(end - start).count();
    cout << "search depth " << depth << " | score: " << score << " | time: " << seconds << " seconds\n";
}

void timedSearchBestMove(Board &b, Colour side, int depth) {
    auto start = chrono::high_resolution_clock::now();
    Move best_move = get_best_move(b, side, depth);
    auto end = chrono::high_resolution_clock::now();
    double seconds = chrono::duration<double>(end - start).count();
    cout << "search best move depth " << depth << " | move: " << best_move.getName() << " | time: " << seconds << " seconds\n";
}

DOCTEST_TEST_SUITE("search speed") {
    TEST_CASE("Starting position - timed search") {
        Board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        Colour side = b.getWhiteTurn() ? WHITE : BLACK;
            timedSearch(b, side, 5);
    }

    TEST_CASE("Kiwipete - timed search") {
        Board b("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
        Colour side = b.getWhiteTurn() ? WHITE : BLACK;
            timedSearch(b, side, 5);
    }

    TEST_CASE("Position 3 - timed search") {
        Board b("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
        Colour side = b.getWhiteTurn() ? WHITE : BLACK;
            timedSearch(b, side, 5);
    }

    TEST_CASE("TalkChess Position - timed search") {
        Board b("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
        Colour side = b.getWhiteTurn() ? WHITE : BLACK;
            timedSearch(b, side, 5);
    }

    TEST_CASE("Position 4 - timed search") {
        Board b("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");
        Colour side = b.getWhiteTurn() ? WHITE : BLACK;
            timedSearch(b, side, 5);
    }

    TEST_CASE("Starting position - timed best move") {
        Board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        Colour side = b.getWhiteTurn() ? WHITE : BLACK;
            timedSearchBestMove(b, side, 5);
    }

    TEST_CASE("Kiwipete - timed best move") {
        Board b("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
        Colour side = b.getWhiteTurn() ? WHITE : BLACK;
            timedSearchBestMove(b, side, 5);
    }
}

