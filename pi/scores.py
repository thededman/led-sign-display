#!/usr/bin/env python3
"""Keeps a file of today's NFL games up to date for the sign, from ESPN's public scoreboard
(unofficial, no key needed). Standard library only.

    python3 scores.py --out /var/lib/led-sign/scores --settings /var/lib/led-sign/settings
    python3 scores.py --once        print today's lines and stop

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
import sys
import tempfile
import time
import urllib.request

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
    parser.add_argument("--once", action="store_true", help="print today's lines and stop")
    args = parser.parse_args()

    if args.once:
        team = read_settings(args.settings).get("team", "")
        print("\n".join(todays_lines(fetch(), team)) or "(no games today)")
        return

    last_ok = time.monotonic()
    while True:
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
        # wake up early if the settings change (scores switched on or off, new favourite team)
        before = pathlib.Path(args.settings).read_text() if os.path.exists(args.settings) else ""
        for _ in range(wait):
            time.sleep(1)
            now = pathlib.Path(args.settings).read_text() if os.path.exists(args.settings) else ""
            if now != before:
                break


if __name__ == "__main__":
    main()
