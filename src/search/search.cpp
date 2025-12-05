#include "search.h";
#include "eval.h";

int minimax(Board &b, bool maximising, int depth) {
    if (depth == 0) {
        return evaluate(b);
    }

    MoveList possible_moves = generate_moves<ALL_MOVES>(b);
    if (maximising) {
        int value = INT32_MIN;
        for (Move m : possible_moves) {
            b.make_move(m);
            value = max(value, minimax(b, false, depth - 1));
            b.unmake_move(m);
        }
        return value;
    } else {
        int value = INT32_MAX;
        for (Move m : possible_moves) {
            b.make_move(m);
            value = min(value, minimax(b, true, depth - 1));
            b.unmake_move(m);
        }
    }
}