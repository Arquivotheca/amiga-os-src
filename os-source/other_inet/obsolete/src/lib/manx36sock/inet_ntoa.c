/*
** inet_ntoa - convert internet address to dot (d.d.d.d) format.
*/

char *
inet_ntoa(l)
	register unsigned long	l;
{
	unsigned int a, b, c, d;
	static char buf[32];

	d = l & 0xff; l >>= 8;
	c = l & 0xff; l >>= 8;
	b = l & 0xff; l >>= 8;
	a = l & 0xff;

	sprintf(buf, "%d.%d.%d.%d", a, b, c, d);
				    
	return buf;
}

