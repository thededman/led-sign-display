// The shows. They draw into fb; the board (or the Mac preview) puts fb on the LEDs.
#pragma once
#include <stdint.h>
#include "config.h"

extern uint8_t fb[WIDTH];  // one byte per column, left to right, bit 0 = top row

// Supplied by the platform
bool frame(uint16_t ms);   // shows fb for ms; returning false skips the rest of the scene
uint32_t rnd(uint32_t n);  // random number from 0 to n-1
// A message typed on the phone page, or nullptr; *isNew is set the first time it's asked for
const char *phoneMessage(bool *isNew);
// Today's football games, one line each ("BUF 17 @ KC 21 Q3 4:32"); a leading '*' marks the
// favourite team's game. None on the UNO.
uint8_t scoreCount();
const char *scoreLine(uint8_t i);
// The day's values from the Pi's today file ("show", "why", "names", "turning"), or "" (never
// null; the UNO has none). The text may change between frames, but the pointer stays valid.
const char *today(const char *key);

void runShow();  // one full pass of the show the day calls for

// The shows the sign holds (src/show.cpp)
struct ShowEntry {
  const char *name;
  void (*run)();
};
const ShowEntry *resolveShow(const char *name);  // an unknown or empty name: the first show
bool showChanged();  // the today file now names a different show from the one playing
