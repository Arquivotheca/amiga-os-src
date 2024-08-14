
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



VOID	InstallFirstWindow(display)
REGIST	struct	Display *display;
{
	REGIST	struct	SuperWindow *superwindow;
	   
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


  
VOID	LinkSuperWindow(display, superwindow)
REGIST	struct	Display *display;
REGIST	struct	SuperWindow *superwindow;
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



SetPresets(display, newwindow, superwindow, presetWidth, presetHeight, 
		presetTopEdge, presetLeftEdge, presetDepth, presetFlags)
REGIST	struct	Display *display;
REGIST	struct	NewWindow *newwindow;
REGIST	struct	SuperWindow *superwindow;
SHORT	presetWidth;
SHORT	presetHeight;
SHORT	presetTopEdge;
SHORT	presetLeftEdge;
SHORT	presetDepth;
SHORT	presetFlags;
{
	if (presetWidth <= newwindow->MaxWidth)
		newwindow->Width = presetWidth;
	if (presetHeight <= newwindow->MaxHeight)
		newwindow->Height = presetHeight;
	if (presetTopEdge + newwindow->Height 
			<= newwindow->MaxHeight)
		newwindow->TopEdge = presetTopEdge;
	if (presetLeftEdge + newwindow->Width 
			<= newwindow->MaxWidth)
		newwindow->LeftEdge = presetLeftEdge;
	superwindow->TextDepth = presetDepth;
	if (FlagIsSet(presetFlags, PRESET_HIRES))
		SetFlag(superwindow->Flags, WINDOW_WAS_HIRES);
	else ClearFlag(superwindow->Flags, WINDOW_WAS_HIRES);
	if (FlagIsSet(presetFlags, PRESET_BORDER_ON))
		SetFlag(display->Modes, BORDER_VISIBLE);
	else ClearFlag(display->Modes, BORDER_VISIBLE);
	if (FlagIsSet(presetFlags, PRESET_PRIVACY))
		SetFlag(superwindow->Flags, WANTS_PRIVACY);
	else ClearFlag(superwindow->Flags, WANTS_PRIVACY);
}



BOOL	GetNewSuperWindow(display, usePresets)
REGIST	struct	Display *display;
BOOL	usePresets;
/* When usePresets is TRUE, presets (if any) will be used with the 
 * super window 
 */
{ 
	REGIST	struct	SuperWindow *superwindow, *firstwindow;
	REGIST	struct	NewWindow *newwindow;
	REGIST	BOOL	retvalue;

	retvalue = FALSE;
	   
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

	newwindow = &superwindow->DisplayNewWindow;

	SetFlag(superwindow->Flags, WINDOW_WAS_HIRES);
	newwindow->Width = 640;

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
	newwindow->FirstGadget = &superwindow->VertGadget;

	LinkSuperWindow(display, superwindow);

	SetFlag(display->Modes, BORDER_VISIBLE);

	if ( usePresets && FlagIsSet(display->Modes2, WINDOW_PRESETS) )
		{
		if (FlagIsClear(display->Modes, COLOR_DISPLAY)
			  && FlagIsSet(display->PresetMonoFlags, PRESETS_GOT))
			SetPresets(display, newwindow, superwindow, 
					display->PresetMonoWidth, 
					display->PresetMonoHeight, 
					display->PresetMonoTopEdge, 
					display->PresetMonoLeftEdge, 
					display->PresetMonoDepth, 
					display->PresetMonoFlags);
		else if (FlagIsSet(display->Modes, COLOR_DISPLAY)
			  && FlagIsSet(display->PresetColorFlags, PRESETS_GOT))
			SetPresets(display, newwindow, superwindow, 
					display->PresetColorWidth, 
					display->PresetColorHeight, 
					display->PresetColorTopEdge, 
					display->PresetColorLeftEdge, 
					display->PresetColorDepth, 
					display->PresetColorFlags);

		RecalcDisplayParms(display);
		}

	retvalue = TRUE;

UNLOCK_RETURN:
	Unlock(&display->DisplayLock);
	return(retvalue);
}



VOID	UnlinkSuperWindow(display, superwindow)
REGIST	struct	Display *display;
REGIST	struct	SuperWindow *superwindow;
{
	REGIST	struct	SuperWindow *nextwindow;

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



VOID	SuperCloseWindow(display)
REGIST	struct	Display *display;
/* NOTE:  this routine decouples the IDCMP Port from the window before
 * closing the window.
 * NOTE:  this routine drains any input from the display task's port
 * before returning.
 */
{
	REGIST	struct	SuperWindow *superwindow;
	REGIST	struct	Window *window;
	REGIST	SHORT	i, i2;
	struct	ViewPort *vport;

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
				((struct ScreenExt *)window->WScreen
						->UserData)->UserCount--;
			else
				{
				/* This is Workbench viewport, so if we're 
				 * the last window then restore the colors 
				 */
				ChangeWBWindowCount(-1);
				if ( (CountWBWindows() == 0)
					  && FlagIsSet(display->Modes2, 
					    WBCOLORS_GRABBED) )
					{
					if (vport = &window->WScreen->ViewPort)
		 				for (i = 0; i < 4; i++)
							{
							i2 = 
							  display->WBColors[i];
							SetRGB4(vport, i, 
								i2 >> 8,
								i2 >> 4,
								i2);
							}
					ClearFlag(display->Modes2, 
							WBCOLORS_GRABBED);
					}
				}


			if (display->TaskPort) DrainPort(display->TaskPort);

			superwindow->DisplayWindow = NULL;

			ClearMenuStrip(window);

			Forbid();
			if (window->UserPort) DrainPort(window->UserPort);
			window->UserPort = NULL;
			ModifyIDCMP(window, NULL);
			Permit();

			if (window->UserData) 
				FreeMem(window->UserData, 
					sizeof(struct WindowExtension));

			CloseWindow(window);
			}
		}

	Unlock(&display->DisplayLock);
}



VOID	DestroySuperWindow(display)
REGIST	struct	Display *display;
{
	REGIST	struct	SuperWindow *superwindow;
	  
	Lock(&display->DisplayLock);

	if (superwindow = display->FirstWindow)
		{
		if (superwindow->DisplayWindow)
			{
			if ((superwindow->DisplayModes & BORDER_VISIBLE) == 0)
				BorderPatrol(display, BORDER_ON, FALSE);
			SuperCloseWindow(display);
			if (superwindow->DisplayScreen)
				ReleaseExtraScreen(display, 
						superwindow->DisplayScreen);
			}

		UnlinkSuperWindow(display, superwindow);

		FreeMem(superwindow, sizeof(struct SuperWindow));
		}

	Unlock(&display->DisplayLock);
}



