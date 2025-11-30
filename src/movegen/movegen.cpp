#include "movegen.h"
#include "../misc/defines.h"
#include "move.h"
#include <cstdint>

vector<int> hor_offset = {1, 2, 2, 1, -1, -2, -2, -1};
vector<int> vert_offset = {2, 1, -1, -2, -2, -1, 1, 2};

//possible files/ranks that knights can be on before moving
vector<uint64_t> allowed_files = {get_files_left[N - 2], get_files_left[N - 3], get_files_left[N - 3], get_files_left[N - 2],
     get_files_right[1], get_files_right[2], get_files_right[2], get_files_right[1]};
vector<uint64_t> allowed_ranks = {get_ranks_below[N - 3], get_ranks_below[N - 2], get_ranks_above[1], get_ranks_above[2],
    get_ranks_above[2], get_ranks_above[1], get_ranks_below[N - 2], get_ranks_below[N - 3]};

constexpr int NORTH = N, SOUTH = -N, WEST = -1, EAST = 1;
constexpr int SOUTH_WEST = -N - 1, SOUTH_EAST = -N + 1, NORTH_EAST = N + 1, NORTH_WEST = N - 1; 

uint64_t attacked_squares;
uint64_t friendly_occ_sq;
uint64_t enemy_occ_sq;
uint64_t all_occ_sq;

uint64_t shift(uint64_t bb, int offset){
    return offset > 0 ? bb << offset : bb >> -offset;
}

vector<Move> generateMoves(Board& b) {
    preCalculations(b);
    vector<Move> moves;

    addPawnMoves(b, moves);
    addKnightMoves(b, moves);
    addSlidingMoves(b, moves);
    addKingMoves(b, moves);

    return moves;
}

void preCalculations(Board& b) {
    friendly_occ_sq = b.getWhiteTurn() ? b.getWhiteOccupiedSquares() : b.getBlackOccupiedSquares();
    enemy_occ_sq = b.getWhiteTurn() ? b.getBlackOccupiedSquares() : b.getWhiteOccupiedSquares();
    all_occ_sq = b.getAllOccupiedSquares();
}

void addPawnMoves(Board& b, vector<Move>& moves) {
    bool whiteTurn = b.getWhiteTurn();
    int forward = whiteTurn ? NORTH : SOUTH;

    uint64_t pawns = b.getPieceBitboard(W_PAWN, whiteTurn);
    uint64_t pr_rank = whiteTurn ? RANK_8 : RANK_1; // promotion rank

    uint64_t push1_pawns = shift(pawns, forward) & ~all_occ_sq;
    uint64_t push2_pawns = shift(push1_pawns & (whiteTurn ? RANK_3 : RANK_6), forward) & ~all_occ_sq;
    uint64_t pr_pawns = push1_pawns & pr_rank;
    
    push1_pawns ^= pr_pawns; // handle promoting pawns seperately from normal pushes
    while(push1_pawns) {
        uint8_t sq = pop_lsb(&push1_pawns);
        moves.push_back(Move(sq - forward, sq, 0, false));
    }
    while(push2_pawns) {
        uint8_t sq = pop_lsb(&push2_pawns);
        moves.push_back(Move(sq - 2 * forward, sq, DOUBLE_PUSH, false));
    }
    while(pr_pawns) {
        uint8_t sq = pop_lsb(&pr_pawns);
        int from_sq = sq - forward;
        moves.push_back(Move(from_sq, sq, PROMOTION_BISHOP, false));
        moves.push_back(Move(from_sq, sq, PROMOTION_KNIGHT, false));
        moves.push_back(Move(from_sq, sq, PROMOTION_ROOK, false));
        moves.push_back(Move(from_sq, sq, PROMOTION_QUEEN, false));
    }

    uint64_t capt_l_pawns = shift(pawns & ~A_FILE, whiteTurn ? NORTH_WEST : SOUTH_WEST) & enemy_occ_sq;
    uint64_t capt_l_pr = capt_l_pawns & pr_rank;
    capt_l_pawns ^= capt_l_pr;

    while(capt_l_pawns) {
        uint8_t sq = pop_lsb(&capt_l_pawns);
        moves.push_back(Move(whiteTurn ? sq - NORTH_WEST : sq - SOUTH_WEST, sq, 0, true));
    }
    while(capt_l_pr) {
        uint8_t sq = pop_lsb(&capt_l_pr);
        int from_sq = whiteTurn ? sq - NORTH_WEST : sq - SOUTH_WEST;
        moves.push_back(Move(from_sq, sq, PROMOTION_BISHOP, true));
        moves.push_back(Move(from_sq, sq, PROMOTION_KNIGHT, true));
        moves.push_back(Move(from_sq, sq, PROMOTION_ROOK, true));
        moves.push_back(Move(from_sq, sq, PROMOTION_QUEEN, true));
    }

    uint64_t capt_r_pawns = shift(pawns & ~F_FILE, whiteTurn ? NORTH_EAST : SOUTH_EAST) & enemy_occ_sq;
    uint64_t capt_r_pr = capt_r_pawns & pr_rank;
    capt_r_pawns ^= capt_r_pr;

    while(capt_r_pawns) {
        uint8_t sq = pop_lsb(&capt_r_pawns);
        moves.push_back(Move(whiteTurn ? sq - NORTH_EAST : sq - SOUTH_EAST, sq, 0, true));
    }
    while(capt_r_pr) {
        uint8_t sq = pop_lsb(&capt_r_pr);
        int from_sq = whiteTurn ? sq - NORTH_EAST : sq - SOUTH_EAST;
        moves.push_back(Move(from_sq, sq, PROMOTION_BISHOP, true));
        moves.push_back(Move(from_sq, sq, PROMOTION_KNIGHT, true));
        moves.push_back(Move(from_sq, sq, PROMOTION_ROOK, true));
        moves.push_back(Move(from_sq, sq, PROMOTION_QUEEN, true));
    }

    // ENPASSANT
    int enp_sq = b.getEnpassantSquare();
    uint64_t can_enp = enp_sq != 0 ? get_file[enp_sq + WEST] : 0;
    can_enp |= enp_sq != N - 1 ? get_file[enp_sq + EAST] : 0;
    can_enp &= (whiteTurn ? RANK_5 : RANK_4) & pawns;
    int target_sq = enp_sq + N * (whiteTurn ? 5 : 2); // set the enpassant square to the correct rank
    // Get the squares adjacent to the enpassant file given by first converting to enpassant square 
    while (can_enp) {
        uint8_t sq = pop_lsb(&can_enp);
        moves.push_back(Move(sq, target_sq, ENPASSANT, true));
    }
}

void addKnightMoves(Board& b, vector<Move>& moves) {
    uint64_t knights = b.getPieceBitboard(W_KNIGHT, b.getWhiteTurn());
    for (int i = 0; i < 8; i++) {
        int offset = hor_offset[i] + vert_offset[i] * N;
        uint64_t valid_knights = knights & allowed_files[i] & allowed_ranks[i];
        uint64_t knight_moves = (offset > 0 ? valid_knights << offset : valid_knights >> -offset) & ~friendly_occ_sq;

        while (knight_moves) {
            uint8_t sq = pop_lsb(&knight_moves);
            moves.push_back(Move(sq - offset, sq, 0 , ((1ULL << sq) & enemy_occ_sq) != 0));
        }
    }
}

void addSlidingMoves(Board& b, vector<Move>& moves) {

}

void addKingMoves(Board& b, vector<Move>& moves) {
    
}