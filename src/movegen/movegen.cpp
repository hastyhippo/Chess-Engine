#include "movegen.h"
#include "../misc/defines.h"
#include "move.h"
#include <cstdint>
#include <unordered_map>

uint64_t friendly_occ_sq;
uint64_t enemy_occ_sq;
uint64_t all_occ_sq;
uint64_t pinned_pieces;
bool in_check;
unordered_map<int, uint64_t> pinned_map;
int num_attackers;
Colour side;

template <GenType Type>
MoveList generate_moves(Board& b) {
    pinned_pieces = 0ULL;
    side = b.getWhiteTurn() ? WHITE : BLACK;

    friendly_occ_sq = b.getColourPieces(side);
    enemy_occ_sq = b.getColourPieces(~side);
    all_occ_sq = friendly_occ_sq | enemy_occ_sq;
    if (!b.getKingBitboard(side)) {
        b.printBoard();
    }
    assert(b.getKingBitboard(side));
    uint8_t king_sq = _tzcnt_u64(b.getKingBitboard(side));

    // Precalculate for pins
    uint64_t x_ray = (getBishopAttacks(enemy_occ_sq, king_sq) & (b.getBishopBitboard(~side) | b.getQueenBitboard(~side)))  
    | (getRookAttacks(enemy_occ_sq, king_sq) & (b.getRookBitboard(~side) | b.getQueenBitboard(~side)));
        
    while(x_ray) {
        int sniper_sq = pop_lsb(&x_ray);
        uint64_t intersect = ray_between(sniper_sq, king_sq) & friendly_occ_sq;
        if (__popcnt64(intersect) == 1) {
            pinned_pieces |= intersect;
            pinned_map[get_lsb(intersect)] = ray_between(sniper_sq, king_sq);
        }
    }
    MoveList moves{};
    uint64_t attackers = b.attackers_to(king_sq);
    num_attackers = __popcnt64(attackers);
    in_check = num_attackers > 0;
    uint64_t valid_sq = (Type == EVASIONS) ? ray_between(get_lsb(attackers), king_sq) :
                        Type == CAPTURES ? enemy_occ_sq :
                        Type == QUIET ? ~enemy_occ_sq : ~0ULL;

    if (num_attackers == 1) valid_sq = ray_between(get_lsb(attackers), king_sq);
    return generate_all_moves(b, valid_sq);
}

MoveList generate_all_moves(Board &b, uint64_t valid_sq) {
    MoveList moves{};
    addKingMoves(b, moves);
    if (num_attackers >= 2) return moves;

    addPawnMoves<ALL_MOVES>(b, moves, valid_sq);
    addPinnedPawns(b, moves, valid_sq);
    addKnightMoves(b, moves, valid_sq);
    addSlidingMoves(b, moves, valid_sq); 
    return moves;
}

