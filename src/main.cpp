#include <bits/stdc++.h>
#include "./initialisation/init.h"
#include "./representation/board.h"
#include "./movegen/movegen.h"
#include "cassert"
#include "misc/defines.h"
#define DOCTEST_CONFIG_IMPLEMENT
#include "./test/doctest.h"
#include "./uci/uci.h"

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
            // context.applyCommandLine(argc, argv);
            context.setOption("test-suite", "movegen speed");
            int res = context.run();
            if (context.shouldExit()) {
                return res;
            }
            return 0;
        } else if (string(argv[i]) == "-s") {
            doctest::Context context;
            context.setOption("no-output-redirect", true);
            context.setOption("test-suite", "search correctness");
            int res = context.run();
            if (context.shouldExit()) return res;
            return 0;
        } else if (string(argv[i]) == "-c") {
            doctest::Context context;
            context.setOption("no-output-redirect", true);
            context.setOption("test-suite", "perft correctness");
            int res = context.run();
            if (context.shouldExit()) {
                return res;
            }
            return 0;
        } else if (string(argv[i]) == "-p") {
            string line;
            while (getline(cin, line)) {
                if (line.empty()) continue;
                istringstream ss(line);
                string fen, token;
                int depth = 5;
                // last token is depth if it's a plain integer
                vector<string> tokens;
                while (ss >> token) tokens.push_back(token);
                if (!tokens.empty()) {
                    try {
                        depth = stoi(tokens.back());
                        tokens.pop_back();
                    } catch (...) {}
                }
                for (size_t j = 0; j < tokens.size(); j++)
                    fen += (j ? " " : "") + tokens[j];
                Board b(fen);
                b.printBoard();
                Move best = get_best_move(b, depth, 0);
                cout << "bestmove " << best.getName() << "\n";
            }
            return 0;
        }
    }

    UCI uci;
    uci.loop();

    // while(true) {
    //     string s;
    //     getline(cin, s);
    //     Board b(s);
    //     int depth;
    //     cin >> depth;
    //     std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    //     vector<string> v;
    //     uint64_t total = divided_perft(b, depth, 1, v);
    //     cout << "\nNodes searched: " << total << "\n";
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