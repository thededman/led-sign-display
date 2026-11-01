// The sign on a Raspberry Pi: runs the show and takes its settings from the phone page.
//   ledsign            run the show
//   ledsign --test     a square on every module, to check the wiring
// The phone page writes a small key=value file (brightness, paused, skip, message) at
// /var/lib/led-sign/settings, or wherever LEDSIGN_SETTINGS points.
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
static std::string lastSettings;  // file contents as last read, to spot changes
static Clock::time_point checkedAt;

static int brightness = BRIGHTNESS;
static bool paused;
static long skipCount = -1;  // the page bumps this to skip the current scene
static bool skipping;
static char message[161];
static bool messageNew;

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

int main(int argc, char **argv) {
  if (const char *path = std::getenv("LEDSIGN_SETTINGS")) settingsPath = path;
  matrixBegin(ORIENTATION, brightness);
  readSettings();
  messageNew = false;  // a message left from before shows in turn, not straight away

  if (argc > 1 && std::strcmp(argv[1], "--test") == 0) {
    for (int c = 0; c < WIDTH; c++) fb[c] = (c % 8 == 0 || c % 8 == 7) ? 0xFF : 0x81;
    for (;;) {
      matrixShow(fb);
      std::this_thread::sleep_for(milliseconds(1000));
    }
  }
  for (;;) runShow();
}
