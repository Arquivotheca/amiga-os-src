/* amigaguidebase.c
 *
 */

#include "amigaguidebase.h"

/*****************************************************************************/

#define	DB(x)	;

/****** amigaguide.library/--background-- ****************************************
*
*   PURPOSE
*	The amigaguide.library provides a means whereby developers
*	can easily add on-line help abilities to their applications.
*
*******************************************************************************
*
* Created:  27-Feb-92, David N. Junod
*
*/

/*****************************************************************************/

struct Library *ASM LibInit (REG (d0) struct AmigaGuideLib * ag, REG (a0) BPTR seglist, REG (a6) struct Library * sysbase)
{
    DB (kprintf ("ag : libinit\n"));

    /********************************/
    /* Initialize all the variables */
    /********************************/

    ag->ag_SegList = seglist;
    ag->ag_SysBase = sysbase;

    InitSemaphore (&ag->ag_Lock);

    /*******************************************/
    /* Open all the ROM libraries that we need */
    /*******************************************/

    if (sysbase->lib_Version >= 39)
    {
	DB (kprintf ("ag : open ROM libraries\n"));
	ag->ag_DOSBase       = OpenLibrary ("dos.library", 39);
	ag->ag_UtilityBase   = OpenLibrary ("utility.library", 39);
	ag->ag_GfxBase       = OpenLibrary ("graphics.library", 39);
	ag->ag_IntuitionBase = OpenLibrary ("intuition.library", 39);
	ag->ag_GadToolsBase  = OpenLibrary ("gadtools.library", 39);
	ag->ag_WorkbenchBase = OpenLibrary ("workbench.library", 39);
	return ((struct Library *) ag);
    }
    return (NULL);
}

/*****************************************************************************/

LONG ASM LibOpenLVO (REG (a6) struct AmigaGuideLib * ag)
{
    struct ExecBase *eb = (struct ExecBase *) ag->ag_SysBase;
    LONG retval = (LONG) ag;
    BOOL success = FALSE;
    BYTE nest;

    DB (kprintf ("ag : libopen\n"));

    /**********/
    /* Forbid */
    /**********/
    ObtainSemaphore (&ag->ag_Lock);
    nest = eb->TDNestCnt;
    Permit ();

    /* Use an internal use counter */
    ag->ag_UseCnt++;
    ag->ag_Lib.lib_Flags &= ~LIBF_DELEXP;

    if (ag->ag_UseCnt == 1)
    {
	/* Create a memory pool */
	if (ag->ag_MemoryPool = CreatePool (MEMF_PUBLIC | MEMF_CLEAR, 512, 512))
	{
	    /* Open disk libraries here */
	    DB (kprintf ("ag : open rexxsyslib.library\n"));
	    ag->ag_RexxSysBase = OpenLibrary ("rexxsyslib.library", 0);
	    DB (kprintf ("ag : open locale.library\n"));
	    if (ag->ag_LocaleBase = OpenLibrary ("locale.library", 38))
		ag->ag_Catalog = OpenCatalogA (NULL, "amigaguide.catalog", NULL);

	    DB (kprintf ("ag : open asl.library\n"));
	    if (ag->ag_AslBase = OpenLibrary ("asl.library", 38))
	    {
		DB (kprintf ("ag : open datatypes.library\n"));
		if (ag->ag_DataTypesBase = OpenLibrary ("datatypes.library", 39))
		{
		    DB (kprintf ("ag : open amigaguide.datatype\n"));
		    if (ag->ag_AmigaGuideClassBase = OpenLibrary ("datatypes/amigaguide.datatype", 39))
		    {
			DB (kprintf ("ag : init window class\n"));
			if (ag->ag_WindowClass = initWindowClass (ag))
			{
			    success = TRUE;
			}
		    }
		}
	    }
	}
	else
	{
	    DB (kprintf ("ag : couldn't create memory pool\n"));
	}
    }
    else
    {
	success = TRUE;
    }

    if (success)
    {
	DB (kprintf ("ag : obtain xref\n"));
	ag->ag_Token = ObtainXRefToken (ag);
    }
    else
    {
	DB (kprintf ("ag : failure\n"));
	CloseLibrary (ag->ag_AmigaGuideClassBase);
	CloseLibrary (ag->ag_DataTypesBase);
	CloseLibrary (ag->ag_AslBase);
	CloseLibrary (ag->ag_LocaleBase);
	CloseLibrary (ag->ag_RexxSysBase);
	ag->ag_AmigaGuideClassBase = ag->ag_DataTypesBase = NULL;
	ag->ag_AslBase = ag->ag_LocaleBase = ag->ag_RexxSysBase = NULL;
	ag->ag_UseCnt--;
	retval = NULL;
    }

    /**********/
    /* Permit */
    /**********/
    eb->TDNestCnt = nest;
    ReleaseSemaphore (&ag->ag_Lock);

    return (retval);
}

/*****************************************************************************/

LONG ASM LibCloseLVO (REG (a6) struct AmigaGuideLib * ag)
{
    struct ExecBase *eb = (struct ExecBase *) ag->ag_SysBase;
    BYTE nest;

    /**********/
    /* Forbid */
    /**********/
    ObtainSemaphore (&ag->ag_Lock);
    nest = eb->TDNestCnt;
    Permit ();

    if (ag->ag_UseCnt)
	ag->ag_UseCnt--;

    if (ag->ag_UseCnt == 0)
    {
	/* Free up the things we opened */
	if (ag->ag_LocaleBase)
	{
	    CloseCatalog (ag->ag_Catalog);
	    CloseLibrary (ag->ag_LocaleBase);
	}
	CloseLibrary (ag->ag_AmigaGuideClassBase);
	CloseLibrary (ag->ag_DataTypesBase);
	CloseLibrary (ag->ag_RexxSysBase);
	CloseLibrary (ag->ag_AslBase);
	freeWindowClass (ag, ag->ag_WindowClass);
	DeletePool (ag->ag_MemoryPool);
	ag->ag_RexxSysBase = ag->ag_LocaleBase = ag->ag_DataTypesBase = NULL;
	ag->ag_AmigaGuideClassBase = NULL;
	ag->ag_WindowClass = NULL;
	ag->ag_MemoryPool = NULL;
	ag->ag_Catalog = NULL;
    }

    /**********/
    /* Permit */
    /**********/
    eb->TDNestCnt = nest;
    ReleaseSemaphore (&ag->ag_Lock);

    if (ag->ag_Lib.lib_Flags & LIBF_DELEXP)
	return (LibExpungeLVO (ag));

    return (NULL);
}

/*****************************************************************************/

LONG ASM LibExpungeLVO (REG (a6) struct AmigaGuideLib * ag)
{
    BPTR seg = ag->ag_SegList;

    if (ag->ag_UseCnt)
    {
	ag->ag_Lib.lib_Flags |= LIBF_DELEXP;
	return (NULL);
    }

    Remove ((struct Node *) ag);

    CloseLibrary (ag->ag_WorkbenchBase);
    CloseLibrary (ag->ag_GadToolsBase);
    CloseLibrary (ag->ag_IntuitionBase);
    CloseLibrary (ag->ag_GfxBase);
    CloseLibrary (ag->ag_UtilityBase);
    CloseLibrary (ag->ag_DOSBase);

    /* Free the library base */
    FreeMem ((APTR)((ULONG)(ag) - (ULONG)(ag->ag_Lib.lib_NegSize)), ag->ag_Lib.lib_NegSize + ag->ag_Lib.lib_PosSize);

    return ((LONG) seg);
}
