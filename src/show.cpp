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

// Text on row 1 with one letter at a time hopping up a row, like a crowd doing the wave
static void drawWave(uint8_t *buf, const char *s, int x, int t, uint8_t wide = 1) {
  const int hop = (t / 2) % (textLen(s) + 3);  // a short rest between waves
  int k = 0;
  for (char c; (c = pgm_read_byte(s)); s++, k++) x += drawChar(buf, c, x, k == hop ? 0 : 1, wide) + wide;
}

// Double width if the text fits that way
static uint8_t widest(const char *s) { return textWidth(s, 2) <= WIDTH ? 2 : 1; }

// A couple of twinkles in the empty columns either side of centred text
static void sparkle(const char *s) {
  const int x0 = centered(s), w = textWidth(s);
  for (uint8_t i = 0; i < 2; i++) {
    int x = rnd(WIDTH - w - 2);
    if (x >= x0 - 1) x += w + 2;  // skip over the text
    setPixel(fb, x, rnd(8));
  }
}

// Confetti fluttering down the sign
static struct {
  int8_t x[24], y[24];
} confetti;

static void confettiStart() {
  for (uint8_t i = 0; i < 24; i++) {
    confetti.x[i] = rnd(WIDTH);
    confetti.y[i] = -(int8_t)rnd(10);
  }
}

