#include "defines.h"
#include "../movegen/move.h"
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
    {0, "a1"}, {1, "b1"}, {2, "c1"}, {3, "d1"}, {4, "e1"}, {5, "f1"}, {6, "g1"}, {7, "h1"},
    {8, "a2"}, {9, "b2"}, {10, "c2"}, {11, "d2"}, {12, "e2"}, {13, "f2"}, {14, "g2"}, {15, "h2"},
    {16, "a3"}, {17, "b3"}, {18, "c3"}, {19, "d3"}, {20, "e3"}, {21, "f3"}, {22, "g3"}, {23, "h3"},
    {24, "a4"}, {25, "b4"}, {26, "c4"}, {27, "d4"}, {28, "e4"}, {29, "f4"}, {30, "g4"}, {31, "h4"},
    {32, "a5"}, {33, "b5"}, {34, "c5"}, {35, "d5"}, {36, "e5"}, {37, "f5"}, {38, "g5"}, {39, "h5"},
    {40, "a6"}, {41, "b6"}, {42, "c6"}, {43, "d6"}, {44, "e6"}, {45, "f6"}, {46, "g6"}, {47, "h6"},
    {48, "a7"}, {49, "b7"}, {50, "c7"}, {51, "d7"}, {52, "e7"}, {53, "f7"}, {54, "g7"}, {55, "h7"},
    {56, "a8"}, {57, "b8"}, {58, "c8"}, {59, "d8"}, {60, "e8"}, {61, "f8"}, {62, "g8"}, {63, "h8"}
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

uint64_t perft(Board &b, int depth) {
    if (depth == 0) {
        return (uint64_t)1;
    }
    
    uint64_t n_moves = 0;
    vector<Move> move_list = generateMoves(b);

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

// Define the magic bitboard tables (initialized later)
SMagic m_bishop_tbl[64];
SMagic m_rook_tbl[64];
uint64_t knight_moves[64];
uint64_t king_moves[64];

uint64_t directions[8][64];
int distance_to_edge[64][8];
uint64_t bishopAttacks[64][512];
uint64_t rookAttacks[64][4096];
uint64_t rook_masks[64];
uint64_t bishop_masks[64];

void initialiseKnightMoves() {
    //Knights
    vector<int> hor_offset = {1, 2, 2, 1, -1, -2, -2, -1};
    vector<int> vert_offset = {2, 1, -1, -2, -2, -1, 1, 2};
    
    //possible files/ranks that knights can be on before moving
    vector<uint64_t> allowed_files = {get_files_left[N - 2], get_files_left[N - 3], get_files_left[N - 3], get_files_left[N - 2],
            get_files_right[1], get_files_right[2], get_files_right[2], get_files_right[1]};
    vector<uint64_t> allowed_ranks = {get_ranks_below[N - 3], get_ranks_below[N - 2], get_ranks_above[1], get_ranks_above[2],
        get_ranks_above[2], get_ranks_above[1], get_ranks_below[N - 2], get_ranks_below[N - 3]};
    
    for (int i = 0; i < 64; i++) {
        uint64_t attacked_sq = 0ULL;
        for (int i = 0; i < 8; i++) {
            uint64_t new_sq = (1ULL << i) & allowed_files[i] & allowed_ranks[i];
            attacked_sq |= shift(new_sq, hor_offset[i] + vert_offset[i] * N);
        }
        knight_moves[i] = attacked_sq;
    }
}

void setDistanceToEdge() {
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            int sq = x + y * 8;
            distance_to_edge[sq][0] = 7 - y; // NORTH
            distance_to_edge[sq][1] = y;     // SOUTH
            distance_to_edge[sq][2] = 7 - x; // EAST
            distance_to_edge[sq][3] = x;     // WEST
            distance_to_edge[sq][4] = min(7 - y, 7 - x);        // NORTH_EAST
            distance_to_edge[sq][5] = min(7 - y, x);       // NORTH_WEST
            distance_to_edge[sq][6] = min(y, 7 - x);  // SOUTH_EAST
            distance_to_edge[sq][7] = min(y, x);           // SOUTH_WEST
        }
    }
}

