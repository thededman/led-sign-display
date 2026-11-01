// MAX7219 driver for a Raspberry Pi, on the SPI bus: /dev/spidev0.0 is MOSI (GPIO10, pin 19),
// SCLK (GPIO11, pin 23) and CE0 (GPIO8, pin 24). The chip select stays low for a whole
// transfer, so one transfer writes the same register on every module in the chain.
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "config.h"
#include "matrix.h"
#include "matrix_map.h"

static const uint32_t SPEED_HZ = 1000000;  // slow and steady for the 3.3 V signals

static int spi = -1;
static uint8_t orient;
static uint8_t level;
static std::chrono::steady_clock::time_point configuredAt;

static void transfer(const uint8_t *bytes) {
  struct spi_ioc_transfer t = {};
  t.tx_buf = (unsigned long)bytes;
  t.len = MODULES * 2;
  t.speed_hz = SPEED_HZ;
  t.bits_per_word = 8;
  if (ioctl(spi, SPI_IOC_MESSAGE(1), &t) < 0) perror("SPI transfer");
}

// Writes the same register on every module
static void sendAll(uint8_t reg, uint8_t value) {
  uint8_t bytes[MODULES * 2];
  for (int m = 0; m < MODULES; m++) {
    bytes[m * 2] = reg;
    bytes[m * 2 + 1] = value;
  }
  transfer(bytes);
}

static void configure() {
  configuredAt = std::chrono::steady_clock::now();
  sendAll(REG_TEST, 0);
  sendAll(REG_DECODE, 0);
  sendAll(REG_SCANLIMIT, 7);
  sendAll(REG_INTENSITY, level);
  sendAll(REG_SHUTDOWN, 1);
}

void matrixBegin(uint8_t orientation, uint8_t brightness) {
  spi = open("/dev/spidev0.0", O_RDWR);
  if (spi < 0) {
    perror("/dev/spidev0.0 (turn SPI on with: sudo raspi-config nonint do_spi 0)");
    exit(1);
  }
  uint8_t mode = SPI_MODE_0;
  uint32_t speed = SPEED_HZ;
  ioctl(spi, SPI_IOC_WR_MODE, &mode);
  ioctl(spi, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
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
  if (std::chrono::steady_clock::now() - configuredAt > std::chrono::seconds(1)) configure();
  for (uint8_t digit = 0; digit < 8; digit++) {
    uint8_t bytes[MODULES * 2], *b = bytes;
    // The first bytes sent travel furthest down the chain, so start with the last module
    for (int dev = MODULES - 1; dev >= 0; dev--) {
      *b++ = REG_DIGIT0 + digit;
      *b++ = digitBits(fb, orient, dev, digit);
    }
    transfer(bytes);
  }
}
