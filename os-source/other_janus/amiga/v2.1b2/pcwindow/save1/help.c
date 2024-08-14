
/* *** help.c ****************************************************************
 * 
 * Help Routines --  Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 23 Apr 86   =RJ Mical=  Created this file
 *
 * **************************************************************************/

#include "zaphod.h"

#define  DBG_HELP_ENTER    1


#define EXTRA_TOP   6
#define EXTRA_LEFT  4

#define HELP_TEXT_COUNT 16

#define HEADER_START      (AUTOTOPEDGE + 4)
#define TEXT_START      (HEADER_START + SAFE_HEIGHT + 4)
#define TEXT_LEFT      (AUTOLEFTEDGE + EXTRA_LEFT)
#define XHELP_HEIGHT      (TEXT_START + (HELP_TEXT_COUNT * SAFE_HEIGHT) + 27)
#define HELP_HEIGHT      200

#define MIN_HELP_HEIGHT      34


struct IntuiText HelpText[HELP_TEXT_COUNT] =
   {
      {
      AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
      AUTOLEFTEDGE + EXTRA_LEFT, HEADER_START,
      &SafeFont,
      NULL,
      NULL,
      },

      {
      AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
      TEXT_LEFT, TEXT_START + SAFE_HEIGHT * 00,
      &SafeFont,
      NULL,
      &HelpText[00],
      },

      {
      AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
      TEXT_LEFT, TEXT_START + SAFE_HEIGHT * 01,
      &SafeFont,
      NULL,
      &HelpText[01],
      },

      {
      AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
      TEXT_LEFT, TEXT_START + SAFE_HEIGHT * 02,
      &SafeFont,
      NULL,
      &HelpText[02],
      },

      {
      AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
      TEXT_LEFT, TEXT_START + SAFE_HEIGHT * 03,
      &SafeFont,
      NULL,
      &HelpText[03],
      },

      {
      AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
      TEXT_LEFT, TEXT_START + SAFE_HEIGHT * 04,
      &SafeFont,
      NULL,
      &HelpText[04],
      },

      {
      AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
      TEXT_LEFT, TEXT_START + SAFE_HEIGHT * 05,
      &SafeFont,
      NULL,
      &HelpText[05],
      },

      {
      AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
      TEXT_LEFT, TEXT_START + SAFE_HEIGHT * 06,
      &SafeFont,
      NULL,
      &HelpText[06],
      },

      {
      AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
      TEXT_LEFT, TEXT_START + SAFE_HEIGHT * 07,
      &SafeFont,
      NULL,
      &HelpText[07],
      },

      {
      AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
      TEXT_LEFT, TEXT_START + SAFE_HEIGHT * 8,
      &SafeFont,
      NULL,
      &HelpText[8],
      },

      {
      AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
      TEXT_LEFT, TEXT_START + SAFE_HEIGHT * 9,
      &SafeFont,
      NULL,
      &HelpText[9],
      },

      {
      AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
      TEXT_LEFT, TEXT_START + SAFE_HEIGHT * 10,
      &SafeFont,
      NULL,
      &HelpText[10],
      },

      {
      AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
      TEXT_LEFT, TEXT_START + SAFE_HEIGHT * 11,
      &SafeFont,
      NULL,
      &HelpText[11],
      },

      {
      AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
      TEXT_LEFT, TEXT_START + SAFE_HEIGHT * 12,
      &SafeFont,
      NULL,
      &HelpText[12],
      },

      {
      AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
      TEXT_LEFT, TEXT_START + SAFE_HEIGHT * 13,
      &SafeFont,
      NULL,
      &HelpText[13],
      },

      {
      AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
      TEXT_LEFT, TEXT_START + SAFE_HEIGHT * 14,
      &SafeFont,
      NULL,
      &HelpText[14],
      },

   };



