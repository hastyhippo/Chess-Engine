#include "move.h"

Move::Move(uint64_t fromSq, uint64_t toSq, uint8_t moveFlag) {
    this->fromSq = fromSq;
    this->toSq = toSq;
    this->moveFlag = moveFlag;
}