CFLAGS = -std=c99 -Wall -Wpedantic -g
CPPFLAGS = -std=c++11 -Wall -Wpedantic -g
EIGEN = -I /usr/include/eigen3

.PHONY: all clean install

all: ldprm conv 

clean:
	@rm -rf bin >/dev/null 2>/dev/null

install:
	yes | sudo apt install libeigen3-dev

conv: bin/conv
bin/conv: src/conv.cpp
	@mkdir -p bin/ >/dev/null 2>/dev/null
	g++ $(CPPFLAGS) $^ -o $@ $(EIGEN)

ldprm: bin/ldprm
bin/ldprm: src/ldprm.c
	@mkdir -p bin/ >/dev/null 2>/dev/null
	gcc $(CFLAGS) $^ -o $@
