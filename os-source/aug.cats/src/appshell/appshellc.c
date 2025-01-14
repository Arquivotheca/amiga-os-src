#include "appshell_internal.h"

void kprintf (void *, ...);

#define	DB(x)	;
#define	DD(x)	;
#define	DH(x)	;

#define el lib->lib

extern ULONG SysBase;

/* Shared system libraries that we need */
ULONG DOSBase, GfxBase, IconBase, IntuitionBase, RexxSysBase;
ULONG UtilityBase, WorkbenchBase, GadToolsBase, AppShellBase;
ULONG IFFParseBase, PrefsBase, AppObjectsBase, LayersBase;
ULONG AmigaGuideBase;

/* Required for DeadKeyConvert processing */
struct ConsoleDevice *ConsoleDevice;

LONG LibExpunge(struct ExtLibrary *lib);

/* This function converts register-parameter hook calling
 * convention into standard C conventions.  It requires a C
 * compiler that supports registerized parameters, such as
 * SAS/C 5.xx or greater.
 */
VOID __asm dispatchHook (
    register __a0 struct Hook * h,
    register __a2 struct AppInfo *ai,
    register __a1 struct AppFunction *af)
{
    DH (kprintf("dispatchHook : hook 0x%lx, ai 0x%lx, msg 0x%lx\n", h, ai, af));
    dispatchRegs (h->h_SubEntry, ai, af->af_CmdLine, af->af_Attrs);
}

VOID __asm newdispatchHook (
    register __a0 struct Hook * h,
    register __a2 struct AppInfo *ai,
    register __a1 struct AppFunction *af)
{
    DH (kprintf("newdispatchHook : hook 0x%lx, ai 0x%lx, msg 0x%lx\n", h, ai, af));
    (*h->h_SubEntry)(h, ai, af);
}

/* need proper shutdown! */
struct ExtLibrary *CInit(struct ExtLibrary *lib)
{
    struct LibBase *base;

    /* allocate the library base */
    if ((((struct Library *)SysBase)->lib_Version >= 36) &&
	(lib->base = base = (struct LibBase *)
	AllocMem (sizeof (struct LibBase), MEMF_CLEAR | MEMF_PUBLIC)))
    {
	AppShellBase = (ULONG) lib;

	/* required libraries */
	if (!(DOSBase = (ULONG)OpenLibrary("dos.library",36)))
	    return (NULL);
	if (!(GfxBase = (ULONG)OpenLibrary("graphics.library",36)))
	    return (NULL);
	if (!(LayersBase = (ULONG)OpenLibrary("layers.library",36)))
	    return (NULL);
	if (!(IconBase = (ULONG)OpenLibrary("icon.library",36)))
	    return (NULL);
	if (!(IntuitionBase = (ULONG)OpenLibrary("intuition.library",36)))
	    return (NULL);
	if (!(UtilityBase = (ULONG)OpenLibrary("utility.library",36)))
	    return (NULL);
	if (!(GadToolsBase = (ULONG)OpenLibrary("gadtools.library",36)))
	    return (NULL);
	if (!(IFFParseBase = (ULONG)OpenLibrary("iffparse.library", 0)))
	    return (NULL);
	if (!(PrefsBase = (ULONG) OpenLibrary ("prefs.library", 0L)))
	    return (NULL);
	if (!(AppObjectsBase = (ULONG) OpenLibrary ("appobjects.library", 36L)))
	    return (NULL);

	/* If the console isn't already setup, prepare it */
	if (!(OpenDevice ("console.device", -1L,
			  (struct IORequest *)&(base->lb_IOReq), 0L)))
	{
	    ConsoleDevice = (struct ConsoleDevice *)base->lb_IOReq.io_Device;
	}
	else
	{
	    return (NULL);
	}

	/* Optional libraries */
	WorkbenchBase = (ULONG) OpenLibrary ("workbench.library",36);
	RexxSysBase = (ULONG) OpenLibrary ("rexxsyslib.library", 0L);
	AmigaGuideBase = (ULONG) OpenLibrary ("amigaguide.library", 0L);

	/* Initialize the tool list */
	NewList (&(base->lb_ToolList));
	NewList (&(base->lb_AppList));

	/* Initialize the data lock */
	InitSemaphore (&(base->lb_Lock));

	/* Initialize the default hook */
	DH (kprintf("dispatchHook 0x%lx, newdispatchHook 0x%lx\n", dispatchHook, newdispatchHook));
	base->lb_Hook.h_Entry = dispatchHook;
	base->lb_NewHook.h_Entry = newdispatchHook;
    }
    else
    {
	return (NULL);
    }

    /* make the library public */
    AddLibrary((struct Library *)lib);
    return(lib);
}

struct ExtLibrary *LibOpen(struct ExtLibrary *lib, LONG version)
{
    DB( kprintf ("Open appshell.library 0x%lx Count %ld\n",lib,lib->lib.lib_OpenCnt) );

    ++el.lib_OpenCnt;
    el.lib_Flags &= ~LIBF_DELEXP;

    return(lib);
}

LibClose(struct ExtLibrary *lib)
{
    DB( kprintf ("Close appshell.library 0x%lx Count %ld\n",lib,lib->lib.lib_OpenCnt) );

    if(lib->lib.lib_OpenCnt)
    {
	--lib->lib.lib_OpenCnt;
	return(NULL);
    }
    if(lib->lib.lib_Flags & LIBF_DELEXP)
    {
	return(LibExpunge(lib));
    }

    return(NULL);
}

LONG LibExpunge(struct ExtLibrary *lib)
{
    struct LibBase *base = lib->base;
    LONG seg;

    DB( kprintf ("Expunge appshell.library 0x%lx\n",  lib) );

    if(lib->lib.lib_OpenCnt)
    {
	lib->lib.lib_Flags |= LIBF_DELEXP;
	return(NULL);
    }

    /* make sure that we have a library base to shutdown */
    if (base)
    {
	/* shutdown the console device */
	if (ConsoleDevice)
	{
	    CloseDevice ((struct IORequest *)&(base->lb_IOReq));
	    ConsoleDevice = NULL;
	}

	if (AppObjectsBase)
	    CloseLibrary ((struct Library *) AppObjectsBase);
	if (UtilityBase)
	    CloseLibrary ((struct Library *) UtilityBase);
	if (WorkbenchBase)
	    CloseLibrary ((struct Library *) WorkbenchBase);
	if (GadToolsBase)
	    CloseLibrary ((struct Library *) GadToolsBase);
	if (RexxSysBase)
	    CloseLibrary ((struct Library *) RexxSysBase);
	if (IFFParseBase)
	    CloseLibrary ((struct Library *) IFFParseBase);
	if (PrefsBase)
	    CloseLibrary ((struct Library *) PrefsBase);
	if (IconBase)
	    CloseLibrary ((struct Library *) IconBase);
	if (IntuitionBase)
	    CloseLibrary ((struct Library *) IntuitionBase);
	if (LayersBase)
	    CloseLibrary ((struct Library *) LayersBase);
	if (GfxBase)
	    CloseLibrary ((struct Library *) GfxBase);
	if (DOSBase)
	    CloseLibrary ((struct Library *) DOSBase);
	if (AmigaGuideBase)
	    CloseLibrary ((struct Library *) AmigaGuideBase);

	/* free the global variables */
	FreeMem (base, sizeof (struct LibBase));
    }

    seg = lib->seglist;

    /* remove the library from the public list */
    Remove((struct Node *)lib);

    return(seg);
}
