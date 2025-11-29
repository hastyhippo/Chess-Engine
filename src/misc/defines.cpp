#include "defines.h"

unordered_map<string, uint64_t> squareToIndex;

vector<string> splitString(const string& str, const char delimiter) {
    vector<string> tokens(0);
    size_t last = 0, next = 0;
    while ((next = str.find(delimiter, last)) != string::npos) {
        tokens.push_back(str.substr(last,next-last));
        last = next + 1;
    }
    tokens.push_back(str.substr(last));
    return tokens;
}


void printBB(uint64_t bb) {
    cout << "Printing: " << bb << "\n";
    for (int i = N_ROWS - 1; i >= 0; i--) {
        for (int j = 0; j < N_COLS; j++) {
            // cout << "\n" << N_COLS * i + j << " is ";
            cout << (((bb & (1uLL << (N_COLS * i + j))) != 0) ? 1 : 0);
        }
        cout << "\n";
    }
}