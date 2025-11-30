#include <bits/stdc++.h>
#include "./initialisation/init.h"
#include "./representation/board.h"
#include "./movegen/movegen.h"
#include "./movegen/move.h"
#include "cassert"
#define DOCTEST_CONFIG_IMPLEMENT
#include "./test/doctest.h"

using namespace std;


int main(int argc, char** argv) {
    Initialise();
    
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
    Board b("b1n5/PP4B1/1P4P1/3Pp3/3P4/P3bpqp/PP3PPP/1K5k w - e6 0 1");
    vector<Move> moves = generateMoves(b);
    cout << "| ";
    for (Move a : moves) {
        cout << a.getName() << " \n";
    }

    Board b2("1k5K/pp3ppp/p3BPQP/3p4/3pP3/1p4p1/pp4b1/B1N5 b - e3 0 1");
    vector<Move> moves2 = generateMoves(b2);
    cout << "| ";
    for (Move a : moves2) {
        cout << a.getName() << " \n";
    }
    assert(moves2.size() == moves.size());
    
}