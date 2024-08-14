#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <hardware/custom.h>
#include <graphics/copper.h>
#include <graphics/gfxmacros.h>
#include <graphics/text.h>
#include <libraries/asl.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <clib/alib_protos.h>
#include <clib/alib_stdio_protos.h>
#include <clib/asl_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

extern struct Custom __far custom;

struct	IntuitionBase   *IntuitionBase = NULL;
struct	GfxBase         *GfxBase = NULL;
struct	AslBase			 *AslBase = NULL;

struct	BitMap   *bmap3 = NULL;
struct	UCopList *ucl[2] = {0};
struct	ViewPort *vplo = NULL;
extern	UWORD    __far RangeSeed;
struct	Screen   *screenlo = NULL;
struct	Window   *windowlo = NULL;
struct	Window   *wbak = NULL;
struct	ScreenModeRequester *scrmodereq = NULL;
int		swap;    /* alternatively go PF2 above PF1 and vice versa */
struct    RastPort *srplo = NULL, *rportbak = NULL;
struct    RasInfo *rinfo2 = NULL;
struct    BitMap  *bmap2 = NULL;
struct    RastPort *rport2 = NULL;
UBYTE     *initialptr[3] = {0};
int       exitval = 0, it_is_done = 0;
UWORD     *getaword = NULL, randval = 0;
struct	TextAttr myfont1 = { (UBYTE *)"topaz.font", 8, 0, 0 };

#define BACKGRND 0
#define BLACK    0

UWORD titletable[] = {

	BACKGRND, 0x0fff,0xD32,0xE60,0xFE5,0x0F7,0x08F,0xB6F,
	/* black,white,red,orange,yellow,brtgrn,blue,purple */
	BACKGRND, 0x0fff,0xD32,0xE60,0xFE5,0x0F7,0x08F,0xB6F,
	BACKGRND, 0x0fff,0xD32,0xE60,0xFE5,0x0F7,0x08F,0xB6F,
	BACKGRND, 0x0fff,0xD32,0xE60,0xFE5,0x0F7,0x08F,0xB6F
};

struct NewScreen nslo = {
	0, 0,320, 200,3,1, 0,SPRITES,SCREENQUIET | CUSTOMSCREEN,&myfont1,
	(UBYTE *)"",NULL,NULL
};

struct NewWindow nw = {
	0, 0, 160, 100, 0, 1, CLOSEWINDOW, WINDOWCLOSE|WINDOWDRAG,
	NULL, NULL, (UBYTE *)"DragMe/CloseMe", NULL, NULL, 5, 5, -1, -1,
	CUSTOMSCREEN
};

struct NewWindow nwbak = {
	0, 0, 320, 200, 0, 1, 0,
	BACKDROP | BORDERLESS,
	NULL, NULL, (UBYTE *)"", NULL, NULL, 5, 5, -1, -1,
	CUSTOMSCREEN
};

void OpenLibs(void);
void CloseLibs(void);
void QUIT(void);
void doScreenModeReq(void);
void openDisplay(void);
void swapPlayfields(void);

void OpenLibs()
{
	if (! (IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library",0L))) {
		printf("Couldn't open intuition.library\n");
		QUIT();
	}
	if (! (GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0L))) {
		printf("Couldn't open graphics.library\n");
		QUIT();
	}
	if (! (AslBase = (struct AslBase *)OpenLibrary("asl.library",0L))) {
		printf("Couldn't open asl.library\n");
		QUIT();
	}
}

void CloseLibs()
{
	if (AslBase) {
		CloseLibrary((struct Library *)AslBase);
	}
	if (GfxBase) {
		CloseLibrary((struct Library *)GfxBase);
	}
	if (IntuitionBase) {
		CloseLibrary((struct Library *)IntuitionBase);
	}
}

