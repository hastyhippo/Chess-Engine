#pragma once
#include <bits/stdc++.h>
#include <stack>
#include "../misc/defines.h"
#include "../movegen/move.h"

using namespace std;

#define ENPASSANT_BITMASK 0b11110000000000
#define HALFMOVES_BITMASK 0b1111110000
#define CASTLING_BITMASK 0b1111

class Move;

struct BoardStateData {
    uint16_t board_info;
    uint64_t piece_bb[6];
    uint64_t colour_bb[2];
    uint8_t pieces_arr[64];
};

class Board {
    private: 
        bool white_turn;
        uint16_t move_number;
        
        // Board state members (previously in BoardState)
        uint16_t board_info;
        uint64_t piece_bb[6];  // 0-> pawn | 1->knight | 2->bishops | 3->rooks | 4-> queens | 5-> kings
        uint64_t colour_bb[2];
        uint8_t pieces_arr[64];
        
        vector<BoardStateData> state_history;
        
        // Helper method for unmakeMove
        void saveState();
        void restoreState(const BoardStateData& state);
        
    public: 
        Board();
        Board(string FEN);
        
        // Piece bitboard methods
        uint64_t getPieceBitboard(PieceType piece_type);
        uint64_t getPieceBitboard(PieceType piece_type, bool current_player);
        void addPieceBitboard(PieceType piece_type, uint64_t to_add);
        uint64_t getColourPieces(bool white);
        uint8_t pieceOn(int sq);
        
        // Board info methods
        uint8_t getCastlingRights();
        uint8_t getEnpassantFile();
        uint8_t getHalfMoveClock();
        void setBoardInfo(uint16_t new_info);
        uint16_t getBoardInfo();
        
        // Move methods
        void makeMove(Move& move, bool white);
        
        // Game state methods
        bool getWhiteTurn();
        void makeMove(Move& move);
        void unmakeMove();
        void printBoard();
};