#pragma once

#include <bits/stdc++.h>

using namespace std;

#define N_ROWS 8
#define N_COLS 8
#define N_SQUARES 64
#define N_PIECE_TYPES 6

#define COL_A 

class Board;
// using the Fancy approach: https://www.chessprogramming.org/Magic_Bitboards
struct SMagic {
    uint64_t* ptr;
    uint64_t mask;
    uint64_t magic;
    int shift;
};

enum PieceType {W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING, 
                B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING};

vector<string>splitString(const string& input, const char delimiter);
void printBB(uint64_t bb);

extern unordered_map<string, uint64_t> name_to_bit;
extern unordered_map<uint64_t, string> bit_to_name;
extern unordered_map<uint8_t, string> sq_to_name;

extern vector<uint64_t> get_file;
extern vector<uint64_t> get_rank;

extern vector<uint64_t> get_files_left;
extern vector<uint64_t> get_files_right;
extern vector<uint64_t> get_ranks_above;
extern vector<uint64_t> get_ranks_below;


// Move generation
extern SMagic m_rook_tbl[64];
extern SMagic m_bishop_tbl[64];
extern uint64_t knight_moves[64];

constexpr int N = 8;

constexpr uint64_t A_FILE = 0x0101010101010101ULL;
constexpr uint64_t B_FILE = 0x0202020202020202ULL;
constexpr uint64_t C_FILE = 0x0404040404040404ULL;
constexpr uint64_t D_FILE = 0x0808080808080808ULL;
constexpr uint64_t E_FILE = 0x1010101010101010ULL;
constexpr uint64_t F_FILE = 0x2020202020202020ULL;
constexpr uint64_t G_FILE = 0x4040404040404040ULL;
constexpr uint64_t H_FILE = 0x8080808080808080ULL;

constexpr uint64_t RANK_1 = 0x00000000000000FFULL;
constexpr uint64_t RANK_2 = 0x000000000000FF00ULL;
constexpr uint64_t RANK_3 = 0x0000000000FF0000ULL;
constexpr uint64_t RANK_4 = 0x00000000FF000000ULL;
constexpr uint64_t RANK_5 = 0x000000FF00000000ULL;
constexpr uint64_t RANK_6 = 0x0000FF0000000000ULL;
constexpr uint64_t RANK_7 = 0x00FF000000000000ULL;
constexpr uint64_t RANK_8 = 0xFF00000000000000ULL;

uint64_t Perft(Board &b, int depth);
uint64_t shift(uint64_t bb, int offset);
void InitialiseMoveGeneration();

inline uint64_t pop_lsb(uint64_t* b) {
    uint64_t sq = _tzcnt_u64(*b);
    *b &= *b - 1;
    return sq;
}