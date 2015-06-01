all: testprog mini_printf.so

SRC= mini_printf.c testprog.c libc.c

testprog: $(SRC)
	$(CC) $(CFLAGS) -std=c99 -DTESTPROG -Wall -pedantic mini_printf.c testprog.c libc.c -o testprog
mini_printf.so: $(SRC)
	$(CC) $(CFLAGS) -std=c99 -fno-stack-protector -nostdlib -Wall -pedantic -o mini_printf.so -fPIC -fpic -shared mini_printf.c testprog.c libc.c
analyze:
	scan-build -o /tmp/_.mini_printf make -j4

clean:
	rm -rf testprog
	rm -rf mini_printf.so
