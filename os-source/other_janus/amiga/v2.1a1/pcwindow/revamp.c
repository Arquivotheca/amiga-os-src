
/**** revamp.c **************************************************************
 * 
 * This files contains the routines that revamp a Zaphod display
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 18 Jan 87   =RJ Mical=  Created this file
 *
 ***************************************************************************/

#include "zaphod.h"
#include <proto/exec.h>

#define  DBG_REVAMP_DISPLAY_ENTER      1

/****i* PCWindow/RevampDisplay ******************************************
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
* This guys expects that any locking is already done.
* REMEMBER:  don't call Lock() on a display before calling here,
* as this guy calls OpenDisplay() which locks the display list and
* then tries to lock the display, which easily leads to the ever-popular
* deadlock situation, so call DeadboltDisplay() rather than Lock().
*
*/

VOID RevampDisplay(display, newmodes, forceAll, borderForceNewWindow)
struct Display *display;
SHORT newmodes;
BOOL forceAll, borderForceNewWindow;
{
   struct MsgPort *saveport;
   ULONG saveidcmp;
   struct SuperWindow *superwindow;
   struct Window *window;
   BOOL revamp, redowindow;
   SHORT i;

#if (DEBUG1 & DBG_REVAMP_DISPLAY_ENTER)
   kprintf("revamp.c:     RevampDisplay(display=0x%lx,newmodes=0x%lx,forceAll=0x%lx,borderForceNewWindow=0x%lx)\n",display,newmodes,forceAll,borderForceNewWindow);
#endif

   superwindow = display->FirstWindow;

   if (forceAll) 
      revamp = TRUE;
   else
   {
      revamp = FALSE;
      if (superwindow && FlagIsSet(newmodes, TEXT_MODE))
      {
         /* We're currently in text mode.  If this depth is 
          * different from the current depth, we have to close 
          * the screen and open a new one.
          */
         if (superwindow->TextDepth != superwindow->DisplayDepth)
            revamp = TRUE;
      }
      else 
	     if (superwindow)
         {
            if (FlagIsSet(newmodes, MEDIUM_RES)) 
			   i = 2;
            else 
			   i = 1;
            if (i != superwindow->DisplayDepth) 
			   revamp = TRUE;
         }

      if ((newmodes & MEDIUM_RES) != (display->Modes & MEDIUM_RES))
         revamp = TRUE;
   }

   Forbid();
   if (superwindow)
   {
      if ((revamp && (window = superwindow->DisplayWindow))
            || borderForceNewWindow)
      {
         saveport = window->UserPort;
         saveidcmp = window->IDCMPFlags;
         if (NOT borderForceNewWindow)
            if (FlagIsClear(display->Modes, BORDER_VISIBLE))
            {
               BorderPatrol(display, BORDER_ON, FALSE);
               ClearFlag(newmodes, BORDER_VISIBLE);
            }

         SuperCloseWindow(display);
         redowindow = TRUE;
      }
      else 
	     redowindow = FALSE;

      /* Do we need to redo the screen? */
      if (revamp && (superwindow->DisplayScreen))
         ReleaseExtraScreen(display, superwindow->DisplayScreen);

      display->Modes = superwindow->DisplayModes = newmodes;

      if (revamp) 
	     OpenDisplay(display);

      if (redowindow)
         OpenDisplayWindow(display, saveport, saveidcmp,FALSE);
   } else {
      display->Modes = newmodes;
      SetColorColors(display, -1);
   }

   Permit();
}
