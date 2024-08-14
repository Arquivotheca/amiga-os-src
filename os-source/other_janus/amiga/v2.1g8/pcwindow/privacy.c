
/**** privacy.c ************************************************************
 * 
 * The Window Privacy Routines
 *
 * Copyright (C) 1987, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 22 Jan 87   =RJ Mical=  Created this file
 *
 **************************************************************************/

#include "zaphod.h"
#include <proto/intuition.h>

#define  DBG_WANTS_PRIVACY_ENTER    1
#define  DBG_WANTS_COMMUNITY_ENTER  1

/****i* PCWindow/WantsPrivacy ******************************************
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

VOID WantsPrivacy(struct Display *display)
{
   struct SuperWindow *superwindow;
   BOOL closerup;
   struct ScreenExt *sext;

#if (DEBUG1 & DBG_WANTS_PRIVACY_ENTER)
   kprintf("privacy.c:    WantsPrivacy(display=0x%lx)\n",display);
#endif

   superwindow = display->FirstWindow;
   if (FlagIsSet(superwindow->Flags, WANTS_PRIVACY)) 
      return;
   SetFlag(superwindow->Flags, WANTS_PRIVACY);

   closerup = FALSE;
   if (FlagIsClear(display->Modes, OPEN_SCREEN))
      closerup = TRUE;
   else 
   {
      sext = ((struct ScreenExt *)superwindow->DisplayScreen->UserData);
      if (sext->UserCount > 1) 
	     closerup = TRUE;
   }
   if (closerup)
      RevampDisplay(display, display->Modes, TRUE, FALSE);

   sext = ((struct ScreenExt *)superwindow->DisplayScreen->UserData);
   SetFlag(sext->Flags, PRIVATE_SCREENING);
   superwindow->DisplayScreen->DefaultTitle = &ScreenUnsharedTitle[0];
   SetWindowTitles(display->FirstWindow->DisplayWindow, (UBYTE *)-1,
                   &ScreenUnsharedTitle[0]);
}

/****i* PCWindow/WantsCommunity ******************************************
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

VOID WantsCommunity(struct Display *display)
{
   struct SuperWindow *superwindow;
   struct ScreenExt *sext;
   BOOL revamp;

#if (DEBUG1 & DBG_WANTS_COMMUNITY_ENTER)
   kprintf("privacy.c:    WantsCommunity(display=0x%lx)\n",display);
#endif

   superwindow = display->FirstWindow;
   if (FlagIsClear(superwindow->Flags, WANTS_PRIVACY)) 
      return;
   ClearFlag(superwindow->Flags, WANTS_PRIVACY);

   revamp = FALSE;
   if (FindSharedScreen(display, &superwindow->DisplayNewScreen))
      revamp = TRUE;
   else 
      if ( FlagIsSet(superwindow->DisplayModes, TEXT_MODE) 
           && FlagIsClear(superwindow->Flags, MEDIUM_RES)
           && (superwindow->DisplayDepth == 2) )
         revamp = TRUE;

   if (revamp)
      RevampDisplay(display, display->Modes, TRUE, FALSE);
   else
   {
      sext = ((struct ScreenExt *)superwindow->DisplayScreen->UserData);
      ClearFlag(sext->Flags, PRIVATE_SCREENING);
      superwindow->DisplayScreen->DefaultTitle = &ScreenTitle[0];
      SetWindowTitles(display->FirstWindow->DisplayWindow, (UBYTE *)-1,
                      &ScreenTitle[0]);
   }
}
