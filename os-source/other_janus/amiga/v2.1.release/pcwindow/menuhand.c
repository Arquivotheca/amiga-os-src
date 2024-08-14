/** menuhand.c ************************************************************
 * 
 * Menu handler for the Display Task of the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 25 Jan 88   -RJ         Added mouse.com support (identified with MOUSE)
 * 9 Apr 86    =RJ=        Created this out of disptask.c
 * 13 Mar 86   =RJ=        Changed this from cdtask.c to disptask.c
 * 20 Feb 86   =RJ Mical=  Created this file
 *   
 *************************************************************************/

#include "zaphod.h"
#include <graphics/gfxbase.h>
#include <graphics/display.h>

extern struct IntuitionBase *IntuitionBase;

static VOID WantsPrivacy(struct Display *display);
static VOID WantsCommunity(struct Display *display);
VOID NewSettings(struct Display *display);

/****i* PCWindow/MenuEvent ******************************************
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

VOID MenuEvent(display, code, seconds, micros)
struct Display *display;
USHORT code;
LONG seconds, micros;
{
   struct MenuItem *item;

   while (code != MENUNULL)
   {
      switch (MENUNUM(code))
      {
         case PROJECT_MENU:
            ProjectEvent(display, code);
            break;
         case EDIT_MENU:
            EditEvent(display, code);
            break;
         case DISPLAY_MENU:
            DisplayEvent(display, code);
            break;
      }
      item = ItemAddress(&MenuHeaders[MENU_HEADERS_COUNT - 1], code);
      code = item->NextSelect;
   }  
}

/****i* PCWindow/ProjectEvent ******************************************
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

VOID ProjectEvent(display, code)
struct Display *display;
USHORT code;
{
   struct SuperWindow *superwindow;

   superwindow = display->FirstWindow;

   switch(ITEMNUM(code))
      {
      case SAVE_ITEM:
         WriteSettingsFile(display);
         break;
      case RESTORE_ITEM:
		 /********************************************/
	     /* NewSettings(display,&LastSavedSettings); */
		 /********************************************/
         if (ReadSettingsFile(display, TRUE))
		 {

         MyLock(&display->DisplayLock);
         superwindow->TextDepth = DefaultColorTextDepth = display->PresetColorDepth;
         if (display->Modes & TEXT_MODE)
         {
            /* We're currently in text mode.  If this depth is 
             * different from the current depth, we have to close the
             * screen and open a new one, ala Crt().
             */
            if (superwindow->TextDepth !=  superwindow->DisplayDepth)
               RevampDisplay(display, display->Modes,  FALSE, FALSE);
         }
         Unlock(&display->DisplayLock);

            SetColorColors(display, -1);
			if(display->FirstWindow)
			{
			   display->FirstWindow->CursorSeconds=CurrentSettings.CursorSeconds;
			   display->FirstWindow->CursorMicros=CurrentSettings.CursorMicros;
			}
			DisplayPriority=CurrentSettings.DisplayPriority;
            NewSettings(display);
		 }
         break;
      case HELP_ITEM:
         Help((SHORT)SUBNUM(code), display);
         break;
      case ABOUT_ITEM:
         MyAlert(RJ_WAS_HERE, display);
         break;
      case CLOSE_ITEM:
         CloseDisplayTask(display);
         break;
      case ADJUST_ITEM:
         AdjustKeyTiming(display);
         break;
      }
}

/****i* PCWindow/EditEvent ******************************************
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

VOID EditEvent(display, code)
struct Display *display;
USHORT code;
{
   UBYTE *ptr;

   switch(ITEMNUM(code))
   {
      case PASTE_ITEM:
         /*PasteClip();*/

         ptr = (UBYTE *)ExamineTextClip();
         SendTextToPC(ptr);
         break;
   }
}

