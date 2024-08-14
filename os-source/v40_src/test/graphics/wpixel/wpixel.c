/*
	wpixel.c -- Mark Dolecki
	Program to test WritePixelArray8 on new hardware
*/

/* Includes */
#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <graphics/rastport.h>
#include <graphics/displayinfo.h>
#include <intuition/intuition.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/gadgetclass.h>
#include <intuition/intuitionbase.h>
#include <intuition/screens.h>
#include <libraries/asl.h>
#include <libraries/gadtools.h>
#include <clib/alib_protos.h>
#include <clib/alib_stdio_protos.h>
#include <clib/asl_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/layers_protos.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/graphics_pragmas.h>

/* Defines */
#define DEBUG TRUE
#define NUMWINS 10
#define MAXDEPTH 8
#define NUMREFRESH 3
#define RECTWIDTH 10
#define RECTHEIGHT 10

#define GD_Depth                               0
#define GD_depth1                              1
#define GD_depth2                              2
#define GD_depth3                              3
#define GD_depth4                              4
#define GD_depth5                              5
#define GD_depth6                              6
#define GD_depth7                              7
#define GD_depth8                              8
#define GD_layered                             9
#define GD_nonlayered                          10
#define GD_simple                              11
#define GD_smart                               12
#define GD_super                               13
#define GD_lay                                 14
#define GD_refresh                             15
#define GD_mask                                16
#define GD_ranmask                             17
#define GD_negmask                             18
#define GD_nummasks                            19

#define GDX_Depth                              0
#define GDX_depth1                             1
#define GDX_depth2                             2
#define GDX_depth3                             3
#define GDX_depth4                             4
#define GDX_depth5                             5
#define GDX_depth6                             6
#define GDX_depth7                             7
#define GDX_depth8                             8
#define GDX_layered                            9
#define GDX_nonlayered                         10
#define GDX_simple                             11
#define GDX_smart                              12
#define GDX_super                              13
#define GDX_lay                                14
#define GDX_refresh                            15
#define GDX_mask                               16
#define GDX_ranmask                            17
#define GDX_negmask                            18
#define GDX_nummasks                           19

 /* Globals */
struct GadToolsBase *GadToolsBase = NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase = NULL;
struct LayersBase *LayersBase = NULL;

struct Screen *TestScreen = NULL;
struct Window *TestWins[NUMWINS];
struct RastPort *RPs[NUMWINS];
struct BitMap *TempBMs[NUMWINS];
struct RastPort *TempRPs[NUMWINS];

UBYTE *vers = "$VER: wpixel 2.1 (12-May-1993) by Mark Dolecki";

/* struct Window *TestWindow1 = NULL;
struct Window *TestWindow2 = NULL;
struct RastPort *rp1 = NULL;
struct RastPort *rp2 = NULL;
struct BitMap   *tempbm1 = NULL;
struct BitMap   *tempbm2 = NULL;
struct RastPort *temprp1 = NULL;
struct RastPort *temprp2 = NULL; */

UBYTE *pixelarray = NULL;      /* [((((RECTWIDTH+15)>>4)<<4)*(RECTHEIGHT + 1))]; */
long pixelarraywidth = 0L;
long pixelarrayheight = 0L;
struct ScreenModeRequester *smr;
ULONG PixCheckSum = 0;
int NumTests = 0;
int numbytes = 0;
int depth = 0;
BOOL deptharray[8] = { TRUE,TRUE,TRUE,TRUE,TRUE,TRUE,TRUE,TRUE };
BOOL layerarray[2] = { TRUE,TRUE };
BOOL refresharray[3] = { TRUE,TRUE,TRUE };
BOOL maskarray[2] = { TRUE,TRUE };
int nummasks = 3;
int rectwidth = 10;
int rectheight = 10;

struct Screen        *Scr = NULL;
APTR                  VisualInfo = NULL;
struct Window        *WPWnd = NULL;
struct Gadget        *WPGList = NULL;
struct Gadget        *WPGadgets[20];
UWORD                 OffX = 0, OffY = 0;
UWORD                 WPLeft = 1;
UWORD                 WPTop = 11;
UWORD                 WPWidth = 286;
UWORD                 WPHeight = 147;
UBYTE                *WPWdt = (UBYTE *)"WPixel";
struct TextAttr      *Font = NULL, Attr;
UWORD                 FontX = 0, FontY = 0;
UWORD                 OffX, OffY;

/* Function Protos */
void dPrintf(char *s);
void unclipWindow(struct Window *);
struct Region *clipWindowToBorders(struct Window *win);
struct Region *clipWindow(struct Window *,LONG,LONG,LONG,LONG);
int SetupScreen( void );
void CloseDownScreen( void );
int OpenWPWindow( void );
void CloseWPWindow( void );
void OpenLibs(void);
void CloseLibs(void);
void ChooseScreenMode(void);
void KillTestWindow(struct Window *, struct RastPort *, struct BitMap *);
ULONG GetChecksum(struct Window *);
void PrintScreenAttrs(struct Screen *);
void showTestParams(int,int,int,int,int,ULONG, ULONG);
void allocatePixelArray(struct Window *);
void freePixelArray(void);
void initPixelArray(void);
void DoPlot(struct Window *, struct RastPort *, struct RastPort *);
void DoTest(int,int,int,int,int);
void showSelections(void);
void QUIT(void);
extern void KPrintF(STRPTR,...);

