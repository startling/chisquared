CFLAGS=-Wall -Werror -g -O0
INSTALL_DIR=~/.bin/

all: chi^2

.PHONY: all test

chi^2: main.c frequencies.c digrams.c
	$(CC) $^ -o $@ $(CFLAGS)

test: chi^2
	./test

install:
	[ -d $(INSTALL_DIR) ] && cp chi^2 $(INSTALL_DIR)
