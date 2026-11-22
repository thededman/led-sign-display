// The Thanksgiving show's words. Edit freely, then rebuild and upload.
// Use capital letters, digits and  ! ? . , ' - & : / ( )  (lowercase shows as capitals).
#pragma once
#include "../platform.h"

static const char MSG_HAPPY[] PROGMEM = "HAPPY";
static const char MSG_THANKS[] PROGMEM = "THANKS";
static const char MSG_GIVING[] PROGMEM = "GIVING!";
static const char MSG_GOBBLE[] PROGMEM = "GOBBLE!";
static const char MSG_PIE[] PROGMEM = "PIE TIME!";
static const char MSG_NOM[] PROGMEM = "NOM!";
static const char MSG_BURP[] PROGMEM = "BURP!";
static const char MSG_RUN[] PROGMEM = "RUN, TURKEY!";
static const char MSG_GOOD[] PROGMEM = "IT'S GOOD!";
static const char MSG_THANKFUL[] PROGMEM = "THANKFUL";
static const char MSG_FOR[] PROGMEM = "FOR...";
static const char MSG_FINALE[] PROGMEM = "HAPPY THANKSGIVING FROM OUR HOUSE TO YOURS!";

// What the sign is thankful for, one at a time after THANKFUL FOR...
static const char T0[] PROGMEM = "FAMILY";
static const char T1[] PROGMEM = "FRIENDS";
static const char T2[] PROGMEM = "FOOD";
static const char T3[] PROGMEM = "FOOTBALL";
static const char T4[] PROGMEM = "PIE!";
static const char T5[] PROGMEM = "NAPS!";
static const char *const THANKFUL_FOR[] PROGMEM = { T0, T1, T2, T3, T4, T5 };
#define THANKFUL_COUNT (sizeof(THANKFUL_FOR) / sizeof(THANKFUL_FOR[0]))

// Two of these scroll by on each pass of the show, in order, with a little parade behind them.
static const char J0[] PROGMEM = "WHY DID THE TURKEY CROSS THE ROAD? TO PROVE HE WASN'T CHICKEN!";
static const char J1[] PROGMEM = "WHAT DO YOU CALL A RUNNING TURKEY? FAST FOOD!";
static const char J2[] PROGMEM = "GOBBLE TILL YOU WOBBLE";
static const char J3[] PROGMEM = "WHAT'S A TURKEY'S FAVORITE DESSERT? PEACH GOBBLER!";
static const char J4[] PROGMEM = "THANKFUL FOR ELASTIC WAISTBANDS";
static const char J5[] PROGMEM = "WHAT SOUND DOES A LIMPING TURKEY MAKE? WOBBLE WOBBLE!";
static const char J6[] PROGMEM = "CALORIES DON'T COUNT ON THANKSGIVING";
static const char J7[] PROGMEM = "WHY CAN'T YOU TAKE A TURKEY TO CHURCH? FOWL LANGUAGE!";
static const char J8[] PROGMEM = "PASS THE GRAVY... AND THE REMOTE";
static const char J9[] PROGMEM = "WHAT DID THE TURKEY SAY TO THE COMPUTER? GOOGLE GOOGLE GOOGLE!";
static const char J10[] PROGMEM = "THE TURKEY WASN'T HUNGRY... HE WAS ALREADY STUFFED";
static const char J11[] PROGMEM = "THANKSGIVING SPORTS: FOOTBALL, EATING AND NAPPING";
static const char J12[] PROGMEM = "WHY DID THE CRANBERRIES TURN RED? THEY SAW THE TURKEY DRESSING!";
static const char J13[] PROGMEM = "LEFTOVERS: THE REAL REASON FOR THE SEASON";
static const char J14[] PROGMEM = "WHAT MUSIC DID THE PILGRIMS LIKE? PLYMOUTH ROCK!";
static const char J15[] PROGMEM = "I CAME FOR THE TURKEY. I STAYED FOR THE PIE.";
static const char J16[] PROGMEM = "KEEP CALM AND GOBBLE ON";
static const char J17[] PROGMEM = "THANKFUL FOR EVERYONE IN THIS ROOM";

static const char *const JOKES[] PROGMEM = {
  J0, J1, J2, J3, J4, J5, J6, J7, J8, J9, J10, J11, J12, J13, J14, J15, J16, J17 };
#define JOKE_COUNT (sizeof(JOKES) / sizeof(JOKES[0]))
