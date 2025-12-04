#include "defines.h"
#include "../movegen/movegen.h"

unordered_map<string, uint64_t> name_to_bit = {
    {"a1", 1ULL << 0},  {"b1", 1ULL << 1},  {"c1", 1ULL << 2},  {"d1", 1ULL << 3},  {"e1", 1ULL << 4},  {"f1", 1ULL << 5},  {"g1", 1ULL << 6},  {"h1", 1ULL << 7},
    {"a2", 1ULL << 8},  {"b2", 1ULL << 9},  {"c2", 1ULL <<10},  {"d2", 1ULL <<11},  {"e2", 1ULL <<12},  {"f2", 1ULL <<13},  {"g2", 1ULL <<14},  {"h2", 1ULL <<15},
    {"a3", 1ULL <<16},  {"b3", 1ULL <<17},  {"c3", 1ULL <<18},  {"d3", 1ULL <<19},  {"e3", 1ULL <<20},  {"f3", 1ULL <<21},  {"g3", 1ULL <<22},  {"h3", 1ULL <<23},
    {"a4", 1ULL <<24},  {"b4", 1ULL <<25},  {"c4", 1ULL <<26},  {"d4", 1ULL <<27},  {"e4", 1ULL <<28},  {"f4", 1ULL <<29},  {"g4", 1ULL <<30},  {"h4", 1ULL <<31},
    {"a5", 1ULL <<32},  {"b5", 1ULL <<33},  {"c5", 1ULL <<34},  {"d5", 1ULL <<35},  {"e5", 1ULL <<36},  {"f5", 1ULL <<37},  {"g5", 1ULL <<38},  {"h5", 1ULL <<39},
    {"a6", 1ULL <<40},  {"b6", 1ULL <<41},  {"c6", 1ULL <<42},  {"d6", 1ULL <<43},  {"e6", 1ULL <<44},  {"f6", 1ULL <<45},  {"g6", 1ULL <<46},  {"h6", 1ULL <<47},
    {"a7", 1ULL <<48},  {"b7", 1ULL <<49},  {"c7", 1ULL <<50},  {"d7", 1ULL <<51},  {"e7", 1ULL <<52},  {"f7", 1ULL <<53},  {"g7", 1ULL <<54},  {"h7", 1ULL <<55},
    {"a8", 1ULL <<56},  {"b8", 1ULL <<57},  {"c8", 1ULL <<58},  {"d8", 1ULL <<59},  {"e8", 1ULL <<60},  {"f8", 1ULL <<61},  {"g8", 1ULL <<62},  {"h8", 1ULL <<63}
};
unordered_map<uint64_t, string> bit_to_name = {
    {1ULL << 0, "a1"}, {1ULL << 1, "b1"}, {1ULL << 2, "c1"}, {1ULL << 3, "d1"}, {1ULL << 4, "e1"}, {1ULL << 5, "f1"}, {1ULL << 6, "g1"}, {1ULL << 7, "h1"},
    {1ULL << 8, "a2"}, {1ULL << 9, "b2"}, {1ULL <<10, "c2"}, {1ULL <<11, "d2"}, {1ULL <<12, "e2"}, {1ULL <<13, "f2"}, {1ULL <<14, "g2"}, {1ULL <<15, "h2"},
    {1ULL <<16, "a3"}, {1ULL <<17, "b3"}, {1ULL <<18, "c3"}, {1ULL <<19, "d3"}, {1ULL <<20, "e3"}, {1ULL <<21, "f3"}, {1ULL <<22, "g3"}, {1ULL <<23, "h3"},
    {1ULL <<24, "a4"}, {1ULL <<25, "b4"}, {1ULL <<26, "c4"}, {1ULL <<27, "d4"}, {1ULL <<28, "e4"}, {1ULL <<29, "f4"}, {1ULL <<30, "g4"}, {1ULL <<31, "h4"},
    {1ULL <<32, "a5"}, {1ULL <<33, "b5"}, {1ULL <<34, "c5"}, {1ULL <<35, "d5"}, {1ULL <<36, "e5"}, {1ULL <<37, "f5"}, {1ULL <<38, "g5"}, {1ULL <<39, "h5"},
    {1ULL <<40, "a6"}, {1ULL <<41, "b6"}, {1ULL <<42, "c6"}, {1ULL <<43, "d6"}, {1ULL <<44, "e6"}, {1ULL <<45, "f6"}, {1ULL <<46, "g6"}, {1ULL <<47, "h6"},
    {1ULL <<48, "a7"}, {1ULL <<49, "b7"}, {1ULL <<50, "c7"}, {1ULL <<51, "d7"}, {1ULL <<52, "e7"}, {1ULL <<53, "f7"}, {1ULL <<54, "g7"}, {1ULL <<55, "h7"},
    {1ULL <<56, "a8"}, {1ULL <<57, "b8"}, {1ULL <<58, "c8"}, {1ULL <<59, "d8"}, {1ULL <<60, "e8"}, {1ULL <<61, "f8"}, {1ULL <<62, "g8"}, {1ULL <<63, "h8"}
};
unordered_map<uint8_t, string> sq_to_name = {
    {static_cast<uint8_t>(0),  "a1"}, {static_cast<uint8_t>(1),  "b1"}, {static_cast<uint8_t>(2),  "c1"}, {static_cast<uint8_t>(3),  "d1"},
    {static_cast<uint8_t>(4),  "e1"}, {static_cast<uint8_t>(5),  "f1"}, {static_cast<uint8_t>(6),  "g1"}, {static_cast<uint8_t>(7),  "h1"},
    {static_cast<uint8_t>(8),  "a2"}, {static_cast<uint8_t>(9),  "b2"}, {static_cast<uint8_t>(10), "c2"}, {static_cast<uint8_t>(11), "d2"},
    {static_cast<uint8_t>(12), "e2"}, {static_cast<uint8_t>(13), "f2"}, {static_cast<uint8_t>(14), "g2"}, {static_cast<uint8_t>(15), "h2"},
    {static_cast<uint8_t>(16), "a3"}, {static_cast<uint8_t>(17), "b3"}, {static_cast<uint8_t>(18), "c3"}, {static_cast<uint8_t>(19), "d3"},
    {static_cast<uint8_t>(20), "e3"}, {static_cast<uint8_t>(21), "f3"}, {static_cast<uint8_t>(22), "g3"}, {static_cast<uint8_t>(23), "h3"},
    {static_cast<uint8_t>(24), "a4"}, {static_cast<uint8_t>(25), "b4"}, {static_cast<uint8_t>(26), "c4"}, {static_cast<uint8_t>(27), "d4"},
    {static_cast<uint8_t>(28), "e4"}, {static_cast<uint8_t>(29), "f4"}, {static_cast<uint8_t>(30), "g4"}, {static_cast<uint8_t>(31), "h4"},
    {static_cast<uint8_t>(32), "a5"}, {static_cast<uint8_t>(33), "b5"}, {static_cast<uint8_t>(34), "c5"}, {static_cast<uint8_t>(35), "d5"},
    {static_cast<uint8_t>(36), "e5"}, {static_cast<uint8_t>(37), "f5"}, {static_cast<uint8_t>(38), "g5"}, {static_cast<uint8_t>(39), "h5"},
    {static_cast<uint8_t>(40), "a6"}, {static_cast<uint8_t>(41), "b6"}, {static_cast<uint8_t>(42), "c6"}, {static_cast<uint8_t>(43), "d6"},
    {static_cast<uint8_t>(44), "e6"}, {static_cast<uint8_t>(45), "f6"}, {static_cast<uint8_t>(46), "g6"}, {static_cast<uint8_t>(47), "h6"},
    {static_cast<uint8_t>(48), "a7"}, {static_cast<uint8_t>(49), "b7"}, {static_cast<uint8_t>(50), "c7"}, {static_cast<uint8_t>(51), "d7"},
    {static_cast<uint8_t>(52), "e7"}, {static_cast<uint8_t>(53), "f7"}, {static_cast<uint8_t>(54), "g7"}, {static_cast<uint8_t>(55), "h7"},
    {static_cast<uint8_t>(56), "a8"}, {static_cast<uint8_t>(57), "b8"}, {static_cast<uint8_t>(58), "c8"}, {static_cast<uint8_t>(59), "d8"},
    {static_cast<uint8_t>(60), "e8"}, {static_cast<uint8_t>(61), "f8"}, {static_cast<uint8_t>(62), "g8"}, {static_cast<uint8_t>(63), "h8"}
};

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

