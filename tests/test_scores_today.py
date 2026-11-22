"""Tests for the scores service writing the today file."""
import datetime
import pathlib
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "pi"))
import scores  # noqa: E402


def T(*a):
    return datetime.datetime(*a)


class UpdateToday(unittest.TestCase):
    def setUp(self):
        self.dir = tempfile.TemporaryDirectory()
        d = pathlib.Path(self.dir.name)
        self.today, self.settings, self.bdays = str(d / "today"), str(d / "settings"), str(d / "birthdays")
        self.missing = str(d / "missing")

    def tearDown(self):
        self.dir.cleanup()

    def read(self, path):
        return pathlib.Path(path).read_text()

    def test_writes_decision(self):
        pathlib.Path(self.settings).write_text("show=auto\n")
        pathlib.Path(self.bdays).write_text("11-22 ALEX\n")
        self.assertTrue(scores.update_today(self.today, self.settings, self.bdays, T(2026, 11, 22, 9), synced=True))
        self.assertEqual(self.read(self.today), "show=birthday\nwhy=auto\nnames=ALEX\nturning=\n")
        # the same decision a minute later doesn't rewrite the file
        self.assertFalse(scores.update_today(self.today, self.settings, self.bdays, T(2026, 11, 22, 9, 1), synced=True))

    def test_unsynced_clock_leaves_file(self):
        pathlib.Path(self.today).write_text("show=thanksgiving\nwhy=auto\nnames=\nturning=\n")
        pathlib.Path(self.bdays).write_text("01-01 ALEX\n")
        self.assertFalse(scores.update_today(self.today, self.settings, self.bdays, T(2031, 1, 1), synced=False))
        self.assertEqual(self.read(self.today), "show=thanksgiving\nwhy=auto\nnames=\nturning=\n")

    def test_missing_files(self):  # no settings and no birthdays yet
        self.assertTrue(scores.update_today(self.today, self.missing, self.missing, T(2026, 11, 22, 9), synced=True))
        self.assertIn("show=thanksgiving\n", self.read(self.today))


if __name__ == "__main__":
    unittest.main()
