/**
 * @file mainwindow.cpp
 * @brief Implementation of the main application window and game UI.
 */

#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSettings>
#include <QDateTime>
#include <QGraphicsDropShadowEffect>
#include <QDebug>
#include <QKeyEvent>

static const QString FONT_STACK = "-apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif";

// --- Section 2.3: Configuration Constants ---
static const int N = 4;        // Grid Rows
static const int M = 4;        // Grid Columns
static const int K = 2048;     // Target Win Tile
static const int P = 90;       // Probability (%) for tile '2'
static const int Q = 10;       // Probability (%) for tile '4'
// --------------------------------------------

static QString getModeButtonStyle(bool active) {
    QString color = active ? "#8f7a66" : "#bbada0";
    return QString("QPushButton { background-color: %1; color: white; font-weight: bold; border-radius: 6px; font-size: 16px; border: none; } "
                   "QPushButton:hover { background-color: #9f8a76; }").arg(color);
}

TileWidget::TileWidget(QWidget *parent) : QWidget(parent), m_value(0) {
    setFixedSize(90, 100); // Extreme compact sizing
    setAttribute(Qt::WA_TransparentForMouseEvents);
    m_face = new QWidget(this);
    m_face->setGeometry(0, 0, 90, 90);
#ifndef Q_OS_WASM
    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(8); shadow->setOffset(0, 3); shadow->setColor(QColor(0, 0, 0, 40));
    m_face->setGraphicsEffect(shadow);
#endif
    m_label = new QLabel(m_face); m_label->setGeometry(0, 0, 90, 90); m_label->setAlignment(Qt::AlignCenter); m_label->show();
    hide();
}

void TileWidget::setValue(int val) {
    if (val == 0) { if (m_value != 0) { m_value = 0; hide(); } return; }
    m_value = val;
    m_label->setText(QString::number(val));
    m_face->setStyleSheet(getTileStyle(val));
    if (!isVisible()) show();
}

void TileWidget::animatePopIn() {
    show(); m_face->show();
    auto *a = new QPropertyAnimation(m_face, "geometry");
    a->setDuration(250); a->setStartValue(QRect(45, 45, 0, 0)); a->setEndValue(QRect(0, 0, 90, 90));
    a->setEasingCurve(QEasingCurve::OutBack); a->start(QAbstractAnimation::DeleteWhenStopped);
}

