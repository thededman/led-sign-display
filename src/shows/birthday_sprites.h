// The birthday show's 8x8 pictures, one byte per row, top row first, leftmost pixel in the high bit.
#pragma once
#include "../platform.h"

// Balloon on a string, two wiggle frames
static const uint8_t BALLOON[2][8] PROGMEM = {
  { 0b00111100,
    0b01111110,
    0b01111110,
    0b01111110,
    0b00111100,
    0b00011000,
    0b00010000,
    0b00001000 },
  { 0b00111100,
    0b01111110,
    0b01111110,
    0b01111110,
    0b00111100,
    0b00011000,
    0b00001000,
    0b00010000 },
};

// A present: lid with a bow, and the box below it, drawn separately so the lid can pop off
static const uint8_t GIFT_LID[8] PROGMEM = {
  0b01100110,
  0b00011000,
  0b11111111,
  0, 0, 0, 0, 0,
};
static const uint8_t GIFT_BOX[8] PROGMEM = {
  0, 0, 0,
  0b10011001,
  0b10011001,
  0b10011001,
  0b10011001,
  0b11111111,
};

// Cupcake with a candle, the flame flickering between two frames
static const uint8_t CUPCAKE[2][8] PROGMEM = {
  { 0b00010000,
    0b00010000,
    0b00111100,
    0b01111110,
    0b11111111,
    0b01111110,
    0b01011010,
    0b00111100 },
  { 0b00001000,
    0b00010000,
    0b00111100,
    0b01111110,
    0b11111111,
    0b01111110,
    0b01011010,
    0b00111100 },
};

// Striped party hat with a pom-pom
static const uint8_t PARTY_HAT[8] PROGMEM = {
  0b00011000,
  0b00011000,
  0b00100100,
  0b00111100,
  0b01000010,
  0b01111110,
  0b10000001,
  0b11111111,
};

// Two music notes
static const uint8_t NOTES[8] PROGMEM = {
  0b00111111,
  0b00100001,
  0b00100001,
  0b00100001,
  0b11100111,
  0b11100111,
  0, 0,
};
