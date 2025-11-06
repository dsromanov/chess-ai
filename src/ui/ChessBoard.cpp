#include "ui/ChessBoard.h"
#include "core/MoveGenerator.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>

namespace Chess {
namespace UI {

ChessBoard::ChessBoard(QWidget* parent)
    : QWidget(parent),
      playerColor_(Color::White), // По умолчанию белые
      flipped_(false),
      selectedSquare_(255),
      dragging_(false),
      dragFrom_(255),
      squareSize_(60),
      lightSquareColor_(240, 217, 181),
      darkSquareColor_(181, 136, 99),
      highlightColor_(255, 255, 0, 100),
      legalMoveColor_(0, 255, 0, 100),
      lastMoveColor_(255, 200, 100, 150)
{
    setMinimumSize(480, 480);
    setMouseTracking(true);
}

void ChessBoard::setBoard(std::shared_ptr<Board> board) {
    board_ = board;
    update();
}

void ChessBoard::highlightSquare(Square sq) {
    highlightedSquares_.push_back(sq);
    update();
}

void ChessBoard::clearHighlights() {
    highlightedSquares_.clear();
    legalMoveSquares_.clear();
    selectedSquare_ = 255;
    update();
}

void ChessBoard::showLegalMoves(Square from) {
    if (!board_) return;
    
    legalMoveSquares_.clear();
    selectedSquare_ = from;
    
    MoveGenerator generator(*board_);
    Color color = board_->position().sideToMove();
    std::vector<Move> moves = generator.generateLegalMoves(color);
    
    for (const Move& move : moves) {
        if (move.from() == from) {
            legalMoveSquares_.push_back(move.to());
        }
    }
    
    update();
}

void ChessBoard::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Рисуем доску
    for (Square sq = 0; sq < NUM_SQUARES; ++sq) {
        drawSquare(painter, sq);
    }
    
    // Рисуем координаты
    drawCoordinates(painter);
    
