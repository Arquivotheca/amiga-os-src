/*** edit.c **************************************************************
*
*   edit.c	- Edit Overscan Rectangles
*
*   Copyright 1989, 1990, Commodore-Amiga, Inc.
*
*   $Id: edit.c,v 36.10 91/02/22 16:19:25 eric Exp $
*
*   $Log:	edit.c,v $
*   Revision 36.10  91/02/22  16:19:25  eric
*   Incorporate Peter's changes to ConvertToGfxDbStyle() to fix B12581
*   ("overscan limits can be set >than max").
*   
*   Revision 36.9  91/02/08  14:31:41  eric
*   Insured that a few pointers are initialized in EditOScans()  (handleregion,
*   editmenu, editvi).  Fixes memoration bug.
*   
*   Revision 36.8  91/01/15  13:42:58  eric
*   Changed obsolete W_ tags to WA_ variants.
*   
*   Revision 36.7  90/08/01  13:06:43  peter
*   The code which was conditional on "#define MODECHOICE" (not defined)
*   has been removed.  This code would have allowed the mode-within-monitor
*   selection that we used to have when Overscan had two scrolling lists.
*   Removed some obsolete frame-drawing code.
*   I was missing some commas in the instruction text array.
*   
*   Revision 36.6  90/07/19  12:16:30  peter
*   The instructions in the edit screen now mention the menus (the keyboard
*   was already mentioned) as a way to exit.
*   
*   Revision 36.5  90/05/16  01:27:32  peter
*   The currently selected handle is now highlighted.
*   
*   Revision 36.4  90/05/15  23:43:38  peter
*   Drag-boxes now move in view-resolution units.
*   Uses PolyDraw() to draw the frame.
*   
*   Revision 36.3  90/05/15  00:36:12  peter
*   Conditionally compiled out code to handle editing of arbitrary modes.
*   Now takes its colors from the WB screen.
*   Removed code for "Show Overscans".
*   SetUpSizing() no longer takes unneeded oscantype parameter.
*   
*   Revision 36.2  90/05/08  12:02:37  peter
*   Explanatory text on edit screen is now always in topaz 8.
*   
*   Revision 36.1  90/04/18  13:49:00  peter
*   Removed comment about VGA-ExtraLores since we no longer support it
*   for editing.
*   
*   Revision 36.0  90/04/09  17:38:43  peter
*   Initial RCS check-in.
*   
*
**************************************************************************/

/*------------------------------------------------------------------------*/

#include <exec/types.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <graphics/regions.h>
#include <graphics/displayinfo.h>
#include <intuition/intuition.h>
#include <libraries/gadtools.h>
#include <string.h>

#include "overscan.h"
#include "display.h"

/*----------------------------------------------------------------------*/

#define OUT 0
#define MID 1
#define IN  2

struct Handles
{
    Point pos[3];	/* handle positions */
};

/*----------------------------------------------------------------------*/

/*  Size of handles in ticks */
#define HANDLETICKS 1024

#define TXTINSTRCOUNT 8
char *TxtInstr[] =
{
    "Use the handles to size",
    "and place your Text Overscan",
    "area.  It may be as big as",
    "you like, but the whole area",
    "should remain visible.",
    "Use menus to exit or",
    "press <ESC>ape to cancel",
    "or <ENTER> to keep changes.",
};

#define STDINSTRCOUNT 7
char *StdInstr[] =
{
    "Use the handles to size and",
    "place your Standard Overscan",
    "area.  It should fill your",
    "whole screen.",
    "Use menus to exit or",
    "press <ESC>ape to cancel",
    "or <ENTER> to keep changes.",
};

/*----------------------------------------------------------------------*/

extern Point dViewPos, ViewPos;

extern struct MonitorEntry *current;
extern struct Menu *menu;
extern struct TextFont *displayfont;

/*----------------------------------------------------------------------*/

struct Rectangle MaxOScan, StdOScan, TxtOScan;
WORD NomWidth, NomHeight, StdWidth, StdHeight;
UWORD HandleWidth, HandleHeight;
UWORD xscale, yscale;

#define IDCMPFLAGS  ( VANILLAKEY | MOUSEBUTTONS | MOUSEMOVE | MENUPICK )

struct Screen *editscr = NULL;
struct Window *editwin = NULL;
struct Region *rgn = NULL;

/*----------------------------------------------------------------------*/

BOOL EditOScans( WORD oscantype, struct Window *wbwin );
BOOL TestSizing( WORD x, WORD y );
void SetUpSizing( WORD x, WORD y );
void DoSizing( WORD x, WORD y );
void DrawRect( struct Window *win, struct Rectangle *rect );
void CalcHandles( struct Rectangle *rect, struct Handles *handles );
void DrawHandles( struct Window *win, struct Handles *handles );
void MoveHandle( struct Window *win, struct Handles *h1, struct Handles *h2 );
void PrintInstructions( struct Window *win, char *text[], int count );
void ShiftRect( struct Rectangle *rect, WORD dx, WORD dy );

