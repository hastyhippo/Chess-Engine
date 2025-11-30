#include "movegen.h"
#include "../misc/defines.h"
#include "move.h"
#include <cstdint>


constexpr int NORTH = N, SOUTH = -N, WEST = -1, EAST = 1;
constexpr int SOUTH_WEST = -N - 1, SOUTH_EAST = -N + 1, NORTH_EAST = N + 1, NORTH_WEST = N - 1; 

uint64_t attacked_squares;
uint64_t friendly_occ_sq;
uint64_t enemy_occ_sq;
uint64_t all_occ_sq;



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
    bool whiteTurn = b.getWhiteTurn();
    uint64_t knights = b.getPieceBitboard(W_KNIGHT, whiteTurn);
    while(knights) {
        int sq = pop_lsb(&knights);
        uint64_t knight_sq = knight_moves[sq] & ~friendly_occ_sq;
        while(knight_sq) {
            int target_sq = pop_lsb(&knight_sq);
            moves.push_back(Move(sq, target_sq, 0, ((1ULL << target_sq) & enemy_occ_sq) != 0));
        }
    }
}

uint64_t getBishopAttacks(uint64_t occ, int sq) {
    uint64_t* aptr = m_bishop_tbl[sq].ptr;
    occ     &= m_bishop_tbl[sq].mask;
    occ     *= m_bishop_tbl[sq].magic;
    occ     >>= m_bishop_tbl[sq].shift;
    return aptr[occ];
}

uint64_t getRookAttacks(uint64_t occ, int sq) {
    uint64_t* aptr = m_rook_tbl[sq].ptr;
    occ     &= m_rook_tbl[sq].mask;
    occ     *= m_rook_tbl[sq].magic;
    occ     >>= m_rook_tbl[sq].shift;
    return aptr[occ];
}

void addSlidingMoves(Board& b, vector<Move>& moves) {
    bool whiteTurn = b.getWhiteTurn();
    uint64_t bishops = b.getPieceBitboard(W_BISHOP, whiteTurn);
    uint64_t rooks = b.getPieceBitboard(W_ROOK, whiteTurn);
    uint64_t queens = b.getPieceBitboard(W_QUEEN, whiteTurn);

    while(bishops) {
        int from_sq = pop_lsb(&bishops);
        uint64_t attacked_sq = getBishopAttacks(all_occ_sq, from_sq);
        while(attacked_sq) {
            int to_sq = pop_lsb(&attacked_sq);
            moves.push_back(Move(from_sq, to_sq, 0, (1ULL << to_sq) & enemy_occ_sq));
        }
    }

    while(rooks) {
        int from_sq = pop_lsb(&rooks);
        uint64_t attacked_sq = getRookAttacks(all_occ_sq, from_sq);
        while(attacked_sq) {
            int to_sq = pop_lsb(&attacked_sq);
            moves.push_back(Move(from_sq, to_sq, 0, (1ULL << to_sq) & enemy_occ_sq));
        }
    }

    while(queens) {
        int from_sq = pop_lsb(&queens);
        uint64_t attacked_sq = getRookAttacks(all_occ_sq, from_sq) 
                            | getBishopAttacks(all_occ_sq, from_sq);
        while(attacked_sq) {
            int to_sq = pop_lsb(&attacked_sq);
            moves.push_back(Move(from_sq, to_sq, 0, (1ULL << to_sq) & enemy_occ_sq));
        }
    }
}





void addKingMoves(Board& b, vector<Move>& moves) {
    
}