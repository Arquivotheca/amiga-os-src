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

#define USEARGS
//#define TEST_COLOURS
//#define TEST_LINECONTROL
//#define TEST_INTCONTROL
//#define TEST_VIDEOCONTROL
#define TEST_SPRITECONTROL

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
struct Library *SpecialFXBase = NULL;

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

    if ((SpecialFXBase = (struct Library *)OpenLibrary("specialfx.library", 39)) == NULL)
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

APTR __asm MyAllocFX(register __a0 struct ViewPort *vp, register __d0 unsigned long type, register __d1 unsigned long number, register __a1 ULONG *error, register __a6 struct Library * );
ULONG __asm MyInstallFXA(register __a0 struct View *, register __a1 struct ViewPort *, register __a2 struct TagItem *, register __a6 struct Library *);
void __asm MyRemoveFX(register __a0 APTR, register __a6 struct Library *);
void __asm MyAnimateFX(register __a0 APTR, register __a6 struct Library *);
void __asm MyFreeFX(register __a0 APTR, register __a6 struct Library *);

/***************************************************************************/

#ifdef EXTRATEST
void ExtraTest(struct Screen *s)
{
    APTR ColourHandle;
    APTR DisplayHandle;
    struct TagItem ti[] =
    {
	{SFX_ColorControl, NULL},
	{TAG_DONE, NULL},
    };
    struct ColorRange *Colours[2];

    if (ColourHandle = AllocFX(SFX_ColorRange, 2, (ULONG *)Colours))
    {
	struct ColorRange **cor = Colours;
	(*cor)->cor_Pen = 1;
	(*cor)->cor_Line = 20;
	(*cor)->cor_Red = -1;
	(*cor)->cor_Green = -1;
	(*cor)->cor_Blue = -1;
	(*cor)->cor_Flags = 0;
	cor++;
	(*cor)->cor_Pen = 1;
	(*cor)->cor_Line = 30;
	(*cor)->cor_Red = -1;
	(*cor)->cor_Green = -1;
	(*cor)->cor_Blue = -1;
	(*cor)->cor_Flags = CORF_RESTORE;
	ti[0].ti_Data = (ULONG)ColourHandle;
	InstallFXA(GfxBase->ActiView, &s->ViewPort, &DisplayHandle, ti);
	MakeScreen(s);
	RethinkDisplay();
	getchar();
	RemoveFX(DisplayHandle);
	FreeFX(ColourHandle);
	MakeScreen(s);
	RethinkDisplay();
	getchar();
    }
};
#endif

