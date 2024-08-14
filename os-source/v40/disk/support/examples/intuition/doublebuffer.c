/*
 * doublebuffer.c - shows double-buffering, attached screens, menu lending
 *
 * (c) Copyright 1992 Commodore-Amiga, Inc.  All rights reserved.
 *
 * This software is provided as-is and is subject to change; no warranties
 * are made.  All use is at your own risk.  No liability or responsibility
 * is assumed.
 *
 * Example shows use of V39 double-buffering functions to achieve
 * 60 frame-per-second animation.  Also shows use of menus in
 * double-buffered screens, and the use of attached screens with
 * menu-lending to achieve the appearance of slider gadgets in
 * double-buffered screens.
 * 
 */

/*----------------------------------------------------------------------*/

#include <exec/types.h>
#include <graphics/displayinfo.h>
#include <graphics/videocontrol.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>

/*----------------------------------------------------------------------*/

STRPTR init_all( void  );
void error_exit( STRPTR errorstring );
struct Gadget *createAllGadgets( struct Gadsget **glistptr, void *vi );
BOOL handleIntuiMessage( struct IntuiMessage *imsg );
void handleDBufMessage( struct Message *dbmsg );
ULONG handleBufferSwap( void );
struct BitMap *makeImageBM( void );
void CloseWindowSafely( struct Window *win );

#define abs( x )	( ( x )>=0?( x ):-( x ) )
int CXBRK( void ) {return( 0 );}

/*----------------------------------------------------------------------*/

/* Some constants to handle the rendering of the animated face */
#define BM_WIDTH	120
#define BM_HEIGHT	60
#define BM_DEPTH	2

/* Odd numbers to give a non-repeating bounce */
#define CONTROLSC_TOP		191
#define SC_ID			HIRES_KEY

/*----------------------------------------------------------------------*/

/* User interface constants and variables */

#define GAD_HORIZ	1
#define GAD_VERT	2

#define MENU_RUN	1
#define MENU_STEP	2
#define MENU_QUIT	3
#define MENU_HSLOW	4
#define MENU_HFAST	5
#define MENU_VSLOW	6
#define MENU_VFAST	7

struct TextAttr Topaz80 =
{
    "topaz.font", 	/* Name */
    8, 			/* YSize */
    0, 			/* Style */
    0, 			/* Flags */
};

struct TagItem vctags[] =
{
    VTAG_BORDERSPRITE_SET, TRUE, 
    TAG_DONE, 0, 
};

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

struct NewMenu demomenu[] =
{
	{ NM_TITLE, "Project", 		  0 , 0, 0, 0, }, 
	{  NM_ITEM, "Run", 		 "R", 0, 0, ( APTR ) MENU_RUN, }, 
	{  NM_ITEM, "Step", 		 "S", 0, 0, ( APTR ) MENU_STEP, }, 
	{  NM_ITEM, NM_BARLABEL, 	  0 , 0, 0, 0, }, 
	{  NM_ITEM, "Slower Horizontal", "1", 0, 0, ( APTR ) MENU_HSLOW, }, 
	{  NM_ITEM, "Faster Horizontal", "2", 0, 0, ( APTR ) MENU_HFAST, }, 
	{  NM_ITEM, "Slower Vertical", 	 "3", 0, 0, ( APTR ) MENU_VSLOW, }, 
	{  NM_ITEM, "Faster Vertical", 	 "4", 0, 0, ( APTR ) MENU_VFAST, }, 

	{  NM_ITEM, NM_BARLABEL, 	  0 , 0, 0, 0, }, 
	{  NM_ITEM, "Quit", 		 "Q", 0, 0, ( APTR ) MENU_QUIT, }, 

	{   NM_END, 0, 			  0 , 0, 0, 0, }, 
};

struct Screen *canvassc = NULL;
struct Screen *controlsc = NULL;
struct Window *controlwin = NULL;
struct Window *canvaswin = NULL;
struct Gadget *glist = NULL;
struct Gadget *horizgad, *vertgad;
struct Menu *menu = NULL;
void *canvasvi = NULL;
void *controlvi = NULL;

/*----------------------------------------------------------------------*/

#define	OK_REDRAW	1	/* Buffer fully detached, ready for redraw */
#define OK_SWAPIN	2	/* Buffer redrawn, ready for swap-in */

struct GfxBase *GfxBase = NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct Library *GadToolsBase = NULL;

