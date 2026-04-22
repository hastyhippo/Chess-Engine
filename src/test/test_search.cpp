#include "doctest.h"
#include "../representation/board.h"
#include "../search/search.h"
#include "../initialisation/init.h"
#include <chrono>

using namespace std;

void timedSearch(Board &b, int depth) {
    auto start = chrono::high_resolution_clock::now();
    int score = negamax(b, depth);
    auto end = chrono::high_resolution_clock::now();
    double seconds = chrono::duration<double>(end - start).count();
    cout << "search depth " << depth << " | score: " << score << " | time: " << seconds << " seconds\n";
}

void timedSearchBestMove(Board &b, int depth) {
    auto start = chrono::high_resolution_clock::now();
    Move best_move = get_best_move(b, depth);
    auto end = chrono::high_resolution_clock::now();
    double seconds = chrono::duration<double>(end - start).count();
    cout << "search best move depth " << depth << " | move: " << best_move.getName() << " | time: " << seconds << " seconds\n";
}

int get_search_result(Board &b, int depth) {
    int score = negamax(b, depth);
    return score;
}

DOCTEST_TEST_SUITE("search speed") {
    // TEST_CASE("Starting position - timed search") {
    //     Board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    //     timedSearch(b, 5);
    // }

    // TEST_CASE("Kiwipete - timed search") {
    //     Board b("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    //     timedSearch(b, 5);
    // }

    // TEST_CASE("Position 3 - timed search") {
    //     Board b("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
    //     timedSearch(b, 5);
    // }

    // TEST_CASE("TalkChess Position - timed search") {
    //     Board b("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
    //     timedSearch(b, 5);
    // }

    // TEST_CASE("Position 4 - timed search") {
    //     Board b("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");
    //     timedSearch(b, 5);
    // }

    // TEST_CASE("Starting position - timed best move") {
    //     Board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    //     timedSearchBestMove(b, 5);
    // }

    // TEST_CASE("Kiwipete - timed best move") {
    //     Board b("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    //     timedSearchBestMove(b, 5);
    // }
    TEST_CASE("mate in 3"){
        Board b("2r3k1/p4p2/3Rp2p/1p2P1pK/8/1P4P1/P3Q2P/1q6 b - - 0 1");
        CHECK(get_search_result(b, 5) == CHECKMATE);
    }

    // TEST_CASE("mate in 4") {
    //     vector<string> fens = {
    //         "8/k2r4/p7/2b1Bp2/P3p3/qp4R1/4QP2/1K6 b - - 0 1",
    //         "r1bk3r/pppq1ppp/5n2/4N1N1/2Bp4/Bn6/P4PPP/4R1K1 w - - 1 0",
    //         "7R/r1p1q1pp/3k4/1p1n1Q2/3N4/8/1PP2PPP/2B3K1 w - - 1 0",
    //         "Q7/p1p1q1pk/3p2rp/4n3/3bP3/7b/PP3PPK/R1B2R2 b - - 0 1",
    //         "4k2r/1R3R2/p3p1pp/4b3/1BnNr3/8/P1P5/5K2 w - - 1 0",
    //         "r2r1n2/pp2bk2/2p1p2p/3q4/3PN1QP/2P3R1/P4PP1/5RK1 w - - 0 1",
    //         "3r1r2/1pp2p1k/p5pp/4P3/2nP3R/2P3QP/P1B1q1P1/5RK1 w - - 1 0"
    //     };

    //     Board b;

    //     for (string fen : fens) {
    //         b = Board(fen);
    //         get_best_move(b, 7);
    //         CHECK(get_search_result(b, 7) == CHECKMATE);
    //     }
    // }
    // TEST_CASE("mate in 5") {
    //     vector<string> fens = {
    //         "4rb1k/2pqn2p/6pn/ppp3N1/P1QP2b1/1P2p3/2B3PP/B3RRK1 w - - 0 24",
    //         "6k1/3b3r/1p1p4/p1n2p2/1PPNpP1q/P3Q1p1/1R1RB1P1/5K2 b - - 0 1",
    //         "2q1nk1r/4Rp2/1ppp1P2/6Pp/3p1B2/3P3P/PPP1Q3/6K1 w - - 0 1",
    //         "6r1/p3p1rk/1p1pPp1p/q3n2R/4P3/3BR2P/PPP2QP1/7K w - - 0 1"
    //     };
    //     Board b;

    //     for (string fen : fens) {
    //         b = Board(fen);
    //         CHECK(get_search_result(b, 9) == CHECKMATE);
    //     }
    // }

    TEST_CASE("Mate in 1") {
        Board b("4r2k/1p3rbp/2p1N1p1/p3n3/P2NB1nq/1P6/4R1P1/B1Q2RK1 b - - 4 32");
        timedSearchBestMove(b, 1);
    }
}

