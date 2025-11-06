#pragma once

#include "core/Board.h"
#include "core/Types.h"

namespace Chess {
namespace AI {

class Evaluator {
public:
    explicit Evaluator(const Board& board);

    // Оценка позиции с точки зрения белых (положительная = белые лучше)
    int evaluate() const;

private:
    const Board& board_;

    // Компоненты оценки
    int evaluateMaterial() const;
    int evaluatePosition() const;
    int evaluateMobility() const;
    int evaluateKingSafety() const;
    int evaluatePawnStructure() const;

    // Piece-Square Tables для позиционной оценки
    static const int PAWN_TABLE[64];
    static const int KNIGHT_TABLE[64];
    static const int BISHOP_TABLE[64];
    static const int ROOK_TABLE[64];
    static const int QUEEN_TABLE[64];
    static const int KING_MIDDLE_GAME_TABLE[64];
    static const int KING_END_GAME_TABLE[64];

    // Оценка для конкретной клетки
    int getPieceSquareValue(const Piece& piece, Square sq) const;

    // Определить, эндшпиль ли
    bool isEndgame() const;
};

}} // namespace Chess::AI

