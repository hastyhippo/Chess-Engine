#pragma once
#include "../misc/defines.h"
#include "../representation/board.h"

static const int PIECE_VALUES[] = { 100, 320, 330, 500, 900, 0 };


extern bool use_pesto;
void init_pesto_tables();
int evaluate(Board &b);