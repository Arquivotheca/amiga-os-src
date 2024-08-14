/* datatypesbase.c
 *
 */

#define	LIBRARY_PRIMITIVES	TRUE

#include "datatypesbase.h"

/****** datatypes.library/--background-- ****************************************
*
*   PURPOSE
*	The datatypes.library provides transparent data handling
*	abilities to applications.  Application developers can register
*	their data format with datatypes.library and provide a class
*	library for handling their data within other applications.
*
*   OVERVIEW
*
*	* Object Oriented
*
*	datatypes.library implementation is object oriented, using the
*	boopsi functions of Intuition.  Each data class is implemented
*	as a shared system library.
*
*	* Embedded Objects
*
*	datatypes.library provides the ability to embed different object
*	types within an application.  For example, an application can
*	embed an picture object or even an AmigaGuide document browser
*	within their application's window.  Objects can also be embedded
*	within other objects.
*
*	* Gadget-like
*
*	Embedded objects are actually boopsi gadgets.  That means that
*	input handling is done on Intuition's task.  Time intensive
*	operations, such as layout when the window size changes, are
*	off-loaded to a sub-process.  Printing, clipboard operations,
*	file read/write are also off-loaded to a separate process on an
*	as-needed basis.
*
*	* Trigger Methods
*
*	Sometimes it is necessary for an application to provide
*	additional controls for navigating through an object.  For
*	example, with an AmigaGuide object it is necessary to have
*	controls for "Contents", "Index", "Browse >", "Browse <".  Each
*	class implements a method that returns the trigger methods, and
*	the appropriate labels, that a class supports.
*
*	* Format Conversion
*
*	As long as the objects are sub-classes of the same class, data
*	from one format can be written out as another format.  For
*	example, it is possible to read in an ILBM file and write out a
*	JPEG file, since both data types are sub-classes of PICTURE.
*
*	* Future Compatible
*
*	Each class implements a method that returns the supported
*	methods within a class.  This way an application can ask an
*	object if it is capable of any particular method, such as
*	DTM_WRITE for example.
*
*	* Data Type Detection
*
*	datatypes.library provides the ability to examine a file or
*	the clipboard to determine what type of data it contains.
*
*******************************************************************************
*
* Created:  27-Feb-92, David N. Junod
*
*/

struct Library *ASM LibInit (REG (d0) struct DataTypesLib * dtl, REG (a0) BPTR seglist, REG (a6) struct Library * sysbase)
{
    UWORD i;

    dtl->dtl_SegList = seglist;
    dtl->dtl_SysBase = sysbase;

    for (i = 0; i < MAX_LOCKS; i++)
	InitSemaphore (&(dtl->dtl_Lock[i]));

    if (sysbase->lib_Version >= 39)
    {
	dtl->dtl_DOSBase       = OpenLibrary ("dos.library", 39);
	dtl->dtl_UtilityBase   = OpenLibrary ("utility.library", 39);
	dtl->dtl_IntuitionBase = OpenLibrary ("intuition.library", 39);
	dtl->dtl_GfxBase       = OpenLibrary ("graphics.library", 39);
	dtl->dtl_LayersBase    = OpenLibrary ("layers.library", 39);

	return ((struct Library *) dtl);
    }
    return (NULL);
}

/*****************************************************************************/

