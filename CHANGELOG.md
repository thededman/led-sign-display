# Changelog

All notable changes to the sign are listed here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and versions follow
[Semantic Versioning](https://semver.org/).

## [Unreleased]

## [1.3.0] - 2026-11-22

### Added
- A build guide in the README for anyone making their own sign: the parts, wiring diagrams
  for the Arduino UNO and the Raspberry Pi Zero W, setting up each one, finding your
  display's orientation, making the show your own, and the 3D-printed case and holder, with
  pictures of every show, the scores screen and the phone page.
- The Pi can run the orientation finder (`ledsign --finder`), so a sign built on a Pi alone
  can find its display's layout without an Arduino.
- An MIT license, so others can use, change and share the project.
- Tools for making those pictures: `tools/preview/screenshots.py`, sample scores in the
  preview (`PREVIEW_SCORES`), and the 3D viewer opens a chosen model and view from its
  address (`viewer.html?model=case&view=back`).

### Changed
- The phone page's message box suggests WELCOME! instead of HAPPY BIRTHDAY!.
- `pi/deploy.sh` sends to your own user name at `ledsign.local` unless you name another.

## [1.2.0] - 2026-11-22

### Changed
- The sign now plays a Thanksgiving show instead of the birthday one: falling leaves, a
  strutting turkey (GOBBLE!), a pumpkin pie that gets eaten (NOM! BURP!), a field goal,
  RUN, TURKEY!, a THANKFUL FOR... list, and eighteen Thanksgiving jokes, each followed by a
  parade of turkey, pilgrim hat, leaf and football. NFL scores still show every other screen
  on the Pi. The birthday show is still in release 1.1.0.

## [1.1.1] - 2026-11-22

### Added
- Issue and pull request templates on GitHub, so every request is tracked from the ask to
  the release.

## [1.1.0] - 2026-11-22

### Changed
- The sign now plays a birthday show for Paul and Larry: names doing the wave, balloons,
  confetti, a cake whose candles blow out and then get eaten (NOM! BURP!), fireworks, a
  present, a party blower, a sing-along, and eighteen birthday jokes, each followed by a
  parade of cupcake, present, party hat and balloon. The Halloween show (with Pac-Man) is
  still in release 1.0.0.

### Added
- On the Pi, every other screen shows today's NFL scores (live, final and upcoming), three
  games at a time with a bouncing football. The phone page switches them on or off, lists
  today's games and picks a favorite team whose game shows every time.
- Runs on a Raspberry Pi Zero W too, with a phone page over Wi-Fi: type a message for the
  sign, change the brightness, skip a scene or pause. Starts by itself at power-on.
- 3D-printable bat-shaped door holder: the sign is the bat's body, the wings hide the UNO and
  the power bank, and it hangs with Command strips. Prints in two halves on a 225 mm bed, with
  a quick fit-test piece to print first.
- Plain 3D-printable case that just encloses the display, with a bay for the sideways cable
  plug, a slot for its wires and Command strip pads at the ends. Both halves print together
  in about an hour.

### Known issues
- The reprinted right half of the plain case, with the cable bay, hasn't been test-fitted yet.

## [1.0.0] - 2026-10-31

### Added
- One-minute Halloween show on a loop: WELCOME!, Pac-Man and the ghosts, the power-pellet
  chase, HAPPY HALLOWEEN, ten rotating jokes, BOO! ghost, bats, jack-o'-lanterns, and a
  glitchy BOO! finale with eyes in the dark.
- Runs on an Arduino UNO, powered from a USB power bank for a whole evening.
- Wiring test and orientation finder for working out how a display is wired.
- Mac preview that plays the show on a picture of the sign.
- Wiring and setup instructions in the README.

### Known issues
- The ESP32 DevKit V1 build can't drive this display without a 3.3 V → 5 V level shifter.

[Unreleased]: https://github.com/thededman/led-sign-display/compare/v1.3.0...HEAD
[1.3.0]: https://github.com/thededman/led-sign-display/compare/v1.2.0...v1.3.0
[1.2.0]: https://github.com/thededman/led-sign-display/compare/v1.1.1...v1.2.0
[1.1.1]: https://github.com/thededman/led-sign-display/compare/v1.1.0...v1.1.1
[1.1.0]: https://github.com/thededman/led-sign-display/compare/v1.0.0...v1.1.0
[1.0.0]: https://github.com/thededman/led-sign-display/releases/tag/v1.0.0
