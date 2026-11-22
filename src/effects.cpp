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

// Confetti fluttering down the sign
static struct {
  int8_t x[24], y[24];
} confetti;

void confettiStart() {
  for (uint8_t i = 0; i < 24; i++) {
    confetti.x[i] = rnd(WIDTH);
    confetti.y[i] = -(int8_t)rnd(10);
  }
}

// Moves the first `count` pieces on by one frame and draws them; with `refill`, pieces that
// reach the bottom start again at the top
void confettiStep(int f, uint8_t count, bool refill) {
  for (uint8_t i = 0; i < count; i++) {
    if (f % (1 + i % 3) == 0) confetti.y[i]++;
    if (confetti.y[i] > 7 && refill) {
      confetti.y[i] = -(int8_t)rnd(4);
      confetti.x[i] = rnd(WIDTH);
    }
    if (rnd(4) == 0) confetti.x[i] += (int)rnd(3) - 1;
    setPixel(fb, confetti.x[i], confetti.y[i]);
  }
}

// A firework launched `age` frames ago: a rocket climbs to (x, y), bursts, then sparkles out
static void drawFirework(int x, int y, int age) {
  static const int8_t DX[8] = {1, 1, 0, -1, -1, -1, 0, 1};
  static const int8_t DY[8] = {0, 1, 1, 1, 0, -1, -1, -1};
  const int rise = 7 - y;
  if (age < 0) return;
  if (age < rise) {
    setPixel(fb, x, 7 - age);
    setPixel(fb, x, 8 - age);
    return;
  }
  const int r = age - rise + 1;
  if (r > 8) return;
  for (uint8_t d = 0; d < 8; d++) {
    for (int k = r - 1; k <= r && k <= 4; k++) {
      if (k < 1 || (r > 4 && rnd(3))) continue;  // after the burst, only a few sparkles
      int stretch = DY[d] == 0 ? k / 2 : 0;       // a wider burst sideways
      setPixel(fb, x + DX[d] * (k + stretch), y + DY[d] * k);
    }
  }
}

void fireworks(uint8_t count) {
  int8_t x[6], y[6];
  int start[6];
  for (uint8_t i = 0; i < count; i++) {
    x[i] = 6 + rnd(WIDTH - 12);
    y[i] = 2 + rnd(3);
    start[i] = i * 7 + rnd(4);
  }
  const int end = start[count - 1] + 15;
  for (int f = 0; f < end; f++) {
    wipe(fb);
    for (uint8_t i = 0; i < count; i++) drawFirework(x[i], y[i], f - start[i]);
    FRAME(50);
  }
}

// A party blower with its paper tube unrolled `len` pixels
static void drawHorn(int len) {
  for (int x = 0; x < 4; x++) {  // mouthpiece
    setPixel(fb, x, 3);
    setPixel(fb, x, 4);
  }
  for (int x = 4; x < 4 + len; x++) {  // striped paper tube
    setPixel(fb, x, 3);
    if (x % 2) setPixel(fb, x, 4);
  }
  const int c = 4 + len;  // the curl at the end
  setPixel(fb, c, 3); setPixel(fb, c, 4);
  setPixel(fb, c + 1, 2); setPixel(fb, c + 2, 2);
  setPixel(fb, c + 3, 3); setPixel(fb, c + 3, 4);
  setPixel(fb, c + 1, 5); setPixel(fb, c + 2, 5);
}

