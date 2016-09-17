CFLAGS=-Wall -Werror -g -O3 -fsanitize=address -fsanitize=undefined -I. -fstack-protector -D_FORTIFY_SOURCE=2
INSTALL_DIR=~/.bin/

all: chi^2

clean:
	rm chi^2

chi^2: main.c frequencies.c digrams.c
	$(CC) $^ -o $@ $(CFLAGS)

test: chi^2
	./test

install:
	[ -d $(INSTALL_DIR) ] && cp chi^2 $(INSTALL_DIR)

.PHONY: all test clean install

