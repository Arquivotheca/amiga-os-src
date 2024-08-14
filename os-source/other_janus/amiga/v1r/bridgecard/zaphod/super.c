
/* ***************************************************************************
 * 
 * Initialization Routines for the Display Tasks of the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY	Name		Description
 * -------	------		--------------------------------------------
 * 18 Mar 86	=RJ Mical=	Created this file from init.c
 *
 * **************************************************************************/

#include "zaphod.h"
								    

GetNewSuperWindow(display)
struct Display *display;
{ 
    struct SuperWindow *superwindow, *firstwindow;
       
    Lock(&display->DisplayLock);

    if ((superwindow = (struct SuperWindow *)AllocMem(
	    sizeof(struct SuperWindow), MEMF_CLEAR)) == NULL)
	{
	Alert(ALERT_NO_MEMORY);
	goto UNLOCK_RETURN;
	}

    superwindow->DisplayModes = display->Modes;

    if (firstwindow = display->FirstWindow)
	{
	superwindow->TextDepth = firstwindow->TextDepth;
	superwindow->CursorSeconds = firstwindow->CursorSeconds;
	superwindow->CursorMicros = firstwindow->CursorMicros;
	}
    else
	{
	if (display->Modes & COLOR_DISPLAY)
	    superwindow->TextDepth = DefaultColorTextDepth;
	else superwindow->TextDepth = 2;
	superwindow->CursorSeconds = DEFAULT_CURSOR_SECONDS;
	superwindow->CursorMicros = DEFAULT_CURSOR_MICROS;
	}

    superwindow->DisplayNewWindow = NewWindow;
    superwindow->DisplayNewScreen = NewScreen;

    superwindow->HorizInfo = HorizInfo;
    superwindow->VertInfo = VertInfo;
    superwindow->HorizGadget = HorizGadget;
    superwindow->VertGadget = VertGadget;

    superwindow->HorizGadget.NextGadget = NULL;
    superwindow->HorizGadget.GadgetRender = (APTR)&superwindow->HorizImage;
    superwindow->HorizGadget.SpecialInfo = (APTR)&superwindow->HorizInfo;
    superwindow->VertGadget.NextGadget = &superwindow->HorizGadget;
    superwindow->VertGadget.GadgetRender = (APTR)&superwindow->VertImage;
    superwindow->VertGadget.SpecialInfo = (APTR)&superwindow->VertInfo;
    superwindow->DisplayNewWindow.FirstGadget = &superwindow->VertGadget;

    LinkSuperWindow(display, superwindow);

    SetFlag(display->Modes, BORDER_VISIBLE);

UNLOCK_RETURN:
    Unlock(&display->DisplayLock);
}


DestroySuperWindow(display)
struct Display *display;
{
    struct SuperWindow *superwindow;
      
    Lock(&display->DisplayLock);
				
    if (superwindow = display->FirstWindow)
	{
	if (superwindow->DisplayWindow)
	    {
	    if ((superwindow->DisplayModes & BORDER_VISIBLE) == 0)
		BorderPatrol(display, BORDER_ON, FALSE);
	    SuperCloseWindow(display);
	    if (superwindow->DisplayScreen)
		SuperCloseScreen(display);
	    }

	UnlinkSuperWindow(display, superwindow);

	FreeMem(superwindow, sizeof(struct SuperWindow));
	}

    Unlock(&display->DisplayLock);
}



   
LinkSuperWindow(display, superwindow)
struct Display *display;
struct SuperWindow *superwindow;
{
    Lock(&display->DisplayLock);

    if (display->FirstWindow)
	display->FirstWindow->DisplayModes = display->Modes;

    superwindow->NextWindow = display->FirstWindow;
    display->FirstWindow = superwindow;
    InstallFirstWindow(display);

    Unlock(&display->DisplayLock);
}


UnlinkSuperWindow(display, superwindow)
struct Display *display;
struct SuperWindow *superwindow;
{
    struct SuperWindow *nextwindow;

    Lock(&display->DisplayLock);

    if (display->FirstWindow == superwindow)
	{
	superwindow->DisplayModes = display->Modes;

	display->FirstWindow = superwindow->NextWindow;
	InstallFirstWindow(display);
	}
    else
	{
	nextwindow = display->FirstWindow;
	while (nextwindow->NextWindow != superwindow)
	    nextwindow = nextwindow->NextWindow;
	nextwindow->NextWindow = superwindow->NextWindow;      
	}

    Unlock(&display->DisplayLock);
}



InstallFirstWindow(display)
struct Display *display;
{
    struct SuperWindow *superwindow;
       
    Lock(&display->DisplayLock);
    if (superwindow = display->FirstWindow)
	{
	display->Modes = superwindow->DisplayModes;
	}
    Unlock(&display->DisplayLock);
}


  
SuperCloseWindow(display)
struct Display *display;
/* NOTE:  this routine decouples the IDCMP Port from the window before
 * closing the window.
 * NOTE:  this routine drains any input from the display task's port
 * before returning.
 */
{
    struct SuperWindow *superwindow;
    struct Window *window;

    Lock(&display->DisplayLock);

    if (superwindow = display->FirstWindow)
	{
	if (window = superwindow->DisplayWindow)
	    {
	    if (display->TaskPort) DrainPort(display->TaskPort);

	    superwindow->DisplayWindow = NULL;

	    ClearMenuStrip(window);

	    if (window->UserPort) DrainPort(window->UserPort);

	    window->UserPort = NULL;
	    CloseWindow(window);
	    }
	}

    Unlock(&display->DisplayLock);
}



SuperCloseScreen(display)
struct Display *display;
{
    struct Screen *screen;
	    
    Lock(&display->DisplayLock);
    if (display->FirstWindow)
	{
	screen = display->FirstWindow->DisplayScreen;
	display->FirstWindow->DisplayScreen = NULL;
	if (screen) CloseScreen(screen);
	}
    Unlock(&display->DisplayLock);
}


