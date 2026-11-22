#!/usr/bin/env python3
"""Keeps a file of today's NFL games up to date for the sign, from ESPN's public scoreboard
(unofficial, no key needed), and decides which show the sign plays today (season.py), writing
it to the today file. Standard library only.

    python3 scores.py --out /var/lib/led-sign/scores --settings /var/lib/led-sign/settings
    python3 scores.py --once        print today's lines and show, and stop

One game per line, away team first: "BUF 17 @ KC 21 Q3 4:32", "NYG 10 @ DAL 24 FINAL",
"MIA @ NE 1:00 PM". The favourite team's game (team= in the settings) comes first, marked
with '*'; then games in progress, finished, and still to come. With scores=0 in the
settings, the file is left empty and the sign shows no scores.
"""
import argparse
import datetime
import json
import os
import pathlib
import subprocess
import sys
import tempfile
import time
import urllib.request

import season

URL = "https://site.api.espn.com/apis/site/v2/sports/football/nfl/scoreboard"
ORDER = {"in": 0, "post": 1, "pre": 2}


def read_settings(path):
    settings = {}
    try:
        for line in pathlib.Path(path).read_text().splitlines():
            key, _, value = line.partition("=")
            settings[key] = value
    except OSError:
        pass
    return settings


def read_text(path):
    try:
        return pathlib.Path(path).read_text()
    except OSError:
        return ""


def clock_synced():
    """True once the Pi's clock has been set from the internet (always true on a Mac)."""
    try:
        result = subprocess.run(["timedatectl", "show", "-p", "NTPSynchronized", "--value"],
                                capture_output=True, text=True, timeout=10)
    except FileNotFoundError:
        return True
    except subprocess.TimeoutExpired:
        return False
    return result.stdout.strip() == "yes"


def update_today(today_path, settings_path, birthdays_path, now, synced):
    """Writes today's show to the today file; returns True if the file changed. With the clock
    not yet synced (an offline reboot) only a show picked on the phone is written, since that
    doesn't depend on the date; otherwise the file is left alone and the sign keeps its show."""
    pick = read_settings(settings_path).get("show", "auto")
    if not synced and pick not in [i for i, _ in season.SHOWS]:
        return False
    birthdays = season.parse_birthdays(read_text(birthdays_path))
    text = season.today_text(season.decide(now, birthdays, pick))
    if read_text(today_path) == text:
        return False
    write(today_path, text.splitlines())
    return True


def fetch():
    request = urllib.request.Request(URL, headers={"User-Agent": "led-sign/1.0"})
    with urllib.request.urlopen(request, timeout=15) as response:
        return json.load(response)


def describe(event):
    """(state, kickoff, teams, line) for one game."""
    competitors = {c["homeAway"]: c for c in event["competitions"][0]["competitors"]}
    away, home = competitors["away"], competitors["home"]
    a, h = away["team"]["abbreviation"].upper(), home["team"]["abbreviation"].upper()
    status = event["status"]
    kind = status["type"]
    state = kind.get("state", "pre")
    kickoff = datetime.datetime.fromisoformat(event["date"].replace("Z", "+00:00")).astimezone()

    if state == "pre":
        line = f"{a} @ {h} {kickoff.strftime('%I:%M %p').lstrip('0')}"
    else:
        line = f"{a} {away.get('score', '0')} @ {h} {home.get('score', '0')}"
        period = int(status.get("period") or 0)
        quarter = "OT" if period >= 5 else f"Q{period}"
        if state == "post":
            line += " FINAL" + (" OT" if "OT" in kind.get("shortDetail", "") else "")
        elif kind.get("name") == "STATUS_HALFTIME":
            line += " HALFTIME"
        elif kind.get("name") == "STATUS_END_PERIOD":
            line += f" END {quarter}"
        else:
            line += f" {quarter} {status.get('displayClock', '')}".rstrip()
    return state, kickoff, (a, h), line


def todays_lines(data, team):
    today = datetime.datetime.now().astimezone().date()
    games = []
    for event in data.get("events", []):
        try:
            state, kickoff, teams, line = describe(event)
        except (KeyError, ValueError, IndexError):
            continue
        if kickoff.date() == today or state == "in":
            favourite = bool(team) and team in teams
            games.append((not favourite, ORDER.get(state, 3), kickoff, ("*" if favourite else "") + line))
    games.sort()
    return [game[-1] for game in games]


def write(path, lines):
    """Writes the whole file at once, so the sign never reads half of it."""
    folder = os.path.dirname(path) or "."
    fd, tmp = tempfile.mkstemp(dir=folder)
    with os.fdopen(fd, "w") as f:
        f.write("".join(line + "\n" for line in lines))
    os.chmod(tmp, 0o644)
    os.replace(tmp, path)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--out", default="/var/lib/led-sign/scores")
    parser.add_argument("--settings", default="/var/lib/led-sign/settings")
    parser.add_argument("--today", default="/var/lib/led-sign/today")
    parser.add_argument("--birthdays", default="/var/lib/led-sign/birthdays")
    parser.add_argument("--once", action="store_true", help="print today's lines and show, and stop")
    args = parser.parse_args()

    if args.once:
        settings = read_settings(args.settings)
        birthdays = season.parse_birthdays(read_text(args.birthdays))
        print(season.today_text(season.decide(datetime.datetime.now(), birthdays, settings.get("show", "auto"))),
              end="")
        print("\n".join(todays_lines(fetch(), settings.get("team", ""))) or "(no games today)")
        return

    last_ok = time.monotonic()
    next_fetch = 0.0
    decided_minute = None
    clock_ok = False  # once synced, the Pi keeps good time even if Wi-Fi drops for a while
    watched = None  # the settings and birthdays files as last seen
    while True:
        now = datetime.datetime.now()
        files = (read_text(args.settings), read_text(args.birthdays))
        minute = now.replace(second=0, microsecond=0)
        if files != watched or minute != decided_minute:  # once a minute, and when they change
            try:
                clock_ok = clock_ok or clock_synced()
                update_today(args.today, args.settings, args.birthdays, now, clock_ok)
            except Exception as error:  # never let the calendar stop the scores
                print(f"today: {error}", file=sys.stderr, flush=True)
            decided_minute = minute
        if watched is not None and files[0] != watched[0]:
            next_fetch = 0.0  # scores switched on or off, or a new favourite: fetch now
        watched = files

        if time.monotonic() >= next_fetch:
            settings = read_settings(args.settings)
            wait = 300
            if settings.get("scores", "1") == "0":
                write(args.out, [])
            else:
                try:
                    lines = todays_lines(fetch(), settings.get("team", ""))
                    write(args.out, lines)
                    last_ok = time.monotonic()
                    live = any(" Q" in l or " OT" in l or "HALFTIME" in l or " END " in l
                               for l in lines if "FINAL" not in l)
                    wait = 30 if live else 120 if lines else 600
                except Exception as error:  # keep going through Wi-Fi drops and ESPN hiccups
                    print(f"scores: {error}", file=sys.stderr, flush=True)
                    if time.monotonic() - last_ok > 15 * 60:
                        write(args.out, [])  # don't leave old scores up for long
                    wait = 60
            next_fetch = time.monotonic() + wait
        time.sleep(1)


if __name__ == "__main__":
    main()
