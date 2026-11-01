#!/bin/sh
# One-time setup on the Pi: turns on the SPI bus and installs the compiler.
set -e
sudo raspi-config nonint do_spi 0
sudo apt-get update
sudo apt-get install -y g++ make
