/*
 * attachdemo.c - demonstrates handling attached screens
 *
 * (c) Copyright 1992-93 Commodore-Amiga, Inc.  All rights reserved.
 *
 * This software is provided as-is and is subject to change; no warranties
 * are made.  All use is at your own risk.  No liability or responsibility
 * is assumed.
 *
 */

/*----------------------------------------------------------------------*/

#include <exec/types.h>
#include <graphics/displayinfo.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>

/*----------------------------------------------------------------------*/

struct ScreenDescriptor
{
    LONG sd_Top;	/* Top edge in non-lace coords */
    LONG sd_Open;	/* If screen is supposed to be open */
    LONG sd_Lace;	/* If screen is supposed to be lace */
    LONG sd_Size;	/* Screen's intended size (see below) */
    LONG sd_Drag;	/* If screen is supposed to be draggable */
    LONG sd_IsLace;	/* If screen was actually last lace */
};

#define SIZE_NORMAL	0
#define SIZE_TALL	1
#define SIZE_WIDE	2
#define SIZE_HUGE	3

/*----------------------------------------------------------------------*/

void error_exit( STRPTR errorstring );
void CloseWindowSafely( struct Window *win );
void StripIntuiMessages( struct MsgPort *mp, struct Window *win );
struct Screen *findPanelScreen( void );
struct Window *openPanel( void );
void closePanel( void );
void handleGadget( struct Gadget *gad, UWORD code );
void jiggleScreen( struct Screen *sc, ULONG flags );

/*----------------------------------------------------------------------*/

struct GfxBase *GfxBase = NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct Library *GadToolsBase = NULL;
struct MsgPort *sharedport = NULL;
void *panel_vi = NULL;
struct Gadget *panel_gadgets = NULL;
struct Window *panel_win = NULL;
struct Screen *panel_screen = NULL;

struct Screen *parent_sc = NULL;
struct Screen *child_sc = NULL;
struct ScreenDescriptor parent_desc =
{
    0,			/* sd_Top */
    FALSE,		/* sd_Open */
    FALSE,		/* sd_Lace */
    SIZE_NORMAL,	/* sd_Size */
    TRUE,		/* sd_Drag */
};

struct ScreenDescriptor child_desc =
{
    80,			/* sd_Top */
    TRUE,		/* sd_Open */
    FALSE,		/* sd_Lace */
    SIZE_NORMAL,	/* sd_Size */
    TRUE,		/* sd_Drag */
};

/*----------------------------------------------------------------------*/

#define CHILD_MAGIC	( (APTR)0x0FACE0FF )
#define PARENT_MAGIC	( (APTR)0x8FACE0FF )

#define GAD_PARENT_OPEN	0	/* settable */
#define GAD_PARENT_LACE	1
#define GAD_PARENT_SIZE	2
#define GAD_PARENT_DRAG	3

#define GAD_CHILD_OPEN	4	/* settable */
#define GAD_CHILD_LACE	5
#define GAD_CHILD_SIZE	6
#define GAD_CHILD_DRAG	7

#define GAD_FORWARD	8
#define GAD_BACK	9
#define GAD_CFORWARD	10
#define GAD_CBACK	11

#define GAD_MOVEPARENT	12
#define GAD_MOVECHILD	13
#define GAD_FMOVEPARENT	14
#define GAD_FMOVECHILD	15

#define GAD_HOMEPARENT	16
#define GAD_HOMECHILD	17

#define NUM_SETTABLE	2

struct Gadget *mygad[ NUM_SETTABLE ];

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

/*----------------------------------------------------------------------*/

STRPTR OpenLabels[] =
{
    "Closed",
    "Open",
    NULL,
};

STRPTR LaceLabels[] =
{
    "Non-Laced",
    "Interlaced",
    NULL,
};

STRPTR SizeLabels[] =
{
    "Normal",
    "Tall",
    "Wide",
    "Huge",
    NULL,
};

STRPTR DragLabels[] =
{
    "Non-Drag",
    "Draggable",
    NULL,
};

