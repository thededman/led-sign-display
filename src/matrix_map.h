// How a frame's pixels land in the MAX7219 registers; shared by the UNO and Pi drivers
#pragma once
#include <stdint.h>
#include "config.h"

// MAX7219 registers (digits are 1-8)
enum { REG_DIGIT0 = 0x01, REG_DECODE = 0x09, REG_INTENSITY = 0x0A,
       REG_SCANLIMIT = 0x0B, REG_SHUTDOWN = 0x0C, REG_TEST = 0x0F };

// Orientation bits. Module makers wire the LEDs differently and the chain can enter from
// either end, so these four switches (16 layouts) cover every combination.
enum {
  O_CHAIN_RIGHT = 1,  // the first module in the chain is the rightmost, seen from the front
  O_DIGIT_COLS  = 2,  // the chip's digit registers drive columns instead of rows
  O_FLIP_X      = 4,  // mirror each module left-right
  O_FLIP_Y      = 8,  // mirror each module top-bottom
};

// The 8 bits of one digit register for the module at chain position `dev`
inline uint8_t digitBits(const uint8_t *fb, uint8_t orient, uint8_t dev, uint8_t digit) {
  uint8_t module = (orient & O_CHAIN_RIGHT) ? MODULES - 1 - dev : dev;  // counted from the left
  uint8_t out = 0;
  for (uint8_t b = 0; b < 8; b++) {
    uint8_t x = (orient & O_DIGIT_COLS) ? digit : b;
    uint8_t y = (orient & O_DIGIT_COLS) ? b : digit;
    if (orient & O_FLIP_X) x = 7 - x;
    if (orient & O_FLIP_Y) y = 7 - y;
    if (fb[module * 8 + x] & (1 << y)) out |= 1 << b;
  }
  return out;
}
