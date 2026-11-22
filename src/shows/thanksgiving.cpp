// The Thanksgiving show: falling leaves, a strutting turkey, a pumpkin pie, a field goal, and
// jokes with a parade of turkey, pilgrim hat, leaf and football.
#include "../config.h"
#if WANT_SHOW(THANKSGIVING)
#include "../effects.h"
#include "../sprites.h"
#include "thanksgiving_sprites.h"
#include "thanksgiving_words.h"

static struct {
  int8_t x[24], y[24];
} leaves;

static void leavesStart() {
  for (uint8_t i = 0; i < 24; i++) {
    leaves.x[i] = rnd(WIDTH);
    leaves.y[i] = -(int8_t)rnd(10);
  }
}

// Moves the first `count` leaves on by one frame and draws them, each two pixels that tip as
// it sways; with `refill`, leaves that reach the bottom start again at the top
static void leavesStep(int f, uint8_t count, bool refill) {
  for (uint8_t i = 0; i < count; i++) {
    if (f % (2 + i % 3) == 0) leaves.y[i]++;  // leaves drift down slowly
    if (leaves.y[i] > 7 && refill) {
      leaves.y[i] = -(int8_t)rnd(4);
      leaves.x[i] = rnd(WIDTH);
    }
    const bool tip = (f / 4 + i) % 2;  // swaying side to side
    if (f % 4 == 0) leaves.x[i] += tip ? 1 : -1;
    setPixel(fb, leaves.x[i], leaves.y[i]);
    setPixel(fb, leaves.x[i] + 1, leaves.y[i] + (tip ? 1 : -1));
  }
}

// A pumpkin pie, 16 wide, with its left edge at x: two dollops of whipped cream on top, the
// filling, a fluted crust and the dish
static void drawPie(uint8_t *buf, int x) {
  for (int c = 0; c < 16; c++) {
    setPixel(buf, x + c, 4);                                       // top of the filling
    if (c == 0 || c == 15 || c % 5 == 2) setPixel(buf, x + c, 5);  // sides and spice
    if (c % 2 == 0) setPixel(buf, x + c, 6);                       // fluted crust
  }
  for (int c = 1; c < 15; c++) setPixel(buf, x + c, 7);           // dish
  for (int cx = x + 4; cx <= x + 11; cx += 7) {                   // whipped cream
    setPixel(buf, cx, 2);
    for (int c = cx - 1; c <= cx + 1; c++) setPixel(buf, c, 3);
  }
}

// Goalposts at the right-hand end: uprights, a crossbar on row 5 and the post below
static void drawGoalposts() {
  const int l = WIDTH - 12, r = WIDTH - 4;
  for (int y = 0; y <= 5; y++) {
    setPixel(fb, l, y);
    setPixel(fb, r, y);
  }
  for (int x = l; x <= r; x++) setPixel(fb, x, 5);
  for (int y = 6; y < 8; y++) setPixel(fb, (l + r) / 2, y);
}

// One of the marchers in the Thanksgiving parade
static void drawMarcher(uint8_t which, int x, int f) {
  const int bob = -((f / 8 + which) % 2);  // up a row every other step
  switch (which % 4) {
    case 0: drawSprite(fb, TURKEY[(f / 6) % 2], x); break;  // the turkey walks instead
    case 1: drawSprite(fb, PILGRIM_HAT, x, bob); break;
    case 2: drawSprite(fb, LEAF, x, bob); break;
    default: drawSprite(fb, FOOTBALL, x, bob); break;
  }
}

// ---------------------------------------------------------------- scenes

// Leaves fall, HAPPY / THANKS / GIVING! drop in one after another, and the words crumble away
static void sceneHappyThanksgiving() {
  leavesStart();
  for (int t = 0; t < 50; t++) {
    wipe(fb);
    leavesStep(t, 24, t < 20);
    FRAME(50);
  }
  dropIn(MSG_HAPPY);
  FRAME(800);
  dropOut(MSG_HAPPY);
  dropIn(MSG_THANKS);
  FRAME(600);
  dropOut(MSG_THANKS);
  dropIn(MSG_GIVING);
  memcpy(bg, fb, WIDTH);
  leavesStart();
  for (int t = 0; t < 40; t++) {
    memcpy(fb, bg, WIDTH);
    leavesStep(t, 8, true);
    FRAME(60);
  }
  crumble();
}

// A turkey struts in, gobbles, and struts off
static void sceneTurkey() {
  const int stopAt = WIDTH - 16;
  const int tx = stopAt - 4 - textWidth(MSG_GOBBLE);
  int f = 0;
  for (int x = WIDTH; x >= stopAt; x--, f++) {
    wipe(fb);
    drawSprite(fb, TURKEY[(f / 3) % 2], x);
    FRAME(STEP_MS * 2);
  }
  for (uint8_t t = 0; t < 36; t++) {
    const bool gobbling = (t / 6) % 2 == 0;  // gobbles in bursts
    wipe(fb);
    drawText(fb, MSG_GOBBLE, tx + (gobbling ? (int)rnd(3) - 1 : 0), gobbling ? t % 2 : 0);
    drawSprite(fb, TURKEY[0], stopAt, gobbling ? -(t % 2) : 0);  // head bobbing
    FRAME(70);
  }
  for (int x = stopAt; x > -8; x--, f++) {
    wipe(fb);
    drawText(fb, MSG_GOBBLE, tx);
    drawSprite(fb, TURKEY[(f / 3) % 2], x);
    FRAME(STEP_MS);
  }
  crumble();
}