void main (int argc, char *argv[])
{
    #define TEMPLATE "P=PEN/N,Y/N,L=LINES/N,R1/N,G1/N,B1/N,R2/N,G2/N,B2/N,R=RESTORE/S,U=UPDATE/S,D=DEPTH/N,X1/N,X2/N,LORES/S,GAP/N,SHIRES/S,I=INTERACTIVE/S,LACE/S,DPF/N,DBUFF/S"
    #define OPT_PEN 0
    #define OPT_Y 1
    #define OPT_LINES 2
    #define OPT_R1 3
    #define OPT_G1 4
    #define OPT_B1 5
    #define OPT_R2 6
    #define OPT_G2 7
    #define OPT_B2 8
    #define OPT_RESTORE 9
    #define OPT_UPDATE 10
    #define OPT_DEPTH 11
    #define OPT_X1 12
    #define OPT_X2 13
    #define OPT_LORES 14
    #define OPT_GAP 15
    #define OPT_SHIRES 16
    #define OPT_INTERACTIVE 17
    #define OPT_LACE 18
    #define OPT_DPF 19
    #define OPT_DBUFF 20
    #define OPT_COUNT 21

    #define WIDTH (lores ? 320 : (shires ? 1280 : 640))
    #define MODEID ((lores ? 0x0 : (shires ? 0x8020 : 0x8000)) | (lace ? 4 : 0))
    #define HEIGHT (lace ? 400 : 200)
    #define DEPTH depth
    #define COLOURS (1 << DEPTH)
    #define NUM_COLOURS (HEIGHT + lines)

    struct Screen *s;
    LONG result[OPT_COUNT];
    LONG *val;
    ULONG pen = 64;
    ULONG y = 50;
    ULONG gap = 5;
    ULONG dpf = 0;
    LONG x1 = 1, x2 = -x1;
    ULONG lines = 100;
    ULONG depth = 8;
    BOOL restore = FALSE, update = FALSE, lores = FALSE, shires = FALSE;
    BOOL interactive = FALSE, lace = FALSE, dbuff = FALSE;
    ULONG RGBFirst[3] = {-1, 0, 0};
    ULONG RGBLast[3] = {-1, -1, 0};
    ULONG Grad[3];
    BOOL GradNeg[3];
    ULONG RGBSky[3] = {0, 0, -1};
    ULONG GradSky[3];
    ULONG error;
    APTR ColourHandle;
    APTR LineControlHandle;
    APTR IntControlHandle;
    APTR VideoControlHandle;
    APTR SpriteControlHandle;
    APTR DisplayHandle;
    struct TagItem vc[] =
    {
	{VTAG_USERCLIP_SET, TRUE},
	{VC_IntermediateCLUpdate, FALSE},
	{TAG_DONE, NULL},
    };
    struct TagItem ti[] =
    {
	{SFX_InstallEffect, NULL},
	{SFX_DoubleBuffer, FALSE},
	{TAG_DONE, NULL},
    };
    int i, j;

    Init () ;

    /* init the result[] */
    result[OPT_PEN] = (ULONG)&pen;
    result[OPT_Y] = (ULONG)&y;
    result[OPT_LINES] = (ULONG)&lines;
    result[OPT_R1] = (ULONG)&RGBFirst[0];
    result[OPT_G1] = (ULONG)&RGBFirst[1];
    result[OPT_B1] = (ULONG)&RGBFirst[2];
    result[OPT_R2] = (ULONG)&RGBLast[0];
    result[OPT_G2] = (ULONG)&RGBLast[1];
    result[OPT_B2] = (ULONG)&RGBLast[2];
    result[OPT_RESTORE] = NULL;
    result[OPT_UPDATE] = FALSE;
    result[OPT_DEPTH] = (ULONG)&depth;
    result[OPT_X1] = (ULONG)&x1;
    result[OPT_X2] = (ULONG)&x2;
    result[OPT_LORES] = FALSE;
    result[OPT_SHIRES] = FALSE;
    result[OPT_LACE] = FALSE;
    result[OPT_DPF] = (ULONG)&dpf;
    result[OPT_INTERACTIVE] = FALSE;
    result[OPT_DBUFF] = FALSE;

#ifdef USEARGS
    if (rdargs = ReadArgs(TEMPLATE, result, NULL))
    {
	if (val = (LONG *)result[OPT_PEN])
	{
		pen = *val;
	}
	if (val = (LONG *)result[OPT_Y])
	{
		y = *val;
	}
	if (val = (LONG *)result[OPT_LINES])
	{
		lines = *val;
	}
	if (val = (LONG *)result[OPT_R1])
	{
		RGBFirst[0] = *val;
	}
	if (val = (LONG *)result[OPT_G1])
	{
		RGBFirst[1] = *val;
	}
	if (val = (LONG *)result[OPT_B1])
	{
		RGBFirst[2] = *val;
	}
	if (val = (LONG *)result[OPT_R2])
	{
		RGBLast[0] = *val;
	}
	if (val = (LONG *)result[OPT_G2])
	{
		RGBLast[1] = *val;
	}
	if (val = (LONG *)result[OPT_B2])
	{
		RGBLast[2] = *val;
	}
	if (result[OPT_RESTORE])
	{
		restore = TRUE;
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
	if (result[OPT_SHIRES])
	{
		shires = TRUE;
	}
	if (result[OPT_INTERACTIVE])
	{
		interactive = TRUE;
	}
	if (result[OPT_LACE])
	{
		lace = TRUE;
	}
	if (val = (LONG *)result[OPT_DPF])
	{
		dpf = *val;
	}
	if (result[OPT_DBUFF])
	{
		dbuff = TRUE;
	}
    }
    else
    {
    	Error("Command line error\n");
    }
#else
    update = FALSE;
    x1 = 10; lines = 10; gap = 0;
#endif	/* USEARGS */

    printf("pen = %ld, y = %ld, lines = %ld, depth = %ld, restore = %s, update = %s\n", pen, y, lines, depth, (restore ? "TRUE" : "FALSE"), (update ? "TRUE" : "FALSE"));
    printf("range (0x%lx, 0x%lx, 0x%lx) - (0x%lx, 0x%lx, 0x%lx)\n", RGBFirst[0], RGBFirst[1], RGBFirst[2], RGBLast[0], RGBLast[1], RGBLast[2]);
    printf("x1 = %ld, x2 = %ld %s\n", x1, x2, (lace ? ", laced" : ""));
    vc[1].ti_Data = (update ? TRUE : FALSE);

    ti[1].ti_Data = dbuff;

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

#ifdef TEST_COLOURS
	if (ColourHandle = AllocFX(&s->ViewPort, SFX_ColorControl, NUM_COLOURS, (ULONG *)&Error))
	{
		struct ColorControl **cc = (struct ColorControl **)ColourHandle;
		ULONG Spare[3];

		printf("Colours\n");
		GradSky[0] = GradSky[2] = 0;
		GradSky[1] = (0xffffffff / HEIGHT);

		if (RGBFirst[0] > RGBLast[0])
		{
			Grad[0] = ((RGBFirst[0] - RGBLast[0]) / lines);
			GradNeg[0] = TRUE;
		}
		else
		{
			Grad[0] = ((RGBLast[0] - RGBFirst[0]) / lines);
			GradNeg[0] = FALSE;
		}
		if (RGBFirst[1] > RGBLast[1])
		{
			Grad[1] = ((RGBFirst[1] - RGBLast[1]) / lines);
			GradNeg[1] = TRUE;
		}
		else
		{
			Grad[1] = ((RGBLast[1] - RGBFirst[1]) / lines);
			GradNeg[1] = FALSE;
		}
		if (RGBFirst[2] > RGBLast[2])
		{
			Grad[2] = ((RGBFirst[2] - RGBLast[2]) / lines);
			GradNeg[2] = TRUE;
		}
		else
		{
			Grad[2] = ((RGBLast[2] - RGBFirst[2]) / lines);
			GradNeg[2] = FALSE;
		}

		for (i = 0; i < y; i++)
		{
			/* Do the sky colours accross the whole screen */
			(*cc)->cc_Pen = 0;
			(*cc)->cc_Line = i;
			(*cc)->cc_Red = RGBSky[0];
			(*cc)->cc_Green = RGBSky[1];
			(*cc)->cc_Blue = RGBSky[2];
			(*cc)->cc_Flags = 0;
			RGBSky[1] += GradSky[1];
			RGBSky[2] -= GradSky[2];
			cc++;
		}

		/* Do joint sky and pen range */
		for (i = 0; i < lines; i++)
		{
			(*cc)->cc_Pen = 0;
			(*cc)->cc_Line = (y + i);
			(*cc)->cc_Red = RGBSky[0];
			(*cc)->cc_Green = RGBSky[1];
			(*cc)->cc_Blue = RGBSky[2];
			(*cc)->cc_Flags = 0;
			RGBSky[1] += GradSky[1];
			RGBSky[2] -= GradSky[2];
			cc++;

			(*cc)->cc_Pen = pen;
			(*cc)->cc_Line = (y + i);
			(*cc)->cc_Red = RGBFirst[0];
			(*cc)->cc_Green = RGBFirst[1];
			(*cc)->cc_Blue = RGBFirst[2];
			(*cc)->cc_Flags = ((restore && (i == (lines - 1))) ? CCF_RESTORE : 0);
			cc++;
			if (GradNeg[0])
			{
				RGBFirst[0] -= Grad[0];
			}
			else
			{
				RGBFirst[0] += Grad[0];
			}
			if (GradNeg[1])
			{
				RGBFirst[1] -= Grad[1];
			}
			else
			{
				RGBFirst[1] += Grad[1];
			}
			if (GradNeg[2])
			{
				RGBFirst[2] -= Grad[2];
			}
			else
			{
				RGBFirst[2] += Grad[2];
			}
		}

		/* and the remainder */
		for (i = 0; i < (HEIGHT - lines - y); i++)
		{
			/* Do the sky colours accross the whole screen */
			(*cc)->cc_Pen = 0;
			(*cc)->cc_Line = (y + lines + i);
			(*cc)->cc_Red = RGBSky[0];
			(*cc)->cc_Green = RGBSky[1];
			(*cc)->cc_Blue = RGBSky[2];
			(*cc)->cc_Flags = 0;
			cc++;
			RGBSky[1] += GradSky[1];
		}
		ti[0].ti_Tag = SFX_InstallEffect;
		ti[0].ti_Data = (ULONG)ColourHandle;
		DisplayHandle = InstallFXA(GfxBase->ActiView, &s->ViewPort, ti);
		MakeScreen(s);
		RethinkDisplay();

//		ExtraTest(s);
		/* Now animate by cycling the colours. */
		for (j = 0; j < x1; j++)
		{
			cc = (struct ColorControl **)ColourHandle; /* first of the non-sky colours */
			cc += (y + 1);
			Spare[0] = (*cc)->cc_Red;
			Spare[1] = (*cc)->cc_Green;
			Spare[2] = (*cc)->cc_Blue;
			for (i = 0; i < (lines - 1); i++)
			{
				/* This colour is the next line's */
				(*cc)->cc_Red = (*(cc + 2))->cc_Red;
				(*cc)->cc_Green = (*(cc + 2))->cc_Green;
				(*cc)->cc_Blue = (*(cc + 2))->cc_Blue;
				(*cc)->cc_Flags |= CCF_MODIFY;
				cc += 2;
			}
			(*cc)->cc_Red = Spare[0];
			(*cc)->cc_Green = Spare[1];
			(*cc)->cc_Blue = Spare[2];
			(*cc)->cc_Flags |= CCF_MODIFY;

			WaitTOF();
			AnimateFX(DisplayHandle);
			if (interactive)
			{
				getchar();
			}
		}
		getchar();
		RemoveFX(DisplayHandle);
		MakeScreen(s);
		RethinkDisplay();
		FreeFX(ColourHandle);
	}
#endif	/* TEST_COLOURS */
#ifdef TEST_LINECONTROL
	if (LineControlHandle = AllocFX(&s->ViewPort, SFX_LineControl, 4, (ULONG *)&Error))
	{
		extern void wibble(void);
		struct LineControl **lc = (struct LineControl **)LineControlHandle;
		struct RasInfo ri1 = *s->ViewPort.RasInfo;
		struct RasInfo ri2 = *s->ViewPort.RasInfo;
		struct RasInfo ri3 = *s->ViewPort.RasInfo;
		struct RasInfo ri4 = *s->ViewPort.RasInfo;
		struct RasInfo ri5, ri6, ri7, ri8;
		Move(rp, 0, 0);
		SetAPen(rp, 0);
		Draw(rp, (WIDTH - 1), (HEIGHT - 1));
		printf("Lines\n");

		(*lc)->lc_RasInfo = &ri1;
		(*lc)->lc_Line = (*lc)->lc_RasInfo->RyOffset = y;
		(*lc)->lc_Count = lines;
		(*(lc+1))->lc_RasInfo = &ri2;
		(*(lc+1))->lc_Line = (*(lc+1))->lc_RasInfo->RyOffset = (y + lines + gap);
		(*(lc+1))->lc_Count = (lines * 2);
		(*(lc+2))->lc_RasInfo = &ri3;
		(*(lc+2))->lc_Line = (*(lc+2))->lc_RasInfo->RyOffset = (y + (lines * 3) + gap);
		(*(lc+2))->lc_Count = (lines * 2);
		(*(lc+3))->lc_RasInfo = &ri4;
		(*(lc+3))->lc_Line = (*(lc+3))->lc_RasInfo->RyOffset = (y + (lines * 5) + (gap * 2));
		(*(lc+3))->lc_Count = lines;

		(*lc)->lc_RasInfo->RxOffset = x1;
		(*(lc+1))->lc_RasInfo->RxOffset = (-x1 / 2);
		(*(lc+2))->lc_RasInfo->RxOffset = (x1 / 2);
		(*(lc+3))->lc_RasInfo->RxOffset = -x1;
		if (dpf)
		{
			if (dpf & 1)
			{
				ri1.Next = &ri5;
				ri5 = ri1;
				ri5.RxOffset = -ri1.RxOffset;
			}
			if (dpf & 2)
			{
				ri2.Next = &ri6;
				ri6 = ri2;
				ri6.RxOffset = -ri2.RxOffset;
			}
			if (dpf & 4)
			{
				ri3.Next = &ri7;
				ri7 = ri3;
				ri7.RxOffset = -ri3.RxOffset;
			}
			if (dpf & 8)
			{
				ri4.Next = &ri8;
				ri8 = ri4;
				ri8.RxOffset = -ri4.RxOffset;
			}
		}

		ti[0].ti_Tag = SFX_InstallEffect;
		ti[0].ti_Data = (ULONG)LineControlHandle;
		DisplayHandle = InstallFXA(GfxBase->ActiView, &s->ViewPort, ti);
		MakeScreen(s);
		RethinkDisplay();
		if (interactive)
		{
			getchar();
		}
		(*lc)->lc_Flags |= LCF_MODIFY;
		(*(lc+1))->lc_Flags |= LCF_MODIFY;
		(*(lc+2))->lc_Flags |= LCF_MODIFY;
		(*(lc+3))->lc_Flags |= LCF_MODIFY;
		for (j = (x1 - 1); j >= x2; j--)
		{
			(*lc)->lc_RasInfo->RxOffset = j;
			(*(lc+1))->lc_RasInfo->RxOffset = (-j / 2);
			(*(lc+2))->lc_RasInfo->RxOffset = (j / 2);
			(*(lc+3))->lc_RasInfo->RxOffset = -j;
			if (dpf)
			{
				ri5.RxOffset = -ri1.RxOffset;
				ri6.RxOffset = -ri2.RxOffset;
				ri7.RxOffset = -ri3.RxOffset;
				ri8.RxOffset = -ri4.RxOffset;
			}
			WaitTOF();
			AnimateFX(DisplayHandle);
			if (interactive)
			{
				kprintf("%ld\n",j);
				getchar();
			}
		}
		getchar();
		RemoveFX(DisplayHandle);
		MakeScreen(s);
		RethinkDisplay();
		FreeFX(LineControlHandle);
	}
#endif	/* TEST_LINECONTROL */
#ifdef TEST_INTCONTROL
	if (IntControlHandle = AllocFX(&s->ViewPort, SFX_IntControl, 2, (ULONG *)&Error))
	{
		extern void CopServer();
		struct InterruptControl **icl = (struct InterruptControl **)IntControlHandle;
		struct Interrupt *CopInt = NULL;
		struct IntStuff
		{
			struct ViewPort *ThisVP;
			struct GfxBase *GfxBase;
			struct Library *SpecialFXBase;
			UWORD y;
		} MyIntStuff;

		printf("INterrupts\n");
		if (CopInt = AllocMem(sizeof(struct Interrupt), MEMF_PUBLIC|MEMF_CLEAR))
		{
			ULONG foo = 1;
			CopInt->is_Node.ln_Type = NT_INTERRUPT;
			CopInt->is_Node.ln_Pri = 0;
			CopInt->is_Node.ln_Name = "CopIntSFX";
			CopInt->is_Data = &MyIntStuff;
			CopInt->is_Code = CopServer;
			MyIntStuff.ThisVP = NULL;
			MyIntStuff.GfxBase = GfxBase;
			MyIntStuff.SpecialFXBase = SpecialFXBase;
			MyIntStuff.y = y;
			AddIntServer(INTB_COPER, CopInt);

			(*icl)->inc_Line = y;
			(*icl)->inc_Flags = 0;
			icl++;
			(*icl)->inc_Line =(y + 10);
			(*icl)->inc_Flags = 0;
			ti[0].ti_Tag = SFX_InstallEffect;
			ti[0].ti_Data = (ULONG)IntControlHandle;
			DisplayHandle = InstallFXA(GfxBase->ActiView, &s->ViewPort, ti);

			MakeScreen(s);
			RethinkDisplay();

			icl = (struct InterruptControl **)IntControlHandle;
			for (j = 0; j < x1; j++)
			{
				WaitTOF();
				(*icl)->inc_Flags = INCF_SET;
				(*(icl += foo))->inc_Flags = INCF_RESET;
				AnimateFX(DisplayHandle);
				if (interactive)
				{
					getchar();
				}
				foo = -foo;
			}

			getchar();

			printf("Screen VP = 0x%lx, IntVP = 0x%lx\n", &s->ViewPort, MyIntStuff.ThisVP);
			RemIntServer(INTB_COPER, CopInt);
			RemoveFX(DisplayHandle);
			MakeScreen(s);
			RethinkDisplay();
			FreeFX(IntControlHandle);
			FreeMem(CopInt, sizeof(struct Interrupt));
		}
		else
		{
			printf("No mem for intserver\n");
		}
	}
#endif
#ifdef TEST_VIDEOCONTROL
	#define VIDEOCONTROLS 4
	if (VideoControlHandle = AllocFX(&s->ViewPort, SFX_FineVideoControl, VIDEOCONTROLS, (ULONG *)&Error))
	{
		struct FineVideoControl **fvc = (struct FineVideoControl **)VideoControlHandle;
		struct TagItem vcti[][3] =
		{
			{
				{VTAG_BORDERSPRITE_SET, TRUE},
				{VTAG_SPEVEN_BASE_SET, 64},
				{TAG_DONE},
			},
			{
				{VTAG_SFX_HAM_SET, TRUE},
				{VTAG_SPRITERESN_SET, SPRITERESN_35NS},
				{TAG_DONE},
			},
			{
				{VTAG_BORDERBLANK_SET, TRUE},
				{VTAG_PF2_TO_SPRITEPRI_SET, 0},
				{TAG_DONE},
			},
			{
				{VTAG_SFX_EHB_SET, TRUE},
				{VTAG_SFX_DEPTH_SET, 6},
				{TAG_DONE},
			},
		};
		int i;
				
		printf("VideoControl\n");
		for (i = 0; i < VIDEOCONTROLS; i++)
		{
			(*fvc)->fvc_TagList = &vcti[i];
			(*fvc)->fvc_Line = y += 20;
			(*fvc)->fvc_Count = 10;
			fvc++;
		}

		ti[0].ti_Tag = SFX_InstallEffect;
		ti[0].ti_Data = (ULONG)VideoControlHandle;
		DisplayHandle = InstallFXA(GfxBase->ActiView, &s->ViewPort, ti);
		MakeScreen(s);
		RethinkDisplay();

		fvc = (struct FineVideoControl **)VideoControlHandle;
		(*fvc)->fvc_Flags |= FVCF_MODIFY;
		for (j = 0; j < x1; j++)
		{
			vcti[0][1].ti_Data = (j << 4);
			WaitTOF();
			AnimateFX(DisplayHandle);	
			if (interactive)
			{
				getchar();
			}
		}

		getchar();

		RemoveFX(DisplayHandle);
		MakeScreen(s);
		RethinkDisplay();
		FreeFX(VideoControlHandle);
	}
#endif
#ifdef TEST_SPRITECONTROL
	#define SPRITECONTROLS 3
	if (SpriteControlHandle = AllocFX(&s->ViewPort, SFX_SpriteControl, SPRITECONTROLS, (ULONG *)&Error))
	{
		struct Screen *wb = LockPubScreen(NULL);
		struct SpriteControl **sc = (struct SpriteControl **)SpriteControlHandle;
		struct ExtSprite *es[SPRITECONTROLS] = {NULL};
		struct BitMap *bm[SPRITECONTROLS] = {NULL};
		BOOL fail = FALSE;
		int i,j;

		printf("sprites\n");
		for (i = 0; ((i < SPRITECONTROLS) && !fail); i++)
		{
			if (bm[i] = AllocBitMap(64, lines, 2, NULL, wb->RastPort.BitMap))
			{
				BltBitMap(wb->RastPort.BitMap, (i * 64), (i * 64), bm[i], 0, 0, 64, lines, 0xc0, 3, NULL);
				if (es[i] = AllocSpriteData(bm[i], SPRITEA_Width, 64, TAG_DONE))
				{
					GetExtSprite(es[i], GSTAG_SPRITE_NUM, (i + 1), TAG_DONE);
					(*sc)->spc_ExtSprite = es[i];
					(*sc)->spc_Line = (y + (i * lines));
					es[i]->es_SimpleSprite.num = (i + 1);
					es[i]->es_SimpleSprite.height = lines - 5;
					es[i]->es_SimpleSprite.x = (i * 64);
					es[i]->es_SimpleSprite.y = ((*sc)->spc_Line + 1);
					sc++;
				}
				else
				{
					fail = TRUE;
				}
			}
			else
			{
				fail = TRUE;
			}
		}

		UnlockPubScreen(NULL, wb);

		if (!fail)
		{
			ti[0].ti_Tag = SFX_InstallEffect;
			ti[0].ti_Data = (ULONG)SpriteControlHandle;
			DisplayHandle = InstallFXA(GfxBase->ActiView, &s->ViewPort, ti);
			MakeScreen(s);
			RethinkDisplay();

			for (i = 0; i < x1; i++)
			{
				sc = (struct SpriteControl **)SpriteControlHandle;
				for (j = 0; j < SPRITECONTROLS; j++)
				{
					(*sc)->spc_Flags = SPCF_MODIFY;
					(*sc)->spc_ExtSprite->es_SimpleSprite.x++;
					(++(*sc)->spc_ExtSprite->es_SimpleSprite.num);
					(*sc)->spc_ExtSprite->es_SimpleSprite.num &= 7;
					(*sc)->spc_ExtSprite->es_SimpleSprite.y += ((i & 1) ? -gap : gap);
					sc++;
				}				
				WaitTOF();
				AnimateFX(DisplayHandle);
				if (interactive)
				{
					getchar();
				}
			}

			getchar();
			RemoveFX(DisplayHandle);
			MakeScreen(s);
			RethinkDisplay();
			FreeFX(SpriteControlHandle);
		}

		for (i = (SPRITECONTROLS - 1); i >= 0; i--)
		{
			if (es[i])
			{
				FreeSpriteData(es[i]);
			}
			if (bm[i])
			{
				FreeBitMap(bm[i]);
			}
			FreeSprite(i + 1);
		}
	}
#endif
	CloseScreen(s);
    }

    CloseAll () ;
}
