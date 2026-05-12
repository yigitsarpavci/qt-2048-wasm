#include "gameengine.h"
#include <algorithm>
#include <chrono>

/**
 * @brief Constructor for the GameEngine.
 * Seeds the RNG with high-resolution clock and initializes the board.
 */
GameEngine::GameEngine(int rows, int cols, int target)
    : m_rows(rows), m_cols(cols), m_target(target), m_p(90), m_q(10), m_score(0), m_bestScore(0)
{
    m_rng.seed(std::chrono::system_clock::now().time_since_epoch().count());
    reset();
}

/**
 * @brief Resets the game to zero state.
 * Strictly adheres to the requirement of starting with two random tiles.
 */
void GameEngine::reset()
{
    m_grid.assign(m_rows, std::vector<int>(m_cols, 0));
    m_score = 0;
    while (!m_history.empty()) m_history.pop();
    addRandomTile();
    addRandomTile();
}

/**
 * @brief Saves current state for Section 4 Undo compliance.
 */
void GameEngine::saveState()
{
    m_history.push({m_grid, m_score, m_bestScore});
}

/**
 * @brief Reverts to the previous valid game state.
 */
void GameEngine::undo()
{
    if (m_history.empty()) return;
    GameState last = m_history.top();
    m_history.pop();
    m_grid = last.grid;
    m_score = last.score;
    m_bestScore = last.bestScore;
}

/**
 * @brief Handles movement in any of the 4 directions.
 * Normalizes logic by rotating/reversing vectors to perform a single-direction merge.
 * @return true if the grid was altered (mandatory for spawning new tiles).
 */
bool GameEngine::move(Direction dir)
{
    std::vector<std::vector<int>> oldGrid = m_grid;
    int oldScore = m_score;
    bool changed = false;

    if (dir == Direction::Left || dir == Direction::Right) {
        for (int i = 0; i < m_rows; ++i) {
            std::vector<int> line = m_grid[i];
            if (dir == Direction::Right) std::reverse(line.begin(), line.end());
            if (slideAndMerge(line)) changed = true;
            if (dir == Direction::Right) std::reverse(line.begin(), line.end());
            m_grid[i] = line;
        }
    } else {
        for (int j = 0; j < m_cols; ++j) {
            std::vector<int> line;
            for (int i = 0; i < m_rows; ++i) line.push_back(m_grid[i][j]);
            if (dir == Direction::Down) std::reverse(line.begin(), line.end());
            if (slideAndMerge(line)) changed = true;
            if (dir == Direction::Down) std::reverse(line.begin(), line.end());
            for (int i = 0; i < m_rows; ++i) m_grid[i][j] = line[i];
        }
    }

    if (changed) {
        m_history.push({oldGrid, oldScore, m_bestScore});
        addRandomTile();
        if (m_score > m_bestScore) m_bestScore = m_score;
        return true;
    }
    return false;
}

/**
 * @brief Core merge algorithm (Section 4 implementation).
 * Implements sliding followed by identical tile merging.
 */
bool GameEngine::slideAndMerge(std::vector<int>& line)
{
    bool changed = false;
    std::vector<int> newLine;
    for (int val : line) if (val != 0) newLine.push_back(val);
    
    for (size_t i = 0; i + 1 < newLine.size(); ++i) {
        if (newLine[i] == newLine[i+1]) {
            newLine[i] *= 2;
            m_score += newLine[i];
            newLine.erase(newLine.begin() + i + 1);
            changed = true;
        }
    }

    while (newLine.size() < (size_t)line.size()) newLine.push_back(0);
    if (newLine != line) changed = true;
    line = newLine;
    return changed;
}

/**
 * @brief Spawns a new tile based on P (getting 2) and Q (getting 4) probabilities.
 */
void GameEngine::addRandomTile()
{
    std::vector<std::pair<int, int>> emptyCells;
    for (int i = 0; i < m_rows; ++i) {
        for (int j = 0; j < m_cols; ++j) {
            if (m_grid[i][j] == 0) emptyCells.push_back({i, j});
        }
    }
    if (emptyCells.empty()) return;

    std::uniform_int_distribution<int> dist(0, emptyCells.size() - 1);
    auto [r, c] = emptyCells[dist(m_rng)];

    std::uniform_int_distribution<int> valDist(1, 100);
    int v = valDist(m_rng);
    m_grid[r][c] = (v <= m_p) ? 2 : 4;
}

bool GameEngine::canMove() const
{
    for (int i = 0; i < m_rows; ++i) {
        for (int j = 0; j < m_cols; ++j) {
            if (m_grid[i][j] == 0) return true;
            if (i + 1 < m_rows && m_grid[i][j] == m_grid[i+1][j]) return true;
            if (j + 1 < m_cols && m_grid[i][j] == m_grid[i][j+1]) return true;
        }
    }
    return false;
}

bool GameEngine::hasWon() const
{
    for (int i = 0; i < m_rows; ++i) {
        for (int j = 0; j < m_cols; ++j) {
            if (m_grid[i][j] >= m_target) return true;
        }
    }
    return false;
}

void GameEngine::setDimensions(int rows, int cols) { m_rows = rows; m_cols = cols; reset(); }
void GameEngine::setTargetValue(int target) { m_target = target; }
void GameEngine::setGenerationChances(int p, int q) { m_p = p; m_q = q; }