// A pumpkin pie: PIE TIME!, eaten a slice at a time (NOM!), then a BURP!
static void scenePie() {
  const int middle = (WIDTH - 16) / 2, left = 4;
  wipe(bg);
  drawPie(bg, middle);
  wipe(fb);
  dissolve(bg, true);
  FRAME(1200);
  for (int x = middle; x >= left; x--) {
    wipe(fb);
    drawPie(fb, x);
    FRAME(STEP_MS);
  }
  const int beside = left + 17, room = WIDTH - beside;  // the space to the right of the pie
  const int pieX = beside + (room - textWidth(MSG_PIE)) / 2;
  for (uint8_t t = 0; t < 40; t++) {
    wipe(fb);
    drawPie(fb, left);
    drawWave(fb, MSG_PIE, pieX, t);
    FRAME(70);
  }
  for (int slice = 1; slice <= 4; slice++) {  // eaten from the right, a slice at a time
    for (uint8_t t = 0; t < 9; t++) {
      wipe(fb);
      if (slice < 4) drawPie(fb, left);
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

// Maple leaves see-saw down past the sign
static void leafFall() {
  const uint8_t n = 5;
  int x[n], start[n];
  for (uint8_t i = 0; i < n; i++) {
    x[i] = 2 + i * 12 + rnd(5);
    start[i] = rnd(14);
  }
  for (int f = 0; f < 80; f++) {  // each leaf drops a row every 3 frames
    wipe(fb);
    for (uint8_t i = 0; i < n; i++) {
      const int age = f - start[i], phase = (age / 6) % 4;
      drawSprite(fb, LEAF, x[i] + (phase == 1) - (phase == 3), -8 + age / 3);
    }
    FRAME(50);
  }
}

// RUN, TURKEY! ...and off it goes with a pilgrim hat after it
static void turkeyRun() {
  const int x0 = centered(MSG_RUN);
  for (uint8_t t = 0; t < 30; t++) {
    wipe(fb);
    drawWave(fb, MSG_RUN, x0, t);
    FRAME(60);
  }
  for (int x = WIDTH, f = 0; x > -40; x -= 2, f++) {
    wipe(fb);
    drawSprite(fb, TURKEY[f % 2], x);
    drawSprite(fb, PILGRIM_HAT, x + 22, -((f / 2) % 2));
    FRAME(STEP_MS);
  }
}

// A field goal: the ball sails over the crossbar between the uprights, IT'S GOOD!
static void fieldGoal() {
  const int steps = 30;
  for (int t = 0; t <= steps; t++) {
    const int p = t * 100 / steps;  // how far along the kick, in percent
    wipe(fb);
    drawGoalposts();
    drawSprite(fb, FOOTBALL, -8 + (WIDTH - 4) * t / steps, 3 - p * (125 - p) / 640);
    FRAME(45);
  }
  for (uint8_t t = 0; t < 36; t++) {
    wipe(fb);
    drawGoalposts();
    drawWave(fb, MSG_GOOD, 2, t);
    FRAME(60);
  }
  crumble();
}

// THANKFUL FOR... and a few things to be thankful for
static void thankful() {
  dropIn(MSG_THANKFUL);
  FRAME(600);
  dropOut(MSG_THANKFUL);
  dropIn(MSG_FOR);
  FRAME(400);
  dropOut(MSG_FOR);
  for (uint8_t i = 0; i < THANKFUL_COUNT; i++) {
    const char *s = (const char *)pgm_read_ptr(&THANKFUL_FOR[i]);
    dropIn(s);
    FRAME(700);
    dropOut(s);
  }
}

static void interlude() {
  static uint8_t last = 255;
  uint8_t pick;
  do pick = rnd(4); while (pick == last);
  last = pick;
  switch (pick) {
    case 0: leafFall(); break;
    case 1: turkeyRun(); break;
    case 2: fieldGoal(); break;
    default: thankful(); break;
  }
}

// The big one scrolls by with leaves falling, then the turkey has the last word
static void sceneFinale() {
  const int w = textWidth(MSG_FINALE);
  leavesStart();
  for (int x = WIDTH, f = 0; x > -w; x--, f++) {
    wipe(fb);
    drawText(fb, MSG_FINALE, x);
    leavesStep(f, 6, true);
    FRAME(STEP_MS);
  }
  const int gx = (WIDTH - 12 - textWidth(MSG_GOBBLE)) / 2;
  for (uint8_t t = 0; t < 40; t++) {
    wipe(fb);
    drawWave(fb, MSG_GOBBLE, gx, t);
    drawSprite(fb, TURKEY[0], WIDTH - 10, -((t / 3) % 2));
    FRAME(60);
  }
  crumble();
}

// Football scores: a few of today's games scroll by, a football before each. The favourite
void runThanksgiving() {
  showMarcher = drawMarcher;
  playThenScores(sceneHappyThanksgiving);
  playThenScores(sceneTurkey);
  playThenScores(scenePie);
  playThenScores(sceneJoke);
  playThenScores(interlude);
  playThenScores(sceneJoke);
  playThenScores(interlude);
  playThenScores(sceneFinale);
}

#endif
