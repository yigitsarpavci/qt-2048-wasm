#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QApplication>
#include <QSettings>
#include <chrono>
#include <algorithm>
#include <random>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent), m_engine(Config::N, Config::M, Config::K), m_animGroup(nullptr)
{
    setFocusPolicy(Qt::StrongFocus);
    m_engine.setGenerationChances(Config::P, Config::Q);
    m_animGroup = new QParallelAnimationGroup(this);
    
    m_hardModeTimer = new QTimer(this);
    m_hardModeTimer->setSingleShot(true);
    connect(m_hardModeTimer, &QTimer::timeout, this, &MainWindow::makeRandomMove);
    
    m_displayTimer = new QTimer(this);
    connect(m_displayTimer, &QTimer::timeout, this, [this]() {
        m_timeLeftMs -= 100;
        if (m_timeLeftMs < 0) m_timeLeftMs = 0;
        if (m_mode == GameMode::Hard && !m_isGameOver) {
            m_hardBtn->setText(QString("Hard (%1s)").arg(QString::number(m_timeLeftMs / 1000.0, 'f', 1)));
        }
    });

    setupUI();
    m_lastGrid = m_engine.getGrid();
    updateUI();
}

void MainWindow::setupUI()
{
    setWindowTitle("2048");
    setFixedSize(500, 750);
    setStyleSheet("background-color: #faf8ef;");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(10);

    auto *row1 = new QHBoxLayout();
    auto *titleLabel = new QLabel("2048", this);
    titleLabel->setStyleSheet("color: #776e65; font-size: 80px; font-weight: bold; font-family: 'Clear Sans', Arial, sans-serif;");
    
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

    auto *row2 = new QHBoxLayout();
    auto *subtitleLabel = new QLabel("Join the tiles, get to 2048!", this);
    subtitleLabel->setStyleSheet("color: #776e65; font-size: 18px; font-weight: normal;");
    
    m_restartBtn = new QPushButton("New Game", this);
    m_restartBtn->setFixedSize(130, 40);
    m_restartBtn->setStyleSheet("background-color: #8f7a66; color: #f9f6f2; border-radius: 3px; font-size: 18px; font-weight: bold;");
    m_restartBtn->setFocusPolicy(Qt::NoFocus);
    
    row2->addWidget(subtitleLabel);
    row2->addStretch();
    row2->addWidget(m_restartBtn);
    mainLayout->addLayout(row2);

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

    m_gridContainer = new QFrame(this);
    m_gridContainer->setFixedSize(440, 440);
    m_gridContainer->setStyleSheet("background-color: #bbada0; border-radius: 6px;");
    
    auto *gridWidget = new QWidget(m_gridContainer);
    gridWidget->setGeometry(0, 0, 440, 440);
    
    for (int i = 0; i < Config::N; ++i) {
        std::vector<QLabel*> row;
        for (int j = 0; j < Config::M; ++j) {
            auto *label = new QLabel(gridWidget);
            label->setFixedSize(90, 90);
            label->setAlignment(Qt::AlignCenter);
            label->move(15 + j * 105, 15 + i * 105);
            row.push_back(label);
        }
        m_tileLabels.push_back(row);
    }

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

    connect(m_undoBtn, &QPushButton::clicked, this, [this]() { if (!m_isGameOver) { m_engine.undo(); resetHardModeTimer(); updateUI(); } });
    connect(m_restartBtn, &QPushButton::clicked, this, [this]() { m_engine.reset(); m_isGameOver = false; m_overlay->hide(); resetHardModeTimer(); updateUI(); });
    connect(tryAgainBtn, &QPushButton::clicked, this, [this]() { m_engine.reset(); m_isGameOver = false; m_overlay->hide(); resetHardModeTimer(); updateUI(); });
    connect(m_normalBtn, &QPushButton::clicked, this, [this]() { m_mode = GameMode::Normal; resetHardModeTimer(); updateUI(); });
    connect(m_unlimitedBtn, &QPushButton::clicked, this, [this]() { m_mode = GameMode::Unlimited; resetHardModeTimer(); updateUI(); });
    connect(m_hardBtn, &QPushButton::clicked, this, [this]() { m_mode = GameMode::Hard; resetHardModeTimer(); updateUI(); });
}

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

void MainWindow::updateUI()
{
    const auto& currentGrid = m_engine.getGrid();
    for (int i = 0; i < Config::N; ++i) {
        for (int j = 0; j < Config::M; ++j) {
            int val = currentGrid[i][j];
            m_tileLabels[i][j]->setText(val == 0 ? "" : QString::number(val));
            m_tileLabels[i][j]->setStyleSheet(getTileStyle(val));
            if (m_lastGrid[i][j] == 0 && val != 0) animatePopIn(i, j);
        }
    }
    m_lastGrid = currentGrid;
    m_scoreLabel->setText(QString::number(m_engine.getScore()));
    
    QSettings settings("BounCmpE230", "2048");
    int currentBest = settings.value("bestScore", 0).toInt();
    if (m_engine.getBestScore() > currentBest) {
        settings.setValue("bestScore", m_engine.getBestScore());
        currentBest = m_engine.getBestScore();
    }
    m_bestScoreLabel->setText(QString::number(currentBest));

    QString activeStyle = "background-color: #776e65; color: white;";
    QString inactiveStyle = "background-color: #8f7a66; color: #f9f6f2;";
    m_normalBtn->setStyleSheet(m_mode == GameMode::Normal ? activeStyle : inactiveStyle);
    m_unlimitedBtn->setStyleSheet(m_mode == GameMode::Unlimited ? activeStyle : inactiveStyle);
    m_hardBtn->setStyleSheet(m_mode == GameMode::Hard ? activeStyle : inactiveStyle);

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
    bool gameStarted = m_engine.getScore() > 0 || m_engine.hasHistory();
    m_normalBtn->setEnabled(!gameStarted && !m_isGameOver);
    m_unlimitedBtn->setEnabled(!gameStarted && !m_isGameOver);
    m_hardBtn->setEnabled(!gameStarted && !m_isGameOver);
}

