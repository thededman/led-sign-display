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

// Bat, wings up and wings down
static const uint8_t BAT[2][8] PROGMEM = {
  { 0b10000001,
    0b11011011,
    0b01111110,
    0b00011000,
    0, 0, 0, 0 },
  { 0b00100100,
    0b00111100,
    0b11111111,
    0b10000001,
    0, 0, 0, 0 },
};

// Jack-o'-lantern, eyes open and eyes shut
static const uint8_t PUMPKIN[2][8] PROGMEM = {
  { 0b00001000,
    0b01111110,
    0b11111111,
    0b10011001,
    0b11111111,
    0b10000001,
    0b11011011,
    0b01111110 },
  { 0b00001000,
    0b01111110,
    0b11111111,
    0b11111111,
    0b11111111,
    0b10000001,
    0b11011011,
    0b01111110 },
};
