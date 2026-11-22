# Show picker (#10) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** The Pi sign holds several shows (Thanksgiving and Birthday for now), plays the one the calendar, a birthday or a phone pick calls for, and the phone page can pick a show and manage birthdays.

**Architecture:** A new pure-Python module `pi/season.py` holds the calendar and the decision. The scores service (`pi/scores.py`) calls it at least once a minute and writes `/var/lib/led-sign/today` (key=value). The C++ show code splits into shared effects plus one file per show; a small table maps the today file's `show=` to a show, and the Pi program ends the current pass when it changes. The phone page server stores the pick in the settings file and birthdays in their own file.

**Tech Stack:** C++17 (clang++ on the Mac, g++ on the Pi, avr-gcc via PlatformIO for the UNO); Python 3 standard library only (`unittest` for tests); plain HTML/JS phone page.

**Spec:** `docs/superpowers/specs/2026-11-22-seasonal-sign-design.md` (piece 1 of its table). Issue #10, part of #6.

## Global Constraints

- Python on the Pi: standard library only. C++17. No new dependencies anywhere.
- The show code must build three ways: Mac preview (`tools/preview/build.sh`), Pi (`make -C pi` / `make -C pi term`), UNO (`pio run -e uno`, 32 KB flash, 2 KB RAM).
- Show ids are lowercase words: `thanksgiving`, `birthday` (later `christmas`, `newyear`, `everyday`, `gameday`, `valentines`, `stpatricks`, `easter`, `july4`, `halloween`).
- Today file: `/var/lib/led-sign/today` (env `LEDSIGN_TODAY` on the Pi, `PREVIEW_TODAY` in the preview), whole-file atomic writes, `key=value` lines, keys `show`, `why` (`auto`|`picked`), `names`, `turning`.
- Birthdays file: `/var/lib/led-sign/birthdays`, one per line `MM-DD NAME` or `YYYY-MM-DD NAME`. Never in the repo.
- Settings file gains `show=` (`auto` or a built show id). Default `auto`.
- Text on the sign: capitals and the font's characters only (`server.LETTERS`).
- Precedence (spec): phone pick > birthday > holiday's own day > [Game day, piece 3] > holiday window > everyday; a show that isn't built falls back to `everyday` if built, else `thanksgiving`.
- Calendar (spec): Valentine's Feb 1–14; St. Patrick's Mar 1–17; Easter the 14 days ending Easter Sunday; 4th of July Jun 21–Jul 4; Halloween Oct 1–31; Thanksgiving Nov 1–fourth Thursday; Christmas the next day–Dec 25; New Year Dec 26–Jan 1. Overlaps: the holiday whose own day comes first wins. Own days: Feb 14, Mar 17, Easter Sunday, Jul 4, Oct 31, Thanksgiving Day, Dec 25, Jan 1, and Dec 31 from 23:00.
- No real names anywhere in the repo (it's public): tests and samples use ALEX, ROBIN, SAM.
- Every commit ends with `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.

## Review Focus

1. **Today file missing, empty, half-written or garbled:** the sign keeps playing; no crash, no blank. Pinned in Task 5 (`parseToday` garbage test) and Task 6 (missing file keeps the last show).
2. **A show name the sign doesn't have** (e.g. `christmas` before piece 2): it plays the fallback and does not spin ending passes instantly. Pinned in Task 5 (`resolveShow` + no-spin test).
3. **A stale or unknown pick in the settings** (`show=halloween` typed by hand, or a show later removed): treated as Automatic. Pinned in Task 1.
4. **Odd birthday input** (lowercase, emoji, 40-character names, 31 February, duplicate entries, empty name): cleaned or rejected, never written raw. Pinned in Task 3.
5. **Clock not synced after an offline reboot:** the today file is left alone. Pinned in Task 2.

---

### Task 1: The calendar and the day's decision (`pi/season.py`)

**Files:**
- Create: `pi/season.py`
- Test: `tests/test_season.py`

**Interfaces:**
- Produces:
  - `SHOWS: list[tuple[str, str]]` — built shows in phone-page order: `[("thanksgiving", "Thanksgiving"), ("birthday", "Birthday")]`.
  - `easter(year: int) -> datetime.date`; `thanksgiving(year: int) -> datetime.date`
  - `calendar_show(now: datetime.datetime) -> tuple[str, bool]` — the holiday show id or `"everyday"`, and whether it's protected (a holiday's own day, or Dec 31 from 23:00).
  - `@dataclass Birthday(name: str, month: int, day: int, year: int | None = None)`
  - `parse_birthdays(text: str) -> list[Birthday]`; `format_birthdays(items: list[Birthday]) -> str`
  - `birthdays_on(day: datetime.date, items: list[Birthday]) -> list[Birthday]`
  - `join_names(names: list[str]) -> str` — `"ALEX"`, `"ALEX & ROBIN"`, `"ALEX, ROBIN & SAM"`.
  - `available(show: str) -> str` — the fallback rule.
  - `decide(now: datetime.datetime, birthdays: list[Birthday], pick: str) -> dict[str, str]` — keys `show`, `why`, `names`, `turning` (always all four; empty strings when unused).
  - `today_text(decision: dict[str, str]) -> str` — `"show=…\nwhy=…\nnames=…\nturning=…\n"` in that key order.

- [ ] **Step 1: Write the failing tests** in `tests/test_season.py` (`sys.path.insert(0, <repo>/pi)`, then `import season`):

```python
D = datetime.date; T = lambda *a: datetime.datetime(*a)
def test_easter(self):          # known dates
    self.assertEqual([season.easter(y) for y in (2026, 2027, 2028, 2038)],
                     [D(2026,4,5), D(2027,3,28), D(2028,4,16), D(2038,4,25)])
def test_thanksgiving(self):
    self.assertEqual([season.thanksgiving(y) for y in (2026, 2027, 2029)],
                     [D(2026,11,26), D(2027,11,25), D(2029,11,22)])
def test_calendar(self):
    cases = {T(2026,11,22,12): ("thanksgiving", False), T(2026,11,26,9): ("thanksgiving", True),
             T(2026,11,27,0): ("christmas", False), T(2026,12,25,8): ("christmas", True),
             T(2026,12,26,8): ("newyear", False), T(2026,12,31,22,59): ("newyear", False),
             T(2026,12,31,23,0): ("newyear", True), T(2027,1,1,10): ("newyear", True),
             T(2027,1,2,10): ("everyday", False), T(2027,2,14,10): ("valentines", True),
             T(2027,3,15,10): ("stpatricks", False),  # Easter 2027 window starts Mar 15
             T(2027,3,17,10): ("stpatricks", True), T(2027,3,18,10): ("easter", False),
             T(2027,3,28,10): ("easter", True), T(2027,6,20,10): ("everyday", False),
             T(2027,6,21,10): ("july4", False), T(2027,10,1,0): ("halloween", False)}
    for now, want in cases.items():
        self.assertEqual(season.calendar_show(now), want, now)
def test_birthdays_parse_and_leap(self):
    items = season.parse_birthdays("11-01 ALEX\n1966-11-01 ROBIN\n02-29 SAM\nnonsense\n13-40 BAD\n")
    self.assertEqual([b.name for b in items], ["ALEX", "ROBIN", "SAM"])
    self.assertEqual(items[1].year, 1966)
    self.assertEqual([b.name for b in season.birthdays_on(D(2027,2,28), items)], ["SAM"])
    self.assertEqual([b.name for b in season.birthdays_on(D(2028,2,28), items)], [])
    self.assertEqual(season.parse_birthdays(season.format_birthdays(items)), items)
def test_join_names(self):
    self.assertEqual([season.join_names(n) for n in (["ALEX"], ["ALEX","ROBIN"], ["ALEX","ROBIN","SAM"], [])],
                     ["ALEX", "ALEX & ROBIN", "ALEX, ROBIN & SAM", ""])
def test_decide_precedence(self):
    b = season.parse_birthdays("11-27 ALEX\n1966-11-27 ROBIN\n")
    d = season.decide(T(2026,11,27,9), b, "auto")
    self.assertEqual(d, {"show": "birthday", "why": "auto", "names": "ALEX & ROBIN", "turning": "ROBIN TURNS 60!"})
    self.assertEqual(season.decide(T(2026,11,27,9), b, "thanksgiving")["show"], "thanksgiving")  # pick wins
    self.assertEqual(season.decide(T(2026,11,27,9), b, "thanksgiving")["why"], "picked")
def test_decide_fallback_and_bad_pick(self):
    d = season.decide(T(2026,12,10,9), [], "auto")       # christmas isn't built yet
    self.assertEqual((d["show"], d["why"]), ("thanksgiving", "auto"))
    d = season.decide(T(2026,11,22,9), [], "halloween")  # stale/unknown pick = Automatic
    self.assertEqual((d["show"], d["why"]), ("thanksgiving", "auto"))
def test_decide_picked_birthday_uses_next_names(self):
    b = season.parse_birthdays("03-02 SAM\n")
    d = season.decide(T(2026,11,22,9), b, "birthday")
    self.assertEqual((d["show"], d["why"], d["names"]), ("birthday", "picked", "SAM"))
    self.assertEqual(season.decide(T(2026,11,22,9), [], "birthday")["names"], "")
def test_midnight_rollover(self):
    b = season.parse_birthdays("11-23 ALEX\n")
    self.assertEqual(season.decide(T(2026,11,22,23,59), b, "auto")["show"], "thanksgiving")
    self.assertEqual(season.decide(T(2026,11,23,0,0), b, "auto")["show"], "birthday")
def test_today_text(self):
    self.assertEqual(season.today_text({"show": "birthday", "why": "auto", "names": "ALEX & ROBIN", "turning": ""}),
                     "show=birthday\nwhy=auto\nnames=ALEX & ROBIN\nturning=\n")
```

- [ ] **Step 2: Run to verify they fail**

Run: `python3 -m unittest discover -s tests -p 'test_season.py' -v`
Expected: errors, `ModuleNotFoundError: No module named 'season'`.

- [ ] **Step 3: Implement `pi/season.py`** with the interfaces above. Easter: the Anonymous Gregorian algorithm. Windows per year as `(show, first_day, own_day)`; `calendar_show` checks the windows of `now.year - 1` and `now.year` (New Year crosses the year), keeps those containing the date, and picks the one whose `own_day` is soonest; protected when the date is that own day, or Dec 31 at or after 23:00. `decide`: pick if it's in `SHOWS` ids → `why=picked`; else today's birthdays → `birthday`; else `calendar_show`; then `available()`. `names` = `join_names` of today's birthdays (for a picked birthday with none today: the next upcoming birthday's names, people sharing that date together). `turning` = `" ".join(f"{name} TURNS {age}!")` for those with a year. A 29 February birthday counts on 28 February in non-leap years.

- [ ] **Step 4: Run to verify they pass** — same command; expected `OK`.

- [ ] **Step 5: Commit** `git add pi/season.py tests/test_season.py && git commit -m "Season calendar and the day's show decision (#10)"`

---

### Task 2: The scores service writes the today file

**Files:**
- Modify: `pi/scores.py` (`main()` loop; new `update_today`)
- Test: `tests/test_scores_today.py`

**Interfaces:**
- Consumes: `season.decide`, `season.parse_birthdays`, `season.today_text`; existing `scores.write(path, lines)` and `read_settings(path)`.
- Produces: `update_today(today_path: str, settings_path: str, birthdays_path: str, now: datetime.datetime, synced: bool) -> bool` (True when it wrote a changed file); `clock_synced() -> bool`; new CLI options `--today` (default `/var/lib/led-sign/today`) and `--birthdays` (default `/var/lib/led-sign/birthdays`).

- [ ] **Step 1: Write the failing tests** (temp directory, `sys.path` to `pi/`, `import scores`):

```python
def test_writes_decision(self):
    write settings "show=auto\n", birthdays "11-22 ALEX\n"
    self.assertTrue(scores.update_today(today, settings, bdays, T(2026,11,22,9), synced=True))
    self.assertEqual(read(today), "show=birthday\nwhy=auto\nnames=ALEX\nturning=\n")
    self.assertFalse(scores.update_today(today, settings, bdays, T(2026,11,22,9,1), synced=True))  # unchanged
def test_unsynced_clock_leaves_file(self):
    write today "show=thanksgiving\nwhy=auto\nnames=\nturning=\n"
    self.assertFalse(scores.update_today(today, settings, bdays, T(2031,1,1), synced=False))
    self.assertEqual(read(today), "show=thanksgiving\nwhy=auto\nnames=\nturning=\n")
def test_missing_files(self):  # no settings, no birthdays yet
    self.assertTrue(scores.update_today(today, missing, missing, T(2026,11,22,9), synced=True))
    self.assertIn("show=thanksgiving\n", read(today))
```

- [ ] **Step 2: Run, expect failure** — `python3 -m unittest discover -s tests -p 'test_scores_today.py' -v` → `AttributeError: module 'scores' has no attribute 'update_today'`.

- [ ] **Step 3: Implement.** `clock_synced()`: `timedatectl show -p NTPSynchronized --value` == `yes`; `FileNotFoundError` (a Mac) → True. Restructure `main()` so it wakes every second: recompute the today file when the wall-clock minute changes or the settings or birthdays file changes; fetch scores on the existing schedule (`wait` seconds, cut short by a settings change as now). `--once` also prints the today decision.

- [ ] **Step 4: Run, expect `OK`;** also `python3 pi/scores.py --once --settings /dev/null --birthdays /dev/null` prints today's games and `show=thanksgiving`.

- [ ] **Step 5: Commit** `"Scores service writes the day's show to the today file (#10)"`

---

### Task 3: Phone page server: show pick and birthdays API

**Files:**
- Modify: `pi/web/server.py`
- Test: `tests/test_server.py`

**Interfaces:**
- Consumes: `season.SHOWS`, `season.parse_birthdays`, `season.format_birthdays`, `season.Birthday` (import via `sys.path.insert(0, str(Path(__file__).resolve().parent.parent))`, which is `pi/` in the repo and `/opt/led-sign/` installed).
- Produces:
  - `DEFAULTS` gains `"show": "auto"`; `load` reads it as text.
  - `clean_pick(value) -> str` — `"auto"` or a `SHOWS` id; anything else → `"auto"`.
  - `clean_birthdays(items: list) -> list[season.Birthday]` — each `{"name", "month", "day", "year"?}`; name via `clean()` cut to 12; drops entries with an empty name, an impossible date (Feb 29 allowed), a year outside 1900..this year; drops exact duplicates; keeps at most 50; sorted by month, day, name.
  - `GET /api/settings` adds `"today": {"show", "why"}` (from the today file; `{}` if missing) and `"shows": [{"id", "name"}]`.
  - `POST /api/settings` accepts `{"show": …}`.
  - `GET /api/birthdays` → `[{"name", "month", "day", "year"}]`; `POST /api/birthdays` with `{"birthdays": [...]}` replaces the whole list and returns the cleaned list.
  - CLI options `--today`, `--birthdays` with the `/var/lib/led-sign/` defaults.

- [ ] **Step 1: Write the failing tests:**

```python
def test_clean_pick(self):
    self.assertEqual([server.clean_pick(v) for v in ("birthday", "auto", "halloween", "", None, 3)],
                     ["birthday", "auto", "auto", "auto", "auto", "auto"])
def test_clean_birthdays(self):
    raw = [{"name": "alex", "month": 11, "day": 1}, {"name": "Robin 🎂", "month": 2, "day": 29, "year": 1966},
           {"name": "x" * 40, "month": 3, "day": 2}, {"name": "", "month": 1, "day": 1},
           {"name": "SAM", "month": 2, "day": 31}, {"name": "SAM", "month": 4, "day": 1, "year": 1800},
           {"name": "alex", "month": 11, "day": 1}, "junk"]
    got = server.clean_birthdays(raw)
    self.assertEqual([(b.name, b.month, b.day, b.year) for b in got],
                     [("ROBIN", 2, 29, 1966), ("XXXXXXXXXXXX", 3, 2, None), ("ALEX", 11, 1, None)])
def test_settings_keep_show(self):  # save then load round-trips show=
    server.save(path, dict(server.DEFAULTS, show="birthday"))
    self.assertEqual(server.load(path)["show"], "birthday")
```

- [ ] **Step 2: Run, expect failure** (`AttributeError: clean_pick`).
- [ ] **Step 3: Implement** the interfaces; the birthdays file is written atomically like settings.
- [ ] **Step 4: Run, expect `OK`.** Then `python3 pi/web/server.py --port 8033 --settings $T/settings --scores $T/scores --today $T/today --birthdays $T/birthdays` and `curl -s localhost:8033/api/settings` shows `"shows"`; a `curl -X POST -d '{"birthdays":[{"name":"alex","month":11,"day":27}]}' localhost:8033/api/birthdays` returns `[{"name": "ALEX", …}]` and writes `11-27 ALEX`.
- [ ] **Step 5: Commit** `"Phone page server: pick a show, keep birthdays (#10)"`

---

### Task 4: Phone page: Show and Birthdays sections

**Files:**
- Modify: `pi/web/index.html`
- Modify: `.claude/launch.json` (the `phone-page` entry gains `--today` and `--birthdays` under `tools/preview/build/`)

**Interfaces:**
- Consumes: Task 3's API.

- [ ] **Step 1: Add a "Show" section** (first, above "Say something"): a status line, "Automatic: Thanksgiving" or "Picked: Birthday" (show names from `settings.shows`, mode from `settings.today.why`, falling back to the settings' `show` when the today file is missing); a `<select id="show">` with "Automatic" and every show; on change `send({show})`, toast "Show picked" or "Back to automatic".
- [ ] **Step 2: Add a "Birthdays" section** (after "Say something"): the list (`NAME — Month D`, plus "(1966)" when there's a year), each with a Remove button; an add row: name (`maxlength=12`, `autocapitalize=characters`), month select, day select (1–31), optional year (`inputmode=numeric`), Add button. Every change POSTs the whole list to `/api/birthdays` and re-renders from the reply. Hint text: "Birthdays play all day on the date. They're kept on the sign, not online."
- [ ] **Step 3: Check it in the browser** with the `phone-page` preview server: pick Birthday → the status says "Picked: Birthday" once a today file says `why=picked` (write one by hand, or run `scores.py` against the same files); add ALEX on today's date, remove it, add a 29 Feb birthday; reload keeps them; dark mode looks right; no console errors.
- [ ] **Step 4: Commit** `"Phone page: pick a show, add birthdays (#10)"`

---

### Task 5: Split the show code; the show table; the today hook

**Files:**
- Create: `src/effects.h`, `src/effects.cpp` (moved out of `src/show.cpp`: `fb`, `bg`, skip flag, `STEP_MS`, `FRAME`, drawing, text, sprites, `dissolve`, `dropIn`, `dropOut`, `drawWave`, `widest`, `crumble`, `parade`, `sceneScores`, `play`, `playThenScores`)
- Create: `src/shows/thanksgiving.cpp`, `src/shows/thanksgiving_words.h` (from `src/messages.h`), `src/shows/thanksgiving_sprites.h` (TURKEY, PILGRIM_HAT, LEAF from `src/sprites.h`)
- Create: `src/today.h`, `src/today.cpp`
- Modify: `src/show.cpp` (now only the show table and `runShow`), `src/show.h`, `src/sprites.h` (keeps only FOOTBALL, the scores ball), `src/config.h`, `src/main.cpp`, `tools/preview/preview.cpp`, `tools/preview/build.sh`, `pi/Makefile`
- Delete: `src/messages.h`
- Test: `tests/show_test.cpp`, `tests/today.sample`, `tests/run.sh`

**Interfaces:**
- Produces (in `src/show.h`, supplied by each platform): `const char *today(const char *key);` — the today value or `""`, never null; pointers stay valid (contents rewritten in place).
- Produces (`src/effects.h`): `extern uint8_t bg[WIDTH]; extern bool sceneStopped;` the `FRAME(ms)` macro (now using `sceneStopped`); `void parade(const char *s, uint8_t marchers, void (*marcher)(uint8_t which, int x, int f));` `void sceneJoke(const char *const *jokes, uint8_t count, uint8_t *next, void (*marcher)(uint8_t, int, int));` (the phone message takes every other joke's turn, as now); `void play(void (*scene)());` `void playThenScores(void (*scene)());` — `play` does nothing when the show has changed (see below); the other helpers keep their current signatures, minus `static`.
- Produces (`src/show.cpp`): `struct ShowEntry { const char *name; void (*run)(); };` `const ShowEntry *resolveShow(const char *name);` (unknown or empty → the first entry, Thanksgiving); `bool showChanged();` (true when `resolveShow(today("show"))` is not the show `runShow` started); `void runShow();` (resolves, remembers, runs one pass).
- Produces (`src/config.h`): `#define SHOW_THANKSGIVING 1`, `#define SHOW_BIRTHDAY 2`; on `ARDUINO`, `#define UNO_SHOW SHOW_THANKSGIVING` and `#define WANT_SHOW(s) (UNO_SHOW == SHOW_##s)`; otherwise `#define WANT_SHOW(s) 1`. Each show file is wrapped in `#if WANT_SHOW(NAME)`; the table lists only wanted shows.
- Produces (`src/today.h`): `struct Today { char show[24]; char why[8]; char names[64]; char turning[96]; };` `void parseToday(const char *text, Today *t);` (fills from `key=value` lines; unknown keys and lines without `=` ignored; values cut to fit; fields missing from the text are emptied) and `const char *todayValue(const Today &t, const char *key);` (`""` for unknown keys). `today.cpp` is wrapped in `#ifndef ARDUINO`.
- Show entry points: `void runThanksgiving();` (and Task 7's `void runBirthday();`) declared in `src/show.cpp`.

- [ ] **Step 1: Record the before-picture.** `tools/preview/build.sh && shasum tools/preview/build/preview.html > $SCRATCH/before.sha` (seed 1031 makes it deterministic; the refactor must not change a single frame).
- [ ] **Step 2: Write the failing test** `tests/show_test.cpp` (its own `frame`, `rnd`, `phoneMessage`, `scoreCount`, `scoreLine`, `today` hooks; a scripted `todayShow` string; plain `assert`s):

```cpp
// resolve
assert(strcmp(resolveShow("thanksgiving")->name, "thanksgiving") == 0);
assert(strcmp(resolveShow("christmas")->name, "thanksgiving") == 0);   // not built yet: fallback
assert(strcmp(resolveShow("")->name, "thanksgiving") == 0);
// an unknown show must not make passes end instantly
todayShow = "christmas"; frames = 0; runShow(); assert(frames > 1000);
// the today file, as the Python writer writes it (tests/today.sample)
Today t; parseToday(readFile("tests/today.sample"), &t);
assert(!strcmp(t.show, "birthday") && !strcmp(t.why, "auto") && !strcmp(t.names, "ALEX & ROBIN") && !strcmp(t.turning, "ROBIN TURNS 60!"));
// garbage: lines without '=', an empty key, a key with no value, a 200-letter name
std::string junk = "garbage\n=\nshow\nnames=" + std::string(200, 'A') + "\n";
parseToday(junk.c_str(), &t);
assert(t.show[0] == 0 && strlen(t.names) == 63);   // no crash; show emptied, names cut to fit
```

  `tests/today.sample` holds exactly `show=birthday\nwhy=auto\nnames=ALEX & ROBIN\nturning=ROBIN TURNS 60!\n`, and `tests/test_season.py` gains `test_sample_matches_writer`: `season.today_text({...those values...}) == open("tests/today.sample").read()`. The mid-pass switch test needs a second real show, so it's in Task 7.

  `tests/run.sh`: `set -e`; builds `tests/show_test.cpp` with `clang++ -std=c++17 -Wall -Wextra -I src` plus every show source, runs it from the repo root, then `python3 -m unittest discover -s tests -v`.
- [ ] **Step 3: Run, expect failure** — `tests/run.sh` fails to compile (`resolveShow` undeclared).
- [ ] **Step 4: Do the split** as listed under Files. Shared code loses `static`; Thanksgiving's scenes stay `static` in `src/shows/thanksgiving.cpp` and include `"../effects.h"`, `"thanksgiving_words.h"`, `"thanksgiving_sprites.h"`; its `drawMarcher` becomes the marcher passed to `parade`/`sceneJoke`; `runThanksgiving()` is today's `runShow()` body. `play` checks `showChanged()` first. Hooks: `src/main.cpp` and the preview get `today()`: the UNO returns `""`; the preview reads `PREVIEW_TODAY` (a file) once at start with `parseToday`. Build lists: `tools/preview/build.sh` and `pi/Makefile` compile `src/effects.cpp src/show.cpp src/today.cpp src/shows/*.cpp`; `pi/Makefile`'s `DEPS` include `../src/shows/*.h`.
- [ ] **Step 5: Run `tests/run.sh`, expect all pass; then `tools/preview/build.sh && shasum -c $SCRATCH/before.sha`** → `OK` (identical show). `make -C pi term` builds with no warnings.
- [ ] **Step 6: Commit** `"Split the show code: shared effects, one file per show, the show table (#10)"`

---

### Task 6: The Pi program follows the today file

**Files:**
- Modify: `pi/main.cpp`

**Interfaces:**
- Consumes: `parseToday`, `todayValue`, `Today` (Task 5).
- Produces: `today(key)` for the Pi; env `LEDSIGN_TODAY` (default `/var/lib/led-sign/today`).

- [ ] **Step 1: Implement:** `readToday()` beside `readSettings()`, called from `checkSettings()`: read the whole file; if it can't be read or is empty, keep the current values; if the text differs from last time, parse into a scratch `Today`, copy each field into the live one in place, and when `show` changed set `skipping = true` (the current scene ends; `play` then skips the rest of the pass; `runShow` starts the new show). Read once at start before the first `runShow()`.
- [ ] **Step 2: Check it in the terminal build:** `printf 'show=thanksgiving\n' > $T/today; LEDSIGN_TODAY=$T/today LEDSIGN_SETTINGS=$T/settings pi/ledsign-term` shows Thanksgiving; in another shell `printf 'show=birthday\nnames=ALEX\n' > $T/today` → within a second the scene ends and (after Task 7) the birthday show starts; `rm $T/today` → keeps playing; `printf 'garbage' > $T/today` → keeps playing.
- [ ] **Step 3: Commit** `"Pi: play the show the today file names, switch when it changes (#10)"`

---

### Task 7: The Birthday show, with names from the phone page

**Files:**
- Create: `src/shows/birthday.cpp`, `src/shows/birthday_words.h`, `src/shows/birthday_sprites.h` (from `v1.1.0`: `git show v1.1.0:src/show.cpp`, `…:src/messages.h`, `…:src/sprites.h`)
- Modify: `src/show.cpp` (table entry), `tests/show_test.cpp`
- Create: `tools/preview/samples/birthday.today` (`show=birthday\nnames=ALEX & ROBIN\nturning=ROBIN TURNS 60!\n`)

**Interfaces:**
- Consumes: `effects.h`; `today("names")`, `today("turning")`.
- Produces: `void runBirthday();` table entry `{"birthday", runBirthday}` under `WANT_SHOW(BIRTHDAY)`.

- [ ] **Step 1: Add the failing tests** to `tests/show_test.cpp`, then run `tests/run.sh` → fails (`birthday` resolves to Thanksgiving):

```cpp
assert(strcmp(resolveShow("birthday")->name, "birthday") == 0);
// a change mid-pass ends the pass early
todayShow = "thanksgiving"; switchAt = 0; frames = 0; runShow(); const int fullPass = frames;
todayShow = "thanksgiving"; switchAt = 300; switchTo = "birthday"; frames = 0; runShow();
assert(frames < fullPass / 2);   // the test's frame() hook swaps todayShow at frame 300 and returns false once
```
- [ ] **Step 2: Port the show.** Scenes from v1.1.0 keep their look. Names come from `today("names")`: `sceneNames` waves each name in turn (split on `", "` and `" & "`, each with `!`, double width when it fits), up to 4; the song and finale are composed in RAM buffers: `"HAPPY BIRTHDAY TO YOU... HAPPY BIRTHDAY DEAR " + names + "!"` and `"HAPPY BIRTHDAY " + names + "!"`. With no names: skip `sceneNames`, the song says `DEAR FRIEND!`, the finale `HAPPY BIRTHDAY!`. When `today("turning")` isn't empty, one extra scene parades it once per pass. Rewrite the jokes that assume two people, a date or real names (v1.1.0's J0, J2, J5, J10, J14, J16) so they work for one person or several and name no one; keep 18 jokes. On the UNO, `#error` if `UNO_SHOW` is `SHOW_BIRTHDAY` ("The birthday show gets its names from the phone page, so it runs on the Pi only."). No real names anywhere.
- [ ] **Step 3: Run `tests/run.sh`** → all pass.
- [ ] **Step 4: Check it in the preview:** `PREVIEW_TODAY=tools/preview/samples/birthday.today tools/preview/build.sh`, open it with the `preview` server: ALEX! and ROBIN! do the wave, the song and finale say ALEX & ROBIN, ROBIN TURNS 60! parades once, scores screens still appear with `PREVIEW_SCORES`. Also an empty-names run (`show=birthday` only) and a three-name run.
- [ ] **Step 5: Commit** `"Birthday show is back, with names from the phone page (#10)"`

---

### Task 8: Install, deploy and keep birthdays safe

**Files:**
- Modify: `pi/Makefile` (install `season.py` to `/opt/led-sign/`), `pi/deploy.sh`

- [ ] **Step 1: Makefile:** `install -m 644 season.py /opt/led-sign/` next to `scores.py`.
- [ ] **Step 2: deploy.sh birthday backup:** before copying, `ssh "$HOST" 'cat /var/lib/led-sign/birthdays 2>/dev/null'` into `~/.led-sign/birthdays` when the reply isn't empty (create `~/.led-sign` with mode 700); after `make install`, if the Pi's file is missing or empty and the local copy exists, send it back (`ssh "$HOST" 'cat > /var/lib/led-sign/birthdays' < ~/.led-sign/birthdays`) and say "Put back N birthdays".
- [ ] **Step 3: Check:** `sh -n pi/deploy.sh`; a dry run against a fake host function is enough on the Mac (the real run happens at deploy time with David's password).
- [ ] **Step 4: Commit** `"Install the calendar; deploy keeps a copy of the birthdays (#10)"`

---

### Task 9: Docs, pictures, release notes, and the full check

**Files:**
- Modify: `README.md` ("Make it yours" paths now `src/shows/…`; Option A: `UNO_SHOW` in `src/config.h`; Option B: the phone page's Show and Birthdays sections; project layout table), `docs/images/phone-page.png` (re-rendered with made-up names), `CHANGELOG.md` (`[1.4.0]`), `TIMELOG.md`

- [ ] **Step 1: Full check:** `tests/run.sh` passes; `tools/preview/build.sh` (Thanksgiving, unchanged); `make -C pi term` no warnings; UNO: `python3 -m venv $SCRATCH/pio && $SCRATCH/pio/bin/pip install -q platformio && $SCRATCH/pio/bin/pio run -e uno` succeeds (flash and RAM within limits; note the numbers); a search of the tree for the family's real names finds nothing.
- [ ] **Step 2: Docs and pictures** as listed; the phone page screenshot shows Show "Automatic: Thanksgiving" and two made-up birthdays.
- [ ] **Step 3: Changelog `[1.4.0]`** for people using the sign: picks its show by date; birthdays on the phone page; pick a show from the phone; birthday show back. Time log line.
- [ ] **Step 4: Commit, push, open the PR** (`Closes #10`, "Part of #6"), using the template; say the Pi deploy and a watch of one pass on the sign are needed before Friday, and what wasn't tested.
