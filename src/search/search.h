#pragma once

#include "../misc/defines.h"
#include "../movegen/movegen.h"
#include "../search/eval.h"
#include <chrono>
using namespace std::chrono;
using Clock     = std::chrono::steady_clock;
using TimePoint = Clock::time_point;

constexpr int MAX_DEPTH = 256;
constexpr int CHECKMATE = 8888888;
constexpr int MATE_SCORE = CHECKMATE - MAX_DEPTH;  // scores above this = forced mate
constexpr int OUTOFTIME = 5555555;
constexpr int INF = 999999999;
constexpr int TT_SIZE = 1 << 22;

Move get_best_move(Board &b, int depth, int time_limit_ms);
Move get_best_move(Board &b, int time_limit_ms = 0);
struct PVLine {
    Move moves[64];
    int length = 0;
};

enum Bound { EXACT, LOWER, UPPER };

struct TTEntry {
    uint64_t key;
    int32_t value;  // int16 can't hold mate scores (CHECKMATE = 8888888)
    int32_t eval;
    Move move;
    uint8_t depth;
    Bound bound;
    uint8_t age;
};

extern TTEntry tt[TT_SIZE];
extern int tt_age;

int negamax(Board &b, int depth, int alpha, int beta, TimePoint time_limit, PVLine &pv);
int qsearch(Board &b, int alpha, int beta, int qdepth);
int SEE(Board &b, Move &m);

TTEntry* probe(uint64_t key);