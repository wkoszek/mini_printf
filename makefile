all: testprog shlib

testprog:
	$(CC) $(CFLAGS) -std=c99 -DTESTPROG -Wall -pedantic mini_printf.c testprog.c libc.c -o testprog
shlib:
	$(CC) $(CFLAGS) -std=c99 -fno-stack-protector -nostdlib -Wall -pedantic -o mini_printf.so -fPIC -fpic -shared mini_printf.c testprog.c libc.c
clean:
	rm -rf printf
	rm -rf mini_printf.so
