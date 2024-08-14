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
#include <exec/interrupts.h>
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
#include <graphics/videocontrol.h>
#include <graphics/sprite.h>
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

#include <hardware/intbits.h>

#include "/SpecialFXBase.h"
#include "/SpecialFX.h"
#include <proto/SpecialFX_protos.h>
#include <pragmas/SpecialFX_pragmas.h>

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
struct SpecialFXBase *SpecialFXBase = NULL;

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

    if ((SpecialFXBase = (struct SpecialFXBase *)OpenLibrary("specialfx.library", 39)) == NULL)
    {
	Error("No specialfx.library");
    }

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

    if (SpecialFXBase)
    {
	RemLibrary((struct Library *)SpecialFXBase);
	CloseLibrary((struct Library *)SpecialFXBase);
    }

    #if USE_DOS == 1
    /* DOS closed by Lattice */
    #endif

    return ;
}

ULONG __asm MyInstallFXA(register __a0 struct View *, register __a1 struct ViewPort *, register __a2 struct TagItem *, register __a6 struct SpecialFXBase *);
void __asm MyRemoveFX(register __a0 APTR, register __a6 struct SpecialFXBase *);
void __asm MyAnimateFX(register __a0 APTR, register __a6 struct SpecialFXBase *);

/***************************************************************************/
void main (int argc, char *argv[])
{
    #define TEMPLATE "Y/N,L=LINES/N,U=UPDATE/S,D=DEPTH/N,X1/N,X2/N,LORES/S,GAP/N,R=REPEAT/S"
    #define OPT_Y 0
    #define OPT_LINES 1
    #define OPT_UPDATE 2
    #define OPT_DEPTH 3
    #define OPT_X1 4
    #define OPT_X2 5
    #define OPT_LORES 6
    #define OPT_GAP 7
    #define OPT_REPEAT 8
    #define OPT_COUNT 9

    #define WIDTH (lores ? 320 : 640)
    #define MODEID (lores ? 0x0 : 0x8000)
    #define HEIGHT 200
    #define DEPTH depth
    #define COLOURS (1 << DEPTH)
    #define NUM_COLOURS (HEIGHT + lines)

    struct Screen *s;
    LONG result[OPT_COUNT];
    LONG *val;
    ULONG y = 100;
    ULONG gap = 5;
    LONG x1 = 10, x2 = -34;
    ULONG lines = 6;
    ULONG depth = 8;
    BOOL update = FALSE, lores = FALSE, repeat = FALSE;
    ULONG error;
    APTR LineControlHandle;
    APTR DisplayHandle;
    UWORD pen = 64;
    struct TagItem vc[] =
    {
	{VTAG_USERCLIP_SET, TRUE},
	{VC_IntermediateCLUpdate, FALSE},
	{TAG_DONE, NULL},
    };
    struct TagItem ti[] =
    {
	{SFX_InstallEffect, NULL},
	{TAG_DONE, NULL},
    };
    int i, j;

    Init () ;

    /* init the result[] */
    result[OPT_Y] = (ULONG)&y;
    result[OPT_LINES] = (ULONG)&lines;
    result[OPT_UPDATE] = FALSE;
    result[OPT_DEPTH] = (ULONG)&depth;
    result[OPT_X1] = (ULONG)&x1;
    result[OPT_X2] = (ULONG)&x2;
    result[OPT_LORES] = FALSE;
    result[OPT_GAP] = (ULONG)&gap;
    result[OPT_REPEAT] = FALSE;

#define USEARGS
#ifdef USEARGS
    if (rdargs = ReadArgs(TEMPLATE, result, NULL))
    {
	if (val = (LONG *)result[OPT_Y])
	{
		y = *val;
	}
	if (val = (LONG *)result[OPT_LINES])
	{
		lines = *val;
	}
	if (result[OPT_UPDATE])
	{
		update = TRUE;
	}
	if (val = (LONG *)result[OPT_DEPTH])
	{
		depth = *val;
	}
	if (val = (LONG *)result[OPT_X1])
	{
		x1 = *val;
	}
	if (val = (LONG *)result[OPT_X2])
	{
		x2 = *val;
	}
	if (result[OPT_LORES])
	{
		lores = TRUE;
	}
	if (val = (LONG *)result[OPT_GAP])
	{
		gap = *val;
	}
	if (result[OPT_REPEAT])
	{
		repeat = TRUE;
	}
    }
    else
    {
    	Error("Command line error\n");
    }
#endif
printf("repeat = %lx\n", repeat);

    if (s = OpenScreenTags(NULL, SA_Depth, DEPTH, 
                                 SA_Width, WIDTH,
                                 SA_Height, HEIGHT,
                                 SA_DisplayID, MODEID,
                                 SA_VideoControl, vc,
                                 TAG_DONE))
    {
	struct RastPort *rp = &s->RastPort;
	SetAPen(rp, pen);
	for (i = 0; i < (WIDTH - COLOURS); i++)
	{
		Move(rp, (pen + i), 0);
		Draw(rp, (pen + i), (HEIGHT - 1));
	}
	for (i = 0; i < pen; i++)
	{
		SetAPen(rp, i);
		Move(rp, i, 0);
		Draw(rp, i, (HEIGHT - 1));
	}
	for (i = (pen + (WIDTH - COLOURS)); i < WIDTH; i++)
	{
		SetAPen(rp, (i - (WIDTH - COLOURS)));
		Move(rp, i, 0);
		Draw(rp, i, (HEIGHT - 1));
	}
	SetAPen(rp, 5);
	Move(rp, pen, 0);
	Draw(rp, (pen + WIDTH - COLOURS), 0);
	SetAPen(rp, 6);
	Move(rp, pen, (HEIGHT - 1));
	Draw(rp, (pen + WIDTH - COLOURS), (HEIGHT - 1));
	
	if (LineControlHandle = AllocFX(&s->ViewPort, SFX_LineControl, 4, (ULONG *)&Error))
	{
		struct LineControl **lc = (struct LineControl **)LineControlHandle;
		struct RasInfo ri1 = *s->ViewPort.RasInfo;
		struct RasInfo ri2 = *s->ViewPort.RasInfo;
		struct RasInfo ri3 = *s->ViewPort.RasInfo;
		struct RasInfo ri4 = *s->ViewPort.RasInfo;
		Move(rp, 0, 0);
		SetAPen(rp, 0);
		Draw(rp, (WIDTH - 1), (HEIGHT - 1));

		(*lc)->lc_RasInfo = &ri1;
		(*lc)->lc_Line = y;
		(*lc)->lc_RasInfo->RyOffset = 0;
		(*lc)->lc_Count = lines;
		(*lc)->lc_Flags = (repeat ? (LCF_REPEATLINE_EVEN | LCF_REPEATLINE_ODD) : 0);
		(*(lc+1))->lc_RasInfo = &ri2;
		(*(lc+1))->lc_Line = (y + lines + gap);
		(*(lc+1))->lc_RasInfo->RyOffset = (repeat ? (HEIGHT - 1) : (HEIGHT - (lines * 2)));
		(*(lc+1))->lc_Count = (lines * 2);
		(*(lc+1))->lc_Flags = (repeat ? (LCF_REPEATLINE_EVEN | LCF_REPEATLINE_ODD) : 0);
		(*(lc+2))->lc_RasInfo = &ri3;
		(*(lc+2))->lc_Line = (*(lc+2))->lc_RasInfo->RyOffset = (y + (lines * 3) + gap);
		(*(lc+2))->lc_Count = (lines * 2);
		(*(lc+3))->lc_RasInfo = &ri4;
		(*(lc+3))->lc_Line = (*(lc+3))->lc_RasInfo->RyOffset = (y + (lines * 5) + (gap * 2));
		(*(lc+3))->lc_Count = lines;
		ti[0].ti_Tag = SFX_InstallEffect;
		ti[0].ti_Data = (ULONG)LineControlHandle;
		DisplayHandle = InstallFXA(GfxBase->ActiView, &s->ViewPort, ti);
		MakeScreen(s);
		RethinkDisplay();
		(*lc)->lc_Flags |= LCF_MODIFY;
		(*(lc+1))->lc_Flags |= LCF_MODIFY;
		(*(lc+2))->lc_Flags |= LCF_MODIFY;
		(*(lc+3))->lc_Flags |= LCF_MODIFY;
		for (j = x1; j >= x2; j--)
		{
			(*lc)->lc_RasInfo->RxOffset = j;
			(*(lc+1))->lc_RasInfo->RxOffset = (-j / 2);
			(*(lc+2))->lc_RasInfo->RxOffset = (j / 2);
			(*(lc+3))->lc_RasInfo->RxOffset = -j;
			WaitTOF();
			AnimateFX(DisplayHandle);
		}
		getchar();
		RemoveFX(DisplayHandle);
		FreeFX(LineControlHandle);
		MakeScreen(s);
		RethinkDisplay();
	}
	CloseScreen(s);
    }
    CloseAll();
}
