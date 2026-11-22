// The birthday show: confetti, the names from the phone page doing the wave, a cake whose
// candles blow out and then get eaten, balloons, fireworks, a present, a party blower, a
// sing-along, and jokes with a parade of cupcake, present, party hat and balloon.
#include "../config.h"
#if WANT_SHOW(BIRTHDAY)
#ifdef ARDUINO
  #error "The birthday show gets its names from the phone page, so it runs on the Pi only."
#endif
#include <stdio.h>
#include "../effects.h"
#include "birthday_sprites.h"
#include "birthday_words.h"

// A couple of twinkles in the empty columns either side of centred text
static void sparkle(const char *s) {
  const int x0 = centered(s), w = textWidth(s);
  for (uint8_t i = 0; i < 2; i++) {
    int x = rnd(WIDTH - w - 2);
    if (x >= x0 - 1) x += w + 2;  // skip over the text
    setPixel(fb, x, rnd(8));
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

// Up to four of the birthday names from the phone page, one at a time with a "!":
// "ALEX, ROBIN & SAM" -> ALEX!, ROBIN!, SAM!. Returns how many.
static uint8_t splitNames(char out[][16]) {
  const char *s = today("names");
  uint8_t n = 0;
  while (*s && n < 4) {
    size_t len = strcspn(s, ",&");
    while (len && s[len - 1] == ' ') len--;  // the space before " & "
    if (len > 12) len = 12;
    if (len) snprintf(out[n++], 16, "%.*s!", (int)len, s);
    s += strcspn(s, ",&");
    while (*s == ',' || *s == '&' || *s == ' ') s++;
  }
  return n;
}

// Each name does the wave, in turn with balloons rising up the sides or in confetti, with an &
// dropping in between
static void sceneNames() {
  char names[4][16];
  const uint8_t n = splitNames(names);
  for (uint8_t i = 0; i < n; i++) {
    if (i) {
      dropIn(MSG_AND);
      FRAME(500);
    }
    const uint8_t wide = widest(names[i]);
    const int x = centered(names[i], wide);
    if (i % 2) confettiStart();
    for (int t = 0; t < 60; t++) {
      wipe(fb);
      drawWave(fb, names[i], x, t, wide);
      if (i % 2 == 0) {
        drawSprite(fb, BALLOON[(t / 4) % 2], -1, 8 - t / 3);
        drawSprite(fb, BALLOON[(t / 4 + 1) % 2], WIDTH - 7, 12 - t / 3);
      } else {
        confettiStep(t, 16, true);
      }
      FRAME(60);
    }
  }
}

// "ROBIN TURNS 60!", when a birth year was given on the phone page
static void sceneTurning() { parade(today("turning"), 4); }

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
  ::sceneJoke(JOKES, JOKE_COUNT, &next);
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
  static char words[128];
  const char *names = today("names");
  snprintf(words, sizeof words, "HAPPY BIRTHDAY TO YOU... HAPPY BIRTHDAY DEAR %s!", names[0] ? names : "FRIEND");
  const int w = textWidth(words), left = 10, right = WIDTH - 10;
  for (int tx = right, f = 0; tx + w > left; f++) {
    if (f % 2 == 0) tx--;
    wipe(fb);
    drawText(fb, words, tx);
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
  static char words[96];
  const char *names = today("names");
  if (names[0]) snprintf(words, sizeof words, "HAPPY BIRTHDAY %s!", names);
  else snprintf(words, sizeof words, "HAPPY BIRTHDAY!");
  fireworks(6);
  const int w = textWidth(words);
  for (int x = WIDTH; x > -w; x--) {
    wipe(fb);
    drawText(fb, words, x);
    FRAME(STEP_MS);
  }
  fireworks(4);
}

void runBirthday() {
  showMarcher = drawMarcher;
  playThenScores(sceneHappyBirthday);
  if (today("names")[0]) playThenScores(sceneNames);
  if (today("turning")[0]) playThenScores(sceneTurning);
  playThenScores(sceneCake);
  playThenScores(sceneJoke);
  playThenScores(interlude);
  playThenScores(sceneJoke);
  playThenScores(interlude);
  playThenScores(sceneFinale);
}

#endif