    // Рисуем фигуры
    if (board_) {
        for (Square sq = 0; sq < NUM_SQUARES; ++sq) {
            if (dragging_ && sq == dragFrom_) {
                continue; // Пропускаем перетаскиваемую фигуру
            }
            
            const Piece& piece = board_->pieceAt(sq);
            if (!piece.isNone()) {
                QRect rect = squareToRect(sq);
                drawPiece(painter, piece, rect);
            }
        }
        
        // Рисуем перетаскиваемую фигуру
        if (dragging_ && dragFrom_ != 255) {
            const Piece& piece = board_->pieceAt(dragFrom_);
            if (!piece.isNone()) {
                QRect rect(dragPosition_.x() - squareSize_ / 2,
                          dragPosition_.y() - squareSize_ / 2,
                          squareSize_, squareSize_);
                drawPiece(painter, piece, rect);
            }
        }
    }
}

void ChessBoard::mousePressEvent(QMouseEvent* event) {
    if (!board_) return;
    
    Square sq = screenToSquare(event->pos());
    if (sq == 255) return;
    
    const Piece& piece = board_->pieceAt(sq);
    
    // Можно взять только СВОЮ фигуру (цвета игрока), и только когда его ход
    if (!piece.isNone() && 
        piece.color() == playerColor_ && 
        board_->position().sideToMove() == playerColor_) {
        dragging_ = true;
        dragFrom_ = sq;
        dragPosition_ = event->pos();
        showLegalMoves(sq);
        emit squareClicked(sq);
    }
}

void ChessBoard::mouseReleaseEvent(QMouseEvent* event) {
    if (!dragging_) return;
    
    Square to = screenToSquare(event->pos());
    
    if (to != 255 && dragFrom_ != 255) {
        // Проверяем, является ли ход легальным
        bool isLegal = false;
        for (Square legalSq : legalMoveSquares_) {
            if (legalSq == to) {
                isLegal = true;
                break;
            }
        }
        
        if (isLegal) {
            // Определить флаг хода
            const Piece& movingPiece = board_->pieceAt(dragFrom_);
            const Piece& targetPiece = board_->pieceAt(to);
            MoveFlag flag = MoveFlag::Normal;
            
            if (!targetPiece.isNone()) {
                flag = MoveFlag::Capture;
            }
            
            // Проверка на превращение пешки
            PieceType promotion = PieceType::None;
            if (movingPiece.type() == PieceType::Pawn) {
                int toRank = getRank(to);
                if ((movingPiece.isWhite() && toRank == 7) ||
                    (movingPiece.isBlack() && toRank == 0)) {
                    flag = MoveFlag::Promotion;
                    promotion = PieceType::Queen; // Автоматически выбираем ферзя
                }
            }
            
            Move move(dragFrom_, to, flag, promotion);
            emit moveRequested(move);
        }
    }
    
    dragging_ = false;
    dragFrom_ = 255;
    clearHighlights();
}

void ChessBoard::mouseMoveEvent(QMouseEvent* event) {
    if (dragging_) {
        dragPosition_ = event->pos();
        update();
    }
}

void ChessBoard::resizeEvent(QResizeEvent* event) {
    Q_UNUSED(event);
    squareSize_ = std::min(width(), height()) / 8;
    update();
}

Square ChessBoard::screenToSquare(const QPoint& pos) const {
    int file = pos.x() / squareSize_;
    int rank = 7 - (pos.y() / squareSize_);
    
    if (flipped_) {
        file = 7 - file;
        rank = 7 - rank;
    }
    
    if (file < 0 || file >= 8 || rank < 0 || rank >= 8) {
        return 255;
    }
    
    return makeSquare(file, rank);
}

QRect ChessBoard::squareToRect(Square sq) const {
    int file = getFile(sq);
    int rank = getRank(sq);
    
    if (flipped_) {
        file = 7 - file;
        rank = 7 - rank;
    }
    
    int x = file * squareSize_;
    int y = (7 - rank) * squareSize_;
    
    return QRect(x, y, squareSize_, squareSize_);
}

void ChessBoard::drawSquare(QPainter& painter, Square sq) {
    QRect rect = squareToRect(sq);
    
    // Цвет клетки
    int file = getFile(sq);
    int rank = getRank(sq);
    bool isLight = (file + rank) % 2 == 0;
    
    painter.fillRect(rect, isLight ? lightSquareColor_ : darkSquareColor_);
    
    // Подсветка выделенной клетки
    if (sq == selectedSquare_) {
        painter.fillRect(rect, highlightColor_);
    }
    
    // Подсветка легальных ходов
    for (Square legalSq : legalMoveSquares_) {
        if (legalSq == sq) {
            painter.setBrush(legalMoveColor_);
            painter.setPen(Qt::NoPen);
            int centerX = rect.center().x();
            int centerY = rect.center().y();
            int radius = squareSize_ / 6;
            painter.drawEllipse(QPoint(centerX, centerY), radius, radius);
            break;
        }
    }
}

void ChessBoard::drawPiece(QPainter& painter, const Piece& piece, const QRect& rect) {
    // Простая отрисовка текстом (можно заменить на изображения)
    painter.setPen(piece.isWhite() ? Qt::white : Qt::black);
    
    QFont font = painter.font();
    font.setPixelSize(squareSize_ * 0.7);
    painter.setFont(font);
    
    // Рисуем тень для лучшей видимости
    painter.setPen(piece.isWhite() ? Qt::black : Qt::white);
    QRect shadowRect = rect.adjusted(2, 2, 2, 2);
    painter.drawText(shadowRect, Qt::AlignCenter, QString::fromStdString(piece.toUnicode()));
    
    // Рисуем саму фигуру
    painter.setPen(piece.isWhite() ? Qt::white : Qt::black);
    painter.drawText(rect, Qt::AlignCenter, QString::fromStdString(piece.toUnicode()));
}

void ChessBoard::drawCoordinates(QPainter& painter) {
    painter.setPen(Qt::black);
    QFont font = painter.font();
    font.setPixelSize(12);
    painter.setFont(font);
    
    // Буквы (a-h)
    for (int file = 0; file < 8; ++file) {
        char letter = 'a' + (flipped_ ? 7 - file : file);
        int x = file * squareSize_ + squareSize_ - 15;
        int y = 8 * squareSize_ - 5;
        painter.drawText(x, y, QString(letter));
    }
    
    // Цифры (1-8)
    for (int rank = 0; rank < 8; ++rank) {
        char number = '1' + (flipped_ ? 7 - rank : rank);
        int x = 5;
        int y = (7 - rank) * squareSize_ + 15;
        painter.drawText(x, y, QString(number));
    }
}

}} // namespace Chess::UI

