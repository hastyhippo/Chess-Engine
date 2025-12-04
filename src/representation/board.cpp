#include "board.h"

// Constructor
Board::Board() {
    white_turn = true;
    move_number = 0;
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

Board::Board(string FEN) {
    // Initialize arrays
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

    map<char, PieceType> fen_char_to_piece = {{'P', W_PAWN}, {'N', W_KNIGHT}, {'B', W_BISHOP}, {'R',W_ROOK}, {'Q', W_QUEEN}, {'K', W_KING}, 
                                    {'p', B_PAWN}, {'n', B_KNIGHT}, {'b', B_BISHOP}, {'r', B_ROOK}, {'q', B_QUEEN}, {'k', B_KING}};

    vector<string> fen_split = splitString(FEN, ' ');
    if (fen_split.size() != 6) {
        cerr << "FEN is not formatted correctly, with " << fen_split.size() << " fields\n";
    }

    // Building the board
    vector<string> row_split = splitString(fen_split[0], '/');
    if (row_split.size() != N_ROWS) {
        cerr << "FEN Rows is not formatted correctly, with " << row_split.size() << " fields for rows\n";
    }

    for (int i = 0; i < N_ROWS; i++) {
        int row = N_ROWS - i - 1, col = 0;
        for (auto ch : row_split[i]) {
            if (!isalpha(ch)) {
                col += (ch-'0');
            } else {
                addPieceBitboard(fen_char_to_piece[ch], 1ULL << (row * N_COLS + col));
                col++;
            }
        }
    }

    uint16_t new_board_info = 0;
    white_turn = (fen_split[1] == "w");
    if (fen_split[2].size() > 4) {
        cerr << "Castling rights is not formatted correctly, with " << fen_split[2].size() << " fields for castling\n";
    }
    string expected_castling_rights = "KQkq";
    uint8_t castling_rights = 0;
    for (int i = 0; i < 4; i++) {
        if(fen_split[2].find(expected_castling_rights[i]) != string::npos) {
            castling_rights |= (1 << i);
        }
    }
    new_board_info |= castling_rights;
    
    if(fen_split[3] != "-") {
        new_board_info |= ((fen_split[3][0] - 'a') << 10);
    } else {
        new_board_info |= (NO_ENP) << 10;
    }

    new_board_info |= stoi(fen_split[4]) << 4;
    this->board_info = new_board_info;
    this->move_number = stoi(fen_split[5]);
    saveState();
}

// Helper methods for state history
void Board::saveState() {
    BoardStateData state;
    state.board_info = board_info;
    for (int i = 0; i < 6; ++i) {
        state.piece_bb[i] = piece_bb[i];
    }
    for (int c = 0; c < 2; ++c) {
        state.colour_bb[c] = colour_bb[c];
    }
    for (int i = 0; i < 64; ++i) {
        state.pieces_arr[i] = pieces_arr[i];
    }
    state_history.push_back(state);
}

void Board::restoreState(const BoardStateData& state) {
    board_info = state.board_info;
    for (int i = 0; i < 6; ++i) {
        piece_bb[i] = state.piece_bb[i];
    }
    for (int c = 0; c < 2; ++c) {
        colour_bb[c] = state.colour_bb[c];
    }
    for (int i = 0; i < 64; ++i) {
        pieces_arr[i] = state.pieces_arr[i];
    }
}

// Piece bitboard methods
uint64_t Board::getColourPieces(bool white) {
    return this->colour_bb[1 - white];
}

uint64_t Board::getPieceBitboard(PieceType piece_type) {
    return this->piece_bb[piece_type % 6] & colour_bb[piece_type / 6];
}

uint64_t Board::getPieceBitboard(PieceType piece_type, bool white) {
    return this->piece_bb[piece_type] & colour_bb[1 - white];
}

void Board::addPieceBitboard(PieceType piece_type, uint64_t to_add) {
    this->piece_bb[piece_type % 6] |= to_add;
    this->colour_bb[piece_type / 6] |= to_add;
    this->pieces_arr[get_lsb(to_add)] = piece_type;
}

uint8_t Board::pieceOn(int sq) {
    return this->pieces_arr[sq];
}

// Board info methods
uint8_t Board::getCastlingRights() {
    return (this->board_info & CASTLING_BITMASK);
}

uint8_t Board::getHalfMoveClock() {
    return (this->board_info & HALFMOVES_BITMASK) >> 4;
}

uint8_t Board::getEnpassantFile() {
    return (this->board_info & ENPASSANT_BITMASK) >> 10;
}

void Board::setBoardInfo(uint16_t new_info) {
    this->board_info = new_info;
}

uint16_t Board::getBoardInfo() {
    return this->board_info;
}

// Move methods
void Board::makeMove(Move& move, bool white) {
    uint8_t from_sq = move.getFromSq();
    uint8_t to_sq = move.getToSq();
    uint8_t move_flag = move.getMoveFlag();

    uint8_t moved_piece = pieceOn(from_sq);
    
    uint8_t castling_rights = getCastlingRights();

    // Update castling rights. Moving from/to king square removes both rights. Moving to/from rook squares removes 1
    castling_rights &= ~(castling_mask[from_sq] | castling_mask[to_sq]);

    if ((pieceOn(to_sq) != EMPTY_SQ ) && (move_flag != ENPASSANT)) {
        uint8_t capt_piece = pieceOn(to_sq);
        piece_bb[capt_piece % 6] ^= (1ULL << to_sq);
        colour_bb[white ? 1 : 0] ^= (1ULL << to_sq);
        pieces_arr[to_sq] = EMPTY_SQ;
    }
    piece_bb[moved_piece % 6] ^= (1ULL << from_sq);
    colour_bb[white ? 0 : 1] ^= (1ULL << from_sq);
    pieces_arr[from_sq] = EMPTY_SQ;

    uint8_t new_piece = move.isPromo() ? move.promoPiece() + (white? 0 : 6) : moved_piece;
    piece_bb[new_piece % 6] ^= (1ULL << to_sq);
    colour_bb[white ? 0 : 1] ^= (1ULL << to_sq);
    pieces_arr[to_sq] = new_piece;


    uint8_t enp_file = NO_ENP;
    if (move_flag == DOUBLE_PUSH) {
        enp_file = (to_sq % 8);
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

// Game state methods
bool Board::getWhiteTurn() {
    return (this->white_turn);
}

void Board::makeMove(Move& move) {
    saveState();
    makeMove(move, white_turn);
    this->move_number++;
    this->white_turn = !this->white_turn;
}

void Board::unmakeMove() {
    BoardStateData prev_state = state_history.back();
    state_history.pop_back();
    restoreState(prev_state);
    this->move_number--;
    this->white_turn = !this->white_turn;
}

void Board::printBoard() {
    const char piece_symbols[] = {'P', 'N', 'B', 'R', 'Q', 'K', 'p', 'n', 'b', 'r', 'q', 'k'};
    
    cout << "----------------------\n";
    cout << "Move number: " << move_number << " | Side to move: " << (white_turn ? "White" : "Black") << " \n";
    
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
    
    uint8_t enp_file = getEnpassantFile();
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
