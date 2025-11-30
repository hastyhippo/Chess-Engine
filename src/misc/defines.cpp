#include "defines.h"

unordered_map<string, uint64_t> name_to_square = {
    {"a1", 1ULL << 0},   {"b1", 1ULL << 1},   {"c1", 1ULL << 2},   {"d1", 1ULL << 3},   {"e1", 1ULL << 4},   {"f1", 1ULL << 5},   {"g1", 1ULL << 6},   {"h1", 1ULL << 7},
    {"a2", 1ULL << 8},   {"b2", 1ULL << 9},   {"c2", 1ULL <<10},   {"d2", 1ULL <<11},   {"e2", 1ULL <<12},   {"f2", 1ULL <<13},   {"g2", 1ULL <<14},   {"h2", 1ULL <<15},
    {"a3", 1ULL <<16},   {"b3", 1ULL <<17},   {"c3", 1ULL <<18},   {"d3", 1ULL <<19},   {"e3", 1ULL <<20},   {"f3", 1ULL <<21},   {"g3", 1ULL <<22},   {"h3", 1ULL <<23},
    {"a4", 1ULL <<24},   {"b4", 1ULL <<25},   {"c4", 1ULL <<26},   {"d4", 1ULL <<27},   {"e4", 1ULL <<28},   {"f4", 1ULL <<29},   {"g4", 1ULL <<30},   {"h4", 1ULL <<31},
    {"a5", 1ULL <<32},   {"b5", 1ULL <<33},   {"c5", 1ULL <<34},   {"d5", 1ULL <<35},   {"e5", 1ULL <<36},   {"f5", 1ULL <<37},   {"g5", 1ULL <<38},   {"h5", 1ULL <<39},
    {"a6", 1ULL <<40},   {"b6", 1ULL <<41},   {"c6", 1ULL <<42},   {"d6", 1ULL <<43},   {"e6", 1ULL <<44},   {"f6", 1ULL <<45},   {"g6", 1ULL <<46},   {"h6", 1ULL <<47},
    {"a7", 1ULL <<48},   {"b7", 1ULL <<49},   {"c7", 1ULL <<50},   {"d7", 1ULL <<51},   {"e7", 1ULL <<52},   {"f7", 1ULL <<53},   {"g7", 1ULL <<54},   {"h7", 1ULL <<55},
    {"a8", 1ULL <<56},   {"b8", 1ULL <<57},   {"c8", 1ULL <<58},   {"d8", 1ULL <<59},   {"e8", 1ULL <<60},   {"f8", 1ULL <<61},   {"g8", 1ULL <<62},   {"h8", 1ULL <<63}
};
unordered_map<uint64_t, string> square_to_name = {
    {1ULL << 0, "a1"},   {1ULL << 1, "b1"},   {1ULL << 2, "c1"},   {1ULL << 3, "d1"},   {1ULL << 4, "e1"},   {1ULL << 5, "f1"},   {1ULL << 6, "g1"},   {1ULL << 7, "h1"},
    {1ULL << 8, "a2"},   {1ULL << 9, "b2"},   {1ULL <<10, "c2"},   {1ULL <<11, "d2"},   {1ULL <<12, "e2"},   {1ULL <<13, "f2"},   {1ULL <<14, "g2"},   {1ULL <<15, "h2"},
    {1ULL <<16, "a3"},   {1ULL <<17, "b3"},   {1ULL <<18, "c3"},   {1ULL <<19, "d3"},   {1ULL <<20, "e3"},   {1ULL <<21, "f3"},   {1ULL <<22, "g3"},   {1ULL <<23, "h3"},
    {1ULL <<24, "a4"},   {1ULL <<25, "b4"},   {1ULL <<26, "c4"},   {1ULL <<27, "d4"},   {1ULL <<28, "e4"},   {1ULL <<29, "f4"},   {1ULL <<30, "g4"},   {1ULL <<31, "h4"},
    {1ULL <<32, "a5"},   {1ULL <<33, "b5"},   {1ULL <<34, "c5"},   {1ULL <<35, "d5"},   {1ULL <<36, "e5"},   {1ULL <<37, "f5"},   {1ULL <<38, "g5"},   {1ULL <<39, "h5"},
    {1ULL <<40, "a6"},   {1ULL <<41, "b6"},   {1ULL <<42, "c6"},   {1ULL <<43, "d6"},   {1ULL <<44, "e6"},   {1ULL <<45, "f6"},   {1ULL <<46, "g6"},   {1ULL <<47, "h6"},
    {1ULL <<48, "a7"},   {1ULL <<49, "b7"},   {1ULL <<50, "c7"},   {1ULL <<51, "d7"},   {1ULL <<52, "e7"},   {1ULL <<53, "f7"},   {1ULL <<54, "g7"},   {1ULL <<55, "h7"},
    {1ULL <<56, "a8"},   {1ULL <<57, "b8"},   {1ULL <<58, "c8"},   {1ULL <<59, "d8"},   {1ULL <<60, "e8"},   {1ULL <<61, "f8"},   {1ULL <<62, "g8"},   {1ULL <<63, "h8"}
};
vector<uint64_t> get_file = {
    0x0101010101010101ULL,
    0x0202020202020202ULL,
    0x0404040404040404ULL,
    0x0808080808080808ULL,
    0x1010101010101010ULL,
    0x2020202020202020ULL,
    0x4040404040404040ULL,
    0x8080808080808080ULL 
};

vector<uint64_t> get_rank = {
    0x00000000000000FFULL,
    0x000000000000FF00ULL,
    0x0000000000FF0000ULL,
    0x00000000FF000000ULL,
    0x000000FF00000000ULL,
    0x0000FF0000000000ULL,
    0x00FF000000000000ULL,
    0xFF00000000000000ULL 
};

int N = 8; // length of chess board 

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
