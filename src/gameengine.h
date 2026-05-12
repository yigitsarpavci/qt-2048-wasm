#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <vector>
#include <stack>
#include <random>

/**
 * @enum Direction
 * @brief Represents the four possible movement vectors on the game grid.
 */
enum class Direction { Up, Down, Left, Right };

/**
 * @struct GameState
 * @brief Memento pattern structure to store the game state for undo operations.
 */
struct GameState {
    std::vector<std::vector<int>> grid; ///< Snapshot of the N x M board.
    int score;                          ///< Snapshot of the current session score.
    int bestScore;                      ///< Snapshot of the persistent best score.
};

/**
 * @class GameEngine
 * @brief Core logic controller for the 2048 game engine.
 * 
 * Implements the mathematical and procedural rules of 2048, including tile sliding,
 * merging, and state persistence for undo functionality. Strictly follows the 
 * Section 4 requirements of the project description.
 */
class GameEngine {
public:
    /**
     * @brief Initializes the engine with specified dimensions and target.
     * @param rows Vertical dimension (N).
     * @param cols Horizontal dimension (M).
     * @param target The winning tile value (K).
     */
    GameEngine(int rows, int cols, int target);

    bool move(Direction dir);           ///< Executes a move and spawns new tile if valid.
    void undo();                        ///< Reverts the grid and score to the previous state.
    void reset();                       ///< Restarts the engine from zero.
    
    // Status Accessors
    bool hasWon() const;                ///< Checks if target K is reached.
    bool canMove() const;               ///< Checks for latent valid moves (loss check).
    int getScore() const { return m_score; }
    int getBestScore() const { return m_bestScore; }
    const std::vector<std::vector<int>>& getGrid() const { return m_grid; }
    bool hasHistory() const { return !m_history.empty(); }

    // Configuration Management (Section 4 Compliance)
    void setDimensions(int rows, int cols);
    void setTargetValue(int target);
    void setGenerationChances(int p, int q);

private:
    void addRandomTile();               ///< Spawns a 2 or 4 based on P/Q probabilities.
    bool slideAndMerge(std::vector<int>& line); ///< Core 1D vector merge logic.
    void saveState();                   ///< Pushes current state to the history stack.

    int m_rows, m_cols, m_target;       ///< N, M, and K constants.
    int m_p, m_q;                       ///< Probability weights for 2 and 4.
    int m_score, m_bestScore;           ///< Score tracking.
    
    std::vector<std::vector<int>> m_grid; ///< The N x M game board.
    std::stack<GameState> m_history;     ///< Stack for multi-level undo support.
    std::mt19937 m_rng;                  ///< Mersenne Twister for high-quality randomness.
};

#endif // GAMEENGINE_H
