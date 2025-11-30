#include "move.h"
#include "../misc/defines.h"

Move::Move(uint64_t fromSq, uint64_t toSq, uint8_t moveFlag) {
    this->fromSq = fromSq;
    this->toSq = toSq;
    this->moveFlag = moveFlag;
}

string Move::getName() {
    string res = squareToName[this->fromSq] + "->" + squareToName[this->toSq];
    if (moveFlag != 0) {
        res += "f: " + to_string(this->moveFlag);
    }
    return res;
}