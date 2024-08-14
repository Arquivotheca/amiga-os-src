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
struct RDArgs *rdargs = NULL;
struct SpecialFXBase *SpecialFXBase = NULL;

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

    if ((SpecialFXBase = (struct SpecialFXBase *)OpenLibrary("specialfx.library", 39)) == NULL)
    {
	Error("No SpecialFX.library");
    }

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

    if (SpecialFXBase)
    {
	RemLibrary((struct Library *)SpecialFXBase);
	CloseLibrary((struct Library *)SpecialFXBase);
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

BOOL OpenMPEGScreen(struct Screen *s, ULONG ChromaPen)
{
    BOOL result = FALSE;
    struct TagItem vc[] =
    {
	{VTAG_USERCLIP_SET, TRUE},
	{VTAG_CHROMA_PEN_SET, 0},
	{VTAG_CHROMAKEY_CLR, TRUE},
	{TAG_DONE, NULL},
    };
    struct TagItem ti[] =
    {
	{SFX_InstallEffect, NULL},
	{TAG_DONE, NULL},
    };
    struct ViewPort *vp = &s->ViewPort;
    APTR VideoControlHandle;
    APTR AnimHandle;
    UWORD lines = s->Height;
    ULONG error;

    {
	vc[1].ti_Data = ChromaPen;
	if (VideoControl(vp->ColorMap, ti) == NULL)
	{
		if (VideoControlHandle = AllocFX(vp, SFX_FineVideoControl, lines, &error))
		{
			struct FineVideoControl **fvc = (struct FineVideoControl **)VideoControlHandle;
			struct FineVideoControl **first = fvc;
			struct FineVideoControl **vctop, **vcbottom;
			struct TagItem tifvc[2][2] =
			{
				{
					{VTAG_CHROMAKEY_CLR, TRUE},
					{TAG_DONE},
				},
				{
					{VTAG_CHROMAKEY_SET, TRUE},
					{TAG_DONE},
				},
			};
			UWORD top;
			int i;

			for (i = 0; i < lines; i++)
			{
				(*fvc)->fvc_TagList = (ULONG)&tifvc[0];
				(*fvc)->fvc_Line = i;
				(*fvc)->fvc_Count = 1;
				fvc++;
			}
			ti[0].ti_Data = (ULONG)VideoControlHandle;
			if (AnimHandle = InstallFXA(GfxBase->ActiView, vp, ti))
			{
				MakeScreen(s);
				RethinkDisplay();

				/* Now animate */
				top = (lines / 2);
				vctop = (first + top);
				vcbottom = (vctop + 1);
				for (i = top; i > 0; i--)
				{
					(*vctop)->fvc_TagList = (ULONG)&tifvc[1];
					(*vctop)->fvc_Flags = FVCF_MODIFY;
					(*vcbottom)->fvc_TagList = (ULONG)&tifvc[1];
					(*vcbottom)->fvc_Flags = FVCF_MODIFY;

					if (i < top)
					{
						(*vctop + 1)->fvc_Flags = 0;
						(*vcbottom - 1)->fvc_Flags = 0;
					}

					vctop--;
					vcbottom++;
	
					WaitTOF();
					AnimateFX(AnimHandle);
				}

				result = TRUE;
				RemoveFX(AnimHandle);
			}
			FreeFX(VideoControlHandle);
			VideoControl(vp->ColorMap, &tifvc[1]);
			MakeScreen(s);
			RethinkDisplay();
		}
	}
    }

    return(result);
}

void __asm MyAnimateFX(register __a0 APTR, register __a6 struct SpecialFXBase *);
ULONG __asm MyInstallFXA(register __a0 struct View *, register __a1 struct ViewPort *, register __a2 struct TagItem *, register __a6 struct SpecialFXBase *);
void __asm MyRemoveFX(register __a0 APTR, register __a6 struct SpecialFXBase *);

void main (int argc, char *argv[])
{
    #define DEPTH 8
    #define WIDTH 640
    #define HEIGHT 200
    #define MODEID 0x8000
    #define NUM_COLOURS (HEIGHT + lines)
    #define COLOURS (1 << DEPTH)
    #define NUM_HAMLINES (lines >> 1)
    #define NUM_INTCONTROL 1

    struct Screen *s;
    ULONG RGBFirst[3] = {-1, 0, 0};
    ULONG RGBLast[3] = {-1, -1, 0};
    ULONG Grad[3];
    BOOL GradNeg[3];
    ULONG RGBSky[3] = {0, 0, -1};
    ULONG GradSky[3] = {0, (0xffffffff / HEIGHT), 0};
    ULONG Error_c = 0, Error_v = 0, Error_i = 0;
    APTR ColourHandle;
    APTR VideoControlHandle;
    APTR IntControlHandle;
    APTR DisplayHandle;
    ULONG y = 50;
    ULONG lines = 100;
    ULONG pen = 64;
    struct TagItem vc[] =
    {
	{VTAG_USERCLIP_SET, TRUE},
	{VC_IntermediateCLUpdate, TRUE},
	{TAG_DONE, NULL},
    };
    struct TagItem ti[] =
    {
	{SFX_InstallEffect, NULL},
	{SFX_InstallEffect, NULL},
	{SFX_InstallEffect, NULL},
	{TAG_DONE, NULL},
    };
    struct Interrupt *CopInt = NULL;
    struct IntStuff
    {
	struct GfxBase *GfxBase;
	struct SpecialFXBase *SpecialFXBase;
	ULONG RGB;
    } MyIntStuff;
    int i, j, k;

    Init () ;

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

	if ((ColourHandle = AllocFX(&s->ViewPort, SFX_ColorControl, NUM_COLOURS, (ULONG *)&Error_c)) &&
	    (VideoControlHandle = AllocFX(&s->ViewPort, SFX_FineVideoControl, NUM_HAMLINES, (ULONG *)&Error_v)) &&
	    (IntControlHandle = AllocFX(&s->ViewPort, SFX_IntControl, NUM_INTCONTROL, (ULONG *)&Error_i)) &&
	    (CopInt = AllocMem(sizeof(struct Interrupt), MEMF_PUBLIC|MEMF_CLEAR)))
	{
		extern void CopServer();
		struct InterruptControl **icl = (struct InterruptControl **)IntControlHandle;
		struct ColorControl **cc = (struct ColorControl **)ColourHandle;
		struct FineVideoControl **fvc = (struct FineVideoControl **)VideoControlHandle;
		struct FineVideoControl **last = NULL;
		struct FineVideoControl **first = fvc;
		struct TagItem fvcti[2][2] =
		{
			{
				{VTAG_SFX_HAM_SET, TRUE},
				{TAG_DONE},
			},
			{
				{VTAG_SFX_NORM_SET, TRUE},
				{TAG_DONE},
			},
		};
		ULONG Spare[3];

		/* Set up the interrupt stuff */

		CopInt->is_Node.ln_Type = NT_INTERRUPT;
		CopInt->is_Node.ln_Pri = 0;
		CopInt->is_Node.ln_Name = "CopIntSFX";
		CopInt->is_Data = &MyIntStuff;
		CopInt->is_Code = CopServer;
		MyIntStuff.GfxBase = GfxBase;
		MyIntStuff.SpecialFXBase = SpecialFXBase;
		MyIntStuff.RGB = 0;
		AddIntServer(INTB_COPER, CopInt);

		(*icl)->ic_Line = (y + lines + 20);
		(*icl)->ic_Flags = 0;

		/* Now for the Colour stuff */

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
			(*cc)->cc_Flags = ((i == (lines - 1)) ? CCF_RESTORE : 0);
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

		/*
		 * Build the effect for VideoControl, where 1 line of (lines/2) lines
		 * in the middle of the animated colour area is HAM.
		 */
		for (i = 0; i < NUM_HAMLINES; i++)
		{
			(*fvc)->fvc_TagList = ((i == 0) ? &fvcti[0] : &fvcti[1]);
			(*fvc)->fvc_Line = ((y + ((lines - NUM_HAMLINES) / 2)) + i);
			(*fvc)->fvc_Count = 1;
			fvc++;
		}

		ti[2].ti_Data = (ULONG)IntControlHandle;
		ti[1].ti_Data = (ULONG)ColourHandle;
		ti[0].ti_Data = (ULONG)VideoControlHandle;
		DisplayHandle = InstallFXA(GfxBase->ActiView, &s->ViewPort, ti);
		MakeScreen(s);
		RethinkDisplay();

		/* Now animate by cycling the colours and altering which
		 * line has HAM.
		 */
		k = 0;	/* for the VideoControl count. */
		last = --fvc;	/* Last in the list */
		fvc = first;
		for (j = 0; j < 500; j++)
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

			/* VideoControl stuff */
			(*fvc)->fvc_TagList = &fvcti[1]; /* set to NORM */
			(*fvc)->fvc_Flags = FVCF_MODIFY;
			fvc++;
			if (k == (NUM_HAMLINES - 1))
			{
				(*first)->fvc_TagList = &fvcti[0]; /* set to HAM */
				(*first)->fvc_Flags = FVCF_MODIFY;
			}
			else
			{
				(*fvc)->fvc_TagList = &fvcti[0]; /* set to HAM */
				(*fvc)->fvc_Flags = FVCF_MODIFY;
			}
			if (k == 0)
			{
				(*last)->fvc_Flags = 0;
			}
			else
			{
				(*fvc-2)->fvc_Flags = 0;
			}
			k++;
			if (k == NUM_HAMLINES)
			{
				k = 0;
				fvc = first;
			}

			WaitTOF();
			AnimateFX(DisplayHandle);
		}
		RemIntServer(INTB_COPER, CopInt);
		RemoveFX(DisplayHandle);
		FreeMem(CopInt, sizeof(struct Interrupt));
	}
	FreeFX(IntControlHandle);
	FreeFX(VideoControlHandle);
	FreeFX(ColourHandle);

	OpenMPEGScreen(s, 1);
	CloseScreen(s);
    }

    CloseAll () ;
}