UBYTE *HelpStrings[HELP_SUBITEMS_COUNT][HELP_TEXT_COUNT] =
   {
     /*(UBYTE *)"|--- Max string length ---------->|", */

/*      {*/
      /* Creators */
/*      (UBYTE *)"PC Bridge and Sidecar created by:",*/
/*      (UBYTE *)"     Steve Beats",*/
/*      (UBYTE *)"     Torsten Burgdorff",*/
/*      (UBYTE *)"     Bob \"Kodiak\" Burns",*/
/*      (UBYTE *)"     Philip Geyer",*/
/*      (UBYTE *)"     Rudolf Goedecke",*/
/*      (UBYTE *)"     Neil Katin",*/
/*      (UBYTE *)"     Bill Kolb",*/
/*      (UBYTE *)"     Dieter Prei\337",*/
/*      (UBYTE *)"     Burkhard Rosin",*/
/*      (UBYTE *)"     Wilfried Rusniok",*/
/*      (UBYTE *)"     Joachim Siedentopf",*/
/*      (UBYTE *)"     Frank-Thomas Ullmann",*/
/*      (UBYTE *)"     Heinz Ullrich",*/
/*      },*/

      {
      /* "Keyboard" */
      (UBYTE *)"Using the Keyboard:",
      (UBYTE *)"Send input to your IBM-PC programs",
      (UBYTE *)"through the Amiga keyboard.",
      (UBYTE *)"",
      (UBYTE *)"There are four PC keys that aren't",
      (UBYTE *)"on the Amiga 1000 keyboard.  But by",
      (UBYTE *)"typing control sequences you are",
      (UBYTE *)"able to transmit these keys:",
      (UBYTE *)"",
      (UBYTE *)"      PC KEY  AMIGA 1000 EQUIVALENT",
      (UBYTE *)" -----------  ---------------------",
      (UBYTE *)"    Num lock  Right AMIGA and N",
      (UBYTE *)" Scroll Lock  Right AMIGA and S",
      (UBYTE *)"               or the HELP Key",
      (UBYTE *)"    Ptr Sc *  Right AMIGA and P",
      (UBYTE *)" + on Keypad  Right AMIGA and +",
      },

      {
      /* Clipboard */
      (UBYTE *)"About Clipboard Copy and Paste:",
      (UBYTE *)"   You can copy text from your PC",
      (UBYTE *)"window into the Amiga Clipboard",
      (UBYTE *)"simply by pressing the left mouse",
      (UBYTE *)"button and moving the mouse.  When",
      (UBYTE *)"the text that you want to copy is",
      (UBYTE *)"highlighted, release the button.",
      (UBYTE *)"   Other programs (e.g. Notepad)",
      (UBYTE *)"let you copy text to and from the",
      (UBYTE *)"Clipboard too.",
      (UBYTE *)"   Clipboard text can be \"pasted\"",
      (UBYTE *)"into your PC window by using the",
      (UBYTE *)"PASTE menu item, or by pressing",
      (UBYTE *)"both right Amiga key and \".\", or",
      (UBYTE *)"by double-clicking the menu button.",
      (UBYTE *)"Text is entered as if by keyboard.",
      },

      {
      /* "Display Priority" */
      (UBYTE *)"About Display Priority:",
      (UBYTE *)"All of the Amiga programs run at a",
      (UBYTE *)"certain \"priority\".  The PC Window",
      (UBYTE *)"display task runs at the normal",
      (UBYTE *)"priority.",
      (UBYTE *)"",
      (UBYTE *)"When the PC display is changing a",
      (UBYTE *)"lot, the PC display emulator",
      (UBYTE *)"will try to consume a lot of the",
      (UBYTE *)"system time and resources.",
      (UBYTE *)"",
      (UBYTE *)"To improve PC performance, select",
      (UBYTE *)"a higher priority.  To improve",
      (UBYTE *)"Amiga performance at the expense",
      (UBYTE *)"of some PC performance, select a",
      (UBYTE *)"lower priority.",
      },

      {
      /* "Number of Text Colors" */
      (UBYTE *)"Number of Text Colors:",
      (UBYTE *)"",
      (UBYTE *)"PC Mono text can appear in up to 4",
      (UBYTE *)"colors,  PC Color text in up to 16",
      (UBYTE *)"colors.  You can elect to have text",
      (UBYTE *)"drawn in fewer colors if you want",
      (UBYTE *)"better performance and are willing",
      (UBYTE *)"to sacrifice some of the colors.",
      (UBYTE *)"",
      (UBYTE *)"The speed improvement from 16-color",
      (UBYTE *)"to 8-color text is significant!",
      (UBYTE *)"",
      (UBYTE *)"Another advantage of using fewer",
      (UBYTE *)"colors is that the display requires",
      (UBYTE *)"less memory.",
      (UBYTE *)"",
      },

      {
      /* "Multiple Windows" */
      (UBYTE *)"About Multiple Windows:",
      (UBYTE *)"You can open multiple windows in",
      (UBYTE *)"any display mode.  You open another",
      (UBYTE *)"window by selecting the menu option",
      (UBYTE *)"OPEN ANOTHER WINDOW.",
      (UBYTE *)"",
      (UBYTE *)"Multiple windows normally appear in",
      (UBYTE *)"the same screen, but you can also",
      (UBYTE *)"move windows to their own display",
      (UBYTE *)"screens by selecting",
      (UBYTE *)"THIS WINDOW'S SCREEN -- OWN SCREEN.",
      (UBYTE *)"",
      (UBYTE *)"You can freeze any of the windows",
      (UBYTE *)"and refer to the window's ",
      (UBYTE *)"information while working in a",
      (UBYTE *)"different window.  ",
      },

      {
      /* Window Freeze */
      (UBYTE *)"About Window Freezing:",
      (UBYTE *)"You can freeze the contents of any",
      (UBYTE *)"window using WINDOW FREEZE -- ON.",
      (UBYTE *)"If you do this, the data displayed",
      (UBYTE *)"in that window won't be updated",
      (UBYTE *)"until you use WINDOW FREEZE -- OFF.",
      (UBYTE *)"After you freeze a window you might",
      (UBYTE *)"do something like open a second",
      (UBYTE *)"window to continue working while",
      (UBYTE *)"referring to the information frozen",
      (UBYTE *)"in the first window.",
      (UBYTE *)"    You can't hide the borders of a",
      (UBYTE *)"frozen window, nor can you move the",
      (UBYTE *)"window to a new screen (by changing",
      (UBYTE *)"the number of text colors, for",
      (UBYTE *)"instance).",
      },

      {
      /* "Borders" */
      (UBYTE *)"About Window Borders:",
      (UBYTE *)"The border has gadgets that you use",
      (UBYTE *)"to size and position your window.",
      (UBYTE *)"You can choose to have the window",
      (UBYTE *)"borders be visible or hidden.  When",
      (UBYTE *)"the borders are hidden, all 80",
      (UBYTE *)"columns and 25 lines are visible.",
      (UBYTE *)"    You can make the borders appear",
      (UBYTE *)"and disappear by double-clicking in",
      (UBYTE *)"the window, or with the SHOW BORDER",
      (UBYTE *)"and HIDE BORDER menus.  Also, when",
      (UBYTE *)"you double-click in a window to",
      (UBYTE *)"hide the border, the window becomes",
      (UBYTE *)"full size.  When you double-click",
      (UBYTE *)"to show borders, the window becomes",
      (UBYTE *)"small size.",
      },

   };



