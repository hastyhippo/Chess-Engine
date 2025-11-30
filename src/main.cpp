#include <bits/stdc++.h>
#include "./misc/defines.h"
#include "./initialisation/init.h"
#include "./representation/board.h"
#include "./movegen/movegen.h"
#include "./movegen/move.h"

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
    Board b("rrn5/PP4B1/1P4P1/3Pp3/3P4/P3bpqp/PP3PPP/1K5k w - e6 0 1");
    vector<Move> moves = generateMoves(b);
    moves.push_back(Move(1<<3, 1<< 4,0));
    cout << "| ";
    for (Move a : moves) {
        cout << a.getName() << " | ";
    }
}