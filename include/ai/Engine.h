#pragma once

#include "core/Board.h"
#include "core/Move.h"
#include <functional>
#include <atomic>
#include <fstream>
#include <mutex>
#include <thread>
#include <vector>
#include <future>
#include <unordered_set>
#include <string>

namespace Chess {
namespace AI {

struct SearchResult {
    Move bestMove;
    int score;
    int depth;
    int nodesSearched;
    double timeSpent;
};

class Engine {
public:
    explicit Engine(Board& board);

    // Найти лучший ход
    SearchResult findBestMove(Color color, int maxDepth = 5);

    // Найти лучший ход с ограничением по времени (миллисекунды)
    SearchResult findBestMoveWithTimeLimit(Color color, int timeMs);

    // Установить сложность (глубина поиска)
    void setDifficulty(int depth) { maxDepth_ = depth; }
    int getDifficulty() const { return maxDepth_; }

    // Callback для прогресса
    using ProgressCallback = std::function<void(int depth, int score, const Move& move)>;
    void setProgressCallback(ProgressCallback callback) { progressCallback_ = callback; }

    // Остановить поиск
    void stop() { shouldStop_ = true; }

    // Установить файл для логирования
    void setLogFile(const std::string& filename) { logFilename_ = filename; }

private:
    Board& board_;
    int maxDepth_;
    std::atomic<bool> shouldStop_;
    ProgressCallback progressCallback_;
    std::string logFilename_;
    std::mutex logMutex_;

    // Minimax с alpha-beta отсечением
    int alphaBeta(int depth, int alpha, int beta, Color color, int& nodesSearched);

    // Quiescence search для стабильной оценки
    int quiescence(int alpha, int beta, Color color, int& nodesSearched, int depth = 0);

    // Упорядочивание ходов для лучшего отсечения
    void orderMoves(std::vector<Move>& moves, Color color) const;

    // Оценка хода для упорядочивания
    int scoreMoveForOrdering(const Move& move, Color color) const;
    
    // Логирование
    void log(const std::string& message);
    void logSearchStart(Color color, int depth, int timeLimit);
    void logMoveEvaluation(const Move& move, int score, int depth, int nodes);
    void logSearchResult(const SearchResult& result);
    
    // Обнаружение повторений и оценка эндшпиля
    int checkPositionRepetition(const std::string& fen) const;
    int evaluateEndgameMate(const Board& board, Color color) const;
    bool isKingOnlyEndgame(const Board& board, Color color) const;
};

}} // namespace Chess::AI