struct IntuiText NextText =
   {
   AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
   AUTOLEFTEDGE, AUTOTOPEDGE,
   &SafeFont,
   (UBYTE *)"Next Help Topic",
   NULL,
   };


SHORT MostRecentHelp = HELP_SUBITEMS_COUNT - 1;


VOID   Help(infoNumber, display)
REGIST   SHORT infoNumber;
REGIST   struct Display *display;
{
   REGIST   struct Window *window;
   REGIST   SHORT i, infoheight;
   REGIST   BOOL hitMeAgain;
   BOOL   hideborders;

#if (DEBUG1 & DBG_HELP_ENTER)
   kprintf("help.c:       Help(infoNumber=0x%lx,display=0x%lx)\n",infoNumber,display);
#endif

   window = NULL;
   hideborders = FALSE;

   if (infoNumber == -1) infoNumber = MostRecentHelp;

   if (display)
      {
      if (display->FirstWindow)
         window = display->FirstWindow->DisplayWindow;
      }

   if (window
         && FlagIsClear(display->Modes, BORDER_VISIBLE)
         && FlagIsClear(display->Modes, PAL_PRESENCE))
      {
      hideborders = TRUE;
      BorderPatrol(display, BORDER_ON, TRUE);
      }

   do
      {
      MostRecentHelp = infoNumber;

      for (i = 0; i < HELP_TEXT_COUNT; i++)
         HelpText[i].IText = HelpStrings[infoNumber][i];

      if (window)
         {
         infoheight = window->WScreen->Height - window->WScreen->TopEdge;
         if (infoheight > HELP_HEIGHT) infoheight = HELP_HEIGHT;
         if (infoheight < MIN_HELP_HEIGHT) return;
         }
      else infoheight = HELP_HEIGHT;

      hitMeAgain = AutoRequest(window, &HelpText[HELP_TEXT_COUNT - 1], 
            &NextText, &OKText, 0, 0, 320, infoheight);

      infoNumber--;
      if (infoNumber < 0) infoNumber = HELP_SUBITEMS_COUNT - 1;
      }
   while (hitMeAgain);

   if (hideborders) BorderPatrol(display, BORDER_OFF, TRUE);
}



