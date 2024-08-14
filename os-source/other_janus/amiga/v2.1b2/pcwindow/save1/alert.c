
/* *** alert.c **************************************************************
 * 
 * Alert and Abort Routines --   Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 11 Apr 86   =RJ Mical=  Created this file
 *
 * *************************************************************************/

#include "zaphod.h"

#define  DBG_ALERT_GRUNT_ENTER   1
#define  DBG_ALERT_GRUNT_RETURN  1
#define  DBG_MY_ALERT_ENTER      1
#define  DBG_MY_ALERT_RETURN     1
#define  DBG_ABORT_ENTER         1
#define  DBG_ABORT_RETURN        1

#define EXTRA_TOP   6
#define EXTRA_LEFT  6

UBYTE *AlertStrings[] =
    {
/*  (UBYTE *)"|--- Maximum string length ------>|", */
    (UBYTE *)"xSystem failure, unable to continu.",
    (UBYTE *)"xCan't Open Key Table File",
    (UBYTE *)"xTask Signals Not Available",
    (UBYTE *)"xIncomplete System",
    (UBYTE *)"xNot Enough Memory",
    (UBYTE *)"xCouldn't Find Janus.Library",
    (UBYTE *)"xCouldn't Make Region",
    (UBYTE *)"xCouldn't Create Port",
    (UBYTE *)"xCouldn't Find Font",
    (UBYTE *)"xCouldn't Create Timer",
    (UBYTE *)"xLock List Corrupt",
    (UBYTE *)"xCan't Make Sidecar Signals",
    (UBYTE *)"xCan't Open Settings File",
    (UBYTE *)"xPlease see the HELP page about COY",
    (UBYTE *)"xYou've already opened that displa!",
    (UBYTE *)"xCan't Open Scancode Table File",
    (UBYTE *)"xCan't hide frozen window's border",
    (UBYTE *)"xCan't change frozen window's text",
    (UBYTE *)"xRJ was here",
    (UBYTE *)"xPC Window 2.0  by Robert J. Mical",
    (UBYTE *)"xWarning: old SidecarKeys.Table fi",

    (UBYTE *)"No memory for Display struc",
    (UBYTE *)"Could not open Graphics.library",
    (UBYTE *)"Could not open Layers.library",
    (UBYTE *)"Could not open DiskFont.library",
    (UBYTE *)"Could not open Janus.library",
    (UBYTE *)"You've already opened that display!",
    (UBYTE *)"No memory for Display Buffer",
    (UBYTE *)"No memory for Normal Font",
    (UBYTE *)"No memory for Underline Font",
    (UBYTE *)"No Text Clip",
    (UBYTE *)"No memory for SuperWindow Struc",
    (UBYTE *)"No memory for Color Window",
    (UBYTE *)"No Task Stack memory",
    (UBYTE *)"No PCDisplay memory",
    (UBYTE *)"No ExtraScreen memory",
    (UBYTE *)"No Window memory",
    (UBYTE *)"No Extension Window memory",
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
#if (DEBUG1 & DBG_ALERT_GRUNT_ENTER)
   kprintf("Alert.c:      AlertGrunt(text=0x%lx,window=0x%lx)\n",text,window);
#endif

    AlertText.IText = text;
    AutoRequest(window, &AlertText, &OKText, 
   &OKText, 0, 0, 320, 47 + EXTRA_TOP);

#if (DEBUG2 & DBG_ALERT_GRUNT_RETURN)
   kprintf("Alert.c:      AlertGrunt: Returns(VOID)\n");
#endif
}

VOID MyAlert(abortNumber, display)
REGISTER USHORT abortNumber;
REGISTER struct Display *display;
{
    REGISTER struct Window *window;
    REGISTER BOOL hideborders;

#if (DEBUG1 & DBG_MY_ALERT_ENTER)
   kprintf("Alert.c:      MyAlert(abortNumber=0x%lx,display=0x%lx)\n",abortNumber,display);
#endif

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

#if (DEBUG2 & DBG_MY_ALERT_RETURN)
   kprintf("Alert.c:      MyAlert: Returns(VOID)\n");
#endif
}

VOID Abort(abortNumber, display)
REGISTER USHORT abortNumber;
REGISTER struct Display *display;
{
#if (DEBUG1 & DBG_ABORT_ENTER)
   kprintf("Alert.c:      Abort(abortNumber=0x%lx,display=0x%lx)\n",abortNumber,display);
#endif

    Forbid(); 

/*???    FreeRemember(&GlobalKey, TRUE);*/
    if (display) FreeRemember(&display->BufferKey, TRUE);

    MyAlert(abortNumber, display);
    FOREVER MyAlert(ALERT_ABORT, display);

/*???    UnmakeWaitWindow();*/
/*???    if (display) CloseDisplayTask(display);*/
/*???    CloseEverything();*/
/*???    exit(-1);*/

#if (DEBUG2 & DBG_ABORT_RETURN)
   kprintf("Alert.c:      Abort: Returns(VOID)\n");
#endif
}
