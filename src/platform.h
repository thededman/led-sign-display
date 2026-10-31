// Lets the show code build both for the boards and for the Mac preview
#pragma once

#ifdef ARDUINO
  #include <Arduino.h>
#else
  #include <stdint.h>
  #include <string.h>
  #define PROGMEM
  #define pgm_read_byte(p) (*(const uint8_t *)(p))
#endif

#ifndef pgm_read_ptr
  #define pgm_read_ptr(p) (*(const void *const *)(p))
#endif
