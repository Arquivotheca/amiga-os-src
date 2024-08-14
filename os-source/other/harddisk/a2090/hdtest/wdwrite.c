#include <exec/types.h>
#include <lattice/stdio.h>
#include "ddefs.h"

extern	void	wdwait();

void wdwrite(sn)
long sn;
{
	register int i;

	initcmdblk(WDWRITE,1,BUFPADDR,sn);
	wdwait();				/* wait for i/o complete */
	i = geterrbits();
	if (i & 0x7F)
	{
		puts(" ");
		putlong(sn);
		puts("  \r");
		wrt_blk_err(sn,i);
	}
}
