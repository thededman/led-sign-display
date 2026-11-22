#!/usr/bin/env python3
"""Turns moments of a preview into pictures of the sign, for the README.

    pip install pillow
    python3 tools/preview/screenshots.py tools/preview/build/preview.html docs/images \\
        turkey@15.0 show@12-24

name@seconds writes name.png, the sign at that moment; name@start-end writes name.gif,
an animation of that stretch. Times are in seconds from the start of the preview.
"""
import re
import sys
from bisect import bisect_right
from pathlib import Path

from PIL import Image, ImageDraw

WIDTH, ROWS = 64, 8
ON, OFF, BACKGROUND = (255, 66, 30), (48, 14, 8), (10, 10, 12)


def load(path):
    frames = [(int(ms), cols) for ms, cols in re.findall(r'\[(\d+),"([0-9a-f]{128})"\]', Path(path).read_text())]
    starts, t = [], 0
    for ms, _ in frames:
        starts.append(t)
        t += ms
    return frames, starts, t


def frame_at(frames, starts, ms):
    return frames[max(0, bisect_right(starts, ms) - 1)][1]


def draw(cols, cell):
    gap, pad = cell // 3, cell
    w = WIDTH * cell + 7 * gap + 2 * pad
    h = ROWS * cell + 2 * pad
    image = Image.new("RGB", (w, h), BACKGROUND)
    pen = ImageDraw.Draw(image)
    r = cell * 0.38
    for x in range(WIDTH):
        bits = int(cols[x * 2:x * 2 + 2], 16)
        cx = pad + x * cell + (x // 8) * gap + cell / 2
        for y in range(ROWS):
            cy = pad + y * cell + cell / 2
            pen.ellipse((cx - r, cy - r, cx + r, cy + r), fill=ON if bits >> y & 1 else OFF)
    return image


def main():
    if len(sys.argv) < 4:
        sys.exit(__doc__)
    frames, starts, total = load(sys.argv[1])
    out = Path(sys.argv[2])
    out.mkdir(parents=True, exist_ok=True)
    for shot in sys.argv[3:]:
        name, _, when = shot.partition("@")
        if "-" in when:
            start, end = (int(float(v) * 1000) for v in when.split("-"))
            images, durations = [], []
            for ms in range(start, min(end, total), 40):  # 25 frames a second
                cols = frame_at(frames, starts, ms)
                if images and cols == images[-1][0]:
                    durations[-1] += 40
                else:
                    images.append((cols, draw(cols, 8)))
                    durations.append(40)
            pictures = [picture for _, picture in images]
            pictures[0].save(out / f"{name}.gif", save_all=True, append_images=pictures[1:],
                             duration=durations, loop=0, optimize=True)
            print(f"{name}.gif: {len(pictures)} frames")
        else:
            draw(frame_at(frames, starts, int(float(when) * 1000)), 12).save(out / f"{name}.png", optimize=True)
            print(f"{name}.png")


if __name__ == "__main__":
    main()
