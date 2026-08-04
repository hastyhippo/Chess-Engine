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

Move get_best_move(Board &b, int depth, int time_limit_ms);
Move get_best_move(Board &b, int time_limit_ms = 0);
struct PVLine {
    Move moves[64];
    int length = 0;
};

int negamax(Board &b, int depth, int alpha, int beta, TimePoint time_limit, PVLine &pv);
int qsearch(Board &b, int alpha, int beta, int qdepth);
int SEE(Board &b, Move &m);
