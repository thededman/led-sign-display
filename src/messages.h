// The words on the sign. Edit freely, then rebuild and upload.
// Use capital letters, digits and  ! ? . , ' - & : / ( )  (lowercase shows as capitals).
#pragma once
#include "platform.h"

// Whose birthday it is. Names show double width when they fit (about five letters and a "!").
// They also appear in MSG_FINALE, MSG_SONG and a few of the jokes below.
static const char NAME1[] PROGMEM = "PAUL!";
static const char NAME2[] PROGMEM = "LARRY!";
static const char MSG_FINALE[] PROGMEM = "HAPPY BIRTHDAY PAUL & LARRY!";
static const char MSG_SONG[] PROGMEM = "HAPPY BIRTHDAY TO YOU... HAPPY BIRTHDAY DEAR PAUL & LARRY!";

static const char MSG_AND[] PROGMEM = "&";
static const char MSG_HAPPY[] PROGMEM = "HAPPY";
static const char MSG_BIRTHDAY[] PROGMEM = "BIRTHDAY";
static const char MSG_WISH[] PROGMEM = "WISH!";
static const char MSG_HOORAY[] PROGMEM = "HOORAY!";
static const char MSG_SURPRISE[] PROGMEM = "SURPRISE!";
static const char MSG_NOM[] PROGMEM = "NOM!";
static const char MSG_BURP[] PROGMEM = "BURP!";
static const char MSG_TOOT[] PROGMEM = "TOOT!";

// Two of these scroll by on each pass of the show, in order, with a little parade behind them.
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
static const char J10[] PROGMEM = "SAME BIRTHDAY, DIFFERENT NUMBER OF CANDLES";
static const char J11[] PROGMEM = "THE CANDLES COST MORE THAN THE CAKE";
static const char J12[] PROGMEM = "CONGRATS ON BEING OLDER THAN YOU'VE EVER BEEN";
static const char J13[] PROGMEM = "OLD? NO WAY - CLASSIC EDITION!";
static const char J14[] PROGMEM = "AGE IMPROVES WINE, CHEESE, PAUL & LARRY";
static const char J15[] PROGMEM = "BIRTHDAY RULE: EAT CAKE FIRST, ASK QUESTIONS LATER";
static const char J16[] PROGMEM = "WARNING: BIRTHDAY BOYS MAY NAP AFTER CAKE";
static const char J17[] PROGMEM = "CAREFUL WITH THE CANDLES - WE DON'T HAVE A FIRE PERMIT";

static const char *const JOKES[] PROGMEM = {
  J0, J1, J2, J3, J4, J5, J6, J7, J8, J9, J10, J11, J12, J13, J14, J15, J16, J17 };
#define JOKE_COUNT (sizeof(JOKES) / sizeof(JOKES[0]))
