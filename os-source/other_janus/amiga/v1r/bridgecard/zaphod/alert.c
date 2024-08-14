
/* ***************************************************************************
 * 
 * Alert and Abort Routines --	Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY	Name		Description
 * -------	------		--------------------------------------------
 * 11 Apr 86	=RJ Mical=	Created this file
 *
 * **************************************************************************/

#include "zaphod.h"


#define EXTRA_TOP   6
#define EXTRA_LEFT  6


UBYTE *AlertStrings[] =
    {
    "Really broken.  Stop all and reboot",
    "Can't Open Key Table File",
    "Task Signals Not Available",
    "Incomplete System",
    "Not Enough Memory",
    "Couldn't Find Library",
    "Couldn't Make Region",
    "Couldn't Create Port",
    "Couldn't Find Font",
    "Couldn't Create Timer",
    "Lock List Corrupt",
    "Can't Make Sidecar Signals",
    "Can't Open Settings File",
    };


struct IntuiText AlertText =
    {
    AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
    AUTOLEFTEDGE + EXTRA_LEFT, AUTOTOPEDGE + EXTRA_TOP,
    &SafeFont,
    NULL, NULL,
    };




Alert(abortNumber, display)
USHORT abortNumber;
struct Display *display;
{
    struct Window *window;

    if (display)
	{
	if (display->FirstWindow)
	    window = display->FirstWindow->DisplayWindow;
	}
    else window = NULL;

    AlertText.IText = AlertStrings[abortNumber];
    AutoRequest(window, &AlertText, NULL, &OKText, 0, 0, 320, 45 + EXTRA_TOP);
}
 

Abort(abortNumber, display)
USHORT abortNumber;
struct Display *display;
{
    Forbid(); 

    if (display) FreeRemember(&display->BufferKey, TRUE);
    FreeRemember(&GlobalKey, TRUE);

    Alert(abortNumber, display);
    Alert(ALERT_ABORT, display);

    RemTask(0);
}
 

