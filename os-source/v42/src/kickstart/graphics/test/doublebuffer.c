/*
 * doublebuffer.c - shows double-buffering, attached screens, menu lending
 *
 * (c) Copyright 1992 Commodore-Amiga, Inc.  All Rights Reserved.
 * Preliminary and Confidential.
 * 
 * THIS CONFIDENTIAL PRELIMINARY SOFTWARE IS FOR DEVELOPER TESTING ONLY.
 * 
 * THIS SOFTWARE MAY NOT BE REDISTRIBUTED.
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

STRPTR init_all();
void error_exit(STRPTR errorstring);
struct Gadget *createAllGadgets( struct Gadsget **glistptr, void *vi );
void handleIntuiMessage( struct IntuiMessage *imsg );
void handleDBufMessage( struct Message *dbmsg );
ULONG handleBufferSwap( void );

#define BM_WIDTH	120
#define BM_HEIGHT	60
#define BM_OFFSET	15
#define BM_DEPTH	2

#define BLT_WIDTH	(BM_WIDTH+2*BM_OFFSET)
#define BLT_HEIGHT	(BM_HEIGHT+2*BM_OFFSET)

/* Odd numbers to give a non-repeating bounce */
#define SC_MINX		0
#define SC_MAXX		639
#define SC_MINY		15
#define SC_MAXY		180
#define SC_ID		HIRES_KEY

#define GAD_HORIZ	1
#define GAD_VERT	2

#define abs(x)	((x)>=0?(x):-(x))

/*----------------------------------------------------------------------*/

struct TextAttr Topaz80 =
{
    "topaz.font",	/* Name */
    8,			/* YSize */
    0,			/* Style */
    0,			/* Flags */
};

struct GfxBase *GfxBase = NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct Library *GadToolsBase = NULL;

struct Screen *canvassc = NULL;
struct Screen *controlsc = NULL;
struct Window *controlwin = NULL;
struct Window *canvaswin = NULL;
struct Gadget *glist = NULL;

struct MsgPort *dbufport = NULL;
struct MsgPort *userport = NULL;

struct ScreenBuffer *scbuf[] =
{
    NULL,
    NULL,
    NULL,
};
struct RastPort rport[3];

ULONG sigs;
ULONG buf_current, buf_nextdraw, buf_nextswap;
ULONG count;
BOOL terminated = FALSE;
struct BitMap *face = NULL;
struct Menu *menu = NULL;
void *myvi = NULL;
void *childvi = NULL;
ULONG status[3];
ULONG frames = 0;
LONG x,y,xstep,xdir,ystep,ydir;

/*----------------------------------------------------------------------*/

#define MENU_RUN	1
#define MENU_STEP	2
#define MENU_QUIT	3
#define MENU_HSLOW	4
#define MENU_HFAST	5
#define MENU_VSLOW	6
#define MENU_VFAST	7

#define	OK_REDRAW	1	/* Buffer fully detached, ready for redraw */
#define OK_SWAPOUT	2	/* Buffer been displayed, ready for swap-out */
#define OK_SWAPIN	4	/* Buffer redrawn, ready for swap-in */

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

struct NewMenu mynewmenu[] =
{
	{ NM_TITLE, "Project",		  0 , 0, 0, 0,},
	{  NM_ITEM, "Run",		 "R", 0, 0, (APTR) MENU_RUN,},
	{  NM_ITEM, "Step",		 "S", 0, 0, (APTR) MENU_STEP,},
	{  NM_ITEM, NM_BARLABEL,	  0 , 0, 0, 0,},
	{  NM_ITEM, "Slower Horizontal", "1", 0, 0, (APTR) MENU_HSLOW,},
	{  NM_ITEM, "Faster Horizontal", "2", 0, 0, (APTR) MENU_HFAST,},
	{  NM_ITEM, "Slower Vertical",	 "3", 0, 0, (APTR) MENU_VSLOW,},
	{  NM_ITEM, "Faster Vertical",	 "4", 0, 0, (APTR) MENU_VFAST,},