struct MsgPort *dbufport = NULL;
struct MsgPort *userport = NULL;

struct ScreenBuffer *scbuf[] =
{
    NULL, 
    NULL, 
};
struct RastPort rport[ 2 ];

ULONG status[ 2 ];

LONG prevx[ 2 ] =
{
    50, 50, 
};

LONG prevy[ 2 ] =
{
    50, 50, 
};

ULONG buf_current, buf_nextdraw, buf_nextswap;
ULONG count;
struct BitMap *face = NULL;
LONG x, y, xstep, xdir, ystep, ydir;

/*----------------------------------------------------------------------*/

main()
{
    STRPTR errorstring;
    ULONG sigs;
    BOOL terminated = FALSE;

    /* Let's get everything initialized */
    if ( errorstring = init_all() )
    {
	error_exit( errorstring );
    }

    count = 0;
    buf_current = 0;
    buf_nextdraw = 1;
    buf_nextswap = 1;
    sigs = 0;

    while ( !terminated )
    {
	/* Check for and handle any IntuiMessages */
	if ( sigs & ( 1 << userport->mp_SigBit ) )
	{
	    struct IntuiMessage *imsg;

	    while ( imsg = GT_GetIMsg( userport ) )
	    {
		terminated |= handleIntuiMessage( imsg );
		GT_ReplyIMsg( imsg );
	    }
	}

	/* Check for and handle any double-buffering messages.
	 * Note that double-buffering messages are "replied" to
	 * us, so we don't want to reply them to anyone.
	 */
	if ( sigs & ( 1 << dbufport->mp_SigBit ) )
	{
	    struct Message *dbmsg;
	    while ( dbmsg = GetMsg( dbufport ) )
	    {
		handleDBufMessage( dbmsg );
	    }
	}


	if ( !terminated )
	{
	    ULONG held_off = 0;
	    /* Only handle swapping buffers if count is non-zero */
	    if ( count )
	    {
		held_off = handleBufferSwap();
	    }
	    if ( held_off )
	    {
		/* If were held-off at ChangeScreenBuffer() time, then we
		 * need to try ChangeScreenBuffer() again, without awaiting
		 * a signal.  We WaitTOF() to avoid busy-looping.
		 */
		 WaitTOF();
	    }
	    else
	    {
		/* If we were not held-off, then we're all done
		 * with what we have to do.  We'll have no work to do
		 * until some kind of signal arrives.  This will normally
		 * be the arrival of the dbi_SafeMessage from the ROM
		 * double-buffering routines, but it might also be an
		 * IntuiMessage.
		 */
		sigs = Wait( ( 1 << dbufport->mp_SigBit ) | ( 1 << userport->mp_SigBit ) );
	    }
	}
    }

    error_exit( NULL );
}


/*----------------------------------------------------------------------*/

/* Handle the rendering and swapping of the buffers */

