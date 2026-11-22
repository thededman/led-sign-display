#!/bin/sh
# Runs every test: the show code (C++) and the Pi's Python services.
set -e
cd "$(dirname "$0")/.."
mkdir -p tests/build
${CXX:-c++} -std=c++17 -O1 -Wall -Wextra -I src -o tests/build/show_test tests/show_test.cpp \
  src/effects.cpp src/show.cpp src/today.cpp src/shows/*.cpp
tests/build/show_test
python3 -m unittest discover -s tests
