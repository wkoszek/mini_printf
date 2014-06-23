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
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>

#include "sys.h"

typedef uint32_t	u32;
typedef uint64_t	u64;

/*
 * Knobs
 */
int	g_knob_no_minus = 1;

/*
 * Settings
 */
u64	g_test_print_mask = 0;	

/*
 * Debugging
 */
int	g_debug = 0;

/*
 * Sample I/O routine with no buffering for now.
 */
void
io_putc(int c)
{
	printf("%c", c);
}

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
	ret32 = ret64;
	(void)lcg_getset(ret32, is_set=1);
	return ret32;
}

void
verif(int argc, char **argv)
{
	u32	seed;
	u64	mask;
	char	fmtstr[1000];
	char	*cptr;
	int	i, j;
	u32	args[6];
	int	(*f_printf)(char *, const char *fmt, ...);
	int	(*f_pf)(const char *fmt, ...);
	char	cmp_buf1[1000], cmp_buf2[1000], tmpbuf[1000], diffbuf[1000];

	f_printf = sprintf;	// stay away from compiler warnings
	f_pf = pf;

	if (argc == 1) {
		seed = strtol(argv[0], (char **) NULL, 16);
		lcg_getset(seed, 1);
	}
	printf("seed=%#08x, argv[0]=%s\n", seed, argv[0]);
	if (g_test_print_mask == 0) {
		g_test_print_mask = 0xffff;
	}
	for (mask = 0; ;mask++ ) {
		if ((mask & g_test_print_mask) == 0) {
			fprintf(stderr, "TEST 0x%016lx SEED 0x%08x\n", mask,
				lcg_getset(0, 0));
		}

		cptr = fmtstr;
		u32 loop_num = lcg_rand() % 0x7;
		for (j = 0; j < loop_num; j++) {
			u32 has_h = lcg_rand() & 1;
			u32 has_pad = lcg_rand() & 1;
			u32 pad_len = lcg_rand() & 0x7;
			u32 is_correct = lcg_rand() & 1;
			u32 pre_str_has = lcg_rand() & 1;
			u32 pre_str_len = lcg_rand() & 0xf;
			u32 post_str_has = lcg_rand() & 1;
			u32 post_str_len = lcg_rand() & 0xf;
			u32 has_d = lcg_rand() & 0xff;
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
			if (has_h) {
				*cptr++ = 'x';
			}
			if (has_d) {
				*cptr++ = 'd';
			}
			if (!has_h || has_d) {
				/*
				 * make sure we don't insert sprintf()
				 * valid format, which isn't supported by
				 * mini_printf
				 */
				*cptr++ = 'W';
			}
			if (post_str_has) {
				for (i = 0; i < post_str_len; i++) {
					*cptr++ = 'a' + (lcg_rand() % 20);
				}
			}
		}

		*cptr++ = 0;

		/* C library printf() puts value to cmp_buf1 */
		f_printf(cmp_buf1, fmtstr, args[0], args[1], args[2], args[3],
			args[4], args[5]);

		/* 
		 * Now we test pf(). We make ugly
		 * http://www.koszek.com/2012/05/19
		 * hack to let us test printf. Better way would be to
		 * abstract io_putc() into buffer operating function, but
		 * this is good enough for now.
		 */
		fflush(stdout);
		setbuf(stdout, tmpbuf);
		f_pf(fmtstr, args[0], args[1], args[2], args[3], args[4],
			args[5]);
		strcpy(cmp_buf2, tmpbuf);
		__fpurge(stdout);
		fflush(stdout);
		memset(tmpbuf, 0, sizeof(tmpbuf));
	
		if (strcmp(cmp_buf1, cmp_buf2) != 0) {
			printf("=======\n");
			printf("fmtstr='%s'\n", fmtstr);
			memset(diffbuf, '.', sizeof(diffbuf));
			for (i = 0; i < strlen(cmp_buf1); i++) {
				if (cmp_buf1[i] != cmp_buf2[i]) {
					diffbuf[i] = '^';
				}
			}
			diffbuf[i] = 0;

			printf("P:%s\n", cmp_buf1);
			printf("V:%s\n", cmp_buf2);
			printf("d:%s\n", diffbuf);
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

int
main(int argc, char **argv)
{
	int	o, flag_v;

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

	argc -= optind;
	argv += optind;

	if (1) pf("wojtek\n");
	if (1) pf("wojtek'\n");
	if (1) pf("zero '%d'\n", 0);
	if (1) pf("wojtek %d'\n", 123);
	if (1) pf("'wojtek %d %d'\n", 123, 789);
	if (1) pf("'wojtek %x %x'\n", 123, 789);
	if (1) pf("len8 == '%08x'\n", 0x123);
	if (1) pf("len8 == '%08x'\n", 0x0);
	if (1) pf("len2 == '%012x'\n", 0x123);
	if (1) pf("%7d\n", 123);

	if (!flag_v) {
		exit(1);
	}
	printf("Verifying mini_printf.c\n");
	verif(argc, argv);

	return 0;
}
