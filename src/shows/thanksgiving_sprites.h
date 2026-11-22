// The Thanksgiving show's 8x8 pictures, one byte per row, top row first, leftmost pixel in the high bit.
#pragma once
#include "../platform.h"

// A turkey walking left: beak and wattle at the front, tail fan up behind, two strides
static const uint8_t TURKEY[2][8] PROGMEM = {
  { 0b01000011,
    0b11100111,
    0b11001111,
    0b01111111,
    0b01111110,
    0b00111100,
    0b00100100,
    0b01100110 },
  { 0b01000011,
    0b11100111,
    0b11001111,
    0b01111111,
    0b01111110,
    0b00111100,
    0b00011000,
    0b00100100 },
};

// Pilgrim hat: tall crown, band with a buckle, wide brim
static const uint8_t PILGRIM_HAT[8] PROGMEM = {
  0,
  0b00111100,
  0b00111100,
  0b00100100,
  0b00111100,
  0b11111111,
  0, 0,
};

// Maple leaf on its stem
static const uint8_t LEAF[8] PROGMEM = {
  0b00010000,
  0b01010100,
  0b01111100,
  0b11111110,
  0b01111100,
  0b00111000,
  0b00010000,
  0b00010000,
};
