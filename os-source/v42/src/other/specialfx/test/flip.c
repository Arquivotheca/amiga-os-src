/* This program will use the SpecialFX LineControl feature to flip 2
 * identically sized images.
 */

/*******************************************************************/
/*        TO USE ONE OF THE FOLLOWING LIBRARIES, SET ITS FLAG TO 1 */
/*                            ELSE SET IT TO 0                     */
/*******************************************************************/

#define USE_EXEC 1
#define USE_DOS 1
#define USE_INTUITION 1
#define USE_GRAPHICS 1
#define USE_SPECIALFX 1
#define USE_LAYERS 0
#define USE_ARP 0
#define USE_ICON 0
#define USE_DISKFONT 0
#define USE_IFFPARSE 1
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
#include <graphics/videocontrol.h>
#include <proto/graphics.h>
#endif
#if USE_SPECIALFX == 1
#include "/specialfx.h"
#include <proto/SpecialFX_protos.h>
#include <pragmas/SpecialFX_pragmas.h>
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
#if USE_SPECIALFX == 1
struct Library *SpecialFXBase = NULL;
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
struct RasInfo ri[4];
struct BitMapHeader bmhd[2];

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

    #if USE_SPECIALFX == 1
    if ((SpecialFXBase = OpenLibrary("specialfx.library",LIBVERSION)) == NULL)
    {
	Error("Could not open specialfx.library");
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

    if (ri[1].BitMap)
    {
	FreeBitMap(ri[1].BitMap);
    }

    if (ri[0].BitMap)
    {
	FreeBitMap(ri[0].BitMap);
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

    #if USE_SPECIALFX == 1
    if (SpecialFXBase)
    {
	CloseLibrary(SpecialFXBase);
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

void __asm ShowILBM(register __a0 struct BitMapHeader *bmhd, register __a1 struct BitMap *bm, register __a2 UBYTE *buff);
void Error (char *String);

#define ID_ILBM MAKE_ID('I','L','B','M')
#define ID_BODY MAKE_ID('B','O','D','Y')
#define ID_BMHD MAKE_ID('B','M','H','D')
#define ID_CMAP MAKE_ID('C','M','A','P')
#define ID_CAMG MAKE_ID('C','A','M','G')

ULONG *GetColours(STRPTR f1, UWORD n)
{
    #define SETRGB(p,r,g,b) {colours[(((p) * 3) + 1)] = (r); colours[(((p) * 3) + 2)] = (g); colours[(((p) * 3) + 3)] = (b);}
    struct IFFHandle *iff = NULL;
    struct StoredProperty *sp;
    ULONG *colours;
    UBYTE *cmap;

    if (colours = (ULONG *)AllocVec(((256 * 12) + 6), MEMF_CLEAR))
    {
	colours[0] = (256 << 16);
	if (iff = AllocIFF())
	{
		if ((iff->iff_Stream = Open(f1, MODE_OLDFILE)) == NULL)
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
					ri[n].BitMap = AllocBitMap(bmhd[n].w, (bmhd[n].h << 1), bmhd[n].nplanes, (BMF_CLEAR | BMF_DISPLAYABLE), NULL);
				}
				if (sp = FindProp(iff, ID_ILBM, ID_CMAP))
				{
					int i;
					cmap = (UBYTE *)sp->sp_Data;
					for (i = 0; i < (1 << bmhd[n].nplanes); i++)
					{
						SETRGB(i, ((*cmap++) << 24), ((*cmap++) << 24), ((*cmap++) << 24));
					}
				}
			}
			CloseIFF(iff);
		}
		Close(iff->iff_Stream);
		FreeIFF(iff);
	}
    }

    return(colours);
}

void ReadILBM(STRPTR ILBM, struct Screen *Screen, ULONG n)
{
    struct IFFHandle *iff = NULL;
    struct ContextNode *cn;
    APTR inbuff;

    if (iff = AllocIFF())
    {
	if ((iff->iff_Stream = Open(ILBM, MODE_OLDFILE)) == NULL)
	{
		FreeIFF(iff);
		return;	//Error("InFile did not open\n");
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
				/* Move the image down so it's in the center
				 * of the BitMap.
				 */
				BltBitMap(ri[n].BitMap, 0, 0, ri[n].BitMap, 0, (bmhd[n].h >> 2), bmhd[n].w, bmhd[n].h, 0xc0, -1, NULL);
				FreeVec(inbuff);
			}
		}
		CloseIFF(iff);
	}
	Close(iff->iff_Stream);
	FreeIFF(iff);
    }
}