/****i* PCWindow/DisplayEvent ******************************************
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

VOID DisplayEvent(display, code)
struct Display *display;
SHORT code;
{
   LONG seconds, micros;
   SHORT i;
   struct SuperWindow *superwindow;
   struct MsgPort *port;
   static BOOL InterlaceOn=FALSE;

   superwindow = display->FirstWindow;

   switch(ITEMNUM(code))
   {
      case FULLSIZE_ITEM:
         if(FlagIsSet(superwindow->Flags, FROZEN_HOSEHEAD))
         {
            MyAlert(ALERT_FROZEN_TEXT, display);
            break;
         }
         FullSize(display);
         break;
      case SMALLSIZE_ITEM:
         MyLock(&display->DisplayLock);
         SmallSize(display);
         if ((display->Modes & BORDER_VISIBLE) == 0)
            BorderPatrol(display, BORDER_ON, TRUE);
         SetFlag(display->CursorFlags, CURSOR_MOVED);
         Unlock(&display->DisplayLock);
         break;
      case SHOWBORDER_ITEM:
         MyLock(&display->DisplayLock);
         if ((display->Modes & BORDER_VISIBLE) == 0)
		 {
            BorderPatrol(display, BORDER_ON, TRUE);
            if (FlagIsSet(display->Modes, PAL_PRESENCE))
               SetFlag(display->Modes, SQUELCH_NEWSIZE);
            SetFlag(display->CursorFlags, CURSOR_MOVED);
			/*CurrentSettings.BordersOn=TRUE;*/
	     } else {
            if (FlagIsSet(superwindow->Flags, FROZEN_HOSEHEAD))
            {
               MyAlert(ALERT_FROZEN_BORDER, display);
               break;
            }
		    if(FlagIsSet(display->Modes,OPEN_SCREEN))
		    {
               BorderPatrol(display, BORDER_OFF, TRUE);
               SetFlag(display->CursorFlags, CURSOR_MOVED);
	        }
		 }
         Unlock(&display->DisplayLock);
         break;

      case FREEZE_ITEM:
	     if(FlagIsSet(superwindow->Flags,FROZEN_HOSEHEAD))
		 {
            ClearFlag(superwindow->Flags, FROZEN_HOSEHEAD);
            GetCRTSetUp(display, TRUE, TRUE);
            if (FlagIsSet(display->Modes, COLOR_DISPLAY))
            {
               /*SetWindowTitles(superwindow->DisplayWindow,
                  &ColorActiveTitle[0],(UBYTE *) -1L);*/
               superwindow->DisplayBorder.Title = &ColorActiveTitle[0];
            } else {
               /*SetWindowTitles(superwindow->DisplayWindow,
                  &MonoActiveTitle[0],(UBYTE *) -1);*/
               superwindow->DisplayBorder.Title = &MonoActiveTitle[0];
            }
            RenderWindow(display, TRUE, TRUE, TRUE, TRUE);
		 } else {
            SetFlag(superwindow->Flags,FROZEN_HOSEHEAD);
            if(FlagIsSet(display->Modes,COLOR_DISPLAY))
            {
               /*SetWindowTitles((struct Window *)superwindow->DisplayWindow,
			      (char *)&ColorFreezeTitle[0],(char *)(-1L));*/
               superwindow->DisplayBorder.Title = &ColorFreezeTitle[0];
            } else {
/*SetWindowTitles(superwindow->DisplayWindow,&MonoFreezeTitle[0],(UBYTE *)~0L);*/
               superwindow->DisplayBorder.Title = &MonoFreezeTitle[0];
            }
		 }
         break;
      case LOCALE_ITEM:
         if (FlagIsSet(superwindow->Flags, FROZEN_HOSEHEAD))
         {
            MyAlert(ALERT_FROZEN_TEXT, display);
            break;
         }
	     if(!display->FirstWindow->DisplayScreen)
		 {
            WantsPrivacy(display);
			/*CurrentSettings.WorkbenchScreen=FALSE;*/
		 } else {
            WantsCommunity(display);
			/*CurrentSettings.WorkbenchScreen=TRUE;*/
		 }
         break;
      case COLOR_ITEM:
         DoColorWindow(display);
         break;
      case CURSOR_ITEM:
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
         MyLock(&display->DisplayLock);
         GetNewSuperWindow(display, FALSE);
         GetCRTSetUp(display, TRUE, FALSE);
         OpenDisplay(display);
         while ((port = FindPort(INPUT_PORT_NAME))  == NULL)
            WaitTOF();
         OpenDisplayWindow(display, port,  DISPLAY_IDCMP_FLAGS,FALSE);
         Unlock(&display->DisplayLock);
         break;
      case REFRESH_ITEM:
         if (FlagIsSet(superwindow->Flags, FROZEN_HOSEHEAD))
         {
            MyAlert(ALERT_FROZEN_TEXT, display);
            break;
         }
         RefreshDisplay(display);
         break;
      case DEPTH_ITEM:
         if (FlagIsSet(superwindow->Flags, FROZEN_HOSEHEAD))
         {
            MyAlert(ALERT_FROZEN_TEXT, display);
            break;
         }
         i = 4 - SUBNUM(code);
         if ((display->Modes & COLOR_DISPLAY) == 0) 
          i -= 2;
         else 
          DefaultColorTextDepth = i;
         MyLock(&display->DisplayLock);
         superwindow->TextDepth = i;
         if (display->Modes & TEXT_MODE)
         {
            /* We're currently in text mode.  If this depth is 
             * different from the current depth, we have to close the
             * screen and open a new one, ala Crt().
             */
            if (superwindow->TextDepth !=  superwindow->DisplayDepth)
               RevampDisplay(display, display->Modes,  FALSE, FALSE);
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
         CursorPriority = i + CURSORPRIORITY_OFFSET;
         PutNewPriority(display, i);
         break;
      case INTERLACE_ITEM:
		 if(InterlaceOn)
		 {
            ClearFlag(GfxBase->system_bplcon0, INTERLACE);
		    InterlaceOn=FALSE;
	 	 } else {
            SetFlag(GfxBase->system_bplcon0, INTERLACE);
		    InterlaceOn=TRUE;
		 }
         RemakeDisplay();
         break;
   }
}
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

