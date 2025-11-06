#pragma once

#include "core/Types.h"
#include <string>

namespace Chess {

// Состояние позиции для отмены ходов
struct PositionState {
    Square enPassantSquare;
    bool whiteCanCastleKingside;
    bool whiteCanCastleQueenside;
    bool blackCanCastleKingside;
    bool blackCanCastleQueenside;
    int halfmoveClock;
    int fullmoveNumber;
};

class Position {
public:
    Position();

    // Получить состояние
    Color sideToMove() const { return sideToMove_; }
    Square enPassantSquare() const { return enPassantSquare_; }
    int halfmoveClock() const { return halfmoveClock_; }
    int fullmoveNumber() const { return fullmoveNumber_; }
    
    // Неконстантные версии для модификации
    int& halfmoveClock() { return halfmoveClock_; }
    int& fullmoveNumber() { return fullmoveNumber_; }

    // Рокировка
    bool canCastleKingside(Color c) const;
    bool canCastleQueenside(Color c) const;

    // Установить состояние
    void setSideToMove(Color c) { sideToMove_ = c; }
    void setEnPassantSquare(Square sq) { enPassantSquare_ = sq; }
    void setCastlingRights(Color c, bool kingside, bool queenside);

    // FEN (Forsyth-Edwards Notation) поддержка
    void setFromFEN(const std::string& fen);
    std::string toFEN() const;

    // Сохранить/восстановить состояние
    PositionState getState() const;
    void setState(const PositionState& state);

private:
    Color sideToMove_;
    Square enPassantSquare_;
    bool whiteCanCastleKingside_;
    bool whiteCanCastleQueenside_;
    bool blackCanCastleKingside_;
    bool blackCanCastleQueenside_;
    int halfmoveClock_;  // Для правила 50 ходов
    int fullmoveNumber_;
};

} // namespace Chess

