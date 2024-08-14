/*
 * makevisible.c - shows off SPOS_MAKEVISIBLE feature
 *
 * (c) Copyright 1992 Commodore-Amiga, Inc.  All rights reserved.
 *
 * This software is provided as-is and is subject to change; no warranties
 * are made.  All use is at your own risk.  No liability or responsibility
 * is assumed.
 * 
 * Opens an oversized autoscrolling screen.  Use the mouse to draw
 * a rectangle on the screen.  Then, use the mouse to scroll the screen
 * anywhere you like.  Press any key to move the drawn rectangle
 * into view using ScreenPosition( sc, SPOS_MAKEVISIBLE, ... ).
 *
 * The SPOS_MAKEVISIBLE feature can be useful to ensure certain areas
 * are visible, for example the cursor of a word-processor.
 *
 * Press "Q" or <Esc> to exit.
 *
 */

/*----------------------------------------------------------------------*/

#include <exec/types.h>
#include <graphics/displayinfo.h>
#include <graphics/gfxmacros.h>
#include <intuition/intuition.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>

/*----------------------------------------------------------------------*/

void error_exit(STRPTR errorstring);

/*----------------------------------------------------------------------*/

struct GfxBase *GfxBase = NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct Screen *myscreen = NULL;
struct Window *mywindow = NULL;

/*----------------------------------------------------------------------*/

UWORD pens[] =
{
	0, /* DETAILPEN */
	1, /* BLOCKPEN	*/
	1, /* TEXTPEN	*/
	2, /* SHINEPEN	*/
	1, /* SHADOWPEN	*/
	3, /* FILLPEN	*/
	2, /* FILLTEXTPEN	*/
	0, /* BACKGROUNDPEN	*/
	2, /* HIGHLIGHTTEXTPEN	*/

	1, /* BARDETAILPEN	*/
	2, /* BARBLOCKPEN	*/
	1, /* BARTRIMPEN	*/

	~0,
};

/*----------------------------------------------------------------------*/

/*  MYDISPLAYID represents the desired mode, and should be taken from
    graphics/displayinfo.h */
#define MYDISPLAYID	(LORES_KEY)
#define MYWIDTH	900
#define MYHEIGHT 600

/*----------------------------------------------------------------------*/

main()
{
    BOOL terminated = FALSE;
    BOOL dragging = FALSE;
    struct IntuiMessage *imsg;
    struct Rectangle drawn_rect, drag_rect;

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

    if (!(myscreen = OpenScreenTags(NULL,
	SA_DisplayID, MYDISPLAYID,
	SA_Overscan, OSCAN_TEXT,
	/*  Other tags can go here: */
	SA_Width, MYWIDTH,
	SA_Height, MYHEIGHT,
	SA_Depth, 2,
	SA_AutoScroll, 1,
	SA_Pens, pens,
	SA_Title, "Draw a rectangle with the mouse.  Scroll it off-screen.  Hit a key to scroll it into view.  <Esc> to quit.",
	TAG_DONE )))
    {
	error_exit("Couldn't open screen\n");
    }
    if (!( mywindow = OpenWindowTags(NULL,
	WA_Borderless, TRUE,
	WA_Backdrop, TRUE,
	WA_IDCMP, MOUSEBUTTONS|VANILLAKEY,
	WA_NoCareRefresh, TRUE,
	WA_Activate, TRUE,
	WA_SmartRefresh, TRUE,
	WA_CustomScreen, myscreen,
	TAG_DONE ) ))
    {
	error_exit("Couldn't open window\n");
    }

    drawn_rect.MinX = 20;
    drawn_rect.MinY = 20;
    drawn_rect.MaxX = 150;
    drawn_rect.MaxY = 100;
    SetABPenDrMd( mywindow->RPort, 3, 0, COMPLEMENT );
    SetOPen( mywindow->RPort, 1 );
    RectFill( mywindow->RPort, drawn_rect.MinX, drawn_rect.MinY,
	drawn_rect.MaxX, drawn_rect.MaxY );

    while ( !terminated )
    {
	Wait( 1 << mywindow->UserPort->mp_SigBit );
	while ( imsg = (struct IntuiMessage *)GetMsg( mywindow->UserPort ) )
	{
	    switch ( imsg->Class )
	    {
	    case VANILLAKEY:
		if ( ( imsg->Code == 0x1B ) || ( imsg->Code == 'q' ) || ( imsg->Code == 'Q' ) )
		{
		    terminated = TRUE;
		}
		else
		{
		    ScreenPosition( myscreen, SPOS_MAKEVISIBLE,
			drawn_rect.MinX, drawn_rect.MinY,
			drawn_rect.MaxX, drawn_rect.MaxY );
		}
		break;

	    case MOUSEBUTTONS:
		{
		    WORD mousex = imsg->MouseX;
		    WORD mousey = imsg->MouseY;

		    if ( mousex < 0 ) mousex = 0;
		    if ( mousex >= myscreen->Width ) mousex = myscreen->Width - 1;
		    if ( mousey < 0 ) mousey = 0;
		    if ( mousey >= myscreen->Height ) mousey = myscreen->Height - 1;

		    if ( imsg->Code == SELECTDOWN )
		    {
			dragging = TRUE;
			drag_rect.MinX = mousex;
			drag_rect.MinY = mousey;
		    }
		    else if ( ( imsg->Code == SELECTUP ) && ( dragging ) )
		    {
			dragging = FALSE;
			if ( mousex > drag_rect.MinX )
			{
			    drag_rect.MaxX = mousex;
			}
			else
			{
			    drag_rect.MaxX = drag_rect.MinX;
			    drag_rect.MinX = mousex;
			}
			if ( mousey > drag_rect.MinY )
			{
			    drag_rect.MaxY = mousey;
			}
			else
			{
			    drag_rect.MaxY = drag_rect.MinY;
			    drag_rect.MinY = mousey;
			}
			SetOPen( mywindow->RPort, 0 );
			RectFill( mywindow->RPort, drawn_rect.MinX, drawn_rect.MinY,
			    drawn_rect.MaxX, drawn_rect.MaxY );
			drawn_rect = drag_rect;
			SetOPen( mywindow->RPort, 1 );
			RectFill( mywindow->RPort, drawn_rect.MinX, drawn_rect.MinY,
			    drawn_rect.MaxX, drawn_rect.MaxY );
		    }
		}
		break;
	    }
	    ReplyMsg( imsg );
	}
    }
    error_exit(NULL);
}


/*----------------------------------------------------------------------*/

void error_exit(STRPTR errorstring)

    {
    if (mywindow)
	{
	CloseWindow(mywindow);
	}

    if (myscreen)
	{
	CloseScreen(myscreen);
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
