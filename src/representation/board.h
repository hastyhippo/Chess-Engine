#pragma once
#include <bits/stdc++.h>
#include "../misc/defines.h"


using namespace std;

#define ENPASSANT_BITMASK 0b1110000000000
#define HALFMOVES_BITMASK 0b1111110000
#define CASTLING_BITMASK 0b1111

class Move;

class Board {
    private: 
        bool white_turn;
        uint16_t board_info;
        uint16_t move_number;
        // castling (4 bits)
        // halfmoves (6 bits)
        // enpassant (3 bits)
        uint64_t piece_bitboards[12];
        uint64_t all_occupied_squares;
        uint64_t white_occupied_squares;
        uint64_t black_occupied_squares;
        
        void updateOccupiedSquares();

    public: 
        Board();
        Board(string FEN);
        uint64_t getPieceBitboard(PieceType piece_type);
        uint64_t getPieceBitboard(PieceType piece_type, bool current_player);
        void addPieceBitboard(PieceType piece_type, uint64_t to_add);

        bool getWhiteTurn();
        uint8_t getCastlingRights();
        uint8_t getEnpassantSquare();
        uint8_t getHalfMoveClock();
        uint64_t getAllOccupiedSquares();
        uint64_t getWhiteOccupiedSquares();
        uint64_t getBlackOccupiedSquares();
        
        void makeMove(Move& move);
        void unmakeMove();
        void printBoard();
};