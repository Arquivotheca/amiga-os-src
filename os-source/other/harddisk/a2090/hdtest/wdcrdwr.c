#include <exec/types.h>
#include <lattice/stdio.h>
#include "ddefs.h"

extern	char	patno;		/* selected pattern number */
extern	char	soft_error();
extern	char	databuf[512];
extern	int	nsec;		/* # of sectors per track */
extern	int	ntrk;		/* # of tracks per cylinder */
extern	int	ncyl;		/* # of cylinders */
extern	long	nblk;		/* number of blocks on device */
extern	long	nblkcyl;	/* number of blocks on a cylinder */
extern	char	rd_error;	/* Set to 1 by read error */
extern	void	wdwait();

wdcrdwr(sn,lncyl)
long sn;
long lncyl;
{
	char *bp;
	char *cp;
	int i, j;

	do {
	puts(" ");
	putlong(sn);
	puts("  \r");
	if (patno == 7) /* If cylinder write/read of changing pattern */
	{
		rd_error = 0;
		cp = (char *) LONGBUFPADDR;
		for (j = 0;j < (int)nblkcyl; ++j) {
			wdblkinit(cp, j); /* fill 512 byte block of ' ' */
			cp += 512;
		}
	}
	else if (rd_error)	/* if pattern corrupted by error, refresh it */
	{
		patt_fill((char *)LONGBUFPADDR,nblkcyl*512L,0x6DB6DB6DL);
		cp = (char *) LONGBUFPADDR;
		bp = databuf;
		for (i=0; i < 512; i++)	/* save 1st sector of pattern */
			*bp++ = *cp++;
		rd_error = 0;
	}
	{
	initcmdblk(WDWRITE,(int)nblkcyl,LONGBUFPADDR,sn);
	wdwait();				/* wait for i/o complete */
	i = geterrbits();
	if (i & 0x7F)
	{
		wrt_blk_err(getblknum(),i);
		sn = getblknum() + 1L;
		rd_error++;
	}
	if (soft_error(i))
	{
		cp = (char *) LONGBUFPADDR;
		for (j = 0;j < (int)nblkcyl; ++j) {
			wdblkinit(cp, ' '); /* fill 512 byte block of ' ' */
			cp += 512;
		}
		initcmdblk(WDREAD,(int)nblkcyl,LONGBUFPADDR,sn);
		wdwait();			/* wait for i/o complete */
		i = geterrbits();
		if (i & 0x7F)
		{
			rd_error++;
			rd_blk_err(getblknum(),i);
			sn = getblknum() + 1L;
		}
		if (soft_error(i))
		{
			cp = (char *) LONGBUFPADDR;
			for (j = 0;(j < (int)nblkcyl) && !rd_error; ++j) {
			    if (patno == 7)
				wdblkinit(&databuf,j);
			    if (wdcompare(cp, databuf))
			    {
				bp = databuf;
				for (i = 0; (i < 512) && !rd_error; ++i)
				   if (*cp++ != *bp++) {
	  				compare_error(sn,--cp,--bp,i);
					rd_error++;
				   }
			    }
			    sn++;
			    cp += 512;			/* next block */
			}
		}
	    }
	}
	} while (rd_error && !check_stop_test());
}
