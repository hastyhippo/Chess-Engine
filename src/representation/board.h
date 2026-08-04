#pragma once
#include <bits/stdc++.h>
#include <stack>
#include "../misc/defines.h"

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

struct BoardState {
    uint16_t board_info;
    uint8_t enpassant_sq;
    uint8_t captured_piece;
    uint8_t halfmove;
    uint64_t hash;
};

class Board {
    private:
        bool white_turn;
        uint16_t move_number;
        uint64_t hash;  //  updated zobrist hash
        uint8_t halfmove_clock;  // plies since last capture/pawn move
        
        // Board state members (previously in BoardState)
        uint16_t board_info;
        uint64_t piece_bb[6];  // 0-> pawn | 1->knight | 2->bishops | 3->rooks | 4-> queens | 5-> kings
        uint64_t colour_bb[2];
        uint8_t pieces_arr[64];
        
        BoardState state_history[512];
        int state_history_size = 0;
        
        void saveState();
        void restoreState(const BoardStateData& state);
        
    public:
        bool isWhiteTurn() const { return white_turn; }
        uint64_t getHash() const { return hash; }
        bool is_repetition();

        Board();
        Board(string FEN);
        
        // Piece bitboard methods
        void addPieceBitboard(uint8_t piece_type, uint64_t to_add);
        uint64_t getColourPieces(Colour side);
        uint8_t pieceOn(int sq);
        
        uint64_t getPawnBitboard(Colour side);
        uint64_t getKnightBitboard(Colour side);
        uint64_t getBishopBitboard(Colour side);
        uint64_t getRookBitboard(Colour side);
        uint64_t getQueenBitboard(Colour side);
        uint64_t getKingBitboard(Colour side);
        
        // Board info methods
        uint8_t getCastlingRights();
        uint8_t getEnpassantFile();
        uint8_t getHalfMoveClock();
        void setBoardInfo(uint16_t new_info);
        uint16_t getBoardInfo();
        bool in_check();

        uint64_t get_occupancy();
        uint64_t attackers_to(uint8_t sq);
        uint64_t attackers_to(uint8_t sq, uint64_t ignore_sq);
        uint64_t attackers_to(uint8_t sq, uint64_t ignore_sq, uint64_t occ);
        //Updating game state
        void add_piece(uint8_t piece, uint8_t sq);
        void move_piece(uint8_t from_sq, uint8_t to_sq);
        void remove_piece(uint8_t sq);
        void swap_piece(uint8_t sq, uint8_t piece);

        // Game state methods
        bool getWhiteTurn();
        void make_move(Move move);
        void unmake_move(Move move);
        void printBoard();
};

int psqt_mg(int colour, int type, int sq);  // defined in eval.cpp

inline void update_movelist_evals(Board &b, MoveList &ml) {
    constexpr int piece_val[6] = {100, 300, 300, 500, 900, 0};
    Colour side = b.isWhiteTurn() ? WHITE : BLACK;

    for (int i = 0; i < ml.size; i++) {
        Move &m = ml.moves[i];
        int16_t score = 0;

        uint8_t fromPiece = b.pieceOn(m.getFromSq());
        uint8_t toPiece = b.pieceOn(m.getToSq());
        uint8_t flag = m.getMoveFlag();

        // Multiplying by 10 ensures that the moves are ordered by piece captured first
        // then ordered by the capturing piece

        if (flag == ENPASSANT) {
            score += 10 * piece_val[PAWN] - piece_val[PAWN];  // victim not on to_sq
        } else if (toPiece == EMPTY_SQ) {
            score += psqt_mg(side, type_of(fromPiece), m.getToSq())
                   - psqt_mg(side, type_of(fromPiece), m.getFromSq());
        } else {
            // MVV-LVA: 10x victim keeps every capture above every quiet
            score += 10 * piece_val[type_of(toPiece)] - piece_val[type_of(fromPiece)];
        }
        if (m.isPromo()) score += piece_val[m.promoPiece()] - piece_val[PAWN];
        if (flag == CASTLE) score += 50;

        ml.evals[i] = (int16_t)score;
    }
}

inline void Board::add_piece(uint8_t piece, uint8_t sq) {
    uint64_t pos = 1ULL << sq;

    pieces_arr[sq] = piece;
    piece_bb[type_of(piece)] |= pos;
    colour_bb[colour_of(piece)] |= pos;
}

inline void Board::remove_piece(uint8_t sq) {
    uint8_t piece = pieceOn(sq);
    assert(piece != EMPTY_SQ);
    uint64_t pos = 1ULL << sq;

    pieces_arr[sq] = EMPTY_SQ;
    piece_bb[type_of(piece)] ^= pos;
    colour_bb[colour_of(piece)] ^= pos;
}

inline void Board::move_piece(uint8_t from_sq, uint8_t to_sq) {
    uint64_t from_to_pos = 1ULL << from_sq | 1ULL << to_sq;

    uint8_t piece = pieceOn(from_sq);
    piece_bb[type_of(piece)] ^= from_to_pos;
    colour_bb[colour_of(piece)] ^= from_to_pos;
    pieces_arr[from_sq] = EMPTY_SQ;
    pieces_arr[to_sq] = piece;
}

inline void Board::swap_piece(uint8_t sq, uint8_t piece) {
    remove_piece(sq);
    add_piece(piece, sq);
}