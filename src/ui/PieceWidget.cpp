#include "ui/PieceWidget.h"

namespace Chess {
namespace UI {

PieceWidget::PieceWidget(const Piece& piece, QWidget* parent)
    : QLabel(parent), piece_(piece)
{
    updatePixmap();
}

void PieceWidget::setPiece(const Piece& piece) {
    piece_ = piece;
    updatePixmap();
}

void PieceWidget::updatePixmap() {
    QString text = QString::fromStdString(piece_.toUnicode());
    setText(text);
    
    QFont font = this->font();
    font.setPixelSize(40);
    setFont(font);
}

}} // namespace Chess::UI

