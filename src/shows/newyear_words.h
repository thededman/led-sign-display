// The New Year show's words. Edit freely, then rebuild and upload.
// Use capital letters, digits and  ! ? . , ' - & : / ( )  (lowercase shows as capitals).
#pragma once
#include "../platform.h"

static const char MSG_HAPPY[] PROGMEM = "HAPPY";
static const char MSG_NEW_YEAR[] PROGMEM = "NEW YEAR!";
static const char MSG_POP[] PROGMEM = "POP!";
static const char MSG_CHEERS[] PROGMEM = "CHEERS!";
static const char MSG_TOOT[] PROGMEM = "TOOT!";
static const char MSG_WOO[] PROGMEM = "WOO-HOO!";
static const char MSG_FINALE[] PROGMEM = "HAPPY NEW YEAR FROM OUR HOUSE TO YOURS!";

// Two of these scroll by on each pass of the show, in order, with a little parade behind them.
static const char J0[] PROGMEM = "MY NEW YEAR'S RESOLUTION IS 1080P!";
static const char J1[] PROGMEM = "I'M GOING TO STOP PUTTING THINGS OFF... STARTING TOMORROW!";
static const char J2[] PROGMEM = "MY RESOLUTION IS TO READ MORE, SO I TURNED ON THE SUBTITLES!";
static const char J3[] PROGMEM = "NEW YEAR, NEW ME? NO THANKS, THIS ONE IS DOING FINE!";
static const char J4[] PROGMEM = "WHAT DID THE CALENDAR SAY AT MIDNIGHT? MY DAYS ARE NUMBERED!";
static const char J5[] PROGMEM = "WOW, I HAVEN'T SEEN YOU SINCE LAST YEAR!";
static const char J6[] PROGMEM = "I'M GOING TO EAT HEALTHIER... RIGHT AFTER THESE LEFTOVERS!";
static const char J7[] PROGMEM = "MY RESOLUTION: GET IN SHAPE. ROUND IS A SHAPE!";
static const char J8[] PROGMEM = "WHY IS THE CALENDAR SO POPULAR? IT HAS A LOT OF DATES!";
static const char J9[] PROGMEM = "WHAT DOES A CLOCK DO WHEN IT'S HUNGRY? IT GOES BACK FOUR SECONDS!";
static const char J10[] PROGMEM = "HOW DO YOU KNOW THE NEW YEAR IS HERE? THE OLD ONE RAN OUT OF DAYS!";
static const char J11[] PROGMEM = "THIS YEAR I WILL FINISH EVERYTHING I STA...";
static const char J12[] PROGMEM = "WHY DID THE CLOCK GET A NEW YEAR'S KISS? IT WAS RIGHT ON TIME!";
static const char J13[] PROGMEM = "RESOLUTION: LOSE 10 POUNDS. ONLY 15 TO GO!";
static const char J14[] PROGMEM = "CONFETTI: YOU'LL STILL BE FINDING IT IN MARCH!";
static const char J15[] PROGMEM = "AULD LANG WHAT NOW?";
static const char J16[] PROGMEM = "THIS YEAR'S PLAN: LESS SCROLLING, MORE STROLLING!";
static const char J17[] PROGMEM = "I TOLD MY CALENDAR A JOKE. IT SAID MY TIMING WAS PERFECT!";

static const char *const JOKES[] PROGMEM = {
  J0, J1, J2, J3, J4, J5, J6, J7, J8, J9, J10, J11, J12, J13, J14, J15, J16, J17 };
#define JOKE_COUNT (sizeof(JOKES) / sizeof(JOKES[0]))
