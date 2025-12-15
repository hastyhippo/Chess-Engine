#pragma once
#include <bits/stdc++.h>
#include "../misc/defines.h"
#include "../movegen/move.h"
#include "../representation/board.h"

using namespace std;

struct SMagic;

template<GenType Type>
MoveList generate_moves(Board &b);
MoveList generate_all_moves(Board &b, uint64_t valid_sq);

void addPinnedPawns(Board& b, MoveList& moves, uint64_t valid_sq);
template<GenType Type>
void addPawnMoves(Board& b, MoveList& moves, uint64_t valid_sq);
void addKnightMoves(Board& b, MoveList& moves, uint64_t valid_sq);
void addSlidingMoves(Board& b, MoveList& moves, uint64_t valid_sq);
void addKingMoves(Board& b, MoveList& moves);

// uint64_t attackers_to(Colour side, uint8_t sq, uint64_t occ, Board& b);
uint64_t ray_between(int sq_1, int sq_2);
// uint64_t getBishopAttacks(uint64_t occ, int sq);
// uint64_t getRookAttacks(uint64_t occ, int sq);
// uint64_t has_attackers(Colour side, uint8_t sq, uint64_t occ, uint64_t ignore_sq, Board& b);
