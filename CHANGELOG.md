# Changelog

All notable changes to the sign are listed here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and versions follow
[Semantic Versioning](https://semver.org/).

## [Unreleased]

### Changed
- The sign now plays a birthday show for Paul and Larry: names doing the wave, balloons,
  confetti, a cake whose candles blow out and then get eaten (NOM! BURP!), fireworks, a
  present, a party blower, a sing-along, and eighteen birthday jokes, each followed by a
  parade of cupcake, present, party hat and balloon. The Halloween show (with Pac-Man) is
  still in release 1.0.0.

### Added
- Runs on a Raspberry Pi Zero W too, with a phone page over Wi-Fi: type a message for the
  sign, change the brightness, skip a scene or pause. Starts by itself at power-on.
- 3D-printable bat-shaped door holder: the sign is the bat's body, the wings hide the UNO and
  the power bank, and it hangs with Command strips. Prints in two halves on a 225 mm bed, with
  a quick fit-test piece to print first.
- Plain 3D-printable case that just encloses the display, with a bay for the sideways cable
  plug, a slot for its wires and Command strip pads at the ends. Both halves print together
  in about an hour.

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

[Unreleased]: https://github.com/thededman/led-sign-display/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/thededman/led-sign-display/releases/tag/v1.0.0
