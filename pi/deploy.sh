#!/bin/sh
# From the Mac: copies the sign program to the Pi, builds it there and (re)starts it.
# It asks for your Pi password when it gets to the sudo steps.
#   pi/deploy.sh                      to david@ledsign.local
#   pi/deploy.sh pi@otherhost.local   somewhere else
#   pi/deploy.sh --setup              first time: also turn on SPI and install the compiler
set -e
SETUP=
if [ "$1" = "--setup" ]; then SETUP=1; shift; fi
HOST=${1:-david@ledsign.local}
cd "$(dirname "$0")/.."
COPYFILE_DISABLE=1 tar czf - --no-xattrs --exclude pi/ledsign-term --exclude __pycache__ src pi |
  ssh "$HOST" 'rm -rf ~/led-sign && mkdir -p ~/led-sign && tar xzf - -C ~/led-sign'
[ -n "$SETUP" ] && ssh -t "$HOST" 'sh ~/led-sign/pi/setup.sh'
ssh -t "$HOST" 'cd ~/led-sign/pi && make && sudo make install'