	{  NM_ITEM, NM_BARLABEL,	  0 , 0, 0, 0,},
	{  NM_ITEM, "Quit...",		 "Q", 0, 0, (APTR) MENU_QUIT,},

	{   NM_END, 0,		  0 , 0, 0, 0,},
};

/*----------------------------------------------------------------------*/

int CXBRK(void) {return(0);}
void KPRINTF() {}
void KPRINTFF() {}

/*----------------------------------------------------------------------*/

struct BitMap *makeImageBM(void);
void CloseWindowSafely( struct Window *win );

struct TagItem vctags[] =
{
    VTAG_BORDERSPRITE_SET, TRUE,
    TAG_DONE, 0,
};

/*----------------------------------------------------------------------*/

main()
{
    STRPTR errorstring;

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
		handleIntuiMessage( imsg );
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
	    if ( !held_off )
	    {
		/* If were held-off at ChangeScreenBuffer() time, then we
		 * need to try CSB() again, without necessarily getting
		 * a signal.  If we were not held-off, then we're all done
		 * with what we have to do.  Let's wait for something
		 * to happen.
		 */
KPRINTF("Waiting\n");
		sigs = Wait( ( 1 << dbufport->mp_SigBit ) | ( 1 << userport->mp_SigBit ) );
	    }
	}
    }

    error_exit(NULL);
}


/*----------------------------------------------------------------------*/

ULONG handleBufferSwap( void )
{
    ULONG held_off = 0;
KPRINTF("Status of next buffer to draw %ld is %ld\n", buf_nextdraw, status[buf_nextdraw]);
    /* 'buf_nextdraw' is the next buffer to draw into.
     * The buffer is ready for drawing when we've received the
     * SafeMessage for that buffer.  Our routine to handle
     * messaging from the double-buffering functions sets the
     * OK_REDRAW flag when this message has appeared.
     *
     * Here, we set the OK_SWAPIN flag after we've redrawn
     * the imagery, since the buffer is ready to be swapped in.
     * We clear the OK_REDRAW flag, since we're done with redrawing
     */
    if ( ( status[ buf_nextdraw ] & ( OK_REDRAW | OK_SWAPIN ) ) == OK_REDRAW )
    {
KPRINTF("Redrawing buffer %ld\n",buf_nextdraw);
	x += xstep*xdir;
	if ( x < SC_MINX )
	{
	    x = SC_MINX;
	    xdir = 1;
	}
	else if (x >= SC_MAXX-BLT_WIDTH )
	{
	    x = SC_MAXX-BLT_WIDTH-1;
	    xdir = -1;
	}

	y += ystep*ydir;
	if ( y < SC_MINY )
	{
	    y = SC_MINY;
	    ydir = 1;
	}
	else if (y >= SC_MAXY-BLT_HEIGHT )
	{
	    y = SC_MAXY-BLT_HEIGHT-1;
	    ydir = -1;
	}
	BltBitMapRastPort( face, 0, 0, &rport[buf_nextdraw], x, y,
	    BLT_WIDTH, BLT_HEIGHT, 0xc0 );
	WaitBlit(); /* Gots to let the BBMRP finish */
SET( buf_nextdraw, 'D' );
SHOW();

	status[ buf_nextdraw ] |= OK_SWAPIN;
	buf_nextdraw = ( buf_nextdraw+1 ) % 3;
    }

KPRINTF("Status of current buffer %ld is %ld\n", buf_current, status[buf_current]);
    /* Let's make sure the currently displayed buffer has been visible
     * for at least one frame.  We know this is true when we've received
     * the DispMessage for this buffer.  Our message handler sets OK_SWAPOUT
     * when the DispMessage has come in.
     * Also, we want to assure that the next frame is rendered before
     * we swap...
     */
    if ( ( status[ buf_current ] & OK_SWAPOUT ) &&
	( status[ buf_nextswap ] & OK_SWAPIN ) )
    {
	scbuf[buf_nextswap]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = dbufport;
	scbuf[buf_nextswap]->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort = dbufport;

KPRINTF("Changing to buffer %ld\n", buf_nextswap);
/*kprintf("CSB\n"); */
	if ( ChangeScreenBuffer( canvassc, scbuf[buf_nextswap] ) )
	{
/*kprintf("  ok\n"); */
CLEAR( 0, 'C' );
CLEAR( 1, 'C' );
CLEAR( 2, 'C' );
SET( buf_nextswap, 'C' );
CLEAR( buf_nextswap, 'D' );
KPRINTFF("\n");
SHOW();
	    frames++;
	    status[ buf_nextswap ] = 0;

	    buf_current = buf_nextswap;
	    buf_nextswap = ( buf_nextswap+1 ) % 3;

	    count--;
	}
	else
	{
/* kprintf("  nok\n"); */
	    held_off = 1;
	}
    }
    return( held_off );
}

