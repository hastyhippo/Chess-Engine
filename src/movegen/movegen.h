#pragma once
#include <bits/stdc++.h>
#include "../misc/defines.h"
#include "../movegen/move.h"
#include "../representation/board.h"

using namespace std;

struct SMagic;
struct MovegenState;

template<GenType Type>
MoveList generate_moves(Board &b);
MoveList generate_all_moves(Board &b, uint64_t valid_sq, uint64_t king_mask, MovegenState &s);

void addPinnedPawns(Board& b, MoveList& moves, uint64_t valid_sq, MovegenState &s);
template<GenType Type>
void addPawnMoves(Board& b, MoveList& moves, uint64_t valid_sq, MovegenState &s);
void addKnightMoves(Board& b, MoveList& moves, uint64_t valid_sq, MovegenState &s);
void addSlidingMoves(Board& b, MoveList& moves, uint64_t valid_sq, MovegenState &s);
void addKingMoves(Board& b, MoveList& moves, uint64_t king_mask, MovegenState &s);

uint64_t ray_between(int sq_1, int sq_2);

uint64_t attackers_of(Board &b, Colour c, uint8_t sq, uint64_t occ);
uint64_t typed_bb(Board &b, Colour c, int t);
