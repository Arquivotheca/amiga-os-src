/* mctest.c
 * Test MemCheck
 *
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <internal/memcheckheader.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <clib/alib_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>

#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>

/*****************************************************************************/

extern struct Library *SysBase, *DOSBase;

/*****************************************************************************/

int CXBRK (void){return (0);}				/* Disable Lattice CTRL/C handling */
int chkabort (void){return (0);}			/* really */

/*****************************************************************************/

struct MemCheckInfo *GetMemCheckInfo (void)
{
    struct MemCheckInfo *mci = NULL;
    struct MsgPort *mp;

    Forbid ();
    if (mp = FindPort (NAME))
	mci = (struct MemCheckInfo *)((struct Task *)mp->mp_SigTask)->tc_UserData;
    Permit ();

    return mci;
}

/*****************************************************************************/

VOID testtask (void)
{
    UBYTE *tooshort;

    if (tooshort = AllocMem (0, MEMF_CLEAR))
	FreeMem (tooshort, 0);
    DeleteTask (FindTask (NULL));
}
/*****************************************************************************/

void main (void)
{
    UBYTE oddname[32], beefname[32];
    struct MemCheckInfo *mci;
    UBYTE *tooshort;
    UBYTE *ptr;

    /* See if MemCheck is even running. */
    if (mci = GetMemCheckInfo ())
    {
	printf ("Running MemCheck %d.%d\n", mci->mci_Version, mci->mci_Revision);
	printf ("MemChunk: %ld  Pre-Size: %ld  Post-Size: %ld\n",
		 mci->mci_InfoSize, mci->mci_PreSize, mci->mci_PostSize);
	printf ("MCTest TCB: %08lx\n\n", FindTask (NULL));

#if 1
	if (tooshort = AllocMem (0, MEMF_CLEAR))
	    FreeMem (tooshort, 0);

	if (ptr = (UBYTE *) AllocMem (16, MEMF_CLEAR))
	{
	    printf ("Allocated 16 bytes at 0x%lx\n", ptr);
	    printf ("Attempting NULL pointer FreeMem\n");
	    FreeMem (NULL, 16);
	    printf ("Attempting NULL size FreeMem\n");
	    FreeMem (ptr, NULL);
	    tooshort = ptr;
	    tooshort += 4;
	    FreeMem (tooshort, 16);
	    printf ("Trashing memory after allocation.\n");
	    strcpy (ptr, "Lonely Hearts Club Band");
	    printf ("Trashing memory before allocation\n");
	    ptr -= 19;
	    strcpy (ptr, "Sergeant's Pepper ");
	    ptr += 19;
	    printf ("Freeing memory with wrong size\n");
	    FreeMem (ptr, 14);
	}

	if (ptr = AllocMem (16, MEMF_CLEAR))
	{
	    strcpy (ptr, "This is sixteens");
	    FreeMem (ptr, 16);
	}

	printf ("FreeMem bogus address\n");
	FreeMem (0x08, 1000);

	printf ("FreeVec bogus address\n");
	FreeVec (0x08);

	printf ("Odd address named task \"oddname\" to Alloc 0 bytes - test on 68000\n");
	strcpy (oddname, " oddname");
	CreateTask (&oddname[1], 0, testtask, 4000);
	Delay (100);

	printf ("0xDEADBEEF named task to Alloc 0 bytes- test on 68000\n");
	beefname[0] = 0xDE;
	beefname[1] = 0xAD;
	beefname[2] = 0xBE;
	beefname[3] = 0xEF;
	CreateTask (beefname, 0, testtask, 4000);

	printf ("Will exit in 10 seconds - hope tasks have deleted selves\n");
	Delay (500);
#endif
    }
    else
	printf ("must be running MemCheck to run this test\n");
}
