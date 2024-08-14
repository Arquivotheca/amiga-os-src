/*
 * 8hamdemo.c - shows off 262144 colors simultaneously
 *
 * (c) Copyright 1992 Commodore-Amiga, Inc.  All rights reserved.
 *
 * This software is provided as-is and is subject to change; no warranties
 * are made.  All use is at your own risk.  No liability or responsibility
 * is assumed.
 * 
 */

#include <exec/types.h>
#include <graphics/displayinfo.h>
#include <intuition/intuition.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/intuition_protos.h>

/*----------------------------------------------------------------------*/

void error_exit(STRPTR errorstring);

int CXBRK(void) { return(0); }
void chkabort(void) { return; }

/*----------------------------------------------------------------------*/

struct Library *DiskfontBase = NULL;
struct GfxBase *GfxBase = NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct Screen *myscreen = NULL;
struct Window *mywindow = NULL;
struct BitMap *bm[2] = {NULL,NULL};
struct RastPort temprp;
struct TextFont *font = NULL;

/*----------------------------------------------------------------------*/

UWORD pens[] =
{
	0, /* DETAILPEN */
	1, /* BLOCKPEN	*/
	1, /* TEXTPEN	*/
	2, /* SHINEPEN	*/
	1, /* SHADOWPEN	*/
	3, /* FILLPEN	*/
	1, /* FILLTEXTPEN	*/
	0, /* BACKGROUNDPEN	*/
	2, /* HIGHLIGHTTEXTPEN	*/

	1, /* BARDETAILPEN	*/
	2, /* BARBLOCKPEN	*/
	1, /* BARTRIMPEN	*/

	~0,
};


/* Color table to pass to SA_Colors32 or LoadRGB32() */
ULONG colortable[] =
{
	0x00040000,	/* 5 colors to load, from zero */

	0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA,	/* Gray */
	0x00000000, 0x00000000, 0x00000000,	/* Black */
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,	/* White */
	0x66666666, 0x88888888, 0xBBBBBBBB,	/* Blue-gray */

	0x00000000,	/* terminator */
};

/*----------------------------------------------------------------------*/

#define PLOTWIDTH 64
#define PLOTHEIGHT 64


/*----------------------------------------------------------------------*/

/* Let's arrange the blue values in a spiral, just to be cute.
 * We'll do it with a table instead of an algorithm out of
 * laziness.
 */
int bluespiral1[] =
{
    56, 55, 54, 53, 52, 51, 50, 49,
    57, 30, 29, 28, 27, 26, 25, 48,
    58, 31, 12, 11, 10,  9, 24, 47,
    59, 32, 13,  2,  1,  8, 23, 46,
    60, 33, 14,  3,  0,  7, 22, 45,
    61, 34, 15,  4,  5,  6, 21, 44,
    62, 35, 16, 17, 18, 19, 20, 43,
    63, 36, 37, 38, 39, 40, 41, 42,
};

int bluespiral2[] =
{
    60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45,
    61, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 44,
    62, 27,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 43,
    63, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
};

/*----------------------------------------------------------------------*/

struct TextAttr topaz80 =
{
    "topaz.font",
    8,
    0,
    0,
};

struct TextAttr preffont =
{
    "times.font",
    15,
    0,
    0,
};

#define MOD_BLUE	64
#define MOD_RED		128
#define MOD_GREEN	192

#define XOFF 6
#define YOFF 5

