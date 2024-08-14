#include <exec/types.h>
#include <lattice/stdio.h>
#include "ddefs.h"

extern	char	soft_error();
extern	char	databuf[512];
extern	void	wdwait();

void wdread(sn)
long sn;
{
	int i;
	char *cp;
	char *bp;
	char c;

	cp = (char *) BUFPADDR;
	wdblkinit(cp, ' '); /* fill 512 byte block of ' ' */
	c = sn;
	initcmdblk(WDREAD,1,BUFPADDR,sn);
	wdwait();				/* wait for i/o complete */
	i = geterrbits();
	cp = (char *)BUFPADDR;
	bp = databuf;
	if (i & 0x7F)
		rd_blk_err(sn,i);
	if (soft_error(i))
	    if (wdcompare(cp, databuf)) {
		for (i = 0; i < 512; ++i)
			if (*cp++ != *bp++) {
				puts(" ");
				putlong(sn);
				puts("  \r");
				compare_error(sn,--cp,--bp,i);
				return;
			}
	}
}