void ConvertToGfxDbStyle( WORD oscantype );
void ConvertToDisplayStyle( void );
WORD modulo( WORD num, WORD div );

WORD rectWidth( struct Rectangle *rect );
WORD rectHeight( struct Rectangle *rect );

/*----------------------------------------------------------------------*/

long HandleDisplayMessage( struct Window *win,
    struct IntuiMessage *imsg, BOOL inedit );

/*----------------------------------------------------------------------*/

/*  Current and Prev are the current and previous rectangles */
struct Rectangle Current, Prev, Undo;
struct Handles CurrentHandles, PrevHandles;

/*  Delta tells us how far the mouse was from each edge when
    the select button went down: */
struct Rectangle Delta;

/*  LoLim and HiLim are the lower and higher limits for each of
    the four edges during a sizing operation: */
struct Rectangle LoLim, HiLim;

/*  horiz = 1, 2, or 3 depending on whether the user hit a left,
    middle, or right handle.  vert = 1, 2, or 3 similarly: */
WORD horiz, vert;

/*----------------------------------------------------------------------*/

#define KEEP_MENU	1
#define CANCEL_MENU	2

struct NewMenu neweditmenu[] =
{
    { NM_TITLE, " Edit ",		    0 , 0, 0, 0,},
    {  NM_ITEM, "Keep Changes",		   "K", 0, 0, ( ( void * )KEEP_MENU ),},
    {  NM_ITEM, "Quit ( Cancel Changes )", "Q", 0, 0, ( ( void * )CANCEL_MENU ),},
    {   NM_END, 0,			    0 , 0, 0, 0},
};

/* The reason we require color zero to be black is because the "unusable"
 * area of the edit screen is the area outside of MaxOScan.  Part of that
 * area is drawn in black, and part of it is drawn in pen zero, even though
 * it is outside of the bitmap.  By making pen zero black, the two regions
 * appear identical, and the user can get a better understanding of the
 * usable area.
 * Note that we invert all the rendering so things still appear to have
 * black text on a gray background.
 */
struct ColorSpec colors[] =
{
    { 0, 0, 0, 0, },	/* Color zero must be black ! */
    { 1, 10, 10, 10, },
    { ~0, 0, 0, 0, },
};

/*----------------------------------------------------------------------*/

/*/ EditOScans()
 *
 * Allow the user to drag-edit the overscan rectangles.
 *
 * Created:  15-Sep-89, Peter Cherna
 * Modified: 22-Jan-90, Peter Cherna
 *
 */

BOOL EditOScans( oscantype, wbwin )

WORD oscantype;
struct Window *wbwin;

{
    WORD terminated;
    struct IntuiMessage *msg, copymsg;
    ULONG signalmask;
    ULONG editwinmask, wbwinmask;
    ULONG signal;
    BOOL retval = FALSE;
    struct Menu *editmenu = NULL;
    void *editvi = NULL;

    /*  sizing tells us if we're in a sizing operation: */
    BOOL sizing;

    /*  Turn off menus for the window on the workbench */
    ClearMenuStrip( wbwin );

    ConvertToDisplayStyle();

    NomWidth = rectWidth( &current->me_DimensionInfo.Nominal );
    NomHeight = rectHeight( &current->me_DimensionInfo.Nominal );
    StdWidth = rectWidth( &StdOScan );
    StdHeight = rectHeight( &StdOScan );

    xscale = current->me_MonitorInfo.ViewResolution.x /
	current->me_DisplayInfo.Resolution.x;

    yscale = current->me_MonitorInfo.ViewResolution.y /
	current->me_DisplayInfo.Resolution.y;

    DP( ( "xscale = %ld, yscale = %ld\n", ( ULONG )xscale, ( ULONG )yscale ) );

    HandleWidth = HANDLETICKS/current->me_DisplayInfo.Resolution.x;
    HandleHeight = HANDLETICKS/current->me_DisplayInfo.Resolution.y;

    if ( !( editscr = OpenScreenTags( NULL,
	SA_DisplayID, current->me_ID,
	SA_Overscan, OSCAN_MAX,
	SA_ShowTitle, FALSE,
	SA_Colors, colors,
	TAG_DONE ) ) )
    {
	goto end;
    }

    if ( !( editvi = GetVisualInfo( editscr,
	TAG_DONE ) ) )
    {
	goto end;
    }

    /*  Build and layout menus: */
    if ( !( editmenu = CreateMenus( neweditmenu,
	TAG_DONE ) ) )
    {
	goto end;
    }

    if ( !LayoutMenus( editmenu, editvi,
	GTMN_NewLookMenus, TRUE,
	TAG_DONE ) )
    {
	goto end;
    }

    if ( !( editwin = OpenWindowTags( NULL,
	WA_Left, 0,
	WA_Top, 0,
	WA_Width, rectWidth( &MaxOScan ),
	WA_Height, rectHeight( &MaxOScan ),

	WA_IDCMP, IDCMPFLAGS,

	WA_Backdrop, TRUE,
	WA_ReportMouse, TRUE,
	WA_NoCareRefresh, TRUE,
	WA_Borderless, TRUE,
	WA_Activate, TRUE,
	WA_SmartRefresh, TRUE,
	WA_NewLookMenus, TRUE,

	WA_CustomScreen, editscr,
	TAG_DONE ) ) )
	
