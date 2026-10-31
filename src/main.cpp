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

#ifdef FINDER

// Orientation finder: shows the 16 layouts in turn. Use the number on the one that reads
// normally, left to right, as ORIENTATION in config.h.
void setup() {
  Serial.begin(115200);
  matrixBegin(0, brightness);
}

void loop() {
  for (uint8_t layout = 0; layout < 16; layout++) {
    Serial.print("Layout ");
    Serial.println(layout);
    matrixOrientation(layout);
    finderCard(layout);
    frame(4000);
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
