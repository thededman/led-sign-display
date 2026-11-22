#!/bin/sh
# Builds the show for the Mac and writes tools/preview/build/preview.html
set -e
cd "$(dirname "$0")"
mkdir -p build
clang++ -std=c++17 -O2 -Wall -Wextra -I ../../src -o build/preview preview.cpp \
  ../../src/effects.cpp ../../src/show.cpp ../../src/today.cpp ../../src/shows/*.cpp
./build/preview build/preview.html
