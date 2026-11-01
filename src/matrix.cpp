#include <Arduino.h>
#include "config.h"
#include "matrix.h"
#include "matrix_map.h"

static uint8_t pinDin = PIN_DIN, pinClk = PIN_CLK, pinCs = PIN_CS;
static uint8_t orient;
static uint8_t level;
static uint32_t configuredAt;

void matrixPins(uint8_t din, uint8_t clk, uint8_t cs) {
  pinDin = din;
  pinClk = clk;
  pinCs = cs;
}

static void sendByte(uint8_t v) {
  for (int8_t b = 7; b >= 0; b--) {
    digitalWrite(pinClk, LOW);
    digitalWrite(pinDin, (v >> b) & 1);
    if (BIT_DELAY_US) delayMicroseconds(BIT_DELAY_US);
    digitalWrite(pinClk, HIGH);  // the chip reads DIN on this edge
    if (BIT_DELAY_US) delayMicroseconds(BIT_DELAY_US);
  }
}

// Writes the same register on every module
static void sendAll(uint8_t reg, uint8_t value) {
  digitalWrite(pinCs, LOW);
  for (uint8_t m = 0; m < MODULES; m++) {
    sendByte(reg);
    sendByte(value);
  }
  digitalWrite(pinCs, HIGH);
}

static void configure() {
  configuredAt = millis();
  sendAll(REG_TEST, 0);
  sendAll(REG_DECODE, 0);
  sendAll(REG_SCANLIMIT, 7);
  sendAll(REG_INTENSITY, level);
  sendAll(REG_SHUTDOWN, 1);
}

void matrixBegin(uint8_t orientation, uint8_t brightness) {
  pinMode(pinDin, OUTPUT);
  pinMode(pinClk, OUTPUT);
  pinMode(pinCs, OUTPUT);
  digitalWrite(pinCs, HIGH);
  orient = orientation;
  level = brightness;
  configure();
}

void matrixOrientation(uint8_t orientation) { orient = orientation; }

void matrixBrightness(uint8_t l) {
  level = l;
  sendAll(REG_INTENSITY, level);
}

void matrixShow(const uint8_t *fb) {
  // A glitch on the 3.3 V lines can knock a module out of its settings; refresh them every second
  if (millis() - configuredAt > 1000) configure();
  for (uint8_t digit = 0; digit < 8; digit++) {
    digitalWrite(pinCs, LOW);
    // The first bytes sent travel furthest down the chain, so start with the last module
    for (int8_t dev = MODULES - 1; dev >= 0; dev--) {
      sendByte(REG_DIGIT0 + digit);
      sendByte(digitBits(fb, orient, dev, digit));
    }
    digitalWrite(pinCs, HIGH);
  }
}
