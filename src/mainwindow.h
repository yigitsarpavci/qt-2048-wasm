#pragma once

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QKeyEvent>
#include <QTimer>
#include <vector>
#include "gameengine.h"
#include "config.h"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    bool event(QEvent *event) override;

private:
    void updateUI();
    void setupUI();
    void makeRandomMove();
    void resetHardModeTimer();
    QString getTileStyle(int value);

    GameEngine m_engine;
    QTimer *m_hardModeTimer;
    QTimer *m_displayTimer;
    int m_timeLeftMs;
    QGridLayout *m_gridLayout;
    std::vector<std::vector<QLabel*>> m_tileLabels;
    QLabel *m_scoreLabel;
    QLabel *m_bestScoreLabel;
    QLabel *m_timerLabel;
    QFrame *m_gridContainer;
    QFrame *m_overlay;
    
    enum class GameMode { Normal, Unlimited, Hard };
    GameMode m_mode = GameMode::Normal;
    bool m_isGameOver = false;

    QPushButton *m_normalBtn;
    QPushButton *m_unlimitedBtn;
    QPushButton *m_hardBtn;
    QPushButton *m_undoBtn;
    QPushButton *m_restartBtn;
};
