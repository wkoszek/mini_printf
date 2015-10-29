/*
 * mini_printf (c) 2012 Wojciech A. Koszek <wkoszek@FreeBSD.org>
 *
 * Along with the main function included is a testbench for verification
 * that mini_printf() is as close and accurate as possible with the main
 * stdlib's function printf().
 *
 * lcg_* routines use Lehmer random number generator, and the code is
 * inspired by:
 *
 * http://en.wikipedia.org/wiki/Park–Miller_random_number_generator
 */
#define _BSD_SOURCE
#include <assert.h>
#include <stdio.h>
#ifndef __APPLE__
#include <stdio_ext.h>
#endif
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>

#include "sys.h"

#define	TMPBUF_LEN	1024

typedef uint32_t	u32;
typedef uint64_t	u64;

/* Knobs */
int	g_knob_no_minus = 1;

/* Settings */
u64	g_test_print_mask = 0;	
int	g_loops = 6;
int	g_print = 0;
u32	g_stop_after_test = -1;

/* Debugging */
int	g_debug = 0;

/*
 * I/O buffer which will immitate stdio's underlying (read: setvbuf())
 * buffer.
 */
char	io_putc_buf[10*TMPBUF_LEN];
int	io_putc_buf_idx = 0;

/* Sample I/O routine with no buffering for now. */
void
io_putc(int c)
{

	io_putc_buf[io_putc_buf_idx] = c;
	io_putc_buf_idx++;

#ifdef TESTPROG
	assert(io_putc_buf_idx < sizeof(io_putc_buf));
#endif
}

#ifdef TESTPROG
/*
 * Delay between generating vectors
 */
static int	g_delay = 0;

/*
 * lcg_* is a bit awkward, but I wanted to make these two functions
 * self-contained, and not depending on any global variables.
 */
u32
lcg_getset(u32 seed, int is_set)
{
	static int	lcg_seed;

	if (is_set) {
		lcg_seed = seed;
	}
	return lcg_seed;
}

u32
lcg_rand(void)
{
	int	is_set;
	u32	val, ret32;
	u64	ret64;

	val = lcg_getset(0, is_set=0);	/* get internal seed */
	if (val == 0) {			/* first invocatoin? */
		lcg_getset(123, is_set=1);	/* initialize 123 */
		val = lcg_getset(0, is_set=0);
	}
	ret64 = ((uint64_t)val * 279470273UL) % 4294967291UL;
	ret32 = (u32)ret64;
	(void)lcg_getset(ret32, is_set=1);
	return ret32;
}

void
verif(int argc, char **argv)
{
	u32	seed, test_num;
	char	fmtstr[TMPBUF_LEN];
	char	*cptr;
	int	i, j;
	u32	args[6];
	int	(*f_printf)(char *, const char *fmt, ...);
	int	(*f_pf)(const char *fmt, ...);
	char	cmp_sys[TMPBUF_LEN], cmp_mini_pf[TMPBUF_LEN],
			diffbuf[TMPBUF_LEN];

	printf("  verif() is starting..\n");
	printf("  g_test_print_mask=%llx, g_delay=%d, g_debug=%d g_print=%d\n",
			(long long)g_test_print_mask, g_delay, g_debug, g_print);

	f_printf = sprintf;	// stay away from compiler warnings
	f_pf = pf;

	seed = 13;
	if (argc == 1) {
		seed = (u32)strtol(argv[0], (char **) NULL, 16);
		lcg_getset(seed, 1);
	}
	printf("  seed=%#08x, argv[0]=%s\n", seed, argv[0]);
	for (test_num = 0; ;test_num++) {
		if ((test_num & g_test_print_mask) == g_test_print_mask) {
			fprintf(stderr, "TEST 0x%016lx SEED 0x%08x\n",
			    (unsigned long)test_num, lcg_getset(0, 0));
		}
		if ((g_stop_after_test != -1) &&
					(test_num >= g_stop_after_test)) {
			fprintf(stderr, "  Requested stop after %d tests\n"
			    "Terminating!\n", g_stop_after_test);
			break;
		}

		cptr = fmtstr;
		u32 loop_num = 1 + (lcg_rand() % 0x7);
		if (loop_num > g_loops) {
			loop_num = g_loops;
		}
		for (j = 0; j < loop_num; j++) {
			u32 pre_str_has = lcg_rand() & 1;
			u32 pre_str_len = lcg_rand() & 0xf;
			u32 is_correct = lcg_rand() & 1;
			u32 has_pad = lcg_rand() & 1;
			u32 pad_len = lcg_rand() & 0x7;
			u32 has_hex = lcg_rand() & 1;
			u32 has_d = lcg_rand() & 0xff;
			u32 post_str_has = lcg_rand() & 1;
			u32 post_str_len = lcg_rand() & 0xf;

			/* Fill the arguments array so we can use it later */
			for (i = 0; i < 6; i++) {
				args[i] = lcg_rand();
				if (g_knob_no_minus) {
					/* very simple workaround */
					args[i] &= 0x7fffffff;
				}
			}
			if (pre_str_has) {
				for (i = 0; i < pre_str_len; i++) {
					*cptr++ = 'a' + (lcg_rand() % 30);
				}
			}
			if (is_correct) {
				*cptr++ = '%';
			}
			if (has_pad) {
				*cptr++ = "123456789"[pad_len];
			}
			if (has_hex) {
				*cptr++ = 'x';
			}
			if (has_d) {
				*cptr++ = 'd';
			}
			if (!has_hex || has_d) {
				/*
				 * make sure we don't insert sprintf()
				 * valid format, which isn't supported by
				 * mini_printf
				 */
				*cptr++ = 'W';
			}

			if (post_str_has) {
				for (i = 0; i < post_str_len; i++) {
					*cptr = 'a' + (lcg_rand() % 20);
					cptr++;
				}
			}
		}

		*cptr++ = 0;

		/* C library printf() puts value to cmp_sys */
		f_printf(cmp_sys, fmtstr, args[0], args[1], args[2], args[3],
			args[4], args[5]);

		/* Reset the test buffer */
		memset(io_putc_buf, 0, sizeof(io_putc_buf));
		io_putc_buf_idx = 0;

		f_pf(fmtstr, args[0], args[1], args[2], args[3], args[4],
			args[5]);
		strcpy(cmp_mini_pf, io_putc_buf);

		if (g_print > 0) {
			fprintf(stderr, "     SYS : '%s'\n", cmp_sys);
			fprintf(stderr, "     PF : '%s'\n", cmp_mini_pf);
		}
		if (strcmp(cmp_sys, cmp_mini_pf) != 0) {
			printf("==== printf() != mini_printf() mismatch ====\n");
			printf("fmtstr='%s'\n", fmtstr);
			memset(diffbuf, '.', sizeof(diffbuf));
			for (i = 0; i < strlen(cmp_sys); i++) {
				if (cmp_sys[i] != cmp_mini_pf[i]) {
					diffbuf[i] = '^';
				}
			}
			diffbuf[i] = 0;

			printf(" SYS_PF:%s\n", cmp_sys);
			printf("MINI_PF:%s\n", cmp_mini_pf);
			printf("   diff:%s\n", diffbuf);
			printf("=====\n");
			printf("pf(\"%s\", %08x, %08x, %08x, %08x, %08x, %08x);\n",
			    	fmtstr,
				args[0], args[1], args[2], args[3], args[4],
				args[5]);
			printf("=====\n");
			exit(1);
		}
		usleep(g_delay);
	}
}

