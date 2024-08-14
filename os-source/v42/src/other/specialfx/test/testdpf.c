/******************************************************************************
*
* This program uses the specialfx.library's LineControl and VideoControl
* features to independantly scroll the odd and even bitplanes of a dualplayfield
* display. We use the VideoControl feature to disable hardware dualplayfield,
* and set up all 256 colours of a 4 and 4 dualplayfield screen to be the 256
* possible combinations of colours of two 16 colour images. Thus, when the images
* move independantly, the two seem to be transluscent.
*
* Three LineControl effects are used. The first just moves the two pictures as
* a whole. The second moves one picture whole, but the second image 'flows'
* down the lower half of the screen. The third effect moves one picture as a
* whole, but mirrors the second in the lower half of the screen.
*
******************************************************************************/

#define LIBVERSION 39


#include <exec/types.h>
#include <exec/exec.h>
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
#include <libraries/iffparse.h>
#include <proto/iffparse.h>

#include <stdio.h>
#include <stdlib.h>

struct BitMapHeader 
{
	UWORD	w, h;		/*  Width, height in pixels */
	WORD	x, y;		/*  x, y position for this bitmap  */
	UBYTE	nplanes;	/*  # of planes  */
	UBYTE	Masking;
	UBYTE	Compression;
	UBYTE	pad1;
	UWORD	TransparentColor;
	UBYTE	XAspect, YAspect;
	WORD	PageWidth, PageHeight;
};

