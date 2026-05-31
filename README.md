# 2048 Game Engine - CMPE 230 Assignment 3

A high-performance 2048 game engine developed using C++ and the Qt Framework. This project demonstrates advanced state management, unlimited undo functionality, and cross-platform compatibility (Desktop & WebAssembly).

## 🎮 Game Features
- **Core Engine**: Full 2048 logic with tile merging and score tracking.
- **Game Modes**:
  - **Normal**: Standard 2048 experience.
  - **Unlimited**: Play beyond 2048 without limits.
  - **Hard**: Time-constrained mode for advanced players.
- **Unlimited Undo**: Efficient state-stack implementation for infinite undos.
- **Persistent High Scores**: Local storage for best performance tracking.

## ⌨️ Controls
- **Arrow Keys**: Move tiles (Up, Down, Left, Right).
- **U Key**: Undo last move.
- **R Key**: Start a new game.
- **Numbers (1, 2, 3)**: Quick-switch between game modes.

## 🛠️ Technical Specifications
- **Framework**: Qt 6.6.1
- **Language**: C++17
- **Target**: Desktop & WebAssembly (WASM)
- **UI Architecture**: Custom QWidget-based layout with dynamic stylesheet (QSS) injection.

## 🚀 Building the Project
### Local Desktop Build
```bash
mkdir build && cd build
cmake ..
make
./2048
```

---
*Developed for CMPE 230: Systems Programming - Spring 2026*
