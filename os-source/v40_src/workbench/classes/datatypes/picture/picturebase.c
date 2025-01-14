/*****************************************************************************/
/******************* THIS FILE IS OBSOLETE ***********************************/
/*****************************************************************************/

/* picturebase.c
 *
 */

#define	LIBRARY_PRIMITIVES	TRUE

#include "picturebase.h"

/*****************************************************************************/

Class *ASM ObtainPictureEngine (REG (a6) struct PictureLib * pl)
{
    return (pl->pl_Class);
}

/*****************************************************************************/

struct Library *ASM LibInit (REG (d0) struct PictureLib * pl, REG (a0) BPTR seglist, REG (a6) struct Library * sysbase)
{
    pl->pl_SegList = seglist;
    pl->pl_SysBase = sysbase;

    if (pl->pl_IntuitionBase = OpenLibrary ("intuition.library", 37))
    {
	pl->pl_GfxBase = OpenLibrary ("graphics.library", 37);
	pl->pl_DOSBase = OpenLibrary ("dos.library", 37);
	pl->pl_UtilityBase = OpenLibrary ("utility.library", 37);
	InitSemaphore (&(pl->pl_Lock));
	return (pl);
    }

    return (NULL);
}

/*****************************************************************************/

LONG ASM LibOpen (REG (a6) struct PictureLib * pl)
{
    struct ExecBase *eb = (struct ExecBase *) pl->pl_SysBase;
    LONG retval = (LONG) pl;
    BOOL success = TRUE;
    BYTE nest;

    /* obtain the semaphore */
    ObtainSemaphore (&(pl->pl_Lock));

    /* Permit() */
    nest = eb->TDNestCnt;
    Permit ();

    /* Use an internal use counter */
    pl->pl_UsageCnt++;
    pl->pl_Lib.lib_Flags &= ~LIBF_DELEXP;

    if (pl->pl_UsageCnt == 1)
    {
	if (pl->pl_Class == NULL)
	{
	    success = FALSE;
	    if (pl->pl_DataTypesBase = OpenLibrary ("datatypes.library", 0))
		if (pl->pl_IFFParseBase = OpenLibrary ("iffparse.library", 0))
		    if (pl->pl_Class = initClass (pl))
			success = TRUE;
	}
    }

    if (!success)
    {
	pl->pl_UsageCnt--;
	retval = NULL;
    }

    /* Restore the Forbid() state */
    eb->TDNestCnt = nest;

    /* release the lock */
    ReleaseSemaphore (&(pl->pl_Lock));

    return (retval);
}

/*****************************************************************************/

LONG ASM LibClose (REG (a6) struct PictureLib * pl)
{
    if (pl->pl_UsageCnt)
	pl->pl_UsageCnt--;

    if ((pl->pl_UsageCnt == 0) && pl->pl_Class)
    {
	if (FreeClass (pl->pl_Class))
	{
	    CloseLibrary (pl->pl_DataTypesBase);
	    CloseLibrary (pl->pl_IFFParseBase);
	    pl->pl_Class = NULL;
	}
	else
	    pl->pl_Lib.lib_Flags |= LIBF_DELEXP;
    }

    if (pl->pl_Lib.lib_Flags & LIBF_DELEXP)
	return (LibExpunge (pl));

    return (NULL);
}

/*****************************************************************************/

LONG ASM LibExpunge (REG (a6) struct PictureLib * pl)
{

    if (pl->pl_UsageCnt)
    {
	pl->pl_Lib.lib_Flags |= LIBF_DELEXP;
	return (NULL);
    }

    if (pl->pl_Class)
    {
	if (FreeClass (pl->pl_Class))
	{
	    CloseLibrary (pl->pl_DataTypesBase);
	    CloseLibrary (pl->pl_IFFParseBase);
	    pl->pl_Class = NULL;
	}
	else
	{
	    pl->pl_Lib.lib_Flags |= LIBF_DELEXP;
	    return (NULL);
	}
    }

    Remove ((struct Node *) pl);

    CloseLibrary (pl->pl_UtilityBase);
    CloseLibrary (pl->pl_DOSBase);
    CloseLibrary (pl->pl_GfxBase);
    CloseLibrary (pl->pl_IntuitionBase);

    return ((LONG) pl->pl_SegList);
}
