#pragma once

#include <cstdint>

enum PieceType {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NO_TYPE};

enum Colour : uint8_t { WHITE = 0, BLACK = 1 };

inline uint8_t piece(Colour colour, PieceType type) {
    return static_cast<uint8_t>(colour) * 6 + static_cast<uint8_t>(type);
}

inline Colour color_of(uint8_t p) {
    return static_cast<Colour>(static_cast<uint8_t>(p) / 6);
}

inline PieceType type_of(uint8_t p) {
    return static_cast<PieceType>(static_cast<uint8_t>(p) % 6);
}
