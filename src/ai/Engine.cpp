#include "ai/Engine.h"
#include "ai/Evaluator.h"
#include "core/MoveGenerator.h"
#include <algorithm>
#include <chrono>
#include <limits>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cstdlib>

namespace Chess {
namespace AI {

Engine::Engine(Board& board) 
    : board_(board), maxDepth_(5), shouldStop_(false), logFilename_("chess_ai.log") {}

SearchResult Engine::findBestMove(Color color, int maxDepth) {
    maxDepth_ = maxDepth;
    shouldStop_ = false;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    MoveGenerator generator(board_);
    std::vector<Move> moves = generator.generateLegalMoves(color);
    
    if (moves.empty()) {
        return SearchResult{Move(), 0, 0, 0, 0.0};
    }
    
    // Упорядочить ходы
    orderMoves(moves, color);
    
    logSearchStart(color, maxDepth_, 0);
    
    Move bestMove = moves[0];
    int bestScore = std::numeric_limits<int>::min();
    std::atomic<int> nodesSearched(0);
    
    // Используем многопоточность только на первом уровне и если ходов достаточно
    bool useParallel = (moves.size() >= 4 && maxDepth_ >= 3);
    
    if (useParallel) {
        // Многопоточный поиск - используем FEN для создания копий доски
        std::string fenBefore = board_.toFEN();
        int searchDepth = maxDepth_;  // Сохраняем в локальную переменную
        std::vector<std::future<std::pair<Move, int>>> futures;
        
        log("Используется многопоточный поиск (" + std::to_string(moves.size()) + " ходов)");
        
        for (const Move& move : moves) {
            if (shouldStop_) break;
            
            futures.push_back(std::async(std::launch::async, [move, searchDepth, color, fenBefore, &nodesSearched]() {
                // Создаем копию доски через FEN
                Board boardCopy;
                boardCopy.setFromFEN(fenBefore);
                Engine threadEngine(boardCopy);
                threadEngine.setDifficulty(searchDepth);
                threadEngine.setLogFile("");  // Отключаем логирование в потоках
                
                boardCopy.makeMove(move);
                int localNodes = 0;
                int score = -threadEngine.alphaBeta(searchDepth - 1, 
                                                    std::numeric_limits<int>::min(), 
                                                    std::numeric_limits<int>::max(), 
                                                    oppositeColor(color), 
                                                    localNodes);
                nodesSearched += localNodes;
                return std::make_pair(move, score);
            }));
        }
        
        // Собираем результаты
        for (size_t i = 0; i < futures.size(); ++i) {
            if (shouldStop_) break;
            
            auto result = futures[i].get();
            int score = result.second;
            
            logMoveEvaluation(result.first, score, maxDepth_, nodesSearched.load());
            
            if (score > bestScore) {
                bestScore = score;
                bestMove = result.first;
                
                if (progressCallback_) {
                    progressCallback_(maxDepth_, score, result.first);
                }
            }
        }
    } else {
        // Однопоточный поиск
        std::string currentFen = board_.toFEN();
        for (const Move& move : moves) {
            if (shouldStop_) break;
            
            board_.makeMove(move);
            int localNodes = 0;
            int score = -alphaBeta(maxDepth_ - 1, 
                                   std::numeric_limits<int>::min(), 
                                   std::numeric_limits<int>::max(), 
                                   oppositeColor(color), 
                                   localNodes);
            nodesSearched += localNodes;
            
            // Проверяем повторение позиции и применяем штраф
            std::string fenAfter = board_.toFEN();
            int repetitionPenalty = checkPositionRepetition(fenAfter);
            if (repetitionPenalty > 0) {
                score -= repetitionPenalty;
                log("  Штраф за повторение позиции: -" + std::to_string(repetitionPenalty));
            }
            
            // Оценка эндшпиля - если у противника только король, приближаемся к нему
            int endgameBonus = evaluateEndgameMate(board_, color);
            if (endgameBonus != 0) {
                score += endgameBonus;
                log("  Бонус за эндшпиль: +" + std::to_string(endgameBonus));
            }
            
            board_.unmakeMove(move);
            
            if (shouldStop_) break;
            
            logMoveEvaluation(move, score, maxDepth_, nodesSearched.load());
            
            if (score > bestScore) {
                bestScore = score;
                bestMove = move;
                
                if (progressCallback_) {
                    progressCallback_(maxDepth_, score, move);
                }
            }
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    SearchResult result{
        bestMove,
        bestScore,
        maxDepth_,
        nodesSearched.load(),
        duration.count() / 1000.0
    };
    
    logSearchResult(result);
    
    return result;
}

SearchResult Engine::findBestMoveWithTimeLimit(Color color, int timeMs) {
    // Начинаем с глубины 1 и увеличиваем (iterative deepening)
    SearchResult lastResult;
    shouldStop_ = false;
    
    log("=== Начало поиска с ограничением по времени ===");
    logSearchStart(color, maxDepth_, timeMs);
    
    auto startTime = std::chrono::high_resolution_clock::now();
    auto deadline = startTime + std::chrono::milliseconds(timeMs);
    
    for (int depth = 1; depth <= maxDepth_; ++depth) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        if (currentTime >= deadline) {
            log("Время истекло, используем результат с глубины " + std::to_string(depth - 1));
            break;
        }
        
        // Проверяем время во время поиска
        int remainingTime = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - currentTime).count();
        if (remainingTime < 100) {  // Если осталось меньше 100мс, не начинаем новую итерацию
            log("Осталось мало времени (" + std::to_string(remainingTime) + "мс), используем предыдущий результат");
            break;
        }
        
        log("Поиск на глубине " + std::to_string(depth) + ", осталось времени: " + std::to_string(remainingTime) + "мс");
        lastResult = findBestMove(color, depth);
        
        // Проверяем, не вышли ли за время
        currentTime = std::chrono::high_resolution_clock::now();
        if (currentTime >= deadline) {
            shouldStop_ = true;
            log("Время истекло во время поиска");
            break;
        }
    }
    
    shouldStop_ = false;
    log("=== Конец поиска ===");
    return lastResult;
}

int Engine::alphaBeta(int depth, int alpha, int beta, Color color, int& nodesSearched) {
    nodesSearched++;
    
    if (shouldStop_) {
        return 0;
    }
    
    if (depth == 0) {
        return quiescence(alpha, beta, color, nodesSearched, 0);
    }
    
    MoveGenerator generator(board_);
    std::vector<Move> moves = generator.generateLegalMoves(color);
    
    // Мат или пат
    if (moves.empty()) {
        if (board_.isCheck(color)) {
            // Мат - очень плохо
            return std::numeric_limits<int>::min() + 1;
        } else {
            // Пат - ничья
            return 0;
        }
    }
    
    // Упорядочить ходы
    orderMoves(moves, color);
    
    int maxScore = std::numeric_limits<int>::min();
    
    for (const Move& move : moves) {
        if (shouldStop_) break;
        
        board_.makeMove(move);
        int score = -alphaBeta(depth - 1, -beta, -alpha, oppositeColor(color), nodesSearched);
        board_.unmakeMove(move);
        
        if (shouldStop_) break;
        
        if (score > maxScore) {
            maxScore = score;
        }
        
        if (score > alpha) {
            alpha = score;
        }
        
        // Beta cutoff
        if (alpha >= beta) {
            break;
        }
    }
    
    return maxScore;
}

int Engine::quiescence(int alpha, int beta, Color color, int& nodesSearched, int depth) {
    nodesSearched++;
    
    if (shouldStop_) {
        return 0;
    }
    
    // Ограничиваем глубину quiescence search для ускорения
    static const int MAX_QUIESCENCE_DEPTH = 3;
    
    if (depth >= MAX_QUIESCENCE_DEPTH) {
        Evaluator evaluator(board_);
        int standPat = evaluator.evaluate();
        return (color == Color::Black) ? -standPat : standPat;
    }
    
    // Статическая оценка
    Evaluator evaluator(board_);
    int standPat = evaluator.evaluate();
    
    if (color == Color::Black) {
        standPat = -standPat;
    }
    
    if (standPat >= beta) {
        return beta;
    }
    
    if (alpha < standPat) {
        alpha = standPat;
    }
    
    // Рассмотрим только взятия
    MoveGenerator generator(board_);
    std::vector<Move> captures = generator.generateCaptures(color);
    
    // Упорядочиваем взятия для лучшего отсечения
    std::sort(captures.begin(), captures.end(), [this, color](const Move& a, const Move& b) {
        return scoreMoveForOrdering(a, color) > scoreMoveForOrdering(b, color);
    });
    
    for (const Move& capture : captures) {
        if (shouldStop_) break;
        
        board_.makeMove(capture);
        int score = -quiescence(-beta, -alpha, oppositeColor(color), nodesSearched, depth + 1);
        board_.unmakeMove(capture);
        
        if (score >= beta) {
            return beta;
        }
        
        if (score > alpha) {
            alpha = score;
        }
    }
    
    return alpha;
}

void Engine::orderMoves(std::vector<Move>& moves, Color color) const {
    // Простая эвристика упорядочивания:
    // 1. Взятия (MVV-LVA - Most Valuable Victim - Least Valuable Attacker)
    // 2. Обычные ходы
    
    std::sort(moves.begin(), moves.end(), [this, color](const Move& a, const Move& b) {
        return scoreMoveForOrdering(a, color) > scoreMoveForOrdering(b, color);
    });
}

int Engine::scoreMoveForOrdering(const Move& move, Color color) const {
    int score = 0;
    
    // Взятия более приоритетны
    if (move.isCapture()) {
        const Piece& victim = board_.pieceAt(move.to());
        const Piece& attacker = board_.pieceAt(move.from());
        
        // MVV-LVA: предпочитаем брать ценные фигуры дешевыми
        score = victim.value() * 10 - attacker.value();
    }
    
    // Превращения пешек очень ценны
    if (move.isPromotion()) {
        score += 8000;
    }
    
    // Центральные ходы лучше
    int toFile = getFile(move.to());
    int toRank = getRank(move.to());
    int centerDist = (toFile > 3 ? toFile - 3 : 3 - toFile) + (toRank > 3 ? toRank - 3 : 3 - toRank);
    score += (14 - centerDist) * 10;
    
    return score;
}

void Engine::log(const std::string& message) {
    if (logFilename_.empty()) return;
    
    std::lock_guard<std::mutex> lock(logMutex_);
    
    std::ofstream logFile(logFilename_, std::ios::app);
    if (logFile.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        ss << "." << std::setfill('0') << std::setw(3) << ms.count();
        
        logFile << "[" << ss.str() << "] " << message << std::endl;
    }
}

void Engine::logSearchStart(Color color, int depth, int timeLimit) {
    std::stringstream ss;
    ss << "Поиск начат: цвет=" << (color == Color::White ? "White" : "Black");
    ss << ", глубина=" << depth;
    if (timeLimit > 0) {
        ss << ", лимит времени=" << timeLimit << "мс";
    }
    ss << ", FEN=" << board_.toFEN();
    log(ss.str());
}

void Engine::logMoveEvaluation(const Move& move, int score, int depth, int nodes) {
    std::stringstream ss;
    ss << "  Ход: " << move.toLongAlgebraic() << " | оценка: " << score 
       << " | глубина: " << depth << " | узлов: " << nodes;
    log(ss.str());
}

void Engine::logSearchResult(const SearchResult& result) {
    std::stringstream ss;
    ss << "Результат поиска: ход=" << result.bestMove.toLongAlgebraic()
       << " | оценка=" << result.score
       << " | глубина=" << result.depth
       << " | узлов=" << result.nodesSearched
       << " | время=" << std::fixed << std::setprecision(2) << result.timeSpent << "с";
    log(ss.str());
}

int Engine::checkPositionRepetition(const std::string& fen) const {
    // Извлекаем только позицию фигур из FEN (до первого пробела)
    // Это позволяет игнорировать информацию о ходе, рокировке и т.д.
    size_t spacePos = fen.find(' ');
    std::string positionOnly = (spacePos != std::string::npos) ? fen.substr(0, spacePos) : fen;
    
    // Используем thread_local для безопасности в многопоточности
    static thread_local std::vector<std::string> recentPositions;
    
    // Проверяем, встречалась ли эта позиция недавно
    int count = 0;
    for (const auto& pos : recentPositions) {
        if (pos == positionOnly) {
            count++;
        }
    }
    
    // Если позиция повторяется 2 или более раз, применяем штраф
    if (count >= 2) {
        return 500;  // Штраф за повторение позиции
    }
    
    // Добавляем текущую позицию в историю (храним последние 10 позиций)
    recentPositions.push_back(positionOnly);
    if (recentPositions.size() > 10) {
        recentPositions.erase(recentPositions.begin());
    }
    
    return 0;
}

bool Engine::isKingOnlyEndgame(const Board& board, Color color) const {
    // Проверяем, остались ли у противника только король
    Color opponentColor = oppositeColor(color);
    int pieceCount = 0;
    
    for (Square sq = 0; sq < NUM_SQUARES; ++sq) {
        const Piece& piece = board.pieceAt(sq);
        if (!piece.isNone() && piece.color() == opponentColor) {
            if (piece.type() == PieceType::King) {
                pieceCount++;
            } else {
                return false;  // Есть не-король
            }
        }
    }
    
    return pieceCount == 1;  // Только король
}

int Engine::evaluateEndgameMate(const Board& board, Color color) const {
    // Если у противника только король, нужно приближаться к нему для мата
    if (!isKingOnlyEndgame(board, color)) {
        return 0;
    }
    
    Color opponentColor = oppositeColor(color);
    Square opponentKing = board.findKing(opponentColor);
    Square myKing = board.findKing(color);
    
    if (opponentKing == 255 || myKing == 255) {
        return 0;
    }
    
    // Вычисляем расстояние между королями
    int opponentFile = getFile(opponentKing);
    int opponentRank = getRank(opponentKing);
    int myFile = getFile(myKing);
    int myRank = getRank(myKing);
    
    int fileDiff = (opponentFile > myFile) ? (opponentFile - myFile) : (myFile - opponentFile);
    int rankDiff = (opponentRank > myRank) ? (opponentRank - myRank) : (myRank - opponentRank);
    int distance = fileDiff + rankDiff;
    
    // Бонус за приближение к королю противника (максимальное расстояние = 14)
    // Чем ближе, тем больше бонус
    int bonus = (14 - distance) * 20;
    
    // Также проверяем, можем ли мы атаковать короля
    if (board.isSquareAttacked(opponentKing, color)) {
        bonus += 100;  // Дополнительный бонус за атаку короля
    }
    
    return bonus;
}

}} // namespace Chess::AI

