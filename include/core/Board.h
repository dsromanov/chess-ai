#pragma once

#include "core/Piece.h"
#include "core/Move.h"
#include "core/Position.h"
#include "core/Types.h"
#include <array>
#include <vector>
#include <string>

namespace Chess {

class Board {
public:
    Board();

    // Получить фигуру на клетке
    const Piece& pieceAt(Square sq) const { return squares_[sq]; }
    Piece& pieceAt(Square sq) { return squares_[sq]; }

    // Установить фигуру
    void setPiece(Square sq, const Piece& piece) { squares_[sq] = piece; }
    void removePiece(Square sq) { squares_[sq] = Piece(); }

    // Позиция
    const Position& position() const { return position_; }
    Position& position() { return position_; }

    // Сделать/отменить ход
    void makeMove(const Move& move);
    void unmakeMove(const Move& move);

    // Проверки состояния
    bool isCheck(Color color) const;
    bool isCheckmate(Color color) const;
    bool isStalemate(Color color) const;
    bool isDraw() const;

    // Найти короля
    Square findKing(Color color) const;

    // Атакована ли клетка
    bool isSquareAttacked(Square sq, Color byColor) const;

    // Инициализация стандартной позиции
    void setupInitialPosition();

    // FEN поддержка
    void setFromFEN(const std::string& fen);
    std::string toFEN() const;

    // Отладочный вывод
    std::string toString() const;

private:
    std::array<Piece, NUM_SQUARES> squares_;
    Position position_;

    // История для отмены ходов
    struct UndoInfo {
        Piece capturedPiece;
        PositionState state;
    };
    std::vector<UndoInfo> history_;
};

} // namespace Chess

