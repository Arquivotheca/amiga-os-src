
/* ***************************************************************************
 * 
 * Window & Screen Open Routines for the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY	Name		Description
 * -------	------		--------------------------------------------
 * 28 Feb 86	=RJ Mical=	Created this file
 *
 * **************************************************************************/

#include "zaphod.h"


OpenDisplay(display)
struct Display *display;
{
    SHORT width, depth, mode, displaymodes;
    struct SuperWindow *superwindow;

    Lock(&display->DisplayLock);

    superwindow = display->FirstWindow;
    displaymodes = display->Modes;

    if (displaymodes & MEDIUM_RES)
	{
	width = 320;
	mode = NULL;
	}
    else
	{
	width = 640;
	mode = HIRES;
	}
    superwindow->DisplayNewScreen.Width = width;
    superwindow->DisplayNewScreen.ViewModes = mode;
    superwindow->DisplayNewWindow.Width = width;

    /* Now, use width to calculate the gettin'-small variables. */
    superwindow->LastSmallX = superwindow->LastSmallWidth = width / 2;
    superwindow->LastSmallY = 50;
    superwindow->LastSmallHeight = 100;

    if (displaymodes & TEXT_MODE)
	{
	depth = superwindow->TextDepth;
	}
    else
	{
	/* graphics-mode can be either 4 colors (2 planes) or 2 (1 plane) */
	if (displaymodes & MEDIUM_RES) depth = 2;
	else depth = 1;
	}
    superwindow->DisplayNewScreen.Depth = superwindow->DisplayDepth = depth;

    if ((displaymodes & TEXT_MODE) && (width == 640) && (depth == 2))
	{
	/* Oh goodie, we can use the Workbench */
	ClearFlag(display->Modes, OPEN_SCREEN);
	superwindow->DisplayScreen = NULL;
	}
    else
	{
	SetFlag(display->Modes, OPEN_SCREEN);
	if ((superwindow->DisplayScreen 
		= OpenScreen(&superwindow->DisplayNewScreen)) == NULL)
	    Alert(ALERT_NO_MEMORY, NULL);
	}

    SetColorColors(display);

    Unlock(&display->DisplayLock);
}


