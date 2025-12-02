#pragma once
#include <bits/stdc++.h>
#include <stack>
#include "../misc/defines.h"
#include "boardstate.h"


using namespace std;

class Move;

class Board {
    private: 
        bool white_turn;
        uint16_t move_number;

        BoardState board_state;
        stack<BoardState> state_history;
        
    public: 
        Board();
        Board(string FEN);
        uint64_t getPieceBitboard(PieceType piece_type);
        uint64_t getPieceBitboard(PieceType piece_type, bool current_player);
        void addPieceBitboard(PieceType piece_type, uint64_t to_add);
        uint8_t getCastlingRights();
        uint8_t getEnpassantSquare();
        uint8_t getHalfMoveClock();
        void setBoardInfo(uint16_t new_info);

        bool getWhiteTurn();
        uint64_t getColourPieces(bool white);    
        void makeMove(Move& move);
        void unmakeMove();
        void printBoard();
};