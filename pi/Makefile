# The sign program for a Raspberry Pi.
#   make && sudo make install    on the Pi: build, then install and start the services
#   make term                    anywhere: a version that draws the sign in the terminal

CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra
CXXFLAGS += -I../src
SHOW = ../src/show.cpp
DEPS = main.cpp $(SHOW) $(wildcard ../src/*.h)
SIGN_USER ?= $(SUDO_USER)

ledsign: $(DEPS) matrix_spidev.cpp
	$(CXX) $(CXXFLAGS) -o $@ main.cpp matrix_spidev.cpp $(SHOW)

ledsign-term: $(DEPS) matrix_term.cpp
	$(CXX) $(CXXFLAGS) -o $@ main.cpp matrix_term.cpp $(SHOW)

term: ledsign-term

install: ledsign
	install -m 755 ledsign /usr/local/bin/ledsign
	install -d /opt/led-sign/web
	install -m 644 web/index.html /opt/led-sign/web/
	install -m 755 web/server.py /opt/led-sign/web/
	install -m 755 scores.py /opt/led-sign/
	install -d -o $(SIGN_USER) /var/lib/led-sign
	for unit in led-sign led-sign-web led-sign-scores; do \
	  sed "s/@USER@/$(SIGN_USER)/" systemd/$$unit.service > /etc/systemd/system/$$unit.service; \
	done
	systemctl daemon-reload
	systemctl enable led-sign led-sign-web led-sign-scores
	systemctl restart led-sign led-sign-web led-sign-scores

clean:
	rm -f ledsign ledsign-term

.PHONY: term install clean
