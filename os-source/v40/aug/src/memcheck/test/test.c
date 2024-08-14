#include <exec/types.h>
#include <exec/memory.h>

#include <clib/exec_protos.h>

#include <pragmas/exec_pragmas.h>

extern struct Library *SysBase;

void main (void)
{
    char *ptr;

    printf ("try to AllocMem zero bytes\n");
    if (ptr = AllocMem (0, NULL))
    {
	printf ("succes\n");
	FreeMem (ptr, 0);
    }
    else
	printf ("failure\n");

    printf ("try to AllocMem %ld bytes\n", 0xC0000000);
    if (ptr = AllocMem (0xC0000000, NULL))
    {
	printf ("succes\n");
	FreeMem (ptr, 0xC0000000);
    }
    else
	printf ("failure\n");

    printf ("try to AllocVec zero bytes\n");
    if (ptr = AllocVec (0, NULL))
    {
	printf ("succes\n");
	FreeVec (ptr);
    }
    else
	printf ("failure\n");

    printf ("try to AllocVec %ld bytes\n", 0xC0000000);
    if (ptr = AllocVec (0xC0000000, NULL))
    {
	printf ("succes\n");
	FreeVec (ptr);
    }
    else
	printf ("failure\n");

    printf ("try to FreeMem 0x8,12\n");
    FreeMem (0x8,12);

    printf ("try to FreeVec 0x8\n");
    FreeVec (0x8);
}
