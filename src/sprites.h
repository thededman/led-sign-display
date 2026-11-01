// 8x8 pictures, one byte per row, top row first, leftmost pixel in the high bit.
// Characters face right; the show mirrors them to face left.
#pragma once
#include "platform.h"

// Pac-Man: mouth wide open, half open, closed
static const uint8_t PAC[3][8] PROGMEM = {
  { 0b00111100,
    0b01111110,
    0b11111000,
    0b11110000,
    0b11110000,
    0b11111000,
    0b01111110,
    0b00111100 },
  { 0b00111100,
    0b01111110,
    0b11111110,
    0b11111000,
    0b11111000,
    0b11111110,
    0b01111110,
    0b00111100 },
  { 0b00111100,
    0b01111110,
    0b11111111,
    0b11111111,
    0b11111111,
    0b11111111,
    0b01111110,
    0b00111100 },
};

// Ghost looking right, two wiggle frames
static const uint8_t GHOST[2][8] PROGMEM = {
  { 0b00111100,
    0b01111110,
    0b11001001,
    0b11001001,
    0b11111111,
    0b11111111,
    0b11111111,
    0b11011011 },
  { 0b00111100,
    0b01111110,
    0b11001001,
    0b11001001,
    0b11111111,
    0b11111111,
    0b11111111,
    0b10110110 },
};

// Frightened ghost (after Pac-Man eats a power pellet)
static const uint8_t SCARED[2][8] PROGMEM = {
  { 0b00111100,
    0b01111110,
    0b11111111,
    0b11011011,
    0b11111111,
    0b10101011,
    0b11010101,
    0b11011011 },
  { 0b00111100,
    0b01111110,
    0b11111111,
    0b11011011,
    0b11111111,
    0b10101011,
    0b11010101,
    0b10110110 },
};

// What's left of a ghost after Pac-Man eats it
static const uint8_t EYES[8] PROGMEM = {
  0b00000000,
  0b00000000,
  0b01100110,
  0b01100110,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
};

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
