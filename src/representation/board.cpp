#include "board.h"

void Board::printBoard() {
    board_state.printBoard();
        
    cout << "Move number: " << move_number << "\n";
    cout << "Side to move: " << (white_turn ? "White" : "Black") << "\n";
}

Board::Board(string FEN) {
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
        new_board_info |= (1 + (fen_split[3][0] - 'a')) << 10;  // Store as 1-8 to match makeMove format
    }

    new_board_info |= stoi(fen_split[4]) << 4;
    this->board_state.setBoardInfo(new_board_info);
    // cout << "boardInfo: " << this->board_info << "\n";
    this->move_number = stoi(fen_split[5]);
    state_history.push(this->board_state);
}

uint64_t Board::getPieceBitboard(PieceType piece_type) {
    return this->board_state.getPieceBitboard(piece_type);
}

void Board::setBoardInfo(uint16_t new_info) {
    this->board_state.setBoardInfo(new_info);
}

uint64_t Board::getPieceBitboard(PieceType piece_type, bool current_player) {
    return this->board_state.getPieceBitboard(piece_type, current_player);
}

void Board::addPieceBitboard(PieceType piece_type, uint64_t to_add) {
    this->board_state.addPieceBitboard(piece_type, to_add);
}

uint64_t Board::getColourPieces(bool white) {
    return this->board_state.getColourPieces(white);
}

uint8_t Board::getCastlingRights() {
    return this->board_state.getCastlingRights();
}

uint8_t Board::getHalfMoveClock() {
    return this->board_state.getHalfMoveClock();
}

uint8_t Board::getEnpassantSquare() {
    return this->board_state.getEnpassantSquare();
}

bool Board::getWhiteTurn() {
    return (this->white_turn);
}

void Board::makeMove(Move& move) {
    board_state.makeMove(move, white_turn);
    state_history.push(board_state);

    this->move_number++;
    this->white_turn = !this->white_turn;
}

void Board::unmakeMove() {
    BoardState prev_state = state_history.top();
    state_history.pop();

    this->move_number--;
    this->white_turn = !this->white_turn;
    this->board_state = prev_state;
}