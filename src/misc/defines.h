#pragma once

#include <bits/stdc++.h>
#include "../movegen/move.h";
using namespace std;

#define N_ROWS 8
#define N_COLS 8
#define N_SQUARES 64
#define N_PIECE_TYPES 6

#define KINGSIDE 0
#define QUEENSIDE 1

class Board;
class Move;
// using the Fancy approach: https://www.chessprogramming.org/Magic_Bitboards
struct SMagic {
    uint64_t* ptr;
    uint64_t mask;
    uint64_t magic;
    int shift;
};



enum PieceType {W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING, 
                B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING};

enum GenType {CAPTURES, EVASIONS, QUIET, ALL_MOVES, PINNED};

vector<string>splitString(const string& input, const char delimiter);
void printBB(uint64_t bb);

extern unordered_map<string, uint64_t> name_to_bit;
extern unordered_map<uint64_t, string> bit_to_name;
extern unordered_map<uint8_t, string> sq_to_name;


constexpr uint64_t get_file[8] = {
    0x0101010101010101ULL,
    0x0202020202020202ULL,
    0x0404040404040404ULL,
    0x0808080808080808ULL,
    0x1010101010101010ULL,
    0x2020202020202020ULL,
    0x4040404040404040ULL,
    0x8080808080808080ULL 
};
constexpr uint64_t get_rank[8] = {
    0x00000000000000FFULL,
    0x000000000000FF00ULL,
    0x0000000000FF0000ULL,
    0x00000000FF000000ULL,
    0x000000FF00000000ULL,
    0x0000FF0000000000ULL,
    0x00FF000000000000ULL,
    0xFF00000000000000ULL 
};
constexpr uint64_t get_files_left[8] = {
    0x0101010101010101ULL,
    0x0303030303030303ULL,
    0x0707070707070707ULL,
    0x0F0F0F0F0F0F0F0FULL,
    0x1F1F1F1F1F1F1F1FULL,
    0x3F3F3F3F3F3F3F3FULL,
    0x7F7F7F7F7F7F7F7FULL,
    0xFFFFFFFFFFFFFFFFULL
};
constexpr uint64_t get_files_right[8] = {
    0xFFFFFFFFFFFFFFFFULL,
    0xFEFEFEFEFEFEFEFEULL,
    0xFCFCFCFCFCFCFCFCULL,
    0xF8F8F8F8F8F8F8F8ULL,
    0xF0F0F0F0F0F0F0F0ULL,
    0xE0E0E0E0E0E0E0E0ULL,
    0xC0C0C0C0C0C0C0C0ULL,
    0x8080808080808080ULL
};
constexpr uint64_t get_ranks_above[8] = {
    0xFFFFFFFFFFFFFFFFULL,
    0xFFFFFFFFFFFFFF00ULL,
    0xFFFFFFFFFFFF0000ULL,
    0xFFFFFFFFFF000000ULL,
    0xFFFFFFFF00000000ULL,
    0xFFFFFF0000000000ULL,
    0xFFFF000000000000ULL,
    0xFF00000000000000ULL
};
constexpr uint64_t get_ranks_below[8] = {
    0x00000000000000FFULL,
    0x000000000000FFFFULL,
    0x0000000000FFFFFFULL,
    0x00000000FFFFFFFFULL,
    0x000000FFFFFFFFFFULL,
    0x0000FFFFFFFFFFFFULL,
    0x00FFFFFFFFFFFFFFULL,
    0xFFFFFFFFFFFFFFFFULL
};

