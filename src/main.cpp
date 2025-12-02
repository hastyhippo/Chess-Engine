#include <bits/stdc++.h>
#include "./initialisation/init.h"
#include "./representation/board.h"
#include "./movegen/movegen.h"
#include "./movegen/move.h"
#include "cassert"
#include "misc/defines.h"
#define DOCTEST_CONFIG_IMPLEMENT
#include "./test/doctest.h"

using namespace std;


int main(int argc, char** argv) {
    init();
    
    for (int i = 1; i < argc; i++) {
        if (string(argv[i]) == "-t") {
            doctest::Context context;
            context.applyCommandLine(argc, argv);
            int res = context.run();
            if (context.shouldExit()) {
                return res;
            }
            return 0;
        }
    }
    // Board b("N6N/4Bb2/2N3b1/3NB3/8/8/8/N1k2K1N w - - 0 1");
    // vector<Move> moves = generateMoves(b);
    // cout << "| ";
    // for (Move a : moves) {
    //     cout << a.getName() << " \n";
    // }

    // Board b2("n6n/4bB2/2n3B1/3nb3/8/8/8/n1k3Kn b - - 0 1");
    // vector<Move> moves2 = generateMoves(b2);
    // cout << "| ";
    // for (Move a : moves2) {
    //     cout << a.getName() << " \n";
    // }
    // assert(moves2.size() == moves.size());
    
    Board b("rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1");
    // Board b("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

    vector<Move> moves = generateMoves<ALL_MOVES>(b);
    cout << "STARTING POSITION\n";
    for (Move a : moves) {
        cout << "MOVE: " << a.getName() << "\n";
        b.makeMove(a);
        b.printBoard();
        b.unmakeMove();
    }
    vector<string> v;
    // divided_perft(b, 3,  1,   v);
    // Board b2("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1");
    // divided_perft(b2, 3, 1, v);
}