#include "core/Position.h"
#include <sstream>

namespace Chess {

Position::Position() 
    : sideToMove_(Color::White),
      enPassantSquare_(255),
      whiteCanCastleKingside_(true),
      whiteCanCastleQueenside_(true),
      blackCanCastleKingside_(true),
      blackCanCastleQueenside_(true),
      halfmoveClock_(0),
      fullmoveNumber_(1) {}

bool Position::canCastleKingside(Color c) const {
    return (c == Color::White) ? whiteCanCastleKingside_ : blackCanCastleKingside_;
}

bool Position::canCastleQueenside(Color c) const {
    return (c == Color::White) ? whiteCanCastleQueenside_ : blackCanCastleQueenside_;
}

void Position::setCastlingRights(Color c, bool kingside, bool queenside) {
    if (c == Color::White) {
        whiteCanCastleKingside_ = kingside;
        whiteCanCastleQueenside_ = queenside;
    } else {
        blackCanCastleKingside_ = kingside;
        blackCanCastleQueenside_ = queenside;
    }
}

PositionState Position::getState() const {
    return PositionState{
        enPassantSquare_,
        whiteCanCastleKingside_,
        whiteCanCastleQueenside_,
        blackCanCastleKingside_,
        blackCanCastleQueenside_,
        halfmoveClock_,
        fullmoveNumber_
    };
}

void Position::setState(const PositionState& state) {
    enPassantSquare_ = state.enPassantSquare;
    whiteCanCastleKingside_ = state.whiteCanCastleKingside;
    whiteCanCastleQueenside_ = state.whiteCanCastleQueenside;
    blackCanCastleKingside_ = state.blackCanCastleKingside;
    blackCanCastleQueenside_ = state.blackCanCastleQueenside;
    halfmoveClock_ = state.halfmoveClock;
    fullmoveNumber_ = state.fullmoveNumber;
}

void Position::setFromFEN(const std::string& fen) {
    // Упрощенная версия парсинга FEN
    std::istringstream ss(fen);
    std::string token;
    
    // Пропускаем доску (первая часть)
    ss >> token;
    
    // Сторона для хода
    ss >> token;
    sideToMove_ = (token == "w") ? Color::White : Color::Black;
    
    // Права на рокировку
    ss >> token;
    whiteCanCastleKingside_ = (token.find('K') != std::string::npos);
    whiteCanCastleQueenside_ = (token.find('Q') != std::string::npos);
    blackCanCastleKingside_ = (token.find('k') != std::string::npos);
    blackCanCastleQueenside_ = (token.find('q') != std::string::npos);
    
    // En passant
    ss >> token;
    if (token != "-") {
        enPassantSquare_ = stringToSquare(token);
    } else {
        enPassantSquare_ = 255;
    }
    
    // Halfmove clock
    if (ss >> token) {
        halfmoveClock_ = std::stoi(token);
    }
    
    // Fullmove number
    if (ss >> token) {
        fullmoveNumber_ = std::stoi(token);
    }
}

std::string Position::toFEN() const {
    std::ostringstream fen;
    
    // Сторона для хода
    fen << (sideToMove_ == Color::White ? "w" : "b") << " ";
    
    // Права на рокировку
    std::string castling;
    if (whiteCanCastleKingside_) castling += 'K';
    if (whiteCanCastleQueenside_) castling += 'Q';
    if (blackCanCastleKingside_) castling += 'k';
    if (blackCanCastleQueenside_) castling += 'q';
    fen << (castling.empty() ? "-" : castling) << " ";
    
    // En passant
    if (enPassantSquare_ != 255) {
        fen << squareToString(enPassantSquare_);
    } else {
        fen << "-";
    }
    fen << " ";
    
    // Halfmove и fullmove
    fen << halfmoveClock_ << " " << fullmoveNumber_;
    
    return fen.str();
}

} // namespace Chess

