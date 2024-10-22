/* classbase.c
 *
 */

#include "classbase.h"

/****** picture.datatype/picture.datatype ****************************************
*
*    NAME
*	picture.datatype -- root data type for pictures.
*
*    FUNCTION
*	The picture.datatype is the super-class for any picture related
*	classes.
*
*    METHODS
*	OM_NEW -- Create a new picture object.
*
*	OM_GET -- Obtain the value of an attribute.
*
*	OM_SET -- Set the values of multiple attributes.
*
*	OM_UPDATE -- Update the values of multiple attributes.
*
*	OM_DISPOSE -- Dispose of a picture object.
*
*	GM_LAYOUT -- Layout the object and notify the application of the
*	    title and size.
*
*	GM_HITTEST -- Determine if the object has been hit with the
*	    mouse.
*
*	GM_GOACTIVE -- Tell the object to go active.
*
*	GM_HANDLEINPUT -- Handle input.
*
*	GM_RENDER -- Cause the graphic to render.
*
*	DTM_PROCLAYOUT -- Layout (remap) the picture on the application's
*	    process.
*
*	DTM_FRAMEBOX -- Obtain the display environment that the picture
*	    requires.
*
*	DTM_SELECT -- Select an area in the picture.
*
*	DTM_CLEARSELECTED -- Deselect the selected area of the picture.
*
*	DTM_COPY -- Copy the selected area of the picture to the clipboard
*	    as an ILBM.  If no area is selected, then the entire picture
*	    is copied.
*
*	DTM_PRINT -- Print the selected area of the picture.  If no area
*	    is selected, then the entire picture is printed.
*
*	DTM_WRITE -- Write the selected area of the picture to a file as an
*	    ILBM.  If no area is selected, then the entire picture is
*	    saved.
*
*    TAGS
*	OBP_Precison (ULONG) -- Precision to use when obtaining colors.
*	    See the PRECISION_ defines in <graphics/view.h>.
*
*	    Applicability is (I).
*
*	PDTA_ModeID (ULONG) -- Set and get the graphic mode id of the
*	    picture.
*
*	    Applicability is (ISG).
*
*	PDTA_BitMapHeader (struct BitMapHeader *) -- Set and get the
*	    base information for the picture.  BitMapHeader is defined in
*	    <datatypes/pictureclass.h>
*
*	    Applicability is (G).
*
*	PDTA_BitMap (struct BitMap *) -- Pointer to a class-allocated
*	    bitmap, that will end up being freed by the picture class in the
*	    OM_DISPOSE method.
*
*	    Applicability is (ISG).
*
*	PDTA_ColorRegisters (struct ColorRegister *) -- Color table.
*
*	    Applicability is (G).
*
*	PDTA_CRegs (ULONG *) -- Color table to use with SetRGB32CM().
*
*	    Applicability is (G).
*
*	PDTA_GRegs (ULONG *) -- Color table.
*
*	    Applicability is (G).
*
*	PDTA_ColorTable (ULONG *) -- Shared pen table.
*
*	    Applicability is (G).
*
*	PDTA_ColorTable2 (ULONG *) -- Shared pen table.
*
*	    Applicability is (G).
*
*	PDTA_Allocated (ULONG) --  Number of shared colors allocated.
*
*	    Applicability is (G).
*
*	PDTA_NumColors (WORD) -- Number of colors used by the picture.
*
*	    Applicability is (ISG).
*
*	PDTA_NumAlloc (WORD) -- Number of colors allocated by the picture.
*
*	    Applicability is (G).
*
*	PDTA_Remap (BOOL) -- Indicate whether the picture should be
*	    remapped or not.
*
*	    Applicability is (I).
*
*	PDTA_Screen (struct Screen *) -- Pointer to the screen to remap
*	    the picture to.  Only used if the object is not going to be
*	    added to a window.
*
*	    Applicability is (IS).
*
*	PDTA_FreeSourceBitMap (BOOL) -- Indicate whether the source
*	    bitmap should be freed immediately by the picture.datatype
*	    after the GM_LAYOUT method is called.
*
*	    Applicability is (IS).
*
*	PDTA_Grab (Point *) -- Pointer to a Point structure, that
*	    defines the grab point of the picture.
*
*	    Applicability is (ISG).
*
*	PDTA_DestBitMap (struct BitMap *) -- Pointer to the remapped
*	    bitmap.
*
*	    Applicability is (G).
*
*	PDTA_ClassBitMap (struct BitMap *) --
*
*	    Applicability is (ISG).
*
*	PDTA_NumSparse (UWORD) -- Number of entries in the sparse color
*	    table.
*
*	    Applicability is (I).
*
*	PDTA_SparseTable (UBYTE *) -- Pointer to a table of pen numbers
*	    indicating which colors should be used when remapping the
*	    picture.  This array must contain as many entries as indicated
*	    by the PDTA_NumSparse tag.
*
*	    Applicability is (I).
*
*******************************************************************************
*
* Created:  27-Feb-92, David N. Junod
*
*/

