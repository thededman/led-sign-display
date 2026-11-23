// The Christmas show: snow, Santa's sleigh and reindeer, a twinkling tree, a snowman who
// shivers, a gingerbread man on the run, and jokes with a parade of tree, present, snowman and
// candy cane.
#include "../config.h"
#if WANT_SHOW(CHRISTMAS)
#include "../effects.h"
#include "christmas_sprites.h"
#include "christmas_words.h"

// ---------------------------------------------------------------- snow

// Snowflakes drifting down the sign, slower than confetti
static struct {
  int8_t x[24], y[24];
} snow;

static void snowStart() {
  for (uint8_t i = 0; i < 24; i++) {
    snow.x[i] = rnd(WIDTH);
    snow.y[i] = -(int8_t)rnd(10);
  }
}

// Moves the first `count` flakes on by one frame and draws them; with `refill`, flakes that
// reach the bottom start again at the top
static void snowStep(int f, uint8_t count, bool refill) {
  for (uint8_t i = 0; i < count; i++) {
    if (f % (2 + i % 3) == 0) snow.y[i]++;
    if (snow.y[i] > 7 && refill) {
      snow.y[i] = -(int8_t)rnd(4);
      snow.x[i] = rnd(WIDTH);
    }
    if (rnd(6) == 0) snow.x[i] += (int)rnd(3) - 1;
    setPixel(fb, snow.x[i], snow.y[i]);
  }
}

// ---------------------------------------------------------------- pictures

// A Christmas tree 13 wide with its left edge at x: a star on top, three tiers and a trunk
static const uint8_t HALF[6] = {1, 2, 3, 2, 4, 6};  // the tree's half-widths, rows 1 to 6

static void drawTree(uint8_t *buf, int x, bool star) {
  const int c = x + 6;
  if (star) {
    setPixel(buf, c, 0);
    setPixel(buf, c - 1, 0);
    setPixel(buf, c + 1, 0);
  }
  for (int r = 0; r < 6; r++)
    for (int dx = -HALF[r]; dx <= HALF[r]; dx++) setPixel(buf, c + dx, r + 1);
  for (int dx = -1; dx <= 1; dx++) setPixel(buf, c + dx, 7);  // trunk
}

// One of the marchers in the Christmas parade, bobbing along
static void drawMarcher(uint8_t which, int x, int f) {
  const int bob = -((f / 8 + which) % 2);  // up a row every other step
  switch (which % 4) {
    case 0: drawSprite(fb, TREE, x, bob); break;
    case 1: drawSprite(fb, PRESENT, x, bob); break;
    case 2: drawSprite(fb, SNOWMAN, x, bob); break;
    default: drawSprite(fb, CANDY_CANE, x, bob); break;
  }
}

// ---------------------------------------------------------------- scenes

// Snow falls, MERRY and CHRISTMAS! drop in, twinkle, and melt away
static void sceneMerryChristmas() {
  snowStart();
  for (int t = 0; t < 50; t++) {
    wipe(fb);
    snowStep(t, 24, t < 25);
    FRAME(60);
  }
  dropIn(MSG_MERRY);
  FRAME(900);
  dropOut(MSG_MERRY);
  dropIn(MSG_CHRISTMAS);
  memcpy(bg, fb, WIDTH);
  snowStart();
  for (int t = 0; t < 45; t++) {
    memcpy(fb, bg, WIDTH);
    snowStep(t, 10, true);
    FRAME(70);
  }
  crumble();
}

