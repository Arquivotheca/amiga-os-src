
/* ***************************************************************************
 * 
 * The Region Creation and Manipulation Routines for the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY	Name	    Description
 * -----------	----------  -------------------------------------------------
 * 16 Feb 86	=RJ Mical=  Burst these routines out of the bulky render.c
 *
 * **************************************************************************/

#include "zaphod.h"


SetMaskAndLock(display, redrawWholeWindow)
struct Display *display;	    
BOOL redrawWholeWindow;
/* This routine creates a damage list in the given window, which list 
 * excludes any window border area.  After calling this routine, you
 * can draw to the window to your little heart's content -- the borders,
 * if there are any borders, won't be disturbed by the rendering.          
 *
 * The redrawWholeWindow arg lets you specify whether you intend to draw 
 * into any area of the entire window (sans border area) or to redraw just 
 * the areas that currently need redrawing (for instance, in response to 
 * a NEWSIZE event), as maintained automatically for you by the layer 
 * library.  Specify TRUE if you want to redraw the entire window.  Specify
 * FALSE if you want to redraw within the current draw-needed areas.
 *
 * By calling this routine, you are promising several things:
 *	- You will call UnmaskUnlock() after your rendering
 *	- You will not disturb window->UserData until after you call 
 *	  UnmaskUnlock()	
 *
 * Based on Workbench code by Neil Katin. 
 */
{
    struct Region *region;
    struct Layer *layer;
    struct Rectangle rect;
    struct Window *window;

    window = display->FirstWindow->DisplayWindow;

    /* Is this LockLayerInfo() necessary?  It would seem not, and yet 
     * you can crash a window without it by simple sizing 
     */     
    LockLayerInfo(&window->WScreen->LayerInfo);

    layer = window->RPort->Layer;
    LockLayerRom(layer);
       
    /* Get the rectangular area within the border (if any area or any 
     * border)
     */
    rect.MinX = window->BorderLeft;
    rect.MinY = window->BorderTop;
    rect.MaxX = window->Width - window->BorderRight - 1;
    rect.MaxY = window->Height - window->BorderBottom - 1;
	      
    window->UserData = NULL;
    if (redrawWholeWindow)
	{
	/* Set up a new region after saving the old.  The new region is the
	 * window sans the border area.
	 */
	if ((region = NewRegion()) == NULL)
	    {
	    Alert(ALERT_NO_NEWREGION, display);
	    goto SKIP_REGION;
	    }

	window->UserData = (UBYTE *)layer->DamageList;
	layer->DamageList = region;
    
	ClearRegion(region);
	OrRectRegion(region, &rect);
	}
    else /* just make sure the borders are masked out of the current window */
	AndRectRegion(layer->DamageList, &rect);

SKIP_REGION:
    BeginUpdate(layer);
}

		      
UnmaskUnlock(display)
struct Display *display;
/* This is the clean-up routine to SetMaskAndLock() */
{
    struct Layer *layer;
    struct Window *window;
    BOOL cleardamage;
			  
    window = display->FirstWindow->DisplayWindow;
    layer = window->RPort->Layer;

    if (window->UserData)
	{
	DisposeRegion(layer->DamageList);
	layer->DamageList = (struct Region *)window->UserData;
	cleardamage = FALSE;
	}
    else cleardamage = TRUE;

    EndUpdate(layer, cleardamage);
    ClearFlag(layer->Flags, LAYERREFRESH);
    UnlockLayerRom(layer);
    /* ??? */
    UnlockLayerInfo(&window->WScreen->LayerInfo);
}


MoveAndSetRegion(display, dx, dy, minx, miny, maxx, maxy)
struct Display *display;
SHORT dx, dy, minx, miny, maxx, maxy;
/* This routine moves the display window of the current display to the
 * area specified by the given arguments.  The destx/y args specify the
 * new location of the top-left corner of the current display area.
 * These args can have any values, negative or otherwise, even potentially
 * locating the current display completely offwindow.
 *
 * After performing the above feat, this routine then 
 * sets up regions in the window's layer to the portion outside
 * of the area of the new display location.  With this, you and I can then 
 * call RenderWindow() with a redisplayWholeWindow of FALSE, and then
 * render the window data exclusive of this new display.
 *
 * Understand?
 *
 * After calling this routine, you will probably want to call RenderWindow()
 * or RenderWindowGrunt().  Note that to take advantage of the mask created
 * by this routine, you should call with redrawWholeDisplay = FALSE.
 * 
 * NOTE:  RenderWindowGrunt() ends with an EndUpdate(), which clears away
 * the region set by this call.
 */
{
    SHORT width, height, right, bottom;
    struct Layer *layer;
    struct RastPort *rport;
	     
    if (NOT Lock(&display->DisplayLock)) goto ABORT_RETURN;
     
    rport = display->FirstWindow->DisplayWindow->RPort;

    if (display->CursorFlags & CURSOR_ON)
	{
	Curse(display);
	ClearFlag(display->CursorFlags, CURSOR_ON);
	SetFlag(display->CursorFlags, CURSOR_MOVED);
	}

    if ( (ABS(dx) > maxx - minx) || (ABS(dy) > maxy - miny) ) goto RETURN;

    ScrollRaster(rport, dx, dy, minx, miny, maxx, maxy);

    width = display->FirstWindow->DisplayWindow->Width;
    right = width - 1;
    height = display->FirstWindow->DisplayWindow->Height;
    bottom = height - 1;

    /* if the deltas exceed the scroll-window, clip them to the edges
     * to allows proper region sizing below
     */
    if (ABS(dx) > maxx - minx) dx = maxx - minx + 1;
    if (ABS(dy) > maxy - miny) dy = maxy - miny + 1;
							 
    layer = rport->Layer;

goto RETURN;
				  
/*???
    if (minx > 0)
	AddRegion(layer, 0, 0, minx - 1, bottom);
    if (maxx < right)
	AddRegion(layer, maxx + 1, 0, right, bottom);
    if (miny > 0)
	AddRegion(layer, 0, 0, right, miny - 1);
    if (maxy < bottom)
	AddRegion(layer, 0, maxy + 1, right, bottom);
 
    if (dx > 0) AddRegion(layer, maxx - dx + 1, miny, maxx, maxy);
    if (dy > 0) AddRegion(layer, minx, maxy - dy + 1, maxx, maxy);
    if (dx < 0) AddRegion(layer, minx, miny, minx -dx - 1, maxy);
    if (dy < 0) AddRegion(layer, minx, miny, maxx, miny -dy - 1);
???*/

RETURN:
    Unlock(&display->DisplayLock);
ABORT_RETURN:
}


/*???
AddRegion(layer, startx, starty, endx, endy)
struct Layer *layer;
SHORT startx, starty, endx, endy;
{
    struct Rectangle rect;

    rect.MinX = startx;
    rect.MinY = starty;
    rect.MaxX = endx;
    rect.MaxY = endy;

    OrRectRegion(layer->DamageList, &rect);
}
???*/
