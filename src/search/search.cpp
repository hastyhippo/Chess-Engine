#include "search.h"
#include "eval.h"
int nodes_searched = 0;
int qnodes_searched = 0;
int negamax(Board &b, int depth, int alpha, int beta, TimePoint time_limit, PVLine &pv) {
    pv.length = 0;

    if (nodes_searched++ % 2048 == 0) {
        if (Clock::now() >= time_limit)
            return OUTOFTIME;
    }

    // repetition or 50-move rule scores as a draw
    if (b.is_repetition() || b.getHalfMoveClock() >= 100)
        return 0;

    if (depth == 0)
        return qsearch(b, alpha, beta, 0);

    MoveList possible_moves = generate_moves<ALL_MOVES>(b);

    if (possible_moves.size == 0)
        return b.in_check() ? -(CHECKMATE + depth) : 0;

    int value = -CHECKMATE - 1;

    update_movelist_evals(b, possible_moves); // For move ordering
    int it = 0;
    while (it < possible_moves.size) { // get it'th best move according to move ordering
        Move m = possible_moves.get_next_move(it++);
        
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

int piece_value(uint8_t piece_type) { 
    if (piece_type == EMPTY_SQ) {
        return 0;
    } else {
        return PIECE_VALUES[piece_type];
    }
}

// Swap algorithm: simulate the capture sequence on to_sq, always recapturing with the least valuable attacker.
int SEE(Board &b, Move &m) {
    static const int val[6] = {100, 300, 300, 500, 900, 20000};
    uint8_t from_sq = m.getFromSq(), to_sq = m.getToSq();
    Colour stm = b.isWhiteTurn() ? WHITE : BLACK;

    uint64_t occ = b.get_occupancy() ^ (1ULL << from_sq);
    int gain[TOTAL_PIECES];
    int d = 0;

    if (m.getMoveFlag() == ENPASSANT) {
        gain[0] = val[PAWN];
        occ ^= 1ULL << (stm == WHITE ? to_sq - 8 : to_sq + 8);
    } else {
        uint8_t victim = b.pieceOn(to_sq);
        gain[0] = victim == EMPTY_SQ ? 0 : val[type_of(victim)];
    }

    int attacker = type_of(b.pieceOn(from_sq));  // piece now standing on to_sq
    Colour side = ~stm;

    // Continually perform recaptures with the least valuable piece for the to_sq
    while (d <= TOTAL_PIECES) {
        uint64_t atk = attackers_of(b, side, to_sq, occ);
        if (!atk) break;

        int t = PAWN;
        uint64_t least = 0;
        for (; t <= KING; t++) {
            least = atk & typed_bb(b, side, t);
            if (least) break;
        }
        // breakpoint: king is last to recapture a piece but can't
        if (t == KING && attackers_of(b, ~side, to_sq, occ)) break;

        d++;
        gain[d] = val[attacker] - gain[d - 1];  // score if this recapture happens
        attacker = t;
        occ ^= least & -least;  // remove the used attacker, may reveal an x-ray
        side = ~side;
    }

    // each side may decline to recapture: negamax the swap list backwards
    while (d) {
        gain[d - 1] = -max(-gain[d - 1], gain[d]);
        d--;
    }
    return gain[0];
}

int qsearch(Board &b, int alpha, int beta, int qdepth) {
    qnodes_searched++;
    bool check = b.in_check();
    int static_eval = 0;

    if (!check) {
        static_eval = evaluate(b);
        if (static_eval >= beta) return beta;
        if (static_eval > alpha) alpha = static_eval;
    }

    // in check: every evasion must be searched, captures alone can't answer a check
    MoveList moves = check ? generate_moves<ALL_MOVES>(b) : generate_moves<CAPTURES>(b);
    if (check && moves.size == 0) {
        return -CHECKMATE + qdepth;
    }

    update_movelist_evals(b, moves);
    int it = 0;
    while (it < moves.size) {
        Move m = moves.get_next_move(it++);

        if (!check) {
            uint8_t victim = b.pieceOn(m.getToSq());
            int vic = m.getMoveFlag() == ENPASSANT ? 100 : 
                        victim != EMPTY_SQ ? PIECE_VALUES[type_of(victim)] : 0;
            if (SEE(b, m) < 0) continue;
        }

        b.make_move(m);
        int score = -qsearch(b, -beta, -alpha, qdepth + 1);
        b.unmake_move(m);

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}

static string score_string(int value, int depth) {
    if (value > MATE_SCORE) {
        int moves_to_mate = (CHECKMATE + depth - value + 1) / 2;
        return "mate " + to_string(moves_to_mate);
    } else if (value < -MATE_SCORE) {
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
        update_movelist_evals(b, moves);

        Move iter_best = moves[0];
        bool out_of_time = false;

        int it = 0;
        while (it < moves.size) {
            Move m = moves.get_next_move(it++);
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

            if (value >= MATE_SCORE) {
                break;  // finds a forced mate
            }
        }

        if (out_of_time) break;

        best_move = iter_best;
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
        
        if (best_value >= MATE_SCORE) {
            break;
        }
    }

    return best_move;
}

Move get_best_move(Board &b, int time_limit_ms) {
    // PVLine caps at 64 plies
    return get_best_move(b, 64, time_limit_ms);
}