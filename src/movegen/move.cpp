#include "move.h"
#include "../misc/defines.h"

Move::Move(uint64_t from_sq, uint64_t to_sq, uint8_t move_flag) {
    this->from_sq = from_sq;
    this->to_sq = to_sq;
    this->move_flag = move_flag;
}

string Move::getName() {
    string res = square_to_name[this->from_sq] + "->" + square_to_name[this->to_sq];
    if (move_flag != 0) {
        res += "f: " + to_string(this->move_flag);
    }
    return res;
}