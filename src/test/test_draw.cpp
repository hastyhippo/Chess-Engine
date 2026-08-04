#include "doctest.h"
#include "../representation/board.h"
#include "../movegen/movegen.h"
#include "../search/search.h"
#include "../misc/defines.h"

static Move find_move(Board& b, const string& name) {
    MoveList moves = generate_moves<ALL_MOVES>(b);
    for (Move m : moves)
        if (m.getName() == name) return m;
    FAIL("move not found: ", name);
    return Move();
}

static void play(Board& b, const vector<string>& names) {
    for (auto& n : names) b.make_move(find_move(b, n));
}

DOCTEST_TEST_SUITE("search correctness") {

    TEST_CASE("Knight shuffle repeats the start position") {
        Board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        CHECK_FALSE(b.is_repetition());

        play(b, {"g1f3", "g8f6", "f3g1"});
        CHECK_FALSE(b.is_repetition());

        play(b, {"f6g8"});
        CHECK(b.is_repetition());
    }

    TEST_CASE("Repetition cleared by unmake") {
        Board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        Move n_back;
        play(b, {"g1f3", "g8f6", "f3g1"});
        n_back = find_move(b, "f6g8");
        b.make_move(n_back);
        CHECK(b.is_repetition());
        b.unmake_move(n_back);
        CHECK_FALSE(b.is_repetition());
    }

    TEST_CASE("Different castling rights are not a repetition") {
        // same piece placement, but the rook trip burns black's castling rights
        Board b("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
        play(b, {"a1b1", "a8b8", "b1a1", "b8a8"});
        CHECK_FALSE(b.is_repetition());
    }

    TEST_CASE("Hash survives make/unmake round trips") {
        Board b("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
        uint64_t h0 = b.getHash();
        CHECK(h0 == zobrist_hash(b));

        Move m = find_move(b, "e1g1");  // castle
        b.make_move(m);
        CHECK(b.getHash() == zobrist_hash(b));
        b.unmake_move(m);
        CHECK(b.getHash() == h0);
    }

    TEST_CASE("Halfmove clock counts, resets, and unmakes") {
        Board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        CHECK(b.getHalfMoveClock() == 0);

        play(b, {"g1f3", "g8f6"});
        CHECK(b.getHalfMoveClock() == 2);

        play(b, {"e2e4"});  // pawn move resets
        CHECK(b.getHalfMoveClock() == 0);

        play(b, {"f6e4"});  // capture resets
        CHECK(b.getHalfMoveClock() == 0);

        Move m = find_move(b, "b1c3");
        b.make_move(m);
        CHECK(b.getHalfMoveClock() == 1);
        b.unmake_move(m);
        CHECK(b.getHalfMoveClock() == 0);
    }

    TEST_CASE("Search scores a 50-move-rule position as a draw") {
        // clock at 99: any quiet reply reaches 100
        Board b("4k3/8/8/8/8/8/8/4K2R w K - 99 80");
        PVLine pv;
        b.make_move(find_move(b, "h1h2"));
        CHECK(b.getHalfMoveClock() == 100);
        int score = negamax(b, 3, -INF, INF, TimePoint::max(), pv);
        CHECK(score == 0);
    }

    TEST_CASE("Search scores a repeated position as a draw") {
        Board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        play(b, {"g1f3", "g8f6", "f3g1", "f6g8"});  // back to startpos

        PVLine pv;
        int score = negamax(b, 4, -INF, INF, TimePoint::max(), pv);
        CHECK(score == 0);
    }
}