    {
	goto end;
    }

    SetMenuStrip( editwin, editmenu );

    if ( !( rgn = NewRegion() ) )
	goto end;

    SetAPen( editwin->RPort, 1 );
    SetRast( editwin->RPort, 1 );
    SetDrMd( editwin->RPort, COMPLEMENT );

    terminated = 0;
    sizing = FALSE;

    horiz = vert = 0;

    /*  LoLim and HiLim represent the low and high bounds of movement
	for each edge ( Min and Max X and Y ).  The outer limit is always
	the MaxOScan rectangle ( fixed ), but the inner limit is the nominal
	size ( which must be set dynamically ).  Here we set the outer limit: */
    LoLim.MinX = MaxOScan.MinX;
    HiLim.MaxX = MaxOScan.MaxX;
    LoLim.MinY = MaxOScan.MinY;
    HiLim.MaxY = MaxOScan.MaxY;


    if ( oscantype == GAD_EDITTXT )
    {
	/*  Set up for editing text-overscan: */
	Current = TxtOScan;
	PrintInstructions( editwin, TxtInstr, TXTINSTRCOUNT );
    }
    else if ( oscantype == GAD_EDITSTD )
    {
	/*  Set up for editing StdOScan: */
	Current = StdOScan;
	PrintInstructions( editwin, StdInstr, STDINSTRCOUNT );
    }

    CalcHandles( &Current, &CurrentHandles );
    Prev = Current;
    PrevHandles = CurrentHandles;

    /*  Draw the frame, and all handles */
    DrawRect( editwin, &Current );
    DrawHandles( editwin, &CurrentHandles );

    wbwinmask = ( 1 << wbwin->UserPort->mp_SigBit );
    editwinmask = ( 1 << editwin->UserPort->mp_SigBit );
    signalmask = ( wbwinmask | editwinmask );
    while ( !terminated )
    {
	signal = Wait( signalmask );
	if ( signal & wbwinmask )
	{
	    while ( ( !terminated ) && ( msg = GT_GetIMsg( wbwin->UserPort ) ) )
	    {
		copymsg = *msg;
		GT_ReplyIMsg( msg );

		if ( retval = HandleDisplayMessage( wbwin, &copymsg, TRUE ) )
		{
		    terminated = CANCEL_MENU;
		}
	    }
	}

	if ( signal & editwinmask )
	{
	    while ( msg = ( struct IntuiMessage * )GetMsg( editwin->UserPort ) )
	    {
		copymsg = *msg;
		ReplyMsg( ( struct Message * )msg );
		switch ( copymsg.Class )
		{
		    case MOUSEMOVE:
			/*  Only do mousemoves if we're sizing: */
			if ( sizing )
			{
			    /* Grab the most up-to-date mouse coordinates */
			    DoSizing( editwin->MouseX, editwin->MouseY );
			}
			break;

		    case MOUSEBUTTONS:
			/*  A mouse press may indicate the start of a sizing
			    operation: */
			if ( copymsg.Code == SELECTDOWN )
			{
			    if ( sizing = TestSizing( copymsg.MouseX, copymsg.MouseY ) )
			    {
				editwin->Flags |= RMBTRAP;
				Undo = Prev = Current;
				PrevHandles = CurrentHandles;
				SetUpSizing( copymsg.MouseX, copymsg.MouseY );
				/*  Mark the highlighted handle,
				    and calculate it: */
				/* Erase the handles */
				DrawHandles( editwin, &CurrentHandles );
				/* But redraw the active handle */
				MoveHandle( editwin, &CurrentHandles, NULL );
			    }
			}
			else if ( ( sizing ) && ( ( copymsg.Code == SELECTUP ) ||
			    ( copymsg.Code == MENUDOWN ) ) )
			{
			    /*  User is done sizing: */
			    editwin->Flags &= ~RMBTRAP;
			    sizing = FALSE;
			    /* Remove the active handle from the display */
			    MoveHandle( editwin, &CurrentHandles, NULL );
			    if ( copymsg.Code == MENUDOWN )
			    {
				/* We must undo.  Remove the rectangle, and put
				 * the old stuff in its place.
				 */
				DrawRect( editwin, &Current );
				Current = Undo;
				CalcHandles( &Current, &CurrentHandles );
				DrawRect( editwin, &Current );
				DrawHandles( editwin, &CurrentHandles );
			    }
			    else
			    {
				/* Restore all handles */
				DrawHandles( editwin, &CurrentHandles );
			    }

			    /*  Transfer data back to TxtOScan or StdOScan: */
			    if ( oscantype == GAD_EDITTXT )
			    {
				TxtOScan = Current;
			    }
			    else
			    {
				StdOScan = Current;
			    }
			}
			break;

		    case MENUPICK:
			if ( copymsg.Code != MENUNULL )
			{
			    terminated = ( WORD )MENU_USERDATA( ItemAddress( editmenu,
				copymsg.Code ) );
			}
			break;

		    case VANILLAKEY:
			if ( copymsg.Code == 13 )
			{
			    /*  Found <ENTER>: exit but keep changes */
			    terminated = KEEP_MENU;
			}
			else if ( copymsg.Code == 27 )
			{
			    /*  Found <ESC>: exit abandoning changes */
			    terminated = CANCEL_MENU;
			}
			break;
		}
	    }
	}
    }
end:
    if ( rgn )
    {
	DisposeRegion( rgn );
	rgn = NULL;		/* Just to be safe... */
    }