static void
usage(const char *progname)
{
	fprintf(stderr, "%s usage:\n", progname);
	fprintf(stderr, "-d         increase debug level\n");
	fprintf(stderr, "-m <mask>  test print mask\n");
	fprintf(stderr, "-l <num>   loops for format creation "
	    "(lower=shorter format string\n");
	fprintf(stderr, "-t <num>   run just <num> unit test\n");
	fprintf(stderr, "-w <secs>  delay test execution by <secs> seconds\n");
	fprintf(stderr, "-v         start verification\n");
}

int
main(int argc, char **argv)
{
	int	o, flag_v, flag_debug, arg_mask, arg_delay, arg_loops;
	int	test_num;

	memset(io_putc_buf, 0, sizeof(io_putc_buf));

	if (argc == 1) {
		usage(argv[0]);
		exit(64);
	}

	flag_v = flag_debug = arg_mask = arg_delay = arg_loops = test_num = 0;
	while ((o = getopt(argc, argv, "dn:m:vw:l:t:ps:")) != -1) {
		switch (o) {
		case 'd':
			flag_debug++;
			break;
		case 'l':
			arg_loops = atoi(optarg);
			break;
		case 'm':
			arg_mask = atoi(optarg);
			break;
		case 't':
			test_num = atoi(optarg);
			break;
		case 'w':
			arg_delay = atoi(optarg);
			break;
		case 'p':
			g_print++;
			break;
		case 's':
			g_stop_after_test = atoi(optarg);
			break;
		case 'v':
			flag_v = 1;
			break;
		default:
			abort();
		}
	}

	argc -= optind;
	argv += optind;

	if (!flag_v && ((arg_mask + arg_delay + arg_loops) > 0)) {
		fprintf(stderr, "-d/-m/-w only make sense with -v\n");
		exit(1);
	}

	g_debug = flag_debug;
	g_test_print_mask = arg_mask;
	g_delay = arg_delay;
	if (arg_loops) {
		g_loops = arg_loops;
	}

	if (!flag_v) {
		if (test_num == 0 || test_num == 1) pf("wojtek\n");
		if (test_num == 0 || test_num == 2) pf("wojtek'\n");
		if (test_num == 0 || test_num == 3) pf("zero '%d'\n", 0);
		if (test_num == 0 || test_num == 4) pf("wojtek %d'\n", 123);
		if (test_num == 0 || test_num == 5) pf("'wojtek %d %d'\n", 123, 789);
		if (test_num == 0 || test_num == 6) pf("'wojtek %x %x'\n", 123, 789);
		if (test_num == 0 || test_num == 7) pf("len8 == '%08x'\n", 0x123);
		if (test_num == 0 || test_num == 8) pf("len8 == '%08x'\n", 0x0);
		if (test_num == 0 || test_num == 9) pf("len2 == '%012x'\n", 0x123);
		if (test_num == 0 || test_num == 10) pf("%7d\n", 123);
		if (test_num == 0 || test_num == 11) pf("%k%k", 1, 1);
		if (test_num == 0 || test_num == 12) pf("woj%6Wtek", 1);
		printf("buf='%s'\n", io_putc_buf);
	} else {
		printf("Verifying mini_printf.c\n");
		verif(argc, argv);
	}
	return 0;
}
#endif
