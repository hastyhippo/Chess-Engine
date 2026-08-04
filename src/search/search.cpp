#include "search.h"

int nodes_searched = 0;

int negamax(Board &b, int depth, int alpha, int beta, TimePoint time_limit, PVLine &pv) {
    pv.length = 0;

    if (nodes_searched++ % 2048 == 0) {
        if (Clock::now() >= time_limit)
            return OUTOFTIME;
    }

    MoveList possible_moves = generate_moves<ALL_MOVES>(b);

    if (possible_moves.size == 0)
        return b.in_check() ? -(CHECKMATE + depth) : 0;

    if (depth == 0)
        return evaluate(b);

    int value = -CHECKMATE - 1;
    for (Move m : possible_moves) {
        PVLine child_pv;
        b.make_move(m);
        int res = negamax(b, depth - 1, -beta, -alpha, time_limit, child_pv);
        b.unmake_move(m);

        if (res == OUTOFTIME) return OUTOFTIME;
        int score = -res;

        if (score > value) {
            value = score;
            pv.moves[0] = m;
            memcpy(pv.moves + 1, child_pv.moves, child_pv.length * sizeof(Move));
            pv.length = child_pv.length + 1;
        }

        alpha = max(alpha, score);
        if (alpha >= beta)
            break;
    }
    return value;
}

static string score_string(int value, int depth) {
    if (value > CHECKMATE - 1000) {
        int moves_to_mate = (CHECKMATE + depth - value + 1) / 2;
        return "mate " + to_string(moves_to_mate);
    } else if (value < -(CHECKMATE - 1000)) {
        int moves_to_mate = (CHECKMATE + depth + value + 1) / 2;
        return "mate -" + to_string(moves_to_mate);
    }
    return "cp " + to_string(value);
}

Move get_best_move(Board &b, int depth, int time_limit_ms) {
    if (generate_moves<ALL_MOVES>(b).size == 0) return Move();

    auto time_limit = time_limit_ms > 0
        ? Clock::now() + std::chrono::milliseconds(time_limit_ms - 50)
        : TimePoint::max();

    Move best_move;
    nodes_searched = 0;

    for (int d = 1; d <= depth; d++) {
        PVLine pv;
        auto start = Clock::now();

        int best_value = -CHECKMATE - 1;
        MoveList moves = generate_moves<ALL_MOVES>(b);
        Move iter_best = moves[0];
        bool out_of_time = false;

        for (Move m : moves) {
            PVLine child_pv;
            b.make_move(m);
            int res = negamax(b, d - 1, -INF, INF, time_limit, child_pv);
            b.unmake_move(m);

            if (res == OUTOFTIME) { out_of_time = true; break; }
            int value = -res;

            if (value > best_value) {
                best_value = value;
                iter_best = m;
                pv.moves[0] = m;
                memcpy(pv.moves + 1, child_pv.moves, child_pv.length * sizeof(Move));
                pv.length = child_pv.length + 1;
            }
        }

        if (out_of_time) break;

        best_move = iter_best;

        {
            double elapsed = duration<double, milli>(Clock::now() - start).count();
            cout << "info depth " << d
                 << " score " << score_string(best_value, d)
                 << " nodes " << nodes_searched
                 << " time " << (int)elapsed
                 << " pv";
            for (int i = 0; i < pv.length; i++)
                cout << " " << pv.moves[i].getName();
            cout << "\n";
            cout.flush();
        }
    }

    return best_move;
}

Move get_best_move(Board &b, int time_limit_ms) {
    MoveList possible_moves = generate_moves<ALL_MOVES>(b);
    if (possible_moves.size == 0) {
        return Move();
    }

    auto start = Clock::now();
    auto time_limit =  time_limit_ms > 0 
        ? Clock::now() + std::chrono::milliseconds(time_limit_ms)
        : TimePoint::max();
    
    Move best_move = possible_moves[0];
    vector<pair<Move,int>> best_action(0);

    int highest_full_depth = 0;
    for (int depth = 0;; depth++) {
        int best_value = INT32_MIN;
        int value = 0;
        for (Move m : possible_moves) {
            b.make_move(m);
            value = negamax(b, depth, time_limit);
            if (value != OUTOFTIME) value = -value;
            b.unmake_move(m);
            
            if (value == OUTOFTIME) {
                break;
            }
            
            if (value > best_value) {
                best_value = value;
                best_move = m;
            }
        }
        if (value == OUTOFTIME) {
            break;
        }
        highest_full_depth = max(depth, highest_full_depth);
        best_action.push_back(make_pair(best_move, best_value));

        cout << "info depth " << depth << " score cp " << best_value << "\n";
    }

    if (best_action.size() == 0) {
        return Move();
    } else {
        return best_action.back().first;
    }
}