#pragma once

#include <bits/stdc++.h>

using namespace std;

#define N_ROWS 8
#define N_COLS 8
enum PieceType {W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING, 
                B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING};

vector<string>splitString(const string& input, const char delimiter);
void printBB(uint64_t bb);