    if ( editwin )
    {
	ClearMenuStrip( editwin );
	CloseWindow( editwin );
	editwin = NULL;
    }

    /*  It is safe to call both FreeMenus() and FreeVisualInfo() with
	NULL parameters, so no checking is needed: */

    FreeMenus( editmenu );
    FreeVisualInfo( editvi );

    if ( editscr )
    {
	CloseScreen( editscr );
	editscr = NULL;
    }

    if ( terminated == KEEP_MENU )
    {
	ConvertToGfxDbStyle( oscantype );
    }

    /*  Restore menus on the workbench window: */
    SetMenuStrip( wbwin, menu );

    return( retval );
}


/*----------------------------------------------------------------------*/

/*/ TestSizing()
 *
 * Test if a mouse pick was on a handle.  Return TRUE if so.
 * Also sets globals horiz and vert.
 *
 * Created:   4-Jan-90, Peter Cherna ( from EditOScans() )
 * Modified:  4-Jan-90, Peter Cherna
 *
 */

BOOL TestSizing( x, y )

WORD x, y;

{
    WORD midh, midv;

    /*  User pressed the button, so maybe we should
	begin sizing... */
    midh = ( Current.MinX + Current.MaxX - HandleWidth ) >> 1;
    midv = ( Current.MinY + Current.MaxY - HandleHeight ) >> 1;

    /*  Test if handle was hit, and identify which */
    if ( ( x >= Current.MinX ) && ( x <= Current.MinX + HandleWidth ) )
    {
	horiz = 1;
    }
    else if ( ( x >= midh ) && ( x <= midh + HandleWidth ) )
    {
	horiz = 2;
    }
    else if ( ( x >= Current.MaxX - HandleWidth ) && ( x <= Current.MaxX ) )
    {
	horiz = 3;
    }
    else
	horiz = 0;

    if ( ( y >= Current.MinY ) && ( y <= Current.MinY + HandleHeight ) )
    {
	vert = 1;
    }
    else if ( ( y >= midv ) && ( y <= midv + HandleHeight ) )
    {
	vert = 2;
    }
    else if ( ( y >= Current.MaxY - HandleHeight ) && ( y <= Current.MaxY ) )
    {
	vert = 3;
    }
    else
	vert = 0;

    /*  If we hit a handle, go into sizing mode: */
    return( ( BOOL )( ( horiz != 0 ) && ( vert != 0 ) ) );
}


/*----------------------------------------------------------------------*/

/*/ SetUpSizing()
 *
 * Calculate movement bounding limits and other parameters for
 * the sizing operation.
 * Affects globals Delta, LoLim, and HiLim.
 *
 * Created:   4-Jan-90, Peter Cherna ( from EditOScans() )
 * Modified: 15-May-90, Peter Cherna
 *
 */

void SetUpSizing( x, y )

WORD x, y;

{
    /*  Round off X and Y to view units: */
    MP( ( "SUS:  ( %ld,%ld ) -> ", ( LONG )x, ( LONG )y ) );
    x -= modulo( x, xscale );
    y -= modulo( y, yscale );
    MP( ( "( %ld,%ld )\n", ( LONG )x, ( LONG )y ) );

    /*  Note offset of mouse from each edge, so we can easily go from
	mouse coordinates back to frame coordinates: */
    Delta.MinX = Current.MinX - x;
    Delta.MaxX = Current.MaxX - x;
    Delta.MinY = Current.MinY - y;
    Delta.MaxY = Current.MaxY - y;

    if ( ( horiz == 2 ) && ( vert == 2 ) )
    {
	/*  We are MOVING the rectangle. 
	    Here, our limits are simply the MaxOScan rectangle. */
	LoLim.MaxX = MaxOScan.MinX;
	HiLim.MinX = MaxOScan.MaxX;
	LoLim.MaxY = MaxOScan.MinY;
	HiLim.MinY = MaxOScan.MaxY;
    }
    else
    {
	/*  We are SIZING a rectangle, which can't be smaller
	    than the Nominal size or bigger than the MaxOScan rect.
	    If the user sizes it large, it will stretch
	    ( or move? ) the other ( StdOScan or TxtOScan ) rect
	    as needed. */
	/*  Inner limit is the Nominal Size: */
	HiLim.MinX = Current.MaxX - NomWidth + 1;
	LoLim.MaxX = Current.MinX + NomWidth - 1;
	HiLim.MinY = Current.MaxY - NomHeight + 1;
	LoLim.MaxY = Current.MinY + NomHeight - 1;
    }
}


