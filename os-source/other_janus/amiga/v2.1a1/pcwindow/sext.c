
/* *** sext.c ****************************************************************
 * 
 * ScreenExt Routines or the Display Tasks of the Zaphod Project
 *
 * Copyright (C) 1987, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 19 Jan 87   =RJ Mical=  Created this file
 *
 * **************************************************************************/

#include "zaphod.h"
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/intuition.h>

#define  DBG_GET_EXTRA_SCREEN_ENTER       1
#define  DBG_GET_EXTRA_SCREEN_NEWSCREEN   1
#define  DBG_RELEASE_EXTRA_SCREEN_ENTER   1
#define  DBG_LINK_EXTRA_SCREEN_ENTER      1
#define  DBG_UNLINK_EXTRA_SCREEN_ENTER    1

/****i* PCWindow/GetExtraScreen ******************************************
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

struct Screen *GetExtraScreen(display, superwindow)
struct Display *display;
struct SuperWindow *superwindow;
{
   struct Screen *screen;
   struct ScreenExt *sext;
   struct NewScreen *newscreen;

#if (DEBUG1 & DBG_GET_EXTRA_SCREEN_ENTER)
   kprintf("sext.c:       GetExtraScreen(display=0x%lx,superwindow=0x%lx)\n",display,superwindow);
#endif

   newscreen = &superwindow->DisplayNewScreen;

   if (FlagIsClear(superwindow->Flags, WANTS_PRIVACY))
      screen = (struct Screen *)FindSharedScreen(display, newscreen);
   else 
      screen = NULL;
   if (screen == NULL)
   {
      if (FlagIsClear(superwindow->Flags, WANTS_PRIVACY))
         newscreen->DefaultTitle = &ScreenTitle[0];
      else
         newscreen->DefaultTitle = &ScreenUnsharedTitle[0];

#if (DEBUG3 & DBG_GET_EXTRA_SCREEN_NEWSCREEN)
   kprintf("sext.c:       GetExtraScreen: newscreen->Height  =%ld\n",newscreen->Height);
   kprintf(":                             newscreen->Width   =%ld\n",newscreen->Width);
   kprintf(":                             newscreen->LeftEdge=%ld\n",newscreen->LeftEdge);
   kprintf(":                             newscreen->TopEdge =%ld\n",newscreen->TopEdge);
   kprintf(":                             newscreen->Depth   =%ld\n",newscreen->Depth);
#endif
      screen = OpenScreen(newscreen);
      if (screen)
      {
         sext = (struct ScreenExt *)AllocMem(
               sizeof(struct ScreenExt), MEMF_CLEAR);
         if (sext == NULL)
         {
            CloseScreen(screen);
            MyAlert(ALERT_NO_MEMORY, NULL);
            return(NULL);
         }

         screen->UserData = (UBYTE *)sext;
         sext->Screen = screen;

         if (FlagIsSet(superwindow->Flags, WANTS_PRIVACY))
         {
            SetFlag(sext->Flags, PRIVATE_SCREENING);
         }
         LinkExtraScreen(display, screen);
      }
   }
   return(screen);
}

/****i* PCWindow/ReleaseExtraScreen ****************************************
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

VOID ReleaseExtraScreen(display, screen)
struct Display *display;
struct Screen *screen;
{
   struct NewScreen *newscreen;

#if (DEBUG1 & DBG_RELEASE_EXTRA_SCREEN_ENTER)
   kprintf("sext.c:       ReleaseExtraScreen(display=0x%lx,screen=0x%lx)\n",display,screen);
#endif
         
   MyLock(&display->DisplayLock);
   if (display->FirstWindow)
   {
      screen = display->FirstWindow->DisplayScreen;
      display->FirstWindow->DisplayScreen = NULL;
      if (screen)
      {
         newscreen = &display->FirstWindow->DisplayNewScreen;
         newscreen->TopEdge = screen->TopEdge;
         if ( ((struct ScreenExt *)screen->UserData)->UserCount <= 0 )
         {
            UnlinkExtraScreen(display, screen);
            FreeMem(screen->UserData, sizeof(struct ScreenExt));
            CloseScreen(screen);
         }
      }
   }
   Unlock(&display->DisplayLock);
}

/****i* PCWindow/LinkExtraScreen ******************************************
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

VOID LinkExtraScreen(display, screen)
struct Display *display;
struct Screen *screen;
{
   struct DisplayList *displaylist;
   struct ScreenExt *sext;

#if (DEBUG1 & DBG_LINK_EXTRA_SCREEN_ENTER)
   kprintf("sext.c:       LinkExtraScreen(display=0x%lx,screen=0x%lx)\n",display,screen);
#endif

   displaylist = (struct DisplayList *)GetDisplayList(display);
   if (NOT MyLock(&displaylist->ScreenListLock)) 
      return;

   sext = (struct ScreenExt *)screen->UserData;

   sext->NextScreenExt = displaylist->FirstScreenExt;
   displaylist->FirstScreenExt = sext;

   Unlock(&displaylist->ScreenListLock);
}

/****i* PCWindow/UnlinkExtraScreen *****************************************
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

VOID UnlinkExtraScreen(display, screen)
struct Display *display;
struct Screen *screen;
{
   struct DisplayList *displaylist;
   struct ScreenExt *sext, *nextsext;

#if (DEBUG1 & DBG_UNLINK_EXTRA_SCREEN_ENTER)
   kprintf("sext.c:       UnlinkExtraScreen(display=0x%lx,screen=0x%lx)\n",display,screen);
#endif

   displaylist = (struct DisplayList *)GetDisplayList(display);
   if (NOT MyLock(&displaylist->ScreenListLock)) 
      return;

   sext = (struct ScreenExt *)screen->UserData;

   if (displaylist->FirstScreenExt == sext)
      displaylist->FirstScreenExt = sext->NextScreenExt;
   else
   {
      nextsext = displaylist->FirstScreenExt;
      while (nextsext->NextScreenExt != sext)
         nextsext = nextsext->NextScreenExt;
      nextsext->NextScreenExt = sext->NextScreenExt;     
   }

   Unlock(&displaylist->ScreenListLock);
}
