/* private.c
 *
 */

#include "memcheck.h"

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

struct MemHeader *AllocMemHeader (struct GlobalData * gd)
{
    struct MemHeader *mh;

    /* Allocate the memory headers */
    if (mh = AllocVec (sizeof (struct MemHeader), MEMF_CLEAR))
    {
	/* Make it one big memheader */
	mh->mh_Upper = (APTR) 0xFFFFFFFF;
    }
    return (mh);
}

/*****************************************************************************/

void FreePrivateList (struct GlobalData * gd)
{
    struct MemHeader *mh = gd->gd_MemHeader;
    struct MemChunk *nmc, *mc;	/* Used for stepping through the memory chunks */
    ULONG bytes;

    Forbid ();

    /* See if we have any chunks to put away */
    if (mh->mh_First)
    {
	/* Step through the chunk list */
	mc = mh->mh_First;
	while (mc)
	{
	    /* Remember the fields since we're going to trash them */
	    nmc = mc->mc_Next;
	    bytes = mc->mc_Bytes;

	    /* Mung the memory */
	    DB (kprintf ("  FreeMem %08lx, %ld\n", mc, bytes));
	    MungMem ((char *) mc, bytes, 0xDEADBEEF);

	    /* Add it back to the system free list */
	    (*gd->gd_FreeMem) (mc, bytes, SysBase);

	    mc = nmc;
	}

	/* Clear the list */
	mh->mh_First = NULL;
	mh->mh_Free = 0;
    }

    Permit ();
}

/*****************************************************************************/

void FreeMemHeader (struct GlobalData * gd, struct MemHeader * mh)
{
    /* Show that we are trying to exit */
    gd->gd_Flags |= GDF_CLOSING;

    /* Free all the entries from the private list */
    FreePrivateList (gd);

    /* Free our memory */
    FreeVec (mh);
}
