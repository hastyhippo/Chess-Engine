#include "search.h"

int nodes_searched = 0;

// template<SearchType Type>
int negamax(Board &b, int depth, TimePoint time_limit) {
    if (nodes_searched++ % 2048 == 0) {
        if (Clock::now() >= time_limit){
            return OUTOFTIME;
        }
    }
    Colour side = b.getWhiteTurn() ? WHITE : BLACK;
    MoveList possible_moves = generate_moves<ALL_MOVES>(b);
    //  if depth = 0 or node is a terminal node then        
    //      return color × the heuristic value of node

    if (possible_moves.size == 0) {
        if (b.in_check()) {
            return -(CHECKMATE + depth);
        } else {
            return 0;
        }
    }
    if (depth == 0) {
        return evaluate(b);
    }
    
//    value := −∞
//    for each child of node do
//    value := max(value, −negamax(child, depth − 1, −color))

    int value = - CHECKMATE - 1;
    for (Move m : possible_moves) {
        b.make_move(m);
        value = max(value, -negamax(b, depth - 1, time_limit));
        b.unmake_move(m);
    }
    
    return value;
}

string get_value_string(int value, int depth) {
    if (value > CHECKMATE - 1000) {
        int moves_to_mate = (CHECKMATE + depth - value + 1) / 2;
        return "mate in " + to_string(moves_to_mate);
    } else if (value < -(CHECKMATE - 1000)) {
        int moves_to_mate = (CHECKMATE + depth + value + 1) / 2;
        return "mated in " + to_string(moves_to_mate);
    } else {
        return to_string(value);
    }
}

Move get_best_move(Board &b, int depth, int time_limit_ms) {
    MoveList possible_moves = generate_moves<ALL_MOVES>(b);
    if (possible_moves.size == 0) {
        return Move();
    }

    auto start = Clock::now();
    auto time_limit =  time_limit_ms > 0 
        ? Clock::now() + std::chrono::milliseconds(time_limit_ms)
        : TimePoint::max();
    
    int best_value = INT32_MIN;
    Move best_move = possible_moves[0];
    
    for (Move m : possible_moves) {
        b.make_move(m);
        int value = -negamax(b, depth - 1, time_limit);
        b.unmake_move(m);
        if (value == OUTOFTIME) {
            cout << "Search timed out!";
            return Move();
        }

        cout << "Move: " << m.getName() << " | " << get_value_string(value, depth) << "\n";
        
        if (value > best_value) {
            best_value = value;
            best_move = m;
        }
    }

    cout << "Evaluation: " << get_value_string(best_value, depth) << "\n";
    return best_move;
}
