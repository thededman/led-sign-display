// Stand-in for the LED driver, for trying the Pi program on a computer: draws the sign in the
// terminal (or, when the output isn't a terminal, prints every frame one after another).
#include <cstdio>
#include <unistd.h>

#include "config.h"
#include "matrix.h"

static uint8_t level;
static bool tty;

void matrixBegin(uint8_t, uint8_t brightness) {
  level = brightness;
  tty = isatty(STDOUT_FILENO);
  if (tty) std::printf("\033[2J");
}

void matrixOrientation(uint8_t) {}

void matrixBrightness(uint8_t l) { level = l; }

void matrixShow(const uint8_t *fb) {
  std::printf(tty ? "\033[H" : "\n");
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < WIDTH; x++) std::putchar((fb[x] >> y) & 1 ? '#' : '.');
    std::putchar('\n');
  }
  std::printf("brightness %2d\n", level);
  std::fflush(stdout);
}
