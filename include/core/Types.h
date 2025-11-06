#pragma once

#include <cstdint>
#include <string>

namespace Chess {

// Типы фигур
enum class PieceType : uint8_t {
    None = 0,
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King
};

// Цвета
enum class Color : uint8_t {
    White = 0,
    Black = 1,
    None = 2
};

// Координаты на доске (0-63)
using Square = uint8_t;

// Битовая доска для оптимизации
using Bitboard = uint64_t;

// Константы
constexpr int BOARD_SIZE = 8;
constexpr int NUM_SQUARES = 64;

// Утилиты для работы с координатами
constexpr Square makeSquare(int file, int rank) {
    return rank * BOARD_SIZE + file;
}

constexpr int getFile(Square sq) {
    return sq % BOARD_SIZE;
}

constexpr int getRank(Square sq) {
    return sq / BOARD_SIZE;
}

// Конвертация в алгебраическую нотацию (e2, e4, etc.)
inline std::string squareToString(Square sq) {
    char file = 'a' + getFile(sq);
    char rank = '1' + getRank(sq);
    return std::string{file, rank};
}

inline Square stringToSquare(const std::string& str) {
    if (str.length() != 2) return 255;
    int file = str[0] - 'a';
    int rank = str[1] - '1';
    if (file < 0 || file >= 8 || rank < 0 || rank >= 8) return 255;
    return makeSquare(file, rank);
}

// Противоположный цвет
inline Color oppositeColor(Color c) {
    return (c == Color::White) ? Color::Black : Color::White;
}

} // namespace Chess

