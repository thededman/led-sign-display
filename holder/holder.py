#!/usr/bin/env python3
"""Door holder for the Halloween LED sign: a bat-winged frame that prints in two halves.

    pip install manifold3d numpy
    python3 holder/holder.py

Writes the STL files to holder/stl. Sizes are in mm. The front face is at z = 0, so the parts print
face down with no supports; the door side is at z = DEPTH. Standing in front of the sign you look
along +z, so your right is -x: the right wing (UNO, where the sign's cable plugs in) is at -x.

The modules drop into the frame from the back and rest against a thin lip. The right wing hides
the UNO, the left wing the power bank; each wing has a post for two Command strips, and the bat
head and feet at the centre take the four loose pins that line the halves up (and strips too).
"""
import math
import pathlib
import struct

import numpy as np
from manifold3d import CrossSection, JoinType, Manifold

# The sign
MODULE = 32.0          # one 8x8 module, square
MODULES = 8
FIT = 0.2              # clearance around the modules, each side
LIP = 0.8              # how far the frame overlaps the edges of the LED faces

# The holder
PLATE = 1.2            # front plate thickness
FRAME = 8.6            # depth of the walls that hug the modules, from the front
WALL = 2.4             # thickness of those walls
RIM = 1.6              # raised edge around the wings, head and feet
RIM_DEPTH = 6.0
SHELL = 1.2            # walls of the hollow posts, head and feet
CARVE = 0.6            # depth of the eyes and wing bones carved into the front
DEPTH = 34.0           # front face to door: clears the module backs and the UNO with its wires
PAD = 17.0             # width of the Command strip posts (strips are 16 mm wide)
NOTCH = (20.0, 16.0)   # cable notch in the posts: height, depth from the door side
UNO = (54.4, 69.6)     # UNO 53.4 x 68.6 standing up, plus clearance
BANK = (66.0, 101.0)   # power bank 65 x 100 standing up, plus clearance
BRACKET = 14.0         # leg length of the corner brackets that hold the UNO and power bank
PIN_D, PIN_L, PIN_FIT = 5.0, 6.0, 0.4

L = MODULES * MODULE + 2 * FIT   # module pocket length
H = MODULE + 2 * FIT             # module pocket height
X0 = L / 2                       # where the pocket ends and a wing begins
BAND_Y = H / 2 + WALL            # outer edge of the band
CONTENT_Y = BANK[1] / 2 + RIM    # wings keep this much height clear for the pockets

OUT = pathlib.Path(__file__).parent / "stl"


def poly(points):
    return CrossSection([points])


def rect(x0, y0, x1, y1):
    return CrossSection.square([x1 - x0, y1 - y0]).translate([x0, y0])


def bite(p, q, depth):
    """A circle through p and q that bites a scallop `depth` deep into a counter-clockwise
    outline whose edge runs from p to q."""
    (px, py), (qx, qy) = p, q
    c = math.hypot(qx - px, qy - py)
    r = (c * c / 4 + depth * depth) / (2 * depth)
    nx, ny = (qy - py) / c, -(qx - px) / c  # outward normal
    cx, cy = (px + qx) / 2 + nx * (r - depth), (py + qy) / 2 + ny * (r - depth)
    return CrossSection.circle(r, 128).translate([cx, cy])


def wing():
    """The right wing in its own coordinates: u = x - X0, v = y."""
    bottom = [(0, -56), (32, -66), (60, -70), (87, -56)]
    tip = (88, 106)  # 128.2 + 88 + the 6 mm pins keeps each half inside a 225 mm bed
    outline = poly(bottom + [(87, 56), tip, (44, 79), (40, 88), (35, 77), (0, 58)])
    for (p, q), depth in zip(zip(bottom, bottom[1:]), (7, 9, 9)):
        outline -= bite(p, q, depth)
    return outline - bite((87, 56), tip, 8)


def bones():
    """Finger bones carved into the front of the right wing: the arm up to the wrist, then
    fingers out to each point of the scalloped edge."""
    wrist = (38, 74)
    lines = [((4, 30), wrist), (wrist, (80, 96)), (wrist, (84, 54)), (wrist, (83, -52)),
             (wrist, (58, -64)), (wrist, (31, -60))]
    out = CrossSection()
    for (a, b) in lines:
        out += (CrossSection.circle(1.1, 24).translate(a) + CrossSection.circle(1.1, 24).translate(b)).hull()
    return out ^ wing().offset(-3, JoinType.Round)


def head():
    return poly([(-24, 17), (24, 17), (24, 36), (18, 52), (12, 40),
                 (-12, 40), (-18, 52), (-24, 36)])


def feet():
    return poly([(-24, -36), (-11, -36), (-8, -43), (-5, -36), (5, -36), (8, -43),
                 (11, -36), (24, -36), (24, -17), (-24, -17)])


def pocket(size):
    """Interior of a wing pocket, against the Command strip post, centred top to bottom."""
    w, h = size
    u0 = PAD + RIM
    return u0, -h / 2, u0 + w, h / 2


def brackets(u0, v0, u1, v1):
    """L-shaped brackets at the four corners of a pocket, standing to just short of the door."""
    t, k = RIM, BRACKET
    parts = CrossSection()
    for cu, su in ((u0, -1), (u1, 1)):
        for cv, sv in ((v0, -1), (v1, 1)):
            ou, ov = cu + su * t, cv + sv * t  # outer corner
            parts += rect(min(cu - su * k, ou), min(cv, ov), max(cu - su * k, ou), max(cv, ov))
            parts += rect(min(cu, ou), min(cv - sv * k, ov), max(cu, ou), max(cv - sv * k, ov))
    return parts.extrude(DEPTH - 0.5)


