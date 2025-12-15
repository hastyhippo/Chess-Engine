#pragma once

#include "../misc/defines.h"
#include "../movegen/movegen.h"
#include "../search/eval.h"

int negamax(Board &b, Colour side, int depth);
Move get_best_move(Board &b, Colour side, int depth);