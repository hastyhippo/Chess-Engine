#include "uci.h"

UCI::UCI() {
    board = Board();
}

Move UCI::parse_uci_move(string uci_move) {
    if (uci_move.length() < 4 || uci_move.length() > 5) {
        return Move();
    }
    
    uint8_t from_file = uci_move[0] - 'a';
    uint8_t from_rank = uci_move[1] - '1';
    uint8_t to_file = uci_move[2] - 'a';
    uint8_t to_rank = uci_move[3] - '1';
    
    uint8_t from_sq = from_rank * 8 + from_file;
    uint8_t to_sq = to_rank * 8 + to_file;
    
    // Check if promotion
    if (uci_move.length() == 5) {
        char promo = uci_move[4];
        uint8_t move_flag = 0;
        switch (promo) {
            case 'q': move_flag = PROMOTION_QUEEN; break;
            case 'r': move_flag = PROMOTION_ROOK; break;
            case 'b': move_flag = PROMOTION_BISHOP; break;
            case 'n': move_flag = PROMOTION_KNIGHT; break;
            default: move_flag = PROMOTION_QUEEN; break;
        }
        return Move(from_sq, to_sq, move_flag);
    }
    
    // Generate moves to find the correct move flag for moves like enpassant which arne't explicity coded
    MoveList moves = generate_moves<ALL_MOVES>(board);
    for (Move m : moves) {
        if (m.getFromSq() == from_sq && m.getToSq() == to_sq) {
            return m;
        }
    }
    assert("move not found");
    return Move(from_sq, to_sq, 0);
}

string UCI::move_to_uci(Move m) {
    string uci = sq_to_name[m.getFromSq()] + sq_to_name[m.getToSq()];
    if (m.isPromo()) {
        PieceType promo = m.promoPiece();
        switch (promo) {
            case QUEEN: uci += "q"; break;
            case ROOK: uci += "r"; break;
            case BISHOP: uci += "b"; break;
            case KNIGHT: uci += "n"; break;
            default: break;
        }
    }
    return uci;
}

void UCI::uci() {
    cout << "id name hippoengine\n";
    cout << "id author hastyhippo\n";
    cout << "uciok\n";
}

void UCI::isready() {
    cout << "readyok\n";
}

void UCI::set_option(string cmd) {
    vector<string> tokens = splitString(cmd, ' ');
    if (tokens.size() >= 3 && tokens[1] == "name") {
    }
}

void UCI::register_engine(string cmd) {

}

void UCI::uci_new_game() {
    board = Board();
}

void UCI::position(string cmd) {
    vector<string> tokens = splitString(cmd, ' ');
    
    if (tokens.size() < 2) return;
    
    // Parse position
    int ind = 1;
    if (tokens[ind++] == "startpos") {
        board = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    } else if (tokens[ind++] == "fen") {
        string new_fen = "";
        for (int fen_tokens = 0; fen_tokens < 6; fen_tokens++) {
            if (ind >= tokens.size()) {
                cout << "FEN STRING invalid and not long enough";
                return;
            }
            new_fen += tokens[ind];
        }
        board = Board(new_fen);
    } else {
        cout << "invalid input for position";
        return;
    }

    // Parse moves
    if (ind < tokens.size() && tokens[ind] == "moves") {
        Move m = parse_uci_move(tokens[ind++]);
        board.make_move(m);
    }

    board.printBoard();
}

void UCI::go(string cmd) {
    vector<string> tokens = splitString(cmd, ' ');
    
    int depth = 5;
    bool infinite = false;
    int movetime = 0;
    int wtime = 0, btime = 0, winc = 0, binc = 0;
    
    for (int i = 1; i < tokens.size(); i++) {
        if (tokens[i++] == "depth") {
            if (i >= tokens.size()) {
                cout << "Error with tokens: depth";
            }
            depth = stoi(tokens[i++]);
        } else if (tokens[i++] == "infinite") {
            infinite = true;
        } else if (tokens[i++] == "movetime") {
            if (i >= tokens.size()) {
                cout << "Error with tokens: movetime";
            }
            movetime = stoi(tokens[i++]);
        } else if (tokens[i++] == "wtime") {
            if (i >= tokens.size()) {
                cout << "Error with tokens: wtime";
            }
            wtime = stoi(tokens[i++]);
        } else if (tokens[i++] == "btime") {
            if (i >= tokens.size()) {
                cout << "Error with tokens: btime";
            }
            btime = stoi(tokens[i++]);
        } else if (tokens[i++] == "winc") {
            if (i >= tokens.size()) {
                cout << "Error with tokens: winc";
            }
            winc = stoi(tokens[i++]);
        } else if (tokens[i++] == "binc") {
            if (i >= tokens.size()) {
                cout << "Error with tokens: binc";
            }
            binc = stoi(tokens[i++]);
        }
    }
    
    stop_search = false;
    
    Colour side_to_move = board.getWhiteTurn() ? WHITE : BLACK;
    int best_value = INT32_MIN;
    
    MoveList moves = generate_moves<ALL_MOVES>(board);
    if (moves.size == 0) {
        cout << "bestmove (none)\n";
        return;
    }
    
    Move best_move = get_best_move(board, side_to_move, depth);
    cout << "bestmove " << move_to_uci(best_move) << "\n";
}

void UCI::stop() {
    stop_search = true;
}

void UCI::ponderhit() {
    // Handle ponder hit
}

void UCI::debug_cmd(string cmd) {
    vector<string> tokens = splitString(cmd, ' ');
    if (tokens.size() >= 2) {
        debug = (tokens[1] == "on");
    }
}

void UCI::loop() {
    std::ios::sync_with_stdio(false);

    string cmd;
    while(getline(cin, cmd)) {
        if (cmd.substr(0, 5) == "debug") {
            debug_cmd(cmd);
        } else if (cmd == "isready") {
            isready();
        } else if (cmd == "uci") {
            uci();
        } else if (cmd.substr(0, 9) == "setoption") {
            set_option(cmd);
        } else if (cmd.substr(0, 8) == "register") {
            register_engine(cmd);
        } else if (cmd == "ucinewgame") {
            uci_new_game();
        } else if (cmd.substr(0, 8) == "position") {
            position(cmd);
        } else if (cmd.substr(0, 2) == "go") {
            go(cmd);
        } else if (cmd == "stop") {
            stop();
        } else if (cmd == "ponderhit") {
            ponderhit();
        } else if (cmd == "quit") {
            exit(0);
        }
    }
}