constexpr uint64_t rook_magics[64] = {
    0xa8002c000108020UL, 0x6c00049b0002001UL, 0x100200010090040UL, 0x2480041000800801UL, 0x280028004000800UL, 0x900410008040022UL, 0x280020001001080UL, 0x2880002041000080UL,
    0xa000800080400034UL, 0x4808020004000UL, 0x2290802004801000UL, 0x411000d00100020UL, 0x402800800040080UL, 0xb000401004208UL, 0x2409000100040200UL, 0x1002100004082UL,
    0x22878001e24000UL, 0x1090810021004010UL, 0x801030040200012UL, 0x500808008001000UL, 0xa08018014000880UL, 0x8000808004000200UL, 0x201008080010200UL, 0x801020000441091UL,
    0x800080204005UL, 0x1040200040100048UL, 0x120200402082UL, 0xd14880480100080UL, 0x12040280080080UL, 0x100040080020080UL, 0x9020010080800200UL, 0x813241200148449UL,
    0x491604001800080UL, 0x100401000402001UL, 0x4820010021001040UL, 0x400402202000812UL, 0x209009005000802UL, 0x810800601800400UL, 0x4301083214000150UL, 0x204026458e001401UL,
    0x40204000808000UL, 0x8001008040010020UL, 0x8410820820420010UL, 0x1003001000090020UL, 0x804040008008080UL, 0x12000810020004UL, 0x1000100200040208UL, 0x430000a044020001UL,
    0x280009023410300UL, 0xe0100040002240UL, 0x200100401700UL, 0x2244100408008080UL, 0x8000400801980UL, 0x2000810040200UL, 0x8010100228810400UL, 0x2000009044210200UL,
    0x4080008040102101UL, 0x40002080411d01UL, 0x2005524060000901UL, 0x502001008400422UL, 0x489a000810200402UL, 0x1004400080a13UL, 0x4000011008020084UL, 0x26002114058042UL
};
constexpr uint64_t bishop_magics[64] = {
    0x89a1121896040240UL,0x2004844802002010UL,0x2068080051921000UL,0x62880a0220200808UL,0x4042004000000UL,0x100822020200011UL,0xc00444222012000aUL,0x28808801216001UL,
    0x400492088408100UL,0x201c401040c0084UL,0x840800910a0010UL,0x82080240060UL,0x2000840504006000UL,0x30010c4108405004UL,0x1008005410080802UL,0x8144042209100900UL,
    0x208081020014400UL,0x4800201208ca00UL,0xf18140408012008UL,0x1004002802102001UL,0x841000820080811UL,0x40200200a42008UL,0x800054042000UL,0x88010400410c9000UL,
    0x520040470104290UL,0x1004040051500081UL,0x2002081833080021UL,0x400c00c010142UL,0x941408200c002000UL,0x658810000806011UL,0x188071040440a00UL,0x4800404002011c00UL,
    0x104442040404200UL,0x511080202091021UL,0x4022401120400UL,0x80c0040400080120UL,0x8040010040820802UL,0x480810700020090UL,0x102008e00040242UL,0x809005202050100UL,
    0x8002024220104080UL,0x431008804142000UL,0x19001802081400UL,0x200014208040080UL,0x3308082008200100UL,0x41010500040c020UL,0x4012020c04210308UL,0x208220a202004080UL,
    0x111040120082000UL,0x6803040141280a00UL,0x2101004202410000UL,0x8200000041108022UL,0x21082088000UL,0x2410204010040UL,0x40100400809000UL,0x822088220820214UL,
    0x40808090012004UL,0x910224040218c9UL,0x402814422015008UL,0x90014004842410UL,0x1000042304105UL,0x10008830412a00UL,0x2520081090008908UL,0x40102000a0a60140UL,
};

constexpr int bishop_m_bits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6
};
constexpr int rook_m_bits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12
};
constexpr uint8_t castling_mask[64] = {
    0b0010, 0b0000, 0b0000, 0b0000, 0b0011, 0b0000, 0b0000, 0b0001,  // a1 loses Q, e1 loses KQ, h1 loses K
    0b0000, 0b0000, 0b0000, 0b0000, 0b0000, 0b0000, 0b0000, 0b0000,
    0b0000, 0b0000, 0b0000, 0b0000, 0b0000, 0b0000, 0b0000, 0b0000,
    0b0000, 0b0000, 0b0000, 0b0000, 0b0000, 0b0000, 0b0000, 0b0000,
    0b0000, 0b0000, 0b0000, 0b0000, 0b0000, 0b0000, 0b0000, 0b0000,
    0b0000, 0b0000, 0b0000, 0b0000, 0b0000, 0b0000, 0b0000, 0b0000,
    0b0000, 0b0000, 0b0000, 0b0000, 0b0000, 0b0000, 0b0000, 0b0000,
    0b1000, 0b0000, 0b0000, 0b0000, 0b1100, 0b0000, 0b0000, 0b0100   // a8 loses q, e8 loses kq, h8 loses k
};

