#include <exec/types.h>
#include <lattice/stdio.h>
#include "ddefs.h"

extern	char	databuf[512];
extern	void	wdwait();

void wdprwrite(sn)
long sn;
{
	register int i;
	register long *ip;

	puts(" ");
	putlong(sn);
	puts("  \r");
	ip = (int *)BUFPADDR;	/* data area */
	for (i = 0; i < 128; ++i)
		*ip++ = rand();		/* generate next in pseudo-sequence */
	initcmdblk(WDWRITE,1,BUFPADDR,sn);
	wdwait();				/* wait for i/o complete */
	i = geterrbits();
	if (i & 0x7F)
		wrt_blk_err(sn,i);
}

void wdprread(sn)
long sn;
{
	register int i;
	register char *cp;
	register long *ip;
	register char *bp;
	puts(" ");
	putlong(sn);
	puts("  \r");
	initcmdblk(WDREAD,1,BUFPADDR,sn);
	wdwait();				/* wait for i/o complete */
	i = geterrbits();
	for (ip = databuf, i = 0; i < 128; ++i)
		*ip++ = rand();
	cp = (char *)BUFPADDR;
	if (i & 0x7F)
		rd_blk_err(sn,i);
	else if (wdcompare(databuf, cp)) {
		for (bp = databuf, i = 0; i < 512; ++i)
			if (*cp++ != *bp++) {
				compare_error(sn,cp,--bp,i);
				return;
			}
	}
}
