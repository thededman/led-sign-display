// Sign settings. Change these, then rebuild and upload.
#pragma once

// Eight 8x8 modules in one row
#define MODULES 8
#define WIDTH (MODULES * 8)

// Wiring: display IN header -> board pins (VCC -> VIN or 5V, GND -> GND)
#if defined(ESP32)
  #define PIN_DIN 23
  #define PIN_CLK 18
  #define PIN_CS 5
  #define PIN_BUTTON 0     // BOOT button: steps through brightness levels
  #define BIT_DELAY_US 4   // slower clock for the 3.3 V signals
#else                      // Arduino UNO
  #define PIN_DIN 11
  #define PIN_CLK 13
  #define PIN_CS 10
  #define BIT_DELAY_US 0
#endif

// Layout number 0-15, worked out with the orientation finder (pio run -e uno-finder -t upload)
#define ORIENTATION 5  // this sign: modules mirrored left-right, cable enters at the right end

// LED brightness 0-15. Low is easier on the power bank and still bright at night.
#define BRIGHTNESS 2

// The shows. The Pi and the Mac preview hold them all and play the one the day calls for; on the
// UNO only one fits, so pick it here.
#define SHOW_THANKSGIVING 1
#define SHOW_BIRTHDAY 2
#ifdef ARDUINO
  #define UNO_SHOW SHOW_THANKSGIVING
  #define WANT_SHOW(s) (UNO_SHOW == SHOW_##s)
#else
  #define WANT_SHOW(s) 1
#endif