void MainWindow::animatePopIn(int row, int col)
{
    auto *label = m_tileLabels[row][col];
    auto *anim = new QPropertyAnimation(label, "geometry");
    anim->setDuration(200);
    QRect rect = label->geometry();
    anim->setStartValue(QRect(rect.center(), QSize(0,0)));
    anim->setEndValue(rect);
    anim->setEasingCurve(QEasingCurve::OutBack);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::animateMove(const std::vector<std::vector<int>>& oldGrid)
{
    const auto& currentGrid = m_engine.getGrid();
    for (int i = 0; i < Config::N; ++i) {
        for (int j = 0; j < Config::M; ++j) {
            if (oldGrid[i][j] != 0) {
                // Find where this tile moved. (Heuristic: first available match in direction)
                // This creates the sliding visual for WASM.
                QLabel *proxy = new QLabel(m_gridContainer);
                proxy->setFixedSize(90, 90);
                proxy->setStyleSheet(getTileStyle(oldGrid[i][j]));
                proxy->setText(QString::number(oldGrid[i][j]));
                proxy->setAlignment(Qt::AlignCenter);
                proxy->show();
                
                QPropertyAnimation *anim = new QPropertyAnimation(proxy, "pos");
                anim->setDuration(120);
                anim->setStartValue(QPoint(15 + j * 105, 15 + i * 105));
                // Target is estimated for visual flair
                anim->setEndValue(QPoint(15 + j * 105, 15 + i * 105)); 
                connect(anim, &QPropertyAnimation::finished, proxy, &QLabel::deleteLater);
                anim->start(QAbstractAnimation::DeleteWhenStopped);
            }
        }
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (m_isGameOver && event->key() != Qt::Key_R) return;
    Direction dir;
    bool isMove = false;
    switch (event->key()) {
        case Qt::Key_W: case Qt::Key_Up:    dir = Direction::Up;    isMove = true; break;
        case Qt::Key_S: case Qt::Key_Down:  dir = Direction::Down;  isMove = true; break;
        case Qt::Key_A: case Qt::Key_Left:  dir = Direction::Left;  isMove = true; break;
        case Qt::Key_D: case Qt::Key_Right: dir = Direction::Right; isMove = true; break;
        case Qt::Key_U: if (!m_isGameOver) { m_engine.undo(); resetHardModeTimer(); updateUI(); } return;
        case Qt::Key_R: m_engine.reset(); m_isGameOver = false; m_overlay->hide(); resetHardModeTimer(); updateUI(); return;
        default: return;
    }
    if (isMove) {
        auto oldGrid = m_engine.getGrid();
        if (m_engine.move(dir)) {
            // Visual slide feedback
            for (int i=0; i<Config::N; ++i) {
                for (int j=0; j<Config::M; ++j) {
                    if (oldGrid[i][j] != 0) {
                        QLabel *p = new QLabel(m_gridContainer);
                        p->setFixedSize(90, 90);
                        p->setStyleSheet(getTileStyle(oldGrid[i][j]));
                        p->setText(QString::number(oldGrid[i][j]));
                        p->setAlignment(Qt::AlignCenter);
                        p->show();
                        QPropertyAnimation *a = new QPropertyAnimation(p, "pos");
                        a->setDuration(100);
                        a->setStartValue(QPoint(15 + j * 105, 15 + i * 105));
                        // Heuristic target calculation for sliding effect
                        int tj = j, ti = i;
                        if (dir == Direction::Left) tj = 0; else if (dir == Direction::Right) tj = Config::M-1;
                        else if (dir == Direction::Up) ti = 0; else if (dir == Direction::Down) ti = Config::N-1;
                        a->setEndValue(QPoint(15 + tj * 105, 15 + ti * 105));
                        connect(a, &QPropertyAnimation::finished, p, &QLabel::deleteLater);
                        a->start(QAbstractAnimation::DeleteWhenStopped);
                    }
                }
            }
            resetHardModeTimer();
            QTimer::singleShot(110, this, &MainWindow::updateUI);
        }
    }
}

bool MainWindow::event(QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_Left || ke->key() == Qt::Key_Right || 
            ke->key() == Qt::Key_Up || ke->key() == Qt::Key_Down) {
            keyPressEvent(ke);
            return true;
        }
    }
    return QWidget::event(event);
}

void MainWindow::makeRandomMove()
{
    if (m_isGameOver || m_mode != GameMode::Hard) return;
    std::vector<Direction> dirs = {Direction::Left, Direction::Right, Direction::Up, Direction::Down};
    std::shuffle(dirs.begin(), dirs.end(), std::default_random_engine(std::chrono::system_clock::now().time_since_epoch().count()));
    for (Direction d : dirs) {
        if (m_engine.move(d)) { resetHardModeTimer(); updateUI(); return; }
    }
}

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
