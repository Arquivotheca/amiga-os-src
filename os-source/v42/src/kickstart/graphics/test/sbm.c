/**********************************************************************/
/*                                                                    */
/*              Program : to test superbitmap bug B5178               */
/*                                                                    */
/*                      Written By : S.A.Shanson                      */
/*                     Date Started : Dec 14 1990                     */
/*              Last Modified : Fri Dec 14 14:29:33 1990              */
/*                            Language : C                            */
/*                                                                    */
/*  SourceComp : Commodore Amiga                                      */
/*  SourceFile : work:src/test/sbm/sbm.c                              */
/*                                                                    */
/*  Called By : CLI/WB                                                */
/*                                                                    */
/**********************************************************************/

/*******************************************************************/
/*        TO USE ONE OF THE FOLLOWING LIBRARIES, SET ITS FLAG TO 1 */
/*                            ELSE SET IT TO 0                     */
/*******************************************************************/

#define USE_EXEC 1
#define USE_DOS 0
#define USE_INTUITION 1
#define USE_GRAPHICS 1
#define USE_LAYERS 1
#define USE_ARP 0
#define USE_ICON 0
#define USE_DISKFONT 0

/* remove this line for working version, using OpenWindow() */
/* #define FUCKED*/


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
#endif
#if USE_DOS == 1
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#endif
#if USE_INTUITION == 1
#include <intuition/screens.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#endif
#if USE_GRAPHICS == 1
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#endif
#if USE_LAYERS == 1
#include <graphics/clip.h>
#include <graphics/layers.h>
#endif
#if USE_ARP == 1
#include <arp/arpbase.h>
#include <arp/arpfunc.h>
#endif
#if USE_ICON == 1
#include <workbench/icon.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>
#endif
#if USE_DISKFONT == 1
#include <libraries/diskfont.h>
#endif

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
struct DiskFontBase *DiskFontBase = NULL ;
#endif

#define DEPTH 2
#define WIDTH 1024
#define HEIGHT 1024	
#define TMPRASSIZE (WIDTH/8)*HEIGHT
#define MAXVECTORS 4

struct Window *w = NULL;
struct BitMap bm;
BYTE *raster = NULL;
struct TmpRas tr;
BYTE *tmpraster = NULL;
struct AreaInfo ai;
WORD aibuff[MAXVECTORS * 5];

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

/*	if (_Backstdout) {
		Forbid () ;
		Write (_Backstdout, String, strlen (String)) ;
		Permit () ;
	}*/

	CloseAll () ;
	exit(0) ;
}

struct NewWindow nw = 
{
	100, 100,
	200, 100,
	-1, -1,
	IDCMP_RAWKEY|IDCMP_CLOSEWINDOW,
	WFLG_SIZEGADGET|WFLG_DRAGBAR|WFLG_CLOSEGADGET|WFLG_SUPER_BITMAP|WFLG_GIMMEZEROZERO,
	NULL,	
	NULL,
	NULL,
	NULL,
	&bm,
	0, 0,
	WIDTH, HEIGHT,
	WBENCHSCREEN
};

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
	if ((IntuitionBase = (struct IntuitionBase *)OpenLibrary ("intuition.library", 0L)) == NULL)
		Error ("Could not open the Intuition.library") ;
	#endif

	#if USE_LAYERS == 1
	/* Open the layers library.... */
	if ((LayersBase = OpenLibrary ("layers.library", 0L)) == NULL)
		Error ("Could not open the layers.library") ;
	#endif

	#if USE_GRAPHICS == 1
	/* Open the graphics library.... */
	if ((GfxBase = (struct GfxBase *)OpenLibrary ("graphics.library", 0L)) == NULL)
		Error ("Could not open the Graphics.library") ;
	#endif

	#if USE_ARP == 1
	/* Open the arp library.... */
	if ((ArpBase = (struct ArpBase *)OpenLibrary ("arp.library", 0L)) == NULL)
		Error ("Could not open the arp.library") ;
	#endif

	#if USE_ICON == 1
	/* Open the icon library.... */
	if ((IconBase = OpenLibrary ("icon.library", 0L)) == NULL)
		Error ("Could not open the icon.library") ;
	#endif

	#if USE_DISKFONT == 1
	/* Open the diskfont library.... */
	if ((DiskFontBase = (struct DiskFontBase *)OpenLibrary ("diskfont.library", 0L)) == NULL)
		Error ("Could not open the DiskFont.library") ;
	#endif

	if ((raster = AllocMem((WIDTH/8)*HEIGHT*DEPTH, MEMF_CHIP|MEMF_CLEAR)) == NULL)
		Error("no raster memory");

	InitBitMap(&bm, DEPTH, WIDTH, HEIGHT);
	bm.Planes[0] = raster;
	bm.Planes[1] = raster + ((WIDTH/8)*HEIGHT);