constexpr uint8_t castling_rights_bits[2] = {
    0b1100, //white
    0b0011 //black
};

// Move generation
extern SMagic m_rook_tbl[64];
extern SMagic m_bishop_tbl[64];
extern uint64_t knight_moves[64];
extern uint64_t king_moves[64];
extern uint64_t pawn_moves[2][64];
extern uint64_t pawn_attacks[2][64];

extern uint64_t directions[8][64];
extern int distance_to_edge[64][8];
extern uint64_t bishopAttacks[64][512];
extern uint64_t rookAttacks[64][4096];
extern uint64_t rook_masks[64];
extern uint64_t bishop_masks[64];
extern uint64_t ray_between_table[64][64];
extern uint64_t ray_through[64][64];

constexpr uint8_t EMPTY_SQ = 15; // Empty square is 0 in pieces_arr
constexpr uint8_t PIECE_OFFSET = 1; // Pieces in pieces_arr start at 1 (enum values + 1)
constexpr uint8_t NO_ENP = 8;


constexpr int MAX_MOVES_GEN = 256;

struct MoveList {
    Move moves[MAX_MOVES_GEN];
    int size = 0;

    void add(Move m) {
        moves[size++] = m;
    }
    Move* begin() { return moves; }
    Move *end() { return moves + size; }
    Move& operator[](int i) { return moves[i]; }
};

constexpr int N = 8;
constexpr int NORTH = N, SOUTH = -N, WEST = -1, EAST = 1;
constexpr int SOUTH_WEST = -N - 1, SOUTH_EAST = -N + 1, NORTH_EAST = N + 1, NORTH_WEST = N - 1; 
constexpr int DIR_OFFSETS[8] = {NORTH, SOUTH, EAST, WEST, NORTH_EAST, NORTH_WEST, SOUTH_EAST, SOUTH_WEST};

constexpr uint64_t A_FILE = 0x0101010101010101ULL;
constexpr uint64_t B_FILE = 0x0202020202020202ULL;
constexpr uint64_t C_FILE = 0x0404040404040404ULL;
constexpr uint64_t D_FILE = 0x0808080808080808ULL;
constexpr uint64_t E_FILE = 0x1010101010101010ULL;
constexpr uint64_t F_FILE = 0x2020202020202020ULL;
constexpr uint64_t G_FILE = 0x4040404040404040ULL;
constexpr uint64_t H_FILE = 0x8080808080808080ULL;

