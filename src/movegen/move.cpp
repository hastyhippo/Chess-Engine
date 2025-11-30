#include "move.h"
#include "../misc/defines.h"

Move::Move(uint8_t from_sq, uint8_t to_sq, uint8_t move_flag, bool capture) {
    this->from_sq = from_sq;
    this->to_sq = to_sq;
    this->move_flag = move_flag;
    this->capture = capture;
}

string Move::getName() {
    vector<string> flag_name = {"O-O", "DBL", "PR_K", "PR_B", "PR_R", "PR_Q"};
    string res = sq_to_name[this->from_sq] + "->" + sq_to_name[this->to_sq];
    if (move_flag != 0) {
        res += " f: " + flag_name[this->move_flag];
    }
    if(capture) {
        res += " C";
    }
    return res;
}