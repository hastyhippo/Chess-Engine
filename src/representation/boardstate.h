#include "bits/stdc++.h"
#include "../misc/defines.h"
using namespace std;

#define ENPASSANT_BITMASK 0b1110000000000
#define HALFMOVES_BITMASK 0b1111110000
#define CASTLING_BITMASK 0b1111


class BoardState {
    private:
        uint64_t piece_bb[12];
        uint16_t board_info;
    public:
        BoardState();
        uint64_t getPieceBitboard(PieceType piece_type);
        uint64_t getPieceBitboard(PieceType piece_type, bool current_player);
        void addPieceBitboard(PieceType piece_type, uint64_t to_add);
        uint8_t getCastlingRights();
        uint8_t getEnpassantSquare();
        uint8_t getHalfMoveClock();
        void setBoardInfo(uint16_t new_info);
        uint16_t getBoardInfo();
};