/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
	OpenLibs() -
		Input -
			none
		Function -
			Opens all necessary disk and rom based libraries.
----------------------------------------------------------------------------*/
void OpenLibs()
{
	if (! (IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library",0L))) {
		dPrintf("Couldn't open intuition.library\n");
		QUIT();
	}
	if (! (GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",36L))) {
		dPrintf("Couldn't open graphics.library\n");
		QUIT();
	}
	if (! (GadToolsBase = (struct GadToolsBase *)OpenLibrary("gadtools.library",0L))) {
		dPrintf("Couldn't open gadtools.library\n");
		QUIT();
	}
	if (! (LayersBase = (struct LayersBase *)OpenLibrary("layers.library",0L))) {
		dPrintf("Couldn't open layers.library\n");
		QUIT();
	}
}


/*----------------------------------------------------------------------------
	CloseLibs()
	Input -
		none
	Function -
		Close all opened libraries.
----------------------------------------------------------------------------*/
void CloseLibs()
{
	if (LayersBase ) {
		CloseLibrary((struct Library *)LayersBase);
	}
	if (GadToolsBase) {
		CloseLibrary((struct Library *)GadToolsBase);
	}
	if (GfxBase) {
		CloseLibrary((struct Library *)GfxBase);
	}
	if (IntuitionBase) {
		CloseLibrary((struct Library *)IntuitionBase);
	}
}


/*----------------------------------------------------------------------------
	dPrintf()
	Input -
		char *s
	Function -
		print a string to stdout and out the serial port
----------------------------------------------------------------------------*/
void dPrintf(char *s)
{
	KPrintF("%s",s);
	printf("%s",s);
}


/*----------------------------------------------------------------------------
	ChooseScreenMode() -
		Input -
		Function -
			Open an ASL screenmode requester to allow user to select screen mode
			for the test.
----------------------------------------------------------------------------*/
void ChooseScreenMode()
{
	if (!(smr = (struct ScreenModeRequester *)AllocAslRequestTags( ASL_ScreenModeRequest,
		ASLSM_TitleText, "Choose Screen Mode",
		TAG_DONE ))) {
		dPrintf("Could not allocate asl request tags\n");
		QUIT();
	}
	if (!(AslRequest(smr,0L))) {
		dPrintf("No screen mode chosen - program aborted.\n");
		FreeAslRequest(smr);
		QUIT();
	}
}


/*----------------------------------------------------------------------------
	KillTestWindow() -
		Input -
			struct Window *wnd
			struct RastPort *rp
			struct BitMap *bm
		Function -
			Generic function that:
				Sets RastPort Write Mask to -1 (default)
				Frees memory allocated for bitmap planes
				Frees memory allocated for bitmap
				Frees memory allocated for rastport
				Closes window
----------------------------------------------------------------------------*/
void KillTestWindow(struct Window *wnd,struct RastPort *rp, struct BitMap *bm)
{
	int i = 0;
	LONG mask = -1;

	if (rp) {
		SetWrMsk(rp, (ULONG)mask);
	}
	if (wnd) {
		if (rp) {
			rp->BitMap = NULL;
			FreeMem(rp,sizeof(struct RastPort));
		}
		if (bm) {
			for (i = 0; i < depth; i++) {
				if (bm->Planes[i]) {
					FreeRaster((PLANEPTR)bm->Planes[i],(ULONG)wnd->Width, (ULONG)wnd->Height);
				}
         }
			FreeMem(bm,sizeof(struct BitMap));
		}
	   CloseWindow(wnd);
	}
}


/*----------------------------------------------------------------------------
	GetChecksum() -
		Input -
			struct Window *win
		Function -
			Calculates the checksum of all pixels in given window
----------------------------------------------------------------------------*/
ULONG GetChecksum(struct Window *win)
{
	int x = 0;
	int y = 0;
	register ULONG checksum = 0;
	register ULONG tmp = 0;
	register ULONG tmp2 = 0;

	for (x = 0; x <= (win->Width); x++) {
		for (y = 0; y <= (win->Height); y++) {
			tmp = checksum << 1;
			tmp2 = (checksum & 0x80000000) >> 31;
/*			KPrintF("tmp = 0x%08lx,tmp2 = 0x%08lx\n",tmp,tmp2); */
			checksum = tmp + tmp2 + (ULONG)ReadPixel(win->RPort,x,y);
		}
	}
	return(checksum);
}


/*----------------------------------------------------------------------------
	PrintScrAttrs() -
		Input -
			struct Screen *scr
		Function -
			Prints out info about given screen structure
----------------------------------------------------------------------------*/
void PrintScrAttrs(struct Screen *scr)
{
	if (scr) {
		printf("\tscr->LeftEdge   %ld\n",scr->LeftEdge);
		KPrintF("\tscr->LeftEdge   %ld\n",scr->LeftEdge);
		printf("\tscr->TopEdge    %ld\n",scr->TopEdge);
		KPrintF("\tscr->TopEdge    %ld\n",scr->TopEdge);
		printf("\tscr->Width      %ld\n",scr->Width);
		KPrintF("\tscr->Width      %ld\n",scr->Width);
		printf("\tscr->Height     %ld\n",scr->Height);
		KPrintF("\tscr->Height     %ld\n",scr->Height);
		printf("\tscr->Flags      %ld\n",scr->Flags);
		KPrintF("\tscr->Flags      %ld\n",scr->Flags);
		printf("\tscr->WBorTop    %ld\n",scr->WBorTop);
		KPrintF("\tscr->WBorTop    %ld\n",scr->WBorTop);
		printf("\tscr->WBorLeft   %ld\n",scr->WBorLeft);
		KPrintF("\tscr->WBorLeft   %ld\n",scr->WBorLeft);
		printf("\tscr->WBorRight  %ld\n",scr->WBorRight);
		KPrintF("\tscr->WBorRight  %ld\n",scr->WBorRight);
		printf("\tscr->WBorBottom %ld\n",scr->WBorBottom );
		KPrintF("\tscr->WBorBottom %ld\n",scr->WBorBottom );
	}
}


/*----------------------------------------------------------------------------
	allocatePixelArray()
		Function -
			Allocates memory for the Pixel Array
----------------------------------------------------------------------------*/
void allocatePixelArray(struct Window *win)
{
	pixelarraywidth = RangeRand(win->Width - 1) + 1;
	pixelarrayheight = RangeRand(win->Height - 1) + 1;
	KPrintF("\tpixelarraywidth = %ld, pixelarrayheight = %ld\n",pixelarraywidth,pixelarrayheight);
	printf("\tpixelarraywidth = %ld, pixelarrayheight = %ld\n",pixelarraywidth,pixelarrayheight);
	pixelarray = (UBYTE *)AllocMem(sizeof(long) * (((pixelarraywidth + 15) >> 4) << 4) * (pixelarrayheight + 1), MEMF_CHIP | MEMF_CLEAR);
	if (! pixelarray) {
		dPrintf("Couldn't allocate memory for pixelarray\n");
		QUIT();
	}

	initPixelArray();
}


/*----------------------------------------------------------------------------
	freePixelArray
		Function - free memory allocated for Pixel Array
----------------------------------------------------------------------------*/
void freePixelArray()
{
	if (pixelarray) {
		FreeMem(pixelarray, sizeof(long) * (((pixelarraywidth + 15) >> 4) << 4) * (pixelarrayheight + 1));
		pixelarray = NULL;
		pixelarraywidth = 0;
		pixelarrayheight = 0;
	}
}


/*----------------------------------------------------------------------------
	initPixelArray()
	Input -
		UBYTE *pixelarray
		int rectwidth
		int rectheight
	Function -
		To initialize or re-initialize the pixel array for WritePixelArray8()
----------------------------------------------------------------------------*/
void initPixelArray()
{
	int i = 0;
	int k = 0;

	numbytes = ((((pixelarraywidth+15)>>4)<<4)*(pixelarrayheight + 1));
	for (i = 0; i < numbytes; i++) {
		k++;
		if (k > 255) {
			k = 0;
		}
		pixelarray[i] = k;
	}
}


/*----------------------------------------------------------------------------
	DoPlot() -
		Input -
			struct Window *win
			struct RastPort *rp
			struct RastPort *tmprp
----------------------------------------------------------------------------*/
void DoPlot(struct Window *win, struct RastPort *rp, struct RastPort *tmprp) {
	int count = 0;
	int x = 0;
	int y = 0;
	int i = 0;
	int numplots = 0;

	allocatePixelArray(win);

	/* Do Random plotting of the pixelarray */

	numplots = 1000;
	for (i = 0; i < numplots; i++) {
		x = RangeRand(win->Width);
		y = RangeRand(win->Height);
		if (((pixelarraywidth + x) < win->Width) && ((pixelarrayheight + y) < win->Height)) {
			count = WritePixelArray8(rp, x, y, x + pixelarraywidth, y + pixelarrayheight, pixelarray, tmprp);
			initPixelArray();
		}
	}

	freePixelArray();
}


/*----------------------------------------------------------------------------
	showTestParams()
		Input -
			int depth
			int layered
			int refresh
			int randomSize
			int maskval
			int wflags
			ULONG Mask
----------------------------------------------------------------------------*/
void showTestParams(int depth,int layered,int refresh,int randomSize,int maskval,ULONG wflags, ULONG Mask)
{
	dPrintf("Screen Attributes:\n");
	PrintScrAttrs(TestScreen);

	KPrintF("%3ld> ", (LONG)depth);
	printf("%3d> ", depth);

	switch (layered)
	{
		case 0:
			dPrintf("Non-layered -- ");
			break;
		case 1:
			dPrintf("Layered -- ");
			break;
		default:
			break;
	}
	switch (refresh)
	{
		case 0:
			dPrintf("Simple Refresh -- ");
			wflags = wflags || WFLG_SIMPLE_REFRESH;
			break;
		case 1:
			dPrintf("Smart Refresh -- ");
			wflags = wflags || WFLG_SMART_REFRESH;
			break;
		case 2:
			dPrintf("SuperBitMap -- ");
			wflags = wflags || WFLG_SUPER_BITMAP;
			break;
		default:
			break;
	}
	KPrintF("Mask value = 0x%lx\n",Mask);
	printf("Mask value = 0x%lx\n",Mask);
}


/*----------------------------------------------------------------------------
	DoTest() -
	Input -
		int depth
		int layered
		int refresh
		int randomSize
		int maskval
	Function -
		Collect all options and perform the test
----------------------------------------------------------------------------*/
void DoTest(int depth,int layered,int refresh,int randomSize,int maskval)
{
	ULONG wflags = 0;
	LONG Mask = 0;
	int i = 0;
	int k = 0;

	KPrintF("Display ID = 0x%08lx\n",(LONG)smr->sm_DisplayID);
	printf("Display ID = 0x%08lx\n",smr->sm_DisplayID);

	/* Open Screen with depth of depth */
	if (!(TestScreen = OpenScreenTags(NULL,
		SA_DisplayID, (smr->sm_DisplayID),
		SA_Depth, depth,
		SA_ShowTitle, TRUE,
		SA_Title, "WritePixelArray8 Test Screen",
  		TAG_END)))
	{
  		dPrintf("Could not open test screen\n");
		QUIT();
	}

	switch (maskval) {
		case 0:
			Mask = RangeRand(255);
			break;
		case 1:
			Mask = -1;
			break;
		default:
			break;
	}

	showTestParams(depth,layered,refresh,randomSize,maskval,wflags,Mask);


	/* Open Windows on the screen */
	for (i = 0; i < NUMWINS; i++) {
		TestWins[i] = OpenWindowTags(NULL,
			WA_Flags, wflags,
			WA_Borderless, FALSE,
			WA_CustomScreen, TestScreen,
			WA_Top, RangeRand(TestScreen->Height),
			WA_Left, RangeRand(TestScreen->Width),
			WA_Width, RangeRand(TestScreen->Width) + 10,
			WA_Height, RangeRand(TestScreen->Height) + 10,
			WA_Title, "TestWindow",
			TAG_END);
		if (! TestWins[i]) {
			KPrintF("Couldn't open test window %ld\n",i);
			printf("Couldn't open test window %ld\n",i);
		}
		else {
			RPs[i] = TestWins[i]->RPort;
			SetWrMsk(RPs[i],Mask);
			TempBMs[i] = (struct BitMap *)AllocMem(sizeof(struct BitMap), MEMF_CHIP || MEMF_CLEAR);
			if	(! TempBMs[i]) {
				dPrintf("Couldn't allocate temporary bitmap\n");
				QUIT();
			}
			InitBitMap(TempBMs[i],depth,(LONG)TestWins[i]->Width,(LONG)TestWins[i]->Height);
			TempRPs[i] = (struct RastPort *)AllocMem(sizeof(struct RastPort), MEMF_CHIP || MEMF_CLEAR);
			if (! TempRPs[i]) {
				QUIT();
			}
			InitRastPort(TempRPs[i]);
			/* copy RPs[i] info to TempRPs[i] */
			TempRPs[i]->AreaPtrn = RPs[i]->AreaPtrn;
			TempRPs[i]->AreaInfo = RPs[i]->AreaInfo;
			TempRPs[i]->GelsInfo = RPs[i]->GelsInfo;
			TempRPs[i]->Mask = RPs[i]->Mask;
			TempRPs[i]->FgPen = RPs[i]->FgPen;
			TempRPs[i]->BgPen = RPs[i]->BgPen;
			TempRPs[i]->AOlPen = RPs[i]->AOlPen;
			TempRPs[i]->DrawMode = RPs[i]->DrawMode;
			TempRPs[i]->AreaPtSz = RPs[i]->AreaPtSz;
			TempRPs[i]->linpatcnt = RPs[i]->linpatcnt;
			TempRPs[i]->Flags = RPs[i]->Flags;
			TempRPs[i]->LinePtrn = RPs[i]->LinePtrn;
			TempRPs[i]->cp_x = RPs[i]->cp_x;
			TempRPs[i]->cp_y = RPs[i]->cp_y;
			TempRPs[i]->PenWidth = RPs[i]->PenWidth;
			TempRPs[i]->PenHeight = RPs[i]->PenHeight;

			TempRPs[i]->BitMap = TempBMs[i];
			TempRPs[i]->Layer = NULL;
			TempRPs[i]->BitMap->BytesPerRow = numbytes;
			for (k = 0; k < depth; k++) {
				TempBMs[i]->Planes[k] = NULL;
			}
			for (k = 0; k < depth; k++) {
				TempBMs[i]->Planes[k] = (PLANEPTR)AllocRaster((ULONG)TestWins[i]->Width, (ULONG)TestWins[i]->Height);
				if (!TempBMs[i]->Planes[k]) {
					KPrintF("Couldn't allocate plane for TempBMs[%ld]\n",i);
					printf("Couldn't allocate plane for TempBMs[%ld]\n",i);
					QUIT();
				}
			}
		}
	}




	/**************** Do plotting *******************/
	for (i = 0; i < NUMWINS; i++) {
		if (TestWins[i]) {
			DoPlot(TestWins[i], RPs[i], TempRPs[i]);
			PixCheckSum = GetChecksum(TestWins[i]);
			KPrintF("\tChecksum for Window %ld = 0x%08lx\n",i,PixCheckSum);
			printf("\tChecksum for Window %ld = 0x%08lx\n",i,PixCheckSum);
			PixCheckSum = 0;
		}
	}



	/* Close all of the open windows and the screen */
	dPrintf("Closing Windows...\n");
	Delay(100);
	for (i = 0; i < NUMWINS; i++) {
		if (TestWins[i]) {
			KillTestWindow(TestWins[i], TempRPs[i], TempBMs[i]);
			TestWins[i] = NULL;
			TempRPs[i] = NULL;
			TempBMs[i] = NULL;
			RPs[i] = NULL;
		}
	}

	if (TestScreen) {
	  	CloseScreen(TestScreen);
		TestScreen = NULL;
   }
}



/*----------------------------------------------------------------------------
	QUIT() -
	Input -
		none
	Function -
		Deallocate screen mode requester
		Close Screen
		Close Libraries
----------------------------------------------------------------------------*/
void QUIT()
{
	int i = 0;

	if (smr) {
		FreeAslRequest(smr);
	}

	for ( i = 0; i < NUMWINS; i++) {
		if (TestWins[i]) {
			KillTestWindow(TestWins[i], TempRPs[i], TempBMs[i]);
			TestWins[i] = NULL;
			TempRPs[i] = NULL;
			TempBMs[i] = NULL;
			RPs[i] = NULL;
		}
	}

	if (pixelarray) {
		freePixelArray();
	}

	if (TestScreen) {
	  	CloseScreen(TestScreen);
		TestScreen = NULL;
   }
	CloseLibs();

   exit(0);
}


/*----------------------------------------------------------------------------
	ComputeX -
	Input -
		UWORD value
	Function -
		Calculate font sensitive X value
----------------------------------------------------------------------------*/
static UWORD ComputeX( UWORD value )
{
    return(( UWORD )(( FontX * value ) / 8 ));
}


/*----------------------------------------------------------------------------
	ComputeY -
	Input -
		UWORD value
	Function -
		Calculate font sensitive Y value
----------------------------------------------------------------------------*/
static UWORD ComputeY( UWORD value )
{
    return(( UWORD )(( FontY * value ) / 8 ));
}


/*----------------------------------------------------------------------------
	ComputeFont()
	Input -
		UWORD width
		UWORD height
	Function -
		Select font for display
----------------------------------------------------------------------------*/
static void ComputeFont( UWORD width, UWORD height )
{
    Font = &Attr;
    Font->ta_Name = GfxBase->DefaultFont->tf_Message.mn_Node.ln_Name;
    Font->ta_YSize = FontY = GfxBase->DefaultFont->tf_YSize;
    FontX = GfxBase->DefaultFont->tf_XSize;

    OffY = Scr->Font->ta_YSize + Scr->WBorTop + 1;
    OffX = Scr->WBorLeft;

    if ( width && height ) {
        if (( ComputeX( width ) + OffX + Scr->WBorRight ) > Scr->Width )
            goto UseTopaz;
        if (( ComputeY( height ) + OffY + Scr->WBorBottom ) > Scr->Height )
            goto UseTopaz;
    }
    return;

UseTopaz:
    Font->ta_Name = (STRPTR)"topaz.font";
    FontX = FontY = Font->ta_YSize = 8;
}


/*----------------------------------------------------------------------------
	SetupScreen() -
	Input -
		none
	Function -
		Lock the workbench screen in order to open initial options window
		Choose display font
----------------------------------------------------------------------------*/
int SetupScreen( void )
{
	if ( ! ( Scr = LockPubScreen((UBYTE * )"Workbench" ))) {
   	return( 1L );
	}

	ComputeFont( 0L, 0L );

	if ( ! ( VisualInfo = GetVisualInfo( Scr, TAG_DONE ))) {
		return( 2L );
	}
	return( 0L );
}


/*----------------------------------------------------------------------------
	CloseDownScreen() -
		Input -
			none
		Function -
			Unlock the workbench screen
----------------------------------------------------------------------------*/
void CloseDownScreen( void )
{
	if ( VisualInfo ) {
		FreeVisualInfo( VisualInfo );
		VisualInfo = NULL;
	}

	if ( Scr        ) {
		UnlockPubScreen( NULL, Scr );
		Scr = NULL;
	}
}


/*----------------------------------------------------------------------------
	OpenWPWindow()
		Input -
			none
		Function -
			Setup up gadtools gadgets and open the initial test options window
----------------------------------------------------------------------------*/
int OpenWPWindow( void )
{
    struct NewGadget     ng;
    struct Gadget       *g;
    UWORD               wleft = WPLeft, wtop = WPTop, ww, wh;

    ComputeFont( WPWidth, WPHeight );

    ww = ComputeX( WPWidth );
    wh = ComputeY( WPHeight );

    if (( wleft + ww + OffX + Scr->WBorRight ) > Scr->Width ) wleft = Scr->Width - ww;
    if (( wtop + wh + OffY + Scr->WBorBottom ) > Scr->Height ) wtop = Scr->Height - wh;

    if ( ! ( g = CreateContext( &WPGList )))
        return( 1L );

    ng.ng_LeftEdge        =    OffX + ComputeX( 4 );
    ng.ng_TopEdge         =    OffY + ComputeY( 5 );
    ng.ng_Width           =    ComputeX( 45 );
    ng.ng_Height          =    ComputeY( 18 );
    ng.ng_GadgetText      =    NULL;
    ng.ng_TextAttr        =    Font;
    ng.ng_GadgetID        =    GD_Depth;
    ng.ng_Flags           =    0;
    ng.ng_VisualInfo      =    VisualInfo;

    g = CreateGadget( TEXT_KIND, g, &ng, GTTX_Text, "Depth:", TAG_DONE );

    WPGadgets[ 0 ] = g;

    ng.ng_LeftEdge        =    OffX + ComputeX( 63 );
    ng.ng_TopEdge         =    OffY + ComputeY( 6 );
    ng.ng_GadgetText      =    (UBYTE *)"1";
    ng.ng_GadgetID        =    GD_depth1;
    ng.ng_Flags           =    PLACETEXT_BELOW;

    g = CreateGadget( CHECKBOX_KIND, g, &ng, GTCB_Checked, TRUE, TAG_DONE );

    WPGadgets[ 1 ] = g;

    ng.ng_LeftEdge        =    OffX + ComputeX( 90 );
    ng.ng_GadgetText      =    (UBYTE *)"2";
    ng.ng_GadgetID        =    GD_depth2;

    g = CreateGadget( CHECKBOX_KIND, g, &ng, GTCB_Checked, TRUE, TAG_DONE );

    WPGadgets[ 2 ] = g;

    ng.ng_LeftEdge        =    OffX + ComputeX( 117 );
    ng.ng_GadgetText      =    (UBYTE *)"3";
    ng.ng_GadgetID        =    GD_depth3;

    g = CreateGadget( CHECKBOX_KIND, g, &ng, GTCB_Checked, TRUE, TAG_DONE );

    WPGadgets[ 3 ] = g;

    ng.ng_LeftEdge        =    OffX + ComputeX( 144 );
    ng.ng_GadgetText      =    (UBYTE *)"4";
    ng.ng_GadgetID        =    GD_depth4;

    g = CreateGadget( CHECKBOX_KIND, g, &ng, GTCB_Checked, TRUE, TAG_DONE );

    WPGadgets[ 4 ] = g;

    ng.ng_LeftEdge        =    OffX + ComputeX( 171 );
    ng.ng_GadgetText      =    (UBYTE *)"5";
    ng.ng_GadgetID        =    GD_depth5;

    g = CreateGadget( CHECKBOX_KIND, g, &ng, GTCB_Checked, TRUE, TAG_DONE );

    WPGadgets[ 5 ] = g;

    ng.ng_LeftEdge        =    OffX + ComputeX( 198 );
    ng.ng_GadgetText      =    (UBYTE *)"6";
    ng.ng_GadgetID        =    GD_depth6;

    g = CreateGadget( CHECKBOX_KIND, g, &ng, GTCB_Checked, TRUE, TAG_DONE );

    WPGadgets[ 6 ] = g;

    ng.ng_LeftEdge        =    OffX + ComputeX( 225 );
    ng.ng_GadgetText      =    (UBYTE *)"7";
    ng.ng_GadgetID        =    GD_depth7;

    g = CreateGadget( CHECKBOX_KIND, g, &ng, GTCB_Checked, TRUE, TAG_DONE );

    WPGadgets[ 7 ] = g;

    ng.ng_LeftEdge        =    OffX + ComputeX( 252 );
    ng.ng_GadgetText      =    (UBYTE *)"8";
    ng.ng_GadgetID        =    GD_depth8;

    g = CreateGadget( CHECKBOX_KIND, g, &ng, GTCB_Checked, TRUE, TAG_DONE );

    WPGadgets[ 8 ] = g;

    ng.ng_LeftEdge        =    OffX + ComputeX( 79 );
    ng.ng_TopEdge         =    OffY + ComputeY( 39 );
    ng.ng_GadgetText      =    (UBYTE *)"Layered";
    ng.ng_GadgetID        =    GD_layered;
    ng.ng_Flags           =    PLACETEXT_RIGHT;

    g = CreateGadget( CHECKBOX_KIND, g, &ng, GTCB_Checked, TRUE, TAG_DONE );

    WPGadgets[ 9 ] = g;

    ng.ng_TopEdge         =    OffY + ComputeY( 51 );
    ng.ng_GadgetText      =    (UBYTE *)"Non-Layered";
    ng.ng_GadgetID        =    GD_nonlayered;

    g = CreateGadget( CHECKBOX_KIND, g, &ng, GTCB_Checked, TRUE, TAG_DONE );

    WPGadgets[ 10 ] = g;

    ng.ng_TopEdge         =    OffY + ComputeY( 74 );
    ng.ng_GadgetText      =    (UBYTE *)"Simple Refresh";
    ng.ng_GadgetID        =    GD_simple;

    g = CreateGadget( CHECKBOX_KIND, g, &ng, GTCB_Checked, TRUE, TAG_DONE );

    WPGadgets[ 11 ] = g;

    ng.ng_TopEdge         =    OffY + ComputeY( 86 );
    ng.ng_GadgetText      =    (UBYTE *)"Smart Refresh";
    ng.ng_GadgetID        =    GD_smart;

    g = CreateGadget( CHECKBOX_KIND, g, &ng, GTCB_Checked, TRUE, TAG_DONE );

    WPGadgets[ 12 ] = g;

    ng.ng_TopEdge         =    OffY + ComputeY( 98 );
    ng.ng_GadgetText      =    (UBYTE *)"Super Refresh";
    ng.ng_GadgetID        =    GD_super;

    g = CreateGadget( CHECKBOX_KIND, g, &ng, GTCB_Checked, TRUE, TAG_DONE );

    WPGadgets[ 13 ] = g;

    ng.ng_LeftEdge        =    OffX + ComputeX( 5 );
    ng.ng_TopEdge         =    OffY + ComputeY( 35 );
    ng.ng_Width           =    ComputeX( 45 );
    ng.ng_Height          =    ComputeY( 18 );
    ng.ng_GadgetText      =    NULL;
    ng.ng_GadgetID        =    GD_lay;
    ng.ng_Flags           =    0;

    g = CreateGadget( TEXT_KIND, g, &ng, GTTX_Text, "Layered:", TAG_DONE );

    WPGadgets[ 14 ] = g;

    ng.ng_LeftEdge        =    OffX + ComputeX( 4 );
    ng.ng_TopEdge         =    OffY + ComputeY( 71 );
    ng.ng_GadgetID        =    GD_refresh;

    g = CreateGadget( TEXT_KIND, g, &ng, GTTX_Text, "Refresh:", TAG_DONE );

    WPGadgets[ 15 ] = g;

    ng.ng_TopEdge         =    OffY + ComputeY( 115 );
    ng.ng_GadgetID        =    GD_mask;

    g = CreateGadget( TEXT_KIND, g, &ng, GTTX_Text, "Mask:", TAG_DONE );

    WPGadgets[ 16 ] = g;

    ng.ng_LeftEdge        =    OffX + ComputeX( 79 );
    ng.ng_TopEdge         =    OffY + ComputeY( 120 );
    ng.ng_GadgetText      =    (UBYTE *)"Random Masks";
    ng.ng_GadgetID        =    GD_ranmask;
    ng.ng_Flags           =    PLACETEXT_RIGHT;

    g = CreateGadget( CHECKBOX_KIND, g, &ng, GTCB_Checked, TRUE, TAG_DONE );

    WPGadgets[ 17 ] = g;

    ng.ng_TopEdge         =    OffY + ComputeY( 132 );
    ng.ng_GadgetText      =    (UBYTE *)"-1 Mask";
    ng.ng_GadgetID        =    GD_negmask;

    g = CreateGadget( CHECKBOX_KIND, g, &ng, GTCB_Checked, TRUE, TAG_DONE );

/*    WPGadgets[ 18 ] = g;

    ng.ng_LeftEdge        =    OffX + ComputeX( 240 );
    ng.ng_TopEdge         =    OffY + ComputeY( 119 );
    ng.ng_Width           =    ComputeX( 38 );
    ng.ng_Height          =    ComputeY( 13 );
    ng.ng_GadgetText      =    (UBYTE *)"#";
    ng.ng_GadgetID        =    GD_nummasks;
    ng.ng_Flags           =    PLACETEXT_LEFT;

    g = CreateGadget( STRING_KIND, g, &ng, GTST_String, "3", GTST_MaxChars, 3, TAG_DONE ); */

    WPGadgets[ 18 ] = g;

    if ( ! g )
        return( 2L );

    if ( ! ( WPWnd = OpenWindowTags( NULL,
                    WA_Left,          wleft,
                    WA_Top,           wtop,
                    WA_Width,         ww + OffX + Scr->WBorRight,
                    WA_Height,        wh + OffY + Scr->WBorBottom,
                    WA_IDCMP,         TEXTIDCMP|CHECKBOXIDCMP|STRINGIDCMP|IDCMP_CLOSEWINDOW|IDCMP_REFRESHWINDOW,
                    WA_Flags,         WFLG_DRAGBAR|WFLG_DEPTHGADGET|WFLG_CLOSEGADGET|WFLG_SMART_REFRESH|WFLG_ACTIVATE,
                    WA_Gadgets,       WPGList,
                    WA_Title,         WPWdt,
                    WA_ScreenTitle,   "WPixel",
                    TAG_DONE )))
        return( 4L );

    GT_RefreshWindow( WPWnd, NULL );

    return( 0L );
}


/*----------------------------------------------------------------------------
	handleWPIDCMP()
		Input -
			none
		Function -
			Handle the IDCMP event loop for the initial options window
----------------------------------------------------------------------------*/
void handleWPIDCMP(void)
{
	struct IntuiMessage *imsg;
	struct Gadget *gad;
	int done = FALSE;

	while (!done)
	{
		Wait(1 << WPWnd->UserPort->mp_SigBit);
		while ((!done) && (imsg =
		GT_GetIMsg(WPWnd->UserPort)))
		{
			if (imsg != NULL)
			{
				GT_ReplyIMsg(imsg);
				switch(imsg->Class)
				{
					case IDCMP_CLOSEWINDOW:
						done = TRUE;
						break;
					case IDCMP_GADGETUP:
						gad = (struct Gadget *)imsg->IAddress;
						switch(gad->GadgetID)
						{
							case GD_depth1:
								deptharray[0] = (deptharray[0] == FALSE) ? TRUE : FALSE;
								break;
							case GD_depth2:
								deptharray[1] = (deptharray[1] == FALSE) ? TRUE : FALSE;
								break;
							case GD_depth3:
								deptharray[2] = (deptharray[2] == FALSE) ? TRUE : FALSE;
								break;
							case GD_depth4:
								deptharray[3] = (deptharray[3] == FALSE) ? TRUE : FALSE;
								break;
							case GD_depth5:
								deptharray[4] = (deptharray[4] == FALSE) ? TRUE : FALSE;
								break;
							case GD_depth6:
								deptharray[5] = (deptharray[5] == FALSE) ? TRUE : FALSE;
								break;
							case GD_depth7:
								deptharray[6] = (deptharray[6] == FALSE) ? TRUE : FALSE;
								break;
							case GD_depth8:
								deptharray[7] = (deptharray[7] == FALSE) ? TRUE : FALSE;
								break;
							case GD_layered:
								layerarray[0] = (layerarray[0] == FALSE) ? TRUE : FALSE;
								break;
							case GD_nonlayered:
								layerarray[1] = (layerarray[1] == FALSE) ? TRUE : FALSE;
								break;
							case GD_simple:
								refresharray[0] = (refresharray[0] == FALSE) ?  TRUE : FALSE;
								break;
							case GD_smart:
								refresharray[1] = (refresharray[1] == FALSE) ?  TRUE : FALSE;
								break;
							case GD_super:
								refresharray[2] = (refresharray[2] == FALSE) ?  TRUE : FALSE;
								break;
							case GD_ranmask:
								maskarray[0] = (maskarray[0] == FALSE) ? TRUE : FALSE;
								break;
							case GD_negmask:
								maskarray[1] = (maskarray[1] == FALSE) ? TRUE : FALSE;
								break;
/*							case GD_nummasks:
								break; */
							default:
								break;
						}
   					break;
					case IDCMP_REFRESHWINDOW:
						GT_BeginRefresh(WPWnd);
						GT_EndRefresh(WPWnd,TRUE);
						break;
					default:
						break;
				}
			}
		}
	}
}


/*----------------------------------------------------------------------------
	CloseWPWindow()
	Input -
		none
	Function -
		Close the initial options window and free its gadgets
----------------------------------------------------------------------------*/
void CloseWPWindow( void )
{
	if ( WPWnd        ) {
		CloseWindow( WPWnd );
		WPWnd = NULL;
	}

	if ( WPGList      ) {
		FreeGadgets( WPGList );
		WPGList = NULL;
	}
}


/*----------------------------------------------------------------------------
	showSelections()
	Input -
		none
	Function -
		Display user choices for test parameters
----------------------------------------------------------------------------*/
void showSelections()
{
	int i = 0;

	/* print out results of user selections */

	dPrintf("\tdepths selected = ");
	for (i = 0; i < 8; i++) {
		if (deptharray[i]) {
			KPrintF("%ld ",i + 1);
			printf("%ld ",i + 1);
		}
	}

	dPrintf("\n\tLayer types selected = ");
	if (layerarray[0]) {
		dPrintf("Layered ");
	}
	if (layerarray[1]) {
		dPrintf("Non-layered ");
	}

	dPrintf("\n\tMasks selected = ");
	if (maskarray[0]) {
		dPrintf("Random ");
	}
	if (maskarray[1]) {
		dPrintf("-1 ");
	}

	dPrintf("\n\tRefresh modes selected = ");
	for (i = 0; i < 3; i++) {
		if (refresharray[i]) {
			switch(i) {
				case 0:
					dPrintf("Simple ");
					break;
				case 1:
					dPrintf("Smart ");
					break;
				case 2:
					dPrintf("Super ");
					break;
				default:
					break;
			}
		}
	}
	dPrintf("\n");
}


/*----------------------------------------------------------------------------
	DoUserSelect()
	Input -
		none
	Function -
		Open the initial options window, get the options, and then display them.
----------------------------------------------------------------------------*/
void DoUserSelect(void)
{
	SetupScreen();
	OpenWPWindow();
	handleWPIDCMP();
	CloseWPWindow();
	CloseDownScreen();
}


/*----------------------------------------------------------------------------
	DoAll()
	Input -
		none
	Function -
		Setup up test options so that all choices are picked.
		Display the options chosen.
----------------------------------------------------------------------------*/
void DoAll(void)
{
	int i = 0;

	for (i = 0; i < 8; i++) {
		deptharray[i] = TRUE;
	}
	for (i = 0; i < 2; i++) {
		layerarray[i] = TRUE;
	}
	for (i = 0; i < 3; i++) {
		refresharray[i] = TRUE;
	}
	for (i = 0; i < 2; i++) {
		maskarray[i] = TRUE;
	}
}


/*----------------------------------------------------------------------------
==============================================================================
	main()
		Input -
			int argc
			char **argv
		Function -
			main entry point
==============================================================================
----------------------------------------------------------------------------*/
void main(int argc, char **argv)
{
	int layered = 0;
	int refresh = 0;
	int randomSize = 0;
	int maskval = 0;

	OpenLibs();


	dPrintf("\n *** wpixel v2.1 - WritePixelArray8 test ***\n");

	if (argc >= 2) {
		if (0 == strcmp(argv[1],"all")) {
			DoAll();
		}
	}
	else {
		DoUserSelect();
	}

	showSelections();
	ChooseScreenMode();

	/* The 'tie up the machine' test */
	for (depth = 1; depth <= 8; depth++) {
		for (layered = 0; layered < 2; layered++) {
			for (refresh = 0; refresh < NUMREFRESH; refresh++) {
				for (maskval = 0; maskval < 2; maskval++) {
					if ((deptharray[depth - 1]) && (layerarray[layered]) && (refresharray[refresh]) && (maskarray[maskval])) {
						NumTests++;
						DoTest(depth, layered, refresh, randomSize, maskval);
					}
				}
			}
		}
	}

	KPrintF("total tests = %ld\n", (LONG)NumTests);
	printf("total tests = %ld\n", (LONG)NumTests);

	QUIT();
}