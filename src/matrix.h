// MAX7219 driver for a row of 8x8 modules
#pragma once
#include <stdint.h>

void matrixBegin(uint8_t orientation, uint8_t brightness);
void matrixOrientation(uint8_t orientation);  // 0-15, see matrix.cpp
void matrixBrightness(uint8_t level);         // 0-15

// Sends one frame: WIDTH columns, left to right, bit 0 = top row
void matrixShow(const uint8_t *fb);
