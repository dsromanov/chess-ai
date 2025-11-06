#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QListWidget>
#include <QComboBox>
#include <QPushButton>
#include <memory>
#include "ui/ChessBoard.h"
#include "core/Board.h"
#include "ai/Engine.h"

namespace Chess {
namespace UI {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onMoveRequested(const Move& move);
    void onSquareClicked(Square sq);
    void onNewGame();
    void onDifficultyChanged(int index);
    void onUndoMove();
    void onAiMove();

private:
    // UI компоненты
    ChessBoard* chessBoard_;
    QLabel* statusLabel_;
    QListWidget* moveList_;
    QComboBox* difficultyCombo_;
    QPushButton* newGameButton_;
    QPushButton* undoButton_;

    // Игровая логика
    std::shared_ptr<Board> board_;
    std::unique_ptr<AI::Engine> aiEngine_;
    
    // Состояние игры
    bool playingAgainstAi_;
    Color playerColor_;
    Color aiColor_;
    std::vector<Move> moveHistory_;
    Square selectedSquare_;

    // Методы
    void setupUi();
    void createMenuBar();
    void connectSignals();
    void showColorSelectionDialog();
    
    void makeMove(const Move& move);
    void updateStatus();
    void updateMoveList();
    bool isGameOver();
    void checkGameState();
    
    // AI
    void triggerAiMove();
};

}} // namespace Chess::UI

