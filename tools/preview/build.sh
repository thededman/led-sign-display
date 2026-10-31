#!/bin/sh
# Builds the show for the Mac and writes tools/preview/build/preview.html
set -e
cd "$(dirname "$0")"
mkdir -p build
clang++ -std=c++17 -O2 -Wall -Wextra -I ../../src preview.cpp ../../src/show.cpp -o build/preview
./build/preview build/preview.html