void QUIT()
{
	int i = 0;

	if(ucl[0]) {
		vplo->UCopIns = NULL;
		FreeCopList( ucl[0]->FirstCopList );
		FreeMem( ucl[0], (LONG) sizeof (struct UCopList) );
   }
	if(ucl[1]) {
		vplo->UCopIns = NULL;
		FreeCopList( ucl[1]->FirstCopList );
		FreeMem( ucl[1], (LONG) sizeof (struct UCopList) );
	}

    /* clean up dual-playfield trick	*/

	if (it_is_done) {
		Forbid();
		screenlo->ViewPort.RasInfo->Next = NULL;
		screenlo->ViewPort.Modes &= ~DUALPF;
		Permit();
		MakeScreen(screenlo);
		RethinkDisplay();
	}

	if (rport2)
		FreeMem(rport2, (LONG) sizeof (struct RastPort));
	if (bmap2) {
		for(i=0; i<3; i++) {
			if (bmap2->Planes[i]) {
				FreeRaster(bmap2->Planes[i], (LONG) screenlo->Width, (LONG) (screenlo->Height<<1));
				/* If second playfield same size as first one, then
				* this would have been (screenlo->Height) instead
				*/
			}
		}
		FreeMem(bmap2, (LONG) sizeof (struct BitMap));
	}
	if (bmap3)
	{
		FreeMem(bmap3, (LONG) sizeof (struct BitMap));
	}
	if (rinfo2) FreeMem(rinfo2, (LONG) sizeof (struct RasInfo));

	if (windowlo) {
		CloseWindow(windowlo);
	}
	if (wbak) {
		CloseWindow(wbak);
	}
	if (screenlo) {
		CloseScreen(screenlo);
	}

	CloseLibs();
	exit(0);
}

void doScreenModeReq()
{
}

void openDisplay()
{
	int i = 0;

	doScreenModeReq();
	screenlo = OpenScreen(&nslo);
	if(screenlo == NULL) {
		exitval = 3;
		QUIT();
	}

	vplo = &screenlo->ViewPort;
	srplo = &screenlo->RastPort;
	LoadRGB4(vplo, titletable, (LONG)32);

	ScreenToFront(screenlo);

	SetRast(srplo, BLACK);
	SetDrMd(srplo, JAM1);

	nw.Screen = screenlo;
	nwbak.Screen = screenlo;

	windowlo = OpenWindow(&nw);
	if (windowlo == NULL) {
		exitval = 200;
		QUIT();
	}

	wbak = OpenWindow(&nwbak);
	if (wbak == NULL) {
		exitval = 201;
		QUIT();
	}

	rportbak = wbak->RPort;

	/* ------ Prepare to Add a second playfield ----------- */
	/*        This code courtesy of Jim Mackraz             */

	/* allocate second playfield's rasinfo, bitmap, and bitplane */
	if (!(rinfo2 = (struct RasInfo *) AllocMem(sizeof(struct RasInfo), MEMF_PUBLIC|MEMF_CLEAR)))  {
		exitval = 5;
		QUIT();
	}
	if (!(bmap3 = (struct BitMap *)AllocMem(sizeof(struct BitMap), MEMF_PUBLIC|MEMF_CLEAR))) {
		exitval = 6;
		QUIT();
	}

	InitBitMap(bmap3, 3L, 320, 200);

	if (!(bmap2 = (struct BitMap *)AllocMem(sizeof(struct BitMap),MEMF_PUBLIC|MEMF_CLEAR))) {
		exitval = 7;
		QUIT();
	}

	/* This example came from the demo "Only Amiga" for which the
	* second playfield is twice as tall as the first playfield.
	* Intuition and the system are fooled into thinking, for display
	* purposes, that only a 200 line playfield is available, nicely
	* matching the playfield area that Intuition is using.  However,
	* for drawing purposes, the drawing area is twice as high so that
	* we can scroll new information into the display area just by
	* later changing the display pointers and remaking the display list.
	* The last parameter for InitBitMap would have been 200
	*/

	InitBitMap(bmap2, 3L, 320, 400);

	/* we'll use 3 planes. */
	for(i=0; i<3; i++)  {
		if (!(bmap2->Planes[i] = (UBYTE *) AllocRaster( screenlo->Width, (screenlo->Height<<1)))) {
			exitval = 8;
			QUIT();
		}
	}

	/* If second playfield same size as first one, then
	* this would have been (screenlo->Height) instead
	*/

	for(i=0; i<3; i++) {
		initialptr[i] = (UBYTE *)bmap2->Planes[i];
		bmap3->Planes[i] = (PLANEPTR)initialptr[i];
	}

	/* get a rastport, and set it up for rendering into bmap2	*/

	if (!(rport2 = (struct RastPort *)AllocMem((LONG) sizeof (struct RastPort), (LONG) MEMF_PUBLIC))) {
		exitval = 9;
		QUIT();
	}

	InitRastPort(rport2);
	rport2->BitMap = bmap2;

	SetRast(rport2, 0L);
	SetOPen(rportbak, 1L);
	for(i=2; i<8; i++) {
		SetAPen(rport2, i);
		RectFill(rport2, 30*i, 20, 25+(30*i), 180);
		SetAPen(rportbak, i);
		RectFill(rportbak, 20, 20*i, 300, 15+(20*i));
	}
}

