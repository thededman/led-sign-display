#!/usr/bin/env python3
"""The sign's phone page: serves index.html and reads and writes the settings file that the
show (ledsign) watches, and the birthdays file the scores service reads to pick the day's show.
Standard library only.

    python3 server.py --port 80 --settings /var/lib/led-sign/settings --scores /var/lib/led-sign/scores
"""
import argparse
import datetime
import json
import os
import pathlib
import sys
import tempfile
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

# season.py sits one folder up: pi/ in the repo, /opt/led-sign/ on the Pi
sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent.parent))
import season  # noqa: E402

PAGE = (pathlib.Path(__file__).parent / "index.html").read_bytes()
DEFAULTS = {"brightness": 2, "paused": 0, "skip": 0, "message": "", "scores": 1, "team": "", "show": "auto"}
NUMBERS = ("brightness", "paused", "skip", "scores")
LETTERS = set(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ")  # the sign's font
MAX_MESSAGE = 160
MAX_NAME = 12
MAX_BIRTHDAYS = 50


def load(path):
    settings = dict(DEFAULTS)
    try:
        for line in pathlib.Path(path).read_text().splitlines():
            key, _, value = line.partition("=")
            if key in NUMBERS:
                settings[key] = int(value or 0)
            elif key in ("message", "team", "show"):
                settings[key] = value
    except (OSError, ValueError):
        pass
    return settings


def save(path, settings):
    write_text(path, "".join(f"{key}={settings[key]}\n" for key in DEFAULTS))


def write_text(path, text):
    """Writes the whole file at once, so nothing ever reads half of it."""
    folder = os.path.dirname(path) or "."
    fd, tmp = tempfile.mkstemp(dir=folder)
    with os.fdopen(fd, "w") as f:
        f.write(text)
    os.chmod(tmp, 0o644)
    os.replace(tmp, path)


def clean(message):
    """Capitals, only characters the sign can show, single spaces, not too long."""
    text = " ".join(str(message).upper().split())
    return "".join(c for c in text if c in LETTERS)[:MAX_MESSAGE]


def clean_pick(value):
    """A show the sign has, or "auto"."""
    return value if isinstance(value, str) and value in [i for i, _ in season.SHOWS] else "auto"


def clean_birthdays(items):
    """Birthdays from the page: names in the sign's letters, real dates, no repeats."""
    this_year = datetime.date.today().year
    kept = []
    for item in items if isinstance(items, list) else []:
        if not isinstance(item, dict):
            continue
        name = clean(item.get("name", ""))[:MAX_NAME].strip()
        try:
            month, day = int(item.get("month")), int(item.get("day"))
            year = item.get("year")
            year = int(year) if year not in (None, "") else None
            datetime.date(2000, month, day)  # a leap year, so 29 February is fine
            if year is not None:
                if not 1900 <= year <= this_year:
                    continue
                datetime.date(year, month, day)
        except (TypeError, ValueError):
            continue
        birthday = season.Birthday(name, month, day, year)
        if name and birthday not in kept:
            kept.append(birthday)
    return sorted(kept, key=lambda b: (b.month, b.day, b.name))[:MAX_BIRTHDAYS]


def birthdays_json(items):
    return [{"name": b.name, "month": b.month, "day": b.day, "year": b.year} for b in items]


def read_today(path):
    """The show the sign is playing now, from the today file the scores service writes."""
    try:
        lines = pathlib.Path(path).read_text().splitlines()
    except OSError:
        return {}
    pairs = dict(line.partition("=")[::2] for line in lines if "=" in line)
    return {key: pairs[key] for key in ("show", "why") if key in pairs}


def todays_games(path):
    """The game lines scores.py keeps, favourite first (marked with a leading '*')."""
    try:
        lines = pathlib.Path(path).read_text().splitlines()
    except OSError:
        return []
    return [{"line": l.lstrip("*"), "favourite": l.startswith("*")} for l in lines if l]


class Handler(BaseHTTPRequestHandler):
    settings_path = ""
    scores_path = ""
    today_path = ""
    birthdays_path = ""

    def state(self, settings):
        """The settings, plus what's playing and the shows there are to pick from."""
        shows = [{"id": i, "name": n} for i, n in season.SHOWS]
        return json.dumps(dict(settings, today=read_today(self.today_path), shows=shows)).encode()

    def birthdays(self):
        try:
            return season.parse_birthdays(pathlib.Path(self.birthdays_path).read_text())
        except OSError:
            return []

    def reply(self, code, body, kind="application/json"):
        self.send_response(code)
        self.send_header("Content-Type", kind)
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self):
        if self.path in ("/", "/index.html"):
            self.reply(200, PAGE, "text/html; charset=utf-8")
        elif self.path == "/api/settings":
            self.reply(200, self.state(load(self.settings_path)))
        elif self.path == "/api/birthdays":
            self.reply(200, json.dumps(birthdays_json(self.birthdays())).encode())
        elif self.path == "/api/scores":
            self.reply(200, json.dumps(todays_games(self.scores_path)).encode())
        else:
            self.reply(404, b'{"error": "not found"}')

    def do_POST(self):
        if self.path not in ("/api/settings", "/api/birthdays"):
            return self.reply(404, b'{"error": "not found"}')
        length = int(self.headers.get("Content-Length") or 0)
        if length > 16384:
            return self.reply(413, b'{"error": "too long"}')
        try:
            change = json.loads(self.rfile.read(length) or b"{}")
            if not isinstance(change, dict):
                raise ValueError
        except ValueError:
            return self.reply(400, b'{"error": "bad request"}')

        if self.path == "/api/birthdays":
            kept = clean_birthdays(change.get("birthdays"))
            write_text(self.birthdays_path, season.format_birthdays(kept))
            return self.reply(200, json.dumps(birthdays_json(kept)).encode())

        settings = load(self.settings_path)
        if "brightness" in change:
            settings["brightness"] = max(0, min(15, int(change["brightness"])))
        if "paused" in change:
            settings["paused"] = 1 if change["paused"] else 0
        if change.get("skip"):
            settings["skip"] += 1
        if "message" in change:
            settings["message"] = clean(change["message"])
        if "scores" in change:
            settings["scores"] = 1 if change["scores"] else 0
        if "team" in change:
            settings["team"] = "".join(c for c in str(change["team"]).upper() if c.isalpha())[:4]
        if "show" in change:
            settings["show"] = clean_pick(change["show"])
        save(self.settings_path, settings)
        self.reply(200, self.state(settings))

    def log_message(self, *args):
        pass


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", type=int, default=80)
    parser.add_argument("--settings", default="/var/lib/led-sign/settings")
    parser.add_argument("--scores", default="/var/lib/led-sign/scores")
    parser.add_argument("--today", default="/var/lib/led-sign/today")
    parser.add_argument("--birthdays", default="/var/lib/led-sign/birthdays")
    args = parser.parse_args()
    Handler.settings_path = args.settings
    Handler.scores_path = args.scores
    Handler.today_path = args.today
    Handler.birthdays_path = args.birthdays
    if not os.path.exists(args.settings):
        save(args.settings, DEFAULTS)
    ThreadingHTTPServer(("", args.port), Handler).serve_forever()


if __name__ == "__main__":
    main()
