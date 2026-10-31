// The Halloween show. It draws into fb; the board (or the Mac preview) puts fb on the LEDs.
#pragma once
#include <stdint.h>
#include "config.h"

extern uint8_t fb[WIDTH];  // one byte per column, left to right, bit 0 = top row

// Supplied by the platform
bool frame(uint16_t ms);   // shows fb for ms; returning false skips the rest of the scene
uint32_t rnd(uint32_t n);  // random number from 0 to n-1

void runShow();                   // one full pass of the show
void finderCard(uint8_t layout);  // orientation finder card, e.g. "7 READ ME"
