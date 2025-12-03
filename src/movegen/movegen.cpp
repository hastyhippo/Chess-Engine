#include "movegen.h"
#include "../misc/defines.h"
#include "move.h"
#include <cstdint>

uint64_t friendly_occ_sq;
uint64_t enemy_occ_sq;
uint64_t all_occ_sq;
uint64_t pinned_pieces;
bool in_check;
bool white;

template <GenType Type>
vector<Move> generateMoves(Board& b) {
    white = b.getWhiteTurn();
    friendly_occ_sq = b.getColourPieces(b.getWhiteTurn());
    enemy_occ_sq = b.getColourPieces(!b.getWhiteTurn());
    all_occ_sq = friendly_occ_sq | enemy_occ_sq;
    uint64_t pinned_pieces = 0ULL;
    uint8_t king_sq = _tzcnt_u64(b.getPieceBitboard(W_KING, white));

    // Precalculate for pins
    uint64_t x_ray = (getBishopAttacks(enemy_occ_sq, king_sq) & (b.getPieceBitboard(W_BISHOP, !white) | b.getPieceBitboard(W_QUEEN, !white)))  
    | (getRookAttacks(enemy_occ_sq, king_sq) & (b.getPieceBitboard(W_ROOK, !white) | b.getPieceBitboard(W_QUEEN, !white)));
        
    while(x_ray) {
        uint64_t intersect = ray_between(king_sq, pop_lsb(&x_ray)) & friendly_occ_sq;
        if (__popcnt64(intersect) == 1) pinned_pieces |= intersect;
    }

    vector<Move> moves;
    uint64_t attackers = attackers_to(!white, king_sq, all_occ_sq, b);
    int num_attackers = __popcnt64(attackers);
    in_check = num_attackers > 0;
    uint64_t valid_sq = (Type == EVASIONS) ? ray_between(king_sq, _tzcnt_u64(attackers)) :
                        Type == CAPTURES ? enemy_occ_sq :
                        Type == QUIET ? ~enemy_occ_sq : ~0ULL;

    if (num_attackers == 1) valid_sq = ray_between(king_sq, _tzcnt_u64(attackers));
    if (num_attackers <= 1) {
        if (in_check) addPawnMoves<EVASIONS>(b, moves, valid_sq);
        else addPawnMoves<Type>(b, moves, valid_sq);
        addKnightMoves(b, moves, valid_sq);
        addSlidingMoves(b, moves, valid_sq); 
    }

    addKingMoves(b, moves);

    vector<Move> new_moves;
    for (Move m: moves) {
        uint64_t from_sq = 1ULL << m.getFromSq();
        if (((pinned_pieces & from_sq) || (m.getMoveFlag() == ENPASSANT)) && square_attacked_after_move(!white, king_sq, m, all_occ_sq, b)) {
            // cout <<  "pruning: " << m.getName() << "\n";
            continue;
        }
        new_moves.push_back(m);
    }

    return new_moves;
}

bool square_attacked_after_move(bool attacker_colour, uint8_t sq, Move m, uint64_t base_occupancy, Board& b) {
    uint64_t from_sq = 1ULL << m.getFromSq();
    uint64_t to_sq = 1ULL << m.getToSq();
    uint64_t modified_occ = base_occupancy ^ from_sq | to_sq;

    if (m.getMoveFlag() == ENPASSANT) {
        uint64_t captured_pawn = shift(to_sq,(white ? SOUTH : NORTH));
        modified_occ ^= captured_pawn;
        to_sq |= captured_pawn;
    }

    return has_attackers(attacker_colour, sq, modified_occ, to_sq, b) != 0;
}


