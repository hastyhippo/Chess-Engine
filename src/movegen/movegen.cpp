#include "movegen.h"
#include "../misc/defines.h"
#include "move.h"
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
    uint64_t pr_rank = get_rank[whiteTurn ? N - 1 : 0]; // promotion rank

    uint64_t pawns = b.getPieceBitboard(W_PAWN, whiteTurn);
    uint64_t push1_pawns = (whiteTurn ? pawns << N : pawns >> N) & ~all_occ_sq;
    uint64_t push2_pawns = (push1_pawns & get_rank[whiteTurn ? 2 : N - 3]);
    push2_pawns = (whiteTurn ? push2_pawns << N : push2_pawns >> N) & ~all_occ_sq;
    uint64_t pr_pawns = push1_pawns & pr_rank;
    

    int forward = whiteTurn ? 1 : -1;
    push1_pawns ^= pr_pawns; // handle promoting pawns seperately from normal pushes
    while(push1_pawns) {
        uint64_t sq = pop_lsb(&push1_pawns);
        moves.push_back(Move(sq - forward * N, sq, 0, false));
    }
    while(push2_pawns) {
        uint64_t sq = pop_lsb(&push2_pawns);
        moves.push_back(Move(sq - 2 * forward * N, sq, DOUBLE_PUSH, false));
    }
    while(pr_pawns) {
        uint64_t sq = pop_lsb(&pr_pawns);
        int from_sq = sq - forward * N;
        moves.push_back(Move(from_sq, sq, PROMOTION_BISHOP, false));
        moves.push_back(Move(from_sq, sq, PROMOTION_KNIGHT, false));
        moves.push_back(Move(from_sq, sq, PROMOTION_ROOK, false));
        moves.push_back(Move(from_sq, sq, PROMOTION_QUEEN, false));
    }

    uint64_t capt_l_pawns = (whiteTurn ? pawns << (N - 1) : pawns >> (N + 1)) & ~get_file[N - 1] & enemy_occ_sq;
    uint64_t capt_l_pr = capt_l_pawns & pr_rank;
    capt_l_pawns ^= capt_l_pr;

    while(capt_l_pawns) {
        uint64_t sq = pop_lsb(&capt_l_pawns);
        moves.push_back(Move(whiteTurn ? sq - (N - 1) : sq + (N + 1), sq, 0, true));
    }
    while(capt_l_pr) {
        uint64_t sq = pop_lsb(&capt_l_pr);
        int from_sq = whiteTurn ? sq - (N - 1) : sq + (N + 1);
        moves.push_back(Move(from_sq, sq, PROMOTION_BISHOP, true));
        moves.push_back(Move(from_sq, sq, PROMOTION_KNIGHT, true));
        moves.push_back(Move(from_sq, sq, PROMOTION_ROOK, true));
        moves.push_back(Move(from_sq, sq, PROMOTION_QUEEN, true));
    }

    uint64_t capt_r_pawns = (whiteTurn ? pawns << (N + 1) : pawns >> (N - 1)) & ~get_file[0] & enemy_occ_sq;
    uint64_t capt_r_pr = capt_r_pawns & pr_rank;
    capt_r_pawns ^= capt_r_pr;

    while(capt_r_pawns) {
        uint64_t sq = pop_lsb(&capt_r_pawns);
        moves.push_back(Move(whiteTurn ? sq - (N + 1) : sq + (N - 1), sq, 0, true));
    }
    while(capt_r_pr) {
        uint64_t sq = pop_lsb(&capt_r_pr);
        int from_sq = whiteTurn ? sq - (N + 1) : sq + (N - 1);
        moves.push_back(Move(from_sq, sq, PROMOTION_BISHOP, true));
        moves.push_back(Move(from_sq, sq, PROMOTION_KNIGHT, true));
        moves.push_back(Move(from_sq, sq, PROMOTION_ROOK, true));
        moves.push_back(Move(from_sq, sq, PROMOTION_QUEEN, true));
    }
}

void addKnightMoves(Board& b, vector<Move>& moves) {

}

void addSlidingMoves(Board& b, vector<Move>& moves) {

}

void addKingMoves(Board& b, vector<Move>& moves) {
    
}