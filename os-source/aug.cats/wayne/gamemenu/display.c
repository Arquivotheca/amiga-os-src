/*************

	Display.c
	W.D.L 930330

**************/

/*
 * COPYRIGHT: Unless otherwise noted, all files are Copyright (c) 1993
 * Commodore-Amiga, Inc.  All rights reserved.
 *
 * DISCLAIMER: This software is provided "as is".  No representations or
 * warranties are made with respect to the accuracy, reliability, performance,
 * currentness, or operation of this software, and all use is at your own risk.
 * Neither commodore nor the authors assume any responsibility or liability
 * whatsoever with respect to your use of this software.
 */

// Tab size is 8!

#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <graphics/videocontrol.h>

#include "devices/cd.h"

#include <hardware/custom.h>
#include <hardware/intbits.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/macros.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#include <string.h>	// for setmem()
#include "stdio.h"

#include "disp_def.h"
#include "retcodes.h"

#include "debugsoff.h"

/*	// Uncomment to get debug output turned on
#define KPRINTF
#include "debugson.h"
*/


IMPORT struct GfxBase		* GfxBase;
IMPORT struct IntuitionBase	* IntuitionBase;
IMPORT struct Custom		  far custom;

IMPORT	BOOL			DisplayIsPal;

ULONG		  smartgfx = 0x12345678;

VOID CloseDisplay( DISP_DEF * disp_def );
//int kprintf(const char *, ...);
int DoILBM( UBYTE * filename, DISP_DEF * disp_def );

STATIC USHORT chip InvisiblePointer[]= {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000};


ULONG
GetModeID( DISP_DEF * disp_def )
{
    /* Check that the ModeID can handle doublebuffering.
     * If not, then find the best ModeID that can.
     */
    struct DisplayInfo	disp;
    ULONG		ModeID = disp_def->ModeID;

    D(PRINTF("GetModeID ENTERED with ModeID= 0x%lx\n",ModeID);)

    ModeID &= ~MONITOR_ID_MASK;

    if ( GetDisplayInfoData(NULL, (UBYTE *)&disp, sizeof(struct DisplayInfo), DTAG_DISP, ModeID) ) {

	if ( !(disp.PropertyFlags & DIPF_IS_DBUFFER) ) {

	    ModeID = BestModeID(BIDTAG_NominalWidth, disp_def->Width,
	    BIDTAG_NominalHeight, disp_def->Height,
	    BIDTAG_Depth, disp_def->Depth,
	    /* Keep the DIPF_ properties of the original
	     * ModeID, but ensure we have DBUFFER
	     * properties too.
	     */
	    BIDTAG_SourceID, ModeID,
	    BIDTAG_DIPFMustHave, DIPF_IS_DBUFFER,
	    TAG_END);
	}
    }

    D(PRINTF("GetModeID RETURNING with ModeID= 0x%lx\n\n",ModeID);)

    return( disp_def->ModeID = ModeID );

} // GetModeID()

#define	DO_CAROLYN_CENTER

#ifdef	DO_CAROLYN_CENTER	// [
/*
 * clipit - passed width and height of a display, and the text, std, and
 *          max overscan rectangles for the mode, clipit fills in the
 *          spos (screen pos) and dclip rectangles to use in centering.
 *          Centered around smallest containing user-editable oscan pref,
 *          with dclip confined to legal maxoscan limits.
 *          Screens which center such that their top is below text
 *          oscan top, will be moved up.
 *          If a non-null uclip is passed, that clip is used instead.
 */
void clipit(SHORT wide, SHORT high,
	    struct Rectangle *spos, struct Rectangle *dclip,
	    struct Rectangle *txto, struct Rectangle *stdo,
	    struct Rectangle *maxo, struct Rectangle *uclip)
{
struct  Rectangle *besto;
SHORT	minx, maxx, miny, maxy;
SHORT	txtw, txth, stdw, stdh, maxw, maxh, bestw, besth;

    /* get the txt, std and max widths and heights */
    txtw = txto->MaxX - txto->MinX + 1;
    txth = txto->MaxY - txto->MinY + 1;
    stdw = stdo->MaxX - stdo->MinX + 1;
    stdh = stdo->MaxY - stdo->MinY + 1;
    maxw = maxo->MaxX - maxo->MinX + 1;
    maxh = maxo->MaxY - maxo->MinY + 1;