// Use template here because knowing if we calculate captures only can prune out chunks of logic
template<GenType Type>
void addPawnMoves(Board& b, vector<Move>& moves, uint64_t valid_sq) {
    int forward = white ? NORTH : SOUTH;

    uint64_t pawns = b.getPieceBitboard(W_PAWN, white);
    uint64_t pr_rank = white ? RANK_8 : RANK_1; // promotion rank
    uint64_t pr2_rank = white ? RANK_7 : RANK_2; // promotion rank

    if constexpr (Type != CAPTURES) {
        uint64_t push1_pawns = shift(pawns, forward) & ~all_occ_sq;
        uint64_t push2_pawns = shift(push1_pawns & (white ? RANK_3 : RANK_6), forward) & ~all_occ_sq;
        uint64_t pr_pawns = push1_pawns & pr_rank;
        
        push1_pawns ^= pr_pawns; // handle promoting pawns seperately from normal pushes

        if constexpr (Type == EVASIONS) {
            push1_pawns &= valid_sq;
            push2_pawns &= valid_sq;
            pr_pawns &= valid_sq;
        }
        while(push1_pawns) {
            uint8_t sq = pop_lsb(&push1_pawns);
            moves.push_back(Move(sq - forward, sq, 0, false));
        }
        while(push2_pawns) {
            uint8_t sq = pop_lsb(&push2_pawns);
            moves.push_back(Move(sq - 2 * forward, sq, DOUBLE_PUSH, false));
        }
        while(pr_pawns) {
            uint8_t sq = pop_lsb(&pr_pawns);
            uint8_t from_sq = sq - forward;
            moves.push_back(Move(from_sq, sq, PROMOTION_QUEEN, false));
            moves.push_back(Move(from_sq, sq, PROMOTION_ROOK, false));
            moves.push_back(Move(from_sq, sq, PROMOTION_KNIGHT, false));
            moves.push_back(Move(from_sq, sq, PROMOTION_BISHOP, false));
        }
    }

    if constexpr (Type == CAPTURES || Type == EVASIONS || Type == ALL_MOVES) {
        uint64_t capt_pawns = pawns & ~pr2_rank;
        while(capt_pawns) {
            uint8_t pawn_sq = pop_lsb(&capt_pawns);
            uint64_t attack_bb = pawn_attacks[1 - white][pawn_sq] & enemy_occ_sq;
            if constexpr (Type == EVASIONS) attack_bb &= valid_sq;

            while(attack_bb) {
                uint8_t to_sq = pop_lsb(&attack_bb);
                moves.push_back(Move(pawn_sq, to_sq, 0, true));
            }
        }
        uint64_t pr_capt_pawns = pawns & pr2_rank;
        while(pr_capt_pawns) {
            uint8_t pawn_sq = pop_lsb(&pr_capt_pawns);
            uint64_t attack_bb = pawn_attacks[1 - white][pawn_sq] & enemy_occ_sq;
            if constexpr (Type == EVASIONS) attack_bb &= valid_sq;

            while(attack_bb) {
                uint8_t to_sq = pop_lsb(&attack_bb);
                moves.push_back(Move(pawn_sq, to_sq, PROMOTION_QUEEN, true));
                moves.push_back(Move(pawn_sq, to_sq, PROMOTION_ROOK, true));
                moves.push_back(Move(pawn_sq, to_sq, PROMOTION_KNIGHT, true));
                moves.push_back(Move(pawn_sq, to_sq, PROMOTION_BISHOP, true));
            }
        }
    
        // ENPASSANT
        int enp_sq = b.getEnpassantSquare();
        if (enp_sq != NO_ENP) {
            uint64_t can_enp = (enp_sq != 0) ? get_file[enp_sq + WEST] : 0;
            can_enp |= (enp_sq != N - 1) ? get_file[enp_sq + EAST] : 0;
    
            can_enp &= (white ? RANK_5 : RANK_4) & pawns;
    
            int target_sq = enp_sq + N * (white ? 5 : 2); // set the enpassant square to the correct rank
            while (can_enp) {
                uint8_t sq = pop_lsb(&can_enp);
                moves.push_back(Move(sq, target_sq, ENPASSANT, true));
            }
        }
    }
}


void addKnightMoves(Board& b, vector<Move>& moves, uint64_t valid_sq) {
    uint64_t knights = b.getPieceBitboard(W_KNIGHT, b.getWhiteTurn());
    while(knights) {
        uint8_t sq = pop_lsb(&knights);
        uint64_t knight_sq = knight_moves[sq] & ~friendly_occ_sq & valid_sq;
        while(knight_sq) {
            uint8_t target_sq = pop_lsb(&knight_sq);
            moves.push_back(Move(sq, target_sq, 0, ((1ULL << target_sq) & enemy_occ_sq) != 0));
        }
    }
}

uint64_t getBishopAttacks(uint64_t occ, int sq) {
    occ     &= m_bishop_tbl[sq].mask;
    occ     *= m_bishop_tbl[sq].magic;
    occ     >>= m_bishop_tbl[sq].shift;
    return m_bishop_tbl[sq].ptr[occ];
}

uint64_t getRookAttacks(uint64_t occ, int sq) {
    occ     &= m_rook_tbl[sq].mask;
    occ     *= m_rook_tbl[sq].magic;
    occ     >>= m_rook_tbl[sq].shift;
    return m_rook_tbl[sq].ptr[occ];
}

void addSlidingMoves(Board& b, vector<Move>& moves, uint64_t valid_sq) {
    uint64_t bishops = b.getPieceBitboard(W_BISHOP, white);
    uint64_t rooks = b.getPieceBitboard(W_ROOK, white);
    uint64_t queens = b.getPieceBitboard(W_QUEEN, white);

    while(bishops) {
        uint8_t from_sq = pop_lsb(&bishops);
        uint64_t attacked_sq = getBishopAttacks(all_occ_sq, from_sq) & ~friendly_occ_sq & valid_sq;
        while(attacked_sq) {
            uint8_t to_sq = pop_lsb(&attacked_sq);
            moves.push_back(Move(from_sq, to_sq, 0, (1ULL << to_sq) & enemy_occ_sq));
        }
    }

    while(rooks) {
        uint8_t from_sq = pop_lsb(&rooks);
        uint64_t attacked_sq = getRookAttacks(all_occ_sq, from_sq) & ~friendly_occ_sq & valid_sq;
        while(attacked_sq) {
            uint8_t to_sq = pop_lsb(&attacked_sq);
            moves.push_back(Move(from_sq, to_sq, 0, (1ULL << to_sq) & enemy_occ_sq));
        }
    }

    while(queens) {
        uint8_t from_sq = pop_lsb(&queens);
        uint64_t attacked_sq = (getRookAttacks(all_occ_sq, from_sq) 
                            | getBishopAttacks(all_occ_sq, from_sq)) & ~friendly_occ_sq & valid_sq;
        while(attacked_sq) {
            uint8_t to_sq = pop_lsb(&attacked_sq);
            moves.push_back(Move(from_sq, to_sq, 0, (1ULL << to_sq) & enemy_occ_sq));
        }
    }
}


