// What every show draws with: letters, sprites, shared effects and the scene runner.
#include "effects.h"
#include "font.h"
#include "sprites.h"

uint8_t fb[WIDTH];
uint8_t bg[WIDTH];
bool sceneStopped;
Marcher showMarcher;

// ---------------------------------------------------------------- drawing

void wipe(uint8_t *buf) { memset(buf, 0, WIDTH); }

void setPixel(uint8_t *buf, int x, int y) {
  if (x >= 0 && x < WIDTH && y >= 0 && y < 8) buf[x] |= 1 << y;
}

// A column moved down by y rows (up if y is negative)
static uint8_t shifted(uint8_t col, int y) {
  if (y <= -8 || y >= 8) return 0;
  return y >= 0 ? col << y : col >> -y;
}

static const uint8_t *glyph(char c) {
  if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
  if (c < ' ' || c > 'Z') c = '?';
  return FONT + (c - ' ') * 5;
}

// Width of a character with its blank side columns trimmed; *first is its first used column
static uint8_t charSpan(char c, uint8_t *first) {
  const uint8_t *g = glyph(c);
  uint8_t a = 0, b = 4;
  while (a < 5 && !pgm_read_byte(g + a)) a++;
  if (a == 5) {  // space
    *first = 0;
    return 2;
  }
  while (!pgm_read_byte(g + b)) b--;
  *first = a;
  return b - a + 1;
}

// Draws a character with its left edge at x (`wide` = 2 doubles its width); returns its width
uint8_t drawChar(uint8_t *buf, char c, int x, int y, uint8_t wide) {
  uint8_t first, w = charSpan(c, &first);
  const uint8_t *g = glyph(c) + first;
  for (uint8_t i = 0; i < w * wide; i++) {
    int cx = x + i;
    if (cx >= 0 && cx < WIDTH) buf[cx] |= shifted(pgm_read_byte(g + i / wide), y);
  }
  return w * wide;
}

uint8_t textLen(const char *s) {
  uint8_t n = 0;
  while (pgm_read_byte(s + n)) n++;
  return n;
}

int textWidth(const char *s, uint8_t wide) {
  int w = 0;
  uint8_t first;
  for (char c; (c = pgm_read_byte(s)); s++) w += (charSpan(c, &first) + 1) * wide;
  return w > 0 ? w - wide : 0;
}

// Draws text from flash with its left edge at x
void drawText(uint8_t *buf, const char *s, int x, int y, uint8_t wide) {
  for (char c; (c = pgm_read_byte(s)); s++) x += drawChar(buf, c, x, y, wide) + wide;
}

int centered(const char *s, uint8_t wide) { return (WIDTH - textWidth(s, wide)) / 2; }

void drawSprite(uint8_t *buf, const uint8_t *rows, int x, int y) {
  for (uint8_t r = 0; r < 8; r++) {
    uint8_t bits = pgm_read_byte(rows + r);
    for (uint8_t c = 0; c < 8; c++)
      if (bits & (0x80 >> c)) setPixel(buf, x + c, y + r);
  }
}


// A text's widest size that fits on the sign: 2 (double width) or 1
uint8_t widest(const char *s) { return textWidth(s, 2) <= WIDTH ? 2 : 1; }

// ---------------------------------------------------------------- effects

// Lights (or clears) the pixels of `target` one at a time in random order
void dissolve(const uint8_t *target, bool lightUp) {
  const uint16_t n = WIDTH * 8;  // a power of two, so p*5 + odd visits every pixel once
  uint16_t p = rnd(n), inc = rnd(n / 2) * 2 + 1;
  for (uint16_t i = 1; i <= n; i++) {
    p = (p * 5 + inc) & (n - 1);
    uint8_t x = p >> 3, bit = 1 << (p & 7);
    if (target[x] & bit) {
      if (lightUp) fb[x] |= bit;
      else fb[x] &= ~bit;
    }
    if (i % 16 == 0) FRAME(40);
  }
}

// Letters drop in one after another, with a little bounce
void dropIn(const char *s) {
  const int n = textLen(s), x0 = centered(s);
  for (int t = 0; t < n * 2 + 10; t++) {
    wipe(fb);
    int x = x0, k = 0;
    for (const char *p = s; char c = pgm_read_byte(p); p++, k++) {
      int y = t - k * 2 - 8;
      if (y > 1) y = 0;  // y == 1 is the bounce
      x += drawChar(fb, c, x, y) + 1;
    }
    FRAME(STEP_MS);
  }
}

