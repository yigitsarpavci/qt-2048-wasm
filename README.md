# CmpE 230 - Assignment 3: 2048 Game

## Project Overview
This project is an implementation of the 2048 puzzle game using the Qt 6 framework. It features a robust grid-based movement engine, multiple game modes, and persistent high-score tracking. The application is designed to be cross-platform and is optimized for WebAssembly (WASM).

## Authors
- **Yiğit Sarp Avcı** (2023400048)
- **Doğukan Sungu** (2023400210)

## Features
- **Game Modes**: 
  - **Normal**: Standard 2048 gameplay.
  - **Unlimited**: Play beyond the target tile value.
  - **Hard Mode**: A 5-second timer forces a random move upon timeout.
- **Undo Functionality**: Unlimited undo support via state history snapshots.
- **Responsive Design**: Fluid UI that adapts to window resizing.
- **Persistent Scoring**: Best score is saved across sessions using local storage.

## Build and Run Instructions

### Prerequisites
- Qt 6.x
- CMake 3.16 or higher
- C++17 compliant compiler

### Local Build
```bash
mkdir build
cd build
cmake ..
cmake --build .
./2048
```

### WebAssembly (WASM) Build
The project is configured to be compiled with Emscripten for WASM deployment. Refer to the GitHub Actions workflow in `.github/workflows/deploy-wasm.yml` for automated build steps.

## Documentation
A detailed technical report including class descriptions, merging strategy, and implementation details is available in `report.pdf`.