void addKingMoves(Board& b, vector<Move>& moves) {
    uint64_t king = b.getPieceBitboard(W_KING, white);
    if (!king) return;

    uint8_t sq = pop_lsb(&king);
    uint64_t king_sq = king_moves[sq] & ~friendly_occ_sq;

    while(king_sq) {
        uint8_t target_sq = pop_lsb(&king_sq);
        if (attackers_to(!white, target_sq, all_occ_sq ^ (1ULL << sq), b)) continue;
        moves.push_back(Move(sq, target_sq, 0, ((1ULL << target_sq) & enemy_occ_sq) != 0));
    }
    if (in_check) return;
    //Castling
    for (int i = 0; i < 2; i++) {
        if ((b.getCastlingRights() & (1ULL << (i + (white ? 0 : 2)))) != 0 && (all_occ_sq & castling_clear_sq[1 - white][i]) == 0) {
            uint64_t cleared_sq = castling_unattacked_sq[1 - white][i];
            bool can_castle = true;
            while(cleared_sq) {
                uint8_t clear_sq = pop_lsb(&cleared_sq);
                can_castle = !has_attackers(!white, clear_sq, all_occ_sq, b);
                if (!can_castle) break;
            }
            if (can_castle) {
                moves.push_back(Move(sq, castling_target_sq[1 - white][i], CASTLE, false));
            }
        }
    }
}

uint64_t attackers_to(bool colour, uint8_t sq, uint64_t occ, Board& b) { // attacker colour
    return  (pawn_attacks[colour][sq] & b.getPieceBitboard(W_PAWN, colour)) |
            (knight_moves[sq] & b.getPieceBitboard(W_KNIGHT, colour)) |
            (getBishopAttacks(occ, sq) & (b.getPieceBitboard(W_BISHOP, colour) | b.getPieceBitboard(W_QUEEN, colour))) | 
            (getRookAttacks(occ, sq) & (b.getPieceBitboard(W_ROOK, colour) | b.getPieceBitboard(W_QUEEN, colour))) | 
            (king_moves[sq] & b.getPieceBitboard(W_KING, colour));
}

uint64_t has_attackers(bool colour, uint8_t sq, uint64_t occ, Board& b) { // attacker colour
    return  ((pawn_attacks[colour][sq] & b.getPieceBitboard(W_PAWN, colour)) ||
              (knight_moves[sq] & b.getPieceBitboard(W_KNIGHT, colour)) ||
              (getBishopAttacks(occ, sq) & (b.getPieceBitboard(W_BISHOP, colour) | b.getPieceBitboard(W_QUEEN, colour))) ||
              (getRookAttacks(occ, sq) & (b.getPieceBitboard(W_ROOK, colour) | b.getPieceBitboard(W_QUEEN, colour))) ||
             (king_moves[sq] & b.getPieceBitboard(W_KING, colour)));
}

uint64_t has_attackers(bool colour, uint8_t sq, uint64_t occ, uint64_t ignore_sq, Board& b) { // attacker colour
    return  ((pawn_attacks[colour][sq] & b.getPieceBitboard(W_PAWN, colour) & ~ignore_sq) ||
              (knight_moves[sq] & b.getPieceBitboard(W_KNIGHT, colour) & ~ignore_sq) ||
              (getBishopAttacks(occ, sq) & (b.getPieceBitboard(W_BISHOP, colour) | b.getPieceBitboard(W_QUEEN, colour)) & ~ignore_sq) ||
              (getRookAttacks(occ, sq) & (b.getPieceBitboard(W_ROOK, colour) | b.getPieceBitboard(W_QUEEN, colour)) & ~ignore_sq) ||
             (king_moves[sq] & b.getPieceBitboard(W_KING, colour)));
}

uint64_t ray_between(int sq_1, int sq_2) {
    return ray_between_table[sq_2][sq_1];
}

template vector<Move> generateMoves<ALL_MOVES>(Board& b);
template vector<Move> generateMoves<CAPTURES>(Board& b);
template vector<Move> generateMoves<QUIET>(Board& b);
template vector<Move> generateMoves<EVASIONS>(Board& b);

template void addPawnMoves<ALL_MOVES>(Board& b, vector<Move>& moves, uint64_t valid_sq);
template void addPawnMoves<CAPTURES>(Board& b, vector<Move>& moves, uint64_t valid_sq);
template void addPawnMoves<QUIET>(Board& b, vector<Move>& moves, uint64_t valid_sq);
template void addPawnMoves<EVASIONS>(Board& b, vector<Move>& moves, uint64_t valid_sq);