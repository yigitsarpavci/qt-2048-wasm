#!/bin/bash
# Masterpiece Local Runner v10.0.1
set -e

echo "--- STARTING LOCAL BUILD ---"
rm -rf build_local
mkdir build_local
cmake -S . -B build_local -DCMAKE_BUILD_TYPE=Release
cmake --build build_local --parallel $(sysctl -n hw.ncpu)

echo "✅ BUILD SUCCESSFUL."
echo "🚀 Running 2048..."
./build_local/2048
