#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QApplication>
#include <QSettings>
#include <chrono>
#include <algorithm>
#include <random>

/**
 * @brief Constructor for the MainWindow.
 * Sets up the game engine, timers for Hard Mode, and the user interface.
 */
MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent), m_engine(Config::N, Config::M, Config::K)
{
    // Ensure the window can capture keyboard events immediately
    setFocusPolicy(Qt::StrongFocus);
    m_engine.setGenerationChances(Config::P, Config::Q);
    
    // Hard Mode logic: triggers a random move after 5 seconds of inactivity
    m_hardModeTimer = new QTimer(this);
    m_hardModeTimer->setSingleShot(true);
    connect(m_hardModeTimer, &QTimer::timeout, this, &MainWindow::makeRandomMove);
    
    // Precision timer for internal time-tracking
    m_displayTimer = new QTimer(this);
    connect(m_displayTimer, &QTimer::timeout, this, [this]() {
        m_timeLeftMs -= 100;
        if (m_timeLeftMs < 0) m_timeLeftMs = 0;
        if (m_mode == GameMode::Hard && !m_isGameOver) {
            m_hardBtn->setText(QString("Hard (%1s)").arg(QString::number(m_timeLeftMs / 1000.0, 'f', 1)));
        }
    });

    setupUI();
    updateUI();
}

/**
 * @brief Constructs the complex UI layout using Qt widgets and layouts.
 * Includes Title, Score boxes, Mode buttons, and the N x M Grid.
 */
