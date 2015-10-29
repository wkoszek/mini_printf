#include "sys.h"

extern void	io_putc(int c);

typedef unsigned int	u32;
typedef unsigned long	u64;

/*
 * String to number conversion
 */
int
vpf_str_to_num(const char *fmtstr, int *resnum)
{
	const char *cptr;
	int	res, digit, is_digit;

	res = 0;
	for (cptr = fmtstr; *fmtstr != '\0'; cptr++) {
		is_digit = (*cptr >= '0' && *cptr <= '9');
		if (!is_digit) {
			break;
		}
		digit = *cptr - '0';
		res *= 10;
		res += digit;
	}
	*resnum = res;
	return ((int)(cptr - fmtstr));
}

/*
 * Number to string conversion
 */
void
vpf_num_to_str(u32 a, int is_hex, int pad_len, int pad_char)
{
	char	buf[32];
	u32	base;
	int	bufidx, i;

	for (i = 0; i < sizeof(buf); i++) {
		buf[i] = pad_char;
	}
	base = 10;
	if (is_hex) {
		base = 16;
	}
	dprintf(stderr, "a=%08x, base=%d\n", a, base);
	bufidx = 0;
	do {
		dprintf(stderr, "a %% base = %d\n", a % base);
		buf[bufidx] = "0123456789abcdef" [ a % base ];
		dprintf(stderr, "buf=%c, bufidx=%d\n", buf[bufidx], bufidx);
		a /= base;
		bufidx++;
	} while (a > 0);

	if (pad_len > 0) {
		if (pad_len >= sizeof(buf)) {
			pad_len = sizeof(buf) - 1;
		}
		if (bufidx < pad_len) {
			bufidx = pad_len;
		}
	}
	buf[bufidx] = '\0';

	for (i = bufidx - 1; i >= 0; i--) {
		io_putc(buf[i]);
	}
}

/*
 * Main body of vpf() routine. Main parsing of the print format happens here
 */
int
vpf(const char *fmtstr, va_list va)
{
	const char *cptr, *prefmtptr;
	int	fmt, c, va_int, pad_len, pad_char, i;

	pad_char = ' ';
	for (cptr = fmtstr; *cptr != '\0'; cptr++) {
		fmt = 0;
		pad_len = 0;
		c = *cptr;
		prefmtptr = cptr;
		if (*cptr == '%') {
			prefmtptr = cptr;
			cptr++;
			if (*cptr >= '0' && *cptr <= '9') {
				dprintf(stderr, "PAD *cptr = %c\n", *cptr);
				cptr += vpf_str_to_num(cptr, &pad_len);
			}
			fmt = *cptr;
			dprintf(stderr, "fmt=%c\n", fmt);
		}
		if ((fmt == 'd') || (fmt == 'x')) {
			va_int = va_arg(va, int);
			dprintf(stderr, "va_int=%08x, pad_len=%d\n", va_int,
				pad_len);
			vpf_num_to_str(va_int, fmt == 'x', pad_len, pad_char);
		} else {
#ifndef MACOSX_PRINTF
			for (i = 0; i < (cptr - prefmtptr); i++) {
				dprintf(stderr, "C=%c\n", prefmtptr[i]);
				io_putc(prefmtptr[i]);
			}
#else
			for (i = 0; i < pad_len - 1; i++) {
				io_putc(pad_char);
			}
#endif
			io_putc(fmt != 0 ? fmt : c);
		}
	}
	return 0;
}

/*
 * Mini printf called "pf"
 */
int
pf(const char *fmt, ...)
{
	va_list	va;

	va_start(va, fmt);
	vpf(fmt, va);
	va_end(va);
	return 0;
}
