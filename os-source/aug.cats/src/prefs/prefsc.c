#include "prefsbase.h"

#define	DB(x)	;
void kprintf(void *, ...);

#define EL lib->el_Lib
	
/* shared libraries that are needed */
struct Library *PrefsBase;
struct Library *DOSBase;
struct Library *UtilityBase;
struct Library *IFFParseBase;
struct Library *GfxBase;
struct Library *IntuitionBase;

#define PREFS_SCREENMODE_N	"screenmode.prefs"
#define PREFS_PALETTE_N		"palette.ilbm"
#define	PREFS_POINTER_N		"pointer.ilbm"
#define	PREFS_BUSYPOINTER_N	"busypointer.ilbm"

/* initialize the library */
struct ExtLibrary * __saveds __asm
CInit ( register __a1 struct ExtLibrary *lib )
{
    struct LibBase *base;
	
    /* allocate the library base */
    if (lib->el_Base = base = (struct LibBase *)
	AllocMem (sizeof (struct LibBase), MEMF_CLEAR | MEMF_PUBLIC))
    {
	/* make our base valid */
	PrefsBase = (struct Library *) lib;

	/* open other required shared libraries */
	if(!(DOSBase=OpenLibrary("dos.library",36)))
	    return (NULL);
	if(!(UtilityBase=OpenLibrary("utility.library",36)))
	    return (NULL);
	if(!(IFFParseBase=OpenLibrary("iffparse.library",0)))
	    return (NULL);
	if(!(GfxBase=OpenLibrary("graphics.library",0)))
	    return (NULL);
	if(!(IntuitionBase=OpenLibrary("intuition.library",0)))
	    return (NULL);
	
	/* use for exclusive access to global data */
	InitSemaphore (&(base->lb_Lock));

	/* Get a lock on the default system directory */
	/* Maybe this should float somehow */
	if (base->lb_SysPrefs = Lock ("ENV:Sys", ACCESS_READ))
	{
	    /* Set up the name table */
	    base->lb_PrefName[PREFS_SCREENMODE]	= PREFS_SCREENMODE_N;
	    base->lb_PrefName[PREFS_PALETTE]	= PREFS_PALETTE_N;
	    base->lb_PrefName[PREFS_POINTER]	= PREFS_POINTER_N;
	    base->lb_PrefName[PREFS_BUSYPOINTER]= PREFS_BUSYPOINTER_N;
	}
	else
	{
	    return (NULL);
	}
    }
	
    /* make the library public */
    AddLibrary((struct Library *)lib);
    return(lib);
}
	
/* open the library */
struct ExtLibrary * __saveds __asm
LibOpen ( register __a6 struct ExtLibrary *lib, register __d0 LONG version )
{
    ++EL.lib_OpenCnt;
    EL.lib_Flags &= ~LIBF_DELEXP;
    return(lib);
}
	
/* expunge the library from the system */
LONG __saveds __asm
LibExpunge ( register __a6 struct ExtLibrary *lib )
{
    struct LibBase *base = lib->el_Base;
    LONG seg = lib->el_SegList;
	
    if(EL.lib_OpenCnt)
    {
	EL.lib_Flags |= LIBF_DELEXP;
	return(NULL);
    }
	
    /* remove the library from the public list */
    DB (kprintf("remove\n"));
    Remove((struct Node *)lib);
	
    /* make sure that we have a library base to shutdown */
    if (base)
    {
	DB (kprintf("sysprefs 0x%lx\n", base->lb_SysPrefs));
	if (base->lb_SysPrefs)
	{
	    /* Unlock the system preference drawer */
	    UnLock (base->lb_SysPrefs);
	}

	/* close other shared libraries */
	DB (kprintf("close libraries\n"));
	if(DOSBase)
	    CloseLibrary(DOSBase);
	if(UtilityBase)
	    CloseLibrary(UtilityBase);
	if(IFFParseBase)
	    CloseLibrary(IFFParseBase);
	if(IntuitionBase)
	    CloseLibrary(IntuitionBase);
	if(GfxBase)
	    CloseLibrary(GfxBase);
	
	/* free the global variables */
	DB (kprintf("freemem\n"));
	FreeMem (base, sizeof (struct LibBase));
    }
	
    /* return the SegList */
    return(seg);
}
	
/* close the library */
LONG __saveds __asm
LibClose( register __a6 struct ExtLibrary *lib )
{
    if(EL.lib_OpenCnt)
    {
	--EL.lib_OpenCnt;
	return(NULL);
    }
	
    if(EL.lib_Flags & LIBF_DELEXP)
    {
	return(LibExpunge(lib));
    }
	
    return(NULL);
}