/*----------------------------------------------------------------------*/

/*/ DoSizing()
 *
 * Function to handle a MOUSEMOVE event to location x, y, representing
 * the sizing or moving of a frame.
 * Moves the 'Current' rectangle accordingly, with limit checking.
 * Affects globals Current, Prev.
 *
 * Created:   4-Jan-90, Peter Cherna ( from EditOScans() )
 * Modified: 15-May-90, Peter Cherna
 *
 */

void DoSizing( x, y )

WORD x, y;

{
    /*  Round off X and Y to view units: */
    MP( ( "DS:  ( %ld,%ld ) -> ", ( LONG )x, ( LONG )y ) );
    x -= modulo( x, xscale );
    y -= modulo( y, yscale );
    MP( ( "( %ld,%ld )\n", ( LONG )x, ( LONG )y ) );

    /*  Act depending on which handle is being held: */
    if ( horiz == 1 )
    {
	/*  A left-side handle: */
	Current.MinX = x + Delta.MinX;
	if ( Current.MinX < LoLim.MinX )
	{
	    Current.MinX = LoLim.MinX;
	}
	else if ( Current.MinX > HiLim.MinX )
	{
	    Current.MinX = HiLim.MinX;
	}
    }
    else if ( horiz == 3 )
    {
	/*  A right-side handle: */
	Current.MaxX = x + Delta.MaxX;
	if ( Current.MaxX < LoLim.MaxX )
	{
	    Current.MaxX = LoLim.MaxX;
	}
	else if ( Current.MaxX > HiLim.MaxX )
	{
	    Current.MaxX = HiLim.MaxX;
	}
    }
    if ( vert == 1 )
    {
	/*  A top-edge handle: */
	Current.MinY = y + Delta.MinY;
	if ( Current.MinY < LoLim.MinY )
	{
	    Current.MinY = LoLim.MinY;
	}
	else if ( Current.MinY > HiLim.MinY )
	{
	    Current.MinY = HiLim.MinY;
	}
    }
    else if ( vert == 3 )
    {
	/*  A bottom-edge handle: */
	Current.MaxY = y + Delta.MaxY;
	if ( Current.MaxY < LoLim.MaxY )
	{
	    Current.MaxY = LoLim.MaxY;
	}
	else if ( Current.MaxY > HiLim.MaxY )
	{
	    Current.MaxY = HiLim.MaxY;
	}
    }
    else if ( ( vert == 2 ) && ( horiz == 2 ) )
    {
	/*  The dead-center handle needs different
	    treatment, since we move, not size: */
	Current.MinX = x + Delta.MinX;
	Current.MaxX = x + Delta.MaxX;
	Current.MinY = y + Delta.MinY;
	Current.MaxY = y + Delta.MaxY;
	/*  Check for violation of limits.  Note that if we're
	    moving the TxtOScan rect, we don't want to check
	    the inner limits, since the inner limit is one of
	    size, not place: */
	if ( Current.MinX < LoLim.MinX )
	{
	    Current.MaxX += LoLim.MinX - Current.MinX;
	    Current.MinX = LoLim.MinX;
	}
	else if ( Current.MinX > HiLim.MinX )
	{
	    Current.MaxX -= Current.MinX - HiLim.MinX;
	    Current.MinX = HiLim.MinX;
	}
	if ( Current.MaxX < LoLim.MaxX )
	{
	    Current.MinX += LoLim.MaxX - Current.MaxX;
	    Current.MaxX = LoLim.MaxX;
	}
	else if ( Current.MaxX > HiLim.MaxX )
	{
	    Current.MinX -= Current.MaxX - HiLim.MaxX;
	    Current.MaxX = HiLim.MaxX;
	}

	if ( Current.MinY < LoLim.MinY )
	{
	    Current.MaxY += LoLim.MinY - Current.MinY;
	    Current.MinY = LoLim.MinY;
	}
	else if ( Current.MinY > HiLim.MinY )
	{
	    Current.MaxY -= Current.MinY - HiLim.MinY;
	    Current.MinY = HiLim.MinY;
	}
	if ( Current.MaxY < LoLim.MaxY )
	{
	    Current.MinY += LoLim.MaxY - Current.MaxY;
	    Current.MaxY = LoLim.MaxY;
	}
	else if ( Current.MaxY > HiLim.MaxY )
	{
	    Current.MinY -= Current.MaxY - HiLim.MaxY;
	    Current.MaxY = HiLim.MaxY;
	}
    }
    /*  Only do graphics if the rect changed: */
    if ( ( Current.MinY != Prev.MinY ) || ( Current.MaxX != Prev.MaxX ) ||
	( Current.MinX != Prev.MinX ) || ( Current.MaxY != Prev.MaxY ) )
    {
	/*  Draw new, erase old ( magic of XOR's ): */
	CalcHandles( &Current, &CurrentHandles );
	DrawRect( editwin, &Current );
	DrawRect( editwin, &Prev );
	MoveHandle( editwin, &CurrentHandles, &PrevHandles );
	Prev = Current;
	PrevHandles = CurrentHandles;
    }
}