static VOID WantsPrivacy(struct Display *display)
{
   struct SuperWindow *superwindow;
   BOOL closerup;
   struct ScreenExt *sext;

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

static VOID WantsCommunity(struct Display *display)
{
   struct SuperWindow *superwindow;
   struct ScreenExt *sext;
   BOOL revamp;
   struct Screen TScreen;

   GetScreenData(&TScreen,sizeof(struct Screen),WBENCHSCREEN,NULL);

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
           && (superwindow->DisplayDepth == TScreen.BitMap.Depth) )
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

/****i* PCWindow/SmallSize ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*      To take an already open SuperWindow and change some, all, or
*      none of its attributes according to a Settings structure.
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
VOID NewSettings(struct Display *display)
{
   SHORT x, y, width, height;
   SHORT screenheight;
   struct SuperWindow *superwindow;
   struct Window *window;
   BOOL sizexfirst, sizeyfirst, needpermit;
   SHORT i,X,Y,H,W,MODES;

   Forbid();
   needpermit = TRUE;
   MyLock(&display->DisplayLock);

   if ((superwindow = display->FirstWindow) == NULL) goto UNLOCK_RETURN;
   if ((window = superwindow->DisplayWindow) == NULL) goto UNLOCK_RETURN;
#if 0
         /*if (FlagIsSet(superwindow->Flags, FROZEN_HOSEHEAD))
         {
            MyAlert(ALERT_FROZEN_TEXT, display);
            break;
         }*/
         /*i = 4 - SUBNUM(code);*/
		 i=display->PresetColorDepth;
         if ((display->Modes & COLOR_DISPLAY) == 0) 
          i -= 2;
         else 
          DefaultColorTextDepth = i;
         /*MyLock(&display->DisplayLock);*/
         superwindow->TextDepth = i;
         if (display->Modes & TEXT_MODE)
         {
            /* We're currently in text mode.  If this depth is 
             * different from the current depth, we have to close the
             * screen and open a new one, ala Crt().
             */
            if (superwindow->TextDepth !=  superwindow->DisplayDepth)
               RevampDisplay(display, display->Modes,  FALSE, FALSE);
         }
         /*Unlock(&display->DisplayLock);*/
#endif

   screenheight = window->WScreen->Height;

   LockLayerInfo(&window->WScreen->LayerInfo);
   LockLayer((long)&window->WScreen->LayerInfo, window->RPort->Layer);

   if(display->Modes&COLOR_DISPLAY)
   {
      X=display->PresetMonoLeftEdge;
      Y=display->PresetMonoTopEdge;
      W=display->PresetMonoWidth;
      H=display->PresetMonoHeight;
      MODES=display->PresetMonoFlags;
   } else {
      X=display->PresetColorLeftEdge;
      Y=display->PresetColorTopEdge;
      W=display->PresetColorWidth;
      H=display->PresetColorHeight;
      MODES=display->PresetColorFlags;
   }

   x = X + W;
   if (x > window->MaxWidth)
      W = window->MaxWidth-X;

   y = Y + H;
   if (y > screenheight)
      H = screenheight - Y;
 
   x = X - window->LeftEdge;
   y = Y - window->TopEdge;
   width = W - window->Width;
   height = H - window->Height;

   /* Now, if the destination position and the current size will go
    * beyond the screen boundaries, then size first, else move first
    */
   if (X  + window->Width > window->MaxWidth)
      sizexfirst = TRUE;
   else 
      sizexfirst = FALSE;

   if (Y  + window->Height > window->MaxHeight)
      sizeyfirst = TRUE;
   else 
      sizeyfirst = FALSE;

   if (sizexfirst && sizeyfirst) {
      if (width || height) SizeWindow(window, width, height);
   } 
   else {
      if (sizexfirst)
         if (width) SizeWindow(window, width, 0);
      if (sizeyfirst) {
         if (height)SizeWindow(window, 0, height); 
      }
   }

   if ((x) || (y)) {
	MoveWindow(window, x, y);
   }
   if ((NOT sizexfirst) && (NOT sizeyfirst)) {
      if (width || height) SizeWindow(window, width, height);
   } 
   else {
      if (NOT sizexfirst) {
         if (width) SizeWindow(window, width, 0);
      }
      if (NOT sizeyfirst) {
         if (height)SizeWindow(window, 0, height);
      }
   }

   UnlockLayer(window->RPort->Layer);
   UnlockLayerInfo(&window->WScreen->LayerInfo);

   needpermit = FALSE;
   Permit();


   /* Now, try to get around a bug in V1.1 Intuition:  when the SizeWindow()
    * function is called with an argument less than zero, the gadgets aren't
    * redrawn.  No problem, you say, just redraw the gadgets after 
    * calling SizeWindow().  Uh uh, bozo.  Calling SizeWindow() doesn't 
    * mean that your window has been sized, it only means that your window
    * will be sized next time Intuition gets control.  Now don't you wish
    * you had had time to make Intuition a device?  Anyway, the following
    * is totally from Kludge-City.  If all sizing was to smaller sizes,
    * the gadget imagery is sure to be totalled under 1.1, so call
    * WaitTOF() a few times to give Intuition a chance to get in there,
    * and then refresh the gadgets.  This isn't going to work 100% of the
    * time, but it should work 99.99% anyway.  Excuse me ...
    */
   MoveSizeWait();
   if ((width < 0) && (height < 0))
   {
      if (FlagIsSet(display->Modes, BORDER_VISIBLE | PAL_PRESENCE))
      {
         /* MyRefreshWindowFrame(window, display); */
         RefreshWindowFrame(window);
      }
   }

UNLOCK_RETURN:
         /*MyLock(&display->DisplayLock);*/
         if(MODES & PRESET_BORDER_ON)
		 {
            BorderPatrol(display, BORDER_ON, TRUE);
            if (FlagIsSet(display->Modes, PAL_PRESENCE))
               SetFlag(display->Modes, SQUELCH_NEWSIZE);
            SetFlag(display->CursorFlags, CURSOR_MOVED);
			/*CurrentSettings.BordersOn=TRUE;*/
	     } else {
            if (FlagIsSet(superwindow->Flags, FROZEN_HOSEHEAD))
            {
               MyAlert(ALERT_FROZEN_BORDER, display);
            } else {
		       if(FlagIsSet(display->Modes,OPEN_SCREEN))
		       {
                  BorderPatrol(display, BORDER_OFF, TRUE);
                  SetFlag(display->CursorFlags, CURSOR_MOVED);
	           }
			}
		 }
         /*Unlock(&display->DisplayLock);*/
   /*ClearFlag(display->Modes2,FULLSIZE);*/
   SetFlag(display->CursorFlags, CURSOR_MOVED);
   Unlock(&display->DisplayLock); 
   if (needpermit) Permit();
}

