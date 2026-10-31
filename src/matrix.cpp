#include <Arduino.h>
#include "config.h"
#include "matrix.h"

// MAX7219 registers (digits are 1-8)
enum { REG_DIGIT0 = 0x01, REG_DECODE = 0x09, REG_INTENSITY = 0x0A,
       REG_SCANLIMIT = 0x0B, REG_SHUTDOWN = 0x0C, REG_TEST = 0x0F };

// Orientation bits. Module makers wire the LEDs differently and the chain can enter from
// either end, so these four switches (16 layouts) cover every combination.
enum {
  O_CHAIN_RIGHT = 1,  // the first module in the chain is the rightmost, seen from the front
  O_DIGIT_COLS  = 2,  // the chip's digit registers drive columns instead of rows
  O_FLIP_X      = 4,  // mirror each module left-right
  O_FLIP_Y      = 8,  // mirror each module top-bottom
};

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

// The 8 bits of one digit register for the module at chain position `dev`
static uint8_t digitBits(const uint8_t *fb, uint8_t dev, uint8_t digit) {
  uint8_t module = (orient & O_CHAIN_RIGHT) ? MODULES - 1 - dev : dev;  // counted from the left
  uint8_t out = 0;
  for (uint8_t b = 0; b < 8; b++) {
    uint8_t x = (orient & O_DIGIT_COLS) ? digit : b;
    uint8_t y = (orient & O_DIGIT_COLS) ? b : digit;
    if (orient & O_FLIP_X) x = 7 - x;
    if (orient & O_FLIP_Y) y = 7 - y;
    if (fb[module * 8 + x] & (1 << y)) out |= 1 << b;
  }
  return out;
}

void matrixShow(const uint8_t *fb) {
  // A glitch on the 3.3 V lines can knock a module out of its settings; refresh them every second
  if (millis() - configuredAt > 1000) configure();
  for (uint8_t digit = 0; digit < 8; digit++) {
    digitalWrite(pinCs, LOW);
    // The first bytes sent travel furthest down the chain, so start with the last module
    for (int8_t dev = MODULES - 1; dev >= 0; dev--) {
      sendByte(REG_DIGIT0 + digit);
      sendByte(digitBits(fb, dev, digit));
    }
    digitalWrite(pinCs, HIGH);
  }
}
