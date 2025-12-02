#include "init.h"

// Local helpers for move generation initialisation
static void initialiseKnightMoves();
static void setDistanceToEdge();
static void setDirections();
static uint64_t setOccupancy(int index, uint64_t attackMask);
static uint64_t getBishopMoves(int sq, uint64_t occupancy);
static uint64_t getRookMoves(int sq, uint64_t occupancy);
static void initialiseMagics();
static void initialiseKingMoves();
static void initialisePawnMoves();

void init() {
    initialiseMoveGeneration();
}

void initialiseMoveGeneration() {
    setDistanceToEdge();
    setDirections();
    initialiseMagics();
    initialisePawnMoves();
    initialiseKnightMoves();
    initialiseKingMoves();
}

static void initialiseKnightMoves() {
    vector<int> hor_offset = {1, 2, 2, 1, -1, -2, -2, -1};
    vector<int> vert_offset = {2, 1, -1, -2, -2, -1, 1, 2};

    vector<uint64_t> allowed_files = {
        get_files_left[N - 2], get_files_left[N - 3], get_files_left[N - 3], get_files_left[N - 2],
        get_files_right[1],    get_files_right[2],    get_files_right[2],    get_files_right[1]
    };
    vector<uint64_t> allowed_ranks = {
        get_ranks_below[N - 3], get_ranks_below[N - 2], get_ranks_above[1],  get_ranks_above[2],
        get_ranks_above[2],     get_ranks_above[1],     get_ranks_below[N - 2], get_ranks_below[N - 3]
    };

    for (int sq = 0; sq < 64; sq++) {
        uint64_t attacked_sq = 0ULL;
        for (int i = 0; i < 8; i++) {
            uint64_t new_sq = (1ULL << sq) & allowed_files[i] & allowed_ranks[i];
            attacked_sq |= shift(new_sq, hor_offset[i] + vert_offset[i] * N);
        }
        knight_moves[sq] = attacked_sq;
    }
}

static void setDistanceToEdge() {
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            int sq = x + y * 8;
            distance_to_edge[sq][0] = 7 - y;               // NORTH
            distance_to_edge[sq][1] = y;                   // SOUTH
            distance_to_edge[sq][2] = 7 - x;               // EAST
            distance_to_edge[sq][3] = x;                   // WEST
            distance_to_edge[sq][4] = min(7 - y, 7 - x);   // NORTH_EAST
            distance_to_edge[sq][5] = min(7 - y, x);       // NORTH_WEST
            distance_to_edge[sq][6] = min(y, 7 - x);       // SOUTH_EAST
            distance_to_edge[sq][7] = min(y, x);           // SOUTH_WEST
        }
    }
}

static void setDirections() {
    for (int index = 0; index < 8; index++) {
        for (int sq = 0; sq < 64; sq++) {
            uint64_t u = 0ULL;
            for (int x = 1; x <= distance_to_edge[sq][index]; x++) {
                u |= 1ULL << (sq + DIR_OFFSETS[index] * x);
            }
            directions[index][sq] = u;
        }
    }
}

static uint64_t setOccupancy(int index, uint64_t attackMask) {
    uint64_t occupancy = 0ULL;
    uint64_t mask = attackMask;
    for (int count = 0; count < __popcnt64(attackMask); count++) {
        int sq = pop_lsb(&mask);
        if ((index & (1 << count)) != 0) {
            occupancy |= 1ULL << sq;
        }
    }
    return occupancy;
}

static uint64_t getBishopMoves(int sq, uint64_t occupancy) {
    uint64_t moves = 0ULL;
    for (int i = 4; i < 8; i++) {
        for (int x = 1; x <= distance_to_edge[sq][i]; x++) {
            int targetSq = sq + DIR_OFFSETS[i] * x;
            moves |= 1ULL << targetSq;
            if ((occupancy & (1ULL << targetSq)) != 0) break;
        }
    }
    return moves;
}

