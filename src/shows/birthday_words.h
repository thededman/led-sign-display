// The birthday show's words. Edit freely, then rebuild and upload. The names come from the
// phone page.
// Use capital letters, digits and  ! ? . , ' - & : / ( )  (lowercase shows as capitals).
#pragma once
#include "../platform.h"

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
static const char J0[] PROGMEM = "MORE CANDLES, MORE WISHES!";
static const char J1[] PROGMEM = "YOU'RE NOT OLD, YOU'RE VINTAGE";
static const char J2[] PROGMEM = "LEGENDS ARE BORN ON DAYS LIKE THIS";
static const char J3[] PROGMEM = "AGE IS JUST A NUMBER... A REALLY BIG ONE";
static const char J4[] PROGMEM = "CAUTION: THIS MANY CANDLES MAY SET OFF THE SMOKE ALARM";
static const char J5[] PROGMEM = "GREAT MINDS THINK ALIKE: CAKE FIRST";
static const char J6[] PROGMEM = "CAKE CALORIES DON'T COUNT TODAY";
static const char J7[] PROGMEM = "ANOTHER YEAR WISER (ALLEGEDLY)";
static const char J8[] PROGMEM = "STILL YOUNG AT HEART (THE KNEES DISAGREE)";
static const char J9[] PROGMEM = "HIP HIP HOORAY!";
static const char J10[] PROGMEM = "WE TRIED TO COUNT THE CANDLES. WE LOST COUNT";
static const char J11[] PROGMEM = "THE CANDLES COST MORE THAN THE CAKE";
static const char J12[] PROGMEM = "CONGRATS ON BEING OLDER THAN YOU'VE EVER BEEN";
static const char J13[] PROGMEM = "OLD? NO WAY - CLASSIC EDITION!";
static const char J14[] PROGMEM = "AGE IMPROVES WINE, CHEESE AND BIRTHDAY STARS";
static const char J15[] PROGMEM = "BIRTHDAY RULE: EAT CAKE FIRST, ASK QUESTIONS LATER";
static const char J16[] PROGMEM = "WARNING: BIRTHDAY STARS MAY NAP AFTER CAKE";
static const char J17[] PROGMEM = "CAREFUL WITH THE CANDLES - WE DON'T HAVE A FIRE PERMIT";

static const char *const JOKES[] PROGMEM = {
  J0, J1, J2, J3, J4, J5, J6, J7, J8, J9, J10, J11, J12, J13, J14, J15, J16, J17 };
#define JOKE_COUNT (sizeof(JOKES) / sizeof(JOKES[0]))
