# mini_printf
### minimal, self-contained and verified printf()

[![Build Status](https://travis-ci.org/wkoszek/mini_printf.svg)](https://travis-ci.org/wkoszek/mini_printf)

This is a minimal `printf()` implementation that doesn't depend on any other
libraries. It's meant to be used for early bootstrapping of DSLs and
embeddable programs, as well as bare-metal software. It supports `%x` and
`%d` format string specifiers only. One can easily extend `mini_printf.c`
for additional format strings.

# How to use it

The `mini_printf` comes in 1 C file called `mini_printf.c`. It has a
function called `pf(const char *fmt, ...)` which works exactly how `printf`
does. It's named `pf()` to prevent from eventual collisions with other
printf-alike API you may have in your programs.

To use it, include `mini_printf.c` in your program. It only wants one thing
from you: you must provide it a `io_putc(int c)` function, which is used
internally to do an actual I/O. You should be able to later call:

	pf("sample");

and get yours `io_putc` routine called for each character.

# How it was tested

The biggest effort went into verification. Testing of `printf` is very
important, since failure of `printf` can lead to many hours of frustration
and debugging, so I tried to make it less likely.

The `testprog.c` has a randomized stress test suite proving my `pf()` is
fairly robust. Stress test basically generates random format strings in a
smart way and plays them against system `printf()` and `mini_printf` and
then tries to compare the results. Mismatch means a failure of the test.

To start stress-testing, type:

	make stresstest

It'll try to generate 1,000,000 random settings and play them against
`mini_printf`.

Several other useful targets are:

- make test.vv
  - very verbose and slow test. Mostly for development.
- make test.v
  - less verbose test. Slightly slower due to not all test data printed out.
- make test
  - long-running test for very stressful testing. Can make your MacBook Air
    boil
- make unittest
  - unit-testing, for the cases when bugs are found

Underlying program for all tests is `testprog`. Its usage:

Flag    | Description
:-------|:-------------------------------------------------
d       | turn debugging on. Can be specified more than once
l <num> | how many loops to run. 1 loop = 1 chunk of format string. Default: 6.
m       | print mask: frequency of printouts after each test execution
t <num> | start only <num> unit test.
w <num> | wait <num> nanoseconds after each test. For debugging only.
p       | print generated format strings
s <num> | stop after <num> tests.
v       | verbose mode.

# Examples

You've looked at the source and want to run only unit test number 11:

	./testprog -d -t 11

You want to run all unit tests:

	./testprog -d

You want to figure out what's going on: random format strings need to be
short (`-l 1`), output must be slow (`-w 500000`) and verbose (`-d -v -p`), and
every generated test must be printed out (`-m 0`):

	./testprog -w 500000 -l 1 -d -v -m 0 -p

# Author

- Wojciech Adam Koszek, [wojciech@koszek.com](mailto:wojciech@koszek.com)
- [http://www.koszek.com](http://www.koszek.com)
