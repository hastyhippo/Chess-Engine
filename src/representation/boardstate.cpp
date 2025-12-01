#include "boardstate.h"

BoardState::BoardState() {
    for (int i = 0; i < 12; i++) {
        this->piece_bb[i] = 0ULL;
    }
    this->board_info = 0;
}

uint64_t BoardState::getPieceBitboard(PieceType piece_type) {
    return this->piece_bb[piece_type];
}

uint64_t BoardState::getPieceBitboard(PieceType piece_type, bool current_player) {
    return this->piece_bb[piece_type + (6 * (1 - current_player))];
}

void BoardState::addPieceBitboard(PieceType piece_type, uint64_t to_add) {
    this->piece_bb[piece_type] = this->piece_bb[piece_type] | to_add;
}

uint8_t BoardState::getCastlingRights() {
    return (this->board_info & CASTLING_BITMASK);
}

uint8_t BoardState::getHalfMoveClock() {
    return (this->board_info & HALFMOVES_BITMASK) >> 4;
}

uint8_t BoardState::getEnpassantSquare() {
    return (this->board_info & ENPASSANT_BITMASK) >> 10;
}

void BoardState::setBoardInfo(uint16_t new_info) {
    this->board_info = new_info;
}

uint16_t BoardState::getBoardInfo() {
    return this->board_info;
}