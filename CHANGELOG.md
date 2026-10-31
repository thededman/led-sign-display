# Changelog

All notable changes to the sign are listed here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and versions follow
[Semantic Versioning](https://semver.org/).

## [Unreleased]

### Added
- 3D-printable bat-shaped door holder: the sign is the bat's body, the wings hide the UNO and
  the power bank, and it hangs with Command strips. Prints in two halves on a 225 mm bed, with
  a quick fit-test piece to print first.
- Plain 3D-printable case that just encloses the display, with a slot for the cable and
  Command strip pads at the ends. Both halves print together in about an hour.

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