    if((wide <= txtw)&&(high <= txth))
	{
	besto = txto;
	bestw = txtw;
	besth = txth;

	D(bug("Best clip is txto\n"));
	}
    else
	{
	besto = stdo;
	bestw = stdw;
	besth = stdh;

	D(bug("Best clip is stdo\n"));
	}

    D(bug("TXTO: mnx=%ld mny=%ld mxx=%ld mxy=%ld  stdw=%ld stdh=%ld\n",
		txto->MinX,txto->MinY,txto->MaxX,txto->MaxY,txtw,txth));
    D(bug("STDO: mnx=%ld mny=%ld mxx=%ld mxy=%ld  stdw=%ld stdh=%ld\n",
		stdo->MinX,stdo->MinY,stdo->MaxX,stdo->MaxY,stdw,stdh));
    D(bug("MAXO: mnx=%ld mny=%ld mxx=%ld mxy=%ld  maxw=%ld maxh=%ld\n",
		maxo->MinX,maxo->MinY,maxo->MaxX,maxo->MaxY,maxw,maxh));

    if(uclip)
	{
	*dclip = *uclip;
    	spos->MinX = uclip->MinX;
	spos->MinY = uclip->MinY;

	D(bug("UCLIP: mnx=%ld mny=%ld maxx=%ld maxy=%ld\n",
			dclip->MinX,dclip->MinY,dclip->MaxX,dclip->MaxY));
	}
    else
	{
	/* CENTER the screen based on best oscan prefs
 	* but confine dclip within max oscan limits
 	*
 	* FIX MinX first */
	spos->MinX = minx = besto->MinX - ((wide - bestw) >> 1);
	maxx = wide + minx - 1;
	if(maxx > maxo->MaxX)  maxx = maxo->MaxX;	/* too right */
	if(minx < maxo->MinX)  minx = maxo->MinX;	/* too left  */

	D(bug("DCLIP: minx adjust from %ld to %ld\n",spos->MinX,minx));

	/* FIX MinY */
	spos->MinY = miny = besto->MinY - ((high - besth) >> 1);
	/* if lower than top of txto, move up */
	spos->MinY = miny = MIN(spos->MinY,txto->MinY);
	maxy = high + miny - 1;
	if(maxy > maxo->MaxY)  maxy = maxo->MaxY;	/* too down  */
	if(miny < maxo->MinY)  miny = maxo->MinY;	/* too up    */

	D(bug("DCLIP: miny adjust from %ld to %ld\n",spos->MinY,miny));

	/* SET up dclip */
	dclip->MinX = minx;
	dclip->MinY = miny;
	dclip->MaxX = maxx;
	dclip->MaxY = maxy;

	D(bug("CENTER: mnx=%ld mny=%ld maxx=%ld maxy=%ld\n",
			dclip->MinX,dclip->MinY,dclip->MaxX,dclip->MaxY));
	}
}

#else				// ][

