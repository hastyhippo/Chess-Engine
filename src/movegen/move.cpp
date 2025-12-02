#include "move.h"
#include "../misc/defines.h"

Move::Move(uint8_t from_sq, uint8_t to_sq, uint8_t move_flag, bool capture) {
    this->from_sq = from_sq;
    this->to_sq = to_sq;
    this->move_flag = move_flag;
    this->capture = capture;
}

string Move::getName() {
    vector<string> flag_name = {"-", "DBL", "PR_K", "PR_B", "PR_R", "PR_Q", "ENP", "O--O"};
    string res = sq_to_name[this->from_sq] + "->" + sq_to_name[this->to_sq];
    if (move_flag != 0) {
        res += " f: " + flag_name[this->move_flag];
    }
    if(capture) {
        res += " C";
    }
    return res;
}

uint8_t Move::getToSq() {
    return this->to_sq;
}
uint8_t Move::getFromSq() {
    return this->from_sq;
}
uint8_t Move::getMoveFlag() {
    return this->move_flag;
}
bool Move::isCapture() {
    return this->capture;
}

bool Move::isPromo() {
    return move_flag >= PROMOTION_KNIGHT && move_flag <= PROMOTION_QUEEN;
}

uint8_t Move::promoPiece() {
    switch(move_flag) {
        case PROMOTION_QUEEN: return W_QUEEN;
        case PROMOTION_ROOK: return W_ROOK;
        case PROMOTION_BISHOP: return W_BISHOP;
        case PROMOTION_KNIGHT: return W_KNIGHT;
    }
    return -1;
} 
