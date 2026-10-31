#!/usr/bin/env python3
"""A plain case for the LED sign: a frame around the eight modules, open at the back, with a
Command strip pad at each end and a slot under the right end for the wires.

    pip install manifold3d numpy
    python3 holder/case.py

Writes holder/stl/case-left.stl and case-right.stl (print both at once; they fit side by side
on a 225 mm bed) and uses the same pins.stl as the bat holder. The front face is at z = 0, so
both print face down with no supports. Looking at the front you look along +z, so your right
is -x: the wire slot is at -x, under the end where the sign's cable plugs in.
"""
from manifold3d import JoinType, Manifold

from holder import FIT, LIP, MODULE, MODULES, OUT, PIN_D, PIN_FIT, PIN_L, rect, save_stl

PLATE = 1.2          # front plate
WALL = 1.6           # side walls
MATRIX = 6.5         # how deep the front of the case hugs the LED blocks (they're about 8 mm)
PCB_ROOM = 6.0       # extra room above and below, behind the LED blocks, for the circuit boards
END_ROOM = 3.0       # extra room past each end, behind the LED blocks, for the cable plug
DEPTH = 30.0         # front face to door: clears the module backs and their jumpers
PAD = 20.0           # Command strip pads at the ends (strips are 16 x 46 mm)
SHELL = 1.2          # walls of the hollow pads
SLOT = (36.0, 12.0)  # wire slot under the right end: length, and how far back from the front it starts

L = MODULES * MODULE + 2 * FIT   # LED block pocket length
H = MODULE + 2 * FIT             # LED block pocket height
X0 = L / 2
HB = H / 2 + PCB_ROOM            # half height of the room behind the LED blocks
YO = HB + WALL                   # outer half height
XE = X0 + END_ROOM + WALL        # outer end of the frame, where the pads start
PIN_Y, PIN_Z = H / 2 + 3.4, PLATE + 3.0  # pins sit in the thick ribs beside the LED blocks


def hollow(shape):
    inside = shape.offset(-SHELL, JoinType.Miter).extrude(DEPTH - PLATE - SHELL)
    return shape.extrude(DEPTH) - inside.translate([0, 0, PLATE])


def case():
    frame = rect(-XE, -YO, XE, YO)
    window = rect(-X0 + LIP, -H / 2 + LIP, X0 - LIP, H / 2 - LIP)
    blocks = rect(-X0, -H / 2, X0, H / 2)
    behind = rect(-X0 - END_ROOM, -HB, X0 + END_ROOM, HB)

    hug = blocks.offset(SHELL, JoinType.Miter) - blocks           # thin walls around the LED blocks
    bosses = rect(-8, H / 2, 8, HB) + rect(-8, -HB, 8, -H / 2)   # solid where the pins go

    body = (frame - window).extrude(PLATE)
    body += (hug + bosses).extrude(PLATE + MATRIX)
    body += (frame - behind).extrude(DEPTH)
    body += hollow(rect(XE, -YO, XE + PAD, YO)) + hollow(rect(-XE - PAD, -YO, -XE, YO))
    slot = Manifold.cube([SLOT[0], WALL + 2, DEPTH]).translate(
        [-X0 - END_ROOM, -YO - 1, SLOT[1]])
    return body - slot


def pin_holes():
    radius = (PIN_D + PIN_FIT) / 2
    out = Manifold()
    for y in (PIN_Y, -PIN_Y):
        out += Manifold.cylinder(PIN_L + 0.6, radius, radius, 48).rotate([0, 90, 0]).translate([0, y, PIN_Z])
    return out


def main():
    whole = case()
    holes = pin_holes()
    left = whole.trim_by_plane([1, 0, 0], 0) - holes
    right = whole.trim_by_plane([-1, 0, 0], 0) - holes.mirror([1, 0, 0])
    for name, part in (("case-left", left), ("case-right", right)):
        save_stl(part, OUT / f"{name}.stl")
        (x0, y0, z0, x1, y1, z1) = part.bounding_box()
        grams = part.volume() / 1000 * 1.24
        print(f"{name:10} {x1 - x0:6.1f} x {y1 - y0:5.1f} x {z1 - z0:4.1f} mm, about {grams:3.0f} g of PLA")


if __name__ == "__main__":
    main()
