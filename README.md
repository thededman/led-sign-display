# LED Sign

A 64×8 LED sign made of eight SainSmart MAX7219 8×8 modules in a row, run by an Arduino UNO.
It plays a birthday show for Paul and Larry on a loop, about 80 seconds a pass:

1. **Confetti**, then **HAPPY** and **BIRTHDAY** drop in, and BIRTHDAY bursts into confetti.
2. The **names** do the wave in double-width letters: **PAUL!** with balloons rising up the
   sides, then **LARRY!** in confetti.
3. A **cake** with two flickering candles: **WISH!**, the candles blow out in puffs of smoke,
   **HOORAY!**, then it's eaten a slice at a time (**NOM!**), and a big **BURP!**.
4. Two **jokes** scroll by (eighteen in all), each followed by a little parade: a cupcake
   with a flickering candle, a present, a party hat and a balloon.
5. Between jokes: **balloons**, **fireworks**, a **present** that pops open (SURPRISE!), a
   **party blower** (TOOT!), or a **sing-along** between two bouncing music notes.
6. **Finale**: fireworks around HAPPY BIRTHDAY PAUL & LARRY!

The Halloween show from Oct 31, 2026 (with Pac-Man) is release `v1.0.0`. To bring it back:
`git checkout v1.0.0 -- src/messages.h src/show.cpp src/sprites.h`, then upload.

## Wiring

The modules chain together: each module's **OUT** header feeds the next module's **IN**. The
board connects to the **IN** header of the first module, labeled VCC, GND, DIN, CS, CLK. On
this sign the cable enters at the **right-hand end**, seen from the front.

| Display IN | UNO pin |
|---|---|
| VCC | 5V |
| GND | GND (the one next to pin 13 is handiest) |
| DIN | 11 |
| CS  | 10 |
| CLK | 13 |

With the UNO's USB port on the left, pins 13, 11 and 10 are at the left end of the long top
header; 5V is on the bottom header between 3.3V and GND.

### ESP32 (not working yet)

The code also builds for an ESP32 DevKit V1 (DIN → D23, CS → D5, CLK → D18, VCC → VIN), but
on this sign the ESP32's 3.3 V signals were too weak: the display stayed dark. Every ESP32 has
3.3 V pins, so a newer one won't help on its own. Two ways to get Wi-Fi:

- **Arduino UNO R4 WiFi** (about $27.50): its main chip runs at 5 V like the UNO, so it uses
  the same wiring and pins, powers from USB-C, and has an ESP32-S3 on board for Wi-Fi.
- An ESP32 plus a **SN74AHCT125** level shifter (about $1.50): VCC to 5 V, the /OE pins to
  GND, ESP32 pins into the A inputs and the Y outputs to DIN, CLK and CS.

## Power

Plug a phone USB power bank into the UNO's USB port. At the default brightness the whole
sign draws roughly 150–300 mA, so a 10,000 mAh bank lasts all night. A 9 V battery on the
barrel jack works too, but only for about two hours.

Brightness is `BRIGHTNESS` in `src/config.h` (0–15, default 2).

## Building and uploading

