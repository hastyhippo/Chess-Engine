#include "movegen.h"
#include "../misc/defines.h"
#include "move.h"
#include <cstdint>

uint64_t friendly_occ_sq;
uint64_t enemy_occ_sq;
uint64_t all_occ_sq;

template <GenType Type>
vector<Move> generateMoves(Board& b) {
    bool white = b.getWhiteTurn();
    preCalculations(b);
    vector<Move> moves;

    int num_attackers = 0;
    uint64_t attacked_sq = 0ULL;

    uint64_t valid_sq = Type == EVASIONS ? ~0ULL :  // TODO: implement get_blockers() for EVASIONS
                        Type == CAPTURES ? enemy_occ_sq :
                        Type == QUIET ? ~enemy_occ_sq : ~0ULL;

    if (num_attackers <= 1) {
        addPawnMoves<Type>(b, moves, valid_sq);
        addKnightMoves(b, moves, valid_sq);
        addSlidingMoves(b, moves, valid_sq); 
    }

    addKingMoves(b, moves, ~attacked_sq);
    return moves;
}

void preCalculations(Board& b) {
    friendly_occ_sq = b.getColourPieces(b.getWhiteTurn());
    enemy_occ_sq = b.getColourPieces(!b.getWhiteTurn());
    all_occ_sq = friendly_occ_sq | enemy_occ_sq;
}

// Use template here because knowing if we calculate captures only can prune out chunks of logic
template<GenType Type>
void addPawnMoves(Board& b, vector<Move>& moves, uint64_t valid_sq) {
    bool white = b.getWhiteTurn();
    int forward = white ? NORTH : SOUTH;

    uint64_t pawns = b.getPieceBitboard(W_PAWN, white);
    uint64_t pr_rank = white ? RANK_8 : RANK_1; // promotion rank

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
        uint64_t capt_pawns = pawns & ~pr_rank;
        while(capt_pawns) {
            uint8_t pawn_sq = pop_lsb(&capt_pawns);
            uint64_t attack_bb = pawn_attacks[1 - white][pawn_sq] & enemy_occ_sq;
            if constexpr (Type == EVASIONS) attack_bb &= valid_sq;

            while(attack_bb) {
                uint8_t to_sq = pop_lsb(&attack_bb);
                moves.push_back(Move(pawn_sq, to_sq, 0, true));
            }
        }
        uint64_t pr_capt_pawns = pawns & pr_rank;
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
            uint64_t can_enp = enp_sq != 0 ? get_file[enp_sq + WEST] : 0;
            can_enp |= enp_sq != N - 1 ? get_file[enp_sq + EAST] : 0;
    
            can_enp &= (white ? RANK_5 : RANK_4) & pawns;
    
            int target_sq = enp_sq + N * (white ? 5 : 2); // set the enpassant square to the correct rank
            while (can_enp) {
                uint8_t sq = pop_lsb(&can_enp);
                moves.push_back(Move(sq, target_sq, ENPASSANT, true));
            }
        }    
    }}

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
    bool whiteTurn = b.getWhiteTurn();
    uint64_t bishops = b.getPieceBitboard(W_BISHOP, whiteTurn);
    uint64_t rooks = b.getPieceBitboard(W_ROOK, whiteTurn);
    uint64_t queens = b.getPieceBitboard(W_QUEEN, whiteTurn);

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


void addKingMoves(Board& b, vector<Move>& moves, uint64_t valid_sq) {
    uint64_t king = b.getPieceBitboard(W_KING, b.getWhiteTurn());
    if (!king) return;
    uint8_t sq = pop_lsb(&king);
    uint64_t king_sq = king_moves[sq] & ~friendly_occ_sq & valid_sq;
    while(king_sq) {
        uint8_t target_sq = pop_lsb(&king_sq);
        moves.push_back(Move(sq, target_sq, 0, ((1ULL << target_sq) & enemy_occ_sq) != 0));
    }

    //Castling
    for (int i = 0; i < 2; i++) {
        if (!(all_occ_sq & castling_clear_sq[b.getWhiteTurn()][i])) {
            moves.push_back(Move(sq, castling_target_sq[1 - b.getWhiteTurn()][i], CASTLE, false));
        }
    }
}

template vector<Move> generateMoves<ALL_MOVES>(Board& b);
template vector<Move> generateMoves<CAPTURES>(Board& b);
template vector<Move> generateMoves<QUIET>(Board& b);
template vector<Move> generateMoves<EVASIONS>(Board& b);

template void addPawnMoves<ALL_MOVES>(Board& b, vector<Move>& moves, uint64_t valid_sq);
template void addPawnMoves<CAPTURES>(Board& b, vector<Move>& moves, uint64_t valid_sq);
template void addPawnMoves<QUIET>(Board& b, vector<Move>& moves, uint64_t valid_sq);
template void addPawnMoves<EVASIONS>(Board& b, vector<Move>& moves, uint64_t valid_sq);