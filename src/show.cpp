#include "platform.h"
#include "show.h"
#include "font.h"
#include "sprites.h"
#include "messages.h"

uint8_t fb[WIDTH];
static uint8_t bg[WIDTH];  // what Pac-Man can eat: words and dots
static bool stop;          // the platform asked to skip the rest of this scene

#define STEP_MS 35  // one animation step

// Shows fb, and leaves the current function if the scene is being skipped
#define FRAME(ms) do { if (stop || !frame(ms)) { stop = true; return; } } while (0)

static const char S200[] PROGMEM = "200";
static const char S400[] PROGMEM = "400";
static const char S800[] PROGMEM = "800";
static const char S1600[] PROGMEM = "1600";
static const char *const SCORES[] PROGMEM = { S200, S400, S800, S1600 };

// ---------------------------------------------------------------- drawing

static void wipe(uint8_t *buf) { memset(buf, 0, WIDTH); }

static void setPixel(uint8_t *buf, int x, int y) {
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
static uint8_t drawChar(uint8_t *buf, char c, int x, int y, uint8_t wide = 1) {
  uint8_t first, w = charSpan(c, &first);
  const uint8_t *g = glyph(c) + first;
  for (uint8_t i = 0; i < w * wide; i++) {
    int cx = x + i;
    if (cx >= 0 && cx < WIDTH) buf[cx] |= shifted(pgm_read_byte(g + i / wide), y);
  }
  return w * wide;
}

static uint8_t textLen(const char *s) {
  uint8_t n = 0;
  while (pgm_read_byte(s + n)) n++;
  return n;
}

static int textWidth(const char *s, uint8_t wide = 1) {
  int w = 0;
  uint8_t first;
  for (char c; (c = pgm_read_byte(s)); s++) w += (charSpan(c, &first) + 1) * wide;
  return w > 0 ? w - wide : 0;
}

// Draws text from flash with its left edge at x
static void drawText(uint8_t *buf, const char *s, int x, int y = 0, uint8_t wide = 1) {
  for (char c; (c = pgm_read_byte(s)); s++) x += drawChar(buf, c, x, y, wide) + wide;
}

static int centered(const char *s, uint8_t wide = 1) { return (WIDTH - textWidth(s, wide)) / 2; }

static void drawSprite(uint8_t *buf, const uint8_t *rows, int x, int y = 0, bool faceLeft = false) {
  for (uint8_t r = 0; r < 8; r++) {
    uint8_t bits = pgm_read_byte(rows + r);
    for (uint8_t c = 0; c < 8; c++)
      if (bits & (faceLeft ? 1 << c : 0x80 >> c)) setPixel(buf, x + c, y + r);
  }
}

// Pac-Man's mouth: open, half, shut, half, open...
static const uint8_t *pacFrame(int step) {
  uint8_t k = (step / 2) % 4;
  return PAC[k == 3 ? 1 : k];
}

// ---------------------------------------------------------------- effects

// Lights (or clears) the pixels of `target` one at a time in random order
static void dissolve(const uint8_t *target, bool lightUp) {
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

// Pac-Man runs across from px eating everything in bg, with ghosts on his tail
static void chompAcross(bool toLeft, uint8_t ghosts, int px) {
  const int dir = toLeft ? -1 : 1;
  const int trail = ghosts ? 12 + (ghosts - 1) * 10 : 0;  // how far back the last ghost is
  for (int s = 0;; s++, px += dir) {
    for (int x = 0; x < WIDTH; x++)
      if (toLeft ? x >= px + 4 : x < px + 4) bg[x] = 0;
    memcpy(fb, bg, WIDTH);
    drawSprite(fb, pacFrame(s), px, 0, toLeft);
    for (uint8_t g = 0; g < ghosts; g++)
      drawSprite(fb, GHOST[(s / 3) % 2], px - dir * (12 + g * 10), 0, toLeft);
    FRAME(STEP_MS);
    int last = px - dir * trail;
    if (toLeft ? last + 8 <= 0 : last >= WIDTH) return;
  }
}

// Letters drop in one after another, with a little bounce
static void dropIn(const char *s) {
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
static void dropOut(const char *s) {
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

// The message scrolls by with Pac-Man and his ghosts after it; then he gobbles the end of it
static void scrollChase(const char *s, uint8_t ghosts) {
  const int w = textWidth(s);
  int tx = WIDTH, px = 0;
  bool hungry = false;
  for (int f = 0;; f++) {
    if (f % 2 == 0) tx--;  // the words move every other frame
    int tail = tx + w;
    if (tail <= WIDTH / 3) hungry = true;  // everyone has read it by now
    px = hungry ? px - 1 : tail + 3;
    wipe(fb);
    drawText(fb, s, tx);
    for (int x = px + 4; x < WIDTH; x++)
      if (x >= 0) fb[x] = 0;  // eaten
    drawSprite(fb, pacFrame(f / 2), px, 0, true);
    for (uint8_t g = 0; g < ghosts; g++)
      drawSprite(fb, GHOST[(f / 6) % 2], px + 12 + g * 10, 0, true);
    FRAME(STEP_MS / 2);
    int last = px + (ghosts ? 12 + (ghosts - 1) * 10 : 0);
    if (last + 8 <= 0) return;
  }
}

// Pac-Man waddles back in, burps, then eats his own burp
static void burp() {
  const int stopAt = WIDTH - 20;
  wipe(fb);
  FRAME(500);
  for (int px = WIDTH; px >= stopAt; px--) {
    wipe(fb);
    drawSprite(fb, pacFrame(px), px, 0, true);
    FRAME(STEP_MS * 2);
  }
  wipe(bg);
  drawText(bg, MSG_BURP, stopAt - 3 - textWidth(MSG_BURP));
  for (int t = 0; t < 16; t++) {
    memcpy(fb, bg, WIDTH);
    drawSprite(fb, PAC[t < 10 ? 0 : 2], stopAt, t < 10 ? t % 2 : 0, true);
    FRAME(t < 10 ? 70 : 120);
  }
  FRAME(600);
  chompAcross(true, 0, stopAt);
}

// Pairs of eyes open, blink and vanish in the dark
static void eyesInTheDark(uint8_t steps) {
  const uint8_t n = 6;
  uint8_t state[n] = {0}, ey[n] = {0};  // state: 0 hidden, 1 open, 2 blinking
  for (uint8_t t = 0; t < steps; t++) {
    wipe(fb);
    for (uint8_t i = 0; i < n; i++) {
      switch (state[i]) {
        case 0: if (rnd(14) == 0) { state[i] = 1; ey[i] = rnd(6); } break;
        case 1: if (rnd(20) == 0) state[i] = 2; else if (rnd(60) == 0) state[i] = 0; break;
        default: state[i] = 1; break;
      }
      if (!state[i]) continue;
      uint8_t col = (state[i] == 1 ? 3 : 2) << ey[i];  // blinking leaves just the lower row
      int x = 3 + i * 10;
      fb[x] |= col; fb[x + 1] |= col;
      fb[x + 4] |= col; fb[x + 5] |= col;
    }
    FRAME(80);
  }
}

// ---------------------------------------------------------------- scenes

static void sceneWelcome() {
  wipe(fb);
  FRAME(600);
  wipe(bg);
  drawText(bg, MSG_WELCOME, centered(MSG_WELCOME));
  dissolve(bg, true);
  FRAME(2500);
  chompAcross(false, 4, -8);
}

// Pac-Man eats a line of dots with the ghosts behind him, then the power pellet turns the tables
static void scenePowerPellet() {
  const int pellet = WIDTH - 4;  // two columns wide
  wipe(bg);
  for (int x = 4; x < pellet - 3; x += 4) bg[x] = 0x18;
  int px = -8, s = 0;
  for (; px + 6 < pellet; px++, s++) {
    for (int x = 0; x < WIDTH && x < px + 4; x++) bg[x] = 0;
    memcpy(fb, bg, WIDTH);
    if ((s / 4) % 2 == 0) fb[pellet] = fb[pellet + 1] = 0x3C;  // the power pellet blinks
    drawSprite(fb, pacFrame(s), px);
    for (uint8_t g = 0; g < 4; g++) drawSprite(fb, GHOST[(s / 3) % 2], px - 11 - g * 9);
    FRAME(STEP_MS);
  }

  // Power pellet eaten: the ghosts turn and run, slowly, and Pac-Man hunts them down
  int gx[4], eyes[4] = {0};  // ghost positions in quarter pixels; eaten ghosts' eyes heading home
  for (uint8_t g = 0; g < 4; g++) gx[g] = (px - 11 - g * 9) * 4;
  uint8_t caught = 0;  // ghosts are caught nearest first
  for (s = 0;; s++) {
    px--;
    for (uint8_t g = 0; g < 4; g++) {
      if (g < caught) eyes[g] += 2;
      else gx[g]--;
    }
    if (caught < 4 && px - gx[caught] / 4 <= 4) {
      const char *score = (const char *)pgm_read_ptr(&SCORES[caught]);
      const int w = textWidth(score);
      int x = gx[caught] / 4 + 4 - w / 2;
      if (x < 0) x = 0;
      wipe(fb);
      for (uint8_t g = caught + 1; g < 4; g++) drawSprite(fb, SCARED[0], gx[g] / 4, 0, true);
      for (uint8_t g = 0; g < caught; g++) drawSprite(fb, EYES, eyes[g]);
      for (int c = x - 1; c <= x + w; c++)
        if (c >= 0 && c < WIDTH) fb[c] = 0;  // a clear window so the score reads
      drawText(fb, score, x);
      FRAME(700);
      eyes[caught] = gx[caught] / 4;
      caught++;
    }
    wipe(fb);
    drawSprite(fb, pacFrame(s), px, 0, true);
    for (uint8_t g = caught; g < 4; g++) drawSprite(fb, SCARED[(s / 3) % 2], gx[g] / 4, 0, true);
    bool eyesOut = false;
    for (uint8_t g = 0; g < caught; g++) {
      drawSprite(fb, EYES, eyes[g]);
      if (eyes[g] < WIDTH) eyesOut = true;
    }
    FRAME(STEP_MS);
    if (px <= -8 && !eyesOut) return;
  }
}

static void sceneHappyHalloween() {
  dropIn(MSG_HAPPY);
  FRAME(1200);
  dropOut(MSG_HAPPY);
  dropIn(MSG_HALLOWEEN);
  FRAME(2500);
  memcpy(bg, fb, WIDTH);
  chompAcross(true, 4, WIDTH);
}

static void sceneJoke() {
  static uint8_t next;
  const char *s = (const char *)pgm_read_ptr(&JOKES[next]);
  next = (next + 1) % JOKE_COUNT;
  bool answer = pgm_read_byte(s) == '@';
  if (answer) s++;
  scrollChase(s, answer ? 0 : 1 + rnd(4));
  if (answer) burp();
}

// A ghost floats in, says BOO! and zooms off
static void ghostBoo() {
  const int stopAt = WIDTH - 14;
  const int tx = stopAt - 4 - textWidth(MSG_BOO, 2);
  int f = 0;
  for (int gx = WIDTH; gx >= stopAt; gx--, f++) {
    wipe(fb);
    drawSprite(fb, GHOST[(f / 3) % 2], gx, (f / 6) % 2, true);
    FRAME(STEP_MS * 2);
  }
  for (int t = 0; t < 30; t++, f++) {
    wipe(fb);
    drawText(fb, MSG_BOO, tx + (t < 12 ? (int)rnd(3) - 1 : 0), 0, 2);
    drawSprite(fb, GHOST[(f / 3) % 2], stopAt, (f / 6) % 2, true);
    FRAME(60);
  }
  for (int gx = stopAt; gx < WIDTH; gx += 2, f++) {
    wipe(fb);
    drawText(fb, MSG_BOO, tx, 0, 2);
    drawSprite(fb, GHOST[(f / 3) % 2], gx);
    FRAME(STEP_MS);
  }
  memcpy(bg, fb, WIDTH);
  dissolve(bg, false);
}

// Bats flap across the sign
static void bats() {
  const uint8_t n = 4;
  int x[n], y[n];
  for (uint8_t i = 0; i < n; i++) {
    x[i] = WIDTH + i * 12 + rnd(8);
    y[i] = rnd(4);
  }
  for (int f = 0;; f++) {
    wipe(fb);
    bool flying = false;
    for (uint8_t i = 0; i < n; i++) {
      if (f % (1 + i % 2) == 0) x[i]--;  // every other bat is slower
      drawSprite(fb, BAT[((f + i) / 3) % 2], x[i], y[i] + ((f + i * 3) / 5) % 2);
      if (x[i] > -8) flying = true;
    }
    FRAME(STEP_MS / 2);
    if (!flying) return;
  }
}

// Three jack-o'-lanterns blink and flicker
static void pumpkins() {
  wipe(bg);
  for (uint8_t i = 0; i < 3; i++) drawSprite(bg, PUMPKIN[0], 7 + i * 21);
  wipe(fb);
  dissolve(bg, true);
  uint8_t shut[3] = {0};
  for (uint8_t t = 0; t < 60; t++) {
    wipe(fb);
    for (uint8_t i = 0; i < 3; i++) {
      if (shut[i]) shut[i]--;
      else if (rnd(18) == 0) shut[i] = 3;
      drawSprite(fb, PUMPKIN[shut[i] ? 1 : 0], 7 + i * 21);
    }
    if (rnd(25) == 0) wipe(fb);  // the candles gutter
    FRAME(60);
  }
  memcpy(fb, bg, WIDTH);
  dissolve(bg, false);
}

static void interlude() {
  static uint8_t last = 255;
  uint8_t pick;
  do pick = rnd(3); while (pick == last);
  last = pick;
  if (pick == 0) ghostBoo();
  else if (pick == 1) bats();
  else pumpkins();
}

// BOO! shakes and glitches, then only eyes in the dark
static void sceneFinale() {
  wipe(fb);
  FRAME(500);
  const int x0 = centered(MSG_BOO, 2);
  for (uint8_t t = 0; t < 36; t++) {
    bool shaking = t < 24;
    wipe(fb);
    drawText(fb, MSG_BOO, x0 + (shaking ? (int)rnd(5) - 2 : 0), shaking && rnd(4) == 0, 2);
    if (shaking && rnd(3) == 0) {  // glitch: one module jumps a row
      uint8_t m = rnd(MODULES), up = rnd(2);
      for (uint8_t c = m * 8; c < m * 8 + 8; c++) fb[c] = up ? fb[c] >> 1 : fb[c] << 1;
    }
    FRAME(shaking ? 60 : 100);
  }
  memcpy(bg, fb, WIDTH);
  dissolve(bg, false);
  eyesInTheDark(60);
}

// ---------------------------------------------------------------- the show

static void play(void (*scene)()) {
  stop = false;
  scene();
}

void runShow() {
  play(sceneWelcome);
  play(scenePowerPellet);
  play(sceneHappyHalloween);
  play(sceneJoke);
  play(interlude);
  play(sceneJoke);
  play(interlude);
  play(sceneFinale);
}

void finderCard(uint8_t layout) {
  static const char READ_ME[] PROGMEM = " READ ME";
  wipe(fb);
  int x = 1;
  if (layout >= 10) x += drawChar(fb, '0' + layout / 10, x, 0) + 1;
  x += drawChar(fb, '0' + layout % 10, x, 0) + 1;
  drawText(fb, READ_ME, x);
}
