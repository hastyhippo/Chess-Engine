#include "board.h"

Board::Board() {
    this->whiteTurn = true;
    this->boardInfo = 0;
    for (int i = 0; i < 12; i++) {
        this->pieceBitboards[i] = 0ULL;
    }
}

void Board::printBoard() {
    vector<char> pieceSymbols = {'P', 'N', 'B', 'R', 'Q', 'K', 'p', 'n', 'b', 'r', 'q', 'k'};
    for (int i = 8; i >= 0; i--) {
        for (int j = 0; j < 8; j++) {
            bool found = false;
            for (int k = 0; k < 12; k++) {
                if (pieceBitboards[k] & (1ULL << (i * 8 + j))) {
                    if (found) {
                        cerr << "Piece represented twice\n";
                        return;
                    }
                    cout << pieceSymbols[k];
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
        this->pieceBitboards[i] = 0ULL;
    }
    map<char, PieceType> FENCharToPiece = {{'P', W_PAWN}, {'N', W_KNIGHT}, {'B', W_BISHOP}, {'R',W_ROOK}, {'Q', W_QUEEN}, {'K', W_KING}, 
                                    {'p', B_PAWN}, {'n', B_KNIGHT}, {'b', B_BISHOP}, {'r', B_ROOK}, {'q', B_QUEEN}, {'k', B_KING}};

    vector<string> FEN_split = splitString(FEN, ' ');
    if (FEN_split.size() != 6) {
        cerr << "FEN is not formatted correctly, with " << FEN_split.size() << " fields\n";
    }

    // Building the board

    vector<string> row_split = splitString(FEN_split[0], '/');
    if (row_split.size() != N_ROWS) {
        cerr << "FEN Rows is not formatted correctly, with " << row_split.size() << " fields for rows\n";
    }

    for (int i = 0; i < N_ROWS; i++) {
        int row = N_ROWS - i - 1, col = 0;
        for (auto ch : row_split[i]) {
            if (!isalpha(ch)) {
                col += (ch-'0');
            } else {
                addPieceBitboard(FENCharToPiece[ch], 1ULL << (row * N_COLS + col));
                col++;
            }
        }
    }

    // printBoard();
    uint16_t newBoardInfo = 0;

    whiteTurn = (FEN_split[1] == "w");
    if (FEN_split[2].size() > 4) {
        cerr << "Castling rights is not formatted correctly, with " << FEN_split[2].size() << " fields for castling\n";
    }
    string expectedCastlingRights = "KQkq";
    uint8_t castlingRights = 0;
    for (int i = 0; i < 4; i++) {
        if(FEN_split[2].find(expectedCastlingRights[i]) != string::npos) {
            castlingRights |= (1 << i);
        }
    }
    // cout << "castling rights: " << castlingRights;
    newBoardInfo |= castlingRights;
    
    if(FEN_split[3] != "-") {
        newBoardInfo |= (FEN_split[3][0] - 'a') << 10;
    }

    newBoardInfo |= stoi(FEN_split[4]) << 4;
    this->boardInfo = newBoardInfo;
    // cout << "boardInfo: " << this->boardInfo << "\n";
    this->moveNumber = stoi(FEN_split[5]);
}

uint64_t Board::getPieceBitboard(PieceType pieceType) {
    return this->pieceBitboards[pieceType];
}

uint64_t Board::getPieceBitboard(PieceType pieceType, bool currentPlayer) {
    return this->pieceBitboards[pieceType + (6 * (1 - currentPlayer))];
}

void Board::addPieceBitboard(PieceType pieceType, uint64_t toAdd) {
    this->pieceBitboards[pieceType] = this->pieceBitboards[pieceType] | toAdd;
    updateOccupiedSquares();
}


uint8_t Board::getCastlingRights() {
    return (this->boardInfo & CASTLING_BITMASK);
}

uint8_t Board::getHalfMoveClock() {
    return (this->boardInfo & HALFMOVES_BITMASK) >> 4;
}

uint8_t Board::getEnpassantSquare() {
    return (this->boardInfo & ENPASSANT_BITMASK) >> 10;
}

bool Board::getWhiteTurn() {
    return (this->whiteTurn);
}

void Board::updateOccupiedSquares() {
    whiteOccupiedSquares = 0ULL;
    blackOccupiedSquares = 0ULL;
    for (int i = 0; i < 6; i++) {
        whiteOccupiedSquares |= pieceBitboards[i];
        blackOccupiedSquares |= pieceBitboards[i + 6];
    }
    allOccupiedSquares = whiteOccupiedSquares | blackOccupiedSquares;
}

uint64_t Board::getAllOccupiedSquares() {
    return allOccupiedSquares;
}

uint64_t Board::getWhiteOccupiedSquares() {
    return whiteOccupiedSquares;
}

uint64_t Board::getBlackOccupiedSquares() {
    return blackOccupiedSquares;
}

void Board::makeMove(Move& move) {
}

void Board::unmakeMove() {

}