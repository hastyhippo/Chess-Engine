#pragma once

#include <bits/stdc++.h>

using namespace std;

#define N_ROWS 8
#define N_COLS 8
#define N_SQUARES 64
#define N_PIECE_TYPES 6

#define COL_A 
enum PieceType {W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING, 
                B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING};

vector<string>splitString(const string& input, const char delimiter);
void printBB(uint64_t bb);

extern unordered_map<string, uint64_t> nameToSquare;
extern unordered_map<uint64_t, string> squareToName;