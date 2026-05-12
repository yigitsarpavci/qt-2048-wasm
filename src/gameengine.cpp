#include "gameengine.h"
#include <algorithm>
#include <chrono>

/**
 * @brief Constructor for the GameEngine.
 * Initializes the random number generator and sets initial dimensions.
 */
GameEngine::GameEngine(int rows, int cols, int target)
    : m_rows(rows), m_cols(cols), m_target(target), m_p(90), m_q(10), m_score(0), m_bestScore(0)
{
    m_rng.seed(std::chrono::system_clock::now().time_since_epoch().count());
    reset();
}

/**
 * @brief Resets the game to its initial state.
 * Clears the grid, score, and history, then adds two starting tiles.
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
 * @brief Saves the current grid and score to the history stack for undo support.
 */
void GameEngine::saveState()
{
    m_history.push({m_grid, m_score, m_bestScore});
}

/**
 * @brief Reverts the game to the last saved state.
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
 * @brief Orchestrates tile movement in a given direction.
 * Normalizes all directions into a 'Left' move logic to avoid redundancy.
 * @return true if the grid changed as a result of the move.
 */
bool GameEngine::move(Direction dir)
{
    std::vector<std::vector<int>> oldGrid = m_grid;
    int oldScore = m_score;
    bool changed = false;

    // Horizontal moves
    if (dir == Direction::Left || dir == Direction::Right) {
        for (int i = 0; i < m_rows; ++i) {
            std::vector<int> line = m_grid[i];
            // Normalize Right to Left by reversing
            if (dir == Direction::Right) std::reverse(line.begin(), line.end());
            if (slideAndMerge(line)) changed = true;
            // Restore orientation
            if (dir == Direction::Right) std::reverse(line.begin(), line.end());
            m_grid[i] = line;
        }
    } 
    // Vertical moves
    else {
        for (int j = 0; j < m_cols; ++j) {
            std::vector<int> line;
            for (int i = 0; i < m_rows; ++i) line.push_back(m_grid[i][j]);
            // Normalize Down to Up by reversing
            if (dir == Direction::Down) std::reverse(line.begin(), line.end());
            if (slideAndMerge(line)) changed = true;
            // Restore orientation
            if (dir == Direction::Down) std::reverse(line.begin(), line.end());
            for (int i = 0; i < m_rows; ++i) m_grid[i][j] = line[i];
        }
    }

    // Post-move processing: save state and spawn new tile if grid changed
    if (changed) {
        m_history.push({oldGrid, oldScore, m_bestScore});
        addRandomTile();
        if (m_score > m_bestScore) m_bestScore = m_score;
        return true;
    }
    return false;
}

/**
 * @brief The core merging algorithm.
 * 1. Slides non-zero tiles to the front.
 * 2. Merges adjacent identical tiles (once per move per tile).
 * 3. Pads the rest of the line with zeros.
 * @return true if the line was modified.
 */
bool GameEngine::slideAndMerge(std::vector<int>& line)
{
    bool changed = false;
    
    // Step 1: Slide (filter non-zero elements)
    std::vector<int> newLine;
    for (int val : line) if (val != 0) newLine.push_back(val);
    
    // Step 2: Merge adjacent matching tiles
    for (size_t i = 0; i + 1 < newLine.size(); ++i) {
        if (newLine[i] == newLine[i+1]) {
            newLine[i] *= 2;      // Combine values
            m_score += newLine[i]; // Update session score
            newLine.erase(newLine.begin() + i + 1); // Remove the merged secondary tile
            changed = true;
            // The loop increment handles the 'once per move' rule correctly
        }
    }

    // Step 3: Pad with zeros to maintain dimensions
    while (newLine.size() < (size_t)line.size()) newLine.push_back(0);
    
    // Check if any change occurred (sliding or merging)
    if (newLine != line) changed = true;
    line = newLine;
    return changed;
}

/**
 * @brief Spawns a new tile (2 or 4) in a random empty cell.
 * Probabilities are governed by Config parameters P and Q.
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

    // Pick a random empty cell
    std::uniform_int_distribution<int> dist(0, emptyCells.size() - 1);
    auto [r, c] = emptyCells[dist(m_rng)];

    // Determine value: 2 with P% probability, else 4
    std::uniform_int_distribution<int> valDist(1, 100);
    int v = valDist(m_rng); m_grid[r][c] = (v <= m_p) ? 2 : 4;
}

/**
 * @brief Checks if any valid moves are remaining on the board.
 * @return true if a move is possible, false if the game is lost.
 */
bool GameEngine::canMove() const
{
    for (int i = 0; i < m_rows; ++i) {
        for (int j = 0; j < m_cols; ++j) {
            // Check for empty spaces
            if (m_grid[i][j] == 0) return true;
            // Check for horizontal merge possibilities
            if (i + 1 < m_rows && m_grid[i][j] == m_grid[i+1][j]) return true;
            // Check for vertical merge possibilities
            if (j + 1 < m_cols && m_grid[i][j] == m_grid[i][j+1]) return true;
        }
    }
    return false;
}

/**
 * @brief Checks if the player has reached the target value K.
 */
bool GameEngine::hasWon() const
{
    for (int i = 0; i < m_rows; ++i) {
        for (int j = 0; j < m_cols; ++j) {
            if (m_grid[i][j] >= m_target) return true;
        }
    }
    return false;
}

// Configuration setters
void GameEngine::setDimensions(int rows, int cols) { m_rows = rows; m_cols = cols; reset(); }
void GameEngine::setTargetValue(int target) { m_target = target; }
void GameEngine::setGenerationChances(int p, int q) { m_p = p; m_q = q; }