// Moves the first `count` pieces on by one frame and draws them; with `refill`, pieces that
// reach the bottom start again at the top
static void confettiStep(int f, uint8_t count, bool refill) {
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

static void fireworks(uint8_t count) {
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

// A two-candle birthday cake, 16 wide, with its left edge at x; candles are at x + 4 and x + 11
static void drawCake(uint8_t *buf, int x, bool lit) {
  for (int c = 0; c < 16; c++) {
    setPixel(buf, x + c, 4);                                       // icing
    if (c % 3 != 2) setPixel(buf, x + c, 5);                       // drips
    if (c == 0 || c == 15 || c % 5 == 2) setPixel(buf, x + c, 6);  // sides and sprinkles
  }
  for (int c = -1; c <= 16; c++) setPixel(buf, x + c, 7);         // plate
  for (int cx = x + 4; cx <= x + 11; cx += 7) {
    setPixel(buf, cx, 2);
    setPixel(buf, cx, 3);
    if (lit) {
      setPixel(buf, cx, 1);
      if (rnd(3) == 0) setPixel(buf, cx, 0);  // the flame flickers taller
    }
  }
}

// ---------------------------------------------------------------- scenes

// Confetti, then HAPPY and BIRTHDAY drop in, and Pac-Man and the ghosts gobble it up
static void sceneHappyBirthday() {
  confettiStart();
  for (int t = 0; t < 45; t++) {
    wipe(fb);
    confettiStep(t, 24, t < 15);
    FRAME(50);
  }
  dropIn(MSG_HAPPY);
  FRAME(1000);
  dropOut(MSG_HAPPY);
  dropIn(MSG_BIRTHDAY);
  memcpy(bg, fb, WIDTH);
  for (uint8_t t = 0; t < 30; t++) {
    memcpy(fb, bg, WIDTH);
    sparkle(MSG_BIRTHDAY);
    FRAME(70);
  }
  chompAcross(false, 4, -8);
}

// Each name does the wave: the first with balloons rising up the sides, the second in confetti
static void sceneNames() {
  uint8_t wide = widest(NAME1);
  int x = centered(NAME1, wide);
  for (int t = 0; t < 60; t++) {
    wipe(fb);
    drawWave(fb, NAME1, x, t, wide);
    drawSprite(fb, BALLOON[(t / 4) % 2], -1, 8 - t / 3);
    drawSprite(fb, BALLOON[(t / 4 + 1) % 2], WIDTH - 7, 12 - t / 3);
    FRAME(60);
  }
  dropIn(MSG_AND);
  FRAME(500);
  wide = widest(NAME2);
  x = centered(NAME2, wide);
  confettiStart();
  for (int t = 0; t < 60; t++) {
    wipe(fb);
    drawWave(fb, NAME2, x, t, wide);
    confettiStep(t, 16, true);
    FRAME(60);
  }
}

// The cake: candles flicker, make a wish, blow them out, HOORAY! ...and Pac-Man eats the cake
static void sceneCake() {
  const int middle = (WIDTH - 16) / 2, left = 4;
  wipe(bg);
  drawCake(bg, middle, true);
  wipe(fb);
  dissolve(bg, true);
  for (uint8_t t = 0; t < 25; t++) {
    wipe(fb);
    drawCake(fb, middle, true);
    FRAME(80);
  }
  for (int x = middle; x >= left; x--) {
    wipe(fb);
    drawCake(fb, x, true);
    FRAME(STEP_MS);
  }
  const int beside = left + 17, room = WIDTH - beside;  // the space to the right of the cake
  const int wishX = beside + (room - textWidth(MSG_WISH)) / 2;
  for (uint8_t t = 0; t < 40; t++) {
    wipe(fb);
    drawCake(fb, left, true);
    drawWave(fb, MSG_WISH, wishX, t);
    FRAME(70);
  }
  for (uint8_t t = 0; t < 14; t++) {  // blown out: a puff of smoke drifts off each candle
    wipe(fb);
    drawCake(fb, left, false);
    drawText(fb, MSG_WISH, wishX, 1);
    for (int k = 0; k < 2; k++) {
      int age = t - k * 2;
      if (age >= 0 && age < 10) setPixel(fb, left + 4 + k * 7 - age / 2, age < 4 ? 1 : 0);
    }
    FRAME(80);
  }
  const int hoorayX = beside + (room - textWidth(MSG_HOORAY)) / 2;
  for (uint8_t t = 0; t < 30; t++) {
    wipe(fb);
    drawCake(fb, left, false);
    drawWave(fb, MSG_HOORAY, hoorayX, t);
    FRAME(60);
  }
  wipe(bg);
  drawCake(bg, left, false);
  drawText(bg, MSG_HOORAY, hoorayX, 1);
  chompAcross(true, 0, WIDTH);
  burp();
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

static void sceneJoke() {
  static uint8_t next;
  const char *s = (const char *)pgm_read_ptr(&JOKES[next]);
  next = (next + 1) % JOKE_COUNT;
  bool answer = pgm_read_byte(s) == '@';
  if (answer) s++;
  scrollChase(s, answer ? 0 : 1 + rnd(4));
  if (answer) burp();
}

// Balloons float up past the sign
static void balloons() {
  const uint8_t n = 5;
  int x[n], start[n];
  for (uint8_t i = 0; i < n; i++) {
    x[i] = 2 + i * 12 + rnd(5);
    start[i] = rnd(12);
  }
  for (int f = 0; f < 75; f++) {  // each balloon rises a row every 3 frames
    wipe(fb);
    for (uint8_t i = 0; i < n; i++)
      drawSprite(fb, BALLOON[((f + i) / 4) % 2], x[i], 8 - (f - start[i]) / 3);
    FRAME(50);
  }
}

// A present shakes, its lid pops off, and out comes SURPRISE!
static void gift() {
  int x = (WIDTH - 8) / 2;
  for (uint8_t t = 0; t < 24; t++) {
    int jiggle = t % 4 == 1 ? 1 : t % 4 == 3 ? -1 : 0;
    wipe(fb);
    drawSprite(fb, GIFT_BOX, x + jiggle);
    drawSprite(fb, GIFT_LID, x + jiggle);
    FRAME(60);
  }
  for (; x > 2; x--) {
    wipe(fb);
    drawSprite(fb, GIFT_BOX, x);
    drawSprite(fb, GIFT_LID, x);
    FRAME(STEP_MS);
  }
  for (int lid = -1; lid >= -3; lid--) {
    wipe(fb);
    drawSprite(fb, GIFT_BOX, x);
    drawSprite(fb, GIFT_LID, x, lid);
    FRAME(80);
  }
  confettiStart();
  for (int t = 0; t < 45; t++) {
    wipe(fb);
    drawSprite(fb, GIFT_BOX, x);
    drawWave(fb, MSG_SURPRISE, x + 10, t);
    confettiStep(t, 10, true);
    FRAME(60);
  }
  memcpy(bg, fb, WIDTH);
  dissolve(bg, false);
}

static void interlude() {
  static uint8_t last = 255;
  uint8_t pick;
  do pick = rnd(4); while (pick == last);
  last = pick;
  if (pick == 0) balloons();
  else if (pick == 1) fireworks(5);
  else if (pick == 2) gift();
  else scenePowerPellet();
}

// Fireworks, then the big one scrolls by, then more fireworks
static void sceneFinale() {
  fireworks(6);
  const int w = textWidth(MSG_FINALE);
  for (int x = WIDTH; x > -w; x--) {
    wipe(fb);
    drawText(fb, MSG_FINALE, x);
    FRAME(STEP_MS);
  }
  fireworks(4);
}

// ---------------------------------------------------------------- the show

static void play(void (*scene)()) {
  stop = false;
  scene();
}

void runShow() {
  play(sceneHappyBirthday);
  play(sceneNames);
  play(sceneCake);
  play(sceneJoke);
  play(interlude);
  play(sceneJoke);
  play(interlude);
  play(sceneFinale);
}
