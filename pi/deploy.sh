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
# The birthdays file: copy it here whenever the Pi has one (even an empty one, so a birthday
# removed on the phone page stays removed); put the copy back only if the Pi has none at all
set +e
ssh "$HOST" 'test -e /var/lib/led-sign/birthdays'
found=$?
set -e
case $found in
  0) mkdir -p "$BACKUP" && chmod 700 "$BACKUP"
     ssh "$HOST" 'cat /var/lib/led-sign/birthdays' > "$BACKUP/birthdays" ;;
  1) ;;
  *) echo "Can't reach $HOST" >&2; exit 1 ;;
esac
COPYFILE_DISABLE=1 tar czf - --no-xattrs --exclude pi/ledsign-term --exclude __pycache__ src pi |
  ssh "$HOST" 'rm -rf ~/led-sign && mkdir -p ~/led-sign && tar xzf - -C ~/led-sign'
[ -n "$SETUP" ] && ssh -t "$HOST" 'sh ~/led-sign/pi/setup.sh'
ssh -t "$HOST" 'cd ~/led-sign/pi && make && sudo make install'
if [ "$found" = 1 ] && [ -s "$BACKUP/birthdays" ]; then
  ssh "$HOST" 'cat > /var/lib/led-sign/birthdays' < "$BACKUP/birthdays"
  echo "Put back the birthdays saved in $BACKUP/birthdays"
fi