constexpr uint64_t A_1 = 1ULL << 0;  constexpr uint64_t B_1 = 1ULL << 1;  constexpr uint64_t C_1 = 1ULL << 2;  constexpr uint64_t D_1 = 1ULL << 3;  constexpr uint64_t E_1 = 1ULL << 4;  constexpr uint64_t F_1 = 1ULL << 5;  constexpr uint64_t G_1 = 1ULL << 6;  constexpr uint64_t H_1 = 1ULL << 7;
constexpr uint64_t A_2 = 1ULL << 8;  constexpr uint64_t B_2 = 1ULL << 9;  constexpr uint64_t C_2 = 1ULL << 10; constexpr uint64_t D_2 = 1ULL << 11; constexpr uint64_t E_2 = 1ULL << 12; constexpr uint64_t F_2 = 1ULL << 13; constexpr uint64_t G_2 = 1ULL << 14; constexpr uint64_t H_2 = 1ULL << 15;
constexpr uint64_t A_3 = 1ULL << 16; constexpr uint64_t B_3 = 1ULL << 17; constexpr uint64_t C_3 = 1ULL << 18; constexpr uint64_t D_3 = 1ULL << 19; constexpr uint64_t E_3 = 1ULL << 20; constexpr uint64_t F_3 = 1ULL << 21; constexpr uint64_t G_3 = 1ULL << 22; constexpr uint64_t H_3 = 1ULL << 23;
constexpr uint64_t A_4 = 1ULL << 24; constexpr uint64_t B_4 = 1ULL << 25; constexpr uint64_t C_4 = 1ULL << 26; constexpr uint64_t D_4 = 1ULL << 27; constexpr uint64_t E_4 = 1ULL << 28; constexpr uint64_t F_4 = 1ULL << 29; constexpr uint64_t G_4 = 1ULL << 30; constexpr uint64_t H_4 = 1ULL << 31;
constexpr uint64_t A_5 = 1ULL << 32; constexpr uint64_t B_5 = 1ULL << 33; constexpr uint64_t C_5 = 1ULL << 34; constexpr uint64_t D_5 = 1ULL << 35; constexpr uint64_t E_5 = 1ULL << 36; constexpr uint64_t F_5 = 1ULL << 37; constexpr uint64_t G_5 = 1ULL << 38; constexpr uint64_t H_5 = 1ULL << 39;
constexpr uint64_t A_6 = 1ULL << 40; constexpr uint64_t B_6 = 1ULL << 41; constexpr uint64_t C_6 = 1ULL << 42; constexpr uint64_t D_6 = 1ULL << 43; constexpr uint64_t E_6 = 1ULL << 44; constexpr uint64_t F_6 = 1ULL << 45; constexpr uint64_t G_6 = 1ULL << 46; constexpr uint64_t H_6 = 1ULL << 47;
constexpr uint64_t A_7 = 1ULL << 48; constexpr uint64_t B_7 = 1ULL << 49; constexpr uint64_t C_7 = 1ULL << 50; constexpr uint64_t D_7 = 1ULL << 51; constexpr uint64_t E_7 = 1ULL << 52; constexpr uint64_t F_7 = 1ULL << 53; constexpr uint64_t G_7 = 1ULL << 54; constexpr uint64_t H_7 = 1ULL << 55;
constexpr uint64_t A_8 = 1ULL << 56; constexpr uint64_t B_8 = 1ULL << 57; constexpr uint64_t C_8 = 1ULL << 58; constexpr uint64_t D_8 = 1ULL << 59; constexpr uint64_t E_8 = 1ULL << 60; constexpr uint64_t F_8 = 1ULL << 61; constexpr uint64_t G_8 = 1ULL << 62; constexpr uint64_t H_8 = 1ULL << 63;

constexpr uint64_t RANK_1 = 0x00000000000000FFULL;
constexpr uint64_t RANK_2 = 0x000000000000FF00ULL;
constexpr uint64_t RANK_3 = 0x0000000000FF0000ULL;
constexpr uint64_t RANK_4 = 0x00000000FF000000ULL;
constexpr uint64_t RANK_5 = 0x000000FF00000000ULL;
constexpr uint64_t RANK_6 = 0x0000FF0000000000ULL;
constexpr uint64_t RANK_7 = 0x00FF000000000000ULL;
constexpr uint64_t RANK_8 = 0xFF00000000000000ULL;

constexpr uint64_t castling_rook_moves[2][2] = {
    {H_1 | F_1, A_1 | D_1},
    {H_8 | F_8, A_8 | D_8}
};

constexpr uint64_t castling_clear_sq[2][2] = {
    {G_1 | F_1, B_1 | C_1 | D_1},
    {G_8 | F_8, B_8 | C_8 | D_8}
};

constexpr uint64_t castling_unattacked_sq[2][2] = {
    {G_1 | F_1,  C_1 | D_1},
    {G_8 | F_8,  C_8 | D_8}
};

constexpr uint64_t castling_target_sq[2][2] = {
    {6, 2},
    {62, 58}
};

uint64_t divided_perft(Board &b, int depth, int print_depth, vector<string>& moves);
uint64_t divided_perft(Board &b, int depth, vector<string>& moves);

uint64_t perft(Board &b, int depth);
uint64_t shift(uint64_t bb, int offset);

inline uint8_t pop_lsb(uint64_t* b) {
    uint8_t sq = _tzcnt_u64(*b);
    *b &= *b - 1;
    return sq;
}