void swapPlayfields()
{
	struct    IntuiMessage    *msg = NULL;
	ULONG     class;

	/* manhandle viewport: install second playfield and change modes	*/
	Forbid();
	rinfo2->BitMap = bmap3;	/* install my bitmap in my rasinfo	*/
	screenlo->ViewPort.RasInfo->Next = rinfo2;

	/* install rinfo for viewport's second playfield	*/
	screenlo->ViewPort.Modes |= DUALPF;

	/* convert viewport			*/
	it_is_done = 1;
	Permit();
	MakeScreen(screenlo);
	RethinkDisplay();

	for(;;) {
		if ((msg = (struct IntuiMessage *)GetMsg(windowlo->UserPort)) == NULL) {
			vplo->UCopIns = ucl[swap^=1];
			/* put "the other" playfield in front */
			MakeScreen(screenlo);
			RethinkDisplay();
			Delay(25);
		}
		else {
			class   = msg->Class;
			ReplyMsg((struct Message *)msg);
			switch (class) {
				case CLOSEWINDOW:
					QUIT();
					break;
				default:
					break;
			}
		}
	}
}

void main()
{
	int i = 0;
	ucl[0] = NULL;
	ucl[1] = NULL;
	swap         = 0;
	it_is_done   = 0;
/*	randval      = 0x5555;
	getaword     = (UWORD *)0;

	for(i=0; i<1024; i++) {
		randval = randval ^ (*getaword++);
	}
	RangeSeed = randval; */
	exitval   = 0;

	OpenLibs();

	openDisplay();

/* ================================================= */
	if ( !(ucl[0] = (struct UCopList *)AllocMem(sizeof(struct UCopList), MEMF_CHIP | MEMF_CLEAR) )) {
		QUIT();
	}

	CINIT( ucl[0], 400L );
	CWAIT( ucl[0], 1L, 5L);
	CMOVE( ucl[0], custom.bplcon2,  (long)0x64);
	CEND ( ucl[0] );

	if ( !(ucl[1] = (struct UCopList *)AllocMem( sizeof (struct UCopList), MEMF_CHIP | MEMF_CLEAR) )) {
		QUIT();
	}

	CINIT( ucl[1], 400L );
	CWAIT( ucl[1], 1L, 5L);
	CMOVE( ucl[1], custom.bplcon2,  (long)0x24);
	CEND ( ucl[1] );

	vplo = &screenlo->ViewPort;
	vplo->UCopIns = ucl[0];
/* ================================================= */

	swapPlayfields();
	QUIT();
}