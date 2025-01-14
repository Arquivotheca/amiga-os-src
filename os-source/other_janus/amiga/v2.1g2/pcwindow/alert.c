
/**** alert.c **************************************************************
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
 **************************************************************************/

#include "zaphod.h"
#include "pcwindow_rev.h"
#include <proto/intuition.h>

#define  DBG_ALERT_GRUNT_ENTER   1
#define  DBG_ALERT_GRUNT_RETURN  1
#define  DBG_MY_ALERT_ENTER      1
#define  DBG_MY_ALERT_RETURN     1
#define  DBG_ABORT_ENTER         1
#define  DBG_ABORT_RETURN        1

#define EXTRA_TOP   6
#define EXTRA_LEFT  6

UBYTE *VerTitle = VERS;
UBYTE *EmbeddedVersion = VERSTAG;

static UBYTE *AlertStrings[] =
    {
/*  (UBYTE *)"|--- Maximum string length ------>|", */
    (UBYTE *)"System failure, unable to continue.",
    (UBYTE *)"Can't Open Key Table File",
    (UBYTE *)"Task Signals Not Available",
    (UBYTE *)"Incomplete System",
    (UBYTE *)"Not Enough Memory",
    (UBYTE *)"Couldn't Find Janus.Library",
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
    (UBYTE *)"RJ was here",
    /* (UBYTE *)"PC Window 2.01",*/
    (UBYTE *)VERS,
    (UBYTE *)"Warning: old SidecarKeys.Table fi",

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

static struct IntuiText AlertText =
   {
      AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
      AUTOLEFTEDGE + EXTRA_LEFT, AUTOTOPEDGE + EXTRA_TOP,
      &SafeFont,
      NULL, NULL,
   };

/****i* PCWindow/MyAlert ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

VOID MyAlert(abortNumber, display)
USHORT abortNumber;
struct Display *display;
{
   struct Window *window;
   BOOL hideborders;

#if (DEBUG1 & DBG_MY_ALERT_ENTER)
   kprintf("Alert.c:      MyAlert(abortNumber=0x%lx,display=0x%lx)\n",abortNumber,display);
#endif

   window = NULL;
   hideborders = FALSE;

   if (display)
   {
      if (display->FirstWindow)
         window = display->FirstWindow->DisplayWindow;
      if (window && FlagIsClear(display->Modes, BORDER_VISIBLE)
                 && FlagIsClear(display->Modes, PAL_PRESENCE))
      {
         hideborders = TRUE;
         BorderPatrol(display, BORDER_ON, TRUE);
      }
   }

   /* AlertGrunt(AlertStrings[abortNumber], window); */
   AlertText.IText = AlertStrings[abortNumber];
   AutoRequest(window,&AlertText,&OKText,&OKText,0,0,320,47+EXTRA_TOP);

   if (hideborders) 
      BorderPatrol(display, BORDER_OFF, TRUE);

#if (DEBUG2 & DBG_MY_ALERT_RETURN)
   kprintf("Alert.c:      MyAlert: Returns(VOID)\n");
#endif
}
