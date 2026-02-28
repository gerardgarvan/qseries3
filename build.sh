#!/usr/bin/env bash
# Build qseries without make. Use g++ or x86_64-w64-mingw32-g++.
set -e
mkdir -p dist
if command -v x86_64-w64-mingw32-g++ >/dev/null 2>&1; then
  x86_64-w64-mingw32-g++ -std=c++20 -O2 -static -Wall -Wextra -o dist/qseries.exe src/main.cpp
else
  g++ -std=c++20 -O2 -static -Wall -Wextra -o dist/qseries.exe src/main.cpp 2>/dev/null || \
  g++ -std=c++20 -O2 -Wall -Wextra -o dist/qseries.exe src/main.cpp
fi
echo "Built dist/qseries.exe"
mkdir -p dist/demo
cp demo/garvan-demo.sh dist/demo/
cp demo/README.md dist/demo/ 2>/dev/null || true
echo "Copied demo to dist/demo/"