// Use template here because knowing if we calculate captures only can prune out chunks of logic
template<GenType Type>
void addPawnMoves(Board& b, MoveList& moves, uint64_t valid_sq) {
    int8_t forward = side == WHITE ? NORTH : SOUTH;

    uint64_t pawns = b.getPawnBitboard(side) & ~pinned_pieces;
    uint64_t pr_rank = side == WHITE ? RANK_8 : RANK_1; // promotion rank
    uint64_t pr2_rank = side == WHITE ? RANK_7 : RANK_2; // promotion rank

    if constexpr (Type != CAPTURES) {
        uint64_t push1_pawns = shift(pawns, forward) & ~all_occ_sq;
        uint64_t push2_pawns = shift(push1_pawns & (side == WHITE ? RANK_3 : RANK_6), forward) & ~all_occ_sq;
        
        push1_pawns &= valid_sq;
        push2_pawns &= valid_sq;

        while(push1_pawns) {
            uint8_t sq = pop_lsb(&push1_pawns);
            if ((1ULL << sq) & pr_rank) {
                moves.add(Move(sq - forward, sq, PROMOTION_QUEEN));
                moves.add(Move(sq - forward, sq, PROMOTION_ROOK));
                moves.add(Move(sq - forward, sq, PROMOTION_KNIGHT));
                moves.add(Move(sq - forward, sq, PROMOTION_BISHOP));
            } else {
                moves.add(Move(sq - forward, sq, 0));
            }
        }
        while(push2_pawns) {
            uint8_t sq = pop_lsb(&push2_pawns);
            moves.add(Move(sq - 2 * forward, sq, DOUBLE_PUSH));
        }
    }

    if constexpr (Type == CAPTURES || Type == EVASIONS || Type == ALL_MOVES) {
        uint64_t capt_pawns = pawns;
        while(capt_pawns) {
            uint8_t pawn_sq = pop_lsb(&capt_pawns);
            uint64_t attack_bb = pawn_attacks[side][pawn_sq] & enemy_occ_sq & valid_sq;

            while(attack_bb) {
                uint8_t to_sq = pop_lsb(&attack_bb);
                if (pr2_rank & (1ULL << pawn_sq)) {
                    moves.add(Move(pawn_sq, to_sq, PROMOTION_QUEEN));
                    moves.add(Move(pawn_sq, to_sq, PROMOTION_ROOK));
                    moves.add(Move(pawn_sq, to_sq, PROMOTION_KNIGHT));
                    moves.add(Move(pawn_sq, to_sq, PROMOTION_BISHOP));
                } else {
                    moves.add(Move(pawn_sq, to_sq, 0));
                }
            }
        }
    
        // ENPASSANT
        int enp_file = b.getEnpassantFile();
        if (enp_file != NO_ENP) {
            uint64_t can_enp = (enp_file != 0) ? get_file[enp_file + WEST] : 0;
            can_enp |= (enp_file != N - 1) ? get_file[enp_file + EAST] : 0;
            can_enp &= (side == WHITE ? RANK_5 : RANK_4) & pawns;
            int enp_sq = enp_file + 8 * (side == WHITE ? 5 : 2);
    
            uint64_t king_bb = b.getKingBitboard(side);
            if (!king_bb) return;
            uint8_t king_sq = get_lsb(king_bb);
            
            while (can_enp) {
                uint8_t from_sq = pop_lsb(&can_enp);
                uint8_t to_sq = enp_sq;
                uint8_t captured_pawn_sq = enp_sq - forward;
                uint64_t modified_occ = all_occ_sq;
                modified_occ ^= (1ULL << from_sq);
                modified_occ ^= (1ULL << captured_pawn_sq);
                modified_occ |= (1ULL << to_sq);
                if (!b.attackers_to(king_sq, 1ULL << captured_pawn_sq)) {
                    moves.add(Move(from_sq, to_sq, ENPASSANT));
                }
            }
        }
    }
}

void addPinnedPawns(Board& b, MoveList& moves, uint64_t valid_sq) {
    int forward = side == WHITE ? NORTH : SOUTH;
    uint64_t pinned_pawns = b.getPawnBitboard(side) & pinned_pieces;
    uint64_t pr_rank = side == WHITE ? RANK_8 : RANK_1; // promotion rank
    uint64_t pr2_rank = side == WHITE ? RANK_7 : RANK_2; // promotion rank
    while(pinned_pawns) {
        uint8_t from_sq = pop_lsb(&pinned_pawns);
        uint64_t allowed_sq = pinned_map[from_sq];
        uint64_t second_rank = side == WHITE ? RANK_3 : RANK_6;
        // Single push
        uint64_t push1 = (1ULL << (from_sq + forward)) & ~all_occ_sq & allowed_sq & valid_sq;
        uint64_t push2 = (shift(push1 & second_rank, forward)) & ~all_occ_sq & allowed_sq & valid_sq;
        if (push1) {
            uint8_t to_sq = pop_lsb(&push1);
            if ((1ULL << to_sq) & pr_rank) {
                moves.add(Move(from_sq, to_sq, PROMOTION_QUEEN));
                moves.add(Move(from_sq, to_sq, PROMOTION_ROOK));
                moves.add(Move(from_sq, to_sq, PROMOTION_KNIGHT));
                moves.add(Move(from_sq, to_sq, PROMOTION_BISHOP));
            } else {
                moves.add(Move(from_sq, to_sq, 0));
            }
        }
        if (push2) {
            uint8_t to_sq = pop_lsb(&push2);
            moves.add(Move(from_sq, to_sq, DOUBLE_PUSH));
        }
        uint64_t capt_sq = pawn_attacks[side][from_sq] & enemy_occ_sq & allowed_sq & valid_sq;
        while(capt_sq) {
            uint8_t to_sq = pop_lsb(&capt_sq);
            if ((1ULL << to_sq) & pr_rank) {
                moves.add(Move(from_sq, to_sq, PROMOTION_QUEEN));
                moves.add(Move(from_sq, to_sq, PROMOTION_ROOK));
                moves.add(Move(from_sq, to_sq, PROMOTION_KNIGHT));
                moves.add(Move(from_sq, to_sq, PROMOTION_BISHOP));
            } else {
                moves.add(Move(from_sq, to_sq, 0));
            }
        }
    }
}


