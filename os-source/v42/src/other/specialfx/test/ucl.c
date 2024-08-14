/******************************************************************************
*
*	$Id:  $
*
******************************************************************************/

/*******************************************************************/
/*        TO USE ONE OF THE FOLLOWING LIBRARIES, SET ITS FLAG TO 1 */
/*                            ELSE SET IT TO 0                     */
/*******************************************************************/

#define USE_EXEC 1
#define USE_DOS 1
#define USE_INTUITION 1
#define USE_GRAPHICS 1
#define USE_LAYERS 0
#define USE_ARP 0
#define USE_ICON 0
#define USE_DISKFONT 0
#define USE_IFFPARSE 0
#define LIBVERSION 39

/*******************************************************************/
/*             TO DISABLE CTRL-C HANDLING, SET NO_CTRL_C TO 1      */
/*******************************************************************/

#define NO_CTRL_C 1

/*******************************************************************/
/*             TO DISABLE CBACK OPTION, SET NO_CBACK TO 1          */
/*******************************************************************/

#define NO_CBACK 1

#if NO_CTRL_C == 1
int CXBRK (void) { return (0) ; }	/* disable ctrl-c */
#endif

/*******************************************************************/
/*             TO ENABLE DEBUG MESSAGES, SET DEBUG TO 1            */
/*******************************************************************/

#define DEBUG 0

#include <exec/types.h>
#if USE_EXEC == 1
#include <exec/exec.h>
#include <proto/exec.h>
#endif
#if USE_DOS == 1
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <proto/dos.h>
#endif
#if USE_INTUITION == 1
#include <intuition/screens.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <proto/intuition.h>
#endif
#if USE_GRAPHICS == 1
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/displayinfo.h>
#include <graphics/copper.h>
#include <graphics/gfxmacros.h>
#include <proto/graphics.h>
#endif
#if USE_LAYERS == 1
#include <graphics/clip.h>
#include <graphics/layers.h>
#include <proto/layers.h>
#endif
#if USE_ARP == 1
#include <arp/arpbase.h>
#include <arp/arpfunc.h>
#endif
#if USE_ICON == 1
#include <workbench/icon.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <proto/icon.h>
#endif
#if USE_DISKFONT == 1
#include <libraries/diskFont.h>
#include <proto/diskfont.h>
#endif
#if USE_IFFPARSE == 1
#include <libraries/iffparse.h>
#include <proto/iffparse.h>
#endif
#include <hardware/custom.h>

#include <stdio.h>
#include <stdlib.h>

/*********************************************************************/
/*                            GLOBAL VARIABLES                       */
/*********************************************************************/

#if USE_DOS == 1
/* DOSBase set up by Lattice */
/* struct DOSBase *DOSBase = NULL ; */
#endif
#if USE_INTUITION == 1
struct IntuitionBase *IntuitionBase = NULL ;
#endif
#if USE_GRAPHICS == 1
struct GfxBase *GfxBase = NULL ;
#endif
#if USE_LAYERS == 1
struct Library *LayersBase = NULL ;
#endif
#if USE_ARP == 1
struct ArpBase *ArpBase = NULL ;
#endif
#if USE_ICON == 1
struct Library *IconBase = NULL ;
#endif
#if USE_DISKFONT == 1
struct DiskFontBase *DiskfontBase = NULL ;
#endif
#if USE_IFFPARSE == 1
struct Library *IFFParseBase = NULL;
#endif
struct RDArgs *rdargs = NULL;

/*******************************************************************/
/*                            CBack Variables                      */
/*******************************************************************/

#if NO_CBACK == 0
long _stack = STACKSIZE ;
char *_procname = PROCNAME ;
long _priority = PROCPRI ;
long _BackGroundIO = 1 ;		/* perform background IO */
extern BPTR _Backstdout ;		/* file handle */
#endif

/**********************************************************************/
/*                                                                    */
/* void Error (char *String)                                          */
/* Print string and exit                                              */
/*                                                                    */
/**********************************************************************/

void Error (char *String)
{
    void CloseAll (void) ;
	
    printf (String) ;

/*  if (_Backstdout)
    {
	Forbid () ;
	Write (_Backstdout, String, strlen (String)) ;
	Permit () ;
    }*/

    CloseAll () ;
    exit(0) ;
}


/**********************************************************************/
/*                                                                    */
/* void Init ()                                                       */
/*                                                                    */
/* Opens all the required libraries                                   */
/* allocates all memory, etc.                                         */
/*                                                                    */
/**********************************************************************/

