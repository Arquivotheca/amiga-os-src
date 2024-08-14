
/* *** alert.c **************************************************************
 * 
 * Alert and Abort Routines --	Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 11 Apr 86   =RJ Mical=  Created this file
 *
 * *************************************************************************/

#include "zaphod.h"


#define EXTRA_TOP   6
#define EXTRA_LEFT  6


UBYTE *AlertStrings[] =
    {
/*  (UBYTE *)"|--- Maximum string length ------>|", */
    (UBYTE *)"System failure, unable to continue.",
    (UBYTE *)"Can't Open Key Table File",
    (UBYTE *)"Task Signals Not Available",
    (UBYTE *)"Incomplete System",
    (UBYTE *)"Not Enough Memory",
    (UBYTE *)"Couldn't Find Library",
    (UBYTE *)"Couldn't Make Region",
    (UBYTE *)"Couldn't Create Port",
    (UBYTE *)"Couldn't Find Font",
    (UBYTE *)"Couldn't Create Timer",
    (UBYTE *)"Lock List Corrupt",
    (UBYTE *)"Can't Make Sidecar Signals",
    (UBYTE *)"Can't Open Settings File",
    (UBYTE *)"Please see the HELP page about COPY",
    (UBYTE *)"You've already opened that display!",
    (UBYTE *)"Can't Open Scancode Table File",
    (UBYTE *)"Can't hide frozen window's border",
    (UBYTE *)"Can't change frozen window's text",
/*???    (UBYTE *)"Oolah!  Oooolaaahhhh!",*/
/*???    (UBYTE *)"Not for release",*/
    (UBYTE *)"RJ was here",
/*  (UBYTE *)"|--- Maximum string length ------>|", */
    (UBYTE *)"PC Window 2.0b6  by Robert J. Mical",
/*???    (UBYTE *)"PC Window 2.0g1  by Robert J. Mical",*/
    (UBYTE *)"Warning: old SidecarKeys.Table file",
    (UBYTE *)"",
    };


struct IntuiText AlertText =
    {
    AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
    AUTOLEFTEDGE + EXTRA_LEFT, AUTOTOPEDGE + EXTRA_TOP,
    &SafeFont,
    NULL, NULL,
    };




VOID AlertGrunt(text, window)
REGISTER UBYTE *text;
REGISTER struct Window *window;
{
    AlertText.IText = text;
    AutoRequest(window, &AlertText, &OKText, 
	&OKText, 0, 0, 320, 47 + EXTRA_TOP);
}
 

VOID Alert(abortNumber, display)
REGISTER USHORT abortNumber;
REGISTER struct Display *display;
{
    REGISTER struct Window *window;
    REGISTER BOOL hideborders;

    window = NULL;
    hideborders = FALSE;

    if (display)
	{
	if (display->FirstWindow)
	    window = display->FirstWindow->DisplayWindow;
	if (window
		&& FlagIsClear(display->Modes, BORDER_VISIBLE)
		&& FlagIsClear(display->Modes, PAL_PRESENCE))
	    {
	    hideborders = TRUE;
	    BorderPatrol(display, BORDER_ON, TRUE);
	    }
	}

    AlertGrunt(AlertStrings[abortNumber], window);

    if (hideborders) BorderPatrol(display, BORDER_OFF, TRUE);
}



VOID Abort(abortNumber, display)
REGISTER USHORT abortNumber;
REGISTER struct Display *display;
{
    Forbid(); 

/*???    FreeRemember(&GlobalKey, TRUE);*/
    if (display) FreeRemember(&display->BufferKey, TRUE);

    Alert(abortNumber, display);
    FOREVER Alert(ALERT_ABORT, display);

/*???    UnmakeWaitWindow();*/
/*???    if (display) CloseDisplayTask(display);*/
/*???    CloseEverything();*/
/*???    exit(-1);*/
}