/* Using OpenWindowTags() doesn't work. Resizing the window trashes the 
 * border and window contents.
 * There is also no close gadget.
*/

#ifdef FUCKED
	if ((w = OpenWindowTags(NULL, 
			WA_Left, 100,
			WA_Top, 100,
			WA_Width, 200,
			WA_Height, 100,
			WA_SuperBitMap, &bm,
			WA_SizeGadget,
			WA_GimmeZeroZero,
			WA_DragBar,
			WA_CloseGadget,
			WA_IDCMP, IDCMP_RAWKEY|IDCMP_CLOSEWINDOW,
			WA_MaxWidth, WIDTH,
			WA_MaxHeight, HEIGHT,
			TAG_END)) == NULL)
		Error("Could not open window");
#else
	if ((w = OpenWindow(&nw)) == NULL)
		Error("Could not open window");
#endif

	if ((tmpraster = AllocMem(TMPRASSIZE, MEMF_CHIP|MEMF_CLEAR)) == NULL)
		Error("No TmpRas memory");

	printf("Hey dude. Allocating %ld\n",TMPRASSIZE);
	InitTmpRas(&tr, tmpraster, TMPRASSIZE);
	w->RPort->TmpRas = &tr;

	InitArea(&ai, &aibuff, MAXVECTORS);
	w->RPort->AreaInfo = &ai;

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

	if (tmpraster)
		FreeMem(tmpraster, TMPRASSIZE);

	if (raster)
		FreeMem(raster, (WIDTH/8)*HEIGHT*DEPTH);

	if (w) {
		w->RPort->TmpRas = NULL;
		w->RPort->AreaInfo = NULL;
		w->RPort->BitMap = NULL;
		CloseWindow(w);
	}

	#if USE_DISKFONT == 1
	/* Close the DiskFont library */
	if (DiskFontBase)
		CloseLibrary ((struct Library *) DiskFontBase) ;
	#endif

	#if USE_ICON == 1
	/* Close the icon library */
	if (IconBase)
		CloseLibrary (IconBase) ;
	#endif

	#if USE_ARP == 1
	/* Close the arp library */
	if (ArpBase)
		CloseLibrary ((struct Library *) ArpBase) ;
	#endif

	#if USE_LAYERS == 1
	/* Close the layers Library */
	if (LayersBase)
		CloseLibrary (LayersBase) ;
	#endif

	#if USE_GRAPHICS == 1
	/* Close the Graphics Library */
	if (GfxBase)
		CloseLibrary ((struct Library *) GfxBase) ;
	#endif

	#if USE_INTUITION == 1
	/* Close the Intuition Library */
	if (IntuitionBase)
		CloseLibrary ((struct Library *) IntuitionBase) ;
	#endif

	#if USE_DOS == 1
	/* DOS closed by Lattice */
	#endif

	return ;
}

/***************************************************************************/

void Scroll(struct Window *w, USHORT Code, USHORT Qual)
{
	int x = 0, y = 0;

	if (Code == CURSORLEFT)
		x = -10;
	if (Code == CURSORRIGHT)
		x = 10;
	if (Code == CURSORUP)
		y = -10;
	if (Code == CURSORDOWN)
		y = 10;

	ScrollLayer(NULL, w->RPort->Layer, x, y);

	return;
}

void MakePattern(struct Window *w)
{
	USHORT rad1, rad2, x, y;

	rad1 = (rand() % (WIDTH/2));
	rad2 = (rand() % (HEIGHT/2));
	x = (rand() % WIDTH);
	y = (rand() % HEIGHT);

	if ((x-rad1) < 0)
		x = rad1;
	if ((x+rad1) > WIDTH)
		x = WIDTH - rad1;
	if ((y-rad2) < 0)
		y = rad2;
	if ((y+rad2) > HEIGHT)
		y = HEIGHT - rad2;

	SetAPen(w->RPort, (rand() % ((1<<DEPTH)-1)));
	AreaEllipse(w->RPort, x, y, rad1, rad2);
	AreaEnd(w->RPort);

	return;
}

void main (int argc, char *argv[])
{
	struct IntuiMessage *event;

	Init () ;

	Move(w->RPort,10,10);
	Text(w->RPort,"UL",2);
	Move(w->RPort,900,10);
	Text(w->RPort,"UR",2);
	Move(w->RPort,900,900);
	Text(w->RPort,"LR",2);
	Move(w->RPort,10,900);
	Text(w->RPort,"LL",2);
	do {
		if (event = (struct IntuiMessage *) GetMsg(w->UserPort)) {
			switch (event->Class) {
				case (IDCMP_CLOSEWINDOW) :
					Error("Bye Bye\n");
					break;
				case (IDCMP_RAWKEY) :
					Scroll(w, event->Code, event->Qualifier);
					break;
				default :
					break;
			}
			ReplyMsg(&(event->ExecMessage));
		}
	} while (1<2);

}
