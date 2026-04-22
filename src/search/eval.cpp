#include "eval.h"
#include <nmmintrin.h>  // For _mm_popcnt_u64

// Material values (in centipawns)
static const int PIECE_VALUES[] = {
    100,   // PAWN
    320,   // KNIGHT
    330,   // BISHOP
    500,   // ROOK
    900,   // QUEEN
    0      // KING (not used in material evaluation)
};

// returns evaluation from the players perspective
int evaluate(Board &b) {
    int score = 0;
    
    // Evaluate White pieces (positive score)
    score += _mm_popcnt_u64(b.getPawnBitboard(WHITE)) * PIECE_VALUES[PAWN];
    score += _mm_popcnt_u64(b.getKnightBitboard(WHITE)) * PIECE_VALUES[KNIGHT];
    score += _mm_popcnt_u64(b.getBishopBitboard(WHITE)) * PIECE_VALUES[BISHOP];
    score += _mm_popcnt_u64(b.getRookBitboard(WHITE)) * PIECE_VALUES[ROOK];
    score += _mm_popcnt_u64(b.getQueenBitboard(WHITE)) * PIECE_VALUES[QUEEN];
    
    // Evaluate Black pieces (negative score)
    score -= _mm_popcnt_u64(b.getPawnBitboard(BLACK)) * PIECE_VALUES[PAWN];
    score -= _mm_popcnt_u64(b.getKnightBitboard(BLACK)) * PIECE_VALUES[KNIGHT];
    score -= _mm_popcnt_u64(b.getBishopBitboard(BLACK)) * PIECE_VALUES[BISHOP];
    score -= _mm_popcnt_u64(b.getRookBitboard(BLACK)) * PIECE_VALUES[ROOK];
    score -= _mm_popcnt_u64(b.getQueenBitboard(BLACK)) * PIECE_VALUES[QUEEN];
    
    // if black, reverse the piece
    return score * (b.getWhiteTurn() ? 1 : -1);
}