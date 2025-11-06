#include "core/MoveValidator.h"

namespace Chess {

MoveValidator::MoveValidator(const Board& board) : board_(board) {}

bool MoveValidator::isLegal(const Move& move, Color color) const {
    // Проверка базовых условий
    if (move.from() >= NUM_SQUARES || move.to() >= NUM_SQUARES) {
        return false;
    }
    
    const Piece& piece = board_.pieceAt(move.from());
    if (piece.isNone() || piece.color() != color) {
        return false;
    }
    
    // Проверка, что ход не оставляет короля под шахом
    return !leavesKingInCheck(move, color);
}

bool MoveValidator::leavesKingInCheck(const Move& move, Color color) const {
    // Сделать ход на временной копии доски
    Board tempBoard = board_;
    tempBoard.makeMove(move);
    
    // Проверить, под шахом ли король
    bool inCheck = tempBoard.isCheck(color);
    
    // Отменить ход (на временной копии, так что это не влияет на настоящую доску)
    // tempBoard.unmakeMove(move);
    
    return inCheck;
}

} // namespace Chess

