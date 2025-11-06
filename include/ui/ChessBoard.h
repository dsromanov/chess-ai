#pragma once

#include <QWidget>
#include <QPixmap>
#include <memory>
#include "core/Board.h"
#include "core/Move.h"

namespace Chess {
namespace UI {

class ChessBoard : public QWidget {
    Q_OBJECT

public:
    explicit ChessBoard(QWidget* parent = nullptr);

    void setBoard(std::shared_ptr<Board> board);
    std::shared_ptr<Board> getBoard() const { return board_; }

    // Установить цвет игрока
    void setPlayerColor(Color color) { playerColor_ = color; }
    Color getPlayerColor() const { return playerColor_; }

    // Подсветка клеток
    void highlightSquare(Square sq);
    void clearHighlights();
    void showLegalMoves(Square from);

    // Переворот доски
    void setFlipped(bool flipped) { flipped_ = flipped; update(); }
    bool isFlipped() const { return flipped_; }

signals:
    void moveRequested(const Move& move);
    void squareClicked(Square sq);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    std::shared_ptr<Board> board_;
    Color playerColor_; // Цвет игрока (человека)
    
    // Визуальное состояние
    bool flipped_;
    Square selectedSquare_;
    std::vector<Square> highlightedSquares_;
    std::vector<Square> legalMoveSquares_;
    
    // Перетаскивание фигур
    bool dragging_;
    Square dragFrom_;
    QPoint dragPosition_;

    // Размеры
    int squareSize_;

    // Цвета темы
    QColor lightSquareColor_;
    QColor darkSquareColor_;
    QColor highlightColor_;
    QColor legalMoveColor_;
    QColor lastMoveColor_;

    // Изображения фигур
    std::map<std::pair<PieceType, Color>, QPixmap> pieceImages_;

    // Вспомогательные методы
    void loadPieceImages();
    Square screenToSquare(const QPoint& pos) const;
    QRect squareToRect(Square sq) const;
    void drawSquare(QPainter& painter, Square sq);
    void drawPiece(QPainter& painter, const Piece& piece, const QRect& rect);
    void drawCoordinates(QPainter& painter);
};

}} // namespace Chess::UI

