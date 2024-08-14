#include <exec/types.h>
#include <lattice/stdio.h>
#include "ddefs.h"

extern	char	soft_error();
extern	char	databuf[512];
extern	long	nblkcyl;	/* number of blocks on a cylinder */
extern	char	rd_error;	/* Set to 1 by read error */
extern	void	wdwait();

wdlread(sn)
long sn;
{
	char *cp;
	char *bp;
	int i, j;

	do {
	puts(" ");
	putlong(sn);
	puts("  \r");
	rd_error = 0;
	cp = (char *) LONGBUFPADDR;	/* initialize buffer */
	for (j = 0;j < (int)nblkcyl; ++j) {
		wdblkinit(cp, ' '); /* fill 512 byte block of ' ' */
		cp += 512;
	}
	initcmdblk(WDREAD,(int)nblkcyl,LONGBUFPADDR,sn);
	wdwait();				/* wait for i/o complete */
	i = geterrbits();
	if (i & 0x7F)
	{
		rd_blk_err(getblknum(),i);
		rd_error += !soft_error(i);
		sn = getblknum() + 1L;
	}
	if (!rd_error)
	{
		cp = (char *) LONGBUFPADDR;
		for (j = 0;(j < (int)nblkcyl) && !rd_error; ++j) {
		    if (wdcompare(cp, databuf))
		    {
			bp = databuf;
			for (i = 0;(i < 512) && !rd_error; ++i)
			   if (*cp++ != *bp++) {
				compare_error(sn,--cp,--bp,i);
				rd_error++;
			   }
		    }
		    cp += 512;				/* next block */
		    sn++;
		}
	}
	} while (rd_error && !check_stop_test());
}
