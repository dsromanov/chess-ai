#include "ai/Evaluator.h"
#include "core/MoveGenerator.h"

namespace Chess {
namespace AI {

// Piece-Square Tables (из Stockfish, упрощенные)
const int Evaluator::PAWN_TABLE[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
     5,  5, 10, 25, 25, 10,  5,  5,
     0,  0,  0, 20, 20,  0,  0,  0,
     5, -5,-10,  0,  0,-10, -5,  5,
     5, 10, 10,-20,-20, 10, 10,  5,
     0,  0,  0,  0,  0,  0,  0,  0
};

const int Evaluator::KNIGHT_TABLE[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};

const int Evaluator::BISHOP_TABLE[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};

const int Evaluator::ROOK_TABLE[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
     5, 10, 10, 10, 10, 10, 10,  5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
     0,  0,  0,  5,  5,  0,  0,  0
};

const int Evaluator::QUEEN_TABLE[64] = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
     -5,  0,  5,  5,  5,  5,  0, -5,
      0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};

const int Evaluator::KING_MIDDLE_GAME_TABLE[64] = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
     20, 20,  0,  0,  0,  0, 20, 20,
     20, 30, 10,  0,  0, 10, 30, 20
};

const int Evaluator::KING_END_GAME_TABLE[64] = {
    -50,-40,-30,-20,-20,-30,-40,-50,
    -30,-20,-10,  0,  0,-10,-20,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-30,  0,  0,  0,  0,-30,-30,
    -50,-30,-30,-30,-30,-30,-30,-50
};

Evaluator::Evaluator(const Board& board) : board_(board) {}

int Evaluator::evaluate() const {
    int score = 0;
    
    score += evaluateMaterial();
    score += evaluatePosition();
    score += evaluateMobility();
    score += evaluateKingSafety();
    score += evaluatePawnStructure();
    
    return score;
}

int Evaluator::evaluateMaterial() const {
    int score = 0;
    
    for (Square sq = 0; sq < NUM_SQUARES; ++sq) {
        const Piece& piece = board_.pieceAt(sq);
        if (piece.isNone()) continue;
        
        int value = piece.value();
        score += (piece.isWhite() ? value : -value);
    }
    
    return score;
}

int Evaluator::evaluatePosition() const {
    int score = 0;
    
    for (Square sq = 0; sq < NUM_SQUARES; ++sq) {
        const Piece& piece = board_.pieceAt(sq);
        if (piece.isNone()) continue;
        
        int psqValue = getPieceSquareValue(piece, sq);
        score += (piece.isWhite() ? psqValue : -psqValue);
    }
    
    return score;
}

int Evaluator::getPieceSquareValue(const Piece& piece, Square sq) const {
    // Переворачиваем таблицу для черных
    Square index = sq;
    if (piece.isBlack()) {
        index = makeSquare(getFile(sq), 7 - getRank(sq));
    }
    
    switch (piece.type()) {
        case PieceType::Pawn:
            return PAWN_TABLE[index];
        case PieceType::Knight:
            return KNIGHT_TABLE[index];
        case PieceType::Bishop:
            return BISHOP_TABLE[index];
        case PieceType::Rook:
            return ROOK_TABLE[index];
        case PieceType::Queen:
            return QUEEN_TABLE[index];
        case PieceType::King:
            if (isEndgame()) {
                return KING_END_GAME_TABLE[index];
            } else {
                return KING_MIDDLE_GAME_TABLE[index];
            }
        default:
            return 0;
    }
}

int Evaluator::evaluateMobility() const {
    MoveGenerator whiteGen(board_);
    MoveGenerator blackGen(board_);
    
    int whiteMobility = whiteGen.generateLegalMoves(Color::White).size();
    int blackMobility = blackGen.generateLegalMoves(Color::Black).size();
    
    return (whiteMobility - blackMobility) * 10;
}

