#include "boardstate.h"

constexpr uint8_t EMPTY_SQ = 15;
#define KINGSIDE 0
#define QUEENSIDE 1

BoardState::BoardState() {
    for (int i = 0; i < 6; ++i) {
        piece_bb[i] = 0ULL;
    }
    for (int c = 0; c < 2; ++c) {
        colour_bb[c] = 0ULL;
    }
    this->board_info = 0;
    for (int i = 0; i < 64; ++i) {
        this->pieces_arr[i] = EMPTY_SQ;
    }
}

uint64_t BoardState::getColourPieces(bool white) {
    return this->colour_bb[1 - white];
}

uint64_t BoardState::getPieceBitboard(PieceType piece_type) {
    return this->piece_bb[piece_type % 6] & colour_bb[piece_type / 6];
}

uint64_t BoardState::getPieceBitboard(PieceType piece_type, bool white) {
    return this->piece_bb[piece_type] & colour_bb[1 - white];
}

void BoardState::addPieceBitboard(PieceType piece_type, uint64_t to_add) {
    this->piece_bb[piece_type % 6] |= to_add;
    this->colour_bb[piece_type / 6] |= to_add;
    this->pieces_arr[_tzcnt_u64(to_add)] = piece_type;
}

uint8_t BoardState::getCastlingRights() {
    return (this->board_info & CASTLING_BITMASK);
}

uint8_t BoardState::getHalfMoveClock() {
    return (this->board_info & HALFMOVES_BITMASK) >> 4;
}

uint8_t BoardState::getEnpassantSquare() {
    return (this->board_info & ENPASSANT_BITMASK) >> 10;
}

void BoardState::setBoardInfo(uint16_t new_info) {
    this->board_info = new_info;
}

uint16_t BoardState::getBoardInfo() {
    return this->board_info;
}

uint8_t BoardState::pieceOn(int sq) {
    return this->pieces_arr[sq];
}

void BoardState::makeMove(Move& move, bool white) {
    uint8_t from_sq = move.getFromSq();
    uint8_t to_sq = move.getToSq();
    uint8_t move_flag = move.getMoveFlag();

    uint8_t moved_piece = pieceOn(from_sq);
    
    // Handle capture first (before moving piece to to_sq)
    if (move.isCapture() && move_flag != ENPASSANT) {
        uint8_t capt_piece = pieceOn(to_sq);
        piece_bb[capt_piece % 6] ^= (1ULL << to_sq);
        colour_bb[white ? 1 : 0] ^= (1ULL << to_sq);
        pieces_arr[to_sq] = EMPTY_SQ;
    }
    
    piece_bb[moved_piece % 6] ^= (1ULL << from_sq);
    colour_bb[white ? 0 : 1] ^= (1ULL << from_sq);
    pieces_arr[from_sq] = EMPTY_SQ;

    uint8_t new_piece = move.isPromo() ? move.promoPiece() : moved_piece;
    piece_bb[new_piece % 6] ^= (1ULL << to_sq);
    colour_bb[white ? 0 : 1] ^= (1ULL << to_sq);
    pieces_arr[to_sq] = new_piece;

    if (move_flag == CASTLE) {
        int side = (to_sq & G_FILE) ? KINGSIDE : QUEENSIDE;
        uint64_t rook_move_bb = castling_rook_moves[white ? 0 : 1][side];
                piece_bb[W_ROOK % 6] ^= rook_move_bb;
        colour_bb[white ? 0 : 1] ^= rook_move_bb;
        
        uint8_t rook_piece = white ? W_ROOK : B_ROOK;
        pieces_arr[side == 0 ? (white ? 7 : 63) : (white ? 0 : 56)] = EMPTY_SQ;
        pieces_arr[side == 0 ? (white ? 5 : 61) : (white ? 3 : 59)] = rook_piece;
    } else if (move_flag == ENPASSANT) {
        uint8_t capt_sq = white ? (to_sq - 8) : (to_sq + 8);
        uint8_t capt_piece = pieceOn(capt_sq);
        piece_bb[capt_piece % 6] ^= (1ULL << capt_sq);
        colour_bb[white ? 1 : 0] ^= (1ULL << capt_sq);
        pieces_arr[capt_sq] = EMPTY_SQ;
    } else if (move_flag == DOUBLE_PUSH) {
        
    }
}