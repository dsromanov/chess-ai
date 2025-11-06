#include "ui/MainWindow.h"
#include "core/MoveGenerator.h"
#include <QMenuBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>

namespace Chess {
namespace UI {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      playingAgainstAi_(true),
      playerColor_(Color::White),
      aiColor_(Color::Black),
      selectedSquare_(255)
{
    board_ = std::make_shared<Board>();
    board_->setupInitialPosition();
    
    aiEngine_ = std::make_unique<AI::Engine>(*board_);
    aiEngine_->setDifficulty(3); // Средняя сложность по умолчанию (было 2, но даже 3 быстрее чем старая 5)
    aiEngine_->setLogFile("chess_ai.log");  // Включаем логирование
    
    setupUi();
    createMenuBar();
    connectSignals();
    
    // Показываем диалог выбора цвета
    showColorSelectionDialog();
    
    updateStatus();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi() {
    setWindowTitle("Chess AI - Шахматы с AI");
    resize(900, 600);
    
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    
    // Шахматная доска
    chessBoard_ = new ChessBoard(this);
    chessBoard_->setBoard(board_);
    mainLayout->addWidget(chessBoard_, 2);
    
    // Боковая панель
    QVBoxLayout* sideLayout = new QVBoxLayout();
    
    // Статус
    statusLabel_ = new QLabel("Ход белых", this);
    statusLabel_->setStyleSheet("font-size: 16px; font-weight: bold; padding: 10px;");
    sideLayout->addWidget(statusLabel_);
    
    // Сложность AI
    QLabel* diffLabel = new QLabel("Сложность AI:", this);
    sideLayout->addWidget(diffLabel);
    
    difficultyCombo_ = new QComboBox(this);
    difficultyCombo_->addItem("Очень легко (глубина 2)");
    difficultyCombo_->addItem("Легко (глубина 3)");
    difficultyCombo_->addItem("Средне (глубина 4)");
    difficultyCombo_->addItem("Сложно (глубина 5)");
    difficultyCombo_->setCurrentIndex(1); // По умолчанию "Легко (3)"
    sideLayout->addWidget(difficultyCombo_);
    
    // Кнопки
    newGameButton_ = new QPushButton("Новая игра", this);
    sideLayout->addWidget(newGameButton_);
    
    undoButton_ = new QPushButton("Отменить ход", this);
    sideLayout->addWidget(undoButton_);
    
    // История ходов
    QLabel* movesLabel = new QLabel("История ходов:", this);
    sideLayout->addWidget(movesLabel);
    
    moveList_ = new QListWidget(this);
    sideLayout->addWidget(moveList_);
    
    sideLayout->addStretch();
    
    mainLayout->addLayout(sideLayout, 1);
}

void MainWindow::createMenuBar() {
    QMenu* gameMenu = menuBar()->addMenu("Игра");
    
    QAction* newGameAction = gameMenu->addAction("Новая игра");
    connect(newGameAction, &QAction::triggered, this, &MainWindow::onNewGame);
    
    QAction* exitAction = gameMenu->addAction("Выход");
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);
    
    QMenu* helpMenu = menuBar()->addMenu("Помощь");
    QAction* aboutAction = helpMenu->addAction("О программе");
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, "О программе", 
            "Chess AI v1.0\n\n"
            "Шахматный движок с AI на основе алгоритма Minimax с Alpha-Beta отсечением.\n\n"
            "Функции:\n"
            "- Полная реализация шахматных правил\n"
            "- AI с разными уровнями сложности\n"
            "- Красивый графический интерфейс\n"
            "- История ходов\n\n"
            "Создано на C++ с Qt и любовью ❤");
    });
}

void MainWindow::connectSignals() {
    connect(chessBoard_, &ChessBoard::moveRequested, this, &MainWindow::onMoveRequested);
    connect(chessBoard_, &ChessBoard::squareClicked, this, &MainWindow::onSquareClicked);
    connect(newGameButton_, &QPushButton::clicked, this, &MainWindow::onNewGame);
    connect(undoButton_, &QPushButton::clicked, this, &MainWindow::onUndoMove);
    connect(difficultyCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onDifficultyChanged);
}

void MainWindow::onMoveRequested(const Move& move) {
    // ChessBoard уже проверил, что это фигура игрока и его ход
    makeMove(move);
    
    // Если играем против AI и теперь ход AI
    if (playingAgainstAi_ && board_->position().sideToMove() == aiColor_) {
        // Небольшая задержка перед ходом AI
        QTimer::singleShot(300, this, &MainWindow::onAiMove);
    }
}

void MainWindow::onSquareClicked(Square sq) {
    selectedSquare_ = sq;
}

void MainWindow::onNewGame() {
    board_->setupInitialPosition();
    chessBoard_->clearHighlights();
    moveHistory_.clear();
    moveList_->clear();
    
    // Показываем диалог выбора цвета
    showColorSelectionDialog();
    
    chessBoard_->update();
    updateStatus();
    updateMoveList();
}

void MainWindow::showColorSelectionDialog() {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Новая игра");
    msgBox.setText("Выберите, за какой цвет вы хотите играть:");
    
    msgBox.addButton("Играть за белых", QMessageBox::ActionRole);
    QPushButton* blackButton = msgBox.addButton("Играть за черных", QMessageBox::ActionRole);
    
    msgBox.exec();
    
    if (msgBox.clickedButton() == blackButton) {
        playerColor_ = Color::Black;
        aiColor_ = Color::White;
        chessBoard_->setPlayerColor(Color::Black); // Устанавливаем цвет игрока
        chessBoard_->setFlipped(true); // Переворачиваем доску для черных
        
        // AI ходит первым
        QTimer::singleShot(500, this, &MainWindow::onAiMove);
    } else {
        playerColor_ = Color::White;
        aiColor_ = Color::Black;
        chessBoard_->setPlayerColor(Color::White); // Устанавливаем цвет игрока
        chessBoard_->setFlipped(false);
    }
}

