

#include "doctest.h"
#include "../representation/board.h"
#include "../movegen/move.h"
#include "../movegen/movegen.h"

DOCTEST_TEST_SUITE("move generation correctness") {
    TEST_CASE("Pawn moves flipped") {
        Board b("b1n5/PP4B1/1P4P1/3Pp3/3P4/P3bpqp/PP3PPP/1K5k w - e6 0 1");
        vector<Move> moves = generateMoves(b);
        Board b2("1k5K/pp3ppp/p3BPQP/3p4/3pP3/1p4p1/pp4b1/B1N5 b - e3 0 1");
        vector<Move> moves2 = generateMoves(b2);
        CHECK(moves2.size() == moves.size());    
    }

    TEST_CASE("Perft B/W are same") {
        Board w("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        Board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1");
        CHECK(Perft(b, 1) == Perft(w, 1));
        CHECK(Perft(b, 2) == Perft(w, 2));
        CHECK(Perft(b, 3) == Perft(w, 3));
        CHECK(Perft(b, 4) == Perft(w, 4));
        CHECK(Perft(b, 5) == Perft(w,5));
    }
}