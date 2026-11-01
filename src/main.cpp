#include <Arduino.h>
#include "config.h"
#include "matrix.h"
#include "show.h"

static uint8_t brightness = BRIGHTNESS;

#ifdef PIN_BUTTON
// Each press of the BOOT button steps to the next brightness level
static void checkButton() {
  static const uint8_t LEVELS[] = {0, 2, 5, 9, 15};
  static bool wasDown;
  static uint32_t changedAt;
  bool down = digitalRead(PIN_BUTTON) == LOW;
  if (down == wasDown || millis() - changedAt < 50) return;
  wasDown = down;
  changedAt = millis();
  if (!down) return;
  uint8_t i = 0;
  while (i < sizeof(LEVELS) && LEVELS[i] <= brightness) i++;
  brightness = i < sizeof(LEVELS) ? LEVELS[i] : LEVELS[0];
  matrixBrightness(brightness);
}
#endif

bool frame(uint16_t ms) {
  uint32_t start = millis();
  matrixShow(fb);
  do {
#ifdef PIN_BUTTON
    checkButton();
#endif
    delay(1);
  } while (millis() - start < ms);
  return true;
}

uint32_t rnd(uint32_t n) { return n ? random(n) : 0; }

const char *phoneMessage(bool *isNew) {  // the UNO has no phone page
  *isNew = false;
  return nullptr;
}

uint8_t scoreCount() { return 0; }  // the UNO has no Wi-Fi for scores
const char *scoreLine(uint8_t) { return nullptr; }

#ifdef FINDER

// Orientation finder: tries the 8 ways a module's LEDs can be wired, 4 seconds each. Every module
// shows an F with a column of 1-8 dots beside it. When the F's look normal, count the dots:
// ORIENTATION in config.h is (dots - 1) * 2, plus 1 if the show's text then comes out with the
// modules in reverse order.
void setup() {
  Serial.begin(115200);
  brightness = 6;  // brighter than the show, so it's easy to read indoors
  matrixBegin(0, brightness);
}

void loop() {
  static const uint8_t F[5] = {0x7F, 0x09, 0x09, 0x09, 0x01};
  for (uint8_t k = 0; k < 8; k++) {
    Serial.print(k + 1);
    Serial.print(" dots = layout ");
    Serial.println(k * 2);
    matrixOrientation(k * 2);
    for (uint8_t m = 0; m < MODULES; m++) {
      for (uint8_t c = 0; c < 8; c++) fb[m * 8 + c] = c < 5 ? F[c] : 0;
      fb[m * 8 + 6] = 0xFF >> (7 - k);  // k + 1 dots
    }
    frame(4000);
  }
}

#elif defined(WIRING_TEST)

// Wiring test: drives the three signal pins in all six orders, 5 seconds each. Each order draws
// its own shape on every module; the order that gives eight identical, clean shapes is right.
static const char *const SHAPES[6] = {
  "square outline", "X", "plus", "centre dot", "two squares", "four corners" };

static uint8_t shapeColumn(uint8_t shape, uint8_t c) {
  static const uint8_t RINGS[8] = {0xFF, 0x81, 0xBD, 0xA5, 0xA5, 0xBD, 0x81, 0xFF};
  bool edge = c == 0 || c == 7, middle = c == 3 || c == 4;
  switch (shape) {
    case 0: return edge ? 0xFF : 0x81;
    case 1: return (1 << c) | (0x80 >> c);
    case 2: return middle ? 0xFF : 0x18;
    case 3: return middle ? 0x18 : 0;
    case 4: return RINGS[c];
    default: return (c < 2 || c > 5) ? 0xC3 : 0;
  }
}

void setup() { Serial.begin(115200); }

void loop() {
  const uint8_t pins[3] = {PIN_DIN, PIN_CLK, PIN_CS};
  static const uint8_t ORDERS[6][3] = {{0, 1, 2}, {1, 0, 2}, {2, 1, 0}, {0, 2, 1}, {1, 2, 0}, {2, 0, 1}};
  for (uint8_t n = 0; n < 6; n++) {
    uint8_t din = pins[ORDERS[n][0]], clk = pins[ORDERS[n][1]], cs = pins[ORDERS[n][2]];
    Serial.print(SHAPES[n]);
    Serial.print(": DIN ");
    Serial.print(din);
    Serial.print(", CLK ");
    Serial.print(clk);
    Serial.print(", CS ");
    Serial.println(cs);
    matrixPins(din, clk, cs);
    matrixBegin(0, brightness);
    for (uint8_t c = 0; c < WIDTH; c++) fb[c] = shapeColumn(n, c % 8);
    for (uint8_t t = 0; t < 10; t++) frame(500);
  }
}

#else

void setup() {
  Serial.begin(115200);
#ifdef PIN_BUTTON
  pinMode(PIN_BUTTON, INPUT_PULLUP);
#endif
#ifndef ESP32
  randomSeed(analogRead(A0));  // the ESP32 has a hardware random number generator
#endif
  matrixBegin(ORIENTATION, brightness);
}

void loop() { runShow(); }

#endif
