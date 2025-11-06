#include "core/Board.h"
#include <sstream>
#include <cmath>

namespace Chess {

Board::Board() {
    // Инициализация пустой доски
    for (auto& sq : squares_) {
        sq = Piece();
    }
}

void Board::setupInitialPosition() {
    // Очистка доски
    for (auto& sq : squares_) {
        sq = Piece();
    }
    
    // Белые фигуры
    setPiece(makeSquare(0, 0), Piece(PieceType::Rook, Color::White));
    setPiece(makeSquare(1, 0), Piece(PieceType::Knight, Color::White));
    setPiece(makeSquare(2, 0), Piece(PieceType::Bishop, Color::White));
    setPiece(makeSquare(3, 0), Piece(PieceType::Queen, Color::White));
    setPiece(makeSquare(4, 0), Piece(PieceType::King, Color::White));
    setPiece(makeSquare(5, 0), Piece(PieceType::Bishop, Color::White));
    setPiece(makeSquare(6, 0), Piece(PieceType::Knight, Color::White));
    setPiece(makeSquare(7, 0), Piece(PieceType::Rook, Color::White));
    
    // Белые пешки
    for (int file = 0; file < 8; ++file) {
        setPiece(makeSquare(file, 1), Piece(PieceType::Pawn, Color::White));
    }
    
    // Черные пешки
    for (int file = 0; file < 8; ++file) {
        setPiece(makeSquare(file, 6), Piece(PieceType::Pawn, Color::Black));
    }
    
    // Черные фигуры
    setPiece(makeSquare(0, 7), Piece(PieceType::Rook, Color::Black));
    setPiece(makeSquare(1, 7), Piece(PieceType::Knight, Color::Black));
    setPiece(makeSquare(2, 7), Piece(PieceType::Bishop, Color::Black));
    setPiece(makeSquare(3, 7), Piece(PieceType::Queen, Color::Black));
    setPiece(makeSquare(4, 7), Piece(PieceType::King, Color::Black));
    setPiece(makeSquare(5, 7), Piece(PieceType::Bishop, Color::Black));
    setPiece(makeSquare(6, 7), Piece(PieceType::Knight, Color::Black));
    setPiece(makeSquare(7, 7), Piece(PieceType::Rook, Color::Black));
    
    // Установка начальной позиции
    position_ = Position();
}

Square Board::findKing(Color color) const {
    for (Square sq = 0; sq < NUM_SQUARES; ++sq) {
        const Piece& piece = pieceAt(sq);
        if (piece.type() == PieceType::King && piece.color() == color) {
            return sq;
        }
    }
    return 255; // Не найден (не должно случиться в нормальной игре)
}

bool Board::isSquareAttacked(Square sq, Color byColor) const {
    int targetFile = getFile(sq);
    int targetRank = getRank(sq);
    
    // Проверка атак пешек
    int pawnDir = (byColor == Color::White) ? 1 : -1;
    int pawnRank = targetRank - pawnDir;
    if (pawnRank >= 0 && pawnRank < 8) {
        if (targetFile > 0) {
            Square leftSq = makeSquare(targetFile - 1, pawnRank);
            const Piece& leftPiece = pieceAt(leftSq);
            if (leftPiece.type() == PieceType::Pawn && leftPiece.color() == byColor) {
                return true;
            }
        }
        if (targetFile < 7) {
            Square rightSq = makeSquare(targetFile + 1, pawnRank);
            const Piece& rightPiece = pieceAt(rightSq);
            if (rightPiece.type() == PieceType::Pawn && rightPiece.color() == byColor) {
                return true;
            }
        }
    }
    
    // Проверка атак коня
    const int knightMoves[8][2] = {
        {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
        {1, -2}, {1, 2}, {2, -1}, {2, 1}
    };
    for (const auto& move : knightMoves) {
        int newFile = targetFile + move[0];
        int newRank = targetRank + move[1];
        if (newFile >= 0 && newFile < 8 && newRank >= 0 && newRank < 8) {
            Square knightSq = makeSquare(newFile, newRank);
            const Piece& piece = pieceAt(knightSq);
            if (piece.type() == PieceType::Knight && piece.color() == byColor) {
                return true;
            }
        }
    }
    
    // Проверка атак по диагоналям (слон, ферзь)
    const int diagDirs[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    for (const auto& dir : diagDirs) {
        int file = targetFile + dir[0];
        int rank = targetRank + dir[1];
        while (file >= 0 && file < 8 && rank >= 0 && rank < 8) {
            Square testSq = makeSquare(file, rank);
            const Piece& piece = pieceAt(testSq);
            if (!piece.isNone()) {
                if (piece.color() == byColor && 
                    (piece.type() == PieceType::Bishop || piece.type() == PieceType::Queen)) {
                    return true;
                }
                break;
            }
            file += dir[0];
            rank += dir[1];
        }
    }
    
    // Проверка атак по прямым (ладья, ферзь)
    const int straightDirs[4][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
    for (const auto& dir : straightDirs) {
        int file = targetFile + dir[0];
        int rank = targetRank + dir[1];
        while (file >= 0 && file < 8 && rank >= 0 && rank < 8) {
            Square testSq = makeSquare(file, rank);
            const Piece& piece = pieceAt(testSq);
            if (!piece.isNone()) {
                if (piece.color() == byColor && 
                    (piece.type() == PieceType::Rook || piece.type() == PieceType::Queen)) {
                    return true;
                }
                break;
            }
            file += dir[0];
            rank += dir[1];
        }
    }
    
    // Проверка атак короля
    const int kingMoves[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1}, {0, -1},
        {0, 1}, {1, -1}, {1, 0}, {1, 1}
    };
    for (const auto& move : kingMoves) {
        int newFile = targetFile + move[0];
        int newRank = targetRank + move[1];
        if (newFile >= 0 && newFile < 8 && newRank >= 0 && newRank < 8) {
            Square kingSq = makeSquare(newFile, newRank);
            const Piece& piece = pieceAt(kingSq);
            if (piece.type() == PieceType::King && piece.color() == byColor) {
                return true;
            }
        }
    }
    
    return false;
}

bool Board::isCheck(Color color) const {
    Square kingSq = findKing(color);
    if (kingSq == 255) return false;
    return isSquareAttacked(kingSq, oppositeColor(color));
}

void Board::makeMove(const Move& move) {
    UndoInfo undo;
    undo.capturedPiece = pieceAt(move.to());
    undo.state = position_.getState();
    history_.push_back(undo);
    
    Piece movingPiece = pieceAt(move.from());
    
    // Обновить счетчик полуходов
    if (movingPiece.type() == PieceType::Pawn || move.isCapture()) {
        position_.halfmoveClock() = 0;
    } else {
        position_.halfmoveClock()++;
    }
    
    // Простое перемещение фигуры
    setPiece(move.to(), movingPiece);
    removePiece(move.from());
    
    // Рокировка
    if (move.isCastling()) {
        int rank = getRank(move.from());
        if (getFile(move.to()) == 6) { // Kingside
            Square rookFrom = makeSquare(7, rank);
            Square rookTo = makeSquare(5, rank);
            setPiece(rookTo, pieceAt(rookFrom));
            removePiece(rookFrom);
        } else { // Queenside
            Square rookFrom = makeSquare(0, rank);
            Square rookTo = makeSquare(3, rank);
            setPiece(rookTo, pieceAt(rookFrom));
            removePiece(rookFrom);
        }
    }
    
    // Превращение пешки
    if (move.isPromotion()) {
        setPiece(move.to(), Piece(move.promotion(), movingPiece.color()));
    }
    
    // Взятие на проходе
    if (move.isEnPassant()) {
        int captureRank = getRank(move.from());
        Square captureSq = makeSquare(getFile(move.to()), captureRank);
        removePiece(captureSq);
    }
    
    // Обновить en passant
    position_.setEnPassantSquare(255);
    if (movingPiece.type() == PieceType::Pawn) {
        int rankDiff = std::abs(getRank(move.to()) - getRank(move.from()));
        if (rankDiff == 2) {
            int epRank = (getRank(move.from()) + getRank(move.to())) / 2;
            position_.setEnPassantSquare(makeSquare(getFile(move.from()), epRank));
        }
    }
    
    // Обновить права на рокировку
    if (movingPiece.type() == PieceType::King) {
        position_.setCastlingRights(movingPiece.color(), false, false);
    }
    if (movingPiece.type() == PieceType::Rook) {
        if (move.from() == makeSquare(0, 0)) {
            position_.setCastlingRights(Color::White, 
                position_.canCastleKingside(Color::White), false);
        } else if (move.from() == makeSquare(7, 0)) {
            position_.setCastlingRights(Color::White, 
                false, position_.canCastleQueenside(Color::White));
        } else if (move.from() == makeSquare(0, 7)) {
            position_.setCastlingRights(Color::Black, 
                position_.canCastleKingside(Color::Black), false);
        } else if (move.from() == makeSquare(7, 7)) {
            position_.setCastlingRights(Color::Black, 
                false, position_.canCastleQueenside(Color::Black));
        }
    }
    
    // Переключить сторону для хода
    Color nextSide = oppositeColor(position_.sideToMove());
    if (nextSide == Color::White) {
        position_.fullmoveNumber()++;
    }
    position_.setSideToMove(nextSide);
}

void Board::unmakeMove(const Move& move) {
    if (history_.empty()) return;
    
    UndoInfo undo = history_.back();
    history_.pop_back();
    
    // Восстановить состояние позиции
    position_.setState(undo.state);
    
    // Вернуть фигуру назад
    Piece movingPiece = pieceAt(move.to());
    setPiece(move.from(), movingPiece);
    
    // Восстановить взятую фигуру
    setPiece(move.to(), undo.capturedPiece);
    
    // Отменить рокировку
    if (move.isCastling()) {
        int rank = getRank(move.from());
        if (getFile(move.to()) == 6) { // Kingside
            Square rookFrom = makeSquare(5, rank);
            Square rookTo = makeSquare(7, rank);
            setPiece(rookTo, pieceAt(rookFrom));
            removePiece(rookFrom);
        } else { // Queenside
            Square rookFrom = makeSquare(3, rank);
            Square rookTo = makeSquare(0, rank);
            setPiece(rookTo, pieceAt(rookFrom));
            removePiece(rookFrom);
        }
    }
    
    // Отменить превращение пешки
    if (move.isPromotion()) {
        setPiece(move.from(), Piece(PieceType::Pawn, movingPiece.color()));
    }
    
    // Отменить взятие на проходе
    if (move.isEnPassant()) {
        int captureRank = getRank(move.from());
        Square captureSq = makeSquare(getFile(move.to()), captureRank);
        Color opponentColor = oppositeColor(movingPiece.color());
        setPiece(captureSq, Piece(PieceType::Pawn, opponentColor));
        removePiece(move.to());
    }
}

std::string Board::toString() const {
    std::ostringstream ss;
    for (int rank = 7; rank >= 0; --rank) {
        ss << (rank + 1) << " ";
        for (int file = 0; file < 8; ++file) {
            Square sq = makeSquare(file, rank);
            ss << pieceAt(sq).toUnicode() << " ";
        }
        ss << "\n";
    }
    ss << "  a b c d e f g h\n";
    return ss.str();
}

bool Board::isCheckmate(Color color) const {
    // Упрощенная проверка (будет реализована в MoveGenerator)
    return false;
}

bool Board::isStalemate(Color color) const {
    // Упрощенная проверка
    return false;
}

bool Board::isDraw() const {
    // Проверка на ничью по правилу 50 ходов
    if (position_.halfmoveClock() >= 100) {
        return true;
    }
    
    // TODO: Проверка на недостаточность материала
    // TODO: Проверка на троекратное повторение
    
    return false;
}

void Board::setFromFEN(const std::string& fen) {
    // Очистка доски
    for (auto& sq : squares_) {
        sq = Piece();
    }
    
    std::istringstream ss(fen);
    std::string boardPart;
    ss >> boardPart;
    
    int rank = 7;
    int file = 0;
    
    for (char c : boardPart) {
        if (c == '/') {
            rank--;
            file = 0;
        } else if (c >= '1' && c <= '8') {
            file += (c - '0');
        } else {
            Color color = (c >= 'A' && c <= 'Z') ? Color::White : Color::Black;
            PieceType type = PieceType::None;
            
            switch (std::tolower(c)) {
                case 'p': type = PieceType::Pawn; break;
                case 'n': type = PieceType::Knight; break;
                case 'b': type = PieceType::Bishop; break;
                case 'r': type = PieceType::Rook; break;
                case 'q': type = PieceType::Queen; break;
                case 'k': type = PieceType::King; break;
            }
            
            if (type != PieceType::None) {
                setPiece(makeSquare(file, rank), Piece(type, color));
            }
            file++;
        }
    }
    
    // Парсинг остальной части FEN
    position_.setFromFEN(fen);
}

std::string Board::toFEN() const {
    std::ostringstream fen;
    
    // Доска
    for (int rank = 7; rank >= 0; --rank) {
        int emptyCount = 0;
        for (int file = 0; file < 8; ++file) {
            Square sq = makeSquare(file, rank);
            const Piece& piece = pieceAt(sq);
            
            if (piece.isNone()) {
                emptyCount++;
            } else {
                if (emptyCount > 0) {
                    fen << emptyCount;
                    emptyCount = 0;
                }
                fen << piece.toChar();
            }
        }
        if (emptyCount > 0) {
            fen << emptyCount;
        }
        if (rank > 0) {
            fen << '/';
        }
    }
    
    fen << " " << position_.toFEN();
    
    return fen.str();
}

} // namespace Chess

