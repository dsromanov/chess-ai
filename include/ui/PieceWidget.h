#pragma once

#include <QLabel>
#include <QPixmap>
#include "core/Piece.h"

namespace Chess {
namespace UI {

class PieceWidget : public QLabel {
    Q_OBJECT

public:
    explicit PieceWidget(const Piece& piece, QWidget* parent = nullptr);

    void setPiece(const Piece& piece);
    const Piece& piece() const { return piece_; }

private:
    Piece piece_;
    QPixmap pixmap_;

    void updatePixmap();
};

}} // namespace Chess::UI

