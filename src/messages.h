// The words on the sign. Edit freely, then rebuild and upload.
// Use capital letters, digits and  ! ? . , ' - & : /  (lowercase shows as capitals).
#pragma once
#include "platform.h"

static const char MSG_WELCOME[] PROGMEM = "WELCOME!";
static const char MSG_HAPPY[] PROGMEM = "HAPPY";
static const char MSG_HALLOWEEN[] PROGMEM = "HALLOWEEN";
static const char MSG_BOO[] PROGMEM = "BOO!";
static const char MSG_BURP[] PROGMEM = "BURP!";

// Two of these scroll by on each pass of the show, in order.
// A line starting with '@' gets an answer from Pac-Man.
static const char J0[] PROGMEM = "TRICK OR TREAT!";
static const char J1[] PROGMEM = "WITCH PARKING ONLY - ALL OTHERS WILL BE TOAD";
static const char J2[] PROGMEM = "SMELL MY FEET!";
static const char J3[] PROGMEM = "TAKE ONE... OK, TAKE TWO";
static const char J4[] PROGMEM = "@WHO ATE ALL THE CANDY?";
static const char J5[] PROGMEM = "EAT, DRINK & BE SCARY";
static const char J6[] PROGMEM = "NICE COSTUME!";
static const char J7[] PROGMEM = "BEWARE OF THE CANDY GOBLIN";
static const char J8[] PROGMEM = "GHOULS JUST WANNA HAVE FUN";
static const char J9[] PROGMEM = "ZOMBIES WELCOME - BRAINS NOT INCLUDED";

static const char *const JOKES[] PROGMEM = { J0, J1, J2, J3, J4, J5, J6, J7, J8, J9 };
#define JOKE_COUNT (sizeof(JOKES) / sizeof(JOKES[0]))