main(argc, argv)
int argc;
char *argv[];
{
    int blue, x, ybase, column;
    int redreverse, red, dred;
    int left, top, width, height, b;
    int perrow = 16;
    int *bluespiral = bluespiral2;
    struct TextAttr *scfont = &topaz80;

    ULONG displayID = NTSC_MONITOR_ID | SUPERHAMLACE_KEY;
    int overscan = OSCAN_TEXT;

    if ( ( argc > 1 ) && ( *argv[1] == '?' ) )
    {
	printf("8HAMDemo [S|P|D|N]\n");
	printf("- S = use Super72\n");
	printf("- P = use PAL hires-lace\n");
	printf("- D = use double-PAL hires-lace\n");
	printf("- N = use NTSC superhires-lace [default]\n");
    }
    else
    {
	if ( ( argc > 1 ) && ( ( *argv[1] == 'D' ) || ( *argv[1] == 'd' ) ) )
	{
	    displayID = DBLPALHIRESHAMFF_KEY;
	    overscan = OSCAN_MAX;
	    perrow = 8;
    	    bluespiral = bluespiral1;
	}

	if ( ( argc > 1 ) && ( ( *argv[1] == 'P' ) || ( *argv[1] == 'p' ) ) )
	{
	    displayID = PAL_MONITOR_ID | HIRESHAMLACE_KEY;
	    overscan = OSCAN_MAX;
	    perrow = 8;
    	    bluespiral = bluespiral1;
	}

	if ( ( argc > 1 ) && ( ( *argv[1] == 'S' ) || ( *argv[1] == 's' ) ) )
	{
	    displayID = 0x89824;
	    overscan = OSCAN_TEXT;
	    perrow = 8;
    	    bluespiral = bluespiral1;
	}

	if ( ( argc > 1 ) && ( ( *argv[1] == 'N' ) || ( *argv[1] == 'n' ) ) )
	{
	    displayID = NTSC_MONITOR_ID | SUPERHAMLACE_KEY;
	    overscan = OSCAN_TEXT;
	    perrow = 16;
    	    bluespiral = bluespiral2;
	}

	if (!( GfxBase = (struct GfxBase *)
	    OpenLibrary("graphics.library", 39L) ))
	    {
	    error_exit("Couldn't open Gfx V39\n");
	    }

	if (!( IntuitionBase = (struct IntuitionBase *)
	    OpenLibrary("intuition.library", 39L) ))
	    {
	    error_exit("Couldn't open Intuition V39\n");
	    }

	if (!( DiskfontBase = (struct Library *)
	    OpenLibrary("diskfont.library", 37L) ))
	    {
	    error_exit("Couldn't open Diskfont V37\n");
	    }

	if ( font = OpenDiskFont( &preffont ) )
	{
	    scfont = &preffont;
	}

	InitRastPort( &temprp );
	bm[0] = AllocBitMap( PLOTWIDTH, PLOTHEIGHT, 8, BMF_INTERLEAVED|BMF_CLEAR, NULL );
	temprp.BitMap = bm[0];
	for ( x=0; x<64; x++ )
	{
	    SetAPen( &temprp, MOD_GREEN+x );
	    Move( &temprp, x, 0 );
	    Draw( &temprp, x, 63 );
	}

	bm[1] = AllocBitMap( PLOTWIDTH, PLOTHEIGHT, 8, BMF_INTERLEAVED|BMF_CLEAR, NULL );
	temprp.BitMap = bm[1];

	for ( x=0; x<64; x++ )
	{
	    SetAPen( &temprp, MOD_GREEN+63-x );
	    Move( &temprp, x, 0 );
	    Draw( &temprp, x, 63 );
	}

	if (!(myscreen = OpenScreenTags(NULL,
	    SA_DisplayID, displayID,
	    SA_Overscan, overscan,
	    /*  Other tags can go here: */
	    SA_Font, scfont,
	    SA_Depth, 8,
	    SA_AutoScroll, 1,
	    SA_Pens, pens,
	    SA_Interleaved, TRUE,
	    SA_Title, "8-plane HAM Demo",
	    SA_Colors32, colortable,
	    SA_Behind, TRUE,
	    TAG_DONE )))
	    {
	    error_exit("Couldn't open screen\n");
	    }

	width = 14 + (perrow*65);
	height = 10 + scfont->ta_YSize + (64/perrow)*64;
	left = ( myscreen->Width - width - scfont->ta_YSize - 2 ) / 2;
	top = ( myscreen->Height - height ) / 2;

	if (!( mywindow = OpenWindowTags(NULL,
	    WA_Left, left,
	    WA_Top, top,
	    WA_Width, width,
	    WA_Height, height,
	    WA_MinWidth, 50,
	    WA_MinHeight, 40,
	    WA_MaxWidth, -1,
	    WA_MaxHeight, -1,
	    WA_IDCMP, CLOSEWINDOW|VANILLAKEY,
	    WA_DepthGadget, TRUE,
	    WA_DragBar, TRUE,
	    WA_Title, "262144 Colors!",
	    WA_CloseGadget, TRUE,
	    WA_NoCareRefresh, TRUE,
	    WA_SmartRefresh, TRUE,
	    WA_CustomScreen, myscreen,
	    WA_Activate, TRUE,
	    TAG_DONE ) ))
	    {
	    error_exit("Couldn't open window\n");
	    }

	b = 0;
	ybase = 0;
	column = 0;
	redreverse=0;

	for ( blue = 0; blue < 64; blue++ )
	{
	    if ( column == 0 )
	    {
		int dy;

		SetAPen( mywindow->RPort, 1 );
		Move( mywindow->RPort, XOFF-1, (YOFF+scfont->ta_YSize)+ybase );
		Draw( mywindow->RPort, XOFF-1, (YOFF+scfont->ta_YSize)+ybase+63 );

		for ( dy = 0; dy < 64; dy++ )
		{
		    if ( redreverse )
		    {
			SetAPen( mywindow->RPort, MOD_RED + 63-dy );
		    }
		    else
		    {
			SetAPen( mywindow->RPort, MOD_RED + dy );
		    }
		    WritePixel( mywindow->RPort, XOFF, (YOFF+scfont->ta_YSize)+ybase+dy );
		}
	    }

	    SetAPen( mywindow->RPort, MOD_BLUE+63-bluespiral[blue]);
	    Move( mywindow->RPort, XOFF+column*65+1, (YOFF+scfont->ta_YSize)+ybase );
	    Draw( mywindow->RPort, XOFF+column*65+1, (YOFF+scfont->ta_YSize)+ybase+63 );
	
	    BltBitMapRastPort( bm[b], 0, 0,
		mywindow->RPort, XOFF+column*65 + 2, (YOFF+scfont->ta_YSize)+ybase,
		PLOTWIDTH, PLOTHEIGHT, 0xC0 );
	    b = 1-b;

	    column++;
	    if ( column == perrow )
	    {
		column = 0;
		ybase += 64;
		redreverse = 1-redreverse;
	    }
	}
	ScreenToFront( myscreen );
	dred = 1; red = 0;
	/* Exit on break or on any IntuiMessage */
	while ( !( SetSignal(0,0) & ( ( 1 << mywindow->UserPort->mp_SigBit ) | SIGBREAKF_CTRL_C ) ) )
	{
	    ScrollRaster( mywindow->RPort, 0, -1, XOFF, (YOFF+scfont->ta_YSize), XOFF, (YOFF+scfont->ta_YSize)+(4096/perrow)-1 );
	    SetAPen( mywindow->RPort, MOD_RED + red );
	    red += dred;
	    if ( red == -1 )
	    {
		dred = 1;
		red = 0;
	    }
	    else if (red == 64 )
	    {
		red = 63;
		dred = -1;
	    }
	    WritePixel( mywindow->RPort, XOFF, (YOFF+scfont->ta_YSize) );
	}
    }
    error_exit(NULL);
}


/*----------------------------------------------------------------------*/

void error_exit(STRPTR errorstring)

    {
    if (mywindow)
	{
	ScreenToBack(myscreen);
	CloseWindow(mywindow);
	}

    if (myscreen)
	{
	CloseScreen(myscreen);
	}

    if ( bm[1] ) FreeBitMap( bm[1] );
    if ( bm[0] ) FreeBitMap( bm[0] );

    if (font) CloseFont(font);

    if (DiskfontBase)
	{
	CloseLibrary(DiskfontBase);
	}

    if (IntuitionBase)
	{
	CloseLibrary(IntuitionBase);
	}

    if (GfxBase)
	{
	CloseLibrary(GfxBase);
	}

    if (errorstring)
	{
	printf(errorstring);
	exit(20);
	}

    exit(0);
    }


/*----------------------------------------------------------------------*/
