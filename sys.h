#include <stdarg.h>

#ifdef TESTPROG
#include <stdio.h>
#endif

#ifdef TESTPROG
extern	int	g_debug;
#define dprintf if (g_debug) fprintf
#else
#define dprintf(...)
#endif

extern void	mem_set(void *addr, int c, int len);
extern int	pf(const char *fmt, ...);
