"""Tests for the phone page server's show pick and birthdays."""
import json
import pathlib
import sys
import tempfile
import threading
import unittest
import urllib.error
import urllib.request

ROOT = pathlib.Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "pi" / "web"))
import server  # noqa: E402


class Server(unittest.TestCase):
    def test_clean_pick(self):
        self.assertEqual([server.clean_pick(v) for v in ("birthday", "auto", "halloween", "", None, 3)],
                         ["birthday", "auto", "auto", "auto", "auto", "auto"])

    def test_clean_birthdays(self):
        raw = [{"name": "alex", "month": 11, "day": 1},
               {"name": "Robin \U0001F382", "month": 2, "day": 29, "year": 1964},
               {"name": "x" * 40, "month": 3, "day": 2},
               {"name": "", "month": 1, "day": 1},
               {"name": "SAM", "month": 2, "day": 31},
               {"name": "SAM", "month": 4, "day": 1, "year": 1800},
               {"name": "alex", "month": 11, "day": 1},
               "junk"]
        got = server.clean_birthdays(raw)
        self.assertEqual([(b.name, b.month, b.day, b.year) for b in got],
                         [("ROBIN", 2, 29, 1964), ("XXXXXXXXXXXX", 3, 2, None), ("ALEX", 11, 1, None)])

    def test_load_cleans_stale_pick(self):
        """A pick of a show the sign doesn't have (typed by hand, or removed) reads as Automatic."""
        with tempfile.TemporaryDirectory() as d:
            path = pathlib.Path(d) / "settings"
            path.write_text("show=halloween\n")
            self.assertEqual(server.load(str(path))["show"], "auto")

    def test_post_without_a_list_keeps_birthdays(self):
        """A POST that isn't a list of birthdays must not empty the file."""
        with tempfile.TemporaryDirectory() as d:
            bdays = pathlib.Path(d) / "birthdays"
            bdays.write_text("11-01 ALEX\n")
            server.Handler.birthdays_path = str(bdays)
            httpd = server.ThreadingHTTPServer(("127.0.0.1", 0), server.Handler)
            threading.Thread(target=httpd.serve_forever, daemon=True).start()
            try:
                url = f"http://127.0.0.1:{httpd.server_address[1]}/api/birthdays"
                for body in (b"{}", b'{"birthdays": "ALEX"}'):
                    request = urllib.request.Request(url, data=body, method="POST")
                    with self.assertRaises(urllib.error.HTTPError) as caught:
                        urllib.request.urlopen(request, timeout=5)
                    self.assertEqual(caught.exception.code, 400)
                    caught.exception.close()
                self.assertEqual(bdays.read_text(), "11-01 ALEX\n")
            finally:
                httpd.shutdown()
                httpd.server_close()

    def test_settings_carry_the_page_version(self):
        """An open phone page reloads itself when this changes, i.e. after the sign is updated."""
        self.assertRegex(server.PAGE_VERSION, r"^[0-9a-f]{12}$")
        with tempfile.TemporaryDirectory() as d:
            server.Handler.settings_path = str(pathlib.Path(d) / "settings")
            server.Handler.today_path = str(pathlib.Path(d) / "today")
            httpd = server.ThreadingHTTPServer(("127.0.0.1", 0), server.Handler)
            threading.Thread(target=httpd.serve_forever, daemon=True).start()
            try:
                url = f"http://127.0.0.1:{httpd.server_address[1]}/api/settings"
                with urllib.request.urlopen(url, timeout=5) as reply:
                    self.assertEqual(json.load(reply)["page"], server.PAGE_VERSION)
                with urllib.request.urlopen(url.replace("/api/settings", "/"), timeout=5) as reply:
                    self.assertIn(f'content="{server.PAGE_VERSION}"', reply.read().decode())
            finally:
                httpd.shutdown()
                httpd.server_close()

    def test_settings_keep_show(self):
        with tempfile.TemporaryDirectory() as d:
            path = str(pathlib.Path(d) / "settings")
            server.save(path, dict(server.DEFAULTS, show="birthday"))
            self.assertEqual(server.load(path)["show"], "birthday")


if __name__ == "__main__":
    unittest.main()