/*----------------------------------------------------------------------*/

void handleIntuiMessage( struct IntuiMessage *imsg )
{
    UWORD code = imsg->Code;

    switch ( imsg->Class )
    {
	case IDCMP_GADGETDOWN:
	case IDCMP_GADGETUP:
	case IDCMP_MOUSEMOVE:
	    switch ( ((struct Gadget *)imsg->IAddress)->GadgetID )
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
STOP();
		    count = 1;
		    break;

		case 'R':
		case 'r':
START();
		    count = ~0;
		    break;

		case 'Q':
		case 'q':
STOP();
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
		switch ( (ULONG) GTMENUITEM_USERDATA(item) )
		{
		    case MENU_RUN:
START();
			count = ~0;
			break;

		    case MENU_STEP:
STOP();
			count = 1;
			break;

		    case MENU_QUIT:
STOP();
			count = 0;
			terminated = TRUE;
			break;

		    case MENU_HSLOW:
			if ( xstep > 0 )
			{
			    xstep--;
			}
			break;

		    case MENU_HFAST:
			if ( xstep < 9 )
			{
			    xstep++;
			}
			break;

		    case MENU_VSLOW:
			if ( ystep > 0 )
			{
			    ystep--;
			}
			break;

		    case MENU_VFAST:
			if ( ystep < 9 )
			{
			    ystep++;
			}
			break;
		}
		code = item->NextSelect;
	    }
	    break;
    }
}

/*----------------------------------------------------------------------*/

void handleDBufMessage( struct Message *dbmsg )
{
    ULONG buffer;

    /* dbi_SafeMessage is followed by an APTR dbi_UserData1, which
     * contains the buffer number.
     */
    buffer = (ULONG) *((APTR **) (dbmsg+1));
CLEAR( (buffer+2) % 3, 'V' );
SHOW();
    status[ ( buffer+2 ) % 3 ] |= OK_REDRAW;
    status[ buffer ] |= OK_SWAPOUT;
}

/*----------------------------------------------------------------------*/

