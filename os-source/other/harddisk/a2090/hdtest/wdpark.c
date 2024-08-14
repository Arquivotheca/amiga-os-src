#include <exec/types.h>
#include <lattice/stdio.h>
#include "ddefs.h"

extern	int	delay_val;	/* Time to wait for next command */
extern	int	errors;		/* total # of errors curent pass */
extern	int	typeno;		/* drive type selected from table */
extern	int	unit;		/* which physical drive ? */
extern	WDINFO	wdinfo[];
extern	void	wdwait();

/*
 * Park the drive's heads in it's shipping zone
 */
wdpark(lun)
register unsigned int lun;
{
	register int i;
	register DISK_TYPE *wdp;
	long logical_addr;

	unit = lun;
	if (lun != 0 && lun != 1)	/* valid luns ? */
	{
		puts("illegal parameters for park!\n");
		return(++errors);
	}
	check_disk_type();		/* determine type of disk present */
	wdp = &wdinfo[typeno].p_type; /* point to correct drive information */
	wdp->d_headhicyl =	/* tell controller more cyls exist */
		(wdp->d_headhicyl & 0xF0) + (wdp->d_park_cyl >> 8);
	wdp->d_cyl = wdp->d_park_cyl & 0xFF;
	delay_val =5;
	wdsetparam();
	delay_val =30;
	initcmdblk(WDREST,1,BUFPADDR, 0L); /* restore drive */
	wdwait();				/* wait for i/o complete */
	i = geterrbits() & 0xff;
	if (i & 0x7f) {
		puts("error 0x"); putbhex(i);
		puts(" on restore.\n");
		++errors;
	}
	logical_addr = (wdp->d_park_cyl *	/* compute park block */
			  (long)wdp->d_sectors *
			 (wdp->d_headhicyl>>4)) - 1;
	initcmdblk(WDSEEK,1,BUFPADDR,logical_addr); /* seek park cylinder */
	wdwait();				/* wait for i/o complete */
	i = geterrbits() & 0xff;
	if (i & 0x7f) {
		puts("error 0x"); putbhex(i);
		puts(" on park.\n");
		++errors;
	}
	else puts("Parked\n");
	return (errors == 0);
}
