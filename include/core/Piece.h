#pragma once

#include "core/Types.h"

namespace Chess {

class Piece {
public:
    Piece();
    Piece(PieceType type, Color color);

    PieceType type() const { return type_; }
    Color color() const { return color_; }
    
    bool isNone() const { return type_ == PieceType::None; }
    bool isWhite() const { return color_ == Color::White; }
    bool isBlack() const { return color_ == Color::Black; }

    // Получить символ фигуры (для отладки и PGN)
    char toChar() const;
    std::string toUnicode() const;

    // Оценка стоимости фигуры
    int value() const;

    bool operator==(const Piece& other) const {
        return type_ == other.type_ && color_ == other.color_;
    }

    bool operator!=(const Piece& other) const {
        return !(*this == other);
    }

private:
    PieceType type_;
    Color color_;
};

} // namespace Chess