void MainWindow::setupUI()
{
    setWindowTitle("2048");
    setFixedSize(500, 750);
    setStyleSheet("background-color: #faf8ef;");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(10);

    // HEADER SECTION: Title and Dynamic Score Boards
    auto *row1 = new QHBoxLayout();
    auto *titleLabel = new QLabel("2048", this);
    titleLabel->setStyleSheet("color: #776e65; font-size: 80px; font-weight: bold; font-family: 'Clear Sans', 'Helvetica Neue', Arial, sans-serif;");
    
    auto *scoreContainer = new QHBoxLayout();
    scoreContainer->setSpacing(5);
    
    auto *scoreBox = new QFrame();
    scoreBox->setFixedSize(90, 55);
    scoreBox->setStyleSheet("background-color: #bbada0; border-radius: 3px;");
    auto *vScore = new QVBoxLayout(scoreBox);
    vScore->setContentsMargins(5, 5, 5, 5);
    auto *sTitle = new QLabel("SCORE", scoreBox);
    sTitle->setAlignment(Qt::AlignCenter);
    sTitle->setStyleSheet("color: #eee4da; font-size: 13px; font-weight: bold;");
    m_scoreLabel = new QLabel("0", scoreBox);
    m_scoreLabel->setAlignment(Qt::AlignCenter);
    m_scoreLabel->setStyleSheet("color: white; font-size: 20px; font-weight: bold;");
    vScore->addWidget(sTitle);
    vScore->addWidget(m_scoreLabel);
    
    auto *bestBox = new QFrame();
    bestBox->setFixedSize(90, 55);
    bestBox->setStyleSheet("background-color: #bbada0; border-radius: 3px;");
    auto *vBest = new QVBoxLayout(bestBox);
    vBest->setContentsMargins(5, 5, 5, 5);
    auto *bTitle = new QLabel("BEST", bestBox);
    bTitle->setAlignment(Qt::AlignCenter);
    bTitle->setStyleSheet("color: #eee4da; font-size: 13px; font-weight: bold;");
    m_bestScoreLabel = new QLabel("0", bestBox);
    m_bestScoreLabel->setAlignment(Qt::AlignCenter);
    m_bestScoreLabel->setStyleSheet("color: white; font-size: 20px; font-weight: bold;");
    vBest->addWidget(bTitle);
    vBest->addWidget(m_bestScoreLabel);
    
    scoreContainer->addWidget(scoreBox);
    scoreContainer->addWidget(bestBox);
    
    row1->addWidget(titleLabel);
    row1->addStretch();
    row1->addLayout(scoreContainer);
    mainLayout->addLayout(row1);

    // ACTION SECTION: Subtitle and 'New Game' Control
    auto *row2 = new QHBoxLayout();
    auto *subtitleLabel = new QLabel("Join the tiles, get to 2048!", this);
    subtitleLabel->setStyleSheet("color: #776e65; font-size: 18px; font-weight: normal;");
    
    m_restartBtn = new QPushButton("New Game", this);
    m_restartBtn->setFixedSize(130, 40);
    m_restartBtn->setStyleSheet("QPushButton { background-color: #8f7a66; color: #f9f6f2; border-radius: 3px; font-size: 18px; font-weight: bold; }"
                                "QPushButton:hover { background-color: #9f8a76; }");
    // CRITICAL: Disable focus on buttons to prevent them from intercepting arrow keys in WASM
    m_restartBtn->setFocusPolicy(Qt::NoFocus);
    
    row2->addWidget(subtitleLabel);
    row2->addStretch();
    row2->addWidget(m_restartBtn);
    mainLayout->addLayout(row2);

    // MODE SELECTION: Normal, Unlimited, Hard and Undo controls
    auto *row3 = new QHBoxLayout();
    m_normalBtn = new QPushButton("Normal", this);
    m_unlimitedBtn = new QPushButton("Unlimited", this);
    m_hardBtn = new QPushButton("Hard", this);
    
    QString modeStyle = "QPushButton { background-color: #8f7a66; color: #f9f6f2; border-radius: 3px; font-size: 14px; font-weight: bold; padding: 8px; }"
                        "QPushButton:hover { background-color: #9f8a76; }";
    m_normalBtn->setStyleSheet(modeStyle);
    m_unlimitedBtn->setStyleSheet(modeStyle);
    m_hardBtn->setStyleSheet(modeStyle);
    m_normalBtn->setFocusPolicy(Qt::NoFocus);
    m_unlimitedBtn->setFocusPolicy(Qt::NoFocus);
    m_hardBtn->setFocusPolicy(Qt::NoFocus);
    
    row3->addWidget(m_normalBtn);
    row3->addWidget(m_unlimitedBtn);
    row3->addWidget(m_hardBtn);
    row3->addStretch();
    
    m_undoBtn = new QPushButton("Undo (U)", this);
    m_undoBtn->setFixedSize(100, 35);
    m_undoBtn->setStyleSheet(modeStyle);
    m_undoBtn->setFocusPolicy(Qt::NoFocus);
    row3->addWidget(m_undoBtn);
    
    mainLayout->addLayout(row3);

    // GAME GRID: Container for the tile labels
    m_gridContainer = new QFrame(this);
    m_gridContainer->setFixedSize(440, 440);
    m_gridContainer->setStyleSheet("background-color: #bbada0; border-radius: 6px;");
    
    auto *gridWidget = new QWidget(m_gridContainer);
    gridWidget->setGeometry(0, 0, 440, 440);
    m_gridLayout = new QGridLayout(gridWidget);
    m_gridLayout->setContentsMargins(15, 15, 15, 15);
    m_gridLayout->setSpacing(15);
    
    // Initialize labels for each grid cell
    for (int i = 0; i < Config::N; ++i) {
        std::vector<QLabel*> row;
        for (int j = 0; j < Config::M; ++j) {
            auto *label = new QLabel(gridWidget);
            label->setFixedSize(90, 90);
            label->setAlignment(Qt::AlignCenter);
            m_gridLayout->addWidget(label, i, j);
            row.push_back(label);
        }
        m_tileLabels.push_back(row);
    }

    // OVERLAY: Displayed on Win/Loss with semi-transparent background
    m_overlay = new QFrame(m_gridContainer);
    m_overlay->setGeometry(0, 0, 440, 440);
    m_overlay->setStyleSheet("background-color: rgba(238, 228, 218, 0.73); border-radius: 6px;");
    auto *vOverlay = new QVBoxLayout(m_overlay);
    auto *overLabel = new QLabel("Game over!", m_overlay);
    overLabel->setAlignment(Qt::AlignCenter);
    overLabel->setStyleSheet("color: #776e65; font-size: 60px; font-weight: bold; background: transparent;");
    auto *tryAgainBtn = new QPushButton("Try again", m_overlay);
    tryAgainBtn->setFixedSize(130, 40);
    tryAgainBtn->setStyleSheet("background-color: #8f7a66; color: #f9f6f2; border-radius: 3px; font-weight: bold; font-size: 18px;");
    tryAgainBtn->setFocusPolicy(Qt::NoFocus);
    vOverlay->addStretch();
    vOverlay->addWidget(overLabel);
    vOverlay->addSpacing(20);
    vOverlay->addWidget(tryAgainBtn, 0, Qt::AlignCenter);
    vOverlay->addStretch();
    m_overlay->hide();

    mainLayout->addWidget(m_gridContainer, 0, Qt::AlignCenter);
    mainLayout->addStretch();

    // Hidden label for compatibility with older UI drafts
    m_timerLabel = new QLabel(this); m_timerLabel->hide();

    // Connect UI signals to lambda slots for interactive control
    connect(m_undoBtn, &QPushButton::clicked, this, [this]() { if (!m_isGameOver) { m_engine.undo(); resetHardModeTimer(); updateUI(); } });
    connect(m_restartBtn, &QPushButton::clicked, this, [this]() { m_engine.reset(); m_isGameOver = false; m_overlay->hide(); resetHardModeTimer(); updateUI(); });
    connect(tryAgainBtn, &QPushButton::clicked, this, [this]() { m_engine.reset(); m_isGameOver = false; m_overlay->hide(); resetHardModeTimer(); updateUI(); });
    connect(m_normalBtn, &QPushButton::clicked, this, [this]() { m_mode = GameMode::Normal; resetHardModeTimer(); updateUI(); });
    connect(m_unlimitedBtn, &QPushButton::clicked, this, [this]() { m_mode = GameMode::Unlimited; resetHardModeTimer(); updateUI(); });
    connect(m_hardBtn, &QPushButton::clicked, this, [this]() { m_mode = GameMode::Hard; resetHardModeTimer(); updateUI(); });
}

