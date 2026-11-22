# Seasonal sign: design

Issue #6. Asked by David on 2026-11-22: "We need to make this a season sign. So when online
need to be pick up and select the event and maybe the scores for the season?"

## Why

The sign lives in the TV room and is meant to stay up all year. Today it holds one show, so
each holiday means someone edits the code and redeploys. David wants it to know the date and
play the right show by itself, let anyone in the house pick a different one from the phone,
and show the scores for whatever sports are in season.

## What David decided

- **Events:** Christmas & New Year; birthdays (names and dates entered on the phone page);
  Valentine's Day, St. Patrick's Day, Easter, the 4th of July; Game day. Halloween and
  Thanksgiving stay.
- **Choosing:** automatic by date when the Pi is online; the phone page can override it.
- **Between holidays:** an **Everyday show**: WELCOME!, today's date, a countdown to the next
  holiday or birthday ("12 DAYS TO VALENTINE'S DAY!"), and jokes that work all year.
- **Game day:** plays **during** a favorite team's game, in any league: from an hour before
  it starts to an hour after it ends. It takes over even from a holiday show, but not on the
  holiday's own day or on a birthday. The rest of the day plays the normal show. (This
  replaces the earlier "football-first, Sundays and big games" note on issue #6: David chose
  "favorite team plays", then "during the game" once it was clear that all-day Game day would
  either hardly show in football season, because holidays cover Oct–Jan, or fill most of the
  summer with an MLB favorite.)
- **Sports:** NFL, NBA, NHL and MLB.
- **How it's built:** the scores service decides which show plays (approach 1 below).
- **Order:** the README guide first (done, v1.3.0), then this, in several pull requests.

## Goals

- The sign on the Pi plays the right show for the day with nobody touching it, all year.
- The phone page shows what's playing and why, and can pick any show.
- Birthdays are entered on the phone page: no names in the code. The repo is public (since
  2026-11-22), so no real names or birthdays go into files, pictures, issues or pull requests.
- Scores screens cover every followed league that has games today, each with its own ball.

## Non-goals

- The UNO doesn't do seasons: no clock, no Wi-Fi, and 32 KB can't hold every show. It keeps
  one show, chosen when you upload.
- No editing the holiday dates from the phone; they're fixed in code (birthdays are the
  exception).
- No other sports (college, soccer), no weather, no clock face.

## Anti-goals (failure even if it "works")

- The sign goes dark, freezes or crash-loops because the calendar, the today file or ESPN
  misbehaves. Whatever goes wrong, the sign keeps playing a show.
- The wrong holiday plays for weeks because a phone pick was forgotten. The page must always
  make a pick obvious.
- Needing the internet to keep running. Offline, the sign keeps the last show it was told.
- Anyone's real name or birthday ends up in the repo, which is public.

## Constraints

- Raspberry Pi Zero W: one slow ARMv6 core, 512 MB. Builds take about 45 s per program.
- Pi services are Python 3 standard library only (no pip on the Pi); the sign is C++17.
- The show code must still build for the UNO (one show at a time, 32 KB flash, 2 KB RAM) and
  for the Mac preview (`tools/preview/build.sh`), which makes the README pictures.
- ESPN's public scoreboard JSON is unofficial and can change; the scores service already
  survives errors and empties the scores after 15 minutes of failures.