ULONG handleBufferSwap( void )
{
    ULONG held_off = 0;
    /* 'buf_nextdraw' is the next buffer to draw into.
     * The buffer is ready for drawing when we've received the
     * dbi_SafeMessage for that buffer.  Our routine to handle
     * messaging from the double-buffering functions sets the
     * OK_REDRAW flag when this message has appeared.
     *
     * Here, we set the OK_SWAPIN flag after we've redrawn
     * the imagery, since the buffer is ready to be swapped in.
     * We clear the OK_REDRAW flag, since we're done with redrawing
     */
    if ( status[ buf_nextdraw ] == OK_REDRAW )
    {
	x += xstep*xdir;
	if ( x < 0 )
	{
	    x = 0;
	    xdir = 1;
	}
	else if ( x > canvassc->Width - BM_WIDTH )
	{
	    x = canvassc->Width - BM_WIDTH - 1;
	    xdir = -1;
	}

	y += ystep*ydir;
	if ( y < canvassc->BarLayer->Height )
	{
	    y = canvassc->BarLayer->Height;
	    ydir = 1;
	}
	else if ( y >= CONTROLSC_TOP - BM_HEIGHT )
	{
	    y = CONTROLSC_TOP - BM_HEIGHT - 1;
	    ydir = -1;
	}

	SetAPen( &rport[ buf_nextdraw ], 0 );
	RectFill( &rport[ buf_nextdraw ], 
	    prevx[ buf_nextdraw ], prevy[ buf_nextdraw ], 
	    prevx[ buf_nextdraw ] + BM_WIDTH - 1, prevy[ buf_nextdraw ] + BM_HEIGHT - 1 );
	prevx[ buf_nextdraw ] = x;
	prevy[ buf_nextdraw ] = y;

	BltBitMapRastPort( face, 0, 0, &rport[ buf_nextdraw ], x, y, 
	    BM_WIDTH, BM_HEIGHT, 0xc0 );

	WaitBlit(); /* Gots to let the BBMRP finish */

	status[ buf_nextdraw ] = OK_SWAPIN;

	/* Toggle which the next buffer to draw is.
	 * If you're using multiple ( >2 ) buffering, you
	 * would use
	 *
	 *    buf_nextdraw = ( buf_nextdraw+1 ) % NUMBUFFERS;
	 *
	 */
	buf_nextdraw ^= 1;
    }

    /* Let's make sure that the next frame is rendered before we swap...
     */
    if ( status[ buf_nextswap ] == OK_SWAPIN )
    {
	scbuf[ buf_nextswap ]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = dbufport;

	if ( ChangeScreenBuffer( canvassc, scbuf[ buf_nextswap ] ) )
	{
	    status[ buf_nextswap ] = 0;

	    buf_current = buf_nextswap;
	    /* Toggle which the next buffer to swap in is.
	     * If you're using multiple ( >2 ) buffering, you
	     * would use
	     *
	     *    buf_nextswap = ( buf_nextswap+1 ) % NUMBUFFERS;
	     *
	     */
	    buf_nextswap ^= 1;

	    count--;
	}
	else
	{
	    held_off = 1;
	}
    }
    return( held_off );
}

/*----------------------------------------------------------------------*/

/* Handle Intuition messages */

BOOL handleIntuiMessage( struct IntuiMessage *imsg )
{
    UWORD code = imsg->Code;
    BOOL terminated = FALSE;

    switch ( imsg->Class )
    {
	case IDCMP_GADGETDOWN:
	case IDCMP_GADGETUP:
	case IDCMP_MOUSEMOVE:
	    switch ( ( ( struct Gadget * )imsg->IAddress )->GadgetID )
	    {
		case GAD_HORIZ:
		    xstep = code;
		    break;

		case GAD_VERT:
		    ystep = code;
		    break;
	    }
	    break;

	case IDCMP_VANILLAKEY:
	    switch ( code )
	    {
		case 'S':
		case 's':
		    count = 1;
		    break;

		case 'R':
		case 'r':
		    count = ~0;
		    break;

		case 'Q':
		case 'q':
		    count = 0;
		    terminated = TRUE;
		    break;
	    }
	    break;

	case IDCMP_MENUPICK:
	    while ( code != MENUNULL )
	    {
		struct MenuItem *item;

		item = ItemAddress( menu, code );
		switch ( ( ULONG ) GTMENUITEM_USERDATA( item ) )
		{
		    case MENU_RUN:
			count = ~0;
			break;

		    case MENU_STEP:
			count = 1;
			break;

		    case MENU_QUIT:
			count = 0;
			terminated = TRUE;
			break;

		    case MENU_HSLOW:
			if ( xstep > 0 )
			{
			    xstep--;
			}
			GT_SetGadgetAttrs( horizgad, controlwin, NULL, 
			    GTSL_Level, xstep, 
			    TAG_DONE );
			break;

		    case MENU_HFAST:
			if ( xstep < 9 )
			{
			    xstep++;
			}
			GT_SetGadgetAttrs( horizgad, controlwin, NULL, 
			    GTSL_Level, xstep, 
			    TAG_DONE );
			break;

		    case MENU_VSLOW:
			if ( ystep > 0 )
			{
			    ystep--;
			}
			GT_SetGadgetAttrs( vertgad, controlwin, NULL, 
			    GTSL_Level, ystep, 
			    TAG_DONE );
			break;

		    case MENU_VFAST:
			if ( ystep < 9 )
			{
			    ystep++;
			}
			GT_SetGadgetAttrs( vertgad, controlwin, NULL, 
			    GTSL_Level, ystep, 
			    TAG_DONE );
			break;
		}
		code = item->NextSelect;
	    }
	    break;
    }
    return( terminated );
}

/*----------------------------------------------------------------------*/

