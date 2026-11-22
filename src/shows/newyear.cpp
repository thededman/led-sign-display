// The New Year show: confetti, the year rolling over, a champagne cork popping, fireworks, a
// party blower, and resolution jokes with a parade of party hat, glass, clock and star. (The
// countdown to midnight on New Year's Eve is in src/effects.cpp, so it runs whatever's playing.)
#include "../config.h"
#if WANT_SHOW(NEWYEAR)
#include "../effects.h"
#include "newyear_sprites.h"
#include "newyear_words.h"

// One of the marchers in the New Year parade, bobbing along
static void drawMarcher(uint8_t which, int x, int f) {
  const int bob = -((f / 8 + which) % 2);  // up a row every other step
  switch (which % 4) {
    case 0: drawSprite(fb, PARTY_HAT, x, bob); break;
    case 1: drawSprite(fb, GLASS, x, bob); break;
    case 2: drawSprite(fb, CLOCK, x, bob); break;
    default: drawSprite(fb, STAR, x, bob); break;
  }
}

// ---------------------------------------------------------------- scenes

// Confetti, then HAPPY and NEW YEAR! drop in, and it all crumbles into confetti
static void sceneHappyNewYear() {
  confettiStart();
  for (int t = 0; t < 40; t++) {
    wipe(fb);
    confettiStep(t, 24, t < 15);
    FRAME(50);
  }
  dropIn(MSG_HAPPY);
  FRAME(900);
  dropOut(MSG_HAPPY);
  dropIn(MSG_NEW_YEAR);
  memcpy(bg, fb, WIDTH);
  confettiStart();
  for (int t = 0; t < 40; t++) {
    memcpy(fb, bg, WIDTH);
    confettiStep(t, 12, true);
    FRAME(70);
  }
  crumble();
}

#ifndef ARDUINO  // the UNO has no clock, so no year
// A digit drawn double width in the middle of a 12-wide cell
static void drawDigit(char c, int cellX, int y) {
  const char one[2] = {c, 0};
  drawChar(fb, c, cellX + (10 - textWidth(one, 2)) / 2, y, 2);
}

// Last year's number rolls over to this year's, digit by digit, then fireworks
static void sceneYear() {
  const uint16_t year = newYearNumber();
  char before[5], after[5];
  for (int i = 3, a = year, b = year - 1; i >= 0; i--, a /= 10, b /= 10) {
    after[i] = '0' + a % 10;
    before[i] = '0' + b % 10;
  }
  after[4] = before[4] = 0;
  int first = 0;  // the first digit that changes
  while (first < 3 && before[first] == after[first]) first++;
  const int x0 = (WIDTH - 4 * 12) / 2;

  for (int t = 0; t < 20; t++) {  // last year, for a moment
    wipe(fb);
    for (int i = 0; i < 4; i++) drawDigit(before[i], x0 + i * 12, 0);
    FRAME(70);
  }
  for (int k = 0; k <= 8; k++) {  // the changing digits roll up, the new ones come from below
    wipe(fb);
    for (int i = 0; i < 4; i++) {
      if (i < first) {
        drawDigit(after[i], x0 + i * 12, 0);
      } else {
        drawDigit(before[i], x0 + i * 12, -k);
        drawDigit(after[i], x0 + i * 12, 8 - k);
      }
    }
    FRAME(70);
  }
  for (int t = 0; t < 30; t++) {
    wipe(fb);
    drawWave(fb, after, x0 + 1, t, 2);
    FRAME(60);
  }
  fireworks(5);
}
#endif

// A champagne bottle shakes, the cork pops across the sign (POP!), and two glasses clink: CHEERS!
static void sceneChampagne() {
  const int bx = 6;
  for (int t = 0; t < 18; t++) {  // shake it
    wipe(fb);
    drawSprite(fb, BOTTLE, bx + (t % 2 ? 1 : -1), 0);
    FRAME(60);
  }
  for (int t = 0; t < 26; t++) {  // the cork flies off to the right and the foam sprays
    wipe(fb);
    drawSprite(fb, BOTTLE, bx, 0);
    const int cx = bx + 5 + t * 2, cy = t * t / 90;
    setPixel(fb, cx, cy);
    setPixel(fb, cx + 1, cy);
    for (uint8_t i = 0; i < 6 && t < 14; i++) setPixel(fb, bx + 5 + rnd(t + 3), rnd(3));
    if (t > 14) drawText(fb, MSG_POP, WIDTH - 2 - textWidth(MSG_POP), t < 17 ? 1 : 0);
    FRAME(45);
  }
  FRAME(500);
  for (int x = 0; x <= 21; x++) {  // two glasses come together...
    wipe(fb);
    drawSprite(fb, GLASS, x, 0);
    drawSprite(fb, GLASS, WIDTH - 8 - x, 0);
    FRAME(STEP_MS);
  }
  for (int t = 0; t < 10; t++) {  // ...clink!
    wipe(fb);
    drawSprite(fb, GLASS, 21, 0);
    drawSprite(fb, GLASS, WIDTH - 29, 0);
    for (uint8_t i = 0; i < 4; i++) setPixel(fb, 30 + rnd(5), rnd(3));
    FRAME(60);
  }
  for (int t = 0; t < 40; t++) {  // and part to the ends for CHEERS!
    const int x = t < 21 ? 21 - t : 0;
    wipe(fb);
    drawSprite(fb, GLASS, x, 0);
    drawSprite(fb, GLASS, WIDTH - 8 - x, 0);
    if (t >= 12) drawWave(fb, MSG_CHEERS, centered(MSG_CHEERS), t);
    FRAME(60);
  }
  memcpy(bg, fb, WIDTH);
  dissolve(bg, false);
}

static void sceneJoke() {
  static uint8_t next;
  ::sceneJoke(JOKES, JOKE_COUNT, &next);
}

// WOO-HOO! in a burst of confetti
static void wooHoo() {
  confettiStart();
  for (int t = 0; t < 50; t++) {
    wipe(fb);
    drawWave(fb, MSG_WOO, centered(MSG_WOO), t);
    confettiStep(t, 16, true);
    FRAME(60);
  }
  crumble();
}

static void interlude() {
  static uint8_t last = 255;
  uint8_t pick;
  do pick = rnd(3); while (pick == last);
  last = pick;
  switch (pick) {
    case 0: fireworks(6); break;
    case 1: partyHorn(MSG_TOOT); break;
    default: wooHoo(); break;
  }
}

// Fireworks, the big one scrolls by through falling confetti, then more fireworks
static void sceneFinale() {
  fireworks(6);
  const int w = textWidth(MSG_FINALE);
  confettiStart();
  for (int x = WIDTH, f = 0; x > -w; f++) {
    if (f % 2 == 0) x--;
    wipe(fb);
    confettiStep(f, 10, true);
    drawText(fb, MSG_FINALE, x);
    FRAME(STEP_MS / 2);
  }
  fireworks(4);
}

void runNewYear() {
  showMarcher = drawMarcher;
  playThenScores(sceneHappyNewYear);
#ifndef ARDUINO
  if (newYearNumber()) playThenScores(sceneYear);
#endif
  playThenScores(sceneChampagne);
  playThenScores(sceneJoke);
  playThenScores(interlude);
  playThenScores(sceneJoke);
  playThenScores(interlude);
  playThenScores(sceneFinale);
}

#endif