- Process (David's standing rules): an issue for each piece, one pull request per issue
  (`Closes #n`), one changelog entry and one tag each, David's approval recorded on the pull
  request before merging, and the pull request says how it was tested.

## Approaches

1. **The scores service decides (chosen).** `scores.py` already runs all day and knows
   today's games. It also works out the day's show (calendar, birthdays, favorite teams'
   games, the phone pick) and writes it to a small file the sign reads, the way it writes the
   scores today. Date math stays short and testable in Python; the sign program just plays what
   it's told. Cost: two programs share a file format that must stay in step.
2. **The sign program decides.** One program, but date math and birthday handling in C++
   (more code, harder to test) and it still needs the scores service for game days.
3. **One program per show, swapped by date.** Show files change least, but every update
   rebuilds ten programs on the Pi Zero, and the sign blinks off at each switch.

David picked 1.

## How it works

### Which show plays

The scores service decides, in this order:

1. **Phone pick.** A show picked on the phone plays until someone sets it back to Automatic.
2. **Birthday.** Every birthday entered on the phone page wins its whole day. People who share
   a birthday appear together. A 29 February birthday shows on 28 February in other years.
3. **A holiday's own day** (Feb 14, Mar 17, Easter Sunday, Jul 4, Oct 31, Thanksgiving Day,
   Dec 25, Jan 1) plays that holiday's show all day. So does New Year's Eve from 11 PM, for
   the midnight countdown.
4. **Game day**, during a favorite team's game: from an hour before its start time to an hour
   after it's final (or an hour after ESPN stops listing it as live, if it's postponed or
   suspended). Several favorites at once: the one playing now, else the one starting soonest.
