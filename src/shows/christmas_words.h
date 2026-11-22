// The Christmas show's words. Edit freely, then rebuild and upload.
// Use capital letters, digits and  ! ? . , ' - & : / ( )  (lowercase shows as capitals).
#pragma once
#include "../platform.h"

static const char MSG_MERRY[] PROGMEM = "MERRY";
static const char MSG_CHRISTMAS[] PROGMEM = "CHRISTMAS!";
static const char MSG_HO1[] PROGMEM = "HO";
static const char MSG_HO2[] PROGMEM = "HO HO";
static const char MSG_HO3[] PROGMEM = "HO HO HO!";
static const char MSG_BRRR[] PROGMEM = "BRRR!";
static const char MSG_JINGLE[] PROGMEM = "JINGLE!";
static const char MSG_JANGLE[] PROGMEM = "JANGLE!";
static const char MSG_RUN[] PROGMEM = "RUN, RUN, AS FAST AS YOU CAN!";
static const char MSG_SNOW_DAY[] PROGMEM = "SNOW DAY!";
static const char MSG_FINALE[] PROGMEM = "MERRY CHRISTMAS FROM OUR HOUSE TO YOURS!";

// Two of these scroll by on each pass of the show, in order, with a little parade behind them.
static const char J0[] PROGMEM = "WHAT DO SNOWMEN EAT FOR BREAKFAST? FROSTED FLAKES!";
static const char J1[] PROGMEM = "WHY IS SANTA SO GOOD AT KARATE? HE HAS A BLACK BELT!";
static const char J2[] PROGMEM = "WHAT DO YOU CALL AN ELF WHO SINGS? A WRAPPER!";
static const char J3[] PROGMEM = "WHAT FALLS ALL WINTER BUT NEVER GETS HURT? SNOW!";
static const char J4[] PROGMEM = "HOW DOES A SNOWMAN GET AROUND? HE RIDES AN ICICLE!";
static const char J5[] PROGMEM = "WHAT DO YOU GET WHEN YOU CROSS A SNOWMAN AND A DOG? FROSTBITE!";
static const char J6[] PROGMEM = "WHERE DOES SANTA KEEP HIS MONEY? IN A SNOW BANK!";
static const char J7[] PROGMEM = "WHY DOES SANTA GO DOWN THE CHIMNEY? IT SOOTS HIM!";
static const char J8[] PROGMEM = "WHAT DO ELVES LEARN IN SCHOOL? THE ELF-ABET!";
static const char J9[] PROGMEM = "WHAT DOES A REINDEER SAY BEFORE A JOKE? THIS ONE WILL SLEIGH YOU!";
static const char J10[] PROGMEM = "WHY IS THE CHRISTMAS TREE BAD AT KNITTING? IT KEEPS DROPPING ITS NEEDLES!";
static const char J11[] PROGMEM = "HOW DO CHRISTMAS TREES GET READY FOR A PARTY? THEY SPRUCE UP!";
static const char J12[] PROGMEM = "WHAT DOES A GINGERBREAD MAN SLEEP ON? A COOKIE SHEET!";
static const char J13[] PROGMEM = "WHAT DO YOU GET WHEN YOU CROSS A BELL AND A SKUNK? JINGLE SMELLS!";
static const char J14[] PROGMEM = "WHAT DO SNOWMEN CALL THEIR KIDS? CHILL-DREN!";
static const char J15[] PROGMEM = "WHAT DOES SANTA SAY WHEN HE GARDENS? HOE, HOE, HOE!";
static const char J16[] PROGMEM = "WHAT DO YOU CALL A CAT ON THE BEACH AT CHRISTMAS? SANDY CLAWS!";
static const char J17[] PROGMEM = "WHAT GOES HO, HO, SWOOSH? SANTA IN A REVOLVING DOOR!";

static const char *const JOKES[] PROGMEM = {
  J0, J1, J2, J3, J4, J5, J6, J7, J8, J9, J10, J11, J12, J13, J14, J15, J16, J17 };
#define JOKE_COUNT (sizeof(JOKES) / sizeof(JOKES[0]))
