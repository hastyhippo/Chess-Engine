

#include "doctest.h"
#include "../representation/board.h"
#include "../movegen/move.h"
#include "../movegen/movegen.h"
#include "../initialisation/init.h"

DOCTEST_TEST_SUITE("move generation correctness") {
    TEST_CASE("Pawn moves flipped") {
        Board b("b1n5/PP4B1/1P4P1/3Pp3/3P4/P3bpqp/PP3PPP/1K5k w - e6 0 1");
        MoveList moves = generateMoves<ALL_MOVES>(b);
        Board b2("1k5K/pp3ppp/p3BPQP/3p4/3pP3/1p4p1/pp4b1/B1N5 b - e3 0 1");
        MoveList moves2 = generateMoves<ALL_MOVES>(b2);
        CHECK(moves2.size == moves.size);    
    }

    TEST_CASE("Perft B/W are same") {
        Board w("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        Board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1");
        CHECK(perft(b, 1) == perft(w, 1));
        CHECK(perft(b, 2) == perft(w, 2));
        CHECK(perft(b, 3) == perft(w, 3));
        CHECK(perft(b, 4) == perft(w, 4));
        CHECK(perft(b, 5) == perft(w,5));
    }
}