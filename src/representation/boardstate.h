#pragma once

#include <bits/stdc++.h>
#include "../misc/defines.h"
#include "../movegen/move.h"

using namespace std;

#define ENPASSANT_BITMASK 0b11110000000000
#define HALFMOVES_BITMASK 0b1111110000
#define CASTLING_BITMASK 0b1111


class BoardState {
    private:
        // uint64_t piece_bb[12];
        uint16_t board_info;
        // castling (4 bits) halfmoves (6 bits) enpassant (4 bits)

        uint64_t piece_bb[6];
        // 0-> pawn | 1->knight | 2->bishops | 3->rooks | 4-> queens | 5-> kings |
        uint64_t colour_bb[2];
        int8_t pieces_arr[64];
    public:
        BoardState();
        uint64_t getPieceBitboard(PieceType piece_type);
        uint64_t getPieceBitboard(PieceType piece_type, bool current_player);
        uint64_t getColourPieces(bool white);
        void addPieceBitboard(PieceType piece_type, uint64_t to_add);
        uint8_t getCastlingRights();
        uint8_t getEnpassantSquare();
        uint8_t getHalfMoveClock();
        void setBoardInfo(uint16_t new_info);
        uint16_t getBoardInfo();
        uint8_t pieceOn(int sq);
        void makeMove(Move& move, bool white);
        void printBoard();
};