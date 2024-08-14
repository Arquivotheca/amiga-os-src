#include <exec/types.h>
#include <lattice/stdio.h>
#include "ddefs.h"

ULONG getblknum()
{
	register WDCMD *cbp;

	cbp = (WDCMD *) CMDBLKPADDR;
	return((ULONG)	(( (ULONG) (cbp->c_lunladd2) & 0x1f) << 16) +
			(ULONG)(cbp->c_ladd1 << 8) + (ULONG)cbp->c_ladd0);
}
