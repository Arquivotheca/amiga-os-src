/* timetest.c
 *
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <libraries/dos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#define MINARGS 3

extern struct ExecBase *SysBase;
extern struct Library *DOSBase;

UBYTE *vers = "\0$VER: timetest 37.1";
UBYTE *Copyright = "timetest v37.1\nCopyright (c) 1990 Commodore-Amiga, Inc.  All Rights Reserved";
UBYTE *usage = "Usage: timetest <allocsize> <iterations>";

void main (int argc, char **argv)
{
    struct DateStamp DI, DE;
    LONG ticks, secs, mins;
    ULONG size, iterate;
    ULONG *array;
    int k;

    if (((argc) && (argc < MINARGS)) || (argv[argc - 1][0] == '?'))
    {
	printf ("%s\n%s\n", Copyright, usage);
    }
    else
    {
	size = atoi (argv[1]);
	if (!size)
	    size = 512;

	iterate = atoi (argv[2]);
	if (!iterate)
	    iterate = 1;

	if (array = AllocVec (sizeof (ULONG) * iterate, MEMF_CLEAR))
	{
	    printf ("Will allocate %ld bytes %ld times\n", size, iterate);

	    DateStamp (&DI);
	    for (k = 0; k < iterate; k++)
		array[k] = AllocVec (size, NULL);
	    DateStamp (&DE);

	    ticks = CompareDates (&DI, &DE);
	    secs   = ticks / 50;
	    mins   = secs / 60;
	    ticks -= secs * 50;
	    printf ("AllocVec()   %02ld:%02ld:%02ld\n", mins, secs, ticks);

	    DateStamp (&DI);
	    for (k = 0; k < iterate; k++)
		FreeVec (array[k]);
	    DateStamp (&DE);

	    ticks = CompareDates (&DI, &DE);
	    secs   = ticks / 50;
	    mins   = secs / 60;
	    ticks -= secs * 50;
	    printf ("FreeVec()    %02ld:%02ld:%02ld\n", mins, secs, ticks);

	    FreeVec (array);
	}
    }
}