/**
 * @brief Resets or stops the Hard Mode countdown timer based on the current state.
 */
void MainWindow::resetHardModeTimer()
{
    if (m_mode == GameMode::Hard && !m_isGameOver) {
        m_timeLeftMs = Config::HARD_MODE_TIMEOUT;
        m_hardModeTimer->start(Config::HARD_MODE_TIMEOUT);
        m_displayTimer->start(100);
    } else {
        m_hardModeTimer->stop();
        m_displayTimer->stop();
        m_hardBtn->setText("Hard");
    }
}

/**
 * @brief Synchronizes the visual labels with the GameEngine's state.
 * Handles Win/Loss condition displays and mode-specific color highlighting.
 */
void MainWindow::updateUI()
{
    const auto& grid = m_engine.getGrid();
    for (int i = 0; i < Config::N; ++i) {
        for (int j = 0; j < Config::M; ++j) {
            int val = grid[i][j];
            m_tileLabels[i][j]->setText(val == 0 ? "" : QString::number(val));
            m_tileLabels[i][j]->setStyleSheet(getTileStyle(val));
        }
    }
    m_scoreLabel->setText(QString::number(m_engine.getScore()));
    
    // Persistent Best Score handling
    QSettings settings("BounCmpE230", "2048");
    int currentBest = settings.value("bestScore", 0).toInt();
    if (m_engine.getBestScore() > currentBest) {
        settings.setValue("bestScore", m_engine.getBestScore());
        currentBest = m_engine.getBestScore();
    }
    m_bestScoreLabel->setText(QString::number(currentBest));

    // Highlight active mode button
    QString activeStyle = "background-color: #776e65; color: white;";
    QString inactiveStyle = "background-color: #8f7a66; color: #f9f6f2;";
    m_normalBtn->setStyleSheet(m_mode == GameMode::Normal ? activeStyle : inactiveStyle);
    m_unlimitedBtn->setStyleSheet(m_mode == GameMode::Unlimited ? activeStyle : inactiveStyle);
    m_hardBtn->setStyleSheet(m_mode == GameMode::Hard ? activeStyle : inactiveStyle);

    // Check game termination conditions
    if (!m_isGameOver) {
        if (m_engine.hasWon() && m_mode == GameMode::Normal) {
            m_isGameOver = true;
            m_overlay->setStyleSheet("background-color: rgba(237, 194, 46, 0.6); border-radius: 6px;");
            static_cast<QLabel*>(m_overlay->layout()->itemAt(1)->widget())->setText("You win!");
            m_overlay->show();
        } else if (!m_engine.canMove()) {
            m_isGameOver = true;
            m_overlay->setStyleSheet("background-color: rgba(238, 228, 218, 0.73); border-radius: 6px;");
            static_cast<QLabel*>(m_overlay->layout()->itemAt(1)->widget())->setText("Game over!");
            m_overlay->show();
        }
    }
    m_undoBtn->setEnabled(!m_isGameOver);
    
    // Disable mode buttons if the game has started (score > 0 or history not empty)
    bool gameStarted = m_engine.getScore() > 0 || m_engine.hasHistory();
    m_normalBtn->setEnabled(!gameStarted && !m_isGameOver);
    m_unlimitedBtn->setEnabled(!gameStarted && !m_isGameOver);
    m_hardBtn->setEnabled(!gameStarted && !m_isGameOver);
}

