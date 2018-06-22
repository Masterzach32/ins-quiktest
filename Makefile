CFLAGS = -std=c99 -Wall -Wpedantic -g
CPPFLAGS = -std=c++11 -Wall -Wpedantic -g
EIGEN = -I /usr/include/eigen3

.PHONY: all clean install

all: ldprm ilconv nconv

clean:
	@rm app/ldprm app/ilconv app/nconv >/dev/null 2>/dev/null || true

install:
	yes | sudo apt install libeigen3-dev

ilconv: app/ilconv
app/ilconv: src/ilconv.cpp
	@mkdir -p app/ >/dev/null 2>/dev/null
	g++ $(CPPFLAGS) $^ -o $@ $(EIGEN)

nconv: app/nconv
app/nconv: src/nconv.c
	@mkdir -p app/ >/dev/null 2>/dev/null
	gcc $(CFLAGS) $^ -o $@

ldprm: app/ldprm
app/ldprm: src/ldprm.c
	@mkdir -p app/ >/dev/null 2>/dev/null
	gcc $(CFLAGS) $^ -o $@