/*----------------------------------------------------------------------*/

void DrawRect( win, rect )

struct Window *win;
struct Rectangle *rect;

{
    static WORD array[8];

    Move( win->RPort, rect->MinX, rect->MinY );

    /* This array describes the rectangle to PolyDraw: */
    array[0] = array[2] = rect->MaxX;
    array[1] = array[7] = rect->MinY;
    array[3] = array[5] = rect->MaxY;
    array[4] = array[6] = rect->MinX;
    array[7]++;

    PolyDraw( win->RPort, 4, array );
}

/*----------------------------------------------------------------------*/

/*/ CalcHandles()
 *
 * Calculate the coordinates of the handles of a given rectangle.
 *
 */

void CalcHandles( rect, handles )

struct Rectangle *rect;
struct Handles *handles;

{
    /* First, we calculate three Points.  The one called OUT is
     * the upper-left point just inside the framing rectangle
     */
    handles->pos[OUT].x = rect->MinX+1;
    handles->pos[OUT].y = rect->MinY+1;

    /* The second Point MID, which holds the upper-left coordinate
     * of the center handle.
     */
    handles->pos[MID].x = ( rect->MinX + rect->MaxX - HandleWidth ) >> 1;
    handles->pos[MID].y = ( rect->MinY + rect->MaxY - HandleHeight ) >> 1;

    /* The third Point, IN, holds the upper-left coordinate just inside
     * the lower-right handle
     */
    handles->pos[IN].x = rect->MaxX-HandleWidth;
    handles->pos[IN].y = rect->MaxY-HandleHeight;

    /* The upper-left coordinates of the nine handles can be derived
     * from combinations of the three points.
     */
}

/*----------------------------------------------------------------------*/

/*/ DrawHandles()
 *
 * Draws a filled rectangle of specified dimensions.
 *
 */

void DrawHandles( win, handles )

struct Window *win;
struct Handles *handles;

{
    ULONG row, col;
    WORD left, top;

    for ( row = OUT; row <= IN; row++ )
    {
	for ( col = OUT; col <= IN; col++ )
	{
	    left = handles->pos[row].x;
	    top = handles->pos[col].y;
	    RectFill( win->RPort, left, top,
		left+HandleWidth-1, top+HandleHeight-1 );
	}
    }

}

/*----------------------------------------------------------------------*/

/*/ MoveHandle()
 *
 * Given one or two Handles structures (h2 may be NULL), draw the
 * active handle as indicated by globals 'horiz' and 'vert'.
 *
 */

void MoveHandle( win, h1, h2 )

struct Window *win;
struct Handles *h1;
struct Handles *h2;

{
    struct Rectangle rect;
    WORD left, top;
    struct RegionRectangle *rrect;

    ClearRegion( rgn );
    rect.MinX = h1->pos[horiz-1].x;
    rect.MinY = h1->pos[vert-1].y;
    rect.MaxX = rect.MinX+HandleWidth-1;
    rect.MaxY = rect.MinY+HandleHeight-1;

    OrRectRegion( rgn, &rect );

    if ( h2 )
    {
	rect.MinX = h2->pos[horiz-1].x;
	rect.MinY = h2->pos[vert-1].y;
	rect.MaxX = rect.MinX+HandleWidth-1;
	rect.MaxY = rect.MinY+HandleHeight-1;

	XorRectRegion( rgn, &rect );
    }

    left = rgn->bounds.MinX;
    top = rgn->bounds.MinY;
    rrect = rgn->RegionRectangle;
    while (rrect)
    {
	RectFill( win->RPort,
	( left+rrect->bounds.MinX ), ( top+rrect->bounds.MinY ),
	( left+rrect->bounds.MaxX ), ( top+rrect->bounds.MaxY ) );
	rrect = rrect->Next;
    }
}

/*----------------------------------------------------------------------*/

#define INTERLINESPACING 10

/*/ PrintInstructions()
 *
 * Renders a brief instruction on what to do into this screen.
 *
 */

void PrintInstructions( win, text, count )

struct Window *win;
char *text[];
int count;

{
    WORD x, y, line, length;

    /*  Put text centered, 1/4 way down the screen: */
    SetFont( win->RPort, displayfont );
    y = ( win->Height - ( count * INTERLINESPACING ) ) / 4;
    x = win->Width / 2;

    for ( line = 0; line < count; line++ )
    {
	length = TextLength( win->RPort, text[line], ( WORD )strlen( text[line] ) );
	Move( win->RPort, ( WORD )( x - length/2 ), y );
	Text( win->RPort, text[line], ( WORD )strlen( text[line] ) );
	y += INTERLINESPACING;
    }
}


/*----------------------------------------------------------------------*/

