#!/bin/sh
# From your computer: copies the sign program to the Pi, builds it there and (re)starts it.
# It asks for your Pi password when it gets to the sudo steps. It also keeps a copy of the
# birthdays from the phone page in ~/.led-sign/, and puts them back on a freshly set-up Pi.
#   pi/deploy.sh                      to you@ledsign.local (your user name on this computer)
#   pi/deploy.sh pi@otherhost.local   somewhere else
#   pi/deploy.sh --setup              first time: also turn on SPI and install the compiler
set -e
SETUP=
if [ "$1" = "--setup" ]; then SETUP=1; shift; fi
HOST=${1:-$USER@ledsign.local}
cd "$(dirname "$0")/.."
BACKUP="$HOME/.led-sign"
saved=$(ssh "$HOST" 'cat /var/lib/led-sign/birthdays 2>/dev/null' || true)
if [ -n "$saved" ]; then
  mkdir -p "$BACKUP" && chmod 700 "$BACKUP"
  printf '%s\n' "$saved" > "$BACKUP/birthdays"
fi
COPYFILE_DISABLE=1 tar czf - --no-xattrs --exclude pi/ledsign-term --exclude __pycache__ src pi |
  ssh "$HOST" 'rm -rf ~/led-sign && mkdir -p ~/led-sign && tar xzf - -C ~/led-sign'
[ -n "$SETUP" ] && ssh -t "$HOST" 'sh ~/led-sign/pi/setup.sh'
ssh -t "$HOST" 'cd ~/led-sign/pi && make && sudo make install'
if [ -z "$saved" ] && [ -s "$BACKUP/birthdays" ]; then
  ssh "$HOST" 'cat > /var/lib/led-sign/birthdays' < "$BACKUP/birthdays"
  echo "Put back $(wc -l < "$BACKUP/birthdays" | tr -d ' ') birthdays from $BACKUP/birthdays"
fi
