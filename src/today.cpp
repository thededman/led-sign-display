#ifndef ARDUINO  // the UNO has no today file
#include <string.h>
#include "today.h"

static void copyValue(char *to, size_t size, const char *from, size_t len) {
  if (len > size - 1) len = size - 1;
  memcpy(to, from, len);
  to[len] = 0;
}

void parseToday(const char *text, Today *t) {
  memset(t, 0, sizeof *t);
  for (const char *line = text; *line;) {
    const char *end = strchr(line, '\n');
    if (!end) end = line + strlen(line);
    const char *stop = end > line && end[-1] == '\r' ? end - 1 : end;
    const char *eq = (const char *)memchr(line, '=', stop - line);
    if (eq) {
      const size_t keyLen = eq - line, valueLen = stop - eq - 1;
      struct { const char *key; char *to; size_t size; } fields[] = {
        {"show", t->show, sizeof t->show}, {"why", t->why, sizeof t->why},
        {"names", t->names, sizeof t->names}, {"turning", t->turning, sizeof t->turning}};
      for (auto &f : fields)
        if (strlen(f.key) == keyLen && memcmp(f.key, line, keyLen) == 0)
          copyValue(f.to, f.size, eq + 1, valueLen);
    }
    line = *end ? end + 1 : end;
  }
}

const char *todayValue(const Today &t, const char *key) {
  if (strcmp(key, "show") == 0) return t.show;
  if (strcmp(key, "why") == 0) return t.why;
  if (strcmp(key, "names") == 0) return t.names;
  if (strcmp(key, "turning") == 0) return t.turning;
  return "";
}
#endif
