#include "movegen.h"
#include "../misc/defines.h"
#include "move.h"
#include <cstdint>

struct MovegenState {
    uint64_t friendly_occ_sq;
    uint64_t enemy_occ_sq;
    uint64_t all_occ_sq;
    uint64_t pinned_pieces;
    uint64_t pinned_ray[64];
    bool in_check;
    int num_attackers;
    Colour side;
};

template <GenType Type>
MoveList generate_moves(Board& b) {
    MovegenState s{};
    s.side = b.getWhiteTurn() ? WHITE : BLACK;
    s.friendly_occ_sq = b.getColourPieces(s.side);
    s.enemy_occ_sq = b.getColourPieces(~s.side);
    s.all_occ_sq = s.friendly_occ_sq | s.enemy_occ_sq;

    assert(b.getKingBitboard(s.side));
    uint8_t king_sq = _tzcnt_u64(b.getKingBitboard(s.side));

    uint64_t x_ray = (get_bishop_attacks(s.enemy_occ_sq, king_sq) & (b.getBishopBitboard(~s.side) | b.getQueenBitboard(~s.side)))
                   | (get_rook_attacks(s.enemy_occ_sq, king_sq) & (b.getRookBitboard(~s.side) | b.getQueenBitboard(~s.side)));

    while (x_ray) {
        int sniper_sq = pop_lsb(&x_ray);
        uint64_t intersect = ray_between(sniper_sq, king_sq) & s.friendly_occ_sq;
        if (__popcnt64(intersect) == 1) {
            int pinned_sq = get_lsb(intersect);
            s.pinned_pieces |= intersect;
            s.pinned_ray[pinned_sq] = ray_between(sniper_sq, king_sq);
        }
    }

    uint64_t attackers = b.attackers_to(king_sq, 0ULL, s.all_occ_sq);
    s.num_attackers = __popcnt64(attackers);
    s.in_check = s.num_attackers > 0;

    uint64_t valid_sq = (Type == EVASIONS) ? ray_between(get_lsb(attackers), king_sq) :
                        (Type == CAPTURES)  ? s.enemy_occ_sq :
                        (Type == QUIET)     ? ~s.enemy_occ_sq : ~0ULL;

    if (s.num_attackers == 1) valid_sq = ray_between(get_lsb(attackers), king_sq);
    return generate_all_moves(b, valid_sq, s);
}

MoveList generate_all_moves(Board &b, uint64_t valid_sq, MovegenState &s) {
    MoveList moves{};
    addKingMoves(b, moves, s);
    if (s.num_attackers >= 2) return moves;

    addPawnMoves<ALL_MOVES>(b, moves, valid_sq, s);
    addPinnedPawns(b, moves, valid_sq, s);
    addKnightMoves(b, moves, valid_sq, s);
    addSlidingMoves(b, moves, valid_sq, s);
    return moves;
}

