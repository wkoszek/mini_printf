all: testprog mini_printf.so

SRC= mini_printf.c testprog.c libc.c

CFLAGS+= -Wall -pedantic -std=c99
LIBFLAGS+= -fno-stack-protector -fPIC -fpic

UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
CFLAGS+=-DMACOSX_PRINTF
endif

testprog: $(SRC) sys.h
	$(CC) $(CFLAGS) -DTESTPROG $(SRC) -o testprog
mini_printf.so: $(SRC) sys.h
	$(CC) $(CFLAGS) $(LIBFLAGS) -nostdlib -o mini_printf.so -shared $(SRC)
analyze:
	scan-build -o /tmp/_.mini_printf make -j4

test.vv:
	./testprog -m 0 -v
test.v:
	./testprog -m 15 -v
test:
	./testprog -m 4095 -v
unittest:
	./testprog -d
stresstest:
	./testprog -v -s 1000000 -m 16383 $$RANDOM

clean:
	rm -rf testprog
	rm -rf mini_printf.so