OpenDisplayWindow(display)
struct Display *display;
/* If this window is due for a CUSTOMSCREEN, before calling this routine, 
 * the Screen must be initialized
 */
{
    SHORT i;
    SHORT width;
    struct NewWindow *newwindow;
    struct Window *window;
    struct SuperWindow *superwindow;

    Lock(&display->DisplayLock);

    superwindow = display->FirstWindow;

    GetPCDisplay(display);
		       
    newwindow = &superwindow->DisplayNewWindow;

    if (display->Modes & OPEN_SCREEN)
	{
	newwindow->Screen = superwindow->DisplayScreen;
	newwindow->Type = CUSTOMSCREEN;
	ScreenToFront(superwindow->DisplayScreen);
	}
    else
	{
	newwindow->Type = WBENCHSCREEN;
	WBenchToFront();
	}

    if (display->Modes & COLOR_DISPLAY)
	newwindow->Title = &ColorActiveTitle[0];
    else
	newwindow->Title = &MonoActiveTitle[0];

    if (display->Modes & MEDIUM_RES)
	{
	width = 320;

	superwindow->HorizGadget.LeftEdge = 0;
	superwindow->HorizGadget.TopEdge = -9;
	superwindow->HorizGadget.Width = -11;
	superwindow->HorizGadget.Height = 10;
	superwindow->VertGadget.LeftEdge = -13;
	superwindow->VertGadget.TopEdge = 10;
	superwindow->VertGadget.Width = 14;
	superwindow->VertGadget.Height = -9 -10;
	}
    else
	{
	width = 640;

	superwindow->HorizGadget.LeftEdge = 0;
	superwindow->HorizGadget.TopEdge = -8;
	superwindow->HorizGadget.Width = -16;
	superwindow->HorizGadget.Height = 9;
	superwindow->VertGadget.LeftEdge = -18;
	superwindow->VertGadget.TopEdge = 10;
	superwindow->VertGadget.Width = 19;
	superwindow->VertGadget.Height = -9 -9;
	}

    newwindow->Width = newwindow->MaxWidth = width;

    if ( (window = OpenWindow(newwindow)) == NULL)
	{
	Alert(ALERT_NO_MEMORY, NULL);
	goto UNLOCK_RETURN;
	}

    superwindow->DisplayWindow = window;

    SetMenuStrip(window, &MenuHeaders[MENU_HEADERS_COUNT - 1]);

/*??? TOTAL CHEAT FOR HANOVER:	invisible gadget solution pending */
if (display->Modes & MEDIUM_RES) 
    {
    window->BorderRight += 2;
    window->GZZWidth -= 2;
    }
else
    {
    window->BorderRight += 3;
    window->GZZWidth -= 3;
    }

    FreeRemember(&display->BufferKey, TRUE);

    /* If we're in text mode, then allocate only enough memory for one
     * page of text.  If we're in graphics mode, we'll need the whole 16K
     */
    if (display->Modes & TEXT_MODE) i = ((width / 8) * 2) * 25;
    else i = 0x4000;

    display->Buffer = AllocRemember(&display->BufferKey, i, MEMF_CHIP);
    if (display->Buffer == NULL) Abort(ALERT_NO_MEMORY, display);

    InitBitMap(&display->BufferBitMap, superwindow->DisplayDepth, width, 200);
    InitBitMap(&display->TextBitMap, superwindow->DisplayDepth, 
	    BUFFER_WIDTH * CHAR_WIDTH, CHAR_HEIGHT);

    i = (width >> 3) * 200;		    /* This are for graphics mode */ 
    display->BufferBitMap.Planes[0] = display->Buffer + (i * 0);
    display->BufferBitMap.Planes[1] = display->Buffer + (i * 1);
    /* These four should never be used, but what the hell */
    display->BufferBitMap.Planes[2] = display->Buffer + (i * 2);
    display->BufferBitMap.Planes[3] = display->Buffer + (i * 3);
    display->BufferBitMap.Planes[4] = display->Buffer + (i * 4);
    display->BufferBitMap.Planes[5] = display->Buffer + (i * 5);

    ClearFlag(display->CursorFlags, CURSOR_ON);
    SetFlag(display->CursorFlags, CURSOR_MOVED);

    ModifyDisplayProps(display);

    if (display->Modes & BORDER_VISIBLE)
	{
	RenderWindow(display, TRUE, TRUE, TRUE);
	TopRightLines(window);
	}
    else
	{
	SetFlag(display->Modes, BORDER_VISIBLE);
	BorderPatrol(display, BORDER_OFF, TRUE);
	}

    SetColorColors(display);

UNLOCK_RETURN:
    Unlock(&display->DisplayLock);
}


USHORT *GetColorAddress(modes, depth)
SHORT modes, depth;
{
    USHORT *colortable;
    SHORT i;

    /* If we're in graphics and high-res, use the special 
     * high-res table 
     */
    if (modes & TEXT_MODE)
	{
	/* OK, text mode is easy */ 
	switch (depth)
	    {
	    case 1:
		colortable = &TextOnePlaneColors[0];
		break;
	    case 2:
		colortable = &TextTwoPlaneColors[0];
		break;
	    case 3:
		colortable = &TextThreePlaneColors[0];
		break;
	    case 4:
	    default:
		colortable = &TextFourPlaneColors[0];
		break;
	    }
	}
    else
	{
	/* This is graphics.  Now, low or high resolution? */
	if (modes & MEDIUM_RES)
	    {
	    if (modes & PALETTE_1)  i = 1;
	    else i = 0;
	    colortable = &LowGraphicsColors[ColorIntenseIndex][i][0];
	    }
	else
	    /* this is high-res graphics */
	    colortable = &HighGraphicsColors[0];
	}

    return(colortable);
}



SetColorColors(display)
struct Display *display;
{
    SHORT red, green, blue, i;
    USHORT *colortable;
    struct ViewPort *vport;
    struct SuperWindow *superwindow;

    if (superwindow = display->FirstWindow)
	{
	vport = NULL;
	if (superwindow->DisplayWindow)
	    vport = &superwindow->DisplayWindow->WScreen->ViewPort;
	else if (superwindow->DisplayScreen)
	    vport = &superwindow->DisplayScreen->ViewPort;

	if (vport)
	    {
	    colortable = GetColorAddress(display->Modes,
		    superwindow->DisplayDepth);
   
	    for (i = 0; i < (1 << superwindow->DisplayDepth); i++)
		{
		red = (*(colortable + i) >> 8)	& 0xF;
		green = (*(colortable + i) >> 4)  & 0xF;
		blue = (*(colortable + i) >> 0)  & 0xF;
		SetRGB4(vport, i, red, green, blue);
		}
	    }
	}
}