The project uses [PlatformIO](https://platformio.org). Install it with the VS Code
PlatformIO extension or `pip install platformio`, plug the UNO in, then from this folder:

```bash
pio run -t upload
```

### If the sign shows garbage

Two test programs find out how a display is wired. You only need them if the wiring or the
modules change; this sign's answer is already in `src/config.h` (`ORIENTATION 5`).

**Wiring test.** Tries the three signal wires in all six orders, 5 seconds each, each with its
own shape: square outline (wired as in the table), X (DIN and CLK swapped), plus (DIN and
CS swapped), centre dot (CLK and CS swapped), two squares, four corners. The order where all
8 modules show the same clean shape is the right one; the serial monitor names the pins.

```bash
pio run -e uno-wiring -t upload
```

**Orientation finder.** Module makers wire their LEDs differently. The finder tries the 8
possibilities, 4 seconds each: every module shows an F with a column of dots beside it.
When the F's look like normal F's, count the dots. `ORIENTATION` is `(dots − 1) × 2`, plus 1
if the cable enters at the right-hand end of the sign.

```bash
pio run -e uno-finder -t upload
```

## Plain case (3D printed)

The quick option: a black frame around the display only, open at the back, with a Command
strip pad at each end. At the right-hand end a bay holds the sideways cable plug, and a slot
underneath lets the wires out. The UNO and power bank stay outside.

Print `holder/stl/case-left.stl` and `case-right.stl` together (load both into Cura and it
places them side by side) plus `pins.stl`: about 1–1.5 hours and 64 g. Same Cura settings as
the bat holder below: face down as exported, no supports. Then:

1. Push two pins into one half (a drop of super glue holds them) and press the halves together.
2. Lay the case face down and lower the LED strip in from the back, LEDs down, with the plug
   in the bay. Feed the wires out through the slot under the bay, plug the blue jumper back in
   across the middle joint (the left four modules go dark without it), and tape across the
   module backs.
3. One Command strip on each end pad, and press it to the door for 30 seconds.

`python3 holder/case.py` rebuilds it; the sizes are at the top of the file.

## Door holder (3D printed)

A bat-shaped frame for hanging the sign with Command strips: the sign is the bat's body, the
right wing hides the UNO and the left wing hides the power bank. The files are in
`holder/stl`; all print **face down, exactly as exported, with no supports**.

| File | Print time on a Neptune 4 | Notes |
|---|---|---|
| `fit-test.stl` | ~10 min | Print this first |
| `pins.stl` | ~5 min | Four alignment pins; print with the fit test |
| `right-half-uno.stl` | ~1.5–2 h, ~63 g | 216 × 176 mm |
| `left-half-power-bank.stl` | ~1.5–2 h, ~62 g | 216 × 176 mm |

Cura settings: PLA, 0.2 mm layers, 3 walls, 15 % infill, supports off, build plate adhesion
Skirt or None (a brim won't fit beside a half on the 225 mm bed).

**Check the fit first.** Slide the fit test onto the left-hand end module from the left. The
module should sit snugly against the thin lip with every LED showing. Too tight or too loose:
change `FIT` in `holder/holder.py` (0.1 mm steps) and rebuild.

**Assembly**
1. Push the pins into one half (a drop of super glue holds them), then press the halves together.
2. Lay the frame face down. Lower the LED strip in from the back, LEDs down, with the cable end
   in the UNO half. Run a strip of tape across the module backs so they can't drop out.
3. Put the UNO in the right wing's pocket and the power bank in the left one (seen from the
   front). The cables run behind the sign through the notches in the two posts.
4. Command strips (small, 16 mm wide): two on each wing post, above and below the notch, and
   one each on the bat's head and feet. Press the whole thing to the door for 30 seconds.

**Changing it.** The design is a Python script with every size at the top:

```bash
pip install manifold3d numpy
python3 holder/holder.py
```

To look at it in 3D, run `python3 -m http.server --directory holder 8032` and open
http://localhost:8032/viewer.html.

## Changing the words

Everything the sign says is in `src/messages.h`. The birthday names are the first lines,
so the next birthday is a quick edit (names also appear in the finale, the song and a few
jokes). Use capital letters, digits and `! ? . , ' - & : / ( )`.

## Previewing on a Mac

`tools/preview/build.sh` runs the show on the Mac and writes
`tools/preview/build/preview.html`, which plays every frame on a picture of the sign. Use it
to try out new words or animations without the hardware.

## Files

| File | What it does |
|---|---|
| `src/config.h` | Pins, orientation, brightness |
| `src/messages.h` | The words |
| `src/show.cpp` | All the animations |
| `src/sprites.h` | Pac-Man, ghosts, bats, pumpkins (editable pixel art) |
| `src/font.h` | 5×7 letters |
| `src/matrix.cpp` | Talks to the MAX7219 chips |
| `src/main.cpp` | Board setup, orientation finder, wiring test |