int Evaluator::evaluateKingSafety() const {
    int score = 0;
    
    // Упрощенная оценка безопасности короля
    Square whiteKing = board_.findKing(Color::White);
    Square blackKing = board_.findKing(Color::Black);
    
    if (whiteKing != 255 && !isEndgame()) {
        // Проверка наличия пешек перед королем
        int file = getFile(whiteKing);
        int rank = getRank(whiteKing);
        
        for (int df = -1; df <= 1; ++df) {
            int checkFile = file + df;
            if (checkFile >= 0 && checkFile < 8 && rank < 7) {
                Square frontSq = makeSquare(checkFile, rank + 1);
                const Piece& piece = board_.pieceAt(frontSq);
                if (piece.type() == PieceType::Pawn && piece.isWhite()) {
                    score += 10;
                }
            }
        }
    }
    
    if (blackKing != 255 && !isEndgame()) {
        int file = getFile(blackKing);
        int rank = getRank(blackKing);
        
        for (int df = -1; df <= 1; ++df) {
            int checkFile = file + df;
            if (checkFile >= 0 && checkFile < 8 && rank > 0) {
                Square frontSq = makeSquare(checkFile, rank - 1);
                const Piece& piece = board_.pieceAt(frontSq);
                if (piece.type() == PieceType::Pawn && piece.isBlack()) {
                    score -= 10;
                }
            }
        }
    }
    
    return score;
}

int Evaluator::evaluatePawnStructure() const {
    int score = 0;
    
    // Проверка сдвоенных пешек
    for (int file = 0; file < 8; ++file) {
        int whitePawns = 0;
        int blackPawns = 0;
        
        for (int rank = 0; rank < 8; ++rank) {
            Square sq = makeSquare(file, rank);
            const Piece& piece = board_.pieceAt(sq);
            
            if (piece.type() == PieceType::Pawn) {
                if (piece.isWhite()) whitePawns++;
                else blackPawns++;
            }
        }
        
        if (whitePawns > 1) score -= 20 * (whitePawns - 1);
        if (blackPawns > 1) score += 20 * (blackPawns - 1);
    }
    
    // Проверка изолированных пешек
    for (int file = 0; file < 8; ++file) {
        bool hasWhitePawn = false;
        bool hasBlackPawn = false;
        bool hasAdjacentWhite = false;
        bool hasAdjacentBlack = false;
        
        for (int rank = 0; rank < 8; ++rank) {
            Square sq = makeSquare(file, rank);
            const Piece& piece = board_.pieceAt(sq);
            
            if (piece.type() == PieceType::Pawn) {
                if (piece.isWhite()) hasWhitePawn = true;
                else hasBlackPawn = true;
            }
            
            // Проверка соседних вертикалей
            for (int df : {-1, 1}) {
                int adjFile = file + df;
                if (adjFile >= 0 && adjFile < 8) {
                    Square adjSq = makeSquare(adjFile, rank);
                    const Piece& adjPiece = board_.pieceAt(adjSq);
                    
                    if (adjPiece.type() == PieceType::Pawn) {
                        if (adjPiece.isWhite()) hasAdjacentWhite = true;
                        else hasAdjacentBlack = true;
                    }
                }
            }
        }
        
        if (hasWhitePawn && !hasAdjacentWhite) score -= 15;
        if (hasBlackPawn && !hasAdjacentBlack) score += 15;
    }
    
    return score;
}

bool Evaluator::isEndgame() const {
    int totalMaterial = 0;
    
    for (Square sq = 0; sq < NUM_SQUARES; ++sq) {
        const Piece& piece = board_.pieceAt(sq);
        if (!piece.isNone() && piece.type() != PieceType::King && piece.type() != PieceType::Pawn) {
            totalMaterial += piece.value();
        }
    }
    
    // Эндшпиль, если материала меньше 2600 (примерно 2 ладьи + конь/слон)
    return totalMaterial < 2600;
}

}} // namespace Chess::AI