uint64_t divided_perft(Board &b, int depth, int print_depth, vector<string>& moves) {
    if (depth == 0) {
        return (uint64_t)1;
    }
    uint64_t n_moves = 0;
    MoveList move_list = generateMoves<ALL_MOVES>(b);

    for (Move m : move_list) {
        b.makeMove(m);
        moves.push_back(m.getName());
        n_moves += divided_perft(b, depth - 1, print_depth - 1, moves);
        moves.pop_back();
        b.unmakeMove();
    }
    if(print_depth == 0) {
        string moves_list = "";
        for (auto a : moves) {
            moves_list += a;
        }   
        cout << moves_list << " | nodes: " << n_moves << "\n";
    }
    return n_moves;
}

uint64_t divided_perft(Board &b, int depth, vector<string>& moves) {
    if (depth == 0) {    
        cout << moves[moves.size()-1] << " ";
        return (uint64_t)1;
    }
    uint64_t n_moves = 0;
    MoveList move_list = generateMoves<ALL_MOVES>(b);

    for (Move m : move_list) {
        b.makeMove(m);
        moves.push_back(m.getName());
        n_moves += divided_perft(b, depth - 1, moves);
        moves.pop_back();
        b.unmakeMove();
    }

    string moves_list = "";
    for (auto a : moves) {
        moves_list += a;
    }  
    for (int i = 0; i < moves.size(); i++) {
        cout << "  ";
    } 
    cout << "\n" << moves_list << " | nodes: " << n_moves << "\n";

    return n_moves;
}

uint64_t perft(Board &b, int depth) {
    if (depth == 0) {
        return (uint64_t)1;
    }
    
    uint64_t n_moves = 0;
    MoveList move_list = generateMoves<ALL_MOVES>(b);

    for (Move m : move_list) {
        b.makeMove(m);
        n_moves += perft(b, depth - 1);
        b.unmakeMove();
    }
    return n_moves;
}

uint64_t shift(uint64_t bb, int offset){
    return offset > 0 ? bb << offset : bb >> -offset;
}

// Define the magic bitboard tables (initialized in initialisation/init.cpp)
SMagic m_bishop_tbl[64];
SMagic m_rook_tbl[64];
uint64_t knight_moves[64];
uint64_t king_moves[64];
uint64_t pawn_moves[2][64];
uint64_t pawn_attacks[2][64];

uint64_t directions[8][64];
int distance_to_edge[64][8];
uint64_t bishopAttacks[64][512];
uint64_t rookAttacks[64][4096];
uint64_t rook_masks[64];
uint64_t bishop_masks[64];
uint64_t ray_between_table[64][64];
uint64_t ray_through[64][64];