5. **Holiday window.** The calendar below. Where two windows overlap (Easter can start before
   St. Patrick's Day ends), the holiday whose day comes first wins until its day has passed.
6. **Everyday.** Any other time.

| Show | Plays |
|---|---|
| Valentine's | Feb 1 – Feb 14 |
| St. Patrick's | Mar 1 – Mar 17 |
| Easter | the 14 days up to and including Easter Sunday (computed each year) |
| 4th of July | Jun 21 – Jul 4 |
| Halloween | Oct 1 – Oct 31 |
| Thanksgiving | Nov 1 – Thanksgiving Day (fourth Thursday of November) |
| Christmas | the day after Thanksgiving – Dec 25 |
| New Year | Dec 26 – Jan 1 |
| Everyday | every other day |

Christmas and New Year can be one show file with two moods or two files; builder's call. On
New Year's Eve the sign counts down 10…1 with the Pi's clock at midnight, then HAPPY NEW
YEAR! with fireworks.

Shows that aren't built yet fall back to the Everyday show, or to Thanksgiving until
Everyday exists, so a calendar entry ahead of its show never leaves the sign blank.

### The today file

The scores service writes `/var/lib/led-sign/today` (or `LEDSIGN_TODAY`), whole-file
atomically like the scores file, key=value lines. Proposed keys (builder may adjust, but the
Python writer and the C++ reader must agree and be tested together):

```
show=christmas                  which show to play
why=auto                        auto or picked (for the phone page)
names=ALEX & ROBIN              birthday names, already in capitals, when show=birthday
countdown=12 DAYS TO VALENTINE'S DAY!   the next holiday or birthday, for Everyday
date=SUNDAY NOVEMBER 22         today, for Everyday
team=BILLS                      the favorite team playing, for Game day
sport=nfl                       its league, for Game day's ball
```

The scores service re-checks at least once a minute and at every game-window boundary, so
Game day starts and ends on time. It only rewrites the file when the Pi's clock is synced with the internet
(`timedatectl show -p NTPSynchronized` says yes), so after an offline reboot the sign keeps
the show it last had. A missing or unreadable file means "keep playing what you're playing"
(first boot with no file: the fallback show). When `show` changes, the sign ends the current
scene and starts the new show, as a phone message does today.

### The sign program

- `src/show.cpp` splits into shared effects (drawing, text, sprites, dissolve, drop-in,
  waves, parade, the scores screen, the scene runner and skip handling) and one file per
  show, each with its own words and sprites, kept apart so names don't clash.
- One Pi program holds every show; `runShow()` plays one pass of whichever show the today
  file names. New platform hooks hand the show the today values (names, countdown, date,
  team).
- The UNO and the Mac preview pick one show at build time (a setting in `src/config.h` for
  the UNO; an environment variable for the preview, plus a sample today file so the preview
  can show names, countdowns and game days).
- Every show keeps the current pattern: a scores screen after every scene when there are
  games; a new phone message jumps the queue.

### Sports

- The scores service fetches each followed league from ESPN's scoreboard
  (`football/nfl`, `basketball/nba`, `hockey/nhl`, `baseball/mlb`), each line tagged with its
  league so the sign draws a football, basketball, puck or baseball before it.
- Line formats in the current style: NFL `Q3 4:32`; NBA `Q3 4:32`; NHL `P2 12:01`, `INT`,
  `OT`, `SO`; MLB `TOP 5`, `BOT 7`, `MID 5`, `FINAL` (with the inning if extra). Exact
  wording is the builder's call within the sign's font (capitals, digits, punctuation).
- Settings: a favorite team per league and an on/off per league. The current `team=` setting
  becomes the NFL favorite. Favorites' games come first, marked as today.
- On a busy day (spring has NBA, NHL and MLB together) the rotation could get long; how many
  games per visit and which to prefer (live, then upcoming, then finished) is the builder's
  call, as long as favorites show every time.

### The phone page

- **Show:** "Automatic: Christmas" or "Picked: Halloween", with a list to pick any show or
  Automatic.
- **Birthdays:** add or remove a name and a month and day (birth year optional, for "60
  TODAY!"-style lines). Stored on the Pi in `/var/lib/led-sign/`, never in the repo.
  *My call:* `pi/deploy.sh` keeps a copy on the computer it runs from, outside the repo
  (`~/.led-sign/birthdays`), and puts it back if the Pi has none, so a re-flashed SD card
  doesn't silently lose everyone's birthday.
- **Sports:** on/off and a favorite team for each of NFL, NBA, NHL, MLB; today's games list
  as now, with the league shown.
- Everything else (message, brightness, skip, pause) stays.

## The pieces, in order

Each is its own issue and pull request, released as its own minor version.

| # | Piece | Needed by |
|---|---|---|
| 1 | Show picker: the split into show files, the calendar and today file, birthdays and show picking on the phone page; shows: Thanksgiving and Birthday (from v1.1.0, names from the phone, general jokes) | before Nov 27 |
| 2 | Christmas & New Year, with the midnight countdown | Nov 27 |
| 3 | NBA, NHL and MLB scores, a favorite team per league, and the Game day show during favorites' games | soon (NBA and NHL are in season) |
| 4 | Everyday show | Jan 2 |
| 5 | Valentine's | Feb 1 |
| 6 | St. Patrick's | Mar 1 |
| 7 | Easter | 2 weeks before Easter (Mar 15 in 2027) |
| 8 | 4th of July | Jun 21 |
| 9 | Halloween, rebuilt with an original letter-eater instead of Pac-Man (Pac-Man belongs to Bandai Namco and the repo is going public) | Oct 1 |

## Testing

- The calendar and precedence get Python unit tests (standard library `unittest`): each
  holiday's window across several years (Easter and Thanksgiving move), overlaps, holidays'
  own days, birthdays including 29 February and shared days, picks, game windows (before,
  during, after, overnight, postponed, two favorites), New Year's Eve, and an unsynced clock.
- The today file: a test that the Python writer's output is what the C++ reader expects.
- Each show is checked in the Mac preview and gets README stills; the Pi program builds in
  the terminal version (`make -C pi term`) and the phone page runs on the Mac against it.
- Each piece is deployed to the sign and watched for at least one pass before the next.

## Calls I made (Claude), for David to check

- The holiday windows' exact dates in the table.
- Phone picks last until set back to Automatic, with the page always showing the mode.
- Offline, the sign keeps the last show; a show not yet built falls back to Everyday (or
  Thanksgiving until Everyday exists).
- Holidays' own days and New Year's Eve from 11 PM beat Game day.
- The birthdays backup on the deploying computer.
- Halloween comes back last (piece 9), rebuilt without Pac-Man.
- Each piece is its own issue, pull request, changelog entry and tag.

## Left to the builder

- The content of every show (scenes, sprites, words, jokes), reviewed by David in the preview
  pictures on each pull request.
- File and function names, the exact today-file keys, how the shows are kept apart in C++.
- How the scores rotation handles very busy days.
- The phone page's layout, in the style it has now.