template<GenType Type>
void addPawnMoves(Board& b, MoveList& moves, uint64_t valid_sq, MovegenState &s) {
    int8_t forward = s.side == WHITE ? NORTH : SOUTH;

    uint64_t pawns = b.getPawnBitboard(s.side) & ~s.pinned_pieces;
    uint64_t pr_rank  = s.side == WHITE ? RANK_8 : RANK_1;
    uint64_t pr2_rank = s.side == WHITE ? RANK_7 : RANK_2;

    if constexpr (Type != CAPTURES) {
        uint64_t push1_pawns = shift(pawns, forward) & ~s.all_occ_sq;
        uint64_t push2_pawns = shift(push1_pawns & (s.side == WHITE ? RANK_3 : RANK_6), forward) & ~s.all_occ_sq;

        push1_pawns &= valid_sq;
        push2_pawns &= valid_sq;

        while (push1_pawns) {
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
        while (push2_pawns) {
            uint8_t sq = pop_lsb(&push2_pawns);
            moves.add(Move(sq - 2 * forward, sq, DOUBLE_PUSH));
        }
    }

    if constexpr (Type == CAPTURES || Type == EVASIONS || Type == ALL_MOVES) {
        uint64_t capt_pawns = pawns;
        while (capt_pawns) {
            uint8_t pawn_sq = pop_lsb(&capt_pawns);
            uint64_t attack_bb = pawn_attacks[s.side][pawn_sq] & s.enemy_occ_sq & valid_sq;

            while (attack_bb) {
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

        int enp_file = b.getEnpassantFile();
        if (enp_file != NO_ENP) {
            uint64_t can_enp = (enp_file != 0)     ? get_file[enp_file + WEST] : 0;
            can_enp          |= (enp_file != N - 1) ? get_file[enp_file + EAST] : 0;
            can_enp &= (s.side == WHITE ? RANK_5 : RANK_4) & pawns;
            int enp_sq = enp_file + 8 * (s.side == WHITE ? 5 : 2);

            uint64_t king_bb = b.getKingBitboard(s.side);
            if (!king_bb) return;
            uint8_t king_sq = get_lsb(king_bb);

            while (can_enp) {
                uint8_t from_sq = pop_lsb(&can_enp);
                uint8_t to_sq = enp_sq;
                uint8_t captured_pawn_sq = enp_sq - forward;
                uint64_t modified_occ = s.all_occ_sq;
                modified_occ ^= (1ULL << from_sq);
                modified_occ ^= (1ULL << captured_pawn_sq);
                modified_occ |= (1ULL << to_sq);
                if (!b.attackers_to(king_sq, 1ULL << captured_pawn_sq, modified_occ)) {
                    moves.add(Move(from_sq, to_sq, ENPASSANT));
                }
            }
        }
    }
}

void addPinnedPawns(Board& b, MoveList& moves, uint64_t valid_sq, MovegenState &s) {
    int forward = s.side == WHITE ? NORTH : SOUTH;
    uint64_t pinned_pawns = b.getPawnBitboard(s.side) & s.pinned_pieces;
    uint64_t pr_rank  = s.side == WHITE ? RANK_8 : RANK_1;
    uint64_t second_rank = s.side == WHITE ? RANK_3 : RANK_6;

    while (pinned_pawns) {
        uint8_t from_sq = pop_lsb(&pinned_pawns);
        uint64_t allowed_sq = s.pinned_ray[from_sq];

        uint64_t push1 = (1ULL << (from_sq + forward)) & ~s.all_occ_sq & allowed_sq & valid_sq;
        uint64_t push2 = shift(push1 & second_rank, forward) & ~s.all_occ_sq & allowed_sq & valid_sq;

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

        uint64_t capt_sq = pawn_attacks[s.side][from_sq] & s.enemy_occ_sq & allowed_sq & valid_sq;
        while (capt_sq) {
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

void addKnightMoves(Board& b, MoveList& moves, uint64_t valid_sq, MovegenState &s) {
    uint64_t knights = b.getKnightBitboard(s.side) & ~s.pinned_pieces;
    while (knights) {
        uint8_t sq = pop_lsb(&knights);
        uint64_t knight_sq = knight_moves[sq] & ~s.friendly_occ_sq & valid_sq;
        while (knight_sq) {
            uint8_t target_sq = pop_lsb(&knight_sq);
            moves.add(Move(sq, target_sq, 0));
        }
    }
}

void addSlidingMoves(Board& b, MoveList& moves, uint64_t valid_sq, MovegenState &s) {
    uint64_t bishops = b.getBishopBitboard(s.side);
    uint64_t rooks   = b.getRookBitboard(s.side);
    uint64_t queens  = b.getQueenBitboard(s.side);

    while (bishops) {
        uint8_t from_sq = pop_lsb(&bishops);
        uint64_t attacked_sq = get_bishop_attacks(s.all_occ_sq, from_sq) & ~s.friendly_occ_sq & valid_sq;
        if ((1ULL << from_sq) & s.pinned_pieces) attacked_sq &= s.pinned_ray[from_sq];
        while (attacked_sq) {
            moves.add(Move(from_sq, pop_lsb(&attacked_sq), 0));
        }
    }

    while (rooks) {
        uint8_t from_sq = pop_lsb(&rooks);
        uint64_t attacked_sq = get_rook_attacks(s.all_occ_sq, from_sq) & ~s.friendly_occ_sq & valid_sq;
        if ((1ULL << from_sq) & s.pinned_pieces) attacked_sq &= s.pinned_ray[from_sq];
        while (attacked_sq) {
            moves.add(Move(from_sq, pop_lsb(&attacked_sq), 0));
        }
    }

    while (queens) {
        uint8_t from_sq = pop_lsb(&queens);
        uint64_t attacked_sq = (get_rook_attacks(s.all_occ_sq, from_sq)
                              | get_bishop_attacks(s.all_occ_sq, from_sq)) & ~s.friendly_occ_sq & valid_sq;
        if ((1ULL << from_sq) & s.pinned_pieces) attacked_sq &= s.pinned_ray[from_sq];
        while (attacked_sq) {
            moves.add(Move(from_sq, pop_lsb(&attacked_sq), 0));
        }
    }
}

void addKingMoves(Board& b, MoveList& moves, MovegenState &s) {
    uint64_t king = b.getKingBitboard(s.side);
    assert(king);

    uint8_t sq = pop_lsb(&king);
    uint64_t king_sq = king_moves[sq] & ~s.friendly_occ_sq;

    while (king_sq) {
        uint8_t target_sq = pop_lsb(&king_sq);
        if (b.attackers_to(target_sq, 0ULL, s.all_occ_sq ^ (1ULL << sq))) continue;
        moves.add(Move(sq, target_sq, 0));
    }

    if (s.in_check) return;

    uint8_t cr = b.getCastlingRights();
    for (int i = 0; i < 2; i++) {
        if ((cr & (1 << (i + (s.side == WHITE ? 0 : 2)))) != 0
            && (s.all_occ_sq & castling_clear_sq[s.side][i]) == 0) {
            uint64_t cleared_sq = castling_unattacked_sq[s.side][i];
            bool can_castle = true;
            while (cleared_sq) {
                uint8_t clear_sq = pop_lsb(&cleared_sq);
                if (b.attackers_to(clear_sq, 0ULL, s.all_occ_sq)) { can_castle = false; break; }
            }
            if (can_castle) moves.add(Move(sq, castling_target_sq[s.side][i], CASTLE));
        }
    }
}

uint64_t ray_between(int sq_1, int sq_2) {
    return ray_between_table[sq_1][sq_2];
}

template MoveList generate_moves<ALL_MOVES>(Board& b);
template MoveList generate_moves<CAPTURES>(Board& b);

template void addPawnMoves<ALL_MOVES>(Board& b, MoveList& moves, uint64_t valid_sq, MovegenState &s);
template void addPawnMoves<CAPTURES>(Board& b, MoveList& moves, uint64_t valid_sq, MovegenState &s);
template void addPawnMoves<QUIET>(Board& b, MoveList& moves, uint64_t valid_sq, MovegenState &s);
template void addPawnMoves<EVASIONS>(Board& b, MoveList& moves, uint64_t valid_sq, MovegenState &s);
template void addPawnMoves<PINNED>(Board& b, MoveList& moves, uint64_t valid_sq, MovegenState &s);