void MainWindow::onDifficultyChanged(int index) {
    // Легко=2, Средне=3, Сложно=4, Эксперт=5
    int depth = 2 + index;
    aiEngine_->setDifficulty(depth);
    qDebug() << "Установлена сложность AI: глубина" << depth;
}

void MainWindow::onUndoMove() {
    if (moveHistory_.empty()) return;
    
    Move lastMove = moveHistory_.back();
    board_->unmakeMove(lastMove);
    moveHistory_.pop_back();
    
    // Если играем против AI, отменяем также ход AI
    if (playingAgainstAi_ && !moveHistory_.empty()) {
        Move aiMove = moveHistory_.back();
        board_->unmakeMove(aiMove);
        moveHistory_.pop_back();
    }
    
    chessBoard_->clearHighlights();
    chessBoard_->update();
    updateStatus();
    updateMoveList();
}

void MainWindow::onAiMove() {
    if (!playingAgainstAi_ || board_->position().sideToMove() != aiColor_) {
        return;
    }
    
    statusLabel_->setText("AI думает...");
    statusLabel_->repaint();
    
    // ВАЖНО: Сохраняем FEN до поиска, чтобы восстановить состояние
    std::string fenBeforeSearch = board_->toFEN();
    
    // Используем ограничение по времени для быстрого ответа (1.5 секунды)
    AI::SearchResult result = aiEngine_->findBestMoveWithTimeLimit(aiColor_, 1500);
    
    // Восстанавливаем состояние доски после поиска AI
    board_->setFromFEN(fenBeforeSearch);
    
    if (result.bestMove.isValid()) {
        makeMove(result.bestMove);
        
        // Показываем информацию о поиске в консоли
        qDebug() << QString("AI ход: %1 | глубина: %2 | оценка: %3 | узлов: %4 | время: %5с")
            .arg(QString::fromStdString(result.bestMove.toLongAlgebraic()))
            .arg(result.depth)
            .arg(result.score)
            .arg(result.nodesSearched)
            .arg(result.timeSpent, 0, 'f', 2);
    }
}

void MainWindow::makeMove(const Move& move) {
    board_->makeMove(move);
    moveHistory_.push_back(move);
    
    chessBoard_->clearHighlights();
    chessBoard_->update();
    
    updateStatus();
    updateMoveList();
    checkGameState();
}

void MainWindow::updateStatus() {
    Color current = board_->position().sideToMove();
    QString colorStr = (current == Color::White) ? "белых" : "черных";
    
    // Проверяем мат
    if (isGameOver()) {
        if (board_->isCheck(current)) {
            // Мат
            QString winner = (current == Color::White) ? "Черные" : "Белые";
            statusLabel_->setText(QString("ШАХ И МАТ! %1 победили!").arg(winner));
            statusLabel_->setStyleSheet("font-size: 18px; font-weight: bold; padding: 10px; color: red; background-color: #ffeeee;");
        } else if (board_->isDraw()) {
            statusLabel_->setText("НИЧЬЯ (правило 50 ходов)");
            statusLabel_->setStyleSheet("font-size: 16px; font-weight: bold; padding: 10px; color: blue;");
        } else {
            // Пат
            statusLabel_->setText("ПАТ - Ничья");
            statusLabel_->setStyleSheet("font-size: 16px; font-weight: bold; padding: 10px; color: blue;");
        }
    } else if (board_->isCheck(current)) {
        statusLabel_->setText(QString("Шах! Ход %1").arg(colorStr));
        statusLabel_->setStyleSheet("font-size: 16px; font-weight: bold; padding: 10px; color: red;");
    } else {
        statusLabel_->setText(QString("Ход %1").arg(colorStr));
        statusLabel_->setStyleSheet("font-size: 16px; font-weight: bold; padding: 10px; color: black;");
    }
}

void MainWindow::updateMoveList() {
    moveList_->clear();
    
    for (size_t i = 0; i < moveHistory_.size(); i += 2) {
        QString moveStr;
        
        int moveNum = i / 2 + 1;
        moveStr += QString("%1. ").arg(moveNum);
        
        // Ход белых
        moveStr += QString::fromStdString(moveHistory_[i].toLongAlgebraic());
        
        // Ход черных (если есть)
        if (i + 1 < moveHistory_.size()) {
            moveStr += " " + QString::fromStdString(moveHistory_[i + 1].toLongAlgebraic());
        }
        
        moveList_->addItem(moveStr);
    }
    
    // Прокрутка вниз
    moveList_->scrollToBottom();
}

bool MainWindow::isGameOver() {
    // Проверяем через генератор ходов
    MoveGenerator generator(*board_);
    Color current = board_->position().sideToMove();
    std::vector<Move> moves = generator.generateLegalMoves(current);
    
    return moves.empty() || board_->isDraw();
}

void MainWindow::checkGameState() {
    if (isGameOver()) {
        Color current = board_->position().sideToMove();
        
        if (board_->isCheck(current)) {
            // Мат
            QString winner = (current == Color::White) ? "Черные" : "Белые";
            QMessageBox::information(this, "Игра окончена", 
                QString("%1 победили! Мат.").arg(winner));
        } else if (board_->isDraw()) {
            // Ничья по правилу 50 ходов
            QMessageBox::information(this, "Игра окончена", "Ничья по правилу 50 ходов.");
        } else {
            // Пат
            QMessageBox::information(this, "Игра окончена", "Пат. Ничья.");
        }
    }
}

}} // namespace Chess::UI


