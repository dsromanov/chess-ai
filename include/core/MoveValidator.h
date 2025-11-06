#pragma once

#include "core/Board.h"
#include "core/Move.h"

namespace Chess {

class MoveValidator {
public:
    explicit MoveValidator(const Board& board);

    // Проверить, легален ли ход
    bool isLegal(const Move& move, Color color) const;

    // Проверить, оставляет ли ход короля под шахом
    bool leavesKingInCheck(const Move& move, Color color) const;

private:
    const Board& board_;
};

} // namespace Chess