STRPTR init_all()
{
    if (!( GfxBase = (struct GfxBase *)
	OpenLibrary("graphics.library", 39L) ))
    {
	return("Couldn't open Gfx V39\n");
    }

    if (!( IntuitionBase = (struct IntuitionBase *)
	OpenLibrary("intuition.library", 39L) ))
    {
	return("Couldn't open Intuition V39\n");
    }

    if (!(GadToolsBase = OpenLibrary("gadtools.library", 39L)))
    {
	return("Couldn't open GadTools V39\n");
    }

    if (!(dbufport = CreateMsgPort() ))
    {
	return("Failed to create port\n");
    }

    if (!(userport = CreateMsgPort() ))
    {
	return("Failed to create port\n");
    }

    if (!(canvassc = OpenScreenTags(NULL,
	SA_DisplayID, SC_ID,
	SA_Overscan, OSCAN_TEXT,
	/*  Other tags can go here: */
	SA_Depth, 2,
	SA_AutoScroll, 1,
	SA_Pens, pens,
	SA_Height, 200,
	SA_ShowTitle, TRUE,
	SA_Title, "Intuition double-buffering example",
	SA_VideoControl, vctags,
	TAG_DONE )))
    {
	return("Couldn't open screen\n");
    }

    if (!(myvi = GetVisualInfo(canvassc,
	TAG_DONE)))
    {
	return("Couldn't get VisualInfo\n");
    }

    if (!( canvaswin = OpenWindowTags(NULL,
	WA_NoCareRefresh, TRUE,
	WA_Activate, TRUE,
	WA_Borderless, TRUE,
	WA_Backdrop, TRUE,
	WA_CustomScreen, canvassc,
	WA_NewLookMenus,TRUE,
	TAG_DONE ) ))
    {
	return("Couldn't open window\n");
    }
    canvaswin->UserPort = userport;

    ModifyIDCMP( canvaswin, IDCMP_MENUPICK|IDCMP_VANILLAKEY );

    if (!(controlsc = OpenScreenTags(NULL,
	SA_DisplayID, SC_ID,
	SA_Overscan, OSCAN_TEXT,
	/*  Other tags can go here: */
	SA_Depth, 2,
	SA_Pens, pens,
	SA_Top, SC_MAXY,
	SA_Height, 28,
	SA_Parent, canvassc,
	SA_ShowTitle, FALSE,
	SA_Draggable, FALSE,
	SA_VideoControl, vctags,
	TAG_DONE )))
    {
	return("Couldn't open screen\n");
    }

    if (!(childvi = GetVisualInfo(controlsc,
	TAG_DONE)))
    {
	return("Couldn't get VisualInfo\n");
    }

    if (!(menu = CreateMenus(mynewmenu,
	TAG_DONE)))
    {
	return("Couldn't create menus\n");
    }

    if (!LayoutMenus(menu, myvi,
	GTMN_NewLookMenus, TRUE,
	TAG_DONE))
    {
	return("Couldn't layout menus\n");
    }

    if (!createAllGadgets( &glist, childvi ))
    {
	return("Couldn't create gadgets\n");
    }

    /* A borderless backdrop window so we can get input */
    if (!( controlwin = OpenWindowTags(NULL,
	WA_NoCareRefresh, TRUE,
	WA_Activate, TRUE,
	WA_Borderless, TRUE,
	WA_Backdrop, TRUE,
	WA_CustomScreen, controlsc,
	WA_NewLookMenus,TRUE,
	WA_Gadgets, glist,
	TAG_DONE ) ))
    {
	return("Couldn't open window\n");
    }

    controlwin->UserPort = userport;
    ModifyIDCMP( controlwin, IDCMP_GADGETDOWN|IDCMP_MOUSEMOVE|IDCMP_GADGETUP|IDCMP_MENUPICK|IDCMP_VANILLAKEY );

    GT_RefreshWindow( controlwin, NULL );
    SetMenuStrip( canvaswin, menu );
    LendMenus( controlwin, canvaswin );

    if (!( scbuf[0] = AllocScreenBuffer( canvassc, NULL, SB_SCREEN_BITMAP ) ) )
    {
	return("Couldn't allocate ScreenBuffer 1\n");
    }

    if (!( scbuf[1] = AllocScreenBuffer( canvassc, NULL, SB_COPY_BITMAP ) ) )
    {
	return("Couldn't allocate ScreenBuffer 2\n");
    }

    if (!( scbuf[2] = AllocScreenBuffer( canvassc, NULL, SB_COPY_BITMAP ) ) )
    {
	return("Couldn't allocate ScreenBuffer 3\n");
    }

    scbuf[0]->sb_DBufInfo->dbi_UserData1 = (APTR) ( 0 );
    scbuf[1]->sb_DBufInfo->dbi_UserData1 = (APTR) ( 1 );
    scbuf[2]->sb_DBufInfo->dbi_UserData1 = (APTR) ( 2 );
    status[ 0 ] = OK_SWAPOUT | OK_REDRAW;
    status[ 1 ] = OK_SWAPOUT | OK_REDRAW;
    status[ 2 ] = OK_SWAPOUT | OK_REDRAW;

    if ( !( face = makeImageBM() ) )
    {
	return("Couldn't allocate image bitmap\n");
    }
    InitRastPort( &rport[0] );
    InitRastPort( &rport[1] );
    InitRastPort( &rport[2] );
    rport[0].BitMap = scbuf[0]->sb_BitMap;
    rport[1].BitMap = scbuf[1]->sb_BitMap;
    rport[2].BitMap = scbuf[2]->sb_BitMap;

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

#define MAXVECTORS	10

struct BitMap *makeImageBM(void)
{
    struct BitMap *bm;
    struct RastPort rport;
    struct AreaInfo area;
    struct TmpRas tmpRas;
    PLANEPTR planePtr;

    BYTE areabuffer[MAXVECTORS*5];

    if ( bm = (struct BitMap *)AllocBitMap( BLT_WIDTH, BLT_HEIGHT,
	BM_DEPTH, BMF_CLEAR, NULL ) )
    {
	if ( planePtr = AllocRaster( BLT_WIDTH, BLT_HEIGHT ) )
	{
	    InitRastPort( &rport );
	    rport.BitMap = bm;

	    InitArea( &area, areabuffer, MAXVECTORS );
	    rport.AreaInfo = &area;

	    InitTmpRas( &tmpRas, planePtr, RASSIZE(BLT_WIDTH, BLT_HEIGHT) );
	    rport.TmpRas = &tmpRas;

	    SetABPenDrMd( &rport, 3, 0, JAM1 );
	    AreaEllipse( &rport, BM_WIDTH/2+BM_OFFSET, BM_HEIGHT/2+BM_OFFSET,
		BM_WIDTH/2-4, BM_HEIGHT/2-4);
	    AreaEnd( &rport );

	    SetAPen( &rport, 2 );
	    AreaEllipse( &rport, 5*BM_WIDTH/16+BM_OFFSET, BM_HEIGHT/4+BM_OFFSET,
		BM_WIDTH/9, BM_HEIGHT/9);
	    AreaEllipse( &rport, 11*BM_WIDTH/16+BM_OFFSET, BM_HEIGHT/4+BM_OFFSET,
		BM_WIDTH/9, BM_HEIGHT/9);
	    AreaEnd( &rport );

	    SetAPen( &rport, 1 );
	    AreaEllipse( &rport, BM_WIDTH/2+BM_OFFSET, 3*BM_HEIGHT/4+BM_OFFSET,
		BM_WIDTH/3, BM_HEIGHT/9);
	    AreaEnd( &rport );

	    FreeRaster( planePtr, BLT_WIDTH, BLT_HEIGHT );
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

#define SCREENWIDTH	SC_MAXX
#define GADWIDTH	56
#define NUMGAD		3
#define GAP		((SCREENWIDTH-GADWIDTH)/(NUMGAD-1))

struct Gadget *createAllGadgets( struct Gadsget **glistptr, void *vi )
{
    struct NewGadget ng;
    struct Gadget *gad;

    gad = CreateContext(glistptr);

    ng.ng_LeftEdge = 100;
    ng.ng_TopEdge = 13;
    ng.ng_Width = 100;
    ng.ng_Height = 12;
    ng.ng_GadgetText = "Horiz:  ";
    ng.ng_TextAttr = &Topaz80;
    ng.ng_VisualInfo = vi;
    ng.ng_GadgetID = GAD_HORIZ;
    ng.ng_Flags = 0;

    gad = CreateGadget(SLIDER_KIND, gad, &ng,
	GTSL_Min, 0,
	GTSL_Max, 9,
	GTSL_Level, 1,
	GTSL_MaxLevelLen, 1,
	GTSL_LevelFormat, "%ld",
	TAG_DONE);

    ng.ng_LeftEdge += 200;
    ng.ng_GadgetID = GAD_VERT;
    ng.ng_GadgetText = "Vert:  ";
    gad = CreateGadget(SLIDER_KIND, gad, &ng,
	GTSL_Min, 0,
	GTSL_Max, 9,
	GTSL_Level, 1,
	GTSL_MaxLevelLen, 1,
	GTSL_LevelFormat, "%ld",
	TAG_DONE);

    return(gad);
}

/*----------------------------------------------------------------------*/

void error_exit(STRPTR errorstring)
{
    if (controlwin)
    {
	ClearMenuStrip(controlwin);
	CloseWindowSafely(controlwin);
    }

    if (canvaswin)
    {
	ClearMenuStrip(canvaswin);
	CloseWindowSafely(canvaswin);
    }

    if (controlsc)
    {
	CloseScreen(controlsc);
    }

    if (canvassc)
    {
	FreeScreenBuffer( canvassc, scbuf[2] );
	FreeScreenBuffer( canvassc, scbuf[1] );
	FreeScreenBuffer( canvassc, scbuf[0] );
	CloseScreen(canvassc);
    }

    if ( dbufport )
    {
	DeleteMsgPort( dbufport );
    }

    if ( userport )
    {
	DeleteMsgPort( userport );
    }

    if (GadToolsBase)
    {
	FreeGadgets(glist);
	FreeMenus(menu);
	FreeVisualInfo(myvi);
	FreeVisualInfo(childvi);
	CloseLibrary(GadToolsBase);
    }

    if (IntuitionBase)
    {
	CloseLibrary(IntuitionBase);
    }

    if (face)
    {
	FreeBitMap(face);
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
 * (note that we don't rely on the ln_Succ pointer
 *  of a message after we have replied it)
 */
void
StripIntuiMessages( mp, win )
struct MsgPort *mp;
struct Window *win;
{
    struct IntuiMessage *msg;
    struct Node *succ;

    msg = (struct IntuiMessage *) mp->mp_MsgList.lh_Head;

    while( succ =  msg->ExecMessage.mn_Node.ln_Succ )
    {

	if( msg->IDCMPWindow ==  win )
	{

	    /* Intuition is about to free this message.
	     * Make sure that we have politely sent it back.
	     */
	    Remove( (struct Node *)msg );

	    ReplyMsg( (struct Message *)msg );
	}
	    
	msg = (struct IntuiMessage *) succ;
    }
}

/*----------------------------------------------------------------------*/

ULONG startsecs, startmicros;
ULONG stopsecs, stopmicros;

START()
{
    CurrentTime( &startsecs, &startmicros );
    frames = 0;
}

STOP()
{
    if ( count )
    {
	CurrentTime( &stopsecs, &stopmicros );
	if ( stopmicros < startmicros )
	{
	    stopmicros += 1000000;
	    stopsecs--;
	}
	printf("%ld frames in %ld.%06ld seconds\n",
	    frames, stopsecs-startsecs, stopmicros-startmicros );
    }
}
/*----------------------------------------------------------------------*/

/* C = currently set buffer (set at CSB, cleared at CSB)
 * V = possibly visible buffer (set at DispMessage, cleared at SafeMessage)
 * D = ready for next swap (set at Draw, cleared at CSB)
 */

char string[] = "            ";

SET( buffer, ch )
{
    int offset;

    switch ( ch )
    {
	case 'C':
	    offset = 0;
	    break;

	case 'V':
	    offset = 1;
	    break;

	case 'D':
	    offset = 2;
	    break;
    }
    offset += buffer * 4;
    string[ offset ] = ch;
}

CLEAR( buffer, ch )
{
    int offset;

    switch ( ch )
    {
	case 'C':
	    offset = 0;
	    break;

	case 'V':
	    offset = 1;
	    break;

	case 'D':
	    offset = 2;
	    break;
    }
    offset += buffer * 4;
    string[ offset ] = ' ';
}

SHOW()
{
    KPRINTFF( "%s\n", string );
}