// Santa's sleigh flies across behind two reindeer, leaving a trail of sparkles: HO HO HO!
static void sceneSleigh() {
  int8_t sparkX[12], sparkY[12], sparkAge[12];
  for (uint8_t i = 0; i < 12; i++) sparkAge[i] = 99;
  for (int x = -28, f = 0; x < WIDTH + 30; x++, f++) {
    wipe(fb);
    const int bob = (f / 4) % 2;
    drawSprite(fb, REINDEER[(f / 3) % 2], x + 20, bob);
    drawSprite(fb, REINDEER[(f / 3 + 1) % 2], x + 10, 1 - bob);
    for (int rx = x + 8; rx < x + 20; rx += 2) setPixel(fb, rx, 4);  // the reins
    drawSprite(fb, SLEIGH, x, bob);
    if (f % 2 == 0) {  // a new sparkle behind the sleigh
      const uint8_t i = (f / 2) % 12;
      sparkX[i] = x - 1;
      sparkY[i] = 2 + rnd(5);
      sparkAge[i] = 0;
    }
    for (uint8_t i = 0; i < 12; i++) {
      if (sparkAge[i] < 10 && (sparkAge[i] < 4 || rnd(2))) setPixel(fb, sparkX[i], sparkY[i]);
      if (sparkAge[i] < 99) sparkAge[i]++;
    }
    FRAME(STEP_MS);
  }
  const char *const ho[3] = {MSG_HO1, MSG_HO2, MSG_HO3};
  const int x0 = centered(MSG_HO3);
  for (uint8_t k = 0; k < 3; k++) {
    for (uint8_t t = 0; t < 8; t++) {
      wipe(fb);
      drawText(fb, ho[k], x0, t < 2 ? 1 : 0);  // each HO hops in
      FRAME(70);
    }
  }
  FRAME(800);
  memcpy(bg, fb, WIDTH);
  dissolve(bg, false);
}

// A tree fades in, its lights twinkle and its star shines; presents slide in on both sides
static void sceneTree() {
  const int tx = (WIDTH - 13) / 2;
  wipe(bg);
  drawTree(bg, tx, true);
  wipe(fb);
  dissolve(bg, true);
  for (int t = 0; t < 70; t++) {
    wipe(fb);
    drawTree(fb, tx, t % 8 < 5);  // the star shines, blinks, shines
    for (uint8_t i = 0; i < 2; i++) {  // the lights: little dark spots that twinkle
      const int r = 2 + rnd(5), h = HALF[r - 1];
      fb[tx + 6 + (int)rnd(2 * h + 1) - h] &= ~(1 << r);
    }
    const int slide = t < 30 ? 0 : t - 30 > 24 ? 24 : t - 30;  // presents slide in after a while
    drawSprite(fb, PRESENT, tx - 10 - (24 - slide), 0);  // from off the left edge
    drawSprite(fb, PRESENT, tx + 15 + (24 - slide), 0);  // and off the right
    FRAME(80);
  }
  crumble();
}

// A snowman is built ball by ball, puts on his hat, and shivers: BRRR!
static void sceneSnowman() {
  const int sx = 8;
  // drop each part of the snowman from above: body (rows 4-7), head (2-3), hat (0-1)
  static const uint8_t PART_TOP[3] = {4, 2, 0}, PART_END[3] = {7, 3, 1};
  for (uint8_t part = 0; part < 3; part++) {
    for (int y = -8; y <= 0; y++) {
      wipe(fb);
      for (uint8_t p = 0; p <= part; p++) {
        const int dy = p == part ? y : 0;
        for (uint8_t r = PART_TOP[p]; r <= PART_END[p]; r++) {
          const uint8_t bits = pgm_read_byte(SNOWMAN + r);
          for (uint8_t c = 0; c < 8; c++)
            if (bits & (0x80 >> c)) setPixel(fb, sx + c, r + dy);
        }
      }
      FRAME(45);
    }
    FRAME(250);
  }
  // stick arms, then a good long shiver
  const int textX = sx + 16 + (WIDTH - sx - 16 - textWidth(MSG_BRRR)) / 2;
  for (int t = 0; t < 40; t++) {
    const int jiggle = t % 2 ? 1 : -1;
    wipe(fb);
    drawSprite(fb, SNOWMAN, sx + jiggle, 0);
    setPixel(fb, sx - 1 + jiggle, 4);
    setPixel(fb, sx - 2 + jiggle, 3);
    setPixel(fb, sx + 8 + jiggle, 4);
    setPixel(fb, sx + 9 + jiggle, 3);
    drawText(fb, MSG_BRRR, textX + (t < 25 ? jiggle : 0), 0);
    FRAME(70);
  }
  crumble();  // and he melts away
}

