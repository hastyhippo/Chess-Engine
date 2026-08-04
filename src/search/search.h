#pragma once

#include "../misc/defines.h"
#include "../movegen/movegen.h"
#include "../search/eval.h"
#include <chrono>
using namespace std::chrono;
using Clock     = std::chrono::steady_clock;
using TimePoint = Clock::time_point;

constexpr int CHECKMATE = 8888888;
constexpr int OUTOFTIME = -5555555;

int negamax(Board &b, int depth, TimePoint time_limit = TimePoint::max());
Move get_best_move(Board &b, int depth, int time_limit_ms);
Move get_best_move(Board &b, int time_limit_ms = 0);