#include "platform.h"
#include "show.h"
#include "font.h"
#include "sprites.h"
#include "messages.h"

uint8_t fb[WIDTH];
static uint8_t bg[WIDTH];  // a picture to fade in or out
static bool stop;          // the platform asked to skip the rest of this scene

#define STEP_MS 35  // one animation step

// Shows fb, and leaves the current function if the scene is being skipped
#define FRAME(ms) do { if (stop || !frame(ms)) { stop = true; return; } } while (0)

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

static void drawSprite(uint8_t *buf, const uint8_t *rows, int x, int y = 0) {
  for (uint8_t r = 0; r < 8; r++) {
    uint8_t bits = pgm_read_byte(rows + r);
    for (uint8_t c = 0; c < 8; c++)
      if (bits & (0x80 >> c)) setPixel(buf, x + c, y + r);
  }
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

// Everything on the sign bursts into confetti and flutters down off the bottom
static void crumble() {
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

// One of the marchers in the birthday parade, bobbing along
static void drawMarcher(uint8_t which, int x, int f) {
  const int bob = -((f / 8 + which) % 2);  // up a row every other step
  switch (which % 4) {
    case 0: drawSprite(fb, CUPCAKE[(f / 6) % 2], x, bob); break;
    case 1: drawSprite(fb, GIFT_BOX, x, bob); drawSprite(fb, GIFT_LID, x, bob); break;
    case 2: drawSprite(fb, PARTY_HAT, x, bob); break;
    default: drawSprite(fb, BALLOON[(f / 6) % 2], x, bob); break;
  }
}

// The message scrolls by with a little birthday parade marching along behind it
static void parade(const char *s, uint8_t marchers) {
  const int w = textWidth(s), gap = 11;
  for (int tx = WIDTH, f = 0; tx + w + 4 + marchers * gap > 0; f++) {
    if (f % 2 == 0) tx--;  // the words move every other frame
    wipe(fb);
    drawText(fb, s, tx);
    for (uint8_t i = 0; i < marchers; i++) drawMarcher(i, tx + w + 4 + i * gap, f);
    FRAME(STEP_MS / 2);
  }
}

// ---------------------------------------------------------------- scenes

// Confetti, then HAPPY and BIRTHDAY drop in, and BIRTHDAY bursts into confetti
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
  crumble();
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

// The cake: candles flicker, make a wish, blow them out, HOORAY! ...then NOM NOM NOM, BURP!
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
  for (int slice = 1; slice <= 4; slice++) {  // eaten from the right, a slice at a time
    for (uint8_t t = 0; t < 9; t++) {
      wipe(fb);
      if (slice < 4) drawCake(fb, left, false);
      for (int x = left + 16 - slice * 4; x <= left + 16; x++) fb[x] = 0;
      drawText(fb, MSG_NOM, beside + 2 + (slice % 2) * 18, t < 3 ? 1 : 0);  // NOM! pops up
      FRAME(70);
    }
  }
  const int burpX = centered(MSG_BURP, 2);
  for (uint8_t t = 0; t < 20; t++) {
    wipe(fb);
    drawText(fb, MSG_BURP, burpX + (t < 8 ? (int)rnd(3) - 1 : 0), t < 8 ? t % 2 : 0, 2);
    FRAME(70);
  }
  crumble();
}

static void sceneJoke() {
  static uint8_t next;
  static bool phoneTurn;  // a message from the phone takes every other joke's place
  bool isNew;
  const char *phone = phoneMessage(&isNew);
  phoneTurn = !phoneTurn;
  if (phone && phoneTurn) {
    parade(phone, 1 + rnd(4));
    return;
  }
  const char *s = (const char *)pgm_read_ptr(&JOKES[next]);
  next = (next + 1) % JOKE_COUNT;
  parade(s, 1 + rnd(4));
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

// The party blower unrolls with a TOOT! and curls back up, three times
static void horn() {
  const int full = 27, tootX = 4 + full + 6;
  for (uint8_t toot = 0; toot < 3; toot++) {
    for (int len = 0; len <= full; len += 3) {
      wipe(fb);
      drawHorn(len);
      FRAME(30);
    }
    for (uint8_t t = 0; t < 12; t++) {
      wipe(fb);
      drawHorn(full);
      drawText(fb, MSG_TOOT, tootX + (t < 4 ? (int)rnd(3) - 1 : 0), 1);
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

// Sing-along: the song scrolls across a little stage with a bouncing note at each end
static void song() {
  const int w = textWidth(MSG_SONG), left = 10, right = WIDTH - 10;
  for (int tx = right, f = 0; tx + w > left; f++) {
    if (f % 2 == 0) tx--;
    wipe(fb);
    drawText(fb, MSG_SONG, tx);
    for (int x = 0; x < left; x++) fb[x] = 0;
    for (int x = right; x < WIDTH; x++) fb[x] = 0;
    drawSprite(fb, NOTES, 0, (f / 10) % 2);
    drawSprite(fb, NOTES, WIDTH - 8, (f / 10 + 1) % 2);
    FRAME(STEP_MS / 2);
  }
}

static void interlude() {
  static uint8_t last = 255;
  uint8_t pick;
  do pick = rnd(5); while (pick == last);
  last = pick;
  switch (pick) {
    case 0: balloons(); break;
    case 1: fireworks(5); break;
    case 2: gift(); break;
    case 3: horn(); break;
    default: song(); break;
  }
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

// Football scores: a few of today's games scroll by, a football before each. The favourite
// team's game is in every time; the others take turns from one visit to the next.
static void sceneScores() {
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

// ---------------------------------------------------------------- the show

static void play(void (*scene)()) {
  bool isNew;
  const char *phone = phoneMessage(&isNew);
  if (phone && isNew) {  // a new message from the phone goes straight up, with the full parade
    stop = false;
    parade(phone, 4);
  }
  stop = false;
  scene();
}

// Every other screen is the football scores, when there are games today
static void playThenScores(void (*scene)()) {
  play(scene);
  play(sceneScores);
}

void runShow() {
  playThenScores(sceneHappyBirthday);
  playThenScores(sceneNames);
  playThenScores(sceneCake);
  playThenScores(sceneJoke);
  playThenScores(interlude);
  playThenScores(sceneJoke);
  playThenScores(interlude);
  playThenScores(sceneFinale);
}