static uint64_t getRookMoves(int sq, uint64_t occupancy) {
    uint64_t moves = 0ULL;
    for (int i = 0; i < 4; i++) {
        for (int x = 1; x <= distance_to_edge[sq][i]; x++) {
            int targetSq = sq + DIR_OFFSETS[i] * x;
            moves |= 1ULL << targetSq;
            if ((occupancy & (1ULL << targetSq)) != 0) break;
        }
    }
    return moves;
}

static void initialiseMagics() {
    for (int sq = 0; sq < 64; sq++) {
        for (int i = 0; i < 8; i++) {
            for (int x = 1; x < distance_to_edge[sq][i]; x++) {
                int targetSq = sq + DIR_OFFSETS[i] * x;
                if (i < 4) {
                    rook_masks[sq] |= 1ULL << targetSq;
                } else {
                    bishop_masks[sq] |= 1ULL << targetSq;
                }
            }
        }
    }

    for (int sq = 0; sq < 64; sq++) {
        for (int count = 0; count < (1 << __popcnt64(bishop_masks[sq])); count++) {
            uint64_t bishopOccupancy = setOccupancy(count, bishop_masks[sq]);
            uint64_t bishopIndex = (bishopOccupancy * bishop_magics[sq]) >> (64 - bishop_m_bits[sq]);
            bishopAttacks[sq][bishopIndex] = getBishopMoves(sq, bishopOccupancy);
        }

        for (int count = 0; count < (1 << __popcnt64(rook_masks[sq])); count++) {
            uint64_t rookOccupancy = setOccupancy(count, rook_masks[sq]);
            uint64_t rookIndex = (rookOccupancy * rook_magics[sq]) >> (64 - rook_m_bits[sq]);
            rookAttacks[sq][rookIndex] = getRookMoves(sq, rookOccupancy);
        }

        m_bishop_tbl[sq] = {bishopAttacks[sq], bishop_masks[sq], bishop_magics[sq], 64 - bishop_m_bits[sq]};
        m_rook_tbl[sq] = {rookAttacks[sq], rook_masks[sq], rook_magics[sq], 64 - rook_m_bits[sq]};
    }
}

static void initialiseKingMoves() {
    for (int sq = 0; sq < 64; sq++) {
        uint64_t attacked_sq = 0ULL;
        uint64_t blocked_sq[8] = {
            RANK_8, RANK_1, H_FILE, A_FILE,
            RANK_8 | H_FILE, RANK_8 | A_FILE,
            RANK_1 | H_FILE, RANK_1 | A_FILE
        };

        for (int i = 0; i < 8; i++) {
            uint64_t valid_sq = (1ULL << sq) & ~blocked_sq[i];
            attacked_sq |= shift(valid_sq, DIR_OFFSETS[i]);
        }
        king_moves[sq] = attacked_sq;
    }
}

static void initialisePawnMoves() {
    for (int sq = 0; sq < 64; sq++) {
        pawn_moves[0][sq] = 0ULL;
        pawn_moves[1][sq] = 0ULL;
        pawn_attacks[0][sq] = 0ULL;
        pawn_attacks[1][sq] = 0ULL;

        uint64_t pawn_bb = 1ULL << sq;

        if (!(pawn_bb & RANK_8)) {
            pawn_moves[0][sq] = 1ULL << (sq + 8);
        }
        if (!(pawn_bb & RANK_1)) {
            pawn_moves[1][sq] = 1ULL << (sq - 8);
        }

        if (!(pawn_bb & RANK_8)) {
            if (!(pawn_bb & A_FILE)) {
                pawn_attacks[0][sq] |= 1ULL << (sq + 7);
            }
            if (!(pawn_bb & H_FILE)) {
                pawn_attacks[0][sq] |= 1ULL << (sq + 9);
            }
        }

        if (!(pawn_bb & RANK_1)) {
            if (!(pawn_bb & A_FILE)) {
                pawn_attacks[1][sq] |= 1ULL << (sq - 9);
            }
            if (!(pawn_bb & H_FILE)) {
                pawn_attacks[1][sq] |= 1ULL << (sq - 7);
            }
        }
    }
}
