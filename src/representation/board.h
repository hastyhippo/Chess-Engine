#pragma once
#include <bits/stdc++.h>
#include "../misc/defines.h"


using namespace std;

#define ENPASSANT_BITMASK 0b1110000000000
#define HALFMOVES_BITMASK 0b1111110000
#define CASTLING_BITMASK 0b1111

class Board {
    private: 
        bool whiteTurn;
        uint16_t boardInfo;
        uint16_t moveNumber;
        // castling (4 bits)
        // halfmoves (6 bits)
        // enpassant (3 bits)
        uint64_t pieceBitboards[12];
    public: 
        Board();
        Board(string FEN);
        uint64_t getPieceBitboard(PieceType pieceType);
        uint64_t getPieceBitboard(PieceType pieceType, bool currentPlayer);
        void addPieceBitboard(PieceType pieceType, uint64_t toAdd);

        bool getWhiteTurn();
        uint8_t getCastlingRights();
        uint8_t getEnpassantSquare();
        uint8_t getHalfMoveClock();
        
        void printBoard();
};