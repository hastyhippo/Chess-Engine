#pragma once
#include <bits/stdc++.h>
#include "../misc/defines.h"
#include "../movegen/move.h"
#include "../representation/board.h"

using namespace std;

vector<Move> generateMoves(Board &b);
void preCalculations(Board& b);
void addPawnMoves(Board& b, vector<Move>& moves);
void addKnightMoves(Board& b, vector<Move>& moves);
void addSlidingMoves(Board& b, vector<Move>& moves);
void addKingMoves(Board& b, vector<Move>& moves);
