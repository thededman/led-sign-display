// The sign on a Raspberry Pi: runs the show and takes its settings from the phone page.
//   ledsign            run the show
//   ledsign --test     a square on every module, to check the wiring
//   ledsign --finder   the orientation finder (see the README): an F and 1-8 dots per module
// The phone page writes a small key=value file (brightness, paused, skip, message) at
// /var/lib/led-sign/settings, or wherever LEDSIGN_SETTINGS points; scores.py keeps today's
// football games, one per line, at /var/lib/led-sign/scores (or LEDSIGN_SCORES).
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <random>
#include <sstream>
#include <string>
#include <thread>

#include "config.h"
#include "matrix.h"
#include "show.h"

using Clock = std::chrono::steady_clock;
using std::chrono::milliseconds;

static const char *settingsPath = "/var/lib/led-sign/settings";
static const char *scoresPath = "/var/lib/led-sign/scores";
static std::string lastSettings;  // file contents as last read, to spot changes
static Clock::time_point checkedAt;

static int brightness = BRIGHTNESS;
static bool paused;
static long skipCount = -1;  // the page bumps this to skip the current scene
static bool skipping;
static char message[161];
static bool messageNew;

// Lines are rewritten in place, so a game being scrolled never points at freed memory
static char scores[16][48];
static uint8_t scoreLines;

static void readScores() {
  std::ifstream in(scoresPath);
  uint8_t n = 0;
  for (std::string line; n < 16 && std::getline(in, line);)
    if (!line.empty()) std::snprintf(scores[n++], sizeof scores[0], "%s", line.c_str());
  scoreLines = n;
}

static void readSettings() {
  std::ifstream in(settingsPath);
  if (!in) return;
  std::stringstream all;
  all << in.rdbuf();
  if (all.str() == lastSettings) return;
  lastSettings = all.str();

  std::istringstream lines(lastSettings);
  for (std::string line; std::getline(lines, line);) {
    size_t eq = line.find('=');
    if (eq == std::string::npos) continue;
    std::string key = line.substr(0, eq), value = line.substr(eq + 1);
    if (key == "brightness") {
      int b = std::atoi(value.c_str());
      b = b < 0 ? 0 : b > 15 ? 15 : b;
      if (b != brightness) matrixBrightness(brightness = b);
    } else if (key == "paused") {
      paused = value == "1";
    } else if (key == "skip") {
      long n = std::atol(value.c_str());
      if (skipCount >= 0 && n != skipCount) skipping = true;
      skipCount = n;
    } else if (key == "message" && value != message) {
      std::snprintf(message, sizeof message, "%s", value.c_str());
      messageNew = message[0] != 0;
      if (messageNew) skipping = true;  // end the current scene so it shows up straight away
    }
  }
}

static void checkSettings() {
  if (Clock::now() - checkedAt < milliseconds(300)) return;
  checkedAt = Clock::now();
  readSettings();
  readScores();
}

bool frame(uint16_t ms) {
  const Clock::time_point until = Clock::now() + milliseconds(ms);
  matrixShow(fb);
  for (Clock::time_point now; (now = Clock::now()) < until;) {
    checkSettings();
    if (paused) {
      static const uint8_t blank[WIDTH] = {0};
      matrixShow(blank);
      while (paused) {
        std::this_thread::sleep_for(milliseconds(200));
        checkSettings();
      }
      matrixShow(fb);
    }
    std::this_thread::sleep_for(std::min<Clock::duration>(milliseconds(10), until - now));
  }
  if (!skipping) return true;
  skipping = false;
  return false;
}

uint32_t rnd(uint32_t n) {
  static std::mt19937 gen{std::random_device{}()};
  return n ? gen() % n : 0;
}

const char *phoneMessage(bool *isNew) {
  *isNew = messageNew;
  messageNew = false;
  return message[0] ? message : nullptr;
}

uint8_t scoreCount() { return scoreLines; }

const char *scoreLine(uint8_t i) { return i < scoreLines ? scores[i] : nullptr; }

const char *today(const char *) { return ""; }

int main(int argc, char **argv) {
  if (const char *path = std::getenv("LEDSIGN_SETTINGS")) settingsPath = path;
  if (const char *path = std::getenv("LEDSIGN_SCORES")) scoresPath = path;
  matrixBegin(ORIENTATION, brightness);
  readSettings();
  readScores();
  messageNew = false;  // a message left from before shows in turn, not straight away

  if (argc > 1 && std::strcmp(argv[1], "--finder") == 0) {
    static const uint8_t F[5] = {0x7F, 0x09, 0x09, 0x09, 0x01};
    for (;;) {
      for (uint8_t k = 0; k < 8; k++) {
        std::printf("%d dots = ORIENTATION %d (or %d if the cable enters at the right-hand end)\n",
                    k + 1, k * 2, k * 2 + 1);
        std::fflush(stdout);
        matrixOrientation(k * 2);
        for (uint8_t m = 0; m < MODULES; m++) {
          for (uint8_t c = 0; c < 8; c++) fb[m * 8 + c] = c < 5 ? F[c] : 0;
          fb[m * 8 + 6] = 0xFF >> (7 - k);  // k + 1 dots
        }
        matrixShow(fb);
        std::this_thread::sleep_for(milliseconds(4000));
      }
    }
  }
  if (argc > 1 && std::strcmp(argv[1], "--test") == 0) {
    for (int c = 0; c < WIDTH; c++) fb[c] = (c % 8 == 0 || c % 8 == 7) ? 0xFF : 0x81;
    for (;;) {
      matrixShow(fb);
      std::this_thread::sleep_for(milliseconds(1000));
    }
  }
  for (;;) runShow();
}
