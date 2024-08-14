#include <exec/types.h>
#include <lattice/stdio.h>
#include "ddefs.h"

char wdcompare(cp,databuf)
register char *cp;
char databuf[512];
{
	register char	*bp;
	register int	i;
	register char	rd_error;

	bp = databuf;
	rd_error = 0;
	for (i = 0; (i < 512) && !rd_error; ++i)
	   if (*cp++ != *bp++)
		rd_error++;
	return(rd_error);
}

void wdblkinit(cp,ch)
register char *cp;
char ch;
{
	register int	i;

	for (i = 0;i < 512; ++i)
	   *cp++ = ch;
}
