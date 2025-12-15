#include "search.h"

constexpr int CHECKMATE = 99999999;
// template<SearchType Type>
int negamax(Board &b, Colour side, int depth) {
    if (depth == 0) {
        int eval = evaluate(b);
        return (side == WHITE) ? eval : -eval;
    }
    
    MoveList possible_moves = generate_moves<ALL_MOVES>(b);
    if (possible_moves.size == 0) {
        
    }

    int value = INT32_MIN;
    for (Move m : possible_moves) {
        b.make_move(m);
        Colour opponent_side = (side == WHITE) ? BLACK : WHITE;
        int score = -negamax(b, opponent_side, depth - 1);
        b.unmake_move(m);
        
        value = max(value, score);
    }
    
    return value;
}

// Explicit template instantiation
// template int negamax<ROOT>(Board &b, Colour side, int depth);
// template int negamax<NODE>(Board &b, Colour side, int depth);

Move get_best_move(Board &b, Colour side, int depth) {
    MoveList possible_moves = generate_moves<ALL_MOVES>(b);
    if (possible_moves.size == 0) {
        return Move();
    }
    
    int best_value = INT32_MIN;
    Move best_move = possible_moves[0];
    
    for (Move m : possible_moves) {
        b.make_move(m);
        Colour opponent_side = (side == WHITE) ? BLACK : WHITE;
        int value = -negamax(b, opponent_side, depth - 1);
        b.unmake_move(m);

        cout << "Move: " << m.getName() << " | " << value << "\n";
        
        if (value > best_value) {
            best_value = value;
            best_move = m;
        }
    }
    
    return best_move;
}
