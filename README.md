# 🎮 Qt 2048 WebAssembly 

![Qt6](https://img.shields.io/badge/Qt-6.x-41CD52?logo=qt&logoColor=white) ![C++](https://img.shields.io/badge/C++-17-00599C?logo=c%2B%2B&logoColor=white) ![WASM](https://img.shields.io/badge/WebAssembly-WASM-654FF0?logo=webassembly&logoColor=white) ![License](https://img.shields.io/badge/License-MIT-yellow.svg)

A high-performance, cross-platform implementation of the 2048 puzzle game, engineered with **C++17** and the **Qt 6** framework. This project demonstrates advanced systems programming concepts, including directional grid merging algorithms, infinite state-history management for undos, and cross-compilation for modern browsers via **WebAssembly**.

---

### 🚀 Live Demo
**Play in your browser:** [https://yigitsarpavci.github.io/qt-2048-wasm/](https://yigitsarpavci.github.io/qt-2048-wasm/)

---

### ✨ Key Features

*   **🎮 Multiple Game Modes:**
    *   **Normal**: Traditional 2048 goal with target $K$.
    *   **Unlimited**: Endless play for high-score chasers.
    *   **Hard Mode**: Intense 5-second countdown penalty (random move on timeout).
*   **⏪ Infinite Undo History:** Revert to any state since the start of the session using a deep-copy stack mechanism.
*   **⚡ High Performance:** Optimized $O(N \cdot M)$ grid processing logic.
*   **📱 Responsive UI:** Aesthetic parity with the original 2048 game, fully responsive and fluid.
*   **🌐 Web Deployment:** Integrated CI/CD workflow that compiles and deploys to GitHub Pages automatically.

---

### 🛠 Technical Architecture

- **Engine Logic**: Decoupled `GameEngine` class using `std::vector` for grid representation and `std::stack` for history.
- **Directional Normalization**: All moves (Up/Down/Right) are mathematically reduced to a "Left" move to minimize code duplication and ensure algorithmic consistency.
- **Event Filtering**: Low-level key interception to ensure smooth WASM interaction, bypassing browser-level keyboard conflicts.

---

### 🔨 Building from Source

```bash
# Clone the repository
git clone https://github.com/bouncmpe230/2048-2023400048-2023400210
cd 2048-2023400048-2023400210

# Build with CMake
mkdir build && cd build
cmake ..
cmake --build .
```

---

### 📄 Documentation
Detailed technical implementation, class diagrams, and signal-slot mapping can be found in the [Project Report (PDF)](./report.pdf).

---
*Developed by Yiğit Sarp Avcı & Doğukan Sungu*