/*/ ShiftRect()
 *
 * Shift a rectangle by a specified dx and dy.
 *
 */

void ShiftRect( rect, dx, dy )

struct Rectangle *rect;
WORD dx, dy;

{
    rect->MinX += dx;
    rect->MaxX += dx;
    rect->MinY += dy;
    rect->MaxY += dy;
}


/*----------------------------------------------------------------------*/

/*/ ConvertToDisplayStyle()
 *
 * We have all the rectangles expressed as they come from the database,
 * in which the upper-left of TxtOScan is nailed to ( 0,0 ), and the
 * others are relative to that.  We want it all relative to the
 * upper-left of MaxOScan nailed at ( 0,0 ).
 *
 */

void ConvertToDisplayStyle()

{
    WORD x, y;

    x = -current->me_DimensionInfo.MaxOScan.MinX;
    y = -current->me_DimensionInfo.MaxOScan.MinY;

    /*  Convert to window coordinates, where top left is ( 0,0 ),
	not ( MaxOScan.MinX, MaxOScan.MinY ): */

    StdOScan = current->me_DimensionInfo.StdOScan;
    TxtOScan = current->me_DimensionInfo.TxtOScan;
    MaxOScan = current->me_DimensionInfo.MaxOScan;

    DP( ( "\nValues upon entry:\n" ) );
    DRECT( "TxtOScan", TxtOScan );
    DRECT( "StdOScan", StdOScan );
    DRECT( "MaxOScan", MaxOScan );
    DP( ( "ViewPos: [%ld,%ld]\n", ( LONG )ViewPos.x, ( LONG )ViewPos.y ) );

    ShiftRect( &MaxOScan, x, y );
    ShiftRect( &StdOScan, x, y );
    ShiftRect( &TxtOScan, x, y );

    DP( ( "\nScreen relative values upon entry:\n" ) );
    DRECT( "TxtOScan", TxtOScan );
    DRECT( "StdOScan", StdOScan );
    DRECT( "MaxOScan", MaxOScan );
}


/*----------------------------------------------------------------------*/

/*/ ConvertToGfxDbStyle()
 *
 * Converts the user's changes to numbers in the style of the graphics
 * database, for all modes of the same MonitorEntry ( virtual monitor ).
 * Now that the user has finished editing the rectangles, we have the
 * new numbers in screen-relative coordinates, in which the upper left
 * of the MaxOScan rectangle is nailed at ( 0,0 ).  We've got to convert
 * these rectangles to Graphics-Database format, in which the TxtOScan
 * has upper-left nailed at ( 0,0 ).  Of course, we have to do scaling
 * between different modes of the same virtual monitor, too.
 *
 */

void ConvertToGfxDbStyle( oscantype )

WORD oscantype;

