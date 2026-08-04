#include "move.h"
#include "../misc/defines.h"

constexpr uint16_t FROM_SQ_MASK = 0b00111111;      // Bits 0-5: 0b00111111
constexpr uint16_t TO_SQ_MASK = 0b111111000000;       // Bits 6-11: 0b111111000000
constexpr uint16_t MOVE_FLAG_MASK = 0b1111000000000000;  // Bits 12-15: 0b1111000000000000
// flag values (move.h): 0 NORMAL | 1 DOUBLE_PUSH | 2-5 PROMOTION N/B/R/Q | 6 ENPASSANT | 7 CASTLE

Move::Move(uint8_t from_sq, uint8_t to_sq, uint8_t move_flag) {
    this->move_data = (from_sq) | (to_sq << 6) | (move_flag << 12);
}

string Move::getName() {
    string res = sq_to_name[getFromSq()] + sq_to_name[getToSq()];
    if (isPromo()) {
        res += "nbrq"[getMoveFlag() - PROMOTION_KNIGHT];
    }
    return res;
}

uint8_t Move::getToSq() {
    return (this->move_data & TO_SQ_MASK) >> 6;
}

uint8_t Move::getFromSq() {
    return this->move_data & FROM_SQ_MASK;
}

uint8_t Move::getMoveFlag() {
    return (this->move_data & MOVE_FLAG_MASK) >> 12;
}

bool Move::isPromo() {
    uint8_t flag = getMoveFlag();
    return flag >= PROMOTION_KNIGHT && flag <= PROMOTION_QUEEN;
}

PieceType Move::promoPiece() {
    switch(getMoveFlag()) {
        case PROMOTION_QUEEN: return QUEEN;
        case PROMOTION_ROOK: return ROOK;
        case PROMOTION_BISHOP: return BISHOP;
        case PROMOTION_KNIGHT: return KNIGHT;
    }
    return NO_TYPE;
} 
