CFLAGS=-Wall -Werror -g -O0
INSTALL_DIR=~/.bin/

all: chisquared

.PHONY: all test

chisquared: main.c
	$(CC) $< -o $@ $(CFLAGS)

test: chisquared
	./test

install:
	[ -d $(INSTALL_DIR) ] && cp chisquared $(INSTALL_DIR)
