/* classbase.c
 *
 */

#include "classbase.h"

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

Class *ASM ObtainBMPEngine (REG (a6) struct ClassBase *cb)
{
    return (cb->cb_Class);
}

/*****************************************************************************/

struct Library *ASM LibInit (REG (d0) struct ClassBase *cb, REG (a0) BPTR seglist, REG (a6) struct Library * sysbase)
{
    cb->cb_SegList = seglist;
    cb->cb_SysBase = sysbase;
    InitSemaphore (&cb->cb_Lock);
    if (cb->cb_SysBase->lib_Version >= 39)
    {
	cb->cb_IntuitionBase = OpenLibrary ("intuition.library",39);
	cb->cb_GfxBase       = OpenLibrary ("graphics.library", 39);
	cb->cb_DOSBase       = OpenLibrary ("dos.library",      39);
	cb->cb_UtilityBase   = OpenLibrary ("utility.library",  39);
	return cb;
    }
    else
    {
	return NULL;
    }
}

/*****************************************************************************/

LONG ASM LibOpen (REG (a6) struct ClassBase *cb)
{
    LONG retval = (LONG) cb;
    BOOL success = TRUE;

    ObtainSemaphore (&cb->cb_Lock);

    /* Use an internal use counter */
    cb->cb_Lib.lib_OpenCnt++;
    cb->cb_Lib.lib_Flags &= ~LIBF_DELEXP;

    if (cb->cb_Lib.lib_OpenCnt == 1)
    {
	if (cb->cb_Class == NULL)
	{
	    success = FALSE;
	    if (cb->cb_IFFParseBase = OpenLibrary ("iffparse.library", 39))
		if (cb->cb_DataTypesBase = OpenLibrary ("datatypes.library", 39))
		    if (cb->cb_SuperClassBase = OpenLibrary ("datatypes/picture.datatype", 39))
			if (cb->cb_Class = initClass (cb))
			    success = TRUE;
	}
    }

    if (!success)
    {
	CloseLibrary (cb->cb_SuperClassBase);
	CloseLibrary (cb->cb_DataTypesBase);
	CloseLibrary (cb->cb_IFFParseBase);
	cb->cb_IFFParseBase = cb->cb_DataTypesBase = cb->cb_SuperClassBase = NULL;

	cb->cb_Lib.lib_OpenCnt--;
	retval = NULL;
    }

    ReleaseSemaphore (&cb->cb_Lock);

    return (retval);
}

/*****************************************************************************/

LONG ASM LibClose (REG (a6) struct ClassBase *cb)
{
    LONG retval = NULL;

    ObtainSemaphore (&cb->cb_Lock);

    if (cb->cb_Lib.lib_OpenCnt)
	cb->cb_Lib.lib_OpenCnt--;

    if ((cb->cb_Lib.lib_OpenCnt == 0) && cb->cb_Class)
    {
	if (FreeClass (cb->cb_Class))
	{
	    cb->cb_Class = NULL;
	    CloseLibrary (cb->cb_SuperClassBase);
	    CloseLibrary (cb->cb_DataTypesBase);
	    CloseLibrary (cb->cb_IFFParseBase);
	}
	else
	{
	    cb->cb_Lib.lib_Flags |= LIBF_DELEXP;
	}
    }

    if (cb->cb_Lib.lib_Flags & LIBF_DELEXP)
	retval = LibExpunge (cb);

    ReleaseSemaphore (&cb->cb_Lock);

    return (retval);
}

/*****************************************************************************/

LONG ASM LibExpunge (REG (a6) struct ClassBase *cb)
{
    Remove ((struct Node *) cb);

    CloseLibrary (cb->cb_UtilityBase);
    CloseLibrary (cb->cb_DOSBase);
    CloseLibrary (cb->cb_GfxBase);
    CloseLibrary (cb->cb_IntuitionBase);

    return ((LONG) cb->cb_SegList);
}