// Letters fall off the bottom one after another
void dropOut(const char *s) {
  const int n = textLen(s), x0 = centered(s);
  for (int t = 0; t < n * 2 + 9; t++) {
    wipe(fb);
    int x = x0, k = 0;
    for (const char *p = s; char c = pgm_read_byte(p); p++, k++) {
      int y = t - k * 2;
      x += drawChar(fb, c, x, y < 0 ? 0 : y) + 1;
    }
    FRAME(STEP_MS);
  }
}

// Text on row 1 with one letter at a time hopping up a row, like a crowd doing the wave
void drawWave(uint8_t *buf, const char *s, int x, int t, uint8_t wide) {
  const int hop = (t / 2) % (textLen(s) + 3);  // a short rest between waves
  int k = 0;
  for (char c; (c = pgm_read_byte(s)); s++, k++) x += drawChar(buf, c, x, k == hop ? 0 : 1, wide) + wide;
}

// Little autumn leaves drifting down the sign
// Everything on the sign crumbles into bits that flutter down off the bottom
void crumble() {
  for (uint8_t t = 0; t < 50; t++) {
    bool any = false;
    for (int y = 7; y >= 0; y--) {
      for (int x = 0; x < WIDTH; x++) {
        if (!(fb[x] & (1 << y))) continue;
        any = true;
        if (rnd(3)) continue;  // pieces let go at different times
        fb[x] &= ~(1 << y);
        if (y < 7) setPixel(fb, x + (int)rnd(3) - 1, y + 1);
      }
    }
    FRAME(60);
    if (!any) return;
  }
}

// The message scrolls by with the show's little parade marching along behind it
void parade(const char *s, uint8_t marchers) {
  const int w = textWidth(s), gap = 11;
  for (int tx = WIDTH, f = 0; tx + w + 4 + marchers * gap > 0; f++) {
    if (f % 2 == 0) tx--;  // the words move every other frame
    wipe(fb);
    drawText(fb, s, tx);
    for (uint8_t i = 0; i < marchers; i++) showMarcher(i, tx + w + 4 + i * gap, f);
    FRAME(STEP_MS / 2);
  }
}

// A joke from the show's list, in order, each with a little parade behind it. A message from
// the phone takes every other joke's place.
void sceneJoke(const char *const *jokes, uint8_t count, uint8_t *next) {
  static bool phoneTurn;
  bool isNew;
  const char *phone = phoneMessage(&isNew);
  phoneTurn = !phoneTurn;
  if (phone && phoneTurn) {
    parade(phone, 1 + rnd(4));
    return;
  }
  const char *s = (const char *)pgm_read_ptr(&jokes[*next]);
  *next = (*next + 1) % count;
  parade(s, 1 + rnd(4));
}

// team's game is in every time; the others take turns from one visit to the next.
void sceneScores() {
  static uint8_t next;
  const uint8_t count = scoreCount();
  if (!count) return;
  const char *games[3];
  uint8_t n = 0;
  const bool favourite = pgm_read_byte(scoreLine(0)) == '*';
  if (favourite) games[n++] = scoreLine(0) + 1;
  const uint8_t others = count - favourite;
  for (uint8_t k = 0; k < others && n < 3; k++) games[n++] = scoreLine(favourite + (next + k) % others);
  if (others) next = (next + n - favourite) % others;

  int w = 0;
  for (uint8_t i = 0; i < n; i++) w += 12 + textWidth(games[i]) + 8;
  for (int x = WIDTH, f = 0; x + w > 0; f++) {
    if (f % 2 == 0) x--;  // the words move every other frame
    wipe(fb);
    int cx = x;
    for (uint8_t i = 0; i < n; i++) {
      drawSprite(fb, FOOTBALL, cx, (f / 8 + i) % 2);  // each football bounces
      cx += 12;
      drawText(fb, games[i], cx);
      cx += textWidth(games[i]) + 8;
    }
    FRAME(STEP_MS / 2);
  }
}

// ---------------------------------------------------------------- running scenes

void play(void (*scene)()) {
  if (showChanged()) return;  // the day's show changed: the rest of this pass is skipped
  bool isNew;
  const char *phone = phoneMessage(&isNew);
  if (phone && isNew) {  // a new message from the phone goes straight up, with the full parade
    sceneStopped = false;
    parade(phone, 4);
  }
  sceneStopped = false;
  scene();
}

// Every other screen is the football scores, when there are games today
void playThenScores(void (*scene)()) {
  play(scene);
  play(sceneScores);
}
