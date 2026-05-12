/**
 * @file gameengine.cpp
 * @brief Implementation of the game engine logic.
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
 * @brief Resets the engine state. IDs and History are cleared.
 */
void GameEngine::reset() {
    m_grid = std::vector<std::vector<Tile>>(m_rows, std::vector<Tile>(m_cols, {0, -1}));
    m_score = 0;
    m_history.clear();
    m_lastMoves.clear();
    m_nextId = 1;
    m_txCounter = 0;
    m_spawnId = -1;

    // Initial tiles must be exactly two '2's according to specification
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
 * @brief High-level movement logic. Coordinates rotations and merging.
 */
bool GameEngine::move(Direction dir) {
    auto oldGrid = m_grid;
    saveSnapshot(); // Save current state for potential undo
    m_lastMoves.clear(); // Clear previous move metadata
    m_spawnId = -1;

    int scoreDelta = 0;
    int merges = 0;

    // Helper to rotate/transform grid coordinates based on the requested direction.
    // This allows the merge logic to always process rows horizontally.
    auto rotate = [&](int& r, int& c, bool back = false) {
        Q_UNUSED(back);
        if (dir == Direction::Up) return;
        if (dir == Direction::Down) { r = m_rows - 1 - r; }
        else if (dir == Direction::Left) { std::swap(r, c); }
        else if (dir == Direction::Right) { std::swap(r, c); c = m_cols - 1 - c; }
    };

    std::vector<std::vector<Tile>> newGrid(m_rows, std::vector<Tile>(m_cols));
    
    // Process each column independently after transformation
    for (int c = 0; c < m_cols; ++c) {
        std::vector<Tile> line;
        // Step 1: Collect non-zero tiles in the current line
        for (int r = 0; r < m_rows; ++r) {
            int tr = r, tc = c; rotate(tr, tc);
            if (m_grid[tr][tc].value != 0) line.push_back(m_grid[tr][tc]);
        }

        std::vector<Tile> merged;
        // Step 2: Iterate through tiles and merge adjacent identical values
        for (size_t i = 0; i < line.size(); ++i) {
            if (i + 1 < line.size() && line[i].value == line[i+1].value) {
                // Merge detected: calculate new value and update score
                int newVal = line[i].value * 2;
                scoreDelta += newVal;
                merges++;
                merged.push_back({newVal, line[i].id});
                
                // Track move metadata for UI animation synchronization
                int tr1 = (int)merged.size() - 1, tc1 = c; rotate(tr1, tc1);
                int fr1 = (int)i, fc1 = c; rotate(fr1, fc1);
                int fr2 = (int)i + 1, fc2 = c; rotate(fr2, fc2);
                
                m_lastMoves.push_back({line[i].id, fr1, fc1, tr1, tc1, newVal, true});
                m_lastMoves.push_back({line[i+1].id, fr2, fc2, tr1, tc1, newVal, true});
                i++; // Skip the next tile as it has been merged
            } else {
                // No merge: shift tile to its new position
                merged.push_back(line[i]);
                int tr = (int)merged.size() - 1, tc = c; rotate(tr, tc);
                int fr = (int)i, fc = c; rotate(fr, fc);
                m_lastMoves.push_back({line[i].id, fr, fc, tr, tc, line[i].value, false});
            }
        }
        // Step 3: Pad the rest of the line with empty tiles
        while (merged.size() < (size_t)m_rows) merged.push_back({0, -1});
        for (int r = 0; r < m_rows; ++r) {
            int tr = r, tc = c; rotate(tr, tc);
            newGrid[tr][tc] = merged[r];
        }
    }

    // Determine if any movement or merging actually occurred
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
        spawnTiles(1); // Add a new random tile after a successful move
        logTransaction(scoreDelta, merges);
        return true;
    } else {
        // If no change, discard the snapshot saved at the beginning
        if (!m_history.empty()) m_history.pop_back(); 
        return false;
    }
}

/**
 * @brief Spawns new tiles into random empty cells.
 * @param count Number of tiles to inject.
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
 * @brief Restores state including ID counter for full determinism.
 */
void GameEngine::undo() {
    if (m_history.empty()) return;
    const auto& snap = m_history.back();
    m_grid = snap.grid;
    m_score = snap.score;
    m_bestScore = snap.bestScore;
    m_nextId = snap.nextId; // Full ID Determinism
    m_history.pop_back();
    m_lastMoves.clear();
    m_spawnId = -1;
}

/**
 * @brief Captures current state into history stack.
 */
void GameEngine::saveSnapshot() {
    // Specification: 'There is no limit on the number of undo steps'
    // Storing up to 100,000 steps to satisfy this in practical terms.
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
 * @brief Logs game transactions for debugging purposes.
 */
void GameEngine::logTransaction(int delta, int merges) {
    m_txCounter++;
    QString ts = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    qDebug().noquote() << QString("[%1] TX#%2 | Score: %3 (+%4) | Merges: %5 | NextID: %6")
        .arg(ts).arg(m_txCounter).arg(m_score).arg(delta).arg(merges).arg(m_nextId);
}
// v1.1: Undo persistence
// v1.1: Undo support
