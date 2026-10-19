#pragma once
#include "../representation/board.h"

// HalfKP 40960 -> 256x2 -> 32 -> 32 -> 1, evaluated in int16/int8/int32.
// Weights come from training/serialize.py; see src/test/nnue_examples.txt for
// the fixture that pins the arithmetic down.

bool nnue_load(const string &path);
bool nnue_ready();
const char *nnue_file();

int nnue_evaluate(Board &b);  // centipawns, side-to-move relative
int nnue_raw(Board &b);       // the int32 before the centipawn conversion

// Accumulator stack. push() reads the board BEFORE the move is made; the
// accumulator itself is only built when nnue_raw/evaluate asks for it.
void nnue_reset();
void nnue_push(Board &b, Move m);
void nnue_push_null();
void nnue_pop();
int nnue_debug_features(Board &b, int white_pov, int *out);
int nnue_raw_refresh(Board &b);  // bypasses the stack; for tests
