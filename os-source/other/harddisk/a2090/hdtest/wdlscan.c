#include "ddefs.h"

extern	char	soft_error();
extern	char	databuf[512];
extern	long	nblkcyl;	/* number of blocks on a cylinder */
extern	char	rd_error;	/* Set to 1 by read error */
extern	void	wdwait();

wdlscan(sn)
long sn;
{
	int i;

	do {
	puts(" ");
	putlong(sn);
	puts("  \r");
	rd_error = 0;
	initcmdblk(WDREAD,(int)nblkcyl,LONGBUFPADDR,sn);
	wdwait();				/* wait for i/o complete */
	i = geterrbits();
	if (i & 0x7F)
	{
		rd_blk_err(getblknum(),i);
		rd_error += !soft_error(i);
		sn = getblknum() + 1L;
	}
	} while (rd_error && !check_stop_test());
}
