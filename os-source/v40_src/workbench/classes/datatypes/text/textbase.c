/* textbase.c
 *
 */

#include "textbase.h"

/****** text.datatype/text.datatype ****************************************
*
*    NAME
*	text.datatype -- Root data type for text.
*
*    FUNCTION
*	The text.datatype is the super-class for any text related classes.
*
*    METHODS
*	OM_NEW -- Create a new text object.
*
*	OM_GET -- Obtain the value of an attribute.
*
*	OM_SET -- Set the values of multiple attributes.
*
*	OM_UPDATE -- Update the values of multiple attributes.
*
*	OM_DISPOSE -- Dispose of a text object.
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
*	GM_RENDER -- Cause the text to render.
*
*	DTM_PROCLAYOUT -- Layout (remap) the text on the application's
*	    process.
*
*	DTM_FRAMEBOX -- Obtain the display environment that the text
*	    requires.
*
*	DTM_SELECT -- Select an area in the text.
*
*	DTM_CLEARSELECTED -- Deselect the selected area of the text.
*
*	DTM_COPY -- Copy the selected area of the text to the clipboard
*	    as FTXT.  If no area is selected, then the entire text
*	    is copied.
*
*	DTM_PRINT -- Print the selected area of the text.  If no area
*	    is selected, then the entire text is printed.
*
*	DTM_WRITE -- Write the selected area of the text to a file.
*	    If no area is selected, then the entire text is saved.
*
*    TAGS
*	DTA_TextAttr (struct TextAttr *) -- Text attribute to use for
*	    the text.
*
*	    Applicability is (ISG).
*
*	DTA_TextFont (struct TextFont *) -- Text font to use for
*	    the text.
*
*	    Applicability is (G).
*
*	TDTA_Buffer (STRPTR) -- Pointer to the text data.
*
*	    Applicability is (ISG).
*
*	TDTA_BufferLen (ULONG) -- Length of text data.
*
*	    Applicability is (ISG).
*
*	TDTA_LineList (struct List *) -- List of lines.  The elements
*	    of the list are Line structures (see <datatype/textclass.h>
*
*	    Applicability is (G).
*
*	TDTA_WordSelect (STRPTR) -- Word that has been double-clicked on.
*
*	    Applicability is (NU).
*
*	TDTA_WordDelim (STRPTR) -- Characters used deliminate words.
*
*	    Applicability is (IS).
*
*	TDTA_WordWrap (BOOL) -- Used to turn word wrap on and off.
*	    Defaults to off.
*
*	    Applicability is (ISG).
*
*    SEE ALSO
*	ascii.datatype
*
*******************************************************************************
*
* Created:  27-Feb-92, David N. Junod
*
*/

Class *ASM ObtainTextEngine (REG (a6) struct TextLib * txl)
{
    return (txl->txl_Class);
}

/*****************************************************************************/

struct Library *ASM LibInit (REG (d0) struct TextLib * txl, REG (a0) BPTR seglist, REG (a6) struct Library * sysbase)
{
    txl->txl_SegList = seglist;
    txl->txl_SysBase = sysbase;

    InitSemaphore (&txl->txl_Lock);

    if (SysBase->lib_Version >= 39)
    {
	txl->txl_IntuitionBase = OpenLibrary ("intuition.library", 39);
	txl->txl_GfxBase       = OpenLibrary ("graphics.library", 39);
	txl->txl_DOSBase       = OpenLibrary ("dos.library", 39);
	txl->txl_UtilityBase   = OpenLibrary ("utility.library", 39);
	txl->txl_LayersBase    = OpenLibrary ("layers.library", 39);

	return (txl);
    }

    return (NULL);
}

/*****************************************************************************/

LONG ASM LibOpen (REG (a6) struct TextLib * txl)
{
    LONG retval = (LONG) txl;
    BOOL success = TRUE;

    ObtainSemaphore (&txl->txl_Lock);

    /* Use an internal use counter */
    txl->txl_Lib.lib_OpenCnt++;
    txl->txl_Lib.lib_Flags &= ~LIBF_DELEXP;

    if (txl->txl_Lib.lib_OpenCnt == 1)
    {
	if (txl->txl_Class == NULL)
	{
	    success = FALSE;
	    if (txl->txl_IFFParseBase = OpenLibrary ("iffparse.library", 0))
		if (txl->txl_DataTypesBase = OpenLibrary ("datatypes.library", 0))
		    if (txl->txl_Class = initClass (txl))
			success = TRUE;
	}
    }

    if (!success)
    {
	txl->txl_Lib.lib_OpenCnt--;
	retval = NULL;
    }

    ReleaseSemaphore (&txl->txl_Lock);

    return (retval);
}

/*****************************************************************************/

LONG ASM LibClose (REG (a6) struct TextLib * txl)
{
    LONG retval = NULL;

    ObtainSemaphore (&txl->txl_Lock);

    if (txl->txl_Lib.lib_OpenCnt)
	txl->txl_Lib.lib_OpenCnt--;

    if ((txl->txl_Lib.lib_OpenCnt == 0) && txl->txl_Class)
    {
	if (FreeClass (txl->txl_Class))
	{
	    CloseLibrary (txl->txl_DataTypesBase);
	    CloseLibrary (txl->txl_IFFParseBase);
	    txl->txl_Class = NULL;
	}
	else
	    txl->txl_Lib.lib_Flags |= LIBF_DELEXP;
    }

    if (txl->txl_Lib.lib_Flags & LIBF_DELEXP)
	retval = LibExpunge (txl);

    ReleaseSemaphore (&txl->txl_Lock);

    return (retval);
}

/*****************************************************************************/

LONG ASM LibExpunge (REG (a6) struct TextLib * txl)
{
    BPTR seg = txl->txl_SegList;

    if (txl->txl_Lib.lib_OpenCnt)
    {
	txl->txl_Lib.lib_Flags |= LIBF_DELEXP;
	return (NULL);
    }

    Remove ((struct Node *) txl);

    CloseLibrary (txl->txl_UtilityBase);
    CloseLibrary (txl->txl_DOSBase);
    CloseLibrary (txl->txl_GfxBase);
    CloseLibrary (txl->txl_IntuitionBase);

    FreeMem ((APTR)((ULONG)(txl) - (ULONG)(txl->txl_Lib.lib_NegSize)), txl->txl_Lib.lib_NegSize + txl->txl_Lib.lib_PosSize);

    return ((LONG) seg);
}
