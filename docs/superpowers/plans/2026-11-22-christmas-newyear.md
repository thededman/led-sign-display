# Christmas & New Year (#17) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** A Christmas show (from the day after Thanksgiving to Dec 25) and a New Year show (Dec 26 – Jan 1), and on New Year's Eve a 10…1 countdown in step with the Pi's clock at midnight.

**Architecture:** Two new show files beside Thanksgiving and Birthday, listed in the show table and in `season.SHOWS` (the calendar already names them). A new platform hook gives the show code the local date and time (the Pi's clock; a simulated clock in the preview and the tests; none on the UNO). Shared helpers turn it into "days until Dec 25", "seconds to the new year" and "the new year's number". The countdown lives in the shared scene runner, so it starts on time whatever show is playing.

**Tech Stack:** As in piece 1: C++17 show code (Pi, Mac preview, UNO), Python 3 standard library.

**Spec:** `docs/superpowers/specs/2026-11-22-seasonal-sign-design.md` (piece 2). Issue #17, part of #6. Builds on `docs/superpowers/plans/2026-11-22-show-picker.md`.

## Global Constraints

- Everything from the show-picker plan's Global Constraints still holds (three builds, no new dependencies, no real names, the font's characters only, commit trailer).
- Show ids `christmas` and `newyear`; names on the phone page "Christmas" and "New Year".
- `src/config.h`: `SHOW_CHRISTMAS 3`, `SHOW_NEWYEAR 4`; both must build for the UNO (no clock there: no sleeps count, no year number, no countdown).
- The Thanksgiving and Birthday shows must stay frame-for-frame identical (preview shasums before and after).
- Calendar windows are unchanged (spec): Christmas from the day after Thanksgiving to Dec 25, New Year Dec 26 – Jan 1, New Year's Eve protected from 23:00.
- The countdown: digit 10 on the sign while 10 ≥ seconds left > 9, …, digit 1 while 1 ≥ seconds left > 0, then HAPPY NEW YEAR at midnight; it runs whatever show is playing.

## Review Focus

1. **The countdown starting late:** a long scene or scores screen running at 23:58 must not push the countdown past midnight. Pinned in Task 2 (the countdown starts from `play()` once 150 s or less are left, and the test runs a whole Thanksgiving pass into it).
2. **No clock, or a clock that's wrong** (UNO, preview without `PREVIEW_CLOCK`, an offline Pi): no countdown, no sleeps line, no year number, and nothing crashes. Pinned in Task 1 (helpers return -1/0) and Task 2.
3. **A pick on New Year's Eve** (someone picked Birthday at 22:00): the countdown still runs at midnight. Pinned in Task 2 (the countdown test runs inside the Thanksgiving show).
4. **Dates around the edges:** Dec 25 itself ("MERRY CHRISTMAS!", not "0 SLEEPS"), Dec 31 before 23:57:30, Jan 1 after midnight (no second countdown). Pinned in Task 1.
5. **Refactor drift:** moving confetti and fireworks out of the birthday show must not change it. Pinned in Task 3 (birthday preview shasum).

---

### Task 1: The local clock hook and date helpers

**Files:**
- Modify: `src/show.h` (hook), `src/effects.h` / `src/effects.cpp` (helpers), `src/main.cpp` (UNO: no clock), `pi/main.cpp` (Pi clock), `tools/preview/preview.cpp` (simulated clock from `PREVIEW_CLOCK`), `tests/show_test.cpp`

**Interfaces:**
- Produces (`src/show.h`, supplied by each platform): `struct LocalTime { uint16_t year; uint8_t month, day; uint32_t ms; };` (`ms` since local midnight) and `bool localTime(LocalTime *t);` — false when there's no clock.
- Produces (`src/effects.h`): `int16_t daysUntil(uint8_t month, uint8_t day);` (0 on the day, -1 without a clock; counts across the year end); `int32_t msToNewYear();` (milliseconds to midnight on Dec 31, else -1); `uint16_t newYearNumber();` (the year that starts next Jan 1 in December, else this year; 0 without a clock).
- Preview: `PREVIEW_CLOCK="YYYY-MM-DD HH:MM:SS"` starts a simulated clock that moves on by each frame's `ms`. Tests: the same, scripted.

