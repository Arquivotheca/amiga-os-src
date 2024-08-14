
/* ***************************************************************************
 * 
 * The Auto-sizing window routines for the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY	Name	    Description
 * -----------	----------  -------------------------------------------------
 * 16 Apr 86	=RJ Mical=  Burst this file out of the bulky render.c
 *
 * **************************************************************************/

#include "zaphod.h"


SmallSize(display)
struct Display *display;
{
    SHORT x, y, width, height;
    struct SuperWindow *superwindow;
    struct Window *window;
    BOOL sizexfirst, sizeyfirst;

    Forbid();
    Lock(&display->DisplayLock);
				       
    superwindow = display->FirstWindow;
    window = superwindow->DisplayWindow;

    LockLayerInfo(&window->WScreen->LayerInfo);
    LockLayer(&window->WScreen->LayerInfo, window->RPort->Layer);

    x = superwindow->LastSmallX + superwindow->LastSmallWidth;
    if (x > window->MaxWidth)
	superwindow->LastSmallWidth = window->MaxWidth
		- superwindow->LastSmallX;

    y = superwindow->LastSmallY + superwindow->LastSmallHeight;
    if (y > window->MaxHeight)
	superwindow->LastSmallHeight = window->MaxHeight
		- superwindow->LastSmallY;
 
    x = superwindow->LastSmallX - window->LeftEdge;
    y = superwindow->LastSmallY - window->TopEdge;
    width = superwindow->LastSmallWidth - window->Width;
    height = superwindow->LastSmallHeight - window->Height;
					  
    /* Now, if the destination position and the current size will go
     * beyond the screen boundaries, then size first, else move first
     */
    if (superwindow->LastSmallX  + window->Width > window->MaxWidth)
	sizexfirst = TRUE;
    else sizexfirst = FALSE;

    if (superwindow->LastSmallY  + window->Height > window->MaxHeight)
	sizeyfirst = TRUE;
    else sizeyfirst = FALSE;
		    
    if (sizexfirst)
	{
	if (width) SizeWindow(window, width, 0);
	}
    if (sizeyfirst)
	{
	if (height) SizeWindow(window, 0, height);
	}
    if ((x) || (y)) MoveWindow(window, x, y);

    if (NOT sizexfirst)
	{
	if (width) SizeWindow(window, width, 0);
	}
    if (NOT sizeyfirst)
	{
	if (height) SizeWindow(window, 0, height);
	}

    UnlockLayer(window->RPort->Layer);
    UnlockLayerInfo(&window->WScreen->LayerInfo);

    Permit();

	   
    /* Now, try to get around a bug in V1.1 Intuition:	when the SizeWindow()
     * function is called with an argument less than zero, the gadgets aren't
     * redrawn.  No problem, you say, just redraw the gadgets after 
     * calling SizeWindow().  Uh uh, bozo.  Calling SizeWindow() doesn't 
     * mean that your window has been sized, it only means that your window
     * will be sized next time Intuition gets control.	Now don't you wish
     * you had had time to make Intuition a device?  Anyway, the following
     * is totally from Kludge-City.  If all sizing was to smaller sizes,
     * the gadget imagery is sure to be totalled under 1.1, so call
     * WaitTOF() a few times to give Intuition a chance to get in there,
     * and then refresh the gadgets.  This isn't going to work 100% of the
     * time, but it should work 99.99% anyway.	Excuse me ...
     */
    WaitTOF();
    WaitTOF();
    WaitTOF();
    WaitTOF();
    if ((width < 0) && (height < 0))
	{
	if (display->Modes & BORDER_VISIBLE)
	    {
	    RefreshWindowFrame(window);
	    }
	}

    Unlock(&display->DisplayLock); 
}


FullSize(display)
struct Display *display;
{
    SHORT x, y, width, height;
    struct Window *window;

    Forbid();
    Lock(&display->DisplayLock);

    window = display->FirstWindow->DisplayWindow;

    LockLayerInfo(&window->WScreen->LayerInfo);
    LockLayer(&window->WScreen->LayerInfo, window->RPort->Layer);

    x = window->LeftEdge;
    y = window->TopEdge;
    width = window->MaxWidth - window->Width;
    height = window->MaxHeight - window->Height;

    if ((x) || (y)) MoveWindow(window, -x, -y);

    if ((width > 0) || (height > 0)) SizeWindow(window, width, height);

    UnlockLayer(window->RPort->Layer);
    UnlockLayerInfo(&window->WScreen->LayerInfo);

    Unlock(&display->DisplayLock);   
    Permit();
}



