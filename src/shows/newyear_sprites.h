// The New Year show's 8x8 pictures, one byte per row, top row first, leftmost pixel in the high bit.
#pragma once
#include "../platform.h"

// A champagne bottle standing up: cork, neck, shoulders, label
static const uint8_t BOTTLE[8] PROGMEM = {
  0b00011000,
  0b00011000,
  0b00011000,
  0b00111100,
  0b00111100,
  0b00100100,
  0b00111100,
  0b00111100,
};

// A champagne glass with bubbles above it
static const uint8_t GLASS[8] PROGMEM = {
  0b00101000,
  0,
  0b01111100,
  0b01111100,
  0b00111000,
  0b00010000,
  0b00010000,
  0b00111000,
};

// A party hat with a pompom on top
static const uint8_t PARTY_HAT[8] PROGMEM = {
  0b00010000,
  0b00010000,
  0b00111000,
  0b00101000,
  0b01111100,
  0b01010100,
  0b11111110,
  0,
};

// A clock at midnight: both hands on the 12
static const uint8_t CLOCK[8] PROGMEM = {
  0b00111100,
  0b01011010,
  0b10011001,
  0b10011001,
  0b10000001,
  0b10000001,
  0b01000010,
  0b00111100,
};

// A star
static const uint8_t STAR[8] PROGMEM = {
  0b00010000,
  0b00010000,
  0b11111110,
  0b01111100,
  0b00111000,
  0b01101100,
  0b01000100,
  0,
};
