#include "core/Move.h"

namespace Chess {

Move::Move() 
    : from_(0), to_(0), flag_(MoveFlag::Normal), promotion_(PieceType::None) {}

Move::Move(Square from, Square to, MoveFlag flag, PieceType promotion)
    : from_(from), to_(to), flag_(flag), promotion_(promotion) {}

std::string Move::toAlgebraic() const {
    // Упрощенная версия (Long Algebraic Notation)
    return squareToString(from_) + squareToString(to_);
}

std::string Move::toLongAlgebraic() const {
    std::string result = squareToString(from_);
    
    if (isCapture()) {
        result += 'x';
    }
    
    result += squareToString(to_);
    
    if (isPromotion()) {
        switch (promotion_) {
            case PieceType::Queen:  result += 'Q'; break;
            case PieceType::Rook:   result += 'R'; break;
            case PieceType::Bishop: result += 'B'; break;
            case PieceType::Knight: result += 'N'; break;
            default: break;
        }
    }
    
    return result;
}

} // namespace Chess

