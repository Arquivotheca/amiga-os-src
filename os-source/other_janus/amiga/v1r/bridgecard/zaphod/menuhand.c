
/* ***************************************************************************
 * 
 * Menu handler for the Display Task of the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY	Name	    Description
 * -----------	----------  -------------------------------------------------
 * 20 Feb 86	=RJ Mical=  Created this file
 * 13 Mar 86	=RJ=	    Changed this from cdtask.c to disptask.c
 * 9 Apr 86	=RJ=	    Created this out of disptask.c
 *   
 * **************************************************************************/

#include "zaphod.h"
			     

MenuEvent(display, code)
struct Display *display;
USHORT code;
{
    struct MenuItem *item;

    while (code != MENUNULL)
	{
	switch (MENUNUM(code))
	    {
	    case PROJECT_MENU:
		ProjectEvent(display, code);
		break;
	    case DISPLAY_MENU:
		DisplayEvent(display, code);
		break;
	    }
	item = ItemAddress(&MenuHeaders[MENU_HEADERS_COUNT - 1], code);
	code = item->NextSelect;
	}  
}



ProjectEvent(display, code)
struct Display *display;
USHORT code;
{
    switch(ITEMNUM(code))
	{
	case SAVE_ITEM:
	    WriteSettingsFile(display);
	    break;
	case RESTORE_ITEM:
	    ReadSettingsFile(display, TRUE);
	    SetColorColors(display);
	    break;
	case INFO_ITEM:
	    Info(SUBNUM(code), display);
	    break;
	case CLOSE_ITEM:
	    CloseDisplayTask(display);
	    break;
	}
}



DisplayEvent(display, code)
struct Display *display;
SHORT code;
{
    LONG seconds, micros;
    SHORT i;
    struct SuperWindow *superwindow;
    struct Window *window;

    superwindow = display->FirstWindow;

    switch (ITEMNUM(code))
	{
	case SIZE_ITEM:
	    if (SUBNUM(code)) FullSize(display);
	    else
		{
		Lock(&display->DisplayLock);
		SmallSize(display);
		if ((display->Modes & BORDER_VISIBLE) == 0)
		    BorderPatrol(display, BORDER_ON, TRUE);
		SetFlag(display->CursorFlags, CURSOR_MOVED);
		Unlock(&display->DisplayLock);
		}
	    break;
	case BORDER_ITEM:
	    Lock(&display->DisplayLock);
	    if (SUBNUM(code)) i = BORDER_ON;
	    else i = BORDER_OFF;  
	    BorderPatrol(display, i, TRUE);
	    SetFlag(display->CursorFlags, CURSOR_MOVED);
	    Unlock(&display->DisplayLock);
	    break;
	case COLOR_ITEM:
	    DoColorWindow(display);
	    break;
	case CURSOR_ITEM:
	    /* Stupid stupid stupid compiler */
	    seconds = 0;
	    micros = 0;
	    switch (SUBNUM(code))
		{
		case 3:
		    seconds = 1;
		    break;
		case 2:
		    micros = 1000000 / 2;
		    break;
		case 1:
		    micros = 1000000 / 4;
		    break;
		case 0:
		    micros = 1000000 / 8;
		    break;
		}
	    superwindow->CursorSeconds = seconds;
	    superwindow->CursorMicros = micros;
	    break;
	case NEWWINDOW_ITEM:
	    Lock(&display->DisplayLock);

	    GetNewSuperWindow(display);
	    OpenDisplay(display);
	    OpenDisplayWindow(display);
	    Forbid();
	    window = display->FirstWindow->DisplayWindow;
	    while ((window->UserPort = FindPort(INPUT_PORT_NAME)) == NULL)
		WaitTOF();
	    ModifyIDCMP(window, DISPLAY_IDCMP_FLAGS);
	    Permit();

	    Unlock(&display->DisplayLock);
	    break;
	case REFRESH_ITEM:
	    RefreshDisplay(display);
	    break;
	case DEPTH_ITEM:
	    i = 4 - SUBNUM(code);
	    if ((display->Modes & COLOR_DISPLAY) == 0) i -= 2;
	    else DefaultColorTextDepth = i;
	    Lock(&display->DisplayLock);
	    superwindow->TextDepth = i;
	    if (display->Modes & TEXT_MODE)
		{
		/* We're currently in text mode.  If this depth is 
		 * different from the current depth, we have to close the
		 * screen and open a new one, ala Crt().
		 */
		if (superwindow->TextDepth != superwindow->DisplayDepth)
		    {
		    SetFlag(display->Modes, OPEN_SCREEN);
		    RevampDisplay(display);
		    }
		}
	    Unlock(&display->DisplayLock);
	    break;
	case PRIORITY_ITEM:
	    switch (SUBNUM(code))
		{
		case 4:
		    i = 10;
		    break;
		case 3:
		    i = 5;
		    break;
		case 2:
		    i = 0;
		    break;
		case 1:
		    i = -5;
		    break;
		case 0:
		default:
		    i = -10;
		    break;
		}
	    SetTaskPri(FindTask(0), i);
	    DisplayPriority = i;
	    CursorPriority = i - 1;
	    PutNewPriority(display, i);
	    break;
	case INTERLACE_ITEM:
	    if (SUBNUM(code)) SetFlag(GfxBase->system_bplcon0, INTERLACE);
	    else ClearFlag(GfxBase->system_bplcon0, INTERLACE);
	    RemakeDisplay();
	    break;
	}
}




