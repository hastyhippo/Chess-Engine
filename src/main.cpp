#include <bits/stdc++.h>
#include "./initialisation/init.h"
#include "./representation/board.h"
#include "./movegen/movegen.h"
#include "cassert"
#include "misc/defines.h"
#define DOCTEST_CONFIG_IMPLEMENT
#include "./test/doctest.h"

using namespace std;

void timedPerft2(Board &b, int depth) {
    auto start = chrono::high_resolution_clock::now();
    uint64_t nodes = perft(b, depth);
    auto end = chrono::high_resolution_clock::now();
    double seconds = chrono::duration<double>(end - start).count();
    double nps = nodes/seconds;
    cout << "perft : " << depth <<" | " << nodes << " nodes in " << seconds << " seconds | " << nps/1e6 << "m nodes/sec\n"; 
}
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
    Board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    for (int i = 1; i <= 5; i++) {
        timedPerft2(b, i);
    }
    while(true) {
        
        string s;
        getline(cin, s);
        Board b(s);
        int depth;
        // timedPerft2(b, 5);
        
        MoveList moves = generateMoves<ALL_MOVES>(b);
        for (Move a : moves) {
            cout << a.getName() << "\n";
            b.makeMove(a);
            // b.printBoard();
            b.unmakeMove(a);
        }
        cin >> depth;
        vector<string> v;
        divided_perft(b, depth, 1, v);
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
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
    Board b2("r3k2r/p1pNqpb1/bn2pnp1/3P4/1p2P3/2N2Q1p/PPPBBPPP/R3K2R b KQkq - 0 1");
    MoveList moves = generateMoves<ALL_MOVES>(b2);
    // b2.printBoard();
    // cout << "STARTING POSITION\n";
    cout <<  moves.size << "\n";
    for (Move a : moves) {
        cout  << a.getName() << "\n";
        // b2.makeMove(a);
        // b2.printBoard();
        // b2.unmakeMove();
    }

    divided_perft(b2, 1, 0, v);

    // for (int i = 3; i >= 1; i--) {
    //     divided_perft()
    // }
}

/*
*/




// f2f3 | nodes: 178891 -> 178889
// a2a4 | nodes: 217831 -> 217832
// b2b4 | nodes: 216129 -> 216145
// c2c4 | nodes: 240081 -> 240082
// d2d4 | nodes: 361794 -> 361790
// f2f4 | nodes: 198475 -> 198473