LONG scwidth[ 4 ] =
{
    640, 640, 960, 960,	/* normal, tall, wide, huge */
};

LONG cheight[ 4 ] =
{
    
    120, 380, 120, 380,	/* normal, tall, wide, huge */
};

LONG pheight[ 4 ] =
{
    
    200, 400, 200, 400,	/* normal, tall, wide, huge */
};

/*----------------------------------------------------------------------*/

struct Screen *
openMyScreen( BOOL isparent )
{
    struct Screen *sc;
    LONG top, drag, width, height, id;
    STRPTR title;
    ULONG attachtag;
    APTR userdata;
    struct Screen *attachdata = NULL;

    if ( isparent )
    {
	top = parent_desc.sd_Top;
	drag = parent_desc.sd_Drag;
	width = scwidth[ parent_desc.sd_Size ];
	if ( parent_desc.sd_Lace )
	{
	    title = "Laced Parent";
	    height = 2*pheight[ parent_desc.sd_Size ];
	    top *= 2;
	    id = HIRESLACE_KEY;
	}
	else
	{
	    title = "Non-laced Parent";
	    height = pheight[ parent_desc.sd_Size ];
	    id = HIRES_KEY;
	}
	attachtag = SA_FrontChild;
	attachdata = child_sc;
	userdata = CHILD_MAGIC;
    }
    else
    {
	top = child_desc.sd_Top;
	drag = child_desc.sd_Drag;
	width = scwidth[ child_desc.sd_Size ];
	if ( child_desc.sd_Lace )
	{
	    title = "Laced Child";
	    height = cheight[ child_desc.sd_Size ]*2;
	    top *= 2;
	    id = HIRESLACE_KEY;
	}
	else
	{
	    title = "Non-laced Child";
	    height = cheight[ child_desc.sd_Size ];
	    id = HIRES_KEY;
	}
	attachtag = SA_Parent;
	attachdata = parent_sc;
	userdata = PARENT_MAGIC;
    }
    
    if ( sc = OpenScreenTags( NULL,
	SA_DisplayID, id,
	SA_Top, top,
	SA_Title, title,
	SA_Width, width,
	SA_Height, height,
	SA_Overscan, OSCAN_TEXT,
	/*  Other tags can go here: */
	SA_Depth, 2,
	SA_Pens, pens,
	SA_Draggable, drag,
	SA_AutoScroll, TRUE,
	attachtag, attachdata,
	TAG_DONE ) )
    {
	sc->UserData = userdata;
    }
    return( sc );
}

struct Screen *
closeMyScreen( struct Screen *sc )
{
    if ( sc )
    {
	if ( panel_screen == sc )
	{
	    closePanel();
	}
	CloseScreen( sc );
    }
    return( ( struct Screen * )NULL );
}

/*----------------------------------------------------------------------*/

void
main( void )

{
    BOOL done = FALSE;
    struct IntuiMessage *imsg;
    ULONG imsgClass;
    UWORD imsgCode;
    APTR imsgIAddress;

    if ( !( GfxBase = ( struct GfxBase * )
	OpenLibrary( "graphics.library", 39L ) ) )
    {
	error_exit( "Couldn't open Gfx V39\n" );
    }

    if ( !( IntuitionBase = ( struct IntuitionBase * )
	OpenLibrary( "intuition.library", 39L ) ) )
    {
	error_exit( "Couldn't open Intuition V39\n" );
    }

    if ( !( GadToolsBase =
	OpenLibrary( "gadtools.library", 39L ) ) )
    {
	error_exit( "Couldn't open GadTools V39\n" );
    }

    if ( !( sharedport = CreateMsgPort() ) )
    {
	error_exit( "No port\n" );
    }

    if ( child_desc.sd_Open )
    {
	child_sc = openMyScreen( FALSE );
	child_desc.sd_IsLace = child_desc.sd_Lace;
    }

    if ( parent_desc.sd_Open )
    {
	parent_sc = openMyScreen( TRUE );
	parent_desc.sd_IsLace = parent_desc.sd_Lace;
    }

    openPanel();

    while ( !done )
    {
	Wait( 1 << sharedport->mp_SigBit );
	while ( imsg = GT_GetIMsg( sharedport ) )
	{
	    imsgClass = imsg->Class;
	    imsgCode = imsg->Code;
	    imsgIAddress = imsg->IAddress;
	    GT_ReplyIMsg( imsg );

	    switch ( imsgClass )
	    {
		case CLOSEWINDOW:
		    done = TRUE;
		    break;

		case REFRESHWINDOW:
		    /* Only the panel-window has IDCMP_REFRESHWINDOW set */
		    GT_BeginRefresh( panel_win );
		    GT_EndRefresh( panel_win, TRUE );
		    break;

		case GADGETUP:
		    handleGadget( imsgIAddress, imsgCode );
		    break;
	    }
	}
    }
    error_exit( NULL );
}


/*----------------------------------------------------------------------*/

void error_exit( STRPTR errorstring )

{
    closePanel();

    closeMyScreen( child_sc );

    closeMyScreen( parent_sc );

    if ( sharedport )
    {
	DeleteMsgPort( sharedport );
    }

    if ( GadToolsBase )
    {
	CloseLibrary( GadToolsBase );
    }

    if ( IntuitionBase )
    {
	CloseLibrary( ( struct Library * )IntuitionBase );
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

/* these functions close an Intuition window
 * that shares a port with other Intuition
 * windows or IPC customers.
 *
 * We are careful to set the UserPort to
 * null before closing, and to free
 * any messages that it might have been
 * sent.
 */

void CloseWindowSafely( win )
struct Window *win;
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
void StripIntuiMessages( mp, win )
struct MsgPort *mp;
struct Window *win;
{
    struct IntuiMessage *msg;
    struct Node *succ;

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
}

/*----------------------------------------------------------------------*/

struct TextAttr Topaz80 =
{
    "topaz.font",
    8,
    0,
    0,
};

struct Window *openPanel( void )
{
    struct NewGadget ng;
    struct Gadget *gad;
    LONG topborder;
    int settable = 0;

    panel_screen = findPanelScreen();

    topborder = panel_screen->WBorTop + panel_screen->Font->ta_YSize + 1;

    if ( panel_vi = GetVisualInfo( panel_screen,
	TAG_DONE ) )
    {
	gad = CreateContext( &panel_gadgets );

	ng.ng_LeftEdge = 90;
	ng.ng_TopEdge = 2+topborder;
	ng.ng_GadgetText = "Parent";
	ng.ng_Width = 120;
	ng.ng_Height = 14;
	ng.ng_TextAttr = &Topaz80;
	ng.ng_VisualInfo = panel_vi;
	ng.ng_GadgetID = GAD_PARENT_OPEN;
	ng.ng_UserData = &parent_desc.sd_Open;
	ng.ng_Flags = NULL;

	mygad[ settable++ ] = gad = CreateGadget( CYCLE_KIND, gad, &ng,
	    GTCY_Labels, OpenLabels,
	    GTCY_Active, parent_desc.sd_Open,
	    TAG_DONE );
	ng.ng_GadgetText = NULL;
	ng.ng_UserData = 0;

	ng.ng_LeftEdge += 130;
	ng.ng_GadgetID++;
	gad = CreateGadget( CYCLE_KIND, gad, &ng,
	    GTCY_Labels, LaceLabels,
	    GTCY_Active, parent_desc.sd_Lace,
	    TAG_DONE );

	ng.ng_LeftEdge += 130;
	ng.ng_GadgetID++;
	gad = CreateGadget( CYCLE_KIND, gad, &ng,
	    GTCY_Labels, SizeLabels,
	    GTCY_Active, parent_desc.sd_Size,
	    TAG_DONE );

	ng.ng_LeftEdge += 130;
	ng.ng_GadgetID++;
	gad = CreateGadget( CYCLE_KIND, gad, &ng,
	    GTCY_Labels, DragLabels,
	    GTCY_Active, parent_desc.sd_Drag,
	    TAG_DONE );

	ng.ng_LeftEdge = 90;
	ng.ng_TopEdge += 18;
	ng.ng_GadgetText = "Child";
	ng.ng_GadgetID++;
	ng.ng_UserData = &child_desc.sd_Open;

	mygad[ settable++ ] = gad = CreateGadget( CYCLE_KIND, gad, &ng,
	    GTCY_Labels, OpenLabels,
	    GTCY_Active, child_desc.sd_Open,
	    TAG_DONE );
	ng.ng_GadgetText = NULL;
	ng.ng_UserData = NULL;

	ng.ng_LeftEdge += 130;
	ng.ng_GadgetID++;
	gad = CreateGadget( CYCLE_KIND, gad, &ng,
	    GTCY_Labels, LaceLabels,
	    GTCY_Active, child_desc.sd_Lace,
	    TAG_DONE );

	ng.ng_LeftEdge += 130;
	ng.ng_GadgetID++;
	gad = CreateGadget( CYCLE_KIND, gad, &ng,
	    GTCY_Labels, SizeLabels,
	    GTCY_Active, child_desc.sd_Size,
	    TAG_DONE );

	ng.ng_LeftEdge += 130;
	ng.ng_GadgetID++;
	gad = CreateGadget( CYCLE_KIND, gad, &ng,
	    GTCY_Labels, DragLabels,
	    GTCY_Active, child_desc.sd_Drag,
	    TAG_DONE );

	ng.ng_LeftEdge = 90;
	ng.ng_TopEdge += 18;
	ng.ng_GadgetText = "Forward";
	ng.ng_GadgetID++;
	ng.ng_UserData = SDEPTH_TOFRONT;
	gad = CreateGadget( BUTTON_KIND, gad, &ng,
	    TAG_DONE );

	ng.ng_LeftEdge += 130;
	ng.ng_GadgetText = "Back";
	ng.ng_GadgetID++;
	ng.ng_UserData = ( APTR )( SDEPTH_TOBACK );
	gad = CreateGadget( BUTTON_KIND, gad, &ng,
	    TAG_DONE );

	ng.ng_LeftEdge += 130;
	ng.ng_GadgetText = "Child Forward";
	ng.ng_GadgetID++;
	ng.ng_UserData = ( APTR )( SDEPTH_TOFRONT|SDEPTH_CHILDONLY );
	gad = CreateGadget( BUTTON_KIND, gad, &ng,
	    TAG_DONE );

	ng.ng_LeftEdge += 130;
	ng.ng_GadgetText = "Child Back";
	ng.ng_GadgetID++;
	ng.ng_UserData = ( APTR )( SDEPTH_TOBACK|SDEPTH_CHILDONLY );
	gad = CreateGadget( BUTTON_KIND, gad, &ng,
	    TAG_DONE );

	ng.ng_LeftEdge = 90;
	ng.ng_TopEdge += 18;
	ng.ng_GadgetText = "Move Parent";
	ng.ng_GadgetID++;
	gad = CreateGadget( BUTTON_KIND, gad, &ng,
	    TAG_DONE );

	ng.ng_LeftEdge += 130;
	ng.ng_GadgetText = "Move Child";
	ng.ng_GadgetID++;
	gad = CreateGadget( BUTTON_KIND, gad, &ng,
	    TAG_DONE );

	ng.ng_LeftEdge += 130;
	ng.ng_GadgetText = "FMove Parent";
	ng.ng_GadgetID++;
	gad = CreateGadget( BUTTON_KIND, gad, &ng,
	    TAG_DONE );

	ng.ng_LeftEdge += 130;
	ng.ng_GadgetText = "FMove Child";
	ng.ng_GadgetID++;
	gad = CreateGadget( BUTTON_KIND, gad, &ng,
	    TAG_DONE );

	ng.ng_LeftEdge = 90;
	ng.ng_TopEdge += 18;
	ng.ng_UserData = 0;
	ng.ng_GadgetText = "Home Parent";
	ng.ng_GadgetID++;
	gad = CreateGadget( BUTTON_KIND, gad, &ng,
	    TAG_DONE );

	ng.ng_LeftEdge += 130;
	ng.ng_GadgetText = "Home Child";
	ng.ng_GadgetID++;
	gad = CreateGadget( BUTTON_KIND, gad, &ng,
	    TAG_DONE );

	if ( gad )    
	{
	    if ( panel_win = OpenWindowTags( NULL,
		WA_Width, 640,
		WA_Height, 105,
		WA_Top, 11,
		WA_Activate, TRUE,
		WA_CloseGadget, TRUE,
		WA_DragBar, TRUE,
		WA_SizeGadget, TRUE,
		WA_DepthGadget, TRUE,
		WA_SimpleRefresh, TRUE,
		WA_Title, "Control Panel",
		WA_Gadgets, panel_gadgets,
		WA_CustomScreen, panel_screen,
		TAG_DONE ) )
	    {
		/* whew! all is OK */
		panel_win->UserPort = sharedport;
		ModifyIDCMP( panel_win, CLOSEWINDOW | VANILLAKEY | CYCLEIDCMP | BUTTONIDCMP | REFRESHWINDOW );
		GT_RefreshWindow( panel_win, NULL );
		return( panel_win );
	    }
	    FreeGadgets( panel_gadgets );
	    panel_gadgets = NULL;
	}
	FreeVisualInfo( panel_vi );
	panel_vi = NULL;
    }
    return( NULL );
}

/*----------------------------------------------------------------------*/

void closePanel( void )
{
    if ( panel_win )
    {
	CloseWindowSafely( panel_win ); panel_win = NULL;
	FreeGadgets( panel_gadgets ); panel_gadgets = NULL;
	FreeVisualInfo( panel_vi ); panel_vi = NULL;
	panel_screen = NULL;
    }
}

/*----------------------------------------------------------------------*/

struct Screen *findPanelScreen( void )
{
    struct Screen *sc;

    ULONG lock = LockIBase( NULL );
    for ( sc = IntuitionBase->FirstScreen; sc; sc = sc->NextScreen )
    {
	if ( ( sc->UserData == CHILD_MAGIC ) || ( sc->UserData == PARENT_MAGIC ) )
	{
	    break;
	}
    }
    UnlockIBase( lock );
    return( sc );
}

/*----------------------------------------------------------------------*/

void handleGadget( struct Gadget *gad, UWORD code )
{
    BOOL undo = FALSE;	/* Set to true if gadget states need to be corrected */
    BOOL rethinkp = FALSE;
    BOOL rethinkc = FALSE;

    switch ( gad->GadgetID )
    {
	case GAD_PARENT_OPEN:
	    if ( !code )	/* request to close */
	    {
		/* Only close parent if child still around */
		if ( ( parent_sc ) && ( child_sc ) )
		{
		    parent_desc.sd_Open = FALSE;
		    rethinkp = TRUE;
		}
		else
		{
		    undo = TRUE;
		}
	    }
	    else		/* request to open */
	    {
		if ( !parent_sc )
		{
		    parent_desc.sd_Open = TRUE;
		    rethinkp = TRUE;
		}
	    }
	    break;

	case GAD_PARENT_LACE:
	    parent_desc.sd_Lace = code;
	    rethinkp = TRUE;
	    break;

	case GAD_PARENT_SIZE:
	    parent_desc.sd_Size = code;
	    rethinkp = TRUE;
	    break;

	case GAD_PARENT_DRAG:
	    parent_desc.sd_Drag = code;
	    rethinkp = TRUE;
	    break;

	case GAD_CHILD_OPEN:
	    if ( !code )	/* request to close */
	    {
		/* Only close child if parent still around */
		if ( ( child_sc ) && ( parent_sc ) )
		{
		    child_desc.sd_Open = FALSE;
		    rethinkc = TRUE;
		}
		else
		{
		    undo = TRUE;
		}
	    }
	    else		/* request to open */
	    {
		if ( !child_sc )
		{
		    child_desc.sd_Open = TRUE;
		    rethinkc = TRUE;
		}
	    }
	    break;

	case GAD_CHILD_LACE:
	    child_desc.sd_Lace = code;
	    rethinkc = TRUE;
	    break;

	case GAD_CHILD_SIZE:
	    child_desc.sd_Size = code;
	    rethinkc = TRUE;
	    break;

	case GAD_CHILD_DRAG:
	    child_desc.sd_Drag = code;
	    rethinkc = TRUE;
	    break;

	case GAD_FORWARD:
	case GAD_BACK:
	case GAD_CFORWARD:
	case GAD_CBACK:
	    if ( child_sc )
	    {
		/* The appropriate screen-depth code is in gad->UserData */
		ScreenDepth( child_sc, ( ULONG )gad->UserData, NULL );
	    }
	    break;

	case GAD_MOVECHILD:
	case GAD_FMOVECHILD:
	    if ( child_sc )
	    {
		jiggleScreen( child_sc, ( gad->GadgetID == GAD_FMOVECHILD ) ?
		    SPOS_FORCEDRAG : 0 );
	    }
	    break;

	case GAD_MOVEPARENT:
	case GAD_FMOVEPARENT:
	    if ( parent_sc )
	    {
		jiggleScreen( parent_sc, ( gad->GadgetID == GAD_FMOVEPARENT ) ?
		    SPOS_FORCEDRAG : 0 );
	    }
	    break;

	case GAD_HOMEPARENT:
	    if ( parent_sc )
	    {
		ScreenPosition( parent_sc, SPOS_ABSOLUTE, 0, 0, 0, 0 );
	    }
	    break;

	case GAD_HOMECHILD:
	    if ( child_sc )
	    {
		ScreenPosition( child_sc, SPOS_ABSOLUTE, 0, 0, 0, 0 );
	    }
	    break;
    }
    if ( rethinkp )
    {
	if ( parent_sc )
	{
	    parent_desc.sd_Top = parent_sc->TopEdge;
	    if ( parent_desc.sd_IsLace )
	    {
		parent_desc.sd_Top /= 2;
	    }
	    parent_sc = closeMyScreen( parent_sc );
	}
	if ( parent_desc.sd_Open )
	{
	    parent_sc = openMyScreen( TRUE );
	    parent_desc.sd_IsLace = parent_desc.sd_Lace;
	}
    }

    if ( rethinkc )
    {
	if ( child_sc )
	{
	    child_desc.sd_Top = child_sc->TopEdge;
	    if ( child_desc.sd_IsLace )
	    {
		child_desc.sd_Top /= 2;
	    }
	    child_sc = closeMyScreen( child_sc );
	}
	if ( child_desc.sd_Open )
	{
	    child_sc = openMyScreen( FALSE );
	    child_desc.sd_IsLace = child_desc.sd_Lace;
	}
    }

    if ( undo && panel_win )
    {
	int i;
	for ( i = 0; i < NUM_SETTABLE; i++ )
	{
	    LONG *longptr;
	    if ( longptr = mygad[ i ]->UserData )
	    {
		GT_SetGadgetAttrs( mygad[ i ], panel_win, NULL,
		    GTCY_Active, *longptr,
		    TAG_DONE );
	    }
	}
    }

    if ( ( !panel_win ) || ( panel_win->WScreen != findPanelScreen() ) )
    {
	closePanel();
	openPanel();
    }
}

/*----------------------------------------------------------------------*/

void jiggleScreen( struct Screen *sc, ULONG flags )
{
    LONG i, step;

    step = 2;
    for ( i = 0; i < 40; i++ )
    {
	ScreenPosition( sc, flags, 0, step, 0, 0 );
	if ( i == 9 )
	{
	    step = -2;
	}
	else if ( i == 29 )
	{
	    step = 2;
	}
    }
    step = -2;
    for ( i = 0; i < 40; i++ )
    {
	ScreenPosition( sc, flags, step, 0, 0, 0 );
	if ( i == 19 )
	{
	    step = 2;
	}
    }
}

/*----------------------------------------------------------------------*/
