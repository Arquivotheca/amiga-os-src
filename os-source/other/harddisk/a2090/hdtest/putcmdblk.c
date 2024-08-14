#include <exec/types.h>
#include <lattice/stdio.h>
#include "ddefs.h"

void putcmdblk()
{
	register WDCMD *cbp;
	register UBYTE *cp;
	register int i;

	printf("\n");
	cbp = (WDCMD *) CMDBLKPADDR;
	for (cp = (char *)cbp, i = 0; i < sizeof(WDCMD); ++i)
	{
		printf(" %2x",cp[i]);
		putbhex((int)cp[i]);
		puts(" ");
	}
	printf("\n");
	puts("\n");
}
