
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

#define  DBG_GET_EXTRA_SCREEN_ENTER       1
#define  DBG_GET_EXTRA_SCREEN_NEWSCREEN   1
#define  DBG_RELEASE_EXTRA_SCREEN_ENTER   1
#define  DBG_LINK_EXTRA_SCREEN_ENTER      1
#define  DBG_UNLINK_EXTRA_SCREEN_ENTER    1

extern struct Library *IntuitionBase;

/*
 * SW ENG: Backed out SA_Pens on 1-21-92 New Look looks
 *         terrible with the CGA palette.
 */
#if 0
static UWORD Pens1[]={
		0L,
		1L,
		~0L,
		};
static UWORD Pens2[]={
		0L,
		1L,
		2L,
		3L,
		~0L,
		};
static UWORD Pens3[]={
		0L,
		1L,
		2L,
		3L,
		4L,
		5L,
		6L,
		7L,
		~0L,
		};
static UWORD Pens4[]={
		0L,
		1L,
		2L,
		3L,
		4L,
		5L,
		6L,
		7L,
		8L,
		9L,
		10L,
		11L,
		12L,
		13L,
		14L,
		15L,
		~0L,
		};
#endif

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

   if (screen == NULL) {
      if (FlagIsClear(superwindow->Flags, WANTS_PRIVACY))
         newscreen->DefaultTitle = &ScreenTitle[0];
      else  
	     newscreen->DefaultTitle = &ScreenUnsharedTitle[0];

      if(IntuitionBase->lib_Version>=36)
	  { 
	     switch(newscreen->Depth)
		 {
		    /*
			 * SW ENG: Backed out SA_Pens on 1-21-92 New Look looks
			 *         terrible with the CGA palette.
			 */
		    case 1:
               screen = OpenScreenTags(newscreen,/*SA_Pens,Pens1,*/TAG_DONE);
			   break;
		    case 2:
               screen = OpenScreenTags(newscreen,/*SA_Pens,Pens2,*/TAG_DONE);
			   break;
		    case 3:
               screen = OpenScreenTags(newscreen,/*SA_Pens,Pens3,*/TAG_DONE);
			   break;
		    case 4:
               screen = OpenScreenTags(newscreen,/*SA_Pens,Pens4,*/TAG_DONE);
			   break;
		 }
	  } else {
         screen = OpenScreen(newscreen);
	  }
      if (screen) {
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

         if (FlagIsSet(superwindow->Flags, WANTS_PRIVACY)) {
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
         if (FlagIsClear(sext->Flags, PRIVATE_SCREENING)) {
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
#if (DEBUG1 & DBG_FIND_SHARED_SCREEN_ENTER)
   kprintf("shared.c:     FindSharedScreen return %lx\n",returnscreen);
#endif
   return(returnscreen);
}