struct IntuitionBase *IntuitionBase = NULL ;
struct GfxBase *GfxBase = NULL ;
struct Library *SpecialFXBase = NULL;
struct Library *IFFParseBase = NULL;
struct RDArgs *rdargs = NULL;
struct RasInfo ri[4];
struct BitMapHeader bmhd[2];
struct Rectangle dclip;
ULONG *colours = NULL;

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
    if ((IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", LIBVERSION)) == NULL)
    {
	Error("Could not open the Intuition.library");
    }

    /* Open the graphics library.... */
    if ((GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", LIBVERSION)) == NULL)
    {
	Error("Could not open the Graphics.library");
    }

    if ((SpecialFXBase = OpenLibrary("specialfx.library",LIBVERSION)) == NULL)
    {
	Error("Could not open specialfx.library");
    }

    /* open IFFParse library */
    if ((IFFParseBase = OpenLibrary("iffparse.library", 37L)) == NULL)
    {
	Error("Could not open the iffparse.library");
    }

    return;
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

    if (colours)
    {
    	FreeVec(colours);
    }

    if (rdargs)
    {
	FreeArgs(rdargs);
    }

    if (ri[1].BitMap)
    {
	FreeBitMap(ri[1].BitMap);
    }

    if (ri[0].BitMap)
    {
	FreeBitMap(ri[0].BitMap);
    }

    if (IFFParseBase)
    {
	CloseLibrary(IFFParseBase);
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

    return;
}

/***************************************************************************/

void __asm ShowILBM(register __a0 struct BitMapHeader *bmhd, register __a1 struct BitMap *bm, register __a2 UBYTE *buff);
void Error (char *String);

#define ID_ILBM MAKE_ID('I','L','B','M')
#define ID_BODY MAKE_ID('B','O','D','Y')
#define ID_BMHD MAKE_ID('B','M','H','D')
#define ID_CMAP MAKE_ID('C','M','A','P')
#define ID_CAMG MAKE_ID('C','A','M','G')

#define MIN(a,b) (((a) < (b)) ? (a) : (b))

/*
 * GetColours() returns a pointer to a table of ULONGs which can then be passed to
 * LoadRGB32(). The colours in the table are a combination of the colours in two
 * IFF pictures of the same depth. For example, if the two images are four bitplanes
 * deep, then there are 16 * 16 = 256 different combinations of the colours.
 *
 * This function was written specifically for two 4 bitplane images.
 *
 * See the autodocs for graphics.library/LoadRGB32() for details of the format
 * of the colour table.
 */
ULONG *GetColours(STRPTR f1, STRPTR f2)
{
    #define SETRGB(p,r,g,b) {colours[(((p) * 3) + 1)] = (r); colours[(((p) * 3) + 2)] = (g); colours[(((p) * 3) + 3)] = (b);}
    #define GETRGB(p,t) {((t)[0]) = colours[(((p) * 3) + 1)]; ((t)[1]) = colours[(((p) * 3) + 2)]; ((t)[2]) = colours[(((p) * 3) + 3)];}
    struct IFFHandle *iff = NULL;
    struct StoredProperty *sp;
    ULONG *colours;
    ULONG n;
    UBYTE *cmap;
    UBYTE remap[] =
    {
	0, 1, 4, 5,
	16, 17, 20, 21,
	64, 65, 68, 69,
	80, 81, 84, 85,
    };

    if (colours = (ULONG *)AllocVec(((256 * 12) + 6), MEMF_CLEAR))
    {
	/* show 256 colours */
	colours[0] = (256 << 16);

	for (n = 0; n < 2; n++)
	{
		if (iff = AllocIFF())
		{
			if ((iff->iff_Stream = Open((n == 0 ? f1 : f2), MODE_OLDFILE)) == NULL)
			{
				FreeIFF(iff);
				FreeVec(colours);
				return(NULL);
			}
			InitIFFasDOS(iff);
			if (OpenIFF(iff, IFFF_READ) == NULL)
			{
				if ((PropChunk(iff, ID_ILBM, ID_BMHD) == 0) &&
				    (PropChunk(iff, ID_ILBM, ID_CMAP) == 0) &&
				    (StopChunk(iff, ID_ILBM, ID_BODY) == 0) &&
				    (ParseIFF(iff, IFFPARSE_SCAN) == 0))
				{
					if (sp = FindProp(iff, ID_ILBM, ID_BMHD))
					{
						bmhd[n] = (*(struct BitMapHeader *)sp->sp_Data);
						ri[n].BitMap = AllocBitMap(bmhd[n].w, bmhd[n].h, bmhd[n].nplanes, (BMF_CLEAR | BMF_DISPLAYABLE), NULL);
					}
					if (sp = FindProp(iff, ID_ILBM, ID_CMAP))
					{
						ULONG RGB[3];
						ULONG tmp[3];
						int i, j;
						cmap = (UBYTE *)sp->sp_Data;
						for (i = 0; i < (1 << bmhd[n].nplanes); i++)
						{
							if (n == 0)
							{
								SETRGB(remap[i], ((*cmap++) << 24), ((*cmap++) << 24), ((*cmap++) << 24));
							}
							else
							{
								RGB[0] = ((*cmap++) << 24);
								RGB[1] = ((*cmap++) << 24);
								RGB[2] = ((*cmap++) << 24);
								for (j = 0; j < (1 << bmhd[0].nplanes); j++)
								{
									GETRGB(remap[j], &tmp[0]);
									SETRGB(((remap[i] << 1) + remap[j]),
									       (MIN(((RGB[0] >> 8) + (tmp[0] >> 8)), 0xffffff) << 8),
									       (MIN(((RGB[1] >> 8) + (tmp[1] >> 8)), 0xffffff) << 8),
									       (MIN(((RGB[2] >> 8) + (tmp[2] >> 8)), 0xffffff) << 8));
								}
							}
						}
					}
				}
				CloseIFF(iff);
			}
			Close(iff->iff_Stream);
			FreeIFF(iff);
		}
	}
    }

    return(colours);
}

/*
 * ReadILBM() reads the IFF ILBM file into a a BitMap.
 */

BOOL ReadILBM(STRPTR ILBM, ULONG n)
{
    struct IFFHandle *iff = NULL;
    struct ContextNode *cn;
    APTR inbuff;
    BOOL retval = FALSE;

    if (iff = AllocIFF())
    {
	if ((iff->iff_Stream = Open(ILBM, MODE_OLDFILE)) == NULL)
	{
		FreeIFF(iff);
		return(FALSE);
	}
	InitIFFasDOS(iff);
	if (OpenIFF(iff, IFFF_READ) == NULL)
	{
		if ((StopChunk(iff, ID_ILBM, ID_BODY) == 0) &&
		    (ParseIFF(iff, IFFPARSE_SCAN) == 0))
		{
			/* we are at the BODY chunk */
			if ((cn = CurrentChunk(iff)) &&
			    (inbuff = AllocVec(cn->cn_Size, 0)))
			{
				ReadChunkBytes(iff, inbuff, cn->cn_Size);
				ShowILBM(&bmhd[n], ri[n].BitMap, inbuff);
				FreeVec(inbuff);
				retval = TRUE;
			}
		}
		CloseIFF(iff);
	}
	Close(iff->iff_Stream);
	FreeIFF(iff);
    }
    return(retval);
}

void CalcDClip(ULONG ModeID, struct Rectangle *dclip)
{
    struct DimensionInfo dims;

    if (GetDisplayInfoData(NULL, (UBYTE *)&dims, sizeof(struct DimensionInfo), DTAG_DIMS, ModeID))
    {
	/* The height of the DClip is the same as the StdOScan set in
	 * preferences, but the width is the Nominal width, positioned as
	 * far right as possible.
	 */
	dclip->MinY = dims.StdOScan.MinY;
	dclip->MaxY = dims.StdOScan.MaxY;
	dclip->MinX = (dims.MaxOScan.MaxX - (dims.Nominal.MaxX - dims.Nominal.MinX + 1));
	dclip->MaxX = dims.MaxOScan.MaxX;
    }
    else
    {
    	/* build a default dclip */
    	dclip->MinY = 0;
    	dclip->MaxY = 200;
    	dclip->MinX = 0;
    	dclip->MaxX = 320;
    }
}

void main (int argc, char *argv[])
{
    #define MODEID LORES_KEY
    #define DEPTH 4
    #define SKIP skip

    #define TEMPLATE "FILE1,FILE2,SKIP/N"
    #define OPT_F1 0
    #define OPT_F2 1
    #define OPT_SKIP 2
    #define OPT_COUNT 3

    LONG result[OPT_COUNT];
    LONG *val;
    ULONG err_l, err_v, err_install;
    STRPTR f1 = NULL, f2 = NULL;
    struct Screen *screen;
    APTR LineControlHandle;
    APTR VideoControlHandle = NULL;
    APTR DisplayHandle;
    struct TagItem ti[] =
    {
	{SFX_InstallEffect, NULL},
	{SFX_InstallEffect, NULL},
	{SFX_DoubleBuffer, TRUE},	/* enable copperlist doublebuffering */
	{SFX_InstallErrorCode, NULL},
	{TAG_DONE, NULL},
    };
    struct TagItem vc[] =
    {
	{VTAG_USERCLIP_SET, TRUE},	/* UserCopperList clipping */
	{VTAG_FULLPALETTE_SET, TRUE},	/* Use all 256 colours */
	{TAG_DONE, NULL},
    };
    struct TagItem fvcti[] =
    {
    	VTAG_SFX_NORM_SET, NULL,	/* VideoControl effect to disable
					 * DualPlayfield mode in hardware
					 */
	TAG_DONE, NULL,
    };
    UWORD skip = 1;

    /* Initialise the two BitMaps */
    ri[0].BitMap = ri[1].BitMap = NULL;

    /* Open the libraries */
    Init () ;

    /* init the result[] */
    result[OPT_F1] = (ULONG)f1;
    result[OPT_F2] = (ULONG)f2;
    result[OPT_SKIP] = (ULONG)&skip;
    if (rdargs = ReadArgs(TEMPLATE, result, NULL))
    {
	f1 = (STRPTR)result[OPT_F1];
	f2 = (STRPTR)result[OPT_F2];
	if (val = (LONG *)result[OPT_SKIP])
	{
		skip = *val;
	}
    }
    else
    {
    	Error("Command line error\n");

    }

    /* Get our ColourTable to pass to OpenScreenTags() */
    if ((colours = GetColours(f1, f2)) == NULL)
    {
	Error("colour error\n");
    }

    if ((ri[0].BitMap == NULL) || (ri[1].BitMap == NULL))
    {
    	Error("Not enough memory for BitMaps\n");
    }

    /* Calculate the DClip required. The LineControl has a number of
     * limitations, one of which is that the scrolling only works
     * properly if the display is pushed all the way to the right of the
     * screen when in Lores 4x mode.
     */

    CalcDClip(MODEID, &dclip);
    
    if (screen = OpenScreenTags(NULL, SA_DisplayID, MODEID,
                                      SA_Depth, DEPTH,
                                      SA_Behind, TRUE,
                                      SA_Colors32, (ULONG)colours,
                                      SA_ColorMapEntries, 256,
                                      SA_VideoControl, vc,
                                      SA_BitMap, ri[0].BitMap,
                                      SA_DClip, &dclip,
                                      TAG_DONE))
    {
	struct RasInfo ridummy = ri[1];

	if ((ReadILBM(f1, 0) == FALSE) ||
	    (ReadILBM(f2, 1) == FALSE))
	{
		Error("ILBM error\n");
	}

	/* Force intuition into building a DualPlayfield screen. */
	ri[0].Next = &ri[1];
	screen->ViewPort.RasInfo->Next = &ridummy;
	screen->ViewPort.Modes |= DUALPF;
	ScreenToFront(screen);
	MakeScreen(screen);
	RethinkDisplay();

	/* Allocate our Handles for the specialfx. */
	if ((LineControlHandle = AllocFX(&screen->ViewPort, SFX_LineControl, 1, &err_l)) &&
	   (VideoControlHandle = AllocFX(&screen->ViewPort, SFX_FineVideoControl, 1, &err_v)))
	{
		struct LineControl **lc = (struct LineControl **)LineControlHandle;
		struct FineVideoControl **fvc = (struct FineVideoControl **)VideoControlHandle;

		fvc[0]->fvc_TagList = fvcti;
		fvc[0]->fvc_Line = 0;
		fvc[0]->fvc_Count = -1;	/* This effect lasts for the whole screen */

		lc[0]->lc_RasInfo = &ri[0];
		/* start this effect at line 1, and display the image from
		 * line 1 of the BitMap.
		 */
		lc[0]->lc_Line = lc[0]->lc_RasInfo->RyOffset = 1;
		/* The effect lasts for the whole screen. */
		lc[0]->lc_Count = screen->Height - 1;
		/* The second playfield also displays from line 1 of the BitMap. */
		lc[0]->lc_RasInfo->Next->RyOffset = 1;

		/* Install the effects */
		ti[0].ti_Data = (ULONG)VideoControlHandle;
		ti[1].ti_Data = (ULONG)LineControlHandle;
		ti[3].ti_Data = (ULONG)&err_install;
		if (DisplayHandle = InstallFXA(GfxBase->ActiView, &screen->ViewPort, ti))
		{
			WORD xpos[2], ypos[2], xdir[2], ydir[2];
			UWORD xmax[2], ymax[2];
			int j;

			/* Display the effect */
			MakeScreen(screen);
			RethinkDisplay();

			/* Show that the LineControl will be animated */
			lc[0]->lc_Flags |= LCF_MODIFY;

			/* calculate the extent and direction of the playfield
			 * animations.
			 */
			xpos[0] = ypos[0] = xpos[1] = ypos[1] = 0;
			xdir[0] = ydir[0] = SKIP; 
			xdir[1] = ydir[1] = (SKIP << 1);
			xmax[0] = (bmhd[1].w - screen->Width);
			xmax[1] = (bmhd[0].w - screen->Width);
			ymax[0] = (bmhd[1].h - screen->Height);
			ymax[1] = (bmhd[0].h - screen->Height);

			/* Animate until a ctrl-C */
			while (!CheckSignal(SIGBREAKF_CTRL_C))
			{
				for (j = 0; j < 2; j++)
				{
					ri[j].RyOffset += ydir[j];
					ri[j].RxOffset += xdir[j];
					if (ri[j].RyOffset < 0)
					{
						ri[j].RyOffset = 0;
						ydir[j] = -ydir[j];
					}
					if (ri[j].RyOffset > ymax[j])
					{
						ri[j].RyOffset = ymax[j];
						ydir[j] = -ydir[j];
					}
					if (ri[j].RxOffset < 0)
					{
						ri[j].RxOffset = 0;
						xdir[j] = -xdir[j];
					}
					if (ri[j].RxOffset > xmax[j])
					{
						ri[j].RxOffset = xmax[j];
						xdir[j] = -xdir[j];
					}
				}
				WaitTOF();
				AnimateFX(DisplayHandle);
			}
			RemoveFX(DisplayHandle);
		}
		else
		{
			if (err_install == SFX_ERR_Animating)
			{
				printf("Animating\n");
			}
		}

		/* We are finished with the LineControl, but not the
		 * VideoControl.
		 */
		FreeFX(LineControlHandle);
	}

	/* This effect will scroll one of the playfields whole, whilst the
	 * second playfield will "flow" down the bottom half of the screen.
	 * This involves two separate LineControl features, one for the top
	 * half, the other for the bottom half. The playfield shown whole is
	 * set up so the two halves connect.
	 */

	if (LineControlHandle = AllocFX(&screen->ViewPort, SFX_LineControl, 2, &err_l))
	{
		struct LineControl **lc = (struct LineControl **)LineControlHandle;
		int j;

		ri[2] = ri[0];
		ri[3] = ri[1];
		ri[2].Next = &ri[3];

		ri[0].RxOffset = 0;
		ri[2].RxOffset = 0;
		ri[0].RyOffset = 1;
		ri[2].RyOffset = (screen->Height >> 1);
		ri[1].RxOffset = 0;
		ri[1].RyOffset = 1;
		ri[3].RxOffset = 0;
		ri[3].RyOffset = 1;

		lc[0]->lc_RasInfo = &ri[0];
		lc[0]->lc_Line = 1;
		lc[0]->lc_Count = (screen->Height >> 1);

		lc[1]->lc_RasInfo = &ri[2];
		lc[1]->lc_Line = ((screen->Height >> 1) + 1);
		lc[1]->lc_Count = (screen->Height >> 1);

		/* Keep the VideoControl handle, but install the new
		 * LineControl handle. 
		 */
		ti[1].ti_Data = (ULONG)LineControlHandle;

		if (DisplayHandle = InstallFXA(GfxBase->ActiView, &screen->ViewPort, ti))
		{
			#define AREAS 4
			WORD xpos[AREAS], ypos[AREAS], xdir[AREAS], ydir[AREAS];
			UWORD xmax[AREAS], ymax[AREAS];

			MakeScreen(screen);
			RethinkDisplay();

			lc[0]->lc_Flags |= LCF_MODIFY;
			lc[1]->lc_Flags |= LCF_MODIFY;
			lc[1]->lc_SkipRateOdd = 0;	/* repeat */

			xpos[0] = ypos[0] = xpos[1] = ypos[1] = xpos[3] = ypos[3] = 0;
			xdir[0] = ydir[0] = xdir[2] = ydir[2] = SKIP; 
			xdir[1] = ydir[1] = (SKIP << 1);
			xdir[3] = ydir[3] = (SKIP * 3);
			xmax[0] = (bmhd[1].w - screen->Width);
			xmax[1] = xmax[3] = (bmhd[0].w - screen->Width);
			ymax[0] = (bmhd[1].h - screen->Height);
			ymax[1] = ymax[3] = (bmhd[0].h - (screen->Height >> 1));

			while (!CheckSignal(SIGBREAKF_CTRL_C))
			{
				for (j = 0; j < AREAS; j++)
				{
					ri[j].RyOffset += ydir[j];
					ri[j].RxOffset += xdir[j];
					if (ri[j].RyOffset < 0)
					{
						ri[j].RyOffset = 0;
						ydir[j] = -ydir[j];
					}
					if (ri[j].RyOffset > ymax[j])
					{
						ri[j].RyOffset = ymax[j];
						ydir[j] = -ydir[j];
					}
					if (ri[j].RxOffset < 0)
					{
						ri[j].RxOffset = 0;
						xdir[j] = -xdir[j];
					}
					if (ri[j].RxOffset > xmax[j])
					{
						ri[j].RxOffset = xmax[j];
						xdir[j] = -xdir[j];
					}
				}
				ri[2].RxOffset = ri[0].RxOffset;
				ri[2].RyOffset = (ri[0].RyOffset + lc[0]->lc_Count);
				ri[3].RxOffset = ri[1].RxOffset;
				ri[3].RyOffset = (ri[1].RyOffset + lc[1]->lc_Count);
				WaitTOF();
				AnimateFX(DisplayHandle);
			}

			RemoveFX(DisplayHandle);
		}
		else
		{
			if (err_install == SFX_ERR_Animating)
			{
				printf("Animating\n");
			}
		}
		FreeFX(LineControlHandle);
	}

	/* This effect is similar, except the bottom half of one of the
	 * playfields is a mirror image of the top half.
	 */

	if (LineControlHandle = AllocFX(&screen->ViewPort, SFX_LineControl, 2, &err_l))
	{
		struct LineControl **lc = (struct LineControl **)LineControlHandle;
		int j;

		ri[2] = ri[0];
		ri[3] = ri[1];
		ri[2].Next = &ri[3];

		ri[0].RxOffset = 0;
		ri[2].RxOffset = 0;
		ri[0].RyOffset = 1;
		ri[2].RyOffset = (screen->Height >> 1);
		ri[1].RxOffset = 0;
		ri[1].RyOffset = 1;
		ri[3].RxOffset = 0;
		ri[3].RyOffset = 1;

		lc[0]->lc_RasInfo = &ri[0];
		lc[0]->lc_Line = 1;
		lc[0]->lc_Count = (screen->Height >> 1);

		lc[1]->lc_RasInfo = &ri[2];
		lc[1]->lc_Line = ((screen->Height >> 1) + 1);
		lc[1]->lc_Count = (screen->Height >> 1);

		ti[1].ti_Data = (ULONG)LineControlHandle;

		if (DisplayHandle = InstallFXA(GfxBase->ActiView, &screen->ViewPort, ti))
		{
			#define AREAS 4
			WORD xpos[AREAS], ypos[AREAS], xdir[AREAS], ydir[AREAS];
			UWORD xmax[AREAS], ymax[AREAS];

			MakeScreen(screen);
			RethinkDisplay();

			lc[0]->lc_Flags |= LCF_MODIFY;
			lc[1]->lc_Flags |= LCF_MODIFY;
			lc[1]->lc_SkipRateOdd = -1;	/* mirror */

			xpos[0] = ypos[0] = xpos[1] = ypos[1] = xpos[3] = ypos[3] = 0;
			xdir[0] = ydir[0] = xdir[2] = ydir[2] = SKIP; 
			xdir[1] = ydir[1] = (SKIP << 1);
			xdir[3] = ydir[3] = (SKIP * 3);
			xmax[0] = (bmhd[1].w - screen->Width);
			xmax[1] = xmax[3] = (bmhd[0].w - screen->Width);
			ymax[0] = (bmhd[1].h - screen->Height);
			ymax[1] = ymax[3] = (bmhd[0].h - (screen->Height >> 1) - 1);

			while (!CheckSignal(SIGBREAKF_CTRL_C))
			{
				for (j = 0; j < AREAS; j++)
				{
					ri[j].RyOffset += ydir[j];
					ri[j].RxOffset += xdir[j];
					if (ri[j].RyOffset < 0)
					{
						ri[j].RyOffset = 0;
						ydir[j] = -ydir[j];
					}
					if (ri[j].RyOffset > ymax[j])
					{
						ri[j].RyOffset = ymax[j];
						ydir[j] = -ydir[j];
					}
					if (ri[j].RxOffset < 0)
					{
						ri[j].RxOffset = 0;
						xdir[j] = -xdir[j];
					}
					if (ri[j].RxOffset > xmax[j])
					{
						ri[j].RxOffset = xmax[j];
						xdir[j] = -xdir[j];
					}
				}
				ri[2].RxOffset = ri[0].RxOffset;
				ri[2].RyOffset = (ri[0].RyOffset + lc[0]->lc_Count);
				ri[3].RxOffset = ri[1].RxOffset;
				ri[3].RyOffset = (ri[1].RyOffset + lc[1]->lc_Count);
				WaitTOF();
				AnimateFX(DisplayHandle);
			}

			RemoveFX(DisplayHandle);
		}
		else
		{
			if (err_install == SFX_ERR_Animating)
			{
				printf("Animating\n");
			}
		}
		FreeFX(LineControlHandle);
	}

	FreeFX(VideoControlHandle);
	screen->ViewPort.RasInfo->Next = NULL;
	CloseScreen(screen);
    }

    CloseAll () ;
}