void handleDBufMessage( struct Message *dbmsg )
{
    ULONG buffer;

    /* dbi_SafeMessage is followed by an APTR dbi_UserData1, which
     * contains the buffer number.  This is an easy way to extract
     * it.
     * The dbi_SafeMessage tells us that it's OK to redraw the
     * in the previous buffer.
     */
    buffer = ( ULONG ) *( ( APTR ** ) ( dbmsg+1 ) );
    /* Mark the previous buffer as OK to redraw into.
     * If you're using multiple ( >2 ) buffering, you
     * would use
     *
     *    ( buffer + NUMBUFFERS - 1 ) % NUMBUFFERS
     *
     */
    status[ buffer^1 ] = OK_REDRAW;
}

/*----------------------------------------------------------------------*/

/* Get the resources and objects we need */

STRPTR init_all( void )
{
    if ( !( GfxBase = ( struct GfxBase * )
	OpenLibrary( "graphics.library", 39L ) ) )
    {
	return( "Couldn't open Gfx V39\n" );
    }

    if ( !( IntuitionBase = ( struct IntuitionBase * )
	OpenLibrary( "intuition.library", 39L ) ) )
    {
	return( "Couldn't open Intuition V39\n" );
    }

    if ( !( GadToolsBase = OpenLibrary( "gadtools.library", 39L ) ) )
    {
	return( "Couldn't open GadTools V39\n" );
    }

    if ( !( dbufport = CreateMsgPort() ) )
    {
	return( "Failed to create port\n" );
    }

    if ( !( userport = CreateMsgPort() ) )
    {
	return( "Failed to create port\n" );
    }

    if ( !( canvassc = OpenScreenTags( NULL, 
	SA_DisplayID, SC_ID, 
	SA_Overscan, OSCAN_TEXT, 
	SA_Depth, 2, 
	SA_AutoScroll, 1, 
	SA_Pens, pens, 
	SA_ShowTitle, TRUE, 
	SA_Title, "Intuition double-buffering example", 
	SA_VideoControl, vctags, 
	SA_SysFont, 1, 
	TAG_DONE ) ) )
    {
	return( "Couldn't open screen\n" );
    }

    if ( !( canvasvi = GetVisualInfo( canvassc, 
	TAG_DONE ) ) )
    {
	return( "Couldn't get VisualInfo\n" );
    }

    if ( !( canvaswin = OpenWindowTags( NULL, 
	WA_NoCareRefresh, TRUE, 
	WA_Activate, TRUE, 
	WA_Borderless, TRUE, 
	WA_Backdrop, TRUE, 
	WA_CustomScreen, canvassc, 
	WA_NewLookMenus, TRUE, 
	TAG_DONE ) ) )
    {
	return( "Couldn't open window\n" );
    }
    canvaswin->UserPort = userport;

    ModifyIDCMP( canvaswin, IDCMP_MENUPICK | IDCMP_VANILLAKEY );

    if ( !( controlsc = OpenScreenTags( NULL, 
	SA_DisplayID, SC_ID, 
	SA_Overscan, OSCAN_TEXT, 
	SA_Depth, 2, 
	SA_Pens, pens, 
	SA_Top, CONTROLSC_TOP, 
	SA_Height, 28, 
	SA_Parent, canvassc, 
	SA_ShowTitle, FALSE, 
	SA_Draggable, FALSE, 
	SA_VideoControl, vctags, 
	SA_Quiet, TRUE, 
	SA_SysFont, 1, 
	TAG_DONE ) ) )
    {
	return( "Couldn't open screen\n" );
    }

    if ( !( controlvi = GetVisualInfo( controlsc, 
	TAG_DONE ) ) )
    {
	return( "Couldn't get VisualInfo\n" );
    }

    if ( !( menu = CreateMenus( demomenu, 
	TAG_DONE ) ) )
    {
	return( "Couldn't create menus\n" );
    }

    if ( !LayoutMenus( menu, canvasvi, 
	GTMN_NewLookMenus, TRUE, 
	TAG_DONE ) )
    {
	return( "Couldn't layout menus\n" );
    }

    if ( !createAllGadgets( &glist, controlvi ) )
    {
	return( "Couldn't create gadgets\n" );
    }

    /* A borderless backdrop window so we can get input */
    if ( !( controlwin = OpenWindowTags( NULL, 
	WA_NoCareRefresh, TRUE, 
	WA_Activate, TRUE, 
	WA_Borderless, TRUE, 
	WA_Backdrop, TRUE, 
	WA_CustomScreen, controlsc, 
	WA_NewLookMenus, TRUE, 
	WA_Gadgets, glist, 
	TAG_DONE ) ) )
    {
	return( "Couldn't open window\n" );
    }

    controlwin->UserPort = userport;
    ModifyIDCMP( controlwin, SLIDERIDCMP | IDCMP_MENUPICK | IDCMP_VANILLAKEY );

    GT_RefreshWindow( controlwin, NULL );
    SetMenuStrip( canvaswin, menu );
    LendMenus( controlwin, canvaswin );

    if ( !( scbuf[ 0 ] = AllocScreenBuffer( canvassc, NULL, SB_SCREEN_BITMAP ) ) )
    {
	return( "Couldn't allocate ScreenBuffer 1\n" );
    }

    if ( !( scbuf[ 1 ] = AllocScreenBuffer( canvassc, NULL, SB_COPY_BITMAP ) ) )
    {
	return( "Couldn't allocate ScreenBuffer 2\n" );
    }

    /* Let's use the UserData to store the buffer number, for
     * easy identification when the message comes back.
     */
    scbuf[ 0 ]->sb_DBufInfo->dbi_UserData1 = ( APTR ) ( 0 );
    scbuf[ 1 ]->sb_DBufInfo->dbi_UserData1 = ( APTR ) ( 1 );
    status[ 0 ] = OK_REDRAW;
    status[ 1 ] = OK_REDRAW;

    if ( !( face = makeImageBM() ) )
    {
	return( "Couldn't allocate image bitmap\n" );
    }
    InitRastPort( &rport[ 0 ] );
    InitRastPort( &rport[ 1 ] );
    rport[ 0 ].BitMap = scbuf[ 0 ]->sb_BitMap;
    rport[ 1 ].BitMap = scbuf[ 1 ]->sb_BitMap;

    x = 50;
    y = 70;
    xstep = 1;
    xdir = 1;
    ystep = 1;
    ydir = -1;

    /* All is OK */
    return( 0 );
}

/*----------------------------------------------------------------------*/

/* Draw a crude "face" for animation */

#define MAXVECTORS	10

struct BitMap *makeImageBM( void )
{
    struct BitMap *bm;
    struct RastPort rport;
    struct AreaInfo area;
    struct TmpRas tmpRas;
    PLANEPTR planePtr;

    BYTE areabuffer[ MAXVECTORS*5 ];

    if ( bm = ( struct BitMap * )AllocBitMap( BM_WIDTH, BM_HEIGHT, 
	BM_DEPTH, BMF_CLEAR, NULL ) )
    {
	if ( planePtr = AllocRaster( BM_WIDTH, BM_HEIGHT ) )
	{
	    InitRastPort( &rport );
	    rport.BitMap = bm;

	    InitArea( &area, areabuffer, MAXVECTORS );
	    rport.AreaInfo = &area;

	    InitTmpRas( &tmpRas, planePtr, RASSIZE( BM_WIDTH, BM_HEIGHT ) );
	    rport.TmpRas = &tmpRas;

	    SetABPenDrMd( &rport, 3, 0, JAM1 );
	    AreaEllipse( &rport, BM_WIDTH/2, BM_HEIGHT/2, 
		BM_WIDTH/2-4, BM_HEIGHT/2-4 );
	    AreaEnd( &rport );

	    SetAPen( &rport, 2 );
	    AreaEllipse( &rport, 5*BM_WIDTH/16, BM_HEIGHT/4, 
		BM_WIDTH/9, BM_HEIGHT/9 );
	    AreaEllipse( &rport, 11*BM_WIDTH/16, BM_HEIGHT/4, 
		BM_WIDTH/9, BM_HEIGHT/9 );
	    AreaEnd( &rport );

	    SetAPen( &rport, 1 );
	    AreaEllipse( &rport, BM_WIDTH/2, 3*BM_HEIGHT/4, 
		BM_WIDTH/3, BM_HEIGHT/9 );
	    AreaEnd( &rport );

	    FreeRaster( planePtr, BM_WIDTH, BM_HEIGHT );
	}
	else
	{
	    FreeBitMap( bm );
	    bm = NULL;
	}
    return( bm );
    }
}

/*----------------------------------------------------------------------*/

/* Make a pair of slider gadgets to control horizontal and vertical
 * speed of motion.
 */
struct Gadget *createAllGadgets( struct Gadsget **glistptr, void *vi )
{
    struct NewGadget ng;
    struct Gadget *gad;

    gad = CreateContext( glistptr );

    ng.ng_LeftEdge = 100;
    ng.ng_TopEdge = 1;
    ng.ng_Width = 100;
    ng.ng_Height = 12;
    ng.ng_GadgetText = "Horiz:  ";
    ng.ng_TextAttr = &Topaz80;
    ng.ng_VisualInfo = vi;
    ng.ng_GadgetID = GAD_HORIZ;
    ng.ng_Flags = 0;

    horizgad = gad = CreateGadget( SLIDER_KIND, gad, &ng, 
	GTSL_Min, 0, 
	GTSL_Max, 9, 
	GTSL_Level, 1, 
	GTSL_MaxLevelLen, 1, 
	GTSL_LevelFormat, "%ld", 
	TAG_DONE );

    ng.ng_LeftEdge += 200;
    ng.ng_GadgetID = GAD_VERT;
    ng.ng_GadgetText = "Vert:  ";
    vertgad = gad = CreateGadget( SLIDER_KIND, gad, &ng, 
	GTSL_Min, 0, 
	GTSL_Max, 9, 
	GTSL_Level, 1, 
	GTSL_MaxLevelLen, 1, 
	GTSL_LevelFormat, "%ld", 
	TAG_DONE );

    return( gad );
}

/*----------------------------------------------------------------------*/

/* Clean up everything and exit, printing the errorstring if any */
void error_exit( STRPTR errorstring )
{
    if ( controlwin )
    {
	ClearMenuStrip( controlwin );
	CloseWindowSafely( controlwin );
    }

    if ( canvaswin )
    {
	ClearMenuStrip( canvaswin );
	CloseWindowSafely( canvaswin );
    }

    if ( controlsc )
    {
	CloseScreen( controlsc );
    }

    if ( canvassc )
    {
	FreeScreenBuffer( canvassc, scbuf[ 1 ] );
	FreeScreenBuffer( canvassc, scbuf[ 0 ] );
	CloseScreen( canvassc );
    }

    if ( dbufport )
    {
	DeleteMsgPort( dbufport );
    }

    if ( userport )
    {
	DeleteMsgPort( userport );
    }

    if ( GadToolsBase )
    {
	FreeGadgets( glist );
	FreeMenus( menu );
	FreeVisualInfo( canvasvi );
	FreeVisualInfo( controlvi );
	CloseLibrary( GadToolsBase );
    }

    if ( IntuitionBase )
    {
	CloseLibrary( IntuitionBase );
    }

    if ( face )
    {
	FreeBitMap( face );
    }

    if ( GfxBase )
    {
	CloseLibrary( GfxBase );
    }

    if ( errorstring )
    {
	printf( errorstring );
	exit( 20 );
    }

    exit( 0 );
}


/*----------------------------------------------------------------------*/

void StripIntuiMessages( struct MsgPort *mp, struct Window *win );

/* these functions close an Intuition window
 * that shares a port with other Intuition
 * windows or IPC customers.
 *
 * We are careful to set the UserPort to
 * null before closing, and to free
 * any messages that it might have been
 * sent.
 */
#include "exec/types.h"
#include "exec/nodes.h"
#include "exec/lists.h"
#include "exec/ports.h"
#include "intuition/intuition.h"

void
CloseWindowSafely( struct Window *win )
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
}

/* remove and reply all IntuiMessages on a port that
 * have been sent to a particular window
 * ( note that we don't rely on the ln_Succ pointer
 *  of a message after we have replied it )
 */
void
StripIntuiMessages( mp, win )
struct MsgPort *mp;
struct Window *win;
{
    struct IntuiMessage *msg;
    struct Node *succ;

    msg = ( struct IntuiMessage * ) mp->mp_MsgList.lh_Head;

    while( succ =  msg->ExecMessage.mn_Node.ln_Succ )
    {

	if( msg->IDCMPWindow ==  win )
	{

	    /* Intuition is about to free this message.
	     * Make sure that we have politely sent it back.
	     */
	    Remove( ( struct Node * )msg );

	    ReplyMsg( ( struct Message * )msg );
	}
	    
	msg = ( struct IntuiMessage * ) succ;
    }
}

/*----------------------------------------------------------------------*/
