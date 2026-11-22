// The shows the sign holds, and which one plays: the one the today file names (on the Pi), or
// the one show the UNO was built with.
#include <string.h>
#include "show.h"

void runThanksgiving();

static const ShowEntry SHOWS[] = {
#if WANT_SHOW(THANKSGIVING)
  {"thanksgiving", runThanksgiving},
#endif
};

static const ShowEntry *current;  // the show this pass is playing

const ShowEntry *resolveShow(const char *name) {
  for (const ShowEntry &show : SHOWS)
    if (strcmp(show.name, name) == 0) return &show;
  return &SHOWS[0];  // a show the sign doesn't have yet plays the first one
}

bool showChanged() { return current && resolveShow(today("show")) != current; }

void runShow() {
  current = resolveShow(today("show"));
  current->run();
}
