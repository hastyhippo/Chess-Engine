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
        bool whiteTurn;
        uint16_t boardInfo;
        uint16_t moveNumber;
        // castling (4 bits)
        // halfmoves (6 bits)
        // enpassant (3 bits)
        uint64_t pieceBitboards[12];
        uint64_t allOccupiedSquares;
        uint64_t whiteOccupiedSquares;
        uint64_t blackOccupiedSquares;
        
        void updateOccupiedSquares();

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
        uint64_t getAllOccupiedSquares();
        uint64_t getWhiteOccupiedSquares();
        uint64_t getBlackOccupiedSquares();
        
        void makeMove(Move& move);
        void unmakeMove();
        void printBoard();
};