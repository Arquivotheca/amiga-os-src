#include "appobjectsbase.h"

#define EL lib->el_Lib

void kprintf(void *, ...);
#define	DB(x)	;

/* Function prototypes for the classes that we own */
Class *initLGroupGClass(VOID);
Class *initAFrameIClass(VOID);
Class *initMTextGClass (VOID);
Class *initMTGroupClass (VOID);
Class *initMTModClass (VOID);
Class *initActionGClass ( VOID );
Class *initSliderGClass (VOID);
Class *initTextIClass (VOID);
Class *initLabelGClass (VOID);
Class *initSysIClass (VOID);
Class *initScrollGClass(VOID);
Class *initMutExGClass(VOID);
Class *initColumnIClass(VOID);
Class *initViewGClass(VOID);
Class *initListViewGClass(VOID);

ULONG freeLGroupGClass(Class *cl);
ULONG freeAFrameIClass(Class *cl);
ULONG freeMTextGClass (Class * cl);
ULONG freeMTGroupClass (Class * cl);
ULONG freeMTModClass (Class *cl);
ULONG freeActionGClass ( Class *cl );
ULONG freeSliderGClass (Class * cl);
ULONG freeTextIClass (Class * cl);
ULONG freeLabelGClass (Class * cl);
ULONG freeSysIClass (Class * cl);
ULONG freeScrollGClass(Class *cl);
ULONG freeMutExGClass(Class *cl);
ULONG freeColumnIClass(Class *cl);
ULONG freeViewGClass(Class *cl);
ULONG freeListViewGClass(Class *cl);

/* shared libraries that are needed */
struct Library *AppObjectsBase;
struct Library *GfxBase;
struct Library *IntuitionBase;
struct Library *KeymapBase;
struct Library *UtilityBase;
struct Library *LayersBase;
struct Library *GadToolsBase;
struct Library *IFFParseBase;

