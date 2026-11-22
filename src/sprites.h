// 8x8 pictures every show shares, one byte per row, top row first, leftmost pixel in the high bit.
#pragma once
#include "platform.h"

// A football, laces up
static const uint8_t FOOTBALL[8] PROGMEM = {
  0,
  0b00111100,
  0b01010110,
  0b10111101,
  0b01010110,
  0b00111100,
  0, 0,
};
