
/* *** super.c ***************************************************************
 * 
 * Initialization Routines for the Display Tasks of the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 18 Mar 86   =RJ Mical=  Created this file
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
	superwindow->DisplayModes2 = display->Modes2;

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

	SetFlag(superwindow->Flags, WINDOW_WAS_HIRES);
	superwindow->DisplayNewWindow.Width = 640;

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
				ReleaseExtraScreen(display, superwindow->DisplayScreen);
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

	/* If we were scrolling, forget about it! 
	 * This allows any new window to start refreshing its display
	 * immediately.
	 */
	ClearFlag(display->Modes2, SCROLLING);

	if (display->FirstWindow)
		{
		display->FirstWindow->DisplayModes = display->Modes;
		display->FirstWindow->DisplayModes2 = display->Modes2;
		}

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

	/* If we were scrolling, forget about it! 
	 * This allows any new window to start refreshing its display
	 * immediately.
	 */
	ClearFlag(display->Modes2, SCROLLING);

	if (display->FirstWindow == superwindow)
		{
		superwindow->DisplayModes = display->Modes;
		superwindow->DisplayModes2 = display->Modes2;

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
	   
	/* If we were scrolling, forget about it! 
	 * This allows the new window to start refreshing its display
	 * immediately.
	 */
	ClearFlag(superwindow->DisplayModes2, SCROLLING);

	Lock(&display->DisplayLock);
	if (superwindow = display->FirstWindow)
		{
		display->Modes = superwindow->DisplayModes;
		display->Modes2 = superwindow->DisplayModes2;
		RecalcDisplayParms(display);
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
			superwindow->DisplayNewWindow.LeftEdge = window->LeftEdge;
			superwindow->DisplayNewWindow.TopEdge = window->TopEdge;
			superwindow->DisplayNewWindow.Width = window->Width;
			superwindow->DisplayNewWindow.Height = window->Height;
			if (FlagIsSet(display->Modes, MEDIUM_RES))
				ClearFlag(superwindow->Flags, WINDOW_WAS_HIRES);
			else
				SetFlag(superwindow->Flags, WINDOW_WAS_HIRES);

			if (display->Modes & OPEN_SCREEN)
				((struct ScreenExt *)window->WScreen->UserData)->UserCount--;

			if (display->TaskPort) DrainPort(display->TaskPort);

			superwindow->DisplayWindow = NULL;

			ClearMenuStrip(window);

			Forbid();
			if (window->UserPort) DrainPort(window->UserPort);
			window->UserPort = NULL;
			ModifyIDCMP(window, NULL);
			Permit();

			if (window->UserData) 
				FreeMem(window->UserData, sizeof(struct WindowExtension));

			CloseWindow(window);
			}
		}

	Unlock(&display->DisplayLock);
}