VOID
CenterDisplay( DISP_DEF * disp_def )
{
    int			i,ht = NTSC_HEIGHT;
    struct Rectangle	drect;
    struct DisplayInfo	disp;

    if ( GetDisplayInfoData(NULL, (UBYTE *)&disp, sizeof(struct DisplayInfo), DTAG_DISP, LORES_KEY) ) {
	if ( disp.PropertyFlags & DIPF_IS_PAL )
	    ht = PAL_HEIGHT;
    }

    // High Resolution guess
    i = SCREEN_WIDTH;
    if ( !( (disp_def->Width >= i) || (disp_def->ModeID & HIRES) ) ) {
	i >>= 1;
    }

    if ( disp_def->Width > i)
	disp_def->Flags |= DISP_OVERSCANX;

    i = ht;

    // InterLace guess
    if ( !( (disp_def->Height >= i) || (disp_def->ModeID & LACE) ) ) {
	i >>= 1;
    }

    if ( disp_def->Height > i)
	disp_def->Flags |= DISP_OVERSCANY;

    if ( disp_def->Flags & DISP_OVERSCAN ) {

	QueryOverscan( disp_def->ModeID, &drect, OSCAN_MAX );

	D(PRINTF("QueryOverscan says MinX= %ld, MaxX= %ld, Width= %ld\n",
	    drect.MinX,drect.MaxX,drect.MaxX-drect.MinX);)
	if ( (disp_def->Flags & DISP_OVERSCANX) && (disp_def->Flags & DISP_CENTERX) ) {
	    D(PRINTF("CenterDisplay() DISP_OVERSCANX\n");)
	    disp_def->NominalWidth = (drect.MaxX - drect.MinX) + 1;
	    disp_def->Left = drect.MinX + ((disp_def->Width - (drect.MaxX-drect.MinX))/2);
	}

	if ( (disp_def->Flags & DISP_OVERSCANY) && (disp_def->Flags & DISP_CENTERY) ) {
	    D(PRINTF("CenterDisplay() DISP_OVERSCANY\n");)
	    disp_def->NominalHeight = (drect.MaxY - drect.MinY) + 1;
	    disp_def->Top = drect.MinY + (disp_def->NominalHeight - disp_def->Height);
	}

	D(PRINTF("CenterDisplay() DISP_OVERSCAN, NominalWidth= %ld, NominalHeight= %ld\nWidth= %ld, Height= %ld\n MinX= %ld, MaxX= %ld, MinY= %ld, MaxY= %ld\n",
	    disp_def->NominalWidth,disp_def->NominalHeight,
	    disp_def->Width,disp_def->Height,
	    drect.MinX,drect.MaxX,drect.MinY,drect.MaxY);)
    }

    D(PRINTF("CenterDisplay() Left= %ld, Top= %ld\n",disp_def->Left,disp_def->Top);)

} // CenterDisplay
#endif				// ]

/*
 * Open a screen and a window as defined by disp_def. Optionally
 * load an ILBM file into background.
 */
ScrWinOpen( DISP_DEF * disp_def, UBYTE * ilbmfile )
{
    struct Window	* win = NULL;
    ULONG		tag;
    ULONG		flags;
    struct Rectangle	drect;

#ifdef	DO_CAROLYN_CENTER	// [
    struct Rectangle	txto,stdo,maxo,spos,dclip;

#endif				// ]

    struct ColorSpec	initialcolors[] = 
			{
			    {0,0,0,0},
			    {-1,0,0,0}
			};
#ifdef	OUTT	// [
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
    
	    (UWORD)~0,
	};
#else		// ][
    UWORD pens[] =
	{
	    1, /* DETAILPEN */
	    1, /* BLOCKPEN	*/
	    7, /* TEXTPEN	*/
	    7, /* SHINEPEN	*/
	    2, /* SHADOWPEN	*/
	    4, /* FILLPEN	*/
	    4, /* FILLTEXTPEN	*/
	    1, /* BACKGROUNDPEN	*/
	    7, /* HIGHLIGHTTEXTPEN	*/
    
	    3, /* BARDETAILPEN	*/
	    5, /* BARBLOCKPEN	*/
	    6, /* BARTRIMPEN	*/
    
	    (UWORD)~0,
	};
#endif		// ]

    D(PRINTF("ScrWinOPen() 1\n");)

    GetModeID( disp_def );

    D(PRINTF("ScrWinOPen() 2\n");)

#ifdef	DO_CAROLYN_CENTER	// [

    if ( disp_def->Flags & DISP_CENTER ) {
	QueryOverscan( disp_def->ModeID, &txto, OSCAN_TEXT );
	QueryOverscan( disp_def->ModeID, &stdo, OSCAN_STANDARD );
	QueryOverscan( disp_def->ModeID, &maxo, OSCAN_MAX );

	clipit( disp_def->Width, disp_def->Height, &spos, &dclip, &txto, &stdo,
	    &maxo,NULL);

	D(PRINTF("spos.MinX= %ld, MinY= %ld\n",spos.MinX,spos.MinY);)

	disp_def->Left = spos.MinX - 12;
	disp_def->Top =  DisplayIsPal ? (spos.MinY + 10) : (spos.MinY-8);
    }
#else				// ][
    if ( disp_def->Flags & DISP_CENTER )
	CenterDisplay( disp_def );

    drect.MinX = disp_def->Left;
    drect.MaxX = disp_def->Left + disp_def->Width;

    drect.MinY = disp_def->Top;
    drect.MaxY = disp_def->Top + disp_def->Height;
#endif				// ]
    /*
     * If we are opening a full sized screen as opposed to a screen
     * that is just the size of the CDXL dimensions.
     */
    if ( disp_def->Flags & DISP_ALLOCBM ) {
	flags = BMF_CLEAR|BMF_DISPLAYABLE;
	D(PRINTF("ScrWinOpen() DISP_ALLOCBM\n");)

	if ( disp_def->Flags & DISP_INTERLEAVED )
	    flags |= BMF_INTERLEAVED;

	if (!(disp_def->bm[0] = (struct BitMap *)AllocBitMap( disp_def->Width, 
	 disp_def->Height, disp_def->Depth, flags, NULL )) ) {

	    D(PRINTF("Could NOT AllocBitMap() 1\n");)
	    disp_def->bm[1] = NULL;
	    return( RC_NO_MEM );
	}

	if (!(disp_def->bm[1] = (struct BitMap *)AllocBitMap( disp_def->Width, 
	 disp_def->Height, disp_def->Depth, flags, NULL )) ) {

	    D(PRINTF("Could NOT AllocBitMap() 2\n");)

	    FreeBitMap( disp_def->bm[0] );
	    disp_def->bm[0] = NULL;

	    return( RC_NO_MEM );
	}
	tag = TAG_IGNORE;
    } else {
	tag = SA_Left;
    }

    D(PRINTF("ScrWinOPen() 3 ModeID= 0x%lx, Left= %ld, Top= %ld, W= %ld, H= %ld, Depth= %ld, bm[0]= 0x%lx\n",
	disp_def->ModeID,disp_def->Left,disp_def->Top,disp_def->Width,disp_def->Height,disp_def->Depth,disp_def->bm[0]);)

    if ( disp_def->screen = OpenScreenTags(NULL,
	SA_Top,		disp_def->Top,
	SA_Left,	disp_def->Left,
	SA_Width,	disp_def->Width,
	SA_Height,	disp_def->Height,
	SA_Depth,	disp_def->Depth,
	SA_BitMap,	disp_def->bm[0],
	SA_DisplayID,	disp_def->ModeID,
	SA_Interleaved,	(disp_def->Flags & DISP_INTERLEAVED) ? TRUE : FALSE,
	SA_ShowTitle,	FALSE,
	SA_Quiet,	TRUE,
	SA_Behind,	TRUE,
	SA_Colors,	initialcolors,
	SA_Pens,	pens,
#ifdef	DO_CAROLYN_CENTER	// [
	SA_DClip,	&dclip,
#else				// ][
	SA_DClip,	&drect,
#endif				// ]
	SA_Overscan,	OSCAN_MAX,
	TAG_DONE) )
    {
/*
	VideoControlTags( disp_def->screen->ViewPort.ColorMap, VC_IntermediateCLUpdate,0,0);
	VideoControlTags( disp_def->screen->ViewPort.ColorMap, VC_IntermediateCLUpdate_Query,&smartgfx,0);
*/

	smartgfx=(smartgfx==0x12345678)?0:-1;
	D(PRINTF("\nsmartgfx= '%ls'\n\n",
	    smartgfx ? "SMART" : "DUMB" );)

	if ( win = OpenWindowTags( NULL,
	    WA_Width,		disp_def->screen->Width,
	    WA_Height,		disp_def->screen->Height,
	    WA_IDCMP,		NULL,	//IDCMP_MOUSEBUTTONS,
	    WA_Flags,		BACKDROP | BORDERLESS | RMBTRAP,
	    WA_Activate,	TRUE,
	    WA_CustomScreen,	disp_def->screen,
	    TAG_DONE) ) {

	    D(PRINTF("ScrWinOPen() 5 Screen->Left= %ld, Top= %ld, Width= %ld, Height= %ld,\n",
		disp_def->screen->LeftEdge,disp_def->screen->TopEdge,
		disp_def->screen->Width,disp_def->screen->Height);)


	    D(PRINTF("ScrWinOPen() 5.1 win->Left= %ld, Top= %ld, Width= %ld, Height= %ld,\n",
		win->LeftEdge,win->TopEdge,win->Width,win->Height);)


	    if ( disp_def->Flags & DISP_NOPOINTER )
		SetPointer( win, InvisiblePointer, 1, 1, 0, 0 );

	    disp_def->vp = &disp_def->screen->ViewPort;

	    if ( 1 /*disp_def->dbuf = AllocDBufInfo( disp_def->vp )*/ ) {

		D(PRINTF("ScrWinOPen() 6\n");)

		if ( (disp_def->Flags & DISP_BACKGROUND) && ilbmfile && *ilbmfile ) {

		    D(PRINTF("ScrWinOPen() 7\n");)

		    if ( DoILBM( ilbmfile, disp_def ) )
			D(PRINTF("Could not open '%ls'\n",ilbmfile);)
		}

		D(PRINTF("ScrWinOPen() 8\n");)

		disp_def->Flags |= DISP_OPEN;
//		ScreenPosition( disp_def->screen, SPOS_ABSOLUTE, disp_def->Left, disp_def->Top, 0, 0);
		ScreenToFront( disp_def->screen );
		return( RC_OK );
	    }
	    D(PRINTF("ScrWinOPen() 9\n");)

	    if ( disp_def->dbuf ) {
		FreeDBufInfo( disp_def->dbuf );
	    }
	    CloseWindow( win );
	}

	D(PRINTF("ScrWinOPen() 10\n");)

	CloseScreen( disp_def->screen );
	disp_def->screen = NULL;

	if ( disp_def->Flags & DISP_ALLOCBM ) {
	    FreeBitMap( disp_def->bm[0] );
	    FreeBitMap( disp_def->bm[1] );
	    disp_def->bm[0] = disp_def->bm[1] = NULL;
	}
	return( RC_NO_WIN );
    }

    D(PRINTF("ScrWinOPen() 11\n");)

    if ( disp_def->Flags & DISP_ALLOCBM ) {
	FreeBitMap( disp_def->bm[0] );
	FreeBitMap( disp_def->bm[1] );
	disp_def->bm[0] = disp_def->bm[1] = NULL;
    }

    return( RC_NO_SC );

} // ScrWinOpen()


/* remove and reply all IntuiMessages on a port that
 * have been sent to a particular window
 * ( note that we don't rely on the ln_Succ pointer
 *  of a message after we have replied it )
 */
VOID
StripIntuiMessages( struct MsgPort * mp, struct Window * win )
{
    struct IntuiMessage *msg;
    struct Node *succ;

    if ( !(mp && win ) )
	return;

    msg = ( struct IntuiMessage * ) mp->mp_MsgList.lh_Head;

    while( succ =  msg->ExecMessage.mn_Node.ln_Succ ) {

	if( msg->IDCMPWindow ==  win ) {

	    /* Intuition is about to free this message.
	     * Make sure that we have politely sent it back.
	     */
	    Remove( ( struct Node * )msg );

	    ReplyMsg( ( struct Message * )msg );
	}
	    
	msg = ( struct IntuiMessage * ) succ;
    }

} // StripIntuiMessages()


VOID
CloseWindowSafely( struct Window * win )
{
    /* we forbid here to keep out of race conditions with Intuition */
    Forbid();

    /* send back any messages for this window 
     * that have not yet been processed
     */
    StripIntuiMessages( win->UserPort, win );

    /* clear UserPort so Intuition will not free it */
    win->UserPort = NULL;

    /* tell Intuition to stop sending more messages */
    ModifyIDCMP( win, 0L );

    /* turn multitasking back on */
    Permit();

    /* and really close the window */
    CloseWindow( win );

} // CloseWindowSafely()


/* 
 *  Close screen or view.
 */
