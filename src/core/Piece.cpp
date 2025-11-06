#include "core/Piece.h"

namespace Chess {

Piece::Piece() : type_(PieceType::None), color_(Color::None) {}

Piece::Piece(PieceType type, Color color) : type_(type), color_(color) {}

char Piece::toChar() const {
    if (type_ == PieceType::None) return '.';
    
    char c;
    switch (type_) {
        case PieceType::Pawn:   c = 'P'; break;
        case PieceType::Knight: c = 'N'; break;
        case PieceType::Bishop: c = 'B'; break;
        case PieceType::Rook:   c = 'R'; break;
        case PieceType::Queen:  c = 'Q'; break;
        case PieceType::King:   c = 'K'; break;
        default: c = '.'; break;
    }
    
    return (color_ == Color::White) ? c : static_cast<char>(c + 32); // lowercase для черных
}

std::string Piece::toUnicode() const {
    if (type_ == PieceType::None) return " ";
    
    // Unicode символы шахматных фигур
    if (color_ == Color::White) {
        switch (type_) {
            case PieceType::King:   return "♔";
            case PieceType::Queen:  return "♕";
            case PieceType::Rook:   return "♖";
            case PieceType::Bishop: return "♗";
            case PieceType::Knight: return "♘";
            case PieceType::Pawn:   return "♙";
            default: return " ";
        }
    } else {
        switch (type_) {
            case PieceType::King:   return "♚";
            case PieceType::Queen:  return "♛";
            case PieceType::Rook:   return "♜";
            case PieceType::Bishop: return "♝";
            case PieceType::Knight: return "♞";
            case PieceType::Pawn:   return "♟";
            default: return " ";
        }
    }
}

int Piece::value() const {
    switch (type_) {
        case PieceType::Pawn:   return 100;
        case PieceType::Knight: return 320;
        case PieceType::Bishop: return 330;
        case PieceType::Rook:   return 500;
        case PieceType::Queen:  return 900;
        case PieceType::King:   return 20000;
        default: return 0;
    }
}

} // namespace Chess

