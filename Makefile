CC = gcc
CFLAGS = -std=c99 -Wall -Wpedantic -g

.PHONY: all clean

all: bin/conv bin/ldprm

clean:
	@rm -rf bin >/dev/null 2>/dev/null

conv: bin/conv
bin/conv: src/conv.c
	@mkdir -p bin/ >/dev/null 2>/dev/null
	$(CC) $(CFLAGS) $^ -o $@

ldprm: bin/ldprm
bin/ldprm: src/ldprm.c
	@mkdir -p bin/ >/dev/null 2>/dev/null
	$(CC) $(CFLAGS) $^ -o $@