QString TileWidget::getTileStyle(int value) {
    QString b = "border-radius: 8px; font-weight: bold; font-family: " + FONT_STACK + "; font-size: 32px; ";
    switch (value) {
        case 2:    return b + "background-color: #eee4da; color: #776e65;";
        case 4:    return b + "background-color: #ede0c8; color: #776e65;";
        case 8:    return b + "background-color: #f2b179; color: white;";
        case 16:   return b + "background-color: #f59563; color: white;";
        case 32:   return b + "background-color: #f67c5f; color: white;";
        case 64:   return b + "background-color: #f65e3b; color: white;";
        case 128:  return b + "background-color: #edcf72; color: white; font-size: 28px;";
        case 256:  return b + "background-color: #edcc61; color: white; font-size: 28px;";
        case 512:  return b + "background-color: #edc850; color: white; font-size: 28px;";
        case 1024: return b + "background-color: #edc53f; color: white; font-size: 22px;";
        case 2048: return b + "background-color: #edc22e; color: white; font-size: 22px;";
        default:   return b + "background-color: #3c3a32; color: white; font-size: 20px;";
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent), m_engine(N, M, K)
{
    setFocusPolicy(Qt::StrongFocus);
    m_engine.setGenerationChances(P / 100.0);
    QSettings s("Group1", "2048"); m_engine.setBestScore(s.value("bestScore", 0).toInt());
    m_isDebugMode = false;
    
    m_hardModeTimer = new QTimer(this); m_hardModeTimer->setSingleShot(true);
    connect(m_hardModeTimer, &QTimer::timeout, this, &MainWindow::makeRandomMove);
    m_displayTimer = new QTimer(this);
    connect(m_displayTimer, &QTimer::timeout, this, [this]() {
        m_timeLeftMs -= 100; if (m_timeLeftMs < 0) m_timeLeftMs = 0;
        if (m_mode == GameMode::Hard && !m_isGameOver) m_hardBtn->setText(QString("Hard (3) [%1s]").arg(QString::number(m_timeLeftMs/1000.0,'f',1)));
    });
    
    setupUI();
    m_engine.reset();
    m_lastBestScore = m_engine.getBestScore();
    QTimer::singleShot(0, this, [this]() { updateUI(); });
}

void MainWindow::setupUI() {
    setWindowTitle("2048"); setFixedSize(480, 700); setStyleSheet("background-color: #faf8ef;");
    auto *mainLayout = new QVBoxLayout(this); mainLayout->setContentsMargins(15, 10, 15, 15); mainLayout->setSpacing(6);

    // --- Header ---
    auto *headerRow = new QHBoxLayout(); headerRow->setAlignment(Qt::AlignTop);
    
    auto *titleContainer = new QWidget(this); titleContainer->setFixedHeight(70);
    auto *titleGroup = new QVBoxLayout(titleContainer); titleGroup->setContentsMargins(0,0,0,0); titleGroup->setSpacing(0);
    titleGroup->setAlignment(Qt::AlignTop); 

    auto *title = new QLabel("2048", titleContainer); title->setStyleSheet("color: #776e65; font-size: 56px; font-weight: bold; margin-top: -10px;");
    auto *instr = new QLabel("Join the tiles, get to 2048!", titleContainer); instr->setStyleSheet("color: #776e65; font-size: 14px; font-weight: 500;");
    titleGroup->addWidget(title); titleGroup->addWidget(instr); titleGroup->addStretch();
    
    auto *scoreRow = new QHBoxLayout(); scoreRow->setSpacing(10); scoreRow->setAlignment(Qt::AlignTop);
    m_scoreBox = new QFrame(); m_scoreBox->setFixedSize(110, 90); m_scoreBox->setStyleSheet("background-color: #bbada0; border-radius: 6px;");
    auto *vs = new QVBoxLayout(m_scoreBox); vs->setContentsMargins(0,12,0,12); vs->setSpacing(0);
    m_scoreTitle = new QLabel("SCORE", m_scoreBox); m_scoreTitle->setAlignment(Qt::AlignCenter); m_scoreTitle->setStyleSheet("color: #eee4da; font-size: 13px; font-weight: bold;");
    m_scoreLabel = new QLabel("0", m_scoreBox); m_scoreLabel->setAlignment(Qt::AlignCenter); m_scoreLabel->setStyleSheet("color: white; font-size: 26px; font-weight: bold;"); 
    vs->addWidget(m_scoreTitle); vs->addWidget(m_scoreLabel);
    
    m_bestBox = new QFrame(); m_bestBox->setFixedSize(110, 90); m_bestBox->setStyleSheet("background-color: #bbada0; border-radius: 6px;");
    auto *vb = new QVBoxLayout(m_bestBox); vb->setContentsMargins(0,12,0,12); vb->setSpacing(0);
    m_bestTitle = new QLabel("BEST", m_bestBox); m_bestTitle->setAlignment(Qt::AlignCenter); m_bestTitle->setStyleSheet("color: #eee4da; font-size: 13px; font-weight: bold;");
    m_bestScoreLabel = new QLabel("0", m_bestBox); m_bestScoreLabel->setAlignment(Qt::AlignCenter); m_bestScoreLabel->setStyleSheet("color: white; font-size: 26px; font-weight: bold;"); 
    vb->addWidget(m_bestTitle); vb->addWidget(m_bestScoreLabel);
    scoreRow->addWidget(m_scoreBox); scoreRow->addWidget(m_bestBox);
    headerRow->addWidget(titleContainer); headerRow->addStretch(); headerRow->addLayout(scoreRow);
    mainLayout->addLayout(headerRow);

    // --- Buttons Grid ---
    auto *buttonGrid = new QGridLayout(); buttonGrid->setSpacing(12);
    QString actionStyle = "QPushButton { background-color: #bbada0; color: white; font-weight: bold; border-radius: 6px; font-size: 16px; border: none; } QPushButton:hover { background-color: #9f8a76; }";
    
    m_restartBtn = new QPushButton("New Game (R)", this); m_undoBtn = new QPushButton("Undo (U)", this);
    m_restartBtn->setFocusPolicy(Qt::NoFocus); m_undoBtn->setFocusPolicy(Qt::NoFocus);
    m_restartBtn->setStyleSheet(actionStyle); m_undoBtn->setStyleSheet(actionStyle);
    m_restartBtn->setFixedHeight(44); m_undoBtn->setFixedHeight(44);
    
    m_normalBtn = new QPushButton("Normal (1)", this); m_unlimitedBtn = new QPushButton("Unlimited (2)", this); m_hardBtn = new QPushButton("Hard (3)", this);
    m_normalBtn->setFocusPolicy(Qt::NoFocus); m_unlimitedBtn->setFocusPolicy(Qt::NoFocus); m_hardBtn->setFocusPolicy(Qt::NoFocus);
    m_normalBtn->setFixedHeight(44); m_unlimitedBtn->setFixedHeight(44); m_hardBtn->setFixedHeight(44);
    m_normalBtn->setStyleSheet(getModeButtonStyle(true)); m_unlimitedBtn->setStyleSheet(getModeButtonStyle(false)); m_hardBtn->setStyleSheet(getModeButtonStyle(false));

    buttonGrid->addWidget(m_restartBtn, 0, 0, 1, 2);
    buttonGrid->addWidget(m_undoBtn, 0, 2, 1, 1);
    buttonGrid->addWidget(m_normalBtn, 1, 0, 1, 1);
    buttonGrid->addWidget(m_unlimitedBtn, 1, 1, 1, 1);
    buttonGrid->addWidget(m_hardBtn, 1, 2, 1, 1);
    mainLayout->addLayout(buttonGrid);

    // --- Status & Grid ---
    m_statusLabel = new QLabel("", this); m_statusLabel->setAlignment(Qt::AlignCenter); m_statusLabel->setStyleSheet("color: #8f7a66; font-size: 18px; font-weight: bold;");
    mainLayout->addWidget(m_statusLabel);

    m_gridWrapper = new QFrame(this); m_gridWrapper->setFixedSize(420, 420);
    m_gridContainer = new QFrame(m_gridWrapper); m_gridContainer->setFixedSize(420, 420); m_gridContainer->setStyleSheet("background-color: #bbada0; border-radius: 6px;");
    for (int i=0; i<16; ++i) {
        auto *bg = new QLabel(m_gridContainer); bg->setFixedSize(90, 90); bg->move(18+(i%4)*98, 18+(i/4)*98);
        bg->setStyleSheet("background-color: #cdc1b4; border-radius: 8px;"); bg->show();
    }
    m_tileLayer = new QFrame(m_gridContainer); m_tileLayer->setGeometry(0, 0, 420, 420); m_tileLayer->setStyleSheet("background: transparent;");
    
    m_overlay = new QFrame(m_gridContainer); m_overlay->setGeometry(0, 0, 420, 420);
    m_overlay->setStyleSheet("background-color: rgba(238, 228, 218, 0.9); border-radius: 6px;");
    auto *vo = new QVBoxLayout(m_overlay); vo->setContentsMargins(25, 25, 25, 25); vo->setSpacing(8);
    m_overlayLabel = new QLabel("You Win!", m_overlay); m_overlayLabel->setAlignment(Qt::AlignCenter); m_overlayLabel->setStyleSheet("color: #776e65; font-size: 44px; font-weight: bold; background: transparent;");
    m_overlayScore = new QLabel("Final Score: 0", m_overlay); m_overlayScore->setAlignment(Qt::AlignCenter); m_overlayScore->setStyleSheet("color: #8f7a66; font-size: 20px; font-weight: bold; background: transparent;");
    m_overlayBest = new QLabel("Best Score: 0", m_overlay); m_overlayBest->setAlignment(Qt::AlignCenter); m_overlayBest->setStyleSheet("color: #8f7a66; font-size: 18px; background: transparent;");
    auto *toRestart = new QPushButton("Try Again (R)", m_overlay); toRestart->setFocusPolicy(Qt::NoFocus); toRestart->setFixedSize(180, 50); toRestart->setStyleSheet(actionStyle);
    vo->addStretch(); vo->addWidget(m_overlayLabel); vo->addWidget(m_overlayScore); vo->addWidget(m_overlayBest); vo->addSpacing(20);
    vo->addWidget(toRestart, 0, Qt::AlignCenter); vo->addStretch();
    m_overlay->hide();
    
    mainLayout->addWidget(m_gridWrapper, 0, Qt::AlignCenter);
    mainLayout->addStretch();

    m_debugPanel = new QLabel("DEBUG: READY", this);
    m_debugPanel->setStyleSheet("background-color: #3c3a32; color: #edc22e; font-family: monospace; font-size: 11px; border-radius: 3px; padding: 5px;");
    mainLayout->addWidget(m_debugPanel); m_debugPanel->setVisible(m_isDebugMode);

    auto atomicReset = [this]() { 
        m_engine.reset(); m_isGameOver = false; m_overlay->hide(); m_lastScore = 0; m_lastBestScore = m_engine.getBestScore(); m_inputQueue.clear(); 
        m_statusLabel->setText(""); updateUI(); resetHardModeTimer(); 
    };
    connect(m_restartBtn, &QPushButton::clicked, this, atomicReset);
    connect(toRestart, &QPushButton::clicked, this, atomicReset);
    connect(m_undoBtn, &QPushButton::clicked, this, [this]() { if (!m_isAnimating && !m_isGameOver) { m_engine.undo(); m_lastScore = m_engine.getScore(); m_scoreTitle->setText("SCORE"); updateUI(); } });
    
    // Specification Section 2.2: 'The player may choose to restart, or switch to another game mode'
    auto modeSwitch = [this](GameMode m) {
        if (m_engine.getScore() == 0 || m_isGameOver) {
            if (m_isGameOver) { m_engine.reset(); m_isGameOver = false; m_overlay->hide(); m_lastScore = 0; m_inputQueue.clear(); m_statusLabel->setText(""); }
            m_mode = m; updateUI(); resetHardModeTimer();
        }
    };
    connect(m_normalBtn, &QPushButton::clicked, this, [modeSwitch]() { modeSwitch(GameMode::Normal); });
    connect(m_unlimitedBtn, &QPushButton::clicked, this, [modeSwitch]() { modeSwitch(GameMode::Unlimited); });
    connect(m_hardBtn, &QPushButton::clicked, this, [modeSwitch]() { modeSwitch(GameMode::Hard); });

    // Lock base geometries with a longer delay for WASM stability
    QTimer::singleShot(200, this, [this]() {
        m_scoreBaseRect = m_scoreBox->geometry();
        m_bestBaseRect = m_bestBox->geometry();
    });
}

void MainWindow::updateBoxFeedback(QFrame* box, QLabel* title, const QString& baseText, int delta) {
    if (!box || !title || delta <= 0) return;
    
    // Use stored base geometry to prevent cumulative 'flying away' effect
    QRect baseR = (box == m_scoreBox) ? m_scoreBaseRect : m_bestBaseRect;
    if (baseR.isNull()) baseR = box->geometry(); 

    title->setText(QString("%1 (+%2)").arg(baseText).arg(delta));
    title->setStyleSheet("color: #776e65; font-size: 13px; font-weight: bold;");
    
    QTimer::singleShot(800, this, [title, baseText]() {
        title->setText(baseText);
        title->setStyleSheet("color: #eee4da; font-size: 13px; font-weight: bold;");
    });

    auto *pa = new QPropertyAnimation(box, "geometry");
    pa->setDuration(200);
    pa->setStartValue(baseR);
    pa->setKeyValueAt(0.5, QRect(baseR.x()-3, baseR.y()-3, baseR.width()+6, baseR.height()+6));
    pa->setEndValue(baseR);
    pa->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::updateUI() {
    const auto& grid = m_engine.getGrid(); const auto& moves = m_engine.getLastMoves();
    int spawnId = m_engine.getSpawnId(); int myMoveId = ++m_currentMoveId;
    
    // Check for score increases and trigger visual feedback
    if (m_engine.getScore() > m_lastScore && !moves.empty()) {
        int delta = m_engine.getScore() - m_lastScore;
        updateBoxFeedback(m_scoreBox, m_scoreTitle, "SCORE", delta);
        
        // Show record break feedback only if best score improved
        if (m_engine.getBestScore() > m_lastBestScore) {
            updateBoxFeedback(m_bestBox, m_bestTitle, "BEST", m_engine.getBestScore() - m_lastBestScore);
        }
    }
    m_lastBestScore = m_engine.getBestScore();
    m_lastScore = m_engine.getScore();

    // Reset current animations if a new update is requested
    if (m_currentAnimationGroup) { m_currentAnimationGroup->stop(); m_currentAnimationGroup->deleteLater(); m_currentAnimationGroup = nullptr; }

    if (moves.empty()) {
        // Static update: Refresh all tile widgets without animation (e.g. on Undo or Start)
        m_isAnimating = false;
        for (auto const& [id, w] : m_tileWidgets) { w->hide(); w->deleteLater(); } m_tileWidgets.clear();
        for (int r=0; r<4; ++r) for (int c=0; c<4; ++c) if (grid[r][c].value != 0) {
            auto *w = new TileWidget(m_tileLayer); w->setValue(grid[r][c].value); w->move(18+c*98, 18+r*98); w->show();
            m_tileWidgets[grid[r][c].id] = w;
        }
        resetHardModeTimer();
    } else {
        // Animated update: Execute parallel move animations for all tiles
        m_isAnimating = true; m_currentAnimationGroup = new QParallelAnimationGroup(this);
        for (const auto& m : moves) if (m_tileWidgets.count(m.id)) {
            auto *a = new QPropertyAnimation(m_tileWidgets[m.id], "pos");
            a->setDuration(100); a->setEndValue(QPoint(18+m.toCol*98, 18+m.toRow*98)); a->setEasingCurve(QEasingCurve::OutCubic);
            m_currentAnimationGroup->addAnimation(a);
        }
        
        // Finalize state after all animations finish
        connect(m_currentAnimationGroup, &QParallelAnimationGroup::finished, this, [this, myMoveId, grid, spawnId]() {
            if (myMoveId != m_currentMoveId) return; // Prevent race conditions from stale animations
            
            std::map<int, TileWidget*> next;
            for (int r=0; r<4; ++r) for (int c=0; c<4; ++c) if (grid[r][c].value != 0) {
                int id = grid[r][c].id;
                if (m_tileWidgets.count(id)) { next[id] = m_tileWidgets[id]; m_tileWidgets.erase(id); }
                else if (id == spawnId) {
                    // Inject the newly spawned tile with a pop-in effect
                    auto *w = new TileWidget(m_tileLayer); w->setValue(grid[r][c].value); w->move(18+c*98, 18+r*98); w->animatePopIn();
                    next[id] = w;
                }
            }
            // Cleanup orphaned widgets (e.g. merged tiles)
            for (auto const& [id, w] : m_tileWidgets) { w->hide(); w->deleteLater(); } m_tileWidgets = next;
            
            // Final position sync and score update
            for (int r=0; r<4; ++r) for (int c=0; c<4; ++c) if (grid[r][c].value != 0) {
                int id = grid[r][c].id; m_tileWidgets[id]->setValue(grid[r][c].value); m_tileWidgets[id]->move(18+c*98, 18+r*98);
            }
            m_isAnimating = false; m_currentAnimationGroup = nullptr; resetHardModeTimer();
            m_undoBtn->setEnabled(m_engine.getHistoryDepth() > 0);
            
            // Process the next input in the queue if the user is playing fast
            if (!m_inputQueue.empty()) { 
                Direction n = m_inputQueue.front(); m_inputQueue.erase(m_inputQueue.begin()); processMove(n);
            }
        });
        m_currentAnimationGroup->start();
    }
    m_scoreLabel->setText(QString::number(m_engine.getScore())); m_bestScoreLabel->setText(QString::number(m_engine.getBestScore()));
    bool won = m_engine.hasWon() && m_mode == GameMode::Normal;
    bool stuck = !m_engine.canMove();
    m_isGameOver = won || stuck;
    if (m_isGameOver) {
        m_overlayLabel->setText(won ? "You Win!" : "GAME OVER!");
        m_overlayScore->setText(QString("Final Score: %1").arg(m_engine.getScore()));
        m_overlayBest->setText(QString("Best Score: %1").arg(m_engine.getBestScore()));
        m_overlay->show(); m_overlay->raise();
    } else { m_overlay->hide(); }
    
    m_normalBtn->setStyleSheet(getModeButtonStyle(m_mode == GameMode::Normal));
    m_unlimitedBtn->setStyleSheet(getModeButtonStyle(m_mode == GameMode::Unlimited));
    m_hardBtn->setStyleSheet(getModeButtonStyle(m_mode == GameMode::Hard));
    
    m_undoBtn->setEnabled(!m_isAnimating && !m_isGameOver && m_engine.getHistoryDepth() > 0);
    QSettings s("Group1", "2048"); s.setValue("bestScore", m_engine.getBestScore());
}

void MainWindow::processMove(Direction dir) {
    if (m_isGameOver || m_isAnimating) return;
    if (m_engine.move(dir)) updateUI();
    else {
        if (m_shakeAnim) { m_shakeAnim->stop(); delete m_shakeAnim; }
        m_shakeAnim = new QPropertyAnimation(m_gridContainer, "pos");
        m_shakeAnim->setDuration(200); QPoint b(0,0); m_shakeAnim->setStartValue(b);
        m_shakeAnim->setKeyValueAt(0.2, b+QPoint(-8,0)); m_shakeAnim->setKeyValueAt(0.4, b+QPoint(8,0));
        m_shakeAnim->setEndValue(b); m_shakeAnim->start();
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->isAutoRepeat()) return;
    int k = event->key();
    if (m_downKeys.count(k)) return;
    m_downKeys.insert(k);

    if (k == Qt::Key_U) { if (!m_isAnimating && !m_isGameOver) { m_engine.undo(); m_lastScore = m_engine.getScore(); m_scoreTitle->setText("SCORE"); updateUI(); } return; }
    if (k == Qt::Key_R) { m_restartBtn->click(); return; }
    if (k == Qt::Key_1) { m_normalBtn->click(); return; }
    if (k == Qt::Key_2) { m_unlimitedBtn->click(); return; }
    if (k == Qt::Key_3) { m_hardBtn->click(); return; }
    Direction d; bool ok = false;
    if (k == Qt::Key_W || k == Qt::Key_Up) { d = Direction::Up; ok = true; }
    else if (k == Qt::Key_S || k == Qt::Key_Down) { d = Direction::Down; ok = true; }
    else if (k == Qt::Key_A || k == Qt::Key_Left) { d = Direction::Left; ok = true; }
    else if (k == Qt::Key_D || k == Qt::Key_Right) { d = Direction::Right; ok = true; }
    if (ok) { if (m_isAnimating) { if (m_inputQueue.size()<1) m_inputQueue.push_back(d); } else processMove(d); }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    if (event->isAutoRepeat()) return;
    m_downKeys.erase(event->key());
}

bool MainWindow::event(QEvent *event) {
    return QWidget::event(event);
}

void MainWindow::resetHardModeTimer() {
    m_hardModeTimer->stop(); m_displayTimer->stop();
    m_hardBtn->setText("Hard (3)");
    if (m_mode == GameMode::Hard && !m_isGameOver) { m_hardModeTimer->start(5000); m_timeLeftMs = 5000; m_displayTimer->start(100); }
}

void MainWindow::makeRandomMove() {
    if (m_isGameOver || m_isAnimating) return;
    std::vector<Direction> ds = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};
    // Ensure the automatic slide is truly random as per Section 2.4
    static std::mt19937 g(static_cast<unsigned int>(std::time(nullptr)));
    std::shuffle(ds.begin(), ds.end(), g);
    for (auto d : ds) if (m_engine.move(d)) { updateUI(); break; }
}
// v1.2: Game modes
// v1.3: Persistence
