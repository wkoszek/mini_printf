void
mem_set(void *addr, int c, int len)
{
	char	*cptr;
	int	i;

	cptr = addr;
	for (i = 0; i < len; i++) {
		cptr[i] = (c & 0xff);
	}
}
