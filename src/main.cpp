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
    
    // while(true) {
    //     string s;
    //     getline(cin, s);
    //     Board b(s);
    //     vector<Move> moves = generateMoves<ALL_MOVES>(b);
    //     cout << "STARTING POSITION\n";
    //     for (Move a : moves) {
    //         cout << "MOVE: " << a.getName() << "\n";
    //         b.makeMove(a);
    //         b.printBoard();
    //         b.unmakeMove();
    //     }
    
    // }
    // Board b("k2r4/8/7b/8/4p1n1/8/4K3/1q6 w - - 0 1");
    // // Board b("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

    // vector<Move> moves = generateMoves<ALL_MOVES>(b);
    // cout << "STARTING POSITION\n";
    // for (Move a : moves) {
    //     cout << "MOVE: " << a.getName() << "\n";
    //     b.makeMove(a);
    //     b.printBoard();
    //     b.unmakeMove();
    // }
    vector<string> v;
    // divided_perft(b, 3,  1,   v);
    Board b2("rnb1kbnr/pp1ppppp/8/q1p5/8/3P4/PPPKPPPP/RNBQ1BNR w kq - 0 1");
    vector<Move> moves = generateMoves<ALL_MOVES>(b2);
    cout << "STARTING POSITION\n";
    for (Move a : moves) {
        cout << "MOVE: " << a.getName() << "\n";
        b2.makeMove(a);
        b2.printBoard();
        b2.unmakeMove();
    }

    divided_perft(b2, 3, 1, v);
}

// d2d3 | nodes: 328515 -> 328511
/*
c7c6 | nodes: 15207 -> 15206
e1d2 | nodes: 488 -> 487



e7e6 | nodes: 21625
c7c5 | nodes: 15972
e7e5 | nodes: 21640

c7c6: 15206
e7e6: 21624
c7c5: 15971
e7e5: 21639
*/




// f2f3 | nodes: 178891 -> 178889
// a2a4 | nodes: 217831 -> 217832
// b2b4 | nodes: 216129 -> 216145
// c2c4 | nodes: 240081 -> 240082
// d2d4 | nodes: 361794 -> 361790
// f2f4 | nodes: 198475 -> 198473