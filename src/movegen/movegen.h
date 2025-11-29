#include <bits/stdc++.h>
#include "../misc/defines.h"
class Move;
class Board;
using namespace std;

void addMoves(Board& b, vector<Move>& moves);
void preCalculations(Board& b);
void addPawnMoves(Board& b, vector<Move>& moves);
void addKnightMoves(Board& b, vector<Move>& moves);
void addSlidingMoves(Board& b, vector<Move>& moves);
void addKingMoves(Board& b, vector<Move>& moves);