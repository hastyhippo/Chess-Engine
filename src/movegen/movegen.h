#pragma once
#include <bits/stdc++.h>
#include "../misc/defines.h"
#include "../movegen/move.h"
#include "../representation/board.h"

using namespace std;

struct SMagic;
template<GenType Type>
vector<Move> generateMoves(Board &b);


void preCalculations(Board& b);
void addKnightMoves(Board& b, vector<Move>& moves, uint64_t valid_sq);
void addSlidingMoves(Board& b, vector<Move>& moves, uint64_t valid_sq);
void addKingMoves(Board& b, vector<Move>& moves, uint64_t valid_sq);
uint64_t attackers_to(bool colour, uint8_t sq, uint64_t occ, Board& b);
uint64_t has_attackers(bool colour, uint8_t sq, uint64_t occ, Board& b);