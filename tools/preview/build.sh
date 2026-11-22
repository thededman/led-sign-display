#!/bin/sh
# Builds the show for the Mac and writes tools/preview/build/preview.html
set -e
# PREVIEW_TODAY and PREVIEW_SCORES may be relative to where you ran this from
for var in PREVIEW_TODAY PREVIEW_SCORES; do
  eval "path=\${$var:-}"
  case "$path" in
    "" | /*) ;;
    *) export "$var=$PWD/$path" ;;
  esac
done
cd "$(dirname "$0")"
mkdir -p build
clang++ -std=c++17 -O2 -Wall -Wextra -I ../../src -o build/preview preview.cpp \
  ../../src/effects.cpp ../../src/show.cpp ../../src/today.cpp ../../src/shows/*.cpp
./build/preview build/preview.html
