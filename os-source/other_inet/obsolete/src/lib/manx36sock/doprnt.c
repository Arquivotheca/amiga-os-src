/*
** Unix compatible _doprnt().  This interface really needs a standard
*/

#include <stdio.h>

static FILE *private_fp;
static putter(c)
	char	c;
{
	aputc(c, private_fp);
}

_doprnt(fmt, argsp, fp)
	char	*fmt;
	void	*argsp;
	FILE	*fp;
{
	private_fp = fp;
	return format(putter, fmt, argsp);
}