/*****************************************************************************/

struct Library *ASM LibInit (REG (d0) struct ClassBase *cb, REG (a0) BPTR seglist, REG (a6) struct Library * sysbase)
{
    cb->cb_SegList = seglist;
    SysBase = sysbase;

    InitSemaphore (&(cb->cb_Lock));

    if (SysBase->lib_Version >= 39)
    {
	IntuitionBase = OpenLibrary ("intuition.library",39);
	GfxBase       = OpenLibrary ("graphics.library", 39);
	DOSBase       = OpenLibrary ("dos.library",      39);
	UtilityBase   = OpenLibrary ("utility.library",  39);
	return (cb);
    }

    return (NULL);
}

/*****************************************************************************/

LONG ASM LibOpen (REG (a6) struct ClassBase *cb)
{
    struct ExecBase *eb = (struct ExecBase *) cb->cb_SysBase;
    LONG retval = (LONG) cb;
    BOOL success = TRUE;
    BYTE nest;

    /************/
    /* FORBID() */
    /************/
    ObtainSemaphore (&(cb->cb_Lock));
    nest = eb->TDNestCnt;
    Permit ();

    /* Use an internal use counter */
    cb->cb_UsageCnt++;
    cb->cb_Lib.lib_Flags &= ~LIBF_DELEXP;

    if (cb->cb_UsageCnt == 1)
    {
	if (cb->cb_Class == NULL)
	{
	    success = FALSE;
	    if (DataTypesBase = OpenLibrary ("datatypes.library", 0))
		if (IFFParseBase = OpenLibrary ("iffparse.library", 0))
		    if (cb->cb_Class = initClass (cb))
			success = TRUE;
	}
    }

    if (!success)
    {
	cb->cb_UsageCnt--;
	retval = NULL;
    }

    /************/
    /* PERMIT() */
    /************/
    eb->TDNestCnt = nest;
    ReleaseSemaphore (&(cb->cb_Lock));

    return (retval);
}

/*****************************************************************************/

LONG ASM LibClose (REG (a6) struct ClassBase *cb)
{
    struct ExecBase *eb = (struct ExecBase *) cb->cb_SysBase;
    LONG retval = NULL;
    BYTE nest;

    /************/
    /* FORBID() */
    /************/
    ObtainSemaphore (&(cb->cb_Lock));
    nest = eb->TDNestCnt;
    Permit ();

    if (cb->cb_UsageCnt)
	cb->cb_UsageCnt--;

    if ((cb->cb_UsageCnt == 0) && cb->cb_Class)
    {
	if (FreeClass (cb->cb_Class))
	{
	    CloseLibrary (DataTypesBase);
	    CloseLibrary (IFFParseBase);
	    cb->cb_Class = NULL;
	}
	else
	    cb->cb_Lib.lib_Flags |= LIBF_DELEXP;
    }

    if (cb->cb_Lib.lib_Flags & LIBF_DELEXP)
	retval = LibExpunge (cb);

    /************/
    /* PERMIT() */
    /************/
    eb->TDNestCnt = nest;
    ReleaseSemaphore (&(cb->cb_Lock));

    return (retval);
}

/*****************************************************************************/

LONG ASM LibExpunge (REG (a6) struct ClassBase *cb)
{
    BPTR seg = cb->cb_SegList;

    if (cb->cb_UsageCnt)
    {
	cb->cb_Lib.lib_Flags |= LIBF_DELEXP;
	return (NULL);
    }

    if (cb->cb_Class)
    {
	if (FreeClass (cb->cb_Class))
	{
	    CloseLibrary (DataTypesBase);
	    CloseLibrary (IFFParseBase);
	    cb->cb_Class = NULL;
	}
	else
	{
	    cb->cb_Lib.lib_Flags |= LIBF_DELEXP;
	    return (NULL);
	}
    }

    Remove ((struct Node *) cb);

    CloseLibrary (UtilityBase);
    CloseLibrary (DOSBase);
    CloseLibrary (GfxBase);
    CloseLibrary (IntuitionBase);

    FreeMem ((APTR)((ULONG)(cb) - (ULONG)(cb->cb_Lib.lib_NegSize)), cb->cb_Lib.lib_NegSize + cb->cb_Lib.lib_PosSize);

    return ((LONG) seg);
}