/* initialize the library */
struct ExtLibrary * __saveds __asm
CInit ( register __a1 struct ExtLibrary *lib )
{
    struct LibBase *base;

    /* allocate the library base */
    if (lib->el_Base = base = (struct LibBase *)
	AllocMem (sizeof (struct LibBase), MEMF_CLEAR | MEMF_PUBLIC))
    {
	WORD i;

	/* make our base valid */
	AppObjectsBase = (struct Library *) lib;

	/* open other required shared libraries */
	if (!(IntuitionBase = OpenLibrary ("intuition.library",36)))
	    return (NULL);
	if (!(GfxBase = OpenLibrary ("graphics.library",36)))
	    return (NULL);
	if (!(KeymapBase = OpenLibrary ("keymap.library", 31)))
	    return (NULL);
	if (!(UtilityBase = OpenLibrary ("utility.library", 36)))
	    return (NULL);
	if (!(LayersBase = OpenLibrary ("layers.library", 36)))
	    return (NULL);
	if (!(GadToolsBase = OpenLibrary ("gadtools.library", 36)))
	    return (NULL);
	if (!(IFFParseBase = OpenLibrary ("iffparse.library", 0)))
	    return (NULL);

	/* Initialize all the classes that we own (order is important!!! */
	if (!(base->lb_Classes[CGTC_APPSYSI] = initSysIClass()))
	    return (NULL);
	if (!(base->lb_Classes[CGTC_TEXTI] = initTextIClass()))
	    return (NULL);
	if (!(base->lb_Classes[CGTC_LABELG] = initLabelGClass()))
	    return (NULL);
	if (!(base->lb_Classes[CGTC_SLIDER] = initSliderGClass()))
	    return (NULL);
	if (!(base->lb_Classes[CGTC_LGROUPG] = initLGroupGClass()))
	    return (NULL);
	if (!(base->lb_Classes[CGTC_AFRAMEI] = initAFrameIClass()))
	    return (NULL);
	if (!(base->lb_Classes[CGTC_ACTION] = initActionGClass()))
	    return (NULL);
	if (!(base->lb_Classes[CGTC_SCROLLER] = initScrollGClass()))
	    return (NULL);
	if (!(base->lb_Classes[CGTC_MTEXTG] = initMTextGClass()))
	    return (NULL);
	if (!(base->lb_Classes[CGTC_MTMODEL] = initMTModClass()))
	    return (NULL);
	if (!(base->lb_Classes[CGTC_MTGROUP] = initMTGroupClass()))
	    return (NULL);
	if (!(base->lb_Classes[CGTC_MUTEX] = initMutExGClass()))
	    return (NULL);

	if (!(base->lb_Classes[CGTC_COLUMNI] = initColumnIClass()))
	    return (NULL);
	if (!(base->lb_Classes[CGTC_VIEWG] = initViewGClass()))
	    return (NULL);
	if (!(base->lb_Classes[CGTC_LISTVIEWG] = initListViewGClass()))
	    return (NULL);

	/* use for exclusive access to global data */
	for (i = 0; i < CGT_MAX_LOCKS; i++)
	    InitSemaphore (&(base->lb_Lock[i]));
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

LONG closeclass (Class *cl, ULONG (*close_func)(Class *))
{
    LONG retval = FALSE;
    UBYTE buff[128];

    /* Make the class unavailable */
    RemoveClass (cl);

    buff[0] = 0;
    if (cl->cl_ID)
    {
	strcpy (buff, cl->cl_ID);
    }
    DB (kprintf ("Close [%s] Object %ld, SubClass %ld\n",
	buff, cl->cl_ObjectCount, cl->cl_SubclassCount));

    if ((cl->cl_ObjectCount == 0) &&
	(cl->cl_SubclassCount == 0))
    {
	retval = (*(close_func))(cl);
    }

    DB (kprintf("return 0x%lx\n", retval));

    return (retval);
}

LONG CheckClasses(VOID)
{
    struct LibBase *base = ((struct ExtLibrary *)AppObjectsBase)->el_Base;
    LONG retval = TRUE;
    Class *cl;

    DB (kprintf("CheckClasses\n"));

    if (cl = base->lb_Classes[CGTC_LISTVIEWG])
    {
	if (retval = closeclass (cl, freeListViewGClass))
	{
	    base->lb_Classes[CGTC_LISTVIEWG] = NULL;
	}
    }

    if (cl = base->lb_Classes[CGTC_VIEWG])
    {
	if (retval = closeclass (cl, freeViewGClass))
	{
	    base->lb_Classes[CGTC_VIEWG] = NULL;
	}
    }

    if (cl = base->lb_Classes[CGTC_COLUMNI])
    {
	if (retval = closeclass (cl, freeColumnIClass))
	{
	    base->lb_Classes[CGTC_COLUMNI] = NULL;
	}
    }

    if (cl = base->lb_Classes[CGTC_MUTEX])
    {
	if (retval = closeclass (cl, freeMutExGClass))
	{
	    base->lb_Classes[CGTC_MUTEX] = NULL;
	}
    }

    if (cl = base->lb_Classes[CGTC_MTGROUP])
    {
	if (retval = closeclass (cl, freeMTModClass))
	{
	    base->lb_Classes[CGTC_MTGROUP] = NULL;
	}
    }

    if (retval && (cl = base->lb_Classes[CGTC_MTMODEL]))
    {
	if (retval = closeclass (cl, freeMTGroupClass))
	{
	    base->lb_Classes[CGTC_MTMODEL] = NULL;
	}
    }

    if (retval && (cl = base->lb_Classes[CGTC_MTEXTG]))
    {
	if (retval = closeclass (cl, freeMTextGClass))
	{
	    base->lb_Classes[CGTC_MTEXTG] = NULL;
	}
    }

    if (retval && (cl = base->lb_Classes[CGTC_SCROLLER]))
    {
	if (retval = closeclass (cl, freeScrollGClass))
	{
	    base->lb_Classes[CGTC_SCROLLER] = NULL;
	}
    }

    if (retval && (cl = base->lb_Classes[CGTC_ACTION]))
    {
	if (retval = closeclass (cl, freeActionGClass))
	{
	    base->lb_Classes[CGTC_ACTION] = NULL;
	}
    }

    if (retval && (cl = base->lb_Classes[CGTC_AFRAMEI]))
    {
	if (retval = closeclass (cl, freeAFrameIClass))
	{
	    base->lb_Classes[CGTC_AFRAMEI] = NULL;
	}
    }

    if (retval && (cl = base->lb_Classes[CGTC_LGROUPG]))
    {
	if (retval = closeclass (cl, freeLGroupGClass))
	{
	    base->lb_Classes[CGTC_LGROUPG] = NULL;
	}
    }

    if (retval && (cl = base->lb_Classes[CGTC_SLIDER]))
    {
	if (retval = closeclass (cl, freeSliderGClass))
	{
	    base->lb_Classes[CGTC_SLIDER] = NULL;
	}
    }

    if (retval && (cl = base->lb_Classes[CGTC_LABELG]))
    {
	if (retval = closeclass (cl, freeLabelGClass))
	{
	    base->lb_Classes[CGTC_LABELG] = NULL;
	}
    }

    if (retval && (cl = base->lb_Classes[CGTC_TEXTI]))
    {
	if (retval = closeclass (cl, freeTextIClass))
	{
	    base->lb_Classes[CGTC_TEXTI] = NULL;
	}
    }

    if (retval && (cl = base->lb_Classes[CGTC_APPSYSI]))
    {
	if (retval = closeclass (cl, freeSysIClass))
	{
	    base->lb_Classes[CGTC_APPSYSI] = NULL;
	}
    }

    return (retval);
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
	return (NULL);
    }

    /* See if anyone is relying on our classes */
    if (!(CheckClasses()))
    {
	EL.lib_Flags |= LIBF_DELEXP;
	return (NULL);
    }

    /* remove the library from the public list */
    Remove((struct Node *)lib);

    /* make sure that we have a library base to shutdown */
    if (base)
    {
	/* Close other shared libraries */
	if (IFFParseBase)
	    CloseLibrary (IFFParseBase);
	if (GadToolsBase)
	    CloseLibrary (GadToolsBase);
	if (GfxBase)
	    CloseLibrary (GfxBase);
	if (IntuitionBase)
	    CloseLibrary (IntuitionBase);
	if (LayersBase)
	    CloseLibrary (LayersBase);
	if (KeymapBase)
	    CloseLibrary (KeymapBase);
	if (UtilityBase)
	    CloseLibrary (UtilityBase);

	/* free the global variables */
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
