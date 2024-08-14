
/* *** shared.c **************************************************************
 * 
 * Screen Sharing Routines for the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 18 Jan 86   =RJ Mical=  Created this file
 *
 * **************************************************************************/


#include "zaphod.h"

#define  DBG_FIND_SHARED_SCREEN_ENTER  1

struct Screen *FindSharedScreen(display, newscreen)
struct Display *display;
struct NewScreen *newscreen;
{
   struct DisplayList *displaylist;
   struct ScreenExt *sext;
   struct Screen *screen, *returnscreen;
   SHORT width, depth;

#if (DEBUG1 & DBG_FIND_SHARED_SCREEN_ENTER)
   kprintf("shared.c:     FindSharedScreen(display=0x%lx,newscreen=0x%lx)\n",display,newscreen);
#endif

   width = newscreen->Width;
   depth = newscreen->Depth;
   returnscreen = NULL;

   displaylist = (struct DisplayList *)GetDisplayList(display);
   if (MyLock(&displaylist->ScreenListLock))
      {
      sext = displaylist->FirstScreenExt;
      while (sext)
         {
         if (FlagIsClear(sext->Flags, PRIVATE_SCREENING))
            {
            screen = sext->Screen;
            if (screen) if ((screen->Width == width)
                  && (screen->BitMap.Depth == depth)
                  && ((screen->Flags & SCREENTYPE) != WBENCHSCREEN))
               {
               returnscreen = screen;
               goto UNLOCK;
               }
            }

         sext = sext->NextScreenExt;
         }

UNLOCK:
      Unlock(&displaylist->ScreenListLock);
      }

   return(returnscreen);
}

