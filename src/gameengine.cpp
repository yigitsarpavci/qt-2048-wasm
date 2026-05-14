/**
 * @file gameengine.cpp
 * @brief Implementation of the 2048 game engine logic.
 */

#include "gameengine.h"
#include <algorithm>
#include <ctime>
#include <QDebug>
#include <QDateTime>

GameEngine::GameEngine(int n, int m, int k) 
    : m_rows(n), m_cols(m), m_k(k), m_rng(static_cast<unsigned int>(std::time(nullptr))) 
{
    reset();
}

/**
 * @brief Resets the game state, clearing the grid, score, and history.
 */
void GameEngine::reset() {
    m_grid = std::vector<std::vector<Tile>>(m_rows, std::vector<Tile>(m_cols, {0, -1}));
    m_score = 0;
    m_history.clear();
    m_lastMoves.clear();
    m_nextId = 1;
    m_txCounter = 0;
    m_spawnId = -1;

    // Start with two initial tiles as per standard rules
    std::vector<std::pair<int, int>> empty;
    for (int r = 0; r < m_rows; ++r)
        for (int c = 0; c < m_cols; ++c)
            empty.push_back({r, c});
    
    std::shuffle(empty.begin(), empty.end(), m_rng);
    for (int i = 0; i < 2; ++i) {
        int id = m_nextId++;
        m_grid[empty[i].first][empty[i].second] = {2, id};
    }
}

void GameEngine::setGenerationChances(double p) {
    m_p = std::clamp(p, 0.0, 1.0);
}

/**
 * @brief Main move logic. Handles coordinate rotation, merging, and state updates.
 */
bool GameEngine::move(Direction dir) {
    auto oldGrid = m_grid;
    saveSnapshot(); // Preserve state for undo
    m_lastMoves.clear();
    m_spawnId = -1;

    int scoreDelta = 0;
    int merges = 0;

    // Rotates coordinates to allow a single left-to-right merge logic to handle all four directions.
    auto rotate = [&](int& r, int& c, bool back = false) {
        Q_UNUSED(back);
        if (dir == Direction::Up) return;
        if (dir == Direction::Down) { r = m_rows - 1 - r; }
        else if (dir == Direction::Left) { std::swap(r, c); }
        else if (dir == Direction::Right) { std::swap(r, c); c = m_cols - 1 - c; }
    };

    std::vector<std::vector<Tile>> newGrid(m_rows, std::vector<Tile>(m_cols));
    
    // Process each line independently based on the current direction's rotation.
    for (int c = 0; c < m_cols; ++c) {
        std::vector<Tile> line;
        for (int r = 0; r < m_rows; ++r) {
            int tr = r, tc = c; rotate(tr, tc);
            if (m_grid[tr][tc].value != 0) line.push_back(m_grid[tr][tc]);
        }

        std::vector<Tile> merged;
        for (size_t i = 0; i < line.size(); ++i) {
            if (i + 1 < line.size() && line[i].value == line[i+1].value) {
                // Combine identical adjacent tiles
                int newVal = line[i].value * 2;
                scoreDelta += newVal;
                merges++;
                merged.push_back({newVal, line[i].id});
                
                // Track metadata for smooth UI animations
                int tr1 = (int)merged.size() - 1, tc1 = c; rotate(tr1, tc1);
                int fr1 = (int)i, fc1 = c; rotate(fr1, fc1);
                int fr2 = (int)i + 1, fc2 = c; rotate(fr2, fc2);
                
                m_lastMoves.push_back({line[i].id, fr1, fc1, tr1, tc1, newVal, true});
                m_lastMoves.push_back({line[i+1].id, fr2, fc2, tr1, tc1, newVal, true});
                i++;
            } else {
                // Shift tile without merging
                merged.push_back(line[i]);
                int tr = (int)merged.size() - 1, tc = c; rotate(tr, tc);
                int fr = (int)i, fc = c; rotate(fr, fc);
                m_lastMoves.push_back({line[i].id, fr, fc, tr, tc, line[i].value, false});
            }
        }
        while (merged.size() < (size_t)m_rows) merged.push_back({0, -1});
        for (int r = 0; r < m_rows; ++r) {
            int tr = r, tc = c; rotate(tr, tc);
            newGrid[tr][tc] = merged[r];
        }
    }

    bool changed = false;
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            if (newGrid[r][c].value != oldGrid[r][c].value) { changed = true; break; }
        }
    }

    if (changed) {
        m_grid = newGrid;
        m_score += scoreDelta;
        if (m_score > m_bestScore) m_bestScore = m_score;
        spawnTiles(1);
        logTransaction(scoreDelta, merges);
        return true;
    } else {
        if (!m_history.empty()) m_history.pop_back(); 
        return false;
    }
}

/**
 * @brief Spawns new tiles in random available empty cells.
 */
void GameEngine::spawnTiles(int count) {
    if (!m_spawnEnabled) return;
    std::vector<std::pair<int, int>> empty;
    for (int r = 0; r < m_rows; ++r)
        for (int c = 0; c < m_cols; ++c)
            if (m_grid[r][c].value == 0) empty.push_back({r, c});

    if (empty.empty()) return;
    std::shuffle(empty.begin(), empty.end(), m_rng);
    
    for (int i = 0; i < std::min(count, (int)empty.size()); ++i) {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        int val = (dist(m_rng) < m_p) ? 2 : 4;
        int id = m_nextId++;
        m_grid[empty[i].first][empty[i].second] = {val, id};
        m_spawnId = id;
    }
}

/**
 * @brief Restores the game state from the history stack.
 */
void GameEngine::undo() {
    if (m_history.empty()) return;
    const auto& snap = m_history.back();
    m_grid = snap.grid;
    m_score = snap.score;
    m_bestScore = snap.bestScore;
    m_nextId = snap.nextId;
    m_history.pop_back();
    m_lastMoves.clear();
    m_spawnId = -1;
}

/**
 * @brief Saves the current game state into the history stack for undo support.
 */
void GameEngine::saveSnapshot() {
    // Maintain a large history stack for continuous undo operations
    if (m_history.size() > 100000) m_history.pop_front();
    m_history.push_back({m_grid, m_score, m_bestScore, m_nextId});
}

bool GameEngine::hasWon() const {
    for (int r = 0; r < m_rows; ++r)
        for (int c = 0; c < m_cols; ++c)
            if (m_grid[r][c].value >= m_k) return true;
    return false;
}

bool GameEngine::canMove() const {
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            if (m_grid[r][c].value == 0) return true;
            if (c + 1 < m_cols && m_grid[r][c].value == m_grid[r][c+1].value) return true;
            if (r + 1 < m_rows && m_grid[r][c].value == m_grid[r+1][c].value) return true;
        }
    }
    return false;
}

/**
 * @brief Logs move transaction details for monitoring and debugging.
 */
void GameEngine::logTransaction(int delta, int merges) {
    m_txCounter++;
    QString ts = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    qDebug().noquote() << QString("[%1] TX#%2 | Score: %3 (+%4) | Merges: %5 | NextID: %6")
        .arg(ts).arg(m_txCounter).arg(m_score).arg(delta).arg(merges).arg(m_nextId);
}
