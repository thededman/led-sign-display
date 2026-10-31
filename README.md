# Halloween LED Sign

A 64×8 LED sign made of eight SainSmart MAX7219 8×8 modules in a row, run by an Arduino UNO.
It plays a one-minute Halloween show on a loop:

1. **WELCOME!** fades in pixel by pixel, then Pac-Man chomps through it with four ghosts behind him.
2. **Power pellet**: Pac-Man eats a row of dots, hits the power pellet, turns on the ghosts and
   eats all four (200, 400, 800, 1600) while their eyes fly home.
3. **HAPPY** drops in, then **HALLOWEEN**, and Pac-Man eats it from the right.
4. Two **jokes** scroll by with Pac-Man and the ghosts chasing them, and he gobbles the end.
5. Between jokes: a ghost that says **BOO!**, flapping **bats**, or blinking **jack-o'-lanterns**.
6. **Finale**: a shaking, glitching **BOO!**, then eyes blinking in the dark.

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
on this sign the ESP32's 3.3 V signals were too weak: the display stayed dark. It needs a
3.3 V → 5 V level shifter such as a 74HCT125 between the ESP32 and the display.

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

## Changing the words

Everything the sign says is in `src/messages.h`. Use capital letters, digits and
`! ? . , ' - & : /`. A joke that starts with `@` ends with Pac-Man burping.

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
