#include "movegen.h"
#include "../misc/defines.h"
uint64_t attacked_squares;
uint64_t friendly_occupied_squares;
uint64_t enemy_occupied_squares;
uint64_t all_occupied_squares;

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
    friendly_occupied_squares = b.getWhiteOccupiedSquares();
    enemy_occupied_squares = b.getBlackOccupiedSquares();
    all_occupied_squares = b.getAllOccupiedSquares();
}

void addPawnMoves(Board& b, vector<Move>& moves) {
    uint64_t pawns = b.getPieceBitboard(W_PAWN, b.getWhiteTurn());

    uint64_t doublepush_pawns = pawns & get_rank[b.getWhiteTurn() ? 1 : N - 2];
    // uint64_t promotion_pawns = pawns & get_rank[b.getWhiteTurn() ? N - 2 : 1];

    uint64_t push1_pawns = (b.getWhiteTurn() ? pawns << N : pawns >> N) & ~all_occupied_squares;

    uint64_t push2_pawns = (push1_pawns & get_rank[b.getWhiteTurn() ? 2 : N - 3]);
    push2_pawns = (b.getWhiteTurn() ? push2_pawns << N : push2_pawns >> N) & ~all_occupied_squares;
    
    uint64_t promotion_pawns = push1_pawns & get_rank[b.getWhiteTurn() ? N - 1 : 0];
    push1_pawns ^= promotion_pawns;

    while(push1_pawns) {
        uint64_t sq = pop_lsb(&push1_pawns);
        cout << sq_to_name[sq] << "\n";
    }
    // cout << "push2\n"; 
    // printBB(push2_pawns);
    // cout << "push1\n"; 
    // printBB(push1_pawns);
    // cout << "promotion\n"; 
    // printBB(promotion_pawns);
}

void addKnightMoves(Board& b, vector<Move>& moves) {

}

void addSlidingMoves(Board& b, vector<Move>& moves) {

}

void addKingMoves(Board& b, vector<Move>& moves) {
    
}