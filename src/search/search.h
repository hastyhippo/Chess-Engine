#pragma once

#include "../misc/defines.h"
#include "../movegen/movegen.h"
#include "../search/eval.h"
#include <chrono>
using namespace std::chrono;
using Clock     = std::chrono::steady_clock;
using TimePoint = Clock::time_point;

constexpr int MAX_DEPTH = 256;
constexpr int MAX_PV_DEPTH = 64;  // PVLine::moves is 64 long
constexpr int CHECKMATE = 8888888;
constexpr int MATE_SCORE = CHECKMATE - MAX_DEPTH;  // scores above this = forced mate
constexpr int OUTOFTIME = 5555555;
constexpr int INF = 999999999;
constexpr int TT_SIZE = 1 << 22;

Move get_best_move(Board &b, int depth, int time_limit_ms);
struct PVLine {
    Move moves[64];
    int length = 0;
};

enum Bound { EXACT, LOWER, UPPER };

constexpr int32_t EVAL_NONE = INT32_MIN;

struct TTEntry {
    uint64_t key;
    int32_t value;
    int32_t eval;   // static eval, cached: NNUE is too slow to recompute  // int16 can't hold mate scores (CHECKMATE = 8888888)
    Move move;
    uint8_t depth;
    Bound bound;
};

extern TTEntry tt[TT_SIZE];

int negamax(Board &b, int depth, int ply, int alpha, int beta, TimePoint time_limit, PVLine &pv);
int qsearch(Board &b, int alpha, int beta, int qdepth);
int SEE(Board &b, Move &m);

TTEntry* probe(uint64_t key);