#include "movegen.h"

uint64_t attackedSquares;
uint64_t friendlyOccupiedSquares;
uint64_t enemyOccupiedSquares;
uint64_t allOccupiedSquares;

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
    friendlyOccupiedSquares = b.getWhiteOccupiedSquares();
    enemyOccupiedSquares = b.getBlackOccupiedSquares();
    allOccupiedSquares = b.getAllOccupiedSquares();
}

void addPawnMoves(Board& b, vector<Move>& moves) {
    uint64_t friendlyPawns = b.getPieceBitboard(W_PAWN, b.getWhiteTurn());
    
}

void addKnightMoves(Board& b, vector<Move>& moves) {

}

void addSlidingMoves(Board& b, vector<Move>& moves) {

}

void addKingMoves(Board& b, vector<Move>& moves) {
    
}