all:
	$(CC) $(CFLAGS) -std=c99 -DTEST_PROG -Wall -pedantic printf.c testprog.c libc.c -o printf
shlib:
	$(CC) $(CFLAGS) -std=c99 -nostdlib -Wall -pedantic -o mini_printf.so -fPIC -fpic -shared printf.c testprog.c libc.c
clean:
	rm -rf printf
	rm -rf mini_printf.so