/**
 * @brief Executes a random valid move. Used for Hard Mode's automated penalty.
 */
void MainWindow::makeRandomMove()
{
    if (m_isGameOver || m_mode != GameMode::Hard) return;
    
    // PDF Compliance: "Forces a random move". 
    // We must try directions until one actually changes the grid.
    std::vector<Direction> dirs = {Direction::Left, Direction::Right, Direction::Up, Direction::Down};
    std::shuffle(dirs.begin(), dirs.end(), std::default_random_engine(std::chrono::system_clock::now().time_since_epoch().count()));
    
    for (Direction d : dirs) {
        if (m_engine.move(d)) {
            resetHardModeTimer(); 
            updateUI();
            return;
        }
    }
    // If no moves are possible, handleMoveResult will eventually be called by regular logic
}

/**
 * @brief Handles keyboard input for movement, undo, and restart.
 */
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (m_isGameOver && event->key() != Qt::Key_R) return;
    bool moved = false;
    switch (event->key()) {
        case Qt::Key_W: case Qt::Key_Up:    moved = m_engine.move(Direction::Up); break;
        case Qt::Key_S: case Qt::Key_Down:  moved = m_engine.move(Direction::Down); break;
        case Qt::Key_A: case Qt::Key_Left:  moved = m_engine.move(Direction::Left); break;
        case Qt::Key_D: case Qt::Key_Right: moved = m_engine.move(Direction::Right); break;
        case Qt::Key_U: if (!m_isGameOver) { m_engine.undo(); moved = true; } break;
        case Qt::Key_R: m_engine.reset(); m_isGameOver = false; m_overlay->hide(); moved = true; break;
    }
    if (moved) { resetHardModeTimer(); updateUI(); }
}

/**
 * @brief Low-level event filter to capture navigation keys before the browser.
 * This is essential for WebAssembly (WASM) compatibility.
 */
bool MainWindow::event(QEvent *event)
{
    // Low-level keyboard interception for WebAssembly.
    // This prevents the browser from consuming navigation keys (Arrows/Space) 
    // for scrolling, ensuring a smooth native-like gameplay experience.
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_Left || ke->key() == Qt::Key_Right || 
            ke->key() == Qt::Key_Up || ke->key() == Qt::Key_Down) {
            keyPressEvent(ke);
            return true; // Consume event to prevent browser scrolling
        }
    }
    return QWidget::event(event);
}

/**
 * @brief Maps tile values to their specific CSS styles (colors and fonts).
 */
QString MainWindow::getTileStyle(int value)
{
    QString base = "border-radius: 3px; font-weight: bold; font-family: 'Clear Sans', Arial, sans-serif; ";
    if (value == 0) return base + "background-color: #cdc1b4;";
    
    QString color = (value <= 4) ? "#776e65" : "#f9f6f2";
    QString bgColor;
    int fontSize = 35;

    switch (value) {
        case 2:    bgColor = "#eee4da"; break;
        case 4:    bgColor = "#ede0c8"; break;
        case 8:    bgColor = "#f2b179"; break;
        case 16:   bgColor = "#f59563"; break;
        case 32:   bgColor = "#f67c5f"; break;
        case 64:   bgColor = "#f65e3b"; break;
        case 128:  bgColor = "#edcf72"; fontSize = 30; break;
        case 256:  bgColor = "#edcc61"; fontSize = 30; break;
        case 512:  bgColor = "#edc850"; fontSize = 30; break;
        case 1024: bgColor = "#edc53f"; fontSize = 25; break;
        case 2048: bgColor = "#edc22e"; fontSize = 25; break;
        default:   bgColor = "#3c3a32"; fontSize = 20; break;
    }

    return base + QString("background-color: %1; color: %2; font-size: %3px;").arg(bgColor, color).arg(fontSize);
}