- [ ] **Step 1: Failing tests** in `tests/show_test.cpp` (a scripted clock the test sets: `setClock(2026, 12, 20, 9, 0, 0)`; frames advance it):
  `daysUntil(12, 25) == 5`; on Dec 25 `== 0`; on Dec 26 `== 364`; without a clock `-1`. `msToNewYear()` at Dec 31 23:59:50 `== 10000`, at Dec 30 `== -1`, at Jan 1 00:00:01 `== -1`. `newYearNumber()` on Dec 31 2026 `== 2027`, on Jan 1 2027 `== 2027`, no clock `0`.
- [ ] **Step 2: Run** `tests/run.sh` → fails to compile (`localTime` undeclared).
- [ ] **Step 3: Implement.** Pi: `std::time` + `localtime_r`. UNO: `return false`. Preview: parse `PREVIEW_CLOCK` once; keep a running `ms` total in `frame()`; convert to a date by stepping days with month lengths (leap years included).
- [ ] **Step 4: `tests/run.sh` passes;** Thanksgiving and Birthday preview shasums unchanged.
- [ ] **Step 5: Commit** `"The shows can read the local date and time (#17)"`

### Task 2: The New Year's Eve countdown

**Files:** `src/effects.cpp` (`play()` and the countdown scene), `tests/show_test.cpp`