void Init ()
{
    #if USE_DOS == 1
    /* DOS library opened by Lattice */
    #endif

    #if USE_INTUITION == 1
    /* Open the intuition library.... */
    if ((IntuitionBase = (struct IntuitionBase *)OpenLibrary ("intuition.library", LIBVERSION)) == NULL)
    {
	Error ("Could not open the Intuition.library") ;
    }
    #endif

    #if USE_GRAPHICS == 1
    /* Open the graphics library.... */
    if ((GfxBase = (struct GfxBase *)OpenLibrary ("graphics.library", LIBVERSION)) == NULL)
    {
	Error ("Could not open the Graphics.library") ;
    }
    #endif

    #if USE_LAYERS == 1
    /* Open the layers library.... */
    if ((LayersBase = OpenLibrary ("layers.library", LIBVERSION)) == NULL)
    {
	Error ("Could not open the layers.library") ;
    }
    #endif

    #if USE_ARP == 1
    /* Open the arp library.... */
    if ((ArpBase = (struct ArpBase *)OpenLibrary ("arp.library", LIBVERSION)) == NULL)
    {
	Error ("Could not open the arp.library") ;
    }
    #endif

    #if USE_ICON == 1
    /* Open the icon library.... */
    if ((IconBase = OpenLibrary ("icon.library", LIBVERSION)) == NULL)
    {
	Error ("Could not open the icon.library") ;
    }
    #endif

    #if USE_DISKFONT == 1
    /* Open the diskfont library.... */
    if ((DiskfontBase = (struct DiskFontBase *)OpenLibrary ("diskfont.library", LIBVERSION)) == NULL)
    {
	Error ("Could not open the DiskFont.library") ;
    }
    #endif

    #if USE_IFFPARSE == 1
    /* open IFFParse library */
    if ((IFFParseBase = OpenLibrary("iffparse.library", 37L)) == NULL)
    {
	Error("Could not open the iffparse.library");
    }
    #endif

    return ;
}

/**********************************************************************/
/*                                                                    */
/* void CloseAll ()                                                   */
/*                                                                    */
/* Closes and tidies up everything that was used.                     */
/*                                                                    */
/**********************************************************************/

void CloseAll ()
{
	/* Close everything in the reverse order in which they were opened */

    if (rdargs)
    {
	FreeArgs(rdargs);
    }

    #if USE_IFFPARSE == 1
    if (IFFParseBase)
    {
	CloseLibrary(IFFParseBase);
    }
    #endif

    #if USE_DISKFONT == 1
    /* Close the DiskFont library */
    if (DiskFontBase)
    {
	CloseLibrary ((struct Library *) DiskfontBase) ;
    }
    #endif

    #if USE_ICON == 1
    /* Close the icon library */
    if (IconBase)
    {
	CloseLibrary (IconBase) ;
    }
    #endif

    #if USE_ARP == 1
    /* Close the arp library */
    if (ArpBase)
    {
	CloseLibrary ((struct Library *) ArpBase) ;
    }
    #endif

    #if USE_LAYERS == 1
    /* Close the layers Library */
    if (LayersBase)
    {
	CloseLibrary (LayersBase) ;
    }
    #endif

    #if USE_GRAPHICS == 1
    /* Close the Graphics Library */
    if (GfxBase)
    {
	CloseLibrary ((struct Library *) GfxBase) ;
    }
    #endif

    #if USE_INTUITION == 1
    /* Close the Intuition Library */
    if (IntuitionBase)
    {
	CloseLibrary ((struct Library *) IntuitionBase) ;
    }
    #endif

    #if USE_DOS == 1
    /* DOS closed by Lattice */
    #endif

    return ;
}

/***************************************************************************/

extern struct Custom far custom;

void main (int argc, char *argv[])
{
    struct Screen *s;
    struct UCopList *ucl;
    #define COLOURS 2

    Init () ;

    if (s = OpenScreenTags(NULL, SA_DisplayID, 4, SA_Depth, 4, TAG_DONE))
    {
	if (ucl = AllocMem(sizeof(struct UCopList), MEMF_PUBLIC|MEMF_CLEAR))
	{
		CINIT(ucl, COLOURS+2);
//		CWAIT(ucl, 10, 0);
		CWait(ucl, 10, 0); ucl->CopList->CopPtr->OpCode |= CPR_NT_LOF; CBump(ucl);
		CMOVE(ucl, custom.color[0], 0xf00);
//		CWAIT(ucl, 11, 0);
		CWait(ucl, 11, 0); ucl->CopList->CopPtr->OpCode |= CPR_NT_SHT; CBump(ucl);
		CMOVE(ucl, custom.color[0], 0x0f0);
		CEND(ucl);
		s->ViewPort.UCopIns = ucl;
		RethinkDisplay();
		getchar();
		FreeVPortCopLists(&s->ViewPort);
	}

	CloseScreen(s);
    }

    CloseAll () ;
}
