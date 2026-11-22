// Tests for the show table and the today file reader, with a scripted platform.
//   tests/run.sh
#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

#include "effects.h"
#include "show.h"
#include "today.h"

static int frames, switchAt = -1;
static const char *todayShow = "thanksgiving", *switchTo = "";

// A scripted clock: off until setClock(), then moved on by every frame
static bool haveClock;
static uint16_t clockYear;
static uint8_t clockMonth, clockDay;
static uint64_t clockMs;  // since midnight; can run past it until localTime() is asked

static void setClock(uint16_t y, uint8_t mo, uint8_t d, uint8_t h, uint8_t mi, uint8_t sec) {
  haveClock = true;
  clockYear = y;
  clockMonth = mo;
  clockDay = d;
  clockMs = ((h * 60 + mi) * 60 + sec) * 1000ull;
}

static uint8_t monthDays(uint16_t y, uint8_t m) {
  static const uint8_t DAYS[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  const bool leap = (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
  return m == 2 && leap ? 29 : DAYS[m - 1];
}

bool localTime(LocalTime *t) {
  if (!haveClock) return false;
  for (; clockMs >= 86400000ull; clockMs -= 86400000ull) {
    if (++clockDay > monthDays(clockYear, clockMonth)) {
      clockDay = 1;
      if (++clockMonth > 12) {
        clockMonth = 1;
        clockYear++;
      }
    }
  }
  *t = {clockYear, clockMonth, clockDay, (uint32_t)clockMs};
  return true;
}

// What was on the sign at chosen moments: ms after setClock()
static uint64_t elapsed;
static uint64_t shotAt[3];
static uint8_t shots[3][WIDTH];
static bool shotTaken[3];

bool frame(uint16_t ms) {
  frames++;
  for (int i = 0; i < 3; i++)
    if (!shotTaken[i] && shotAt[i] >= elapsed && shotAt[i] < elapsed + ms) {
      std::memcpy(shots[i], fb, WIDTH);
      shotTaken[i] = true;
    }
  elapsed += ms;
  clockMs += ms;
  if (frames == switchAt) {  // the today file changes: the platform ends the scene
    todayShow = switchTo;
    return false;
  }
  assert(frames < 200000);  // a pass that never ends
  return true;
}
uint32_t rnd(uint32_t n) {
  static uint32_t s = 1031;
  s = s * 1103515245u + 12345u;
  return n ? (s >> 8) % n : 0;
}
const char *phoneMessage(bool *isNew) {
  *isNew = false;
  return nullptr;
}
uint8_t scoreCount() { return 0; }
const char *scoreLine(uint8_t) { return nullptr; }
const char *today(const char *key) { return std::strcmp(key, "show") == 0 ? todayShow : ""; }

static std::string readFile(const char *path) {
  std::ifstream in(path);
  std::stringstream all;
  all << in.rdbuf();
  return all.str();
}

int main() {
  // which show a name means
  assert(std::strcmp(resolveShow("thanksgiving")->name, "thanksgiving") == 0);
  assert(std::strcmp(resolveShow("christmas")->name, "thanksgiving") == 0);  // not built yet
  assert(std::strcmp(resolveShow("")->name, "thanksgiving") == 0);

  // a show the sign doesn't have plays the fallback in full, instead of ending every pass
  todayShow = "christmas";
  frames = 0;
  runShow();
  assert(frames > 1000);

  // the birthday show is there too
  assert(std::strcmp(resolveShow("birthday")->name, "birthday") == 0);

  // a change of show mid-pass ends the pass early
  todayShow = "thanksgiving";
  switchAt = -1;
  frames = 0;
  runShow();
  const int fullPass = frames;
  todayShow = "thanksgiving";
  switchAt = 300;
  switchTo = "birthday";
  frames = 0;
  runShow();
  assert(frames < fullPass / 2);
  switchAt = -1;

  // the birthday show plays a full pass with names, and without any
  todayShow = "birthday";
  frames = 0;
  runShow();
  assert(frames > 1000);

  // the date helpers
  setClock(2026, 12, 20, 9, 0, 0);
  assert(daysUntil(12, 25) == 5);
  setClock(2026, 12, 25, 9, 0, 0);
  assert(daysUntil(12, 25) == 0);
  setClock(2026, 12, 26, 9, 0, 0);
  assert(daysUntil(12, 25) == 364);
  setClock(2026, 12, 31, 23, 59, 50);
  assert(msToNewYear() == 10000 && newYearNumber() == 2027);
  setClock(2026, 12, 30, 12, 0, 0);
  assert(msToNewYear() == -1);
  setClock(2027, 1, 1, 0, 0, 1);
  assert(msToNewYear() == -1 && newYearNumber() == 2027);
  setClock(2028, 2, 28, 23, 0, 0);  // a leap year: the 29th comes next
  assert(daysUntil(3, 1) == 2);
  haveClock = false;
  assert(daysUntil(12, 25) == -1 && msToNewYear() == -1 && newYearNumber() == 0);

  // New Year's Eve: the countdown runs on time, inside whatever show is playing
  setClock(2026, 12, 31, 23, 58, 0);
  elapsed = 0;
  const uint64_t moments[3] = {110500, 119500, 120500};  // 23:59:50.5, 23:59:59.5, 00:00:00.5
  for (int i = 0; i < 3; i++) {
    shotAt[i] = moments[i];
    shotTaken[i] = false;
  }
  todayShow = "thanksgiving";
  runShow();
  runShow();
  assert(shotTaken[0] && shotTaken[1] && shotTaken[2]);
  uint8_t ten[WIDTH] = {0}, one[WIDTH] = {0};
  drawText(ten, "10", centered("10", 2), 0, 2);
  drawText(one, "1", centered("1", 2), 0, 2);
  assert(std::memcmp(shots[0], ten, WIDTH) == 0);
  assert(std::memcmp(shots[1], one, WIDTH) == 0);
  assert(std::memcmp(shots[2], one, WIDTH) != 0);  // midnight: not still on 1
  haveClock = false;

  // the today file, as the Python writer writes it
  Today t;
  const std::string sample = readFile("tests/today.sample");
  parseToday(sample.c_str(), &t);
  assert(!std::strcmp(t.show, "birthday") && !std::strcmp(t.why, "auto"));
  assert(!std::strcmp(t.names, "ALEX & ROBIN") && !std::strcmp(t.turning, "ROBIN TURNS 60!"));
  assert(!std::strcmp(todayValue(t, "names"), "ALEX & ROBIN") && !std::strcmp(todayValue(t, "nope"), ""));

  // garbage: lines without '=', an empty key, a key with no value, a 200-letter name
  const std::string junk = "garbage\n=\nshow\nnames=" + std::string(200, 'A') + "\n";
  parseToday(junk.c_str(), &t);
  assert(t.show[0] == 0 && std::strlen(t.names) == sizeof t.names - 1);

  std::puts("show tests: OK");
  return 0;
}
