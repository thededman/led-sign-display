// The words on the sign. Edit freely, then rebuild and upload.
// Use capital letters, digits and  ! ? . , ' - & : / ( )  (lowercase shows as capitals).
#pragma once
#include "platform.h"

// Whose birthday it is. Names show double width when they fit (about five letters and a "!").
// They also appear in MSG_FINALE and a couple of the jokes below.
static const char NAME1[] PROGMEM = "PAUL!";
static const char NAME2[] PROGMEM = "LARRY!";
static const char MSG_FINALE[] PROGMEM = "HAPPY BIRTHDAY PAUL & LARRY!";

static const char MSG_AND[] PROGMEM = "&";
static const char MSG_HAPPY[] PROGMEM = "HAPPY";
static const char MSG_BIRTHDAY[] PROGMEM = "BIRTHDAY";
static const char MSG_WISH[] PROGMEM = "WISH!";
static const char MSG_HOORAY[] PROGMEM = "HOORAY!";
static const char MSG_SURPRISE[] PROGMEM = "SURPRISE!";
static const char MSG_BURP[] PROGMEM = "BURP!";

// Two of these scroll by on each pass of the show, in order.
// A line starting with '@' gets an answer from Pac-Man.
static const char J0[] PROGMEM = "TWO BIRTHDAYS, ONE CAKE? NO WAY - TWO CAKES!";
static const char J1[] PROGMEM = "YOU'RE NOT OLD, YOU'RE VINTAGE";
static const char J2[] PROGMEM = "LEGENDS ARE BORN ON NOVEMBER 1ST";
static const char J3[] PROGMEM = "AGE IS JUST A NUMBER... A REALLY BIG ONE";
static const char J4[] PROGMEM = "CAUTION: THIS MANY CANDLES MAY SET OFF THE SMOKE ALARM";
static const char J5[] PROGMEM = "PAUL & LARRY: GREAT MINDS SHARE A BIRTHDAY";
static const char J6[] PROGMEM = "CAKE CALORIES DON'T COUNT TODAY";
static const char J7[] PROGMEM = "ANOTHER YEAR WISER (ALLEGEDLY)";
static const char J8[] PROGMEM = "STILL YOUNG AT HEART (THE KNEES DISAGREE)";
static const char J9[] PROGMEM = "HIP HIP HOORAY!";

static const char *const JOKES[] PROGMEM = { J0, J1, J2, J3, J4, J5, J6, J7, J8, J9 };
#define JOKE_COUNT (sizeof(JOKES) / sizeof(JOKES[0]))
