/**
 * @file mainwindow.h
 * @brief UI Controller for 2048 with Doxygen compliance & Focus Management.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QTimer>
#include <map>
#include <vector>
#include <set>
#include "gameengine.h"

/**
 * @enum GameMode
 * @brief Difficulty modes defined by the project specification.
 */
enum class GameMode { Normal, Unlimited, Hard };

/**
 * @class TileWidget
 * @brief Visual representation of a single tile with animations.
 */
class TileWidget : public QWidget {
    Q_OBJECT
public:
    explicit TileWidget(QWidget *parent = nullptr);
    
    /**
     * @brief Updates the tile's numeric value and visual style.
     * @param val New value.
     */
    void setValue(int val);
    
    /** @return Current numeric value. */
    int getValue() const { return m_value; }
    
    /** @brief Triggers the "pop-in" scale animation for new tiles. */
    void animatePopIn();

private:
    QString getTileStyle(int value);
    int m_value;
    QLabel *m_label;
    QWidget *m_face;
};

/**
 * @class MainWindow
 * @brief Main UI controller handling input, animations, and mode management.
 */
class MainWindow : public QWidget {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    /** @brief Handles keyboard input with auto-repeat protection. */
    void keyPressEvent(QKeyEvent *event) override;
    
    /** @brief Handles key release to clear physical state. */
    void keyReleaseEvent(QKeyEvent *event) override;
    
    /** @brief Standard event handling. */
    bool event(QEvent *event) override;

private:
    // UI Setup & Maintenance
    void setupUI();
    void updateBoxFeedback(QFrame* box, QLabel* title, const QString& baseText, int delta);
    void updateUI();
    
    // Movement Processing
    void processMove(Direction dir);
    void resetHardModeTimer();
    void makeRandomMove();

    // Logic Engine
    GameEngine m_engine;
    GameMode m_mode = GameMode::Normal;

    // UI State
    bool m_isAnimating = false;
    bool m_isGameOver = false;
    bool m_isDebugMode = false;
    int m_lastScore = 0;
    QRect m_scoreBaseRect;
    QRect m_bestBaseRect;
    int m_lastBestScore = 0;
    int m_currentMoveId = 0;
    int m_timeLeftMs = 5000;

    QFrame *m_scoreBox, *m_bestBox;
    QLabel *m_scoreTitle, *m_bestTitle;
    QLabel *m_scoreLabel, *m_bestScoreLabel;
    QLabel *m_statusLabel, *m_debugPanel;
    QFrame *m_gridWrapper, *m_gridContainer, *m_tileLayer;
    
    // Buttons
    QPushButton *m_restartBtn, *m_undoBtn;
    QPushButton *m_normalBtn, *m_unlimitedBtn, *m_hardBtn;

    // Overlay Components
    QFrame *m_overlay;
    QLabel *m_overlayLabel, *m_overlayScore, *m_overlayBest;

    // Animation & Input Management
    std::map<int, TileWidget*> m_tileWidgets;
    std::vector<Direction> m_inputQueue;
    std::set<int> m_downKeys;
    QParallelAnimationGroup *m_currentAnimationGroup = nullptr;
    QPropertyAnimation *m_shakeAnim = nullptr;
    QTimer *m_hardModeTimer;
    QTimer *m_displayTimer;
};

#endif // MAINWINDOW_H
// v1.3: UI Polish
// v1.4: UX
