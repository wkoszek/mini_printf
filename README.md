# mini_printf -- minimal self-contained printf()

[![Build Status](https://travis-ci.org/wkoszek/mini_printf.svg)](https://travis-ci.org/wkoszek/mini_printf)

*Wojciech Adam Koszek* **wojciech@koszek**

This is a minimal printf() implementation that doesn't depend on wny other
libraries. The testprog.c has a randomized stress test suite proving my
printf() is fairly robust. Testprog can be run with options:

	-d
		debugging

	-w
		add delay between test iterations.

	-v
		run randomization suite

	-m
		print mask for tests. Each tests from 0 to INF will run when
		-v is specified. -m <arg> will spit out on console which
		test you're currently running in, as long as test_number %
		<arg> == 0. So <arg> of 0 means print a line every test,
		<arg> of 0xff means: print a line every 255 tests etc.
	-w
		insert delay in between tests

### Porting

Library is self contained in mini_printf.c. To make it work on your
system you can start by doing:

	make

it will compile a ``testprog''. To move it from there, you'll take
``mini_printf.c'', and provide your own ``io_putc(int c)''.
