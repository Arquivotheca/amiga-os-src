/*
 * This example code uses specialfx.library to create a smooth sky gradient down
 * the screen's border. In the middle of the display, one of the pens is set to
 * range from red to yellow, and one line is set to be HAM.
 * The red-yellow range is then cycled up the screen, whilst the HAM line is
 * cycled down the screen.
 * The area of the display above the red-yellow range is split into two
 * halves and scrolled independantly.
 *
 * Whilst all this is happening, a copper interrupt is continuously modifying
 * one of the colour registers.
 */


int CXBRK (void) { return (0) ; }	/* disable ctrl-c */

#include <exec/types.h>
#include <exec/exec.h>
#include <exec/interrupts.h>
#include <hardware/intbits.h>
#include <proto/exec.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <proto/dos.h>
#include <intuition/screens.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <proto/intuition.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/displayinfo.h>
#include <graphics/videocontrol.h>
#include <proto/graphics.h>
#include <libraries/specialfx.h>
#include <proto/SpecialFX_protos.h>
#include <pragmas/SpecialFX_pragmas.h>

#include <stdio.h>
#include <stdlib.h>

/*********************************************************************/
/*                            GLOBAL VARIABLES                       */
/*********************************************************************/

struct IntuitionBase *IntuitionBase = NULL ;
struct GfxBase *GfxBase = NULL ;
struct Library *SpecialFXBase = NULL;
struct RDArgs *rdargs = NULL;

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
    /* Open the intuition library.... */
    if ((IntuitionBase = (struct IntuitionBase *)OpenLibrary ("intuition.library", 39)) == NULL)
    {
	Error ("Could not open the Intuition.library") ;
    }

    /* Open the graphics library.... */
    if ((GfxBase = (struct GfxBase *)OpenLibrary ("graphics.library", 39)) == NULL)
    {
	Error ("Could not open the Graphics.library") ;
    }

    if ((SpecialFXBase = OpenLibrary("specialfx.library", 40)) == NULL)
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
	CloseLibrary(SpecialFXBase);
    }

    /* Close the Graphics Library */
    if (GfxBase)
    {
	CloseLibrary ((struct Library *) GfxBase) ;
    }

    /* Close the Intuition Library */
    if (IntuitionBase)
    {
	CloseLibrary ((struct Library *) IntuitionBase) ;
    }

    return ;
}

/***************************************************************************/

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
    #define NUM_LINECONTROL 2

    struct Screen *s;
    ULONG RGBFirst[3] = {-1, 0, 0};
    ULONG RGBLast[3] = {-1, -1, 0};
    ULONG Grad[3];
    BOOL GradNeg[3];
    ULONG RGBSky[3] = {0, 0, -1};
    ULONG GradSky[3] = {0, (0xffffffff / HEIGHT), 0};
    ULONG Error_c = NULL, Error_v = NULL, Error_i = NULL, Error_l = NULL;
    APTR ColourHandle = NULL;
    APTR LineControlHandle = NULL;
    APTR VideoControlHandle = NULL;
    APTR IntControlHandle = NULL;
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
	{SFX_InstallEffect, NULL},
	{SFX_DoubleBuffer, FALSE},
	{TAG_DONE, NULL},
    };
    struct Interrupt *CopInt = NULL;
    struct IntStuff
    {
	struct GfxBase *GfxBase;
	struct Library *SpecialFXBase;
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
	    (LineControlHandle = AllocFX(&s->ViewPort, SFX_LineControl, NUM_LINECONTROL, (ULONG *)&Error_l)) &&
	    (CopInt = AllocMem(sizeof(struct Interrupt), MEMF_PUBLIC|MEMF_CLEAR)))
	{
		extern void CopServer();
		struct InterruptControl **icl = (struct InterruptControl **)IntControlHandle;
		struct ColorControl **cc = (struct ColorControl **)ColourHandle;
		struct LineControl **lc = (struct LineControl **)LineControlHandle;
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
		struct RasInfo ri[NUM_LINECONTROL];

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

		(*icl)->inc_Line = (y + lines + 20);
		(*icl)->inc_Flags = INCF_SET;

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
			(*fvc)->fvc_TagList = ((i == 0) ? &fvcti[0][0] : &fvcti[1][0]);
			(*fvc)->fvc_Line = ((y + ((lines - NUM_HAMLINES) / 2)) + i);
			(*fvc)->fvc_Count = 1;
			fvc++;
		}

		/* Build the effect for the LineControl. The area above the
		 * ColorControl is split into two, and both are scrolled separately
		 * in different directions.
		 */
		for (j = 0; j < NUM_LINECONTROL; j++)
		{
			ri[j] = *s->ViewPort.RasInfo;
		}
		(*lc)->lc_RasInfo = &ri[0];
		(*lc)->lc_Line = ri[0].RyOffset = 1;
		(*lc)->lc_Count = ((y / 2) - 1);
		(*(lc+1))->lc_RasInfo = &ri[1];
		/* If we leave ri[1].RyOffset as 0, we would display lines 
		 * 0 thru (y/2) of the bitmap at lines ((y/2)+1) thru (y-1)
		 * of the screen.
		 */
		(*(lc+1))->lc_Line = ri[1].RyOffset = (y / 2);
		(*(lc+1))->lc_Count = ((y / 2) - 1);

		ti[3].ti_Data = (ULONG)LineControlHandle;
		ti[2].ti_Data = (ULONG)IntControlHandle;
		ti[1].ti_Data = (ULONG)ColourHandle;
		ti[0].ti_Data = (ULONG)VideoControlHandle;
		DisplayHandle = InstallFXA(GfxBase->ActiView, &s->ViewPort, ti);

		if (DisplayHandle)
		{
			MakeScreen(s);
			RethinkDisplay();

			for (j = 0; j < NUM_LINECONTROL; j++)
			{
				(*(lc + j))->lc_Flags = LCF_MODIFY;
			}

			/* Now animate by cycling the colours, altering which
			 * line has HAM, and changing the offsets of the scroll ranges.
			 */
			k = 0;	/* for the VideoControl count. */
			last = --fvc;	/* Last in the list */
			fvc = first;
			for (j = 0; j < 500; j++)
			{
				cc = (struct ColorControl **)ColourHandle;
				cc += (y + 1);	 /* first of the non-sky colours */
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
				(*fvc)->fvc_TagList = &fvcti[1][0]; /* set to NORM */
				(*fvc)->fvc_Flags = FVCF_MODIFY;
				fvc++;
				if (k == (NUM_HAMLINES - 1))
				{
					(*first)->fvc_TagList = &fvcti[0][0]; /* set to HAM */
					(*first)->fvc_Flags = FVCF_MODIFY;
				}
				else
				{
					(*fvc)->fvc_TagList = &fvcti[0][0]; /* set to HAM */
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

				/* LineControl stuff */
				(*lc)->lc_RasInfo->RxOffset = (j * 10);
				(*(lc+1))->lc_RasInfo->RxOffset = (-j * 10);

				WaitTOF();
				AnimateFX(DisplayHandle);
			}
		}
		RemIntServer(INTB_COPER, CopInt);
		RemoveFX(DisplayHandle);
		FreeMem(CopInt, sizeof(struct Interrupt));
	}
	if (Error_c || Error_v || Error_i || Error_l)
	{
		printf("Allocation error occured\n");
	}
	FreeFX(LineControlHandle);
	FreeFX(IntControlHandle);
	FreeFX(VideoControlHandle);
	FreeFX(ColourHandle);

	CloseScreen(s);
    }

    CloseAll () ;
}
