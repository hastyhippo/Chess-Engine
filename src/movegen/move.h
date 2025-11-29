#pragma once
#include <bits/stdc++.h>
#define CASTLE 0
#define ENPASSANT 1
#define PROMOTION_KNIGHT 2
#define PROMOTION_BISHOP 3
#define PROMOTION_ROOK 4
#define PROMOTION_QUEEN 5
using namespace std;


class Move {
    private:
        uint64_t fromSq;
        uint64_t toSq;
        uint8_t moveFlag;
    public:
        Move(uint64_t fromSq, uint64_t toSq, uint8_t moveFlag);
};