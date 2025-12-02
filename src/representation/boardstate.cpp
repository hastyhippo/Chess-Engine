#include "boardstate.h"
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

uint8_t BoardState::getEnpassantSquare() { // 0 means no square 1-8 inclusive means that rank
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
    
    uint8_t castling_rights = getCastlingRights();

    // Update castling rights. Moving from/to king square removes both rights. Moving to/from rook squares removes 1
    castling_rights &= ~(castling_mask[from_sq] | castling_mask[to_sq]);

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


    uint8_t enp_file = NO_ENP;
    if (move_flag == DOUBLE_PUSH) {
        enp_file = to_sq % 8;
    } else if (move_flag == CASTLE) {
        int side = ((1ULL << to_sq) & G_FILE) ? KINGSIDE : QUEENSIDE;
        uint64_t rook_move_bb = castling_rook_moves[white ? 0 : 1][side];
        piece_bb[W_ROOK % 6] ^= rook_move_bb;
        colour_bb[white ? 0 : 1] ^= rook_move_bb;
        
        uint8_t rook_piece = white ? W_ROOK : B_ROOK;
        pieces_arr[side == KINGSIDE ? (white ? 7 : 63) : (white ? 0 : 56)] = EMPTY_SQ;
        pieces_arr[side == KINGSIDE ? (white ? 5 : 61) : (white ? 3 : 59)] = rook_piece;
    } else if (move_flag == ENPASSANT) {
        uint8_t capt_sq = white ? (to_sq - 8) : (to_sq + 8);
        uint8_t capt_piece = pieceOn(capt_sq);
        piece_bb[capt_piece % 6] ^= (1ULL << capt_sq);
        colour_bb[white ? 1 : 0] ^= (1ULL << capt_sq);
        pieces_arr[capt_sq] = EMPTY_SQ;
    }
    
    // Update board_info with new castling rights and en passant
    uint16_t new_board_info = (castling_rights & CASTLING_BITMASK);
    new_board_info |= (enp_file << 10) & ENPASSANT_BITMASK;
    new_board_info |= ((board_info & HALFMOVES_BITMASK));
    board_info = new_board_info;
}

void BoardState::printBoard() {
    const char piece_symbols[] = {'P', 'N', 'B', 'R', 'Q', 'K', 'p', 'n', 'b', 'r', 'q', 'k'};
    
    uint8_t castling = getCastlingRights();
    if (castling == 0) {
        cout << "Castling: - | ";
    } else {
        cout << "Castling: ";
        if (castling & 0b0001) cout << "K";  // White kingside
        if (castling & 0b0010) cout << "Q";  // White queenside
        if (castling & 0b0100) cout << "k";  // Black kingside
        if (castling & 0b1000) cout << "q";  // Black queenside
        cout << " | ";
    }
    
    uint8_t enp_file = getEnpassantSquare();
    if (enp_file != NO_ENP) {
        char file_char = 'a' + enp_file;
        cout << "En passant file: " << file_char << " |";
    }
    cout << " Halfmove clock: " << (int)getHalfMoveClock() << " \n\n";

    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            int sq = rank * 8 + file;
            uint8_t piece = pieceOn(sq);
            
            if (piece == EMPTY_SQ) {
                cout << "-";
            } else if (piece < 12) {
                cout << piece_symbols[piece];
            } else {
                cout << "?";
            }
        }
        cout << "\n";
    }
    cout << "----------------------\n";
}