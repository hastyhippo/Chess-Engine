#include <bits/stdc++.h>
#include "./misc/defines.h"
#include "./representation/board.h"
#include "./movegen/movegen.h"
#define DOCTEST_CONFIG_IMPLEMENT
#include "./test/doctest.h"

using namespace std;


int main(int argc, char** argv) {
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


}