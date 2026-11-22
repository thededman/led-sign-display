"""Tests for pi/season.py: the holiday calendar and which show plays today."""
import datetime
import pathlib
import sys
import unittest

ROOT = pathlib.Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "pi"))
import season  # noqa: E402

D = datetime.date


def T(*a):
    return datetime.datetime(*a)


class Calendar(unittest.TestCase):
    def test_easter(self):
        self.assertEqual([season.easter(y) for y in (2026, 2027, 2028, 2038)],
                         [D(2026, 4, 5), D(2027, 3, 28), D(2028, 4, 16), D(2038, 4, 25)])

    def test_thanksgiving(self):
        self.assertEqual([season.thanksgiving(y) for y in (2026, 2027, 2029)],
                         [D(2026, 11, 26), D(2027, 11, 25), D(2029, 11, 22)])

    def test_calendar(self):
        cases = {
            T(2026, 11, 22, 12): ("thanksgiving", False),
            T(2026, 11, 26, 9): ("thanksgiving", True),
            T(2026, 11, 27, 0): ("christmas", False),
            T(2026, 12, 25, 8): ("christmas", True),
            T(2026, 12, 26, 8): ("newyear", False),
            T(2026, 12, 31, 22, 59): ("newyear", False),
            T(2026, 12, 31, 23, 0): ("newyear", True),
            T(2027, 1, 1, 10): ("newyear", True),
            T(2027, 1, 2, 10): ("everyday", False),
            T(2027, 2, 14, 10): ("valentines", True),
            T(2027, 3, 15, 10): ("stpatricks", False),  # Easter 2027's window starts Mar 15
            T(2027, 3, 17, 10): ("stpatricks", True),
            T(2027, 3, 18, 10): ("easter", False),
            T(2027, 3, 28, 10): ("easter", True),
            T(2027, 6, 20, 10): ("everyday", False),
            T(2027, 6, 21, 10): ("july4", False),
            T(2027, 10, 1, 0): ("halloween", False),
        }
        for now, want in cases.items():
            self.assertEqual(season.calendar_show(now), want, now)


class Birthdays(unittest.TestCase):
    def test_birthdays_parse_and_leap(self):
        items = season.parse_birthdays("11-01 ALEX\n1966-11-01 ROBIN\n02-29 SAM\nnonsense\n13-40 BAD\n")
        self.assertEqual([b.name for b in items], ["ALEX", "ROBIN", "SAM"])
        self.assertEqual(items[1].year, 1966)
        self.assertEqual([b.name for b in season.birthdays_on(D(2027, 2, 28), items)], ["SAM"])
        self.assertEqual([b.name for b in season.birthdays_on(D(2028, 2, 28), items)], [])
        self.assertEqual([b.name for b in season.birthdays_on(D(2028, 2, 29), items)], ["SAM"])
        self.assertEqual(season.parse_birthdays(season.format_birthdays(items)), items)

    def test_join_names(self):
        self.assertEqual([season.join_names(n) for n in (["ALEX"], ["ALEX", "ROBIN"], ["ALEX", "ROBIN", "SAM"], [])],
                         ["ALEX", "ALEX & ROBIN", "ALEX, ROBIN & SAM", ""])


class Decide(unittest.TestCase):
    def test_decide_precedence(self):
        b = season.parse_birthdays("11-27 ALEX\n1966-11-27 ROBIN\n")
        d = season.decide(T(2026, 11, 27, 9), b, "auto")
        self.assertEqual(d, {"show": "birthday", "why": "auto", "names": "ALEX & ROBIN",
                             "turning": "ROBIN TURNS 60!"})
        picked = season.decide(T(2026, 11, 27, 9), b, "thanksgiving")  # a pick wins
        self.assertEqual((picked["show"], picked["why"]), ("thanksgiving", "picked"))

    def test_decide_fallback_and_bad_pick(self):
        d = season.decide(T(2026, 12, 10, 9), [], "auto")  # christmas isn't built yet
        self.assertEqual((d["show"], d["why"]), ("thanksgiving", "auto"))
        d = season.decide(T(2026, 11, 22, 9), [], "halloween")  # stale or unknown pick = Automatic
        self.assertEqual((d["show"], d["why"]), ("thanksgiving", "auto"))

    def test_decide_picked_birthday_uses_next_names(self):
        b = season.parse_birthdays("03-02 SAM\n")
        d = season.decide(T(2026, 11, 22, 9), b, "birthday")
        self.assertEqual((d["show"], d["why"], d["names"]), ("birthday", "picked", "SAM"))
        self.assertEqual(season.decide(T(2026, 11, 22, 9), [], "birthday")["names"], "")

    def test_midnight_rollover(self):
        b = season.parse_birthdays("11-23 ALEX\n")
        self.assertEqual(season.decide(T(2026, 11, 22, 23, 59), b, "auto")["show"], "thanksgiving")
        self.assertEqual(season.decide(T(2026, 11, 23, 0, 0), b, "auto")["show"], "birthday")

    def test_today_text(self):
        self.assertEqual(season.today_text({"show": "birthday", "why": "auto", "names": "ALEX & ROBIN",
                                            "turning": ""}),
                         "show=birthday\nwhy=auto\nnames=ALEX & ROBIN\nturning=\n")


class TodayFile(unittest.TestCase):
    def test_sample_matches_writer(self):
        """tests/today.sample is what the sign's C++ reader is tested against."""
        decision = {"show": "birthday", "why": "auto", "names": "ALEX & ROBIN", "turning": "ROBIN TURNS 60!"}
        self.assertEqual(season.today_text(decision), (ROOT / "tests" / "today.sample").read_text())


if __name__ == "__main__":
    unittest.main()