void setDirections() {
    for (int index = 0; index < 8; index++) {
        for (int sq = 0; sq < 64; sq++) {
            uint64_t u = 0ULL;
            for (int x = 1; x <= distance_to_edge[sq][index]; x++) {
                u |= 1ULL << (sq + DIR_OFFSETS[index] * x);
            }
            directions[index][sq] = u;
        }
    }
}

uint64_t setOccupancy(int index, uint64_t attackMask) {
    uint64_t occupancy = 0ULL;
    uint64_t mask = attackMask;
    for (int count = 0; count < __popcnt64(attackMask); count++) {
        int sq = pop_lsb(&mask);
        if ((index & (1 << count)) != 0) {
            occupancy |= 1ULL << sq;
        }
    }
    return occupancy;
}

uint64_t getBishopMoves(int sq, uint64_t occupancy) {
    uint64_t moves = 0ULL;
    for (int i = 4; i < 8; i++) {
        for (int x = 1; x <= distance_to_edge[sq][i]; x++) {
            int targetSq = sq + DIR_OFFSETS[i] * x;
            moves |= 1ULL << targetSq;
            if ((occupancy & (1ULL << targetSq)) != 0) break;
        }
    }
    return moves;
}

uint64_t getRookMoves(int sq, uint64_t occupancy) {
    uint64_t moves = 0ULL;
    for (int i = 0; i < 4; i++) {
        for (int x = 1; x <= distance_to_edge[sq][i]; x++) {
            int targetSq = sq + DIR_OFFSETS[i] * x;
            moves |= 1ULL << targetSq;
            if ((occupancy & (1ULL << targetSq)) != 0) break;
        }
    }
    return moves;
}

void initialiseMagics() {
    // First, calculate masks for rooks and bishops
    for (int sq = 0; sq < 64; sq++) {
        for (int i = 0; i < 8; i++) {
            for (int x = 1; x < distance_to_edge[sq][i]; x++) {
                int targetSq = sq + DIR_OFFSETS[i] * x;
                if (i < 4) {
                    rook_masks[sq] |= 1ULL << targetSq;
                } else {
                    bishop_masks[sq] |= 1ULL << targetSq;
                }
            }
        }
    }
    
    // Initialize magic tables
    for (int sq = 0; sq < 64; sq++) {
        // Initialize bishop attacks
        for (int count = 0; count < 1 << __popcnt64(bishop_masks[sq]); count++) {
            uint64_t bishopOccupancy = setOccupancy(count, bishop_masks[sq]);
            uint64_t bishopIndex = (bishopOccupancy * bishop_magics[sq]) >> (64 - bishop_m_bits[sq]);
            bishopAttacks[sq][bishopIndex] = getBishopMoves(sq, bishopOccupancy);
        }

        // Initialize rook attacks
        for (int count = 0; count < 1 << __popcnt64(rook_masks[sq]); count++) {
            uint64_t rookOccupancy = setOccupancy(count, rook_masks[sq]);
            uint64_t rookIndex = (rookOccupancy * rook_magics[sq]) >> (64 - rook_m_bits[sq]);
            rookAttacks[sq][rookIndex] = getRookMoves(sq, rookOccupancy);
        }
        
        // Set up magic table pointers
        m_bishop_tbl[sq] = {bishopAttacks[sq], bishop_masks[sq], bishop_magics[sq], 64 - bishop_m_bits[sq]};
        m_rook_tbl[sq] = {rookAttacks[sq], rook_masks[sq], rook_magics[sq], 64 - rook_m_bits[sq]};
    }
}

void initialiseKingMoves() {
    for (int sq = 0; sq < 64; sq++) {
        uint64_t attacked_sq = 0ULL;
        uint64_t blocked_sq[8] = {RANK_8, RANK_1, H_FILE, A_FILE, RANK_8 | H_FILE, RANK_8 | A_FILE, 
        RANK_1 | H_FILE, RANK_1 | A_FILE};

        for (int i = 0; i < 8; i++) {
            uint64_t valid_sq = (1ULL << sq) & ~blocked_sq[i];
            attacked_sq |= shift(valid_sq, DIR_OFFSETS[i]);
        }
        king_moves[sq] = attacked_sq;
    }
}

void initialiseMoveGeneration() {
    setDistanceToEdge();
    setDirections();
    initialiseMagics();
    initialiseKnightMoves();
    initialiseKingMoves();

    for (int i = 0; i < 64; i++) {
        printBB(king_moves[i]);
    }
}

