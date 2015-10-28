all: testprog mini_printf.so

SRC= mini_printf.c testprog.c libc.c

testprog: $(SRC) sys.h
	$(CC) $(CFLAGS) -std=c99 -DTESTPROG -Wall -pedantic mini_printf.c testprog.c libc.c -o testprog
mini_printf.so: $(SRC) sys.h
	$(CC) $(CFLAGS) -std=c99 -fno-stack-protector -nostdlib -Wall -pedantic -o mini_printf.so -fPIC -fpic -shared mini_printf.c testprog.c libc.c
analyze:
	scan-build -o /tmp/_.mini_printf make -j4

test.vv:
	./testprog -m 0 -v
test.v:
	./testprog -m 15 -v
test:
	./testprog -m 4095 -v

clean:
	rm -rf testprog
	rm -rf mini_printf.so
