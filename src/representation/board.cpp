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

    map<char, uint8_t> fen_char_to_piece = {
        {'P', piece(WHITE, PAWN)}, {'N', piece(WHITE, KNIGHT)}, {'B', piece(WHITE, BISHOP)}, 
        {'R', piece(WHITE, ROOK)}, {'Q', piece(WHITE, QUEEN)}, {'K', piece(WHITE, KING)}, 
        {'p', piece(BLACK, PAWN)}, {'n', piece(BLACK, KNIGHT)}, {'b', piece(BLACK, BISHOP)}, 
        {'r', piece(BLACK, ROOK)}, {'q', piece(BLACK, QUEEN)}, {'k', piece(BLACK, KING)}
    };

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
                uint8_t piece_val = fen_char_to_piece[ch];
                addPieceBitboard(piece_val, 1ULL << (row * N_COLS + col));
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
    // state_history.push_back(state);
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
uint64_t Board::getColourPieces(Colour side) {
    return this->colour_bb[side];
}

void Board::addPieceBitboard(uint8_t piece_type, uint64_t to_add) {
    this->piece_bb[type_of(piece_type)] |= to_add;
    this->colour_bb[colour_of(piece_type)] |= to_add;
    this->pieces_arr[get_lsb(to_add)] = piece_type;
}

uint8_t Board::pieceOn(int sq) {
    return this->pieces_arr[sq];
}

uint64_t Board::getPawnBitboard(Colour side) {
    return piece_bb[PAWN] & colour_bb[side];
}

uint64_t Board::getKnightBitboard(Colour side) {
    return piece_bb[KNIGHT] & colour_bb[side];
}

uint64_t Board::getBishopBitboard(Colour side) {
    return piece_bb[BISHOP] & colour_bb[side];
}

uint64_t Board::getRookBitboard(Colour side) {
    return piece_bb[ROOK] & colour_bb[side];
}

uint64_t Board::getQueenBitboard(Colour side) {
    return piece_bb[QUEEN] & colour_bb[side];
}

uint64_t Board::getKingBitboard(Colour side) {
    return piece_bb[KING] & colour_bb[side];
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

// Game state methods
bool Board::getWhiteTurn() {
    return (this->white_turn);
}

constexpr uint8_t NO_PIECE = 255;
constexpr uint8_t NO_ENP_SQ = 255;

void Board::makeMove(Move m) {
    Colour side = white_turn ? WHITE : BLACK;
    
    // Save old board_info BEFORE modifying it
    BoardState new_bs;
    new_bs.board_info = board_info;  // Save OLD board_info
    new_bs.enpassant_sq = NO_ENP_SQ;
    new_bs.captured_piece = NO_PIECE;

    uint8_t from_sq = m.getFromSq();
    uint8_t to_sq = m.getToSq();
    uint8_t move_flag = m.getMoveFlag();
    uint8_t castling_rights = getCastlingRights();

    castling_rights &= ~(castling_mask[from_sq] | castling_mask[to_sq]);

    uint8_t piece_on_target = pieceOn(to_sq);

    if ((piece_on_target != EMPTY_SQ) && (move_flag != ENPASSANT)) {
        new_bs.captured_piece = piece_on_target;
        remove_piece(to_sq);
    }

    move_piece(from_sq, to_sq);

    if (m.isPromo()) {
        swap_piece(to_sq, piece(side, m.promoPiece()));
    }

    uint8_t enp_file = NO_ENP;
    if (move_flag == DOUBLE_PUSH) {
        enp_file = (to_sq % 8);
    } else if (move_flag == CASTLE) {
        int castling_side = to_sq > from_sq ? KINGSIDE : QUEENSIDE;        
        move_piece(castling_rook_from_sq[side][castling_side],  castling_rook_to_sq[side][castling_side]);

    } else if (move_flag == ENPASSANT) {
        uint8_t capt_sq = (side == WHITE) ? (to_sq - 8) : (to_sq + 8);
        new_bs.enpassant_sq = capt_sq;
        uint8_t capt_piece = pieceOn(capt_sq);
        new_bs.captured_piece = capt_piece;
        remove_piece(capt_sq);
    }
    
    // Update board_info with new castling rights and en passant
    uint16_t new_board_info = (castling_rights & CASTLING_BITMASK);
    new_board_info |= (enp_file << 10) & ENPASSANT_BITMASK;
    new_board_info |= (board_info & HALFMOVES_BITMASK);
    board_info = new_board_info;

    state_history.push_back(new_bs);
    this->move_number++;
    this->white_turn = !this->white_turn;
}

void Board::unmakeMove(Move m) {
    BoardState prev_state = state_history.back();
    state_history.pop_back();

    this->move_number--;
    this->white_turn = !this->white_turn;
    
    Colour side = white_turn ? WHITE : BLACK;
    uint8_t from_sq = m.getFromSq();
    uint8_t to_sq = m.getToSq();
    uint8_t move_flag = m.getMoveFlag();

    board_info = prev_state.board_info;

    if (move_flag == CASTLE) {
        int castling_side = to_sq > from_sq ? KINGSIDE : QUEENSIDE;
        move_piece(castling_rook_to_sq[side][castling_side], castling_rook_from_sq[side][castling_side]);
    } else if (move_flag == ENPASSANT) {
        add_piece(prev_state.captured_piece, prev_state.enpassant_sq);
    }

    if (m.isPromo()) {
        swap_piece(to_sq, piece(side, PAWN));
    }

    move_piece(to_sq, from_sq);

    if ((prev_state.captured_piece != NO_PIECE) && (move_flag != ENPASSANT)) {
        add_piece(prev_state.captured_piece, to_sq);
    }
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
