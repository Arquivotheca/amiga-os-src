/* asciibase.c
 *
 */

#define	LIBRARY_PRIMITIVES	TRUE

#include "asciibase.h"

/****** ascii.datatype/ascii.datatype ****************************************
*
*    NAME
*	ascii.datatype -- data type for ANSI text.
*
*    FUNCTION
*	The ASCII data type, a sub-class of the text.datatype, is used
*	to load ANSI text files.
*
*    METHODS
*	OM_NEW -- Create a new text object from ANSI characters.  The
*	    source may be either a file or the clipboard.
*
*    SEE ALSO
*	text.datatype.
*
*******************************************************************************
*
* Created:  27-Feb-92, David N. Junod
*
*/

/*****************************************************************************/

Class *ASM ObtainASCIIEngine (REG (a6) struct ASCIILib * ascii)
{
    return (ascii->ascii_Class);
}

/*****************************************************************************/

struct Library *ASM LibInit (REG (d0) struct ASCIILib * ascii, REG (a0) BPTR seglist, REG (a6) struct Library * sysbase)
{
    ascii->ascii_SegList = seglist;
    ascii->ascii_SysBase = sysbase;

    if (ascii->ascii_IntuitionBase = OpenLibrary ("intuition.library", 39))
    {
	ascii->ascii_GfxBase = OpenLibrary ("graphics.library", 39);
	ascii->ascii_DOSBase = OpenLibrary ("dos.library", 39);
	ascii->ascii_UtilityBase = OpenLibrary ("utility.library", 39);
	ascii->ascii_LayersBase = OpenLibrary ("layers.library", 39);
	InitSemaphore (&(ascii->ascii_Lock));
	return (ascii);
    }

    return (NULL);
}

/*****************************************************************************/

LONG ASM LibOpen (REG (a6) struct ASCIILib * ascii)
{
    LONG retval = (LONG) ascii;
    BOOL success = TRUE;

    ObtainSemaphore (&ascii->ascii_Lock);

    /* Use an internal use counter */
    ascii->ascii_Lib.lib_OpenCnt++;
    ascii->ascii_Lib.lib_Flags &= ~LIBF_DELEXP;

    if (ascii->ascii_Lib.lib_OpenCnt == 1)
    {
	if (ascii->ascii_Class == NULL)
	{
	    success = FALSE;
	    if (ascii->ascii_DataTypesBase = OpenLibrary ("datatypes.library", 0))
		if (ascii->ascii_SuperClassBase = OpenLibrary ("datatypes/text.datatype", 0))
		    if (ascii->ascii_Class = initClass (ascii))
			success = TRUE;
	}
    }

    if (!success)
    {
	CloseLibrary (ascii->ascii_SuperClassBase);
	CloseLibrary (ascii->ascii_DataTypesBase);
	ascii->ascii_DataTypesBase = ascii->ascii_SuperClassBase = NULL;
	ascii->ascii_Lib.lib_OpenCnt--;
	retval = NULL;
    }

    ReleaseSemaphore (&ascii->ascii_Lock);

    return (retval);
}

/*****************************************************************************/

LONG ASM LibClose (REG (a6) struct ASCIILib * ascii)
{
    LONG retval = NULL;

    ObtainSemaphore (&ascii->ascii_Lock);

    if (ascii->ascii_Lib.lib_OpenCnt)
	ascii->ascii_Lib.lib_OpenCnt--;

    if ((ascii->ascii_Lib.lib_OpenCnt == 0) && ascii->ascii_Class)
    {
	if (FreeClass (ascii->ascii_Class))
	{
	    CloseLibrary (ascii->ascii_SuperClassBase);
	    CloseLibrary (ascii->ascii_DataTypesBase);
	    ascii->ascii_Class = NULL;
	}
	else
	    ascii->ascii_Lib.lib_Flags |= LIBF_DELEXP;
    }

    if (ascii->ascii_Lib.lib_Flags & LIBF_DELEXP)
	retval = LibExpunge (ascii);

    ReleaseSemaphore (&ascii->ascii_Lock);

    return retval;
}

/*****************************************************************************/

LONG ASM LibExpunge (REG (a6) struct ASCIILib * ascii)
{
    BPTR seg = ascii->ascii_SegList;

    if (ascii->ascii_Lib.lib_OpenCnt)
    {
	ascii->ascii_Lib.lib_Flags |= LIBF_DELEXP;
	return (NULL);
    }

    Remove ((struct Node *) ascii);

    CloseLibrary (ascii->ascii_UtilityBase);
    CloseLibrary (ascii->ascii_DOSBase);
    CloseLibrary (ascii->ascii_LayersBase);
    CloseLibrary (ascii->ascii_GfxBase);
    CloseLibrary (ascii->ascii_IntuitionBase);

    FreeMem ((APTR)((ULONG)(ascii) - (ULONG)(ascii->ascii_Lib.lib_NegSize)), ascii->ascii_Lib.lib_NegSize + ascii->ascii_Lib.lib_PosSize);

    return ((LONG) seg);
}