VOID
CloseDisplay( DISP_DEF * disp_def )
{
    D(PRINTF("CloseDisplay() 1\n");)

    if ( disp_def->dbuf ) {
	D(PRINTF("CloseDisplay() 2\n");)
	FreeDBufInfo( disp_def->dbuf );
	disp_def->dbuf = NULL;
    }
    /* 
     * Close the screen and window.
     */
    if ( disp_def->screen ) {
	D(PRINTF("CloseDisplay() 2\n");)

	while ( disp_def->screen->FirstWindow ) {
	    D(PRINTF("CloseDisplay() 4\n");)
	    CloseWindowSafely( disp_def->screen->FirstWindow );
	}

	CloseScreen( disp_def->screen );
	disp_def->screen = NULL;
    }
    /* 
     * If we allocated these bitmaps with AllocBitMap(),
     * free them with FreeBitMap().
     */
    if ( disp_def->Flags & DISP_ALLOCBM ) {
	D(PRINTF("CloseDisplay() 5\n");)

	if ( disp_def->bm[0] ) {
	    D(PRINTF("CloseDisplay() 6\n");)

	    FreeBitMap( disp_def->bm[0] );
	    disp_def->bm[0] = NULL;
	}

	if ( disp_def->bm[1] ) {
	    D(PRINTF("CloseDisplay() 7\n");)

	    FreeBitMap( disp_def->bm[1] );
	    disp_def->bm[1] = NULL;
	}
    }

    if ( disp_def->colorrecord && disp_def->crecsize ) {
	D(PRINTF("CloseDisplay() 8 crecsize= %ld\n",disp_def->crecsize);)

	FreeMem( disp_def->colorrecord, disp_def->crecsize );
	disp_def->colorrecord = NULL;
	disp_def->crecsize = NULL;
    }
    disp_def->Flags &= ~DISP_OPEN;

} // CloseDisplay()


#define	REVERSE	-1
#define	FORWARD	 1

VOID
ColorCycle( DISP_DEF * disp_def )
{
    Color32	  color,last,* colortable32;
    int		  i,j,cnt;
    BOOL	  load;

    if ( !(colortable32 = (Color32 *)(&disp_def->colorrecord[2])) )
	return;

    load = FALSE;

    for ( i = 0; i < MAX_CRNG; i++ ) {
	if ( !disp_def->crng[i].active )
	    continue;

//	D(PRINTF("i: %ld active= %ld\n",i,disp_def->crng[i].active);)

	cnt = disp_def->cyclecount[i] + disp_def->crng[i].rate;
	if ( cnt >= 0x4000 ) {
	    cnt -= 0x4000;
	    load = TRUE;

	    if ( disp_def->crng[i].active == FORWARD )  {

		D(PRINTF("%ld: FORWARD low= %ld, high= %ld\n",
		    i,disp_def->crng[i].low,disp_def->crng[i].high);)

		D(
		 if ( i == 1 ) {
		    D(PRINTF("\n");)
		    DumpColorTable32( colortable32, disp_def->crng[i].low, disp_def->crng[i].high );
		 }
		)
#ifdef	OUTT	// [
		color = colortable32[disp_def->crng[i].high];
		for (j=disp_def->crng[i].high; j > disp_def->crng[i].low;j -= 2) {
		    colortable32[j-1] = colortable32[j];
		}
		colortable32[j] = color;

#else		// ][

		color = colortable32[disp_def->crng[i].high];
		for (j=disp_def->crng[i].low; j <= disp_def->crng[i].high;j++) {
		    last = colortable32[j];
		    colortable32[j] = color;
		    color = last;
		}
#endif		// ]
		D(
		 if ( i == 1 ) {
		    DumpColorTable32( colortable32, disp_def->crng[i].low, disp_def->crng[i].high );
		 }
		)

	    } else {
		D(PRINTF("REVERSE\n");)
		color = colortable32[disp_def->crng[i].low];
		for (j=disp_def->crng[i].low; j < disp_def->crng[i].high;j += 2) {
		    colortable32[j] = colortable32[j+1];
		}
		colortable32[j] = color;
	    }
	}
	disp_def->cyclecount[i] = cnt;
    }

    if ( load ) {
	D(PRINTF("Loading numcolors= %ld, first= %ld, Depth= %ld\n",
	    disp_def->colorrecord[0],disp_def->colorrecord[1],disp_def->Depth);)

	LoadRGB32( disp_def->vp, (ULONG *)disp_def->colorrecord );
    }

} // ColorCycle()


#ifdef	DEBUG	// [
DumpColorTable32( Color32 * c32, int low, int high )
{
    int i;

    for ( i = low; i < high; i++ ) {
	D(PRINTF("r= 0x%lxd, g= 0x%lx, b= 0x%lx\n",c32[i].r,c32[i].g,c32[i].b);)
    }

   return( TRUE );
}
#endif		// ]