LONG ASM LibOpen (REG (a6) struct DataTypesLib * dtl)
{
    LONG retval = (LONG) dtl;
    struct NamedObject *no;
    BOOL success = TRUE;

    ObtainSemaphore (&(dtl->dtl_Lock[MASTER_LOCK]));

    /* Use an internal use counter */
    dtl->dtl_UsageCnt++;
    dtl->dtl_Lib.lib_Flags &= ~LIBF_DELEXP;

    if (dtl->dtl_UsageCnt == 1)
    {
	if (dtl->dtl_DTClass == NULL)
	{
	    success = FALSE;

	    /* Now attempt to get a handle on the public token */
	    if (no = FindNamedObject (NULL, TOKEN_NAME, NULL))
	    {
		dtl->dtl_Token = (struct Token *) no->no_Object;
		ReleaseNamedObject (no);

		if (dtl->dtl_LocaleBase = OpenLibrary ("locale.library", 38))
		    dtl->dtl_Catalog = OpenCatalogA (NULL, "Sys/libs.catalog", NULL);

		if (dtl->dtl_IFFParseBase = OpenLibrary ("iffparse.library", 0))
		    if (dtl->dtl_DTClass = initDTClass (dtl))
			success = TRUE;
	    }
	}
    }

    if (!success)
    {
	dtl->dtl_UsageCnt--;
	retval = NULL;

	/* Close the classes */
	CloseClasses (dtl);

	/* Free up the things we opened */
	if (dtl->dtl_LocaleBase)
	{
	    CloseCatalog (dtl->dtl_Catalog);
	    CloseLibrary (dtl->dtl_LocaleBase);
	}

	CloseLibrary (dtl->dtl_IFFParseBase);

	/* Null bases */
	dtl->dtl_IFFParseBase = NULL;
	dtl->dtl_LocaleBase = NULL;
	dtl->dtl_Catalog = NULL;
	dtl->dtl_DTClass = NULL;
    }
    ReleaseSemaphore (&(dtl->dtl_Lock[MASTER_LOCK]));
    return (retval);
}

/*****************************************************************************/

#define	DTC	dtl->dtl_DTClass

BOOL ASM CloseClasses (REG (a6) struct DataTypesLib * dtl)
{
    if (DTC)
	if (FreeClass (DTC))
	    return (TRUE);
    return (FALSE);
}

/*****************************************************************************/

LONG ASM LibClose (REG (a6) struct DataTypesLib * dtl)
{
    if (dtl->dtl_UsageCnt)
	dtl->dtl_UsageCnt--;

    if ((dtl->dtl_UsageCnt == 0) && dtl->dtl_DTClass)
    {
	if (CloseClasses (dtl))
	{
	    /* Free up the things we opened */
	    if (dtl->dtl_LocaleBase)
	    {
		CloseCatalog (dtl->dtl_Catalog);
		CloseLibrary (dtl->dtl_LocaleBase);
	    }
	    CloseLibrary (dtl->dtl_IFFParseBase);

	    /* Null bases */
	    dtl->dtl_IFFParseBase = NULL;
	    dtl->dtl_LocaleBase = NULL;
	    dtl->dtl_Catalog = NULL;
	    dtl->dtl_DTClass = NULL;
	}
    }

    if (dtl->dtl_Lib.lib_Flags & LIBF_DELEXP)
	return (LibExpunge (dtl));

    return (NULL);
}

/*****************************************************************************/

LONG ASM LibExpunge (REG (a6) struct DataTypesLib * dtl)
{
    BPTR seg = dtl->dtl_SegList;

    if (dtl->dtl_UsageCnt)
    {
	dtl->dtl_Lib.lib_Flags |= LIBF_DELEXP;
	return (NULL);
    }
    else if (dtl->dtl_DTClass)
    {
	if (CloseClasses (dtl))
	{
	    /* Free up the things we opened */
	    if (dtl->dtl_LocaleBase)
	    {
		CloseCatalog (dtl->dtl_Catalog);
		CloseLibrary (dtl->dtl_LocaleBase);
	    }

	    CloseLibrary (dtl->dtl_IFFParseBase);

	    /* Null bases */
	    dtl->dtl_IFFParseBase = NULL;
	    dtl->dtl_LocaleBase = NULL;
	    dtl->dtl_Catalog = NULL;
	    dtl->dtl_DTClass = NULL;
	}
	else
	{
	    dtl->dtl_Lib.lib_Flags |= LIBF_DELEXP;
	    return (NULL);
	}
    }

    /* Remove the library base */
    Remove ((struct Node *) dtl);

    /* Close the libraries */
    CloseLibrary (dtl->dtl_LayersBase);
    CloseLibrary (dtl->dtl_GfxBase);
    CloseLibrary (dtl->dtl_IntuitionBase);
    CloseLibrary (dtl->dtl_UtilityBase);
    CloseLibrary (dtl->dtl_DOSBase);

    /* Free the library base */
    FreeMem ((APTR)((ULONG)(dtl) - (ULONG)(dtl->dtl_Lib.lib_NegSize)), dtl->dtl_Lib.lib_NegSize + dtl->dtl_Lib.lib_PosSize);

    return ((LONG) seg);
}
