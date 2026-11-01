#!/bin/sh
# One-time setup on the Pi: turns on the SPI bus, and installs the compiler if it's missing.
set -e
sudo raspi-config nonint do_spi 0
command -v g++ >/dev/null && command -v make >/dev/null || {
  sudo apt-get update
  sudo apt-get install -y g++ make
}