void main (int argc, char *argv[])
{
    #define MODEID (PAL_MONITOR_ID | HIRES_KEY)
    #define DEPTH 8

    #define TEMPLATE "FILE1,FILE2"
    #define OPT_F1 0
    #define OPT_F2 1
    #define OPT_COUNT 2

    LONG result[OPT_COUNT];
    LONG *val;
    ULONG err_l;
    STRPTR f1 = NULL, f2 = NULL;
    struct Screen *screen;
    APTR LineControlHandle;
    APTR DisplayHandle;
    ULONG *Palette[2];	/* 2 palettes for the two entries */
    struct TagItem ti[] =
    {
	{SFX_InstallEffect, NULL},
	{TAG_DONE, NULL},
    };
    struct TagItem vc[] =
    {
	{VTAG_USERCLIP_SET, TRUE},
	{TAG_DONE, NULL},
    };

    ri[0].BitMap = ri[1].BitMap = NULL;
    Init () ;

    /* init the result[] */
    result[OPT_F1] = (ULONG)f1;
    result[OPT_F2] = (ULONG)f2;
    if (rdargs = ReadArgs(TEMPLATE, result, NULL))
    {
	f1 = (STRPTR)result[OPT_F1];
	f2 = (STRPTR)result[OPT_F2];
    }
    else
    {
    	Error("Command line error\n");

    }

    if ((Palette[0] = GetColours(f1, 0)) == NULL)
    {
	Error("colour error\n");
    }
    if ((Palette[1] = GetColours(f2, 1)) == NULL)
    {
	FreeVec(Palette[0]);
	Error("colour error\n");
    }

    if (screen = OpenScreenTags(NULL, SA_DisplayID, MODEID,
                                      SA_Depth, DEPTH,
                                      SA_Colors32, (ULONG)Palette[0],
                                      SA_BitMap, ri[0].BitMap,
                                      SA_Height, bmhd[0].h,
                                      //SA_Top, -(bmhd[0].h >> 2),
                                      TAG_DONE))
    {
	struct RasInfo ridummy = ri[1];
	ReadILBM(f1, screen, 0);
	ReadILBM(f2, screen, 1);
	if ((ri[0].BitMap == NULL) || (ri[1].BitMap == NULL))
	{
    		CloseAll();
	}

	if (LineControlHandle = AllocFX(&screen->ViewPort, SFX_LineControl, 2, &err_l))
	{
		struct LineControl **lc = (struct LineControl **)LineControlHandle;
		int i;

		/* Start by displaying image 0 in full */
		ri[0].RxOffset = 0;
		ri[0].RyOffset = (bmhd[0].h >> 2);	/* Top of image 0 */
		ri[1].RxOffset = 0;
		ri[1].RyOffset = (bmhd[0].h >> 1);	/* Mid of image 0 */

		lc[0]->lc_RasInfo = &ri[0];
		lc[0]->lc_Line = 1;
		lc[0]->lc_Count = (bmhd[0].h >> 1);
		lc[1]->lc_RasInfo = &ri[1];
		lc[1]->lc_Line = ((screen->Height >> 1) + 1);
		lc[1]->lc_Count = bmhd[0].h;

		ti[0].ti_Data = (ULONG)LineControlHandle;

		if (DisplayHandle = InstallFXA(GfxBase->ActiView, &screen->ViewPort, ti))
		{
			MakeScreen(screen);
			RethinkDisplay();

			lc[0]->lc_Flags |= LCF_MODIFY;
			lc[1]->lc_Flags |= LCF_MODIFY;
			/* Now, shrink image 0 */
			for (i = 2; i < 7; i++)
			{
				lc[0]->lc_SkipRateEven = lc[0]->lc_SkipRateOdd = i;
				lc[1]->lc_SkipRateEven = lc[1]->lc_SkipRateOdd = i;
				ri[0].RyOffset = ((bmhd[0].h >> 2) - (bmhd[0].h >> i));
				ri[1].RyOffset = ((bmhd[0].h >> 2) + (bmhd[0].h >> i));
				WaitTOF();
				AnimateFX(DisplayHandle);
				getchar();
			}
			
			RemoveFX(DisplayHandle);
		}
		FreeFX(LineControlHandle);
	}

	CloseScreen(screen);
    }

    CloseAll () ;
}
