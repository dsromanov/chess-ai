#pragma once

#include "core/Board.h"
#include "core/Move.h"
#include <vector>

namespace Chess {

class MoveGenerator {
public:
    explicit MoveGenerator(const Board& board);

    // Генерировать все легальные ходы
    std::vector<Move> generateLegalMoves(Color color) const;

    // Генерировать только взятия (для quiescence search)
    std::vector<Move> generateCaptures(Color color) const;

private:
    const Board& board_;

    // Генерация псевдо-легальных ходов (без проверки на шах)
    std::vector<Move> generatePseudoLegalMoves(Color color) const;

    // Генерация ходов для конкретных типов фигур
    void generatePawnMoves(Square from, Color color, std::vector<Move>& moves) const;
    void generateKnightMoves(Square from, Color color, std::vector<Move>& moves) const;
    void generateBishopMoves(Square from, Color color, std::vector<Move>& moves) const;
    void generateRookMoves(Square from, Color color, std::vector<Move>& moves) const;
    void generateQueenMoves(Square from, Color color, std::vector<Move>& moves) const;
    void generateKingMoves(Square from, Color color, std::vector<Move>& moves) const;

    // Вспомогательные методы
    void addMoveIfValid(Square from, Square to, Color color, std::vector<Move>& moves) const;
    bool isSquareValid(int file, int rank) const;
};

} // namespace Chess

