#include <exec/types.h>
#include <lattice/stdio.h>
#include "ddefs.h"

extern	int	stop_test;	/* Not 0 means <ctrl-c> typed */
extern	long	bad_blocks[300];/* bad block list */
extern	int	idx_bad_blocks;
extern	int	nsec;		/* # of sectors per track */
extern	int	ntrk;		/* # of tracks per cylinder */
extern	long	nblkcyl;	/* number of blocks on a cylinder */
extern	void	wdwait();

void wdfmttrk(t, c)
{
	register int i;
	register char *bp;
	register int count;
	register long offs;
	register long block_number;
	puts(" ");
	puts("t=");
	putint(t);
	puts(" c=");
	putint(c);
	puts("  \r");
	offs = (nsec * (long)t) + (nblkcyl * (long)c);
	bp = (char *)BUFPADDR;
	if (t < 0 || t >= ntrk)
	{
		puts("track number out of bounds\n");
		stop_test++;
		return;
	}
	i = (nsec - t) % nsec;
	for (count = 0; count < nsec; ++count) {
		block_number = (nsec * (long)t) + (nblkcyl * (long)c) + i;
			/* if block is in bad_block list, mark as bad */
		if ((findlong(block_number,bad_blocks,idx_bad_blocks) == -1)
			|| (block_number  == 0L))
			*bp++ = 0;
		else
		{
			*bp++ = 0x80;
			puts("Block ");
			putlong(block_number);
			puts(" marked bad\n");
		}
		*bp++ = i;
		i = (i + 1) % nsec;
	}
	initcmdblk((UWORD)WDFMTT,(int)nsec,(long)BUFPADDR,(long)offs);
	wdwait();				/* wait for i/o complete */
	i = geterrbits();
	if (i & 0x7F) {
		derror("Format",i);
	}
}
