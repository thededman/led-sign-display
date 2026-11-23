// The Christmas show's 8x8 pictures, one byte per row, top row first, leftmost pixel in the high bit.
#pragma once
#include "../platform.h"

// A reindeer galloping right: antlers, head, body, and two strides of legs
static const uint8_t REINDEER[2][8] PROGMEM = {
  { 0b00000101,
    0b00000010,
    0b00000111,
    0b01111110,
    0b11111100,
    0b01111000,
    0b01001000,
    0b10000100 },
  { 0b00000101,
    0b00000010,
    0b00000111,
    0b01111110,
    0b11111100,
    0b01111000,
    0b00110000,
    0b00101000 },
};

// Santa in his sleigh, heading right: hat, head, the sleigh's curled front, and a runner
static const uint8_t SLEIGH[8] PROGMEM = {
  0b00110000,
  0b01110000,
  0b00110000,
  0b10110001,
  0b11111110,
  0b01111100,
  0b01000100,
  0b11111111,
};

// A little Christmas tree with a star on top
static const uint8_t TREE[8] PROGMEM = {
  0b00010000,
  0b00111000,
  0b00010000,
  0b00111000,
  0b01111100,
  0b11111110,
  0b00010000,
  0b00111000,
};

// A present with a bow
static const uint8_t PRESENT[8] PROGMEM = {
  0b01000100,
  0b00101000,
  0b11111110,
  0b10010010,
  0b11111110,
  0b10010010,
  0b10010010,
  0b11111110,
};

// A snowman in a top hat: hat rows 0-1, head 2-3, body 4-7
static const uint8_t SNOWMAN[8] PROGMEM = {
  0b00111000,
  0b01111100,
  0b00111000,
  0b00111000,
  0b01111100,
  0b11111110,
  0b11111110,
  0b01111100,
};

// A candy cane
static const uint8_t CANDY_CANE[8] PROGMEM = {
  0b00111000,
  0b01000100,
  0b01000100,
  0b00000100,
  0b00000100,
  0b00000100,
  0b00000100,
  0b00000100,
};

// A gingerbread man running: head, arms out, two strides
static const uint8_t GINGERBREAD[2][8] PROGMEM = {
  { 0b00011000,
    0b00011000,
    0b01111110,
    0b00111100,
    0b00111100,
    0b00100100,
    0b01000010,
    0 },
  { 0b00011000,
    0b00011000,
    0b01111110,
    0b00111100,
    0b00111100,
    0b00011000,
    0b00100100,
    0 },
};

// A bell: a rounded top, straight sides, a flared lip and the clapper below
static const uint8_t BELL[8] PROGMEM = {
  0b00011000,
  0b00111100,
  0b00111100,
  0b00111100,
  0b01111110,
  0b11111111,
  0b00011000,
  0,
};