#ifndef ARDUINO  // the UNO has no clock
// "3 SLEEPS TILL CHRISTMAS!" in the last month before it
static void sceneSleeps() {
  static char words[40];
  const int16_t days = daysUntil(12, 25);
  if (days < 1 || days > 30) return;
  uint8_t i = 0;
  if (days >= 10) words[i++] = '0' + days / 10;
  words[i++] = '0' + days % 10;
  const char *rest = days == 1 ? " SLEEP TILL CHRISTMAS!" : " SLEEPS TILL CHRISTMAS!";
  while (*rest) words[i++] = *rest++;
  words[i] = 0;
  parade(words, 4);
}
#endif

static void sceneJoke() {
  static uint8_t next;
  ::sceneJoke(JOKES, JOKE_COUNT, &next);
}

// A gingerbread man runs after his own words
static void gingerbread() {
  const int w = textWidth(MSG_RUN);
  for (int tx = WIDTH, f = 0; tx + w + 14 > 0; f++) {
    if (f % 2 == 0) tx--;
    wipe(fb);
    drawText(fb, MSG_RUN, tx);
    drawSprite(fb, GINGERBREAD[(f / 4) % 2], tx + w + 5, (f / 4) % 2 ? -1 : 0);
    FRAME(STEP_MS / 2);
  }
}

// Two bells swing: JINGLE! JANGLE!
static void bells() {
  for (int t = 0; t < 60; t++) {
    const int swing = (t / 5) % 2 ? 1 : -1;
    wipe(fb);
    drawSprite(fb, BELL, 2 + swing, 0);
    drawSprite(fb, BELL, WIDTH - 10 - swing, 0);
    const char *word = (t / 15) % 2 ? MSG_JANGLE : MSG_JINGLE;
    drawText(fb, word, centered(word), t % 15 < 2 ? 1 : 0);
    FRAME(60);
  }
  memcpy(bg, fb, WIDTH);
  dissolve(bg, false);
}

// Heavy snow settles along the bottom: SNOW DAY!
static void snowDay() {
  uint8_t pile[WIDTH];
  memset(pile, 0, WIDTH);
  snowStart();
  for (int t = 0; t < 120; t++) {
    wipe(fb);
    for (uint8_t i = 0; i < 24; i++) {
      if (t % (1 + i % 3) == 0) snow.y[i]++;
      const int x = snow.x[i];
      if (x >= 0 && x < WIDTH && snow.y[i] >= 7 - pile[x]) {  // landed on the pile
        if (pile[x] < 1) pile[x]++;
        snow.y[i] = -(int8_t)rnd(4);
        snow.x[i] = rnd(WIDTH);
      }
      setPixel(fb, snow.x[i], snow.y[i]);
    }
    for (int x = 0; x < WIDTH; x++)
      for (uint8_t h = 0; h < pile[x]; h++) setPixel(fb, x, 7 - h);
    if (t > 70) drawText(fb, MSG_SNOW_DAY, centered(MSG_SNOW_DAY), 0);
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
    case 0: gingerbread(); break;
    case 1: bells(); break;
    default: snowDay(); break;
  }
}

// The sleigh goes by once more, then the big one scrolls through the falling snow
static void sceneFinale() {
  const int w = textWidth(MSG_FINALE);
  snowStart();
  for (int x = WIDTH, f = 0; x > -w; f++) {
    if (f % 2 == 0) x--;
    wipe(fb);
    snowStep(f, 12, true);
    drawText(fb, MSG_FINALE, x);
    FRAME(STEP_MS / 2);
  }
  sceneSleigh();
}

void runChristmas() {
  showMarcher = drawMarcher;
  playThenScores(sceneMerryChristmas);
  playThenScores(sceneSleigh);
#ifndef ARDUINO
  const int16_t days = daysUntil(12, 25);
  if (days >= 1 && days <= 30) playThenScores(sceneSleeps);
#endif
  playThenScores(sceneTree);
  playThenScores(sceneJoke);
  playThenScores(interlude);
  playThenScores(sceneSnowman);
  playThenScores(sceneJoke);
  playThenScores(interlude);
  playThenScores(sceneFinale);
}

#endif
