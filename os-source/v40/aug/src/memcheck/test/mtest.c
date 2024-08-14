#include <exec/types.h>
#include <exec/memory.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <clib/exec_protos.h>

#include <pragmas/exec_pragmas.h>

extern struct Library *SysBase;

void main (void)
{
    BOOL going = TRUE;
    char *mem = NULL;
    UBYTE buff[80];
    ULONG size = 0;

    while (going)
    {
	printf ("size> ");
	gets (buff);

	if (strnicmp (buff, "q", 1) == 0)
	{
	    going = FALSE;
	}
	else
	{
	    if (mem)
		FreeMem (mem, size);

	    size = atoi (buff);
	    printf ("  %ld\n", size);

	    mem = AllocMem (size, MEMF_CLEAR);
	    printf ("  %08lx, %ld\n", mem, size);
	}
    }

    if (mem)
	FreeMem (mem, size);
}
