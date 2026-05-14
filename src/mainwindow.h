/**
 * @file mainwindow.h
 * @brief Main application window and user interface management for the 2048 game.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QTimer>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <set>
#include <map>
#include "gameengine.h"

/**
 * @enum GameMode
 * @brief Defines the available gameplay rulesets.
 */
enum class GameMode {
    Normal,     ///< Standard rules: Game ends at 2048
    Unlimited,  ///< Continuous play after reaching 2048
    Hard        ///< Time-pressured mode with automatic random moves
};

/**
 * @class TileWidget
 * @brief Represents the visual representation of a single game tile.
 */
class TileWidget : public QWidget {
    Q_OBJECT
public:
    explicit TileWidget(QWidget *parent = nullptr);

    /**
     * @brief Updates the tile's value and visual style.
     * @param val New tile value.
     */
    void setValue(int val);

    /**
     * @brief Plays a pop-in entrance animation.
     */
    void animatePopIn();

private:
    QString getTileStyle(int value);
    QLabel *m_label;
    QWidget *m_face;
    int m_value;
};

/**
 * @class MainWindow
 * @brief The main interface class coordinating the game engine and UI components.
 */
class MainWindow : public QWidget {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    bool event(QEvent *event) override;

private:
    void setupUI();
    void updateUI();
    void processMove(Direction dir);
    void resetHardModeTimer();
    void makeRandomMove();
    void updateBoxFeedback(QFrame* box, QLabel* title, const QString& baseText, int delta);

    GameEngine m_engine;
    GameMode m_mode = GameMode::Normal;
    std::map<int, TileWidget*> m_tileWidgets;
    
    // UI Elements
    QLabel *m_scoreLabel, *m_bestScoreLabel;
    QLabel *m_statusLabel, *m_debugPanel;
    QPushButton *m_restartBtn, *m_undoBtn;
    QPushButton *m_normalBtn, *m_unlimitedBtn, *m_hardBtn;
    
    QFrame *m_gridContainer, *m_gridWrapper, *m_tileLayer;
    QFrame *m_overlay;
    QLabel *m_overlayLabel, *m_overlayScore, *m_overlayBest;

    // State & Animation
    bool m_isGameOver = false;
    bool m_isAnimating = false;
    bool m_isDebugMode = false;
    std::set<int> m_downKeys;
    std::vector<Direction> m_inputQueue;
    
    QParallelAnimationGroup *m_currentAnimationGroup = nullptr;
    QPropertyAnimation *m_shakeAnim = nullptr;
    
    int m_lastScore = 0;
    int m_lastBestScore = 0;
    int m_currentMoveId = 0;
    int m_timeLeftMs = 5000;
    bool m_preventTimerReset = false;

    QFrame *m_scoreBox, *m_bestBox;
    QLabel *m_scoreTitle, *m_bestTitle;
    QRect m_scoreBaseRect, m_bestBaseRect;

    QTimer *m_hardModeTimer;
    QTimer *m_displayTimer;
};

#endif // MAINWINDOW_H
