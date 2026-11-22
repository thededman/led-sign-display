"""Which show the sign plays today: the holiday calendar, birthdays and a show picked on the
phone. Standard library only; the scores service calls decide() and writes the result to the
today file the sign reads.
"""
import dataclasses
import datetime

# The shows the sign has, in the order the phone page lists them: (id, name)
SHOWS = [("thanksgiving", "Thanksgiving"), ("birthday", "Birthday")]

D = datetime.date


def easter(year):
    """Easter Sunday (the Anonymous Gregorian algorithm)."""
    a, b, c = year % 19, year // 100, year % 100
    d, e = b // 4, b % 4
    f = (b + 8) // 25
    g = (b - f + 1) // 3
    h = (19 * a + b - d - g + 15) % 30
    i, k = c // 4, c % 4
    l = (32 + 2 * e + 2 * i - h - k) % 7
    m = (a + 11 * h + 22 * l) // 451
    month = (h + l - 7 * m + 114) // 31
    day = (h + l - 7 * m + 114) % 31 + 1
    return D(year, month, day)


def thanksgiving(year):
    """The fourth Thursday of November."""
    first = D(year, 11, 1)
    return first + datetime.timedelta(days=(3 - first.weekday()) % 7 + 21)


def windows(year):
    """(show, first day, the holiday's own day) for each holiday that ends in or after `year`."""
    day = datetime.timedelta(days=1)
    tg = thanksgiving(year)
    return [
        ("valentines", D(year, 2, 1), D(year, 2, 14)),
        ("stpatricks", D(year, 3, 1), D(year, 3, 17)),
        ("easter", easter(year) - 13 * day, easter(year)),
        ("july4", D(year, 6, 21), D(year, 7, 4)),
        ("halloween", D(year, 10, 1), D(year, 10, 31)),
        ("thanksgiving", D(year, 11, 1), tg),
        ("christmas", tg + day, D(year, 12, 25)),
        ("newyear", D(year, 12, 26), D(year + 1, 1, 1)),
    ]


def calendar_show(now):
    """(show, protected): the holiday whose window holds `now` (the one whose day comes first
    when windows overlap), or "everyday". Protected on the holiday's own day, and on New
    Year's Eve from 11 PM for the midnight countdown."""
    today = now.date()
    current = [w for w in windows(today.year - 1) + windows(today.year) if w[1] <= today <= w[2]]
    if not current:
        return "everyday", False
    show, _, own_day = min(current, key=lambda w: w[2])
    eve = show == "newyear" and (today.month, today.day) == (12, 31) and now.hour >= 23
    return show, today == own_day or eve


@dataclasses.dataclass
class Birthday:
    name: str
    month: int
    day: int
    year: int = None


def parse_birthdays(text):
    """Lines of "MM-DD NAME" or "YYYY-MM-DD NAME"; anything else is skipped."""
    items = []
    for line in text.splitlines():
        when, _, name = line.strip().partition(" ")
        name = name.strip()
        parts = when.split("-")
        try:
            numbers = [int(p) for p in parts]
            if not name or len(parts) not in (2, 3):
                continue
            year = numbers[0] if len(parts) == 3 else None
            month, day = numbers[-2:]
            D(year or 2000, month, day)  # 2000 was a leap year, so 29 February is fine
        except ValueError:
            continue
        items.append(Birthday(name, month, day, year))
    return items


def format_birthdays(items):
    lines = []
    for b in items:
        when = f"{b.month:02d}-{b.day:02d}"
        lines.append(f"{b.year:04d}-{when} {b.name}" if b.year else f"{when} {b.name}")
    return "".join(line + "\n" for line in lines)


def _leap(year):
    return year % 4 == 0 and (year % 100 != 0 or year % 400 == 0)


def _on(b, day):
    """True when birthday `b` is celebrated on `day` (29 February on the 28th in other years)."""
    if (b.month, b.day) == (day.month, day.day):
        return True
    return (b.month, b.day) == (2, 29) and (day.month, day.day) == (2, 28) and not _leap(day.year)


def birthdays_on(day, items):
    return [b for b in items if _on(b, day)]


def _next_birthdays(today, items):
    """(date, people) for the next day on or after `today` with a birthday, or (None, [])."""
    for ahead in range(367):
        day = today + datetime.timedelta(days=ahead)
        people = birthdays_on(day, items)
        if people:
            return day, people
    return None, []


def join_names(names):
    if len(names) < 2:
        return "".join(names)
    return ", ".join(names[:-1]) + " & " + names[-1]


def available(show):
    """The show itself if the sign has it, else Everyday, else Thanksgiving."""
    ids = [i for i, _ in SHOWS]
    if show in ids:
        return show
    return "everyday" if "everyday" in ids else "thanksgiving"


def decide(now, birthdays, pick):
    """What the sign plays now: {"show", "why", "names", "turning"}."""
    today = now.date()
    todays = birthdays_on(today, birthdays)
    if pick in [i for i, _ in SHOWS]:
        show, why = pick, "picked"
    else:
        show, why = ("birthday" if todays else calendar_show(now)[0]), "auto"
    show = available(show)

    names = turning = ""
    if show == "birthday":
        day, people = (today, todays) if todays else _next_birthdays(today, birthdays)
        names = join_names([p.name for p in people])
        turning = " ".join(f"{p.name} TURNS {day.year - p.year}!" for p in people if p.year)
    return {"show": show, "why": why, "names": names, "turning": turning}


def today_text(decision):
    """The today file's contents, one key=value per line."""
    keys = ("show", "why", "names", "turning")
    return "".join(f"{k}={' '.join(str(decision.get(k, '')).split())}\n" for k in keys)
