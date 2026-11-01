#!/usr/bin/env python3
"""The sign's phone page: serves index.html and reads and writes the settings file that the
show (ledsign) watches. Standard library only.

    python3 server.py --port 80 --settings /var/lib/led-sign/settings --scores /var/lib/led-sign/scores
"""
import argparse
import json
import os
import pathlib
import tempfile
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

PAGE = (pathlib.Path(__file__).parent / "index.html").read_bytes()
DEFAULTS = {"brightness": 2, "paused": 0, "skip": 0, "message": "", "scores": 1, "team": ""}
NUMBERS = ("brightness", "paused", "skip", "scores")
LETTERS = set(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ")  # the sign's font
MAX_MESSAGE = 160


def load(path):
    settings = dict(DEFAULTS)
    try:
        for line in pathlib.Path(path).read_text().splitlines():
            key, _, value = line.partition("=")
            if key in NUMBERS:
                settings[key] = int(value or 0)
            elif key in ("message", "team"):
                settings[key] = value
    except (OSError, ValueError):
        pass
    return settings


def save(path, settings):
    """Writes the whole file at once, so the show never reads half of it."""
    text = "".join(f"{key}={settings[key]}\n" for key in DEFAULTS)
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
            self.reply(200, json.dumps(load(self.settings_path)).encode())
        elif self.path == "/api/scores":
            self.reply(200, json.dumps(todays_games(self.scores_path)).encode())
        else:
            self.reply(404, b'{"error": "not found"}')

    def do_POST(self):
        if self.path != "/api/settings":
            return self.reply(404, b'{"error": "not found"}')
        length = int(self.headers.get("Content-Length") or 0)
        if length > 4096:
            return self.reply(413, b'{"error": "too long"}')
        try:
            change = json.loads(self.rfile.read(length) or b"{}")
        except ValueError:
            return self.reply(400, b'{"error": "bad request"}')

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
        save(self.settings_path, settings)
        self.reply(200, json.dumps(settings).encode())

    def log_message(self, *args):
        pass


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", type=int, default=80)
    parser.add_argument("--settings", default="/var/lib/led-sign/settings")
    parser.add_argument("--scores", default="/var/lib/led-sign/scores")
    args = parser.parse_args()
    Handler.settings_path = args.settings
    Handler.scores_path = args.scores
    if not os.path.exists(args.settings):
        save(args.settings, DEFAULTS)
    ThreadingHTTPServer(("", args.port), Handler).serve_forever()


if __name__ == "__main__":
    main()
