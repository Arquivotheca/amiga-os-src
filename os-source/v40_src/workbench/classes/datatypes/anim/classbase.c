/* classbase.c
 *
 */

#include "classbase.h"

/*****************************************************************************/

#define	DB(x)	;

/****** anim.datatype/anim.datatype ****************************************
*
*    NAME
*	anim.datatype -- data type for ANIM movies.
*
*    FUNCTION
*	The anim data type, a sub-class of the movie.datatype, is used
*	to load ANIM files.
*
*    METHODS
*	OM_NEW -- Create a new movie object from an ANIM.  The
*	    source may be either a file or the clipboard.
*
*    SEE ALSO
*	animation.datatype
*
*******************************************************************************
*
* Created:  27-Feb-92, David N. Junod
*
*/

/*****************************************************************************/

Class *ASM ObtainAnimEngine (REG (a6) struct ClassBase *cb)
{
    return (cb->cb_Class);
}

/*****************************************************************************/

struct Library *ASM LibInit (REG (d0) struct ClassBase *cb, REG (a0) BPTR seglist, REG (a6) struct Library * sysbase)
{
    cb->cb_SegList = seglist;
    SysBase = (struct ExecBase *) sysbase;
    InitSemaphore (&cb->cb_Lock);
    if (((struct Library *)SysBase)->lib_Version >= 39)
    {
	IntuitionBase = OpenLibrary ("intuition.library",39);
	GfxBase       = OpenLibrary ("graphics.library", 39);
	DOSBase       = OpenLibrary ("dos.library",      39);
	UtilityBase   = OpenLibrary ("utility.library",  39);
	return (struct Library *) cb;
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
	    IFFParseBase = OpenLibrary ("iffparse.library", 0);
	    DataTypesBase = OpenLibrary ("datatypes.library", 0);

	    if (DataTypesBase && IFFParseBase)
	    {
		if (cb->cb_SuperClassBase = OpenLibrary ("datatypes/animation.datatype", 39))
		{
		    if (cb->cb_Class = initClass (cb))
		    {
			success = TRUE;
		    }
		    else
		    {
			DB (kprintf ("can't init anim class\n"));
		    }
		}
		else
		{
		    DB (kprintf ("can't open datatypes/animation.datatype\n"));
		}
	    }
	    else
	    {
		DB (kprintf ("can't open datatypes.library or iffparse.library\n"));
	    }
	}
    }

    if (!success)
    {
	CloseLibrary (cb->cb_SuperClassBase);
	CloseLibrary (DataTypesBase);
	CloseLibrary (IFFParseBase);
	IFFParseBase = DataTypesBase = NULL;
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
	    CloseLibrary (cb->cb_SuperClassBase);
	    CloseLibrary (DataTypesBase);
	    CloseLibrary (IFFParseBase);
	    cb->cb_Class = NULL;
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
    BPTR seg = cb->cb_SegList;

    if (cb->cb_Lib.lib_OpenCnt)
    {
	cb->cb_Lib.lib_Flags |= LIBF_DELEXP;
	return (NULL);
    }

    Remove ((struct Node *) cb);

    CloseLibrary (UtilityBase);
    CloseLibrary (DOSBase);
    CloseLibrary (GfxBase);
    CloseLibrary (IntuitionBase);

    FreeMem ((APTR)((ULONG)(cb) - (ULONG)(cb->cb_Lib.lib_NegSize)), cb->cb_Lib.lib_NegSize + cb->cb_Lib.lib_PosSize);

    return ((LONG) seg);
}
