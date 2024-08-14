#include <exec/types.h>
#include <lattice/stdio.h>
#include "ddefs.h"
extern	int	unit;			/* which physical drive ? */

initcmdblk(opcode,blockcnt,dma, blocknum) /* initialize command block */
char opcode; int blockcnt; long dma; long blocknum;
{
	register WDCMD *cbp;
	register int i;
	register char *cp;

#ifdef debug
	puts("\nopcode = ");	
	putbhex(opcode);
	puts(" blockcnt = ");
	putint(blockcnt);
	puts(" dma = ");
	putlhex(dma);
	puts(" blocknum = ");
	putlhex(blocknum);
	puts("\n");
#endif
	cbp = (WDCMD *) CMDBLKPADDR;
	for (cp = (char *)cbp, i = 0; i < sizeof(WDCMD); ++i)
		cp[i] = 0;
	cbp->c_opcode = opcode;
	cbp->c_blockcnt = blockcnt;
	cbp->c_highdma = (dma >> 16);	/* phys segment */
	cbp->c_middma = (dma >> 8);	/* offset */
	cbp->c_lowdma = dma;
	cbp->c_lunhiaddr = (unit << 5) | (blocknum >> 16);
	cbp->c_midaddr = blocknum >> 8;
	cbp->c_lowaddr = blocknum ;
	cbp->c_errorbits = 0xff;		/* make ready */
}
