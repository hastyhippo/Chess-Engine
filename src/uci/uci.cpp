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
    size_t ind = 1;
    if (tokens[ind] == "startpos") {
        ind++;
        board = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    } else if (tokens[ind] == "fen") {
        ind++;
        string new_fen = "";
        for (int fen_tokens = 0; fen_tokens < 6; fen_tokens++) {
            if (ind >= tokens.size()) {
                cout << "FEN STRING invalid and not long enough";
                return;
            }
            new_fen += tokens[ind++] + " ";
        }
        board = Board(new_fen);
    } else {
        return;
    }

    // Parse moves
    if (ind < tokens.size() && tokens[ind] == "moves") {
        ind++;
        while (ind < tokens.size()) {
            Move m = parse_uci_move(tokens[ind++]);
            board.make_move(m);
        }
    }

    if (debug) board.printBoard();
}

// Rough time allocation: spend ~1/20th of the remaining clock plus half the
// increment, but never plan past the flag (small safety margin for overhead).
static int allocate_time_ms(int mytime, int myinc, int movetime) {
    if (movetime > 0) return movetime;   // GUI dictated an exact per-move time
    if (mytime <= 0) return 1000;        // no clock info: default to 1s per move
    int t = mytime / 20 + myinc / 2;
    t = min(t, mytime - 50);
    return max(t, 10);
}

void UCI::go(string cmd) {
    vector<string> tokens = splitString(cmd, ' ');

    int depth = 5;
    int movetime = 0;
    int wtime = 0, btime = 0, winc = 0, binc = 0;
    bool infinite = false;

    for (int i = 1; i < (int)tokens.size(); i++) {
        if (tokens[i] == "perft" && i + 1 < (int)tokens.size()) {
            int perft_depth = stoi(tokens[++i]);
            MoveList moves = generate_moves<ALL_MOVES>(board);
            uint64_t total = 0;
            for (Move m : moves) {
                board.make_move(m);
                uint64_t nodes = perft(board, perft_depth - 1);
                board.unmake_move(m);
                cout << move_to_uci(m) << ": " << nodes << "\n";
                total += nodes;
            }
            cout << "\nNodes searched: " << total << "\n";
            return;
        } 
        else if (tokens[i] == "depth"      && i + 1 < (int)tokens.size()) depth    = stoi(tokens[++i]);
        else if (tokens[i] == "movetime"   && i + 1 < (int)tokens.size()) movetime = stoi(tokens[++i]);
        else if (tokens[i] == "wtime"      && i + 1 < (int)tokens.size()) wtime    = stoi(tokens[++i]);
        else if (tokens[i] == "btime"      && i + 1 < (int)tokens.size()) btime    = stoi(tokens[++i]);
        else if (tokens[i] == "winc"       && i + 1 < (int)tokens.size()) winc     = stoi(tokens[++i]);
        else if (tokens[i] == "binc"       && i + 1 < (int)tokens.size()) binc     = stoi(tokens[++i]);
        else if (tokens[i] == "infinite")   infinite = true;
    }

    stop_search = false;

    MoveList moves = generate_moves<ALL_MOVES>(board);
    if (moves.size == 0) {
        cout << "bestmove (none)\n";
        return;
    }
    
    int mytime = board.isWhiteTurn() ? wtime : btime;
    int myinc  = board.isWhiteTurn() ? winc : binc;

    Move best_move;
    if (infinite) {
        best_move = get_best_move(board, 0);            // no limit: search until "stop"
    } else if (depth > 0 && mytime == 0 && movetime == 0) {
        best_move = get_best_move(board, depth, 0);     // fixed-depth mode (e.g. "go depth 6")
    } else {
        best_move = get_best_move(board, allocate_time_ms(mytime, myinc, movetime));
    }
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

