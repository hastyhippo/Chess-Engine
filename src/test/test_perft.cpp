#include "doctest.h"
#include "../representation/board.h"
#include "../movegen/move.h"
#include "../movegen/movegen.h"

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

DOCTEST_TEST_SUITE("perft") {
    TEST_CASE("Starting position - perft(0-6)") {
        Board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        CHECK(Perft(b, 0) == 1);
        CHECK(Perft(b, 1) == 20);
        CHECK(Perft(b, 2) == 400);
        CHECK(Perft(b, 3) == 8902);
        CHECK(Perft(b, 4) == 197281);
        CHECK(Perft(b, 5) == 4865609);
        CHECK(Perft(b, 6) == 119060324);
    }
}