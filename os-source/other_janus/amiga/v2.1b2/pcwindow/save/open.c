
/* *** open.c ****************************************************************
 * 
 * Window & Screen Open Routines for the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 28 Feb 86   =RJ Mical=  Created this file
 *
 * **************************************************************************/

#include "zaphod.h"
#include <exec/memory.h>

void SetLeftOfCloseGadget();
void SetColorColors();


BOOL   OpenDisplay(display)
REGIST   struct   Display *display;
{
   SHORT   width, depth, mode, displaymodes;
   REGIST   struct   SuperWindow *superwindow;
   struct   Screen *screen;
   BOOL   retvalue;

   retvalue = FALSE;

   MyLock(&display->DisplayLock);

   superwindow = display->FirstWindow;
   displaymodes = display->Modes;

   if (displaymodes & MEDIUM_RES)
      {
      width = 320;
      mode = NULL;
      }
   else
      {
      width = 640;
      mode = HIRES;
      }
   superwindow->DisplayNewScreen.Width = width;
   superwindow->DisplayNewScreen.ViewModes = mode;

   /* Now, use width to calculate the gettin'-small variables. */
   superwindow->LastSmallX = superwindow->LastSmallWidth = width / 2;
   superwindow->LastSmallY = 50;
   superwindow->LastSmallHeight = 100;

   if (displaymodes & TEXT_MODE)
      {
      depth = superwindow->TextDepth;
      }
   else
      {
      /* graphics-mode can be either 4 colors (2 planes) or 2 (1 plane) */
      if (displaymodes & MEDIUM_RES) depth = 2;
      else depth = 1;
      }
   superwindow->DisplayNewScreen.Depth = superwindow->DisplayDepth = depth;

   if ((displaymodes & TEXT_MODE) && (width == 640) && (depth == 2)
         && FlagIsClear(superwindow->Flags, WANTS_PRIVACY))
      {
      /* Oh goodie, we can use the Workbench */
      ClearFlag(display->Modes, OPEN_SCREEN);
      superwindow->DisplayScreen = NULL;
      }
   else
      {
      screen = (struct Screen *)GetExtraScreen(display, superwindow);
      if (superwindow->DisplayScreen = screen)
         SetFlag(display->Modes, OPEN_SCREEN);
      else
         {
         MyAlert(ALERT_NO_MEMORY, NULL);
         ClearFlag(display->Modes, OPEN_SCREEN);
         goto DONE;
         }
      }

   SetColorColors(display, -1);

   if (FlagIsClear(display->Modes, OPEN_SCREEN))
      SetThosePrefs();

   retvalue = TRUE;

DONE:
   Unlock(&display->DisplayLock);
   return(retvalue);
}



BOOL   OpenDisplayWindow(display, port, idcmp)
struct Display *display;
struct MsgPort *port;
ULONG idcmp;
/* If this window is due for a CUSTOMSCREEN, before calling this routine, 
 * the Screen must be initialized
 */
{
   SHORT displaywidth, heightadjust, i;
   struct NewWindow *newwindow;
   struct Window *window;
   struct WindowExtension *windowext;
   struct SuperWindow *superwindow;
   BOOL success;

   window = NULL;
   windowext = NULL;
   success = FALSE;

   MyLock(&display->DisplayLock);

   GetPCDisplay(display);

   superwindow = display->FirstWindow;
   newwindow = &superwindow->DisplayNewWindow;

   if (display->Modes & OPEN_SCREEN)
      {
      ((struct ScreenExt *)superwindow->DisplayScreen->UserData)
            ->UserCount += 1;
      newwindow->Screen = superwindow->DisplayScreen;
      newwindow->Type = CUSTOMSCREEN;
      ScreenToFront(superwindow->DisplayScreen);
      }
   else
      {
      newwindow->Type = WBENCHSCREEN;
      WBenchToFront();
      }

   if (FlagIsSet(display->Modes, COLOR_DISPLAY))
      newwindow->Title = &ColorActiveTitle[0];
   else
      newwindow->Title = &MonoActiveTitle[0];

   if (FlagIsSet(display->Modes, MEDIUM_RES))
      {
      displaywidth = 320;

      superwindow->HorizGadget.LeftEdge = 0;
      superwindow->HorizGadget.TopEdge = -9;
      superwindow->HorizGadget.Width = -11;
      superwindow->HorizGadget.Height = 10;
      superwindow->VertGadget.LeftEdge = -13;
      superwindow->VertGadget.TopEdge = 10;
      superwindow->VertGadget.Width = 14;
      superwindow->VertGadget.Height = -9 -10;
      }
   else
      {
      displaywidth = 640;

      superwindow->HorizGadget.LeftEdge = 0;
      superwindow->HorizGadget.TopEdge = -8;
      superwindow->HorizGadget.Width = -16;
      superwindow->HorizGadget.Height = 9;
      superwindow->VertGadget.LeftEdge = -18;
      superwindow->VertGadget.TopEdge = 10;
      superwindow->VertGadget.Width = 19;
      superwindow->VertGadget.Height = -9 -9;
      }

   newwindow->MaxWidth = displaywidth;
   heightadjust = 0;
   if (FlagIsClear(superwindow->Flags, HEIGHT_INITED))
      {
      if (FlagIsClear(display->Modes, PAL_PRESENCE))
         newwindow->MaxHeight = ZAPHOD_HEIGHT;
      else newwindow->MaxHeight 
            = ZAPHOD_HEIGHT + TopBorderOn
            + superwindow->HorizGadget.Height + 1;
      if (FlagIsClear(display->Modes2, WINDOW_PRESETS))
         newwindow->Height = newwindow->MaxHeight;
      SetFlag(superwindow->Flags, HEIGHT_INITED);
      }
   else
      {
      if (FlagIsSet(display->Modes, PAL_PRESENCE))
         {
         if ( FlagIsSet(superwindow->Flags, WINDOW_WAS_HIRES)
               && FlagIsSet(display->Modes, MEDIUM_RES) )
            heightadjust = 1;
         else if ( FlagIsClear(superwindow->Flags, WINDOW_WAS_HIRES)
               && FlagIsClear(display->Modes, MEDIUM_RES) )
            heightadjust = -1;
         }
      }

   newwindow->Height += heightadjust;
   newwindow->MaxHeight += heightadjust;
   i = GlobalScreenHeight - (newwindow->TopEdge + newwindow->Height);
   if (i < 0) newwindow->TopEdge += i;

   if ( FlagIsSet(superwindow->Flags, WINDOW_WAS_HIRES)
         && FlagIsSet(display->Modes, MEDIUM_RES) )
      {
      newwindow->LeftEdge /= 2;
      newwindow->Width /= 2;
      }

   else if ( FlagIsClear(superwindow->Flags, WINDOW_WAS_HIRES)
         && FlagIsClear(display->Modes, MEDIUM_RES) )
      {
      newwindow->LeftEdge *= 2;
      newwindow->Width *= 2;
      }

   if ( (window = OpenWindow(newwindow)) == NULL)
      {
      MyAlert(ALERT_NO_MEMORY, NULL);
      goto UNLOCK_RETURN;
      }

   if ( (windowext = (struct WindowExtension *)AllocMem(
         sizeof(struct WindowExtension), 
         MEMF_CLEAR)) == NULL)
      {
      MyAlert(ALERT_NO_MEMORY, NULL);
      goto UNLOCK_RETURN;
      }

   window->UserData = (BYTE *)windowext;
   windowext->Window = window;
   windowext->Display = display;
   SetFlag(windowext->Flags, RJ_IS_GROOVY);

   if (window->UserPort = port) ModifyIDCMP(window, idcmp);

   superwindow->DisplayWindow = window;

   SetMenuStrip(window, &MenuHeaders[MENU_HEADERS_COUNT - 1]);

/*??? TOTAL CHEAT FOR HANOVER:  invisible gadget solution pending */
/*??? 28-JAN-88:  RJ sez "ha!"                                    */
if (display->Modes & MEDIUM_RES) 
   {
   window->BorderRight += 2;
   window->GZZWidth -= 2;
   }
else
   {
   window->BorderRight += 3;
   window->GZZWidth -= 3;
   }

   InitDisplayBitMap(display, superwindow, displaywidth);

   SetLeftOfCloseGadget(window);

   ClearFlag(display->CursorFlags, CURSOR_ON);
   SetFlag(display->CursorFlags, CURSOR_MOVED);

   ModifyDisplayProps(display);

   if (display->Modes & BORDER_VISIBLE)
      {
      RenderWindow(display, TRUE, TRUE, TRUE, TRUE);
      TopRightLines(window);
      }
   else
      {
      SetFlag(display->Modes, BORDER_VISIBLE);
      BorderPatrol(display, BORDER_OFF, TRUE);
      }

   SetColorColors(display, CountWBWindows());

   if (FlagIsClear(display->Modes, OPEN_SCREEN))
      {
      ChangeWBWindowCount(1);
      }

   success = TRUE;

UNLOCK_RETURN:
   Unlock(&display->DisplayLock);
   if (NOT success)
      {
      if (window) 
         {
         Forbid();
         if (window->UserPort) DrainPort(window->UserPort);
         window->UserPort = NULL;
         ModifyIDCMP(window, NULL);
         Permit();

         CloseWindow(window);
         }
      if (windowext) 
         FreeMem(windowext, sizeof(struct WindowExtension));
      }
   return(success);
}



void SetLeftOfCloseGadget(window)
struct Window *window;
{
   struct Gadget *gadget;

   gadget = window->FirstGadget;

   while (gadget)
      {
      if ((gadget->GadgetType & ~GADGETTYPE) == CLOSE)
         {
         LeftOfCloseGadget = gadget->LeftEdge + gadget->Width;
         gadget = NULL;
         }
      else gadget = gadget->NextGadget;
      }
}



USHORT *GetColorAddress(modes, depth)
SHORT modes, depth;
{
   USHORT *colortable;
   SHORT i;

   /* If we're in graphics and high-res, use the special 
    * high-res table 
    */
   if (modes & TEXT_MODE)
      {
      /* OK, text mode is easy */ 
      switch (depth)
         {
         case 1:
            colortable = &TextOnePlaneColors[0];
            break;
         case 2:
            colortable = &TextTwoPlaneColors[0];
            break;
         case 3:
            colortable = &TextThreePlaneColors[0];
            break;
         case 4:
         default:
            colortable = &TextFourPlaneColors[0];
            break;
         }
      }
   else
      {
      /* This is graphics.  Now, low or high resolution? */
      if (modes & MEDIUM_RES)
         {
         if (modes & PALETTE_1)  i = 1;
         else i = 0;
         colortable = &LowGraphicsColors[ColorIntenseIndex][i][0];
         }
      else
         /* this is high-res graphics */
         colortable = &HighGraphicsColors[0];
      }

   return(colortable);
}



void SetColorColors(display, wbwindows)
struct Display *display;
SHORT   wbwindows;
/* The wbwindows count specifies the number of workbench windows out there.  
 * This is used when opening/closing displays.  If you're calling this 
 * routine just to set colors (for instance, from GetCRTSetup()) then 
 * use the default value of -1.  
 */
{
   SHORT red, green, blue, i;
   USHORT *colortable;
   struct ViewPort *vport;
   struct SuperWindow *superwindow;

   if (superwindow = display->FirstWindow)
      {
      vport = NULL;
      if (superwindow->DisplayWindow)
         vport = &superwindow->DisplayWindow->WScreen->ViewPort;
      else if (superwindow->DisplayScreen)
         vport = &superwindow->DisplayScreen->ViewPort;

      if (vport)
         {
         if ( FlagIsClear(display->Modes, OPEN_SCREEN)
               &&  (wbwindows == 0) )
            {
            /* This is Workbench viewport, so grab 
             * copy of colors until later
             */
             for (i = 0; i < 4; i++)
               display->WBColors[i] = GetRGB4(
                     vport->ColorMap, i);
            SetFlag(display->Modes2, WBCOLORS_GRABBED);
            }

         colortable = GetColorAddress(display->Modes,
               superwindow->DisplayDepth);
   
         for (i = 0; i < (1 << superwindow->DisplayDepth); i++)
            {
            red   = *(colortable + i) >> 8;
            green = *(colortable + i) >> 4;
            blue  = *(colortable + i);
            SetRGB4(vport, i, red, green, blue);
            }
         }
      }
}


