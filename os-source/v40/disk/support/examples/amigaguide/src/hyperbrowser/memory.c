/* memory.c
 *
 */

#include "hyperbrowser.h"

/*****************************************************************************/

#define HEXASC  39

#define HEXSIZE 61
#define OUTSIZE 512

void showmemory (struct GlobalData * gd, ULONG address)
{
    char *ptr = (char *) address;
    char outbuf[OUTSIZE];
    char *msg = NULL;
    int counter = 0;
    int cyc = 0;
    ULONG *mem;
    int i, max;
    int c;

    /* Build the title */
    strcpy (gd->gd_Node, "@{b}Memory@{ub}\n\n");

    if (address)
    {
	mem = ((ULONG *)address)-1;
	max = MIN (*mem, 384);

	if (max == *mem)
	    bprintf (gd, "%08lx, %ld\n", address, *mem);
	else
	    bprintf (gd, "%08lx\n", address);

	for (i = 0; i < max; i++)
	{
	    c = (int *)(*ptr & 0xFF);
	    ptr++;

	    if (!cyc)
	    {
		/* Time to init the output buffer */
		memset(outbuf, ' ', HEXSIZE);
		outbuf[HEXSIZE-1] = 0;
	    }
	    counter++;

	    msg = outbuf+(cyc<<1)+(cyc>>2);
	    msg[0] = "0123456789ABCDEF"[c>>4];
	    msg[1] = "0123456789ABCDEF"[c&15];
	    if (((c+1)&0x7f) <= ' ') c = '.';

	    msg = "%04lx: %s\n";
	    outbuf[HEXASC+cyc] = c;
	    cyc++;
	    if (cyc != 16) continue;

	    cyc = 0;
	    bprintf (gd, msg, counter, outbuf);
	    msg = NULL;
	}

	if (msg)
	{
	    bprintf (gd, msg, counter, outbuf);
	}
    }
    else
    {
	strcat (gd->gd_Node, "NULL pointer\n");
    }
}
