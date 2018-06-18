CFLAGS = -std=c99 -Wall -Wpedantic -g
CPPFLAGS = -std=c++11 -Wall -Wpedantic -g
EIGEN = -I /usr/include/eigen3

.PHONY: all clean install

all: ldprm conv 

clean:
	@rm app/ldprm app/conv >/dev/null 2>/dev/null

install:
	yes | sudo apt install libeigen3-dev

conv: app/conv
app/conv: src/conv.cpp
	@mkdir -p app/ >/dev/null 2>/dev/null
	g++ $(CPPFLAGS) $^ -o $@ $(EIGEN)

ldprm: app/ldprm
app/ldprm: src/ldprm.c
	@mkdir -p app/ >/dev/null 2>/dev/null
	gcc $(CFLAGS) $^ -o $@
