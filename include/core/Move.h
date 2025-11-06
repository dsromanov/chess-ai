#pragma once

#include "core/Types.h"

namespace Chess {

// Специальные флаги хода
enum class MoveFlag : uint8_t {
    Normal = 0,
    Capture = 1,
    EnPassant = 2,
    Castling = 3,
    Promotion = 4,
    DoublePawnPush = 5
};

class Move {
public:
    Move();
    Move(Square from, Square to, MoveFlag flag = MoveFlag::Normal, 
         PieceType promotion = PieceType::None);

    Square from() const { return from_; }
    Square to() const { return to_; }
    MoveFlag flag() const { return flag_; }
    PieceType promotion() const { return promotion_; }

    bool isCapture() const { 
        return flag_ == MoveFlag::Capture || flag_ == MoveFlag::EnPassant; 
    }
    bool isPromotion() const { return flag_ == MoveFlag::Promotion; }
    bool isCastling() const { return flag_ == MoveFlag::Castling; }
    bool isEnPassant() const { return flag_ == MoveFlag::EnPassant; }

    // Конвертация в алгебраическую нотацию
    std::string toAlgebraic() const;
    std::string toLongAlgebraic() const;

    bool isValid() const { return from_ != to_; }

    bool operator==(const Move& other) const {
        return from_ == other.from_ && to_ == other.to_ && 
               flag_ == other.flag_ && promotion_ == other.promotion_;
    }

private:
    Square from_;
    Square to_;
    MoveFlag flag_;
    PieceType promotion_;
};

} // namespace Chess