def hollow(shape):
    """A block from the front plate to the door, hollow inside to save plastic and time."""
    inside = shape.offset(-SHELL, JoinType.Miter).extrude(DEPTH - PLATE - SHELL)
    return shape.extrude(DEPTH) - inside.translate([0, 0, PLATE])


def wing_parts(pocket_size):
    """Command strip post and pocket brackets for the right wing, in sign coordinates."""
    post = hollow(rect(RIM, -CONTENT_Y, RIM + PAD, CONTENT_Y))
    notch = Manifold.cube([PAD + 2, NOTCH[0], NOTCH[1] + 1]).translate(
        [RIM - 1, -NOTCH[0] / 2, DEPTH - NOTCH[1]])
    return ((post - notch) + brackets(*pocket(pocket_size))).translate([X0, 0, 0])


def sign_holder():
    right = wing().translate([X0, 0])
    outline = (rect(-X0 - WALL, -BAND_Y, X0 + WALL, BAND_Y) + right + right.mirror([1, 0])
               + head() + feet())
    window = rect(-X0 + LIP, -H / 2 + LIP, X0 - LIP, H / 2 - LIP)
    frame = rect(-X0 - WALL, -BAND_Y, X0 + WALL, BAND_Y) - rect(-X0, -H / 2, X0, H / 2)
    rim = outline - outline.offset(-RIM, JoinType.Miter)

    body = (outline - window).extrude(PLATE)
    body += frame.extrude(FRAME) + rim.extrude(RIM_DEPTH)
    body += hollow(head()) + hollow(feet())
    body += (rect(-8, 23, 8, 33) + rect(-8, -32, 8, -22)).extrude(DEPTH)  # solid around the pins
    body += wing_parts(UNO).mirror([1, 0, 0])                # right wing: UNO
    body += wing_parts(BANK)                                 # left wing: power bank
    eyes = CrossSection.circle(2.5, 48).translate([-7, 29]) + CrossSection.circle(2.5, 48).translate([7, 29])
    right_bones = bones().translate([X0, 0])
    carving = eyes + right_bones + right_bones.mirror([1, 0])
    return body - carving.extrude(CARVE)


def pin_holes(radius, length):
    """The four alignment pin holes at the centre joint, running to +x from x = 0."""
    out = Manifold()
    for y in (28.0, -27.0):
        for z in (12.0, 24.0):
            out += Manifold.cylinder(length, radius, radius, 48).rotate([0, 90, 0]).translate([0, y, z])
    return out


def halves():
    """The left and right halves, as seen from the front."""
    whole = sign_holder()
    holes = pin_holes((PIN_D + PIN_FIT) / 2, PIN_L + 0.6)
    left = whole.trim_by_plane([1, 0, 0], 0) - holes
    right = whole.trim_by_plane([-1, 0, 0], 0) - holes.mirror([1, 0, 0])
    return left, right


def pins():
    """Four loose pins, printed standing up so they're strong."""
    out = Manifold()
    for i in range(4):
        out += Manifold.cylinder(2 * PIN_L, PIN_D / 2, PIN_D / 2, 48).translate([i * 10.0, 0, 0])
    return out


def fit_test():
    """One module's worth of frame with the end lip: print it first and try it on the sign."""
    x0 = X0 - MODULE - 6
    window = rect(x0 - 1, -H / 2 + LIP, X0 - LIP, H / 2 - LIP)
    frame = rect(x0, -BAND_Y, X0 + WALL, BAND_Y) - rect(x0 - 1, -H / 2, X0, H / 2)
    return (rect(x0, -BAND_Y, X0 + WALL, BAND_Y) - window).extrude(PLATE) + frame.extrude(FRAME)


def save_stl(part, path):
    mesh = part.to_mesh()
    verts = np.asarray(mesh.vert_properties)[:, :3]
    tris = verts[np.asarray(mesh.tri_verts)]
    normals = np.cross(tris[:, 1] - tris[:, 0], tris[:, 2] - tris[:, 0])
    normals /= np.linalg.norm(normals, axis=1, keepdims=True) + 1e-12
    data = np.zeros(len(tris), dtype=[("n", "<f4", 3), ("v", "<f4", (3, 3)), ("a", "<u2")])
    data["n"], data["v"] = normals, tris
    with open(path, "wb") as f:
        f.write(b"Halloween sign holder".ljust(80, b" "))
        f.write(struct.pack("<I", len(tris)))
        f.write(data.tobytes())


def main():
    OUT.mkdir(exist_ok=True)
    left, right = halves()
    parts = (("left-half-power-bank", left), ("right-half-uno", right), ("pins", pins()),
             ("fit-test", fit_test()))
    for name, part in parts:
        save_stl(part, OUT / f"{name}.stl")
        (x0, y0, z0, x1, y1, z1) = part.bounding_box()
        grams = part.volume() / 1000 * 1.24
        print(f"{name:20} {x1 - x0:6.1f} x {y1 - y0:6.1f} x {z1 - z0:5.1f} mm, about {grams:3.0f} g of PLA")


if __name__ == "__main__":
    main()