**Interfaces:** Produces `void newYearCountdown();` (shared scene): until 10 s are left, "NEW YEAR IN" scrolls by with the minutes and seconds left (e.g. `1:42`); then the digits 10…1 double width and centred, each changing exactly as the second turns (poll the clock every 50 ms); at midnight HAPPY NEW YEAR! with fireworks (Task 3's shared fireworks) and the year number when there is one, about 30 s. `play()` runs it instead of the scene when `0 < msToNewYear() <= 150000`.

- [ ] **Step 1: Failing test:** clock at 2026-12-31 23:58:00, `todayShow = "thanksgiving"`, run `runShow()` twice; the frame hook captures `fb` when the clock passes 23:59:50.5 and 23:59:59.5 and 00:00:00.5. Assert the first equals a buffer the test draws with `drawText(buf, "10", centered("10", 2), 0, 2)`, the second the same for `"1"`, and the third is not the `"1"` buffer.
- [ ] **Step 2: Run** → fails (no countdown yet).
- [ ] **Step 3: Implement** (needs Task 3's fireworks: do Task 3 first if easier, or draw the midnight part with `confetti` and add fireworks after Task 3).
- [ ] **Step 4: Tests pass;** a preview with `PREVIEW_CLOCK="2026-12-31 23:58:30"` shows it; Thanksgiving/Birthday shasums unchanged without `PREVIEW_CLOCK`.
- [ ] **Step 5: Commit** `"New Year's Eve: a countdown to midnight on whatever show is playing (#17)"`

### Task 3: Share confetti and fireworks

**Files:** `src/effects.h`, `src/effects.cpp`, `src/shows/birthday.cpp`

**Interfaces:** Produces `void confettiStart(); void confettiStep(int f, uint8_t count, bool refill); void fireworks(uint8_t count);` moved unchanged from the birthday show.

- [ ] **Step 1:** Record the birthday preview shasum (`PREVIEW_TODAY=tools/preview/samples/birthday.today`).
- [ ] **Step 2:** Move them; birthday keeps calling them.
- [ ] **Step 3:** `tests/run.sh` passes; the birthday shasum is unchanged.
- [ ] **Step 4: Commit** `"Share confetti and fireworks between shows (#17)"`

### Task 4: The Christmas show

**Files:** Create `src/shows/christmas.cpp`, `christmas_words.h`, `christmas_sprites.h`; modify `src/show.cpp` (table), `src/config.h` (`SHOW_CHRISTMAS`), `pi/season.py` (`SHOWS`), `tests/test_season.py`, `tests/show_test.cpp`; create `tools/preview/samples/christmas.today`.

**Interfaces:** `void runChristmas();` table entry `{"christmas", runChristmas}`.

- [ ] **Step 1: Failing tests:** C++ `resolveShow("christmas")->name == "christmas"`, a full pass runs (> 1000 frames) with and without a clock. Python: `decide(2026-12-10, [], "auto")["show"] == "christmas"`; the old fallback test moves to a date whose show isn't built (2027-02-10 → `thanksgiving`).
- [ ] **Step 2: Run** → fail.
- [ ] **Step 3: Build the show** (content is the builder's call; David reviews the stills on the PR): snow falling, then MERRY / CHRISTMAS! dropping in; Santa's sleigh and reindeer flying across with HO HO HO!; a tree whose lights twinkle and star shines; a snowman built ball by ball who shivers (BRRR!); "N SLEEPS TILL CHRISTMAS!" when the clock says 1–30 days (skipped on the day and without a clock); 18 Christmas jokes that work for any family, with a parade of tree, present, snowman and candy cane; interludes (snowfall, a gingerbread man running, bells JINGLE!); finale MERRY CHRISTMAS FROM OUR HOUSE TO YOURS! Pixel art 8×8 like the other shows. No real names.
- [ ] **Step 4: Tests pass; preview** with `PREVIEW_TODAY=tools/preview/samples/christmas.today PREVIEW_CLOCK="2026-12-20 18:00:00"`: contact sheet of a pass reviewed by eye; stills and a GIF for the README.
- [ ] **Step 5: Commit** `"Christmas show (#17)"`

### Task 5: The New Year show

**Files:** Create `src/shows/newyear.cpp`, `newyear_words.h`, `newyear_sprites.h`; modify `src/show.cpp`, `src/config.h` (`SHOW_NEWYEAR`), `pi/season.py`, `tests/test_season.py`, `tests/show_test.cpp`; create `tools/preview/samples/newyear.today`.

**Interfaces:** `void runNewYear();` table entry `{"newyear", runNewYear}`.

- [ ] **Step 1: Failing tests** as in Task 4 for `newyear` (Python: `decide(2026-12-28)` → `newyear`).
- [ ] **Step 2: Run** → fail.
- [ ] **Step 3: Build the show:** confetti, then HAPPY / NEW YEAR! dropping in; the year rolling over (`2026` → `2027`, its last digit flipping, with fireworks; just NEW YEAR! without a clock); a champagne bottle whose cork pops (POP!) with bubbles rising; resolution jokes (18) with a parade of party hat, champagne glass, clock and star; interludes (fireworks, confetti, a party horn); finale HAPPY NEW YEAR FROM OUR HOUSE TO YOURS!
- [ ] **Step 4: Tests pass; preview** with `PREVIEW_CLOCK="2026-12-28 20:00:00"` and with `"2026-12-31 23:58:30"` (the countdown); stills for the README.
- [ ] **Step 5: Commit** `"New Year show (#17)"`

### Task 6: Docs, release, full check

**Files:** `README.md` (gallery: Christmas and New Year stills; "Shows by the date" paragraph lists them; the countdown), `CHANGELOG.md` (`[1.5.0]`), `TIMELOG.md`, `docs/images/*`.

- [ ] **Step 1: Full check:** `tests/run.sh`; preview of all four shows; `make -C pi term` no warnings; UNO `pio run -e uno` with `UNO_SHOW` set to each of Thanksgiving, Christmas and New Year (flash and RAM within limits); search the tree for real names.
- [ ] **Step 2: Docs and changelog;** commit; final whole-branch review; fixes; push; PR (`Closes #17`, part of #6) saying it must be on the Pi before Friday, Nov 27.
