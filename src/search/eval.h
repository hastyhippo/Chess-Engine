#pragma once
#include "../misc/defines.h"
#include "../representation/board.h"

static const int PIECE_VALUES[] = { 100, 320, 330, 500, 900, 0 };


// Old evaluations stay reachable so they can be matched against new ones.
enum EvalMode { EVAL_PESTO, EVAL_MATERIAL, EVAL_NNUE, EVAL_MODE_COUNT };

extern EvalMode eval_mode;
bool set_eval_mode(const string &name);   // false if the name is unknown
const char *eval_mode_name();
const char *eval_mode_name(EvalMode m);

void init_pesto_tables();
int evaluate(Board &b);