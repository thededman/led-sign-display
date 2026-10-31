# Halloween LED Sign

A 64×8 LED sign made of eight SainSmart MAX7219 8×8 modules in a row, run by an ESP32 DevKit V1
(or an Arduino UNO). It plays a one-minute Halloween show on a loop:

1. **WELCOME!** fades in pixel by pixel, then Pac-Man chomps through it with four ghosts behind him.
2. **Power pellet**: Pac-Man eats a row of dots, hits the power pellet, turns on the ghosts and
   eats all four (200, 400, 800, 1600) while their eyes fly home.
3. **HAPPY** drops in, then **HALLOWEEN**, and Pac-Man eats it from the right.
4. Two **jokes** scroll by with Pac-Man and the ghosts chasing them, and he gobbles the end.
5. Between jokes: a ghost that says **BOO!**, flapping **bats**, or blinking **jack-o'-lanterns**.
6. **Finale**: a shaking, glitching **BOO!**, then eyes blinking in the dark.

## Wiring

The modules chain together: each module's **OUT** header feeds the next module's **IN**. The
board connects to the **IN** header of the first module, which is labeled VCC, GND, DIN, CS, CLK.
On this sign, the first module is the left one seen from the front (the right one from the back).

### ESP32 DevKit V1

| Display IN | ESP32 pin |
|---|---|
| VCC | VIN |
| GND | GND |
| DIN | D23 |
| CS  | D5  |
| CLK | D18 |

The display wires end in male pins and so does the ESP32, so use female-to-female jumpers or
a breadboard. The ESP32 sends 3.3 V signals, a bit under the MAX7219's rated 3.5 V; it
usually works, and the code clocks the data slowly to help. If modules show garbage, use
shorter wires, add a 74HCT125 level shifter, or switch to the UNO.

### Arduino UNO (fallback)

| Display IN | UNO pin |
|---|---|
| VCC | 5V |
| GND | GND |
| DIN | 11 |
| CS  | 10 |
| CLK | 13 |

## Power

Plug a phone USB power bank into the board's USB port. At the default brightness the whole
sign draws roughly 150–300 mA, so a 10,000 mAh bank lasts all night. A 9 V battery on the
UNO's barrel jack works too, but only for about two hours.

Press the ESP32's **BOOT** button to step through brightness levels (0, 2, 5, 9, 15). The
starting level is `BRIGHTNESS` in `src/config.h`.

## Building and uploading

The project uses [PlatformIO](https://platformio.org). Install it with the VS Code
PlatformIO extension or `pip install platformio`, then from this folder:

```bash
pio run -e finder -t upload
```

That's the **orientation finder**. Module makers wire their LEDs differently, so the sign cycles
through 16 layouts, 4 seconds each, showing for example `7 READ ME`. Exactly one reads
normally, left to right. Put that number in `src/config.h` as `ORIENTATION`, then upload
the show:

```bash
pio run -t upload
```

For the UNO, use `pio run -e uno -t upload`. The orientation number stays the same, because it
belongs to the display, not the board.

If the ESP32 upload stalls at `Connecting...`, hold the **BOOT** button until it starts writing.

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
| `src/main.cpp` | Board setup, BOOT button, orientation finder |
