/**
 * @file gameengine.h
 * @brief Professional 2048 Engine with ID Determinism & Doxygen Docs.
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
 * @brief Valid movement directions in the 2D grid.
 */
enum class Direction { Up, Down, Left, Right };

/**
 * @struct TileMove
 * @brief Encapsulates a tile transition for UI synchronization.
 */
struct TileMove {
    int id;           ///< Unique identifier of the tile
    int fromRow;      ///< Starting row index
    int fromCol;      ///< Starting column index
    int toRow;        ///< Ending row index
    int toCol;        ///< Ending column index
    int value;        ///< Value of the tile after the transition
    bool merged;      ///< True if this move resulted in a merge
};

/**
 * @struct Tile
 * @brief Represents a single grid cell.
 */
struct Tile {
    int value = 0;    ///< Numeric value (0 for empty)
    int id = -1;      ///< Persistence ID for animation tracking
};

/**
 * @struct GameSnapshot
 * @brief Capture of the game state for atomic Undo/Redo.
 */
struct GameSnapshot {
    std::vector<std::vector<Tile>> grid; ///< Grid state
    int score;                           ///< Current score
    int bestScore;                       ///< Best score at that moment
    int nextId;                          ///< ID counter state for determinism
};

/**
 * @class GameEngine
 * @brief High-performance, deterministic engine for 2048 logic.
 */
class GameEngine {
public:
    /**
     * @brief Constructs the engine with specific grid dimensions.
     * @param n Rows count
     * @param m Columns count
     * @param k Target value for victory
     */
    GameEngine(int n, int m, int k);

    /**
     * @brief Processes a grid transformation in the given direction.
     * @param dir The requested direction.
     * @return true if the board state changed.
     */
    bool move(Direction dir);

    /**
     * @brief Resets the internal state to the initial setup.
     */
    void reset();

    /**
     * @brief Reverts the board to the last stored snapshot.
     */
    void undo();

    // --- Accessors ---
    const std::vector<std::vector<Tile>>& getGrid() const { return m_grid; }
    int getScore() const { return m_score; }
    int getBestScore() const { return m_bestScore; }
    const std::vector<TileMove>& getLastMoves() const { return m_lastMoves; }
    int getSpawnId() const { return m_spawnId; }
    int getHistoryDepth() const { return (int)m_history.size(); }
    int getTxCount() const { return m_txCounter; }
    
    /** @return true if a tile >= K exists in the grid. */
    bool hasWon() const;
    
    /** @return true if empty cells exist or valid merges are possible. */
    bool canMove() const;

    // --- Developer & Configuration API ---
    void setTile(int r, int c, int val) { if (r>=0 && r<m_rows && c>=0 && c<m_cols) { m_grid[r][c].value = val; m_grid[r][c].id = -1; } }
    void setScore(int s) { m_score = s; }
    void setSpawnEnabled(bool e) { m_spawnEnabled = e; }
    void setBestScore(int s) { m_bestScore = s; }
    
    /**
     * @brief Updates the tile generation probability.
     * @param p Chance of spawning a '2' (0.0 to 1.0).
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
// v1.2: Modes
