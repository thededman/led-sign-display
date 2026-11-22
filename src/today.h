// The Pi's today file: which show plays and the words it needs, written by the scores service
// (pi/scores.py) as key=value lines.
#pragma once

struct Today {
  char show[24];     // thanksgiving, birthday, ...
  char why[8];       // auto or picked
  char names[64];    // birthday names: "ALEX & ROBIN"
  char turning[96];  // "ROBIN TURNS 60!"
};

// Fills t from the file's text; unknown keys and lines without '=' are skipped, values that
// don't fit are cut, and keys missing from the text are left empty
void parseToday(const char *text, Today *t);
const char *todayValue(const Today &t, const char *key);  // "" for an unknown key
