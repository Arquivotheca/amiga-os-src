
/* ***************************************************************************
 * 
 * This files contains the routines that render the various Zaphod windows
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY	Name	    Description
 * -----------	----------  -------------------------------------------------
 * 23 Feb 86	=RJ Mical=  Created this file
 *
 * **************************************************************************/

#include "zaphod.h"


RenderWindow(display, recalc, stuffTextBuffer, redrawWholeDisplay)
struct Display *display;
BOOL recalc, stuffTextBuffer, redrawWholeDisplay;
/* This routine calls RenderWindowGrunt() with arguments describing that
 * a rendering of the entire display should be done. 
 * Please see RenderWindowGrunt() for a description of this function and
 * its arguments.
 */
{
    RenderWindowGrunt(display, recalc, stuffTextBuffer, redrawWholeDisplay, 
	    -1, -1);
}



RenderWindowGrunt(display, recalc, stuffTextBuffer, redrawWholeDisplay,
	startRow, endRow)
struct Display *display;
BOOL recalc, stuffTextBuffer, redrawWholeDisplay;
SHORT startRow, endRow;
/* Render the window anew.
 * If recalc is ...
 *	- TRUE:
 *	  The display size has changed.  Therefore, the display parameters
 *	  should be recalculated before the display is rendered.
 *	- FALSE:
 *	  The display has not changed since the last render, or the
 *	  parameters have already been recalculated, so leave
 *	  the display parameters as they stand.
 * If stuffTextBuffer is ...
 *	- TRUE:     
 *	  - Text Mode	  
 *	    If we're in text mode, then go and stuff the text buffer 
 *	    from the PC buffer.  This means further that we want to force 
 *	    the entire text display to be refreshed.  To achieve this, the
 *	    NewStartCol and NewLength arrays won't be recalculated and 
 *	    instead will be set equal to the entire text width, 
 *	    which will force all of the buffer to be drawn.  When
 *	    stuffTextBuffer is TRUE recalc (above) should be TRUE too.
 *	  - Graphics Mode
 *	    if we're in graphics mode, then blast the PC memory into 
 *	    the Buffer whether or not stuffTextBuffer is set.
 *	- FALSE:
 *	  If we're in text mode, then FALSE means that the text buffer
 *	  is still physically synchronized with the PC memory, even though
 *	  the information content of the two buffers may have changed.
 *	  So don't get a whole new copy of PC memory, and instead just make
 *	  the comparisons based on the current state of things.  The
 *	  NewStartCol and NewLengths arrays will be calculated
 *	  based on the current states of the buffers.  If we're in graphics
 *	  mode, the PC memory is blasted into the Buffer regardless
 *	  of the setting of stuffTextBuffer, because the blast is so fast
 *	  (less than 10 milliseconds under normal high-res conditions, around
 *	  35 milliseconds under medium-res).
 * If redrawWholeDisplay is ...
 *	- TRUE:
 *	  We want to draw into any area of the entire display (excluding 
 *	  borders) which is the usual case.
 *	- FALSE:
 *	  The display doesn't need to be redrawn, only refreshed.  This 
 *	  is the case when, for instance, we get a REFRESHWINDOW event 
 *	  through the IDCMP, specifying that only part of the window
 *	  need be refreshed.
 * startRow = the starting row for the rendering.
 * endRow = the last row to be rendered.	 
 *
 * NOTE:  please be advised that even as you read this, users of the 
 * MoveAndSetRegion() function are being told that this routine ends 
 * with an EndUpdate(), which clears away the region created by the 
 * MoveAndSetRegion() call.
 */
{
    SHORT row, col;

    Lock(&display->DisplayLock);

    SetMaskAndLock(display, redrawWholeDisplay);

    if (recalc) RecalcDisplayParms(display);
    if (startRow < 0)
	{
	startRow = 0;
	endRow = display->TextHeight - 1;
	} 

    if (display->Modes & TEXT_MODE)
	{
	if (stuffTextBuffer) StuffTextBuffer(display, startRow, endRow);
	RenderText(display, stuffTextBuffer, startRow, endRow);

	/* Now, check if the cursor was overwritten and clear the cursor
	 * flag if it was.
	 */
	if (display->CursorFlags & CURSOR_ON)
	    {
	    row = display->CursorOldRow - display->TextStartRow;
	    if ((row >= 0) || (row < display->TextHeight))
		{
		col = display->CursorOldCol - display->TextStartCol;
		if ((col < 0) || (col >= display->TextWidth))
		    ClearFlag(display->CursorFlags, CURSOR_ON);
		else
		    {
		    if ((col >= 0) && (col < display->TextNewLength[row]))
			ClearFlag(display->CursorFlags, CURSOR_ON);
		    }
		}
	    else ClearFlag(display->CursorFlags, CURSOR_ON);
	    }
	}
    else
	{
#ifdef DIAG
	kprintf("Calling Blast and Render functions\n");
	kprintf("Display struct = %lx\n",display);
#endif
	BlastGraphicsPlanes(display);
	RenderGraphics(display);
	}
    UnmaskUnlock(display);

    Unlock(&display->DisplayLock);
}


RenderText(display, stuffTextBuffer, startRow, endRow)
struct Display *display;
BOOL stuffTextBuffer;
SHORT startRow, endRow;
{
    SHORT i;

    if (stuffTextBuffer)
	{
	/* To force the refresh, simply set the NewLengths array to
	 * indicate that all lines need complete refreshing
	 */	 
	for (i = startRow; i <= endRow; i++)
	    display->TextNewLength[i] = display->TextWidth;
	}
    else RecalcNewLengths(display, startRow, endRow);

    DrawText(display);
}


RenderGraphics(display)
struct Display *display;
{
    struct Window *window;

    window = display->FirstWindow->DisplayWindow;

    BltBitMapRastPort(
	    &display->BufferBitMap, 
	    display->DisplayStartCol, display->DisplayStartRow, 
	    window->RPort, 
	    window->BorderLeft,
	    window->BorderTop,
	    display->DisplayWidth, display->DisplayHeight, 
	    0x00C0);   /* Copies the planes to the window */
}
		     


RefreshDisplay(display)
struct Display *display;
{
    if (display->Modes & BORDER_VISIBLE)
	{
	RefreshWindowFrame(display->FirstWindow->DisplayWindow);
	TopRightLines(display->FirstWindow->DisplayWindow);
	}

    RenderWindow(display, FALSE, TRUE, TRUE);
}



/*??? Someday this should be called from crt.c too */
RevampDisplay(display)
struct Display *display;
{
    struct MsgPort *saveport;
    ULONG saveidcmp;
    struct Window *window;

    Forbid();
    if (display->FirstWindow)
	{
	if (window = display->FirstWindow->DisplayWindow)
	    {
	    saveport = window->UserPort;
	    saveidcmp = window->IDCMPFlags;
	    if ((display->Modes & BORDER_VISIBLE) == 0)
		{
		BorderPatrol(display, BORDER_ON, FALSE);
		ClearFlag(display->Modes, BORDER_VISIBLE);
		}
	    SuperCloseWindow(display);
				  
	    /* Do we need to redo the screen? */
	    if (display->FirstWindow->DisplayScreen)
		SuperCloseScreen(display);

	    OpenDisplay(display);
	    OpenDisplayWindow(display);
	    window = display->FirstWindow->DisplayWindow;
	    window->UserPort = saveport;
	    ModifyIDCMP(window, saveidcmp);
	    }
	}    

    Permit();
}




