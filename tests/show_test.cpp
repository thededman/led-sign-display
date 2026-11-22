// Tests for the show table and the today file reader, with a scripted platform.
//   tests/run.sh
#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

#include "show.h"
#include "today.h"

static int frames, switchAt = -1;
static const char *todayShow = "thanksgiving", *switchTo = "";

bool frame(uint16_t) {
  frames++;
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
