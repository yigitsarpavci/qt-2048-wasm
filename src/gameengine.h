#pragma once

#include <vector>
#include <stack>
#include <random>

enum class Direction {
    Up,
    Down,
    Left,
    Right
};

/**
 * @brief Represents a snapshot of the game state for Undo functionality.
 */
struct GameState {
    std::vector<std::vector<int>> grid;
    int score;
    int bestScore;
};

/**
 * @brief The GameEngine class handles the core logic of the 2048 game.
 * It manages the grid state, tile merging, random generation, and movement.
 * This class is designed to be independent of the UI layer.
 */
class GameEngine {
public:
    GameEngine(int rows = 4, int cols = 4, int target = 2048);

    void reset();
    bool move(Direction dir);
    void undo();
    bool canMove() const;
    bool hasWon() const;

    const std::vector<std::vector<int>>& getGrid() const { return m_grid; }
    int getScore() const { return m_score; }
    int getBestScore() const { return m_bestScore; }
    bool hasHistory() const { return !m_history.empty(); }

    void setDimensions(int rows, int cols);
    void setTargetValue(int target);
    void setGenerationChances(int p, int q); // p for 2, q for 4

private:
    void addRandomTile();
    bool slideAndMerge(std::vector<int>& line);
    void saveState();

    int m_rows;
    int m_cols;
    int m_target;
    int m_p; // Chance of 2 (%)
    int m_q; // Chance of 4 (%)
    int m_score;
    int m_bestScore;
    std::vector<std::vector<int>> m_grid;
    std::stack<GameState> m_history;

    std::mt19937 m_rng;
};