{
    WORD TxtWidth, TxtHeight, StdWidth, StdHeight;
    Point shift, size;

    DP( ( "\nScreen relative values:\n" ) );
    DRECT( "TxtOScan", TxtOScan );
    DRECT( "StdOScan", StdOScan );
    DRECT( "MaxOScan", MaxOScan );

    TxtWidth = rectWidth( &TxtOScan );
    TxtHeight = rectHeight( &TxtOScan );
    StdWidth = rectWidth( &StdOScan );
    StdHeight = rectHeight( &StdOScan );
    DP( ( "TxtSize ( %ld,%ld )\n", TxtWidth, TxtHeight ) );
    DP( ( "StdSize ( %ld,%ld )\n", StdWidth, StdHeight ) );

    /*  TxtOScan must be no bigger than StdOScan.  Let size measure
	this excess, or zero if ok: */
    if ( ( size.x = ( TxtWidth - StdWidth ) ) > 0 )
    {
    }
    else
	size.x = 0;

    if ( ( size.y = ( TxtHeight - StdHeight ) ) > 0 )
    {
    }
    else
	size.y = 0;

    /*  Whichever rectangle we edit, it is the OTHER one which
	must be "bullied", i.e. it must get bigger/smaller: */
    if ( oscantype == GAD_EDITSTD )
    {
	DP( ( "TxtOScan too big by <%ld,%ld>\n", ( LONG )size.x, ( LONG )size.y ) );
	TxtOScan.MaxX -= size.x;
	TxtOScan.MaxY -= size.y;
    }
    else /* oscantype == GAD_EDITTXT */
    {
	DP( ( "StdOScan too small by <%ld,%ld>\n", ( LONG )size.x, ( LONG )size.y ) );
	StdOScan.MaxX += size.x;
	StdOScan.MaxY += size.y;
    }

    /*  TxtOScan must remain inside StdOScan.  Let shift measure
	the offset needed to make it so.  A positive offset indicates
	that TxtOScan must be increased ( or StdOScan decreased ), and
	vice versa: */
    if ( ( shift.x = ( StdOScan.MinX - TxtOScan.MinX ) ) > 0 )
    {
    }
    else if ( ( shift.x = ( StdOScan.MaxX - TxtOScan.MaxX ) ) < 0 )
    {
    }
    else
	shift.x = 0;

    if ( ( shift.y = ( StdOScan.MinY - TxtOScan.MinY ) ) > 0 )
    {
    }
    else if ( ( shift.y = ( StdOScan.MaxY - TxtOScan.MaxY ) ) < 0 )
    {
    }
    else
	shift.y = 0;


    /*  Whichever rectangle we edit, it is the OTHER one which
	must be "bullied", i.e. it must move: */
    if ( oscantype == GAD_EDITSTD )
    {
	DP( ( "TxtOScan must move by <%ld,%ld>\n", ( LONG )shift.x, ( LONG )shift.y ) );
	ShiftRect( &TxtOScan, shift.x, shift.y );
    }
    else /* oscantype == GAD_EDITTXT */
    {
	DP( ( "StdOScan must move by <%ld,%ld>\n", ( LONG )-shift.x, ( LONG )-shift.y ) );
	ShiftRect( &StdOScan, ( WORD )( -shift.x ), ( WORD )( -shift.y ) );
    }

    /*  By this point, we have legal TxtOScan and StdOScan
	rectangles ( S contains T ) in display-relative coordinates. */


    /*  We know that the ViewPosition added to the MaxOScan upper
	limit ( in ViewResolution units ) is the dViewPos constant
	( calculated earlier ).  However, the rectangles are
	MaxOScan-relative, and not yet TxtOScan-relative, so we
	should use -TxtOScan.Min in place of MaxOScan.Min.
	Use this to figure out the ViewPosition: */

    DP( ( "Resolution: <%ld,%ld>\n",
	( LONG )current->me_DisplayInfo.Resolution.x,
	( LONG )current->me_DisplayInfo.Resolution.y ) );
    DP( ( "View Resolution: <%ld,%ld>\n",
	( LONG )current->me_MonitorInfo.ViewResolution.x,
	( LONG )current->me_MonitorInfo.ViewResolution.y ) );

    ViewPos.x = dViewPos.x + ( ( ( LONG )TxtOScan.MinX ) *
	current->me_DisplayInfo.Resolution.x ) /
	current->me_MonitorInfo.ViewResolution.x;

    ViewPos.y = dViewPos.y + ( ( ( LONG )TxtOScan.MinY ) *
	current->me_DisplayInfo.Resolution.y ) /
	current->me_MonitorInfo.ViewResolution.y;

    DP( ( "ViewPos: [%ld,%ld]\n", ( LONG )ViewPos.x, ( LONG )ViewPos.y ) );

    /*  Since the ViewPos can only be measured in integral units
	of ViewResolution, it's possible to have a few pixels rounded
	off here.  The result will be"that the StdOScan and TxtOScan
	rectangles will be shifted left ( or up ) from what the user
	intended, by the amount needed to achieve this granularity.
	However, the MaxOScan rectangle is not free to move by this
	small amount ( being fixed in position, but always described
	as ViewPos-relative ), so we have to account for this rounding.  */

    /*  Measure how much the ViewPosition shifted, in mode resolution: */
    shift.x = ( ( current->me_MonitorInfo.ViewPosition.x - ViewPos.x ) *
	current->me_MonitorInfo.ViewResolution.x ) /
	current->me_DisplayInfo.Resolution.x;
    shift.y = ( ( current->me_MonitorInfo.ViewPosition.y - ViewPos.y ) *
	current->me_MonitorInfo.ViewResolution.y ) /
	current->me_DisplayInfo.Resolution.y;

    /*  Shift MaxOScan accordingly: */
    ShiftRect( &current->me_DimensionInfo.MaxOScan, shift.x, shift.y );

    /*  Throw the TxtOScan and StdOScan rectangles back into
	TxtOScan-relative coordinates: */
    shift.x = -TxtOScan.MinX;
    shift.y = -TxtOScan.MinY;
    ShiftRect( &TxtOScan, shift.x, shift.y );
    ShiftRect( &StdOScan, shift.x, shift.y );
    current->me_DimensionInfo.TxtOScan = TxtOScan;
    current->me_DimensionInfo.StdOScan = StdOScan;

    current->me_MonitorInfo.ViewPosition = ViewPos;

    DP( ( "After shifting and sliding:\n" ) );
    DRECT( "TxtOScan", current->me_DimensionInfo.TxtOScan );
    DRECT( "StdOScan", current->me_DimensionInfo.StdOScan );
    DRECT( "MaxOScan", current->me_DimensionInfo.MaxOScan );
}


/*----------------------------------------------------------------------*/

/*/ modulo()
 *
 * Return the true modulo of a pair of numbers, even for negative numbers.
 * For example, modulo( -1, div ) = ( div-1 ), not -1.
 *
 * Created:  15-May-90, Peter Cherna
 * Modified: 15-May-90, Peter Cherna
 *
 */

WORD modulo( num, div )

WORD num;
WORD div;

{
    WORD result;

    result = num % div;

    if ( result < 0 )
    {
	result += div;
    }
    return( result );
}

/*----------------------------------------------------------------------*/
