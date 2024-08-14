#include <exec/types.h>
#include <lattice/stdio.h>
#include "ddefs.h"

extern	int	stop_test;	/* Not 0 means <ctrl-c> typed */
extern	int	nsec;		/* # of sectors per track */
extern	int	ntrk;		/* # of tracks per cylinder */
extern	long	nblkcyl;	/* number of blocks on a cylinder */
extern	void	wdwait();

void wdvfytrk(t, c)
{
	register int i;
	register long offs;

	offs = (nsec * (long)t) + (nblkcyl * (long)c);
	puts(" ");
	puts("t=");
	putint(t);
	puts(" c=");
	putint(c);
	puts("  \r");
	if (t < 0 || t >= ntrk)
	{
		puts("track # out of bounds\n");
		stop_test++;
		return;
	}
	initcmdblk(WDCTF,nsec,BUFPADDR,offs);
	wdwait();				/* wait for i/o complete */
	i = geterrbits();
	if (i & 0x7F) {
		derror("Verify",i);
	}
}