void addKnightMoves(Board& b, MoveList& moves, uint64_t valid_sq) {
    uint64_t knights = b.getKnightBitboard(side) & ~pinned_pieces;
    while(knights) {
        uint8_t sq = pop_lsb(&knights);
        uint64_t knight_sq = knight_moves[sq] & ~friendly_occ_sq & valid_sq;
        while(knight_sq) {
            uint8_t target_sq = pop_lsb(&knight_sq);
            moves.add(Move(sq, target_sq, 0));
        }
    }
}

void addSlidingMoves(Board& b, MoveList& moves, uint64_t valid_sq) {
    uint64_t bishops = b.getBishopBitboard(side);
    uint64_t rooks = b.getRookBitboard(side);
    uint64_t queens = b.getQueenBitboard(side);

    while(bishops) {
        uint8_t from_sq = pop_lsb(&bishops);
        uint64_t attacked_sq = getBishopAttacks(all_occ_sq, from_sq) & ~friendly_occ_sq & valid_sq;
        if ((1ULL<<from_sq) & pinned_pieces) attacked_sq &= pinned_map[from_sq];

        while(attacked_sq) {
            uint8_t to_sq = pop_lsb(&attacked_sq);
            moves.add(Move(from_sq, to_sq, 0));
        }
    }

    while(rooks) {
        uint8_t from_sq = pop_lsb(&rooks);
        uint64_t attacked_sq = getRookAttacks(all_occ_sq, from_sq) & ~friendly_occ_sq & valid_sq;
        if ((1ULL<<from_sq) & pinned_pieces) attacked_sq &= pinned_map[from_sq];

        while(attacked_sq) {
            uint8_t to_sq = pop_lsb(&attacked_sq);
            moves.add(Move(from_sq, to_sq, 0));
        }
    }

    while(queens) {
        uint8_t from_sq = pop_lsb(&queens);
        uint64_t attacked_sq = (getRookAttacks(all_occ_sq, from_sq) 
                            | getBishopAttacks(all_occ_sq, from_sq)) & ~friendly_occ_sq & valid_sq;
        if ((1ULL<<from_sq) & pinned_pieces) attacked_sq &= pinned_map[from_sq];

        while(attacked_sq) {
            uint8_t to_sq = pop_lsb(&attacked_sq);
            moves.add(Move(from_sq, to_sq, 0));
        }
    }
}


void addKingMoves(Board& b, MoveList& moves) {
    uint64_t king = b.getKingBitboard(side);
    assert(king);
    if (!king) return;

    uint8_t sq = pop_lsb(&king);
    uint64_t king_sq = king_moves[sq] & ~friendly_occ_sq;

    while(king_sq) {
        uint8_t target_sq = pop_lsb(&king_sq);
        if (b.attackers_to(target_sq, 0ULL, all_occ_sq ^ (1ULL << sq))) continue;
        moves.add(Move(sq, target_sq, 0));
    }
    if (in_check) return;
    //Castling
    for (int i = 0; i < 2; i++) {
        if ((b.getCastlingRights() & (1ULL << (i + (side == WHITE ? 0 : 2)))) != 0 && (all_occ_sq & castling_clear_sq[side][i]) == 0) {
            uint64_t cleared_sq = castling_unattacked_sq[side][i];
            bool can_castle = true;
            while(cleared_sq) {
                uint8_t clear_sq = pop_lsb(&cleared_sq);
                can_castle = !b.attackers_to(clear_sq);
                if (!can_castle) break;
            }
            if (can_castle) {
                moves.add(Move(sq, castling_target_sq[side][i], CASTLE));
            }
        }
    }
}

// return a ray that is always inclusive of sq_1 and ends 1 square before sq_2
uint64_t ray_between(int sq_1, int sq_2) {
    return ray_between_table[sq_1][sq_2];
}

template MoveList generate_moves<ALL_MOVES>(Board& b);
template MoveList generate_moves<CAPTURES>(Board& b);

template void addPawnMoves<ALL_MOVES>(Board& b, MoveList& moves, uint64_t valid_sq);
template void addPawnMoves<CAPTURES>(Board& b, MoveList& moves, uint64_t valid_sq);
template void addPawnMoves<QUIET>(Board& b, MoveList& moves, uint64_t valid_sq);
template void addPawnMoves<EVASIONS>(Board& b, MoveList& moves, uint64_t valid_sq);
template void addPawnMoves<PINNED>(Board& b, MoveList& moves, uint64_t valid_sq);
