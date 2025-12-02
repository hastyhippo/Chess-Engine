#pragma once
#include <bits/stdc++.h>
#include "../misc/defines.h"
#include "../movegen/move.h"
#include "../representation/board.h"

using namespace std;

struct SMagic;
template<GenType Type>
vector<Move> generateMoves(Board &b);


void addKnightMoves(Board& b, vector<Move>& moves, uint64_t valid_sq);
void addSlidingMoves(Board& b, vector<Move>& moves, uint64_t valid_sq);
void addKingMoves(Board& b, vector<Move>& moves, uint64_t valid_sq);
uint64_t attackers_to(bool colour, uint8_t sq, uint64_t occ, Board& b);
uint64_t has_attackers(bool colour, uint8_t sq, uint64_t occ, Board& b);
bool square_attacked_after_move(bool attacker_colour, uint8_t sq, Move m, uint64_t base_occupancy, Board& b);
uint64_t ray_between(int sq_1, int sq_2);
uint64_t getBishopAttacks(uint64_t occ, int sq);
uint64_t getRookAttacks(uint64_t occ, int sq);
uint64_t getQueenAttacks(uint64_t occ, int sq);
uint64_t has_attackers(bool colour, uint8_t sq, uint64_t occ, uint64_t to_sq, Board& b);