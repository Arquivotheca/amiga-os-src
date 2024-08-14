/* classbase.c
 *
 */

#include "classbase.h"
#include "ilbm_rev.h"

/****** ilbm.datatype/ilbm.datatype ****************************************
*
*    NAME
*	ilbm.datatype -- data type for ILBM pictures.
*
*    FUNCTION
*	The ILBM data type, a sub-class of the picture.datatype, is used
*	to load ILBM IFF picture files.
*
*    METHODS
*	OM_NEW -- Create a new picture object from an ILBM IFF.  The
*	    source may be either a file or the clipboard.
*
*    SEE ALSO
*	picture.datatype.
*
*******************************************************************************
*
* Created:  27-Feb-92, David N. Junod
*
*/

/*****************************************************************************/

Class *ASM ObtainILBMEngine (REG (a6) struct ClassBase *cb)
{
    return (cb->cb_Class);
}

/*****************************************************************************/

struct Library *ASM LibInit (REG (d0) struct ClassBase *cb, REG (a0) BPTR seglist, REG (a6) struct Library * sysbase)
{
    cb->cb_SegList = seglist;
    cb->cb_SysBase = sysbase;
    cb->cb_Lib.lib_Revision = REVISION;
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
	    if (cb->cb_IFFParseBase = OpenLibrary ("iffparse.library", 0))
		if (cb->cb_DataTypesBase = OpenLibrary ("datatypes.library", 0))
		    if (cb->cb_SuperClassBase = OpenLibrary ("datatypes/picture.datatype", 0))
			if (cb->cb_Class = initClass (cb))
			    success = TRUE;
	}
    }

    if (!success)
    {
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
    BPTR seg = cb->cb_SegList;

    if (cb->cb_Lib.lib_OpenCnt)
    {
	cb->cb_Lib.lib_Flags |= LIBF_DELEXP;
	return (NULL);
    }

    Remove ((struct Node *) cb);

    CloseLibrary (cb->cb_UtilityBase);
    CloseLibrary (cb->cb_DOSBase);
    CloseLibrary (cb->cb_GfxBase);
    CloseLibrary (cb->cb_IntuitionBase);

    FreeMem ((APTR)((ULONG)(cb) - (ULONG)(cb->cb_Lib.lib_NegSize)), cb->cb_Lib.lib_NegSize + cb->cb_Lib.lib_PosSize);

    return ((LONG) seg);
}