// The party blower unrolls with its sound (TOOT!) and curls back up, three times
void partyHorn(const char *toot) {
  const int full = 27, tootX = 4 + full + 6;
  for (uint8_t round = 0; round < 3; round++) {
    for (int len = 0; len <= full; len += 3) {
      wipe(fb);
      drawHorn(len);
      FRAME(30);
    }
    for (uint8_t t = 0; t < 12; t++) {
      wipe(fb);
      drawHorn(full);
      drawText(fb, toot, tootX + (t < 4 ? (int)rnd(3) - 1 : 0), 1);
      FRAME(60);
    }
    for (int len = full; len >= 0; len -= 3) {
      wipe(fb);
      drawHorn(len);
      FRAME(30);
    }
    FRAME(200);
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

// ---------------------------------------------------------------- the date

// Days since 1970-01-01 (Howard Hinnant's days_from_civil)
static int32_t dayNumber(int32_t y, uint8_t m, uint8_t d) {
  y -= m <= 2;
  const int32_t era = (y >= 0 ? y : y - 399) / 400;
  const uint32_t yoe = (uint32_t)(y - era * 400);
  const uint32_t doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5 + d - 1;
  const uint32_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + (int32_t)doe - 719468;
}

int16_t daysUntil(uint8_t month, uint8_t day) {
  LocalTime t;
  if (!localTime(&t)) return -1;
  const int32_t today = dayNumber(t.year, t.month, t.day);
  int32_t target = dayNumber(t.year, month, day);
  if (target < today) target = dayNumber(t.year + 1, month, day);
  return target - today;
}

int32_t msToNewYear() {
  LocalTime t;
  if (!localTime(&t) || t.month != 12 || t.day != 31) return -1;
  return 86400000L - (int32_t)t.ms;
}

uint16_t newYearNumber() {
  LocalTime t;
  if (!localTime(&t)) return 0;
  return t.month == 12 ? t.year + 1 : t.year;
}

// ---------------------------------------------------------------- running scenes

#ifndef ARDUINO  // the UNO has no clock, so no countdown

// Writes a number of whole seconds as "1:42", or just "9" under a minute when bare
static void writeTime(char *buf, int32_t secs, bool bare) {
  uint8_t i = 0;
  const int32_t m = secs / 60, s = secs % 60;
  if (bare && m == 0) {
    if (s >= 10) buf[i++] = '0' + s / 10;
    buf[i++] = '0' + s % 10;
  } else {
    if (m >= 10) buf[i++] = '0' + m / 10;
    buf[i++] = '0' + m % 10;
    buf[i++] = ':';
    buf[i++] = '0' + s / 10;
    buf[i++] = '0' + s % 10;
  }
  buf[i] = 0;
}

// New Year's Eve: the time left ticks down, the last ten seconds count down in big digits in
// step with the clock, and at midnight HAPPY NEW YEAR! with fireworks
void newYearCountdown() {
  char buf[8];
  for (int f = 0; msToNewYear() > 10000; f++) {  // 2:30 ... 0:11, with twinkles either side
    writeTime(buf, (msToNewYear() + 999) / 1000, false);
    wipe(fb);
    drawText(fb, buf, centered(buf, 2), 0, 2);
    if (f % 4 == 0) {
      memset(bg, 0, WIDTH);  // new twinkles every few frames
      for (uint8_t i = 0; i < 4; i++) setPixel(bg, i % 2 ? WIDTH - 1 - rnd(6) : rnd(6), rnd(8));
    }
    for (int x = 0; x < WIDTH; x++) fb[x] |= bg[x];
    FRAME(100);
  }
  for (int32_t left; (left = msToNewYear()) > 0;) {  // 10 ... 1, each on its own second
    writeTime(buf, (left + 999) / 1000, true);
    wipe(fb);
    drawText(fb, buf, centered(buf, 2), 0, 2);
    FRAME(50);
  }
  // Midnight
  confettiStart();
  for (int t = 0; t < 30; t++) {
    wipe(fb);
    drawText(fb, "HAPPY", centered("HAPPY", 2), 0, 2);
    confettiStep(t, 24, true);
    FRAME(60);
  }
  fireworks(6);
  dropIn("NEW YEAR!");
  FRAME(1500);
  const uint16_t year = newYearNumber();
  if (year) {
    char digits[6] = {0};
    for (int i = 3, y = year; i >= 0; i--, y /= 10) digits[i] = '0' + y % 10;
    for (int t = 0; t < 40; t++) {
      wipe(fb);
      drawWave(fb, digits, centered(digits, 2), t, 2);
      FRAME(60);
    }
  }
  fireworks(6);
  fireworks(6);
}
#endif

void play(void (*scene)()) {
#ifndef ARDUINO
  const int32_t left = msToNewYear();
  if (left > 0 && left <= 150000) {  // whatever is playing, the countdown starts on time
    sceneStopped = false;
    newYearCountdown();
    return;
  }
#endif
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
