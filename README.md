# mini_printf

Wojciech Adam Koszek ``wojciech@koszek''

This is a minimal printf() implementation that doesn't depend on wny other
libraries. The testprog.c has a randomized stress test suite proving my
printf() is fairly robust. Testprog can be run with options:

### Porting

Library is self contained in mini_printf.c. To make it work on your
system you can start by doing:

	make

it will compile a ``testprog''. To move it from there, you'll take
``mini_printf.c'', and provide your own ``io_putc(int c)''.

	-d
		debugging
	-w
		add delay between test iterations.

	flag_v = 0;
	while ((o = getopt(argc, argv, "dn:m:vw:")) != -1) {
		switch (o) {
		case 'd':
			g_debug = 1;
			break;
		case 'm':
			g_test_print_mask = atoi(optarg);
			break;
		case 'w':
			g_delay = atoi(optarg);
			break;
		case 'v':
			flag_v = 1;
			break;
		default:
			abort();
		}
	}
