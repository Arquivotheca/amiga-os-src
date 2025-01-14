
/* *** menuhand.c ************************************************************
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
 * **************************************************************************/

#include "zaphod.h"
#include <graphics/gfxbase.h>
#include <graphics/display.h>
                      
#define  DBG_MENU_EVENT_ENTER       1
#define  DBG_PROJECT_EVENT_ENTER    1
#define  DBG_EDIT_EVENT_ENTER       1
#define  DBG_DISPLAY_EVENT_ENTER    1
#define  DBG_PASTE_CLIP_ENTER       1
#define  DBG_TEST_DOUBLE_MENU_ENTER 1

LONG OldMenuSeconds = 0;
LONG OldMenuMicros = 0;



void MenuEvent(display, code, seconds, micros)
struct Display *display;
USHORT code;
LONG seconds, micros;
{
   struct MenuItem *item;

#if (DEBUG1 & DBG_MENU_EVENT_ENTER)
   kprintf("menuhand.c:   MenuEvent(display=0x%lx,code=0x%lx,seconds=0x%lx,micros=0x%lx)\n",display,code,seconds,micros);
#endif

   if (code == MENUNULL)
         {
         TestDoubleMenu(seconds, micros);
         return;
         }

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



void ProjectEvent(display, code)
struct Display *display;
USHORT code;
{
#if (DEBUG1 & DBG_PROJECT_EVENT_ENTER)
   kprintf("menuhand.c:   ProjectEvent(display=0x%lx,code=0x%lx)\n",display,code);
#endif

   switch(ITEMNUM(code))
      {
      case SAVE_ITEM:
         WriteSettingsFile(display);
         break;
      case RESTORE_ITEM:
         if (ReadSettingsFile(display, TRUE))
            SetColorColors(display, -1);
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
/*???      case ADJUST KEY TIMING:*/
      case ADJUST_ITEM:
         AdjustKeyTiming(display);
         break;
      }
}



void EditEvent(display, code)
struct Display *display;
USHORT code;
{
#if (DEBUG1 & DBG_EDIT_EVENT_ENTER)
   kprintf("menuhand.c:   EditEvent(display=0x%lx,code=0x%lx)\n",display,code);
#endif

   switch(ITEMNUM(code))
      {
      case COPY_ITEM:
            MyAlert(ALERT_COPY_HELP, display);
         break;
      case PASTE_ITEM:
         PasteClip();
         break;
      }
}



void DisplayEvent(display, code)
struct Display *display;
SHORT code;
{
   LONG seconds, micros;
   SHORT i;
   struct SuperWindow *superwindow;
/*
   struct Window *window;
*/
   struct MsgPort *port;
/*
   struct DisplayList *displaylist;
*/
#if (DEBUG1 & DBG_DISPLAY_EVENT_ENTER)
   kprintf("menuhand.c:   DisplayEvent(display=0x%lx,code=0x%lx)\n",display,code);
#endif

   superwindow = display->FirstWindow;

   switch (ITEMNUM(code))
      {
      case FULLSIZE_ITEM:
         if (FlagIsSet(superwindow->Flags, FROZEN_HOSEHEAD))
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
         BorderPatrol(display, BORDER_ON, TRUE);
         if (FlagIsSet(display->Modes, PAL_PRESENCE))
            SetFlag(display->Modes, SQUELCH_NEWSIZE);
         SetFlag(display->CursorFlags, CURSOR_MOVED);
         Unlock(&display->DisplayLock);
         break;
      case HIDEBORDER_ITEM:
         if (FlagIsSet(superwindow->Flags, FROZEN_HOSEHEAD))
            {
            MyAlert(ALERT_FROZEN_BORDER, display);
            break;
            }

         MyLock(&display->DisplayLock);
         BorderPatrol(display, BORDER_OFF, TRUE);
         SetFlag(display->CursorFlags, CURSOR_MOVED);
         Unlock(&display->DisplayLock);
         break;
      case FREEZE_ITEM:
         if (SUBNUM(code))
            {
            SetFlag(superwindow->Flags, FROZEN_HOSEHEAD);
            if (FlagIsSet(display->Modes, COLOR_DISPLAY))
               {
               SetWindowTitles(superwindow->DisplayWindow,
                     &ColorFreezeTitle[0],(UBYTE *) -1);
               superwindow->DisplayBorder.Title = &ColorFreezeTitle[0];
               }
            else
               {
               SetWindowTitles(superwindow->DisplayWindow,
                     &MonoFreezeTitle[0],(UBYTE *) -1);
               superwindow->DisplayBorder.Title = &MonoFreezeTitle[0];
               }
            }
         else if (FlagIsSet(superwindow->Flags, FROZEN_HOSEHEAD))
            {
            ClearFlag(superwindow->Flags, FROZEN_HOSEHEAD);
            GetCRTSetUp(display, TRUE, TRUE);
            if (FlagIsSet(display->Modes, COLOR_DISPLAY))
               {
               SetWindowTitles(superwindow->DisplayWindow,
                     &ColorActiveTitle[0],(UBYTE *) -1);
               superwindow->DisplayBorder.Title = &ColorActiveTitle[0];
               }
            else
               {
               SetWindowTitles(superwindow->DisplayWindow,
                     &MonoActiveTitle[0],(UBYTE *) -1);
               superwindow->DisplayBorder.Title = &MonoActiveTitle[0];
               }
            RenderWindow(display, TRUE, TRUE, TRUE, TRUE);
            }
         break;
      case LOCALE_ITEM:
         if (FlagIsSet(superwindow->Flags, FROZEN_HOSEHEAD))
            {
            MyAlert(ALERT_FROZEN_TEXT, display);
            break;
            }

         if (SUBNUM(code))
            WantsPrivacy(display);
         else
            WantsCommunity(display);
         break;
/*???      case MOUSEOWNER_ITEM:*/
         /* MOUSE */
/*???         TalkWithZaphod(display, TOGGLE_PCMOUSE, FALSE);*/
/*???         break;*/
      case COLOR_ITEM:
         DoColorWindow(display);
         break;
      case CURSOR_ITEM:
         /* Stupid stupid stupid compiler */
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
         while ((port = FindPort(INPUT_PORT_NAME)) 
               == NULL)
            WaitTOF();
         OpenDisplayWindow(display, port, 
               DISPLAY_IDCMP_FLAGS);
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
         if ((display->Modes & COLOR_DISPLAY) == 0) i -= 2;
         else DefaultColorTextDepth = i;

         MyLock(&display->DisplayLock);
         superwindow->TextDepth = i;
         if (display->Modes & TEXT_MODE)
            {
            /* We're currently in text mode.  If this depth is 
             * different from the current depth, we have to close the
             * screen and open a new one, ala Crt().
             */
            if (superwindow->TextDepth != 
                  superwindow->DisplayDepth)
               RevampDisplay(display, display->Modes, 
                     FALSE, FALSE);
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
         if (SUBNUM(code)) SetFlag(GfxBase->system_bplcon0, INTERLACE);
         else ClearFlag(GfxBase->system_bplcon0, INTERLACE);
         RemakeDisplay();
         break;
      }
}


void PasteClip()
{
   UBYTE *ptr;

#if (DEBUG1 & DBG_PASTE_CLIP_ENTER)
   kprintf("menuhand.c:   PasteClip(VOID)\n");
#endif

   ptr = (UBYTE *)ExamineTextClip();
   SendTextToPC(ptr);
}


void TestDoubleMenu(seconds, micros)
LONG seconds, micros;
{
#if (DEBUG1 & DBG_TEST_DOUBLE_MENU_ENTER)
   kprintf("menuhand.c:   TestDoubleMenu(seconds=0x%lx,micros=0x%lx)\n",seconds,micros);
#endif

   if (DoubleClick(OldMenuSeconds, OldMenuMicros, seconds, micros))
      {
      PasteClip();
      OldMenuSeconds = 0;
      OldMenuMicros = 0;
      }
   else
      { 
      OldMenuSeconds = seconds;
      OldMenuMicros = micros;
      }
}

