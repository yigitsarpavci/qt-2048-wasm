/**
 * @file gameengine.h
 * @brief Core logic for the 2048 game mechanics, including grid management, movement, and history tracking.
 */

#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <deque>
#include <vector>
#include <random>
#include <optional>
#include <QtCore/qglobal.h>

/**
 * @enum Direction
 * @brief Represents the four cardinal directions for tile movement.
 */
enum class Direction { Up, Down, Left, Right };

/**
 * @struct TileMove
 * @brief Encapsulates metadata for a tile transition, used for UI animations.
 */
struct TileMove {
    int id;           ///< Unique persistent identifier of the tile
    int fromRow;      ///< Source row index
    int fromCol;      ///< Source column index
    int toRow;        ///< Destination row index
    int toCol;        ///< Destination column index
    int value;        ///< Tile value after transition (e.g., doubled if merged)
    bool merged;      ///< Indicates if the transition involved a merge
};

/**
 * @struct Tile
 * @brief Represents a single cell in the game grid.
 */
struct Tile {
    int value = 0;    ///< Numeric value of the tile (0 indicates an empty cell)
    int id = -1;      ///< Persistent ID used for tracking and animating the tile across moves
};

/**
 * @struct GameSnapshot
 * @brief Stores the full state of the game for undo operations.
 */
struct GameSnapshot {
    std::vector<std::vector<Tile>> grid; ///< Current state of the grid
    int score;                           ///< Current player score
    int bestScore;                       ///< Best score achieved up to this state
    int nextId;                          ///< Next available ID for tile generation
};

/**
 * @class GameEngine
 * @brief Handles the deterministic game logic, move processing, and state management for 2048.
 */
class GameEngine {
public:
    /**
     * @brief Constructs the game engine with specified dimensions and goal.
     * @param n Number of rows
     * @param m Number of columns
     * @param k Target value for a winning tile (e.g., 2048)
     */
    GameEngine(int n, int m, int k);

    /**
     * @brief Processes a move attempt in the specified direction.
     * @param dir The direction to slide the tiles.
     * @return true if any tiles moved or merged, triggering a state update and spawn.
     */
    bool move(Direction dir);

    /**
     * @brief Resets the engine to a clean starting state with two initial tiles.
     */
    void reset();

    /**
     * @brief Reverts the game state to the most recent snapshot in the history stack.
     */
    void undo();

    // --- State Accessors ---
    const std::vector<std::vector<Tile>>& getGrid() const { return m_grid; }
    int getScore() const { return m_score; }
    int getBestScore() const { return m_bestScore; }
    const std::vector<TileMove>& getLastMoves() const { return m_lastMoves; }
    int getSpawnId() const { return m_spawnId; }
    int getHistoryDepth() const { return (int)m_history.size(); }
    int getTxCount() const { return m_txCounter; }
    
    /**
     * @brief Checks if the player has achieved the winning tile value.
     */
    bool hasWon() const;
    
    /**
     * @brief Determines if any valid moves remain (empty cells or possible merges).
     */
    bool canMove() const;

    // --- Configuration and Debugging ---
    void setTile(int r, int c, int val) { if (r>=0 && r<m_rows && c>=0 && c<m_cols) { m_grid[r][c].value = val; m_grid[r][c].id = -1; } }
    void setScore(int s) { m_score = s; }
    void setSpawnEnabled(bool e) { m_spawnEnabled = e; }
    void setBestScore(int s) { m_bestScore = s; }
    
    /**
     * @brief Configures the probability of spawning a '2' vs a '4'.
     * @param p Probability (0.0 to 1.0) of generating a '2'.
     */
    void setGenerationChances(double p);

private:
    void spawnTiles(int count);
    void saveSnapshot();
    void logTransaction(int delta, int merges);

    int m_rows, m_cols, m_k;
    double m_p = 0.9; 
    std::vector<std::vector<Tile>> m_grid;
    std::deque<GameSnapshot> m_history;
    std::vector<TileMove> m_lastMoves;
    
    int m_score = 0;
    int m_bestScore = 0;
    int m_nextId = 1;
    int m_spawnId = -1;
    int m_txCounter = 0; 
    bool m_spawnEnabled = true;

    std::mt19937 m_rng;
};

#endif // GAMEENGINE_H
