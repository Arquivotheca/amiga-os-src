#include <exec/types.h>
#include <lattice/stdio.h>
#include "ddefs.h"

/*
 * set the drive and controller parameters for the WD hard disk.
 * this uses the the values in the parameter table in this file.
 * these have to match the order and type of those found in rom.c ...
 * make sure that both the floppy and WD command blocks are cleared.
 */

extern	WDINFO	wdinfo[];
extern	int	typeno;		/* drive type selected from table */
extern	int	unit;		/* which physical drive ? */
extern	void	wdwait();

wdsetparam()
{
	register WDINFO *ip;
	int i;

		/* initialize command block */
	initcmdblk((char)(unit ? WDSDP1 : WDSDP),0,LONGBUFPADDR, 0);
	ip = (WDINFO *)LONGBUFPADDR;
	*ip = wdinfo[typeno];
	wdwait();				/* wait for i/o complete */

	i = geterrbits();
	if (i & 0x7F) {
		derror("init",i);
	}
}
