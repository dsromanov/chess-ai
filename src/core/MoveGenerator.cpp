#include "core/MoveGenerator.h"
#include "core/MoveValidator.h"

namespace Chess {

MoveGenerator::MoveGenerator(const Board& board) : board_(board) {}

std::vector<Move> MoveGenerator::generateLegalMoves(Color color) const {
    std::vector<Move> pseudoLegal = generatePseudoLegalMoves(color);
    std::vector<Move> legal;
    
    MoveValidator validator(board_);
    for (const Move& move : pseudoLegal) {
        if (!validator.leavesKingInCheck(move, color)) {
            legal.push_back(move);
        }
    }
    
    return legal;
}

std::vector<Move> MoveGenerator::generateCaptures(Color color) const {
    std::vector<Move> allMoves = generateLegalMoves(color);
    std::vector<Move> captures;
    
    for (const Move& move : allMoves) {
        if (move.isCapture()) {
            captures.push_back(move);
        }
    }
    
    return captures;
}

std::vector<Move> MoveGenerator::generatePseudoLegalMoves(Color color) const {
    std::vector<Move> moves;
    
    for (Square sq = 0; sq < NUM_SQUARES; ++sq) {
        const Piece& piece = board_.pieceAt(sq);
        if (piece.isNone() || piece.color() != color) {
            continue;
        }
        
        switch (piece.type()) {
            case PieceType::Pawn:
                generatePawnMoves(sq, color, moves);
                break;
            case PieceType::Knight:
                generateKnightMoves(sq, color, moves);
                break;
            case PieceType::Bishop:
                generateBishopMoves(sq, color, moves);
                break;
            case PieceType::Rook:
                generateRookMoves(sq, color, moves);
                break;
            case PieceType::Queen:
                generateQueenMoves(sq, color, moves);
                break;
            case PieceType::King:
                generateKingMoves(sq, color, moves);
                break;
            default:
                break;
        }
    }
    
    return moves;
}

void MoveGenerator::generatePawnMoves(Square from, Color color, std::vector<Move>& moves) const {
    int file = getFile(from);
    int rank = getRank(from);
    int direction = (color == Color::White) ? 1 : -1;
    int startRank = (color == Color::White) ? 1 : 6;
    int promotionRank = (color == Color::White) ? 7 : 0;
    
    // Ход вперед
    int newRank = rank + direction;
    if (newRank >= 0 && newRank < 8) {
        Square toSq = makeSquare(file, newRank);
        if (board_.pieceAt(toSq).isNone()) {
            if (newRank == promotionRank) {
                // Превращение
                moves.push_back(Move(from, toSq, MoveFlag::Promotion, PieceType::Queen));
                moves.push_back(Move(from, toSq, MoveFlag::Promotion, PieceType::Rook));
                moves.push_back(Move(from, toSq, MoveFlag::Promotion, PieceType::Bishop));
                moves.push_back(Move(from, toSq, MoveFlag::Promotion, PieceType::Knight));
            } else {
                moves.push_back(Move(from, toSq));
            }
            
            // Двойной ход с начальной позиции
            if (rank == startRank) {
                Square doubleSq = makeSquare(file, rank + 2 * direction);
                if (board_.pieceAt(doubleSq).isNone()) {
                    moves.push_back(Move(from, doubleSq, MoveFlag::DoublePawnPush));
                }
            }
        }
    }
    
    // Взятия
    for (int df : {-1, 1}) {
        int newFile = file + df;
        if (newFile >= 0 && newFile < 8 && newRank >= 0 && newRank < 8) {
            Square toSq = makeSquare(newFile, newRank);
            const Piece& target = board_.pieceAt(toSq);
            
            if (!target.isNone() && target.color() != color) {
                if (newRank == promotionRank) {
                    moves.push_back(Move(from, toSq, MoveFlag::Promotion, PieceType::Queen));
                    moves.push_back(Move(from, toSq, MoveFlag::Promotion, PieceType::Rook));
                    moves.push_back(Move(from, toSq, MoveFlag::Promotion, PieceType::Bishop));
                    moves.push_back(Move(from, toSq, MoveFlag::Promotion, PieceType::Knight));
                } else {
                    moves.push_back(Move(from, toSq, MoveFlag::Capture));
                }
            }
            
            // Взятие на проходе
            if (toSq == board_.position().enPassantSquare()) {
                moves.push_back(Move(from, toSq, MoveFlag::EnPassant));
            }
        }
    }
}

void MoveGenerator::generateKnightMoves(Square from, Color color, std::vector<Move>& moves) const {
    int file = getFile(from);
    int rank = getRank(from);
    
    const int knightMoves[8][2] = {
        {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
        {1, -2}, {1, 2}, {2, -1}, {2, 1}
    };
    
    for (const auto& move : knightMoves) {
        int newFile = file + move[0];
        int newRank = rank + move[1];
        if (isSquareValid(newFile, newRank)) {
            addMoveIfValid(from, makeSquare(newFile, newRank), color, moves);
        }
    }
}

void MoveGenerator::generateBishopMoves(Square from, Color color, std::vector<Move>& moves) const {
    int file = getFile(from);
    int rank = getRank(from);
    
    const int directions[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    
    for (const auto& dir : directions) {
        int newFile = file + dir[0];
        int newRank = rank + dir[1];
        
        while (isSquareValid(newFile, newRank)) {
            Square toSq = makeSquare(newFile, newRank);
            const Piece& target = board_.pieceAt(toSq);
            
            if (target.isNone()) {
                moves.push_back(Move(from, toSq));
            } else {
                if (target.color() != color) {
                    moves.push_back(Move(from, toSq, MoveFlag::Capture));
                }
                break;
            }
            
            newFile += dir[0];
            newRank += dir[1];
        }
    }
}

void MoveGenerator::generateRookMoves(Square from, Color color, std::vector<Move>& moves) const {
    int file = getFile(from);
    int rank = getRank(from);
    
    const int directions[4][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
    
    for (const auto& dir : directions) {
        int newFile = file + dir[0];
        int newRank = rank + dir[1];
        
        while (isSquareValid(newFile, newRank)) {
            Square toSq = makeSquare(newFile, newRank);
            const Piece& target = board_.pieceAt(toSq);
            
            if (target.isNone()) {
                moves.push_back(Move(from, toSq));
            } else {
                if (target.color() != color) {
                    moves.push_back(Move(from, toSq, MoveFlag::Capture));
                }
                break;
            }
            
            newFile += dir[0];
            newRank += dir[1];
        }
    }
}

void MoveGenerator::generateQueenMoves(Square from, Color color, std::vector<Move>& moves) const {
    generateBishopMoves(from, color, moves);
    generateRookMoves(from, color, moves);
}

void MoveGenerator::generateKingMoves(Square from, Color color, std::vector<Move>& moves) const {
    int file = getFile(from);
    int rank = getRank(from);
    
    const int directions[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1}, {0, -1},
        {0, 1}, {1, -1}, {1, 0}, {1, 1}
    };
    
    for (const auto& dir : directions) {
        int newFile = file + dir[0];
        int newRank = rank + dir[1];
        if (isSquareValid(newFile, newRank)) {
            addMoveIfValid(from, makeSquare(newFile, newRank), color, moves);
        }
    }
    
    // Рокировка
    const Position& pos = board_.position();
    if (!board_.isCheck(color)) {
        // Kingside
        if (pos.canCastleKingside(color)) {
            Square f1 = makeSquare(file + 1, rank);
            Square g1 = makeSquare(file + 2, rank);
            
            if (board_.pieceAt(f1).isNone() && board_.pieceAt(g1).isNone() &&
                !board_.isSquareAttacked(f1, oppositeColor(color)) &&
                !board_.isSquareAttacked(g1, oppositeColor(color))) {
                moves.push_back(Move(from, g1, MoveFlag::Castling));
            }
        }
        
        // Queenside
        if (pos.canCastleQueenside(color)) {
            Square d1 = makeSquare(file - 1, rank);
            Square c1 = makeSquare(file - 2, rank);
            Square b1 = makeSquare(file - 3, rank);
            
            if (board_.pieceAt(d1).isNone() && board_.pieceAt(c1).isNone() &&
                board_.pieceAt(b1).isNone() &&
                !board_.isSquareAttacked(d1, oppositeColor(color)) &&
                !board_.isSquareAttacked(c1, oppositeColor(color))) {
                moves.push_back(Move(from, c1, MoveFlag::Castling));
            }
        }
    }
}

void MoveGenerator::addMoveIfValid(Square from, Square to, Color color, std::vector<Move>& moves) const {
    const Piece& target = board_.pieceAt(to);
    if (target.isNone()) {
        moves.push_back(Move(from, to));
    } else if (target.color() != color) {
        moves.push_back(Move(from, to, MoveFlag::Capture));
    }
}

bool MoveGenerator::isSquareValid(int file, int rank) const {
    return file >= 0 && file < 8 && rank >= 0 && rank < 8;
}

} // namespace Chess

