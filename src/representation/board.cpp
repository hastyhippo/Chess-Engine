#include "board.h"

Board::Board() {
    this->white_turn = true;
    this->board_info = 0;
    for (int i = 0; i < 12; i++) {
        this->piece_bitboards[i] = 0ULL;
    }
}

void Board::printBoard() {
    vector<char> piece_symbols = {'P', 'N', 'B', 'R', 'Q', 'K', 'p', 'n', 'b', 'r', 'q', 'k'};
    for (int i = 8; i >= 0; i--) {
        for (int j = 0; j < 8; j++) {
            bool found = false;
            for (int k = 0; k < 12; k++) {
                if (piece_bitboards[k] & (1ULL << (i * 8 + j))) {
                    if (found) {
                        cerr << "Piece represented twice\n";
                        return;
                    }
                    cout << piece_symbols[k];
                    found = true;
                }
            }
            if (!found) {
                cout << "-";
            }
        }
        cout << "\n";
    }
}

Board::Board(string FEN) {
    for (int i = 0; i < 12; i++) {
        this->piece_bitboards[i] = 0ULL;
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

    // printBoard();
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
    // cout << "castling rights: " << castling_rights;
    new_board_info |= castling_rights;
    
    if(fen_split[3] != "-") {
        new_board_info |= (fen_split[3][0] - 'a') << 10;
    }

    new_board_info |= stoi(fen_split[4]) << 4;
    this->board_info = new_board_info;
    // cout << "boardInfo: " << this->board_info << "\n";
    this->move_number = stoi(fen_split[5]);
}

uint64_t Board::getPieceBitboard(PieceType piece_type) {
    return this->piece_bitboards[piece_type];
}

uint64_t Board::getPieceBitboard(PieceType piece_type, bool current_player) {
    return this->piece_bitboards[piece_type + (6 * (1 - current_player))];
}

void Board::addPieceBitboard(PieceType piece_type, uint64_t to_add) {
    this->piece_bitboards[piece_type] = this->piece_bitboards[piece_type] | to_add;
    updateOccupiedSquares();
}


uint8_t Board::getCastlingRights() {
    return (this->board_info & CASTLING_BITMASK);
}

uint8_t Board::getHalfMoveClock() {
    return (this->board_info & HALFMOVES_BITMASK) >> 4;
}

uint8_t Board::getEnpassantSquare() {
    return (this->board_info & ENPASSANT_BITMASK) >> 10;
}

bool Board::getWhiteTurn() {
    return (this->white_turn);
}

void Board::updateOccupiedSquares() {
    white_occupied_squares = 0ULL;
    black_occupied_squares = 0ULL;
    for (int i = 0; i < 6; i++) {
        white_occupied_squares |= piece_bitboards[i];
        black_occupied_squares |= piece_bitboards[i + 6];
    }
    all_occupied_squares = white_occupied_squares | black_occupied_squares;
}

uint64_t Board::getAllOccupiedSquares() {
    return all_occupied_squares;
}

uint64_t Board::getWhiteOccupiedSquares() {
    return white_occupied_squares;
}

uint64_t Board::getBlackOccupiedSquares() {
    return black_occupied_squares;
}

void Board::makeMove(Move& move) {
}

void Board::unmakeMove() {

}