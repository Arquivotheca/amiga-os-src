
/* *** menus.c ***************************************************************
 * 
 * Menu Headers for the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 09-may-89   -BK         Changed the Close menu item to Quit
 *                         Added key short cut for Quit
 * 26 Jan 88   -RJ         Added mouse.com support (identified with MOUSE)
 * 3 Apr 86    =RJ Mical=  Created this file
 *
 * **************************************************************************/


#define EGLOBAL_CANCEL TRUE /* this prevents eglobal.c from being included */
#include "zaphod.h"

extern struct TextAttr SafeFont;


#define MENU_HEADER_WIDTH      80
#define MENU_HEADER_LEFT      6
#define EDIT_HEADER_WIDTH      (MENU_HEADER_WIDTH - 3 * CHARACTER_WIDTH)

#define PROJECT_MENU_LEFT      MENU_HEADER_LEFT
#define DISPLAY_MENU_LEFT      PROJECT_MENU_LEFT + MENU_HEADER_WIDTH
#define EDIT_MENU_LEFT            DISPLAY_MENU_LEFT + MENU_HEADER_WIDTH

/*???#define PROJECT_ITEMS_WIDTH      132*/
#define PROJECT_ITEMS_WIDTH      140
#define PROJECT_ITEMS_LEFT      0
#define EDIT_ITEMS_WIDTH      MENU_HEADER_WIDTH + 18
#define EDIT_ITEMS_LEFT            (-3 - 12)
#define DISPLAY_ITEMS_WIDTH      230
#define DISPLAY_ITEMS_LEFT      (-9)

#define SUB_YOFFSET            -2

#define HELP_SUBS_WIDTH            (16 * 8) + 2
#define HELP_SUBS_LEFT            (6 * 8) + 2

#define BORDER_SUBS_COUNT         2
#define BORDER_SUBS_WIDTH         (6 * 8) + 2
#define BORDER_SUBS_LEFT         -(BORDER_SUBS_WIDTH) + 12

#define SIZE_SUBS_COUNT         2
#define SIZE_SUBS_WIDTH         (10 * 8) + 2
#define SIZE_SUBS_LEFT         -(SIZE_SUBS_WIDTH) + 12

#define FREEZE_SUBS_COUNT         2
#define FREEZE_SUBS_WIDTH         (4 * 8) + 2
#define FREEZE_SUBS_LEFT         -(FREEZE_SUBS_WIDTH) + 12

#define CURSOR_SUBS_COUNT          4
#define CURSOR_SUBS_WIDTH          (5 * 8) + 2
#define CURSOR_SUBS_LEFT          -(CURSOR_SUBS_WIDTH) + 12

#define DEPTH_SUBS_COUNT         4
#define DEPTH_SUBS_WIDTH         (9 * 8) + 2
#define DEPTH_SUBS_LEFT         -(DEPTH_SUBS_WIDTH) + 12

#define LOCALE_SUBS_COUNT         2
#define LOCALE_SUBS_WIDTH         (10 * 8) + 2
#define LOCALE_SUBS_LEFT         -(LOCALE_SUBS_WIDTH) + 12

#define PRIORITY_SUBS_COUNT    5
#define PRIORITY_SUBS_WIDTH    (6 * 8) + 2
#define PRIORITY_SUBS_LEFT         -(PRIORITY_SUBS_WIDTH) + 12

#define INTERLACE_SUBS_COUNT   2
#define INTERLACE_SUBS_WIDTH   (5 * 8) + 2
#define INTERLACE_SUBS_LEFT    -(INTERLACE_SUBS_WIDTH) + 12




struct IntuiText HelpSubItemText[] =
   {
   /* "Window Borders" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"Window Borders",
      NULL,
      },

   /* "Window Freeze" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"Window Freeze",
      NULL,
      },

   /* "Multiple Windows" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"Multiple Windows",
      NULL,
      },

   /* "Text Colors" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"Text Colors",
      NULL,
      },

   /* "Display Priority" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"Display Priority",
      NULL,
      },

   /* "Copy and Paste" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"Copy and Paste",
      NULL,
      },

   /* "Keyboard" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"Keyboard",
      NULL,
      },

   };


struct IntuiText FreezeSubItemText[] =
   {
   /* "On" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"  On",
      NULL,
      },

   /* "Off" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)" Off",
      NULL,
      },

   };


struct IntuiText LocaleSubItemText[] =
   {
   /* "Own Screen" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"Own Screen",
      NULL,
      },

   /* "Shared Screen" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"     Share",
      NULL,
      },

   };


struct IntuiText SetCursorBlinkRateSubItemText[] =
   {
   /* "1" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)" 1/2",
      NULL,
      },

   /* "2" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"  1",
      NULL,
      },

   /* "3" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"  2",
      NULL,
      },

   /* "4" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"  4",
      NULL,
      },

   };


struct IntuiText NumberOfTextColorsSubItemText[] =
   {
   /* "1 (2 colors)" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)" 2 Colors",
      NULL,
      },

   /* "2 (4 colors)" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)" 4 Colors",
      NULL,
      },

   /* "3 (8 colors)" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)" 8 Colors",
      NULL,
      },

   /* "4 (16 colors)" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"16 Colors",
      NULL,
      },

   };


struct IntuiText DisplayPrioritySubItemText[] =
   {
   /* "+ 10" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)" + 10",
      NULL,
      },

   /* "+ 5" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"  + 5",
      NULL,
      },

   /* "0" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"    0",
      NULL,
      },

   /* "- 5" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"  - 5",
      NULL,
      },

   /* "- 10" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)" - 10",
      NULL,
      },

   };


struct IntuiText ProjectItemText[PROJECT_ITEMS_COUNT] =
   {
   /* "Close" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"Quit",
      NULL,
      },

   /* "About" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"About",
      NULL,
      },

/* "ADJUST KEY TIMING" */
{
0, 1,
0,
1, 1,
&SafeFont,
(UBYTE *)"Adjust Key Timing",
NULL,
},

   /* "Help" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"Help",
      NULL,
      },

   /* "Restore Settings" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"Restore Settings",
      NULL,
      },

   /* "Save Settings" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"Save Settings",
      NULL,
      },

   };




struct IntuiText EditItemText[EDIT_ITEMS_COUNT] =
   {
   /* "Paste" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"Paste",
      NULL,
      },

   /* "Copy" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"Copy",
      NULL,
      },

   };




struct IntuiText DisplayItemText[DISPLAY_ITEMS_COUNT] =
   {
   /* "Interlace" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"  Interlace",
      NULL,
      },

   /* "Set Display Task Priority" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"  Set Display Task Priority",
      NULL,
      },

   /* "Number of Text Colors" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"  Number of Text Colors",
      NULL,
      },

   /* "Refresh Display" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"  Refresh Display",
      NULL,
      },

   /* "Open Another Window" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"  Open Another Window",
      NULL,
      },

   /* "Set Cursor Blink Rate" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"  Set Cursor Blink Rate",
      NULL,
      },

   /* "Color" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"  Color",
      NULL,
      },

   /* "Mouse Owner" */
/*???      {*/
/*???      0, 1,*/
/*???      0,*/
/*???      1, 1,*/
/*???      &SafeFont,*/
/*???      (UBYTE *)"  Switch Mouse Ownership",*/
/*???      NULL,*/
/*???      },*/

   /* "Window Locale" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"  This Window's Screen",
      NULL,
      },

   /* "Window Freeze" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"  Window Freeze",
      NULL,
      },

   /* "Hide Border" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"  Hide Border",
      NULL,
      },

   /* "Show Border" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"  Show Border",
      NULL,
      },

   /* "Small-Size Window" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"  Small-Size Window",
      NULL,
      },

   /* "Full-Size Window" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"  Full-Size Window",
      NULL,
      },

   };



struct MenuItem HelpSubItems[HELP_SUBITEMS_COUNT] =
   {
   /* "Window Borders" */
      {
      NULL,
      HELP_SUBS_LEFT, SAFE_HEIGHT * 00 + SUB_YOFFSET,
      HELP_SUBS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&HelpSubItemText[00],
      NULL,
      0,
      NULL,
      0,
      },

   /* "Window Freeze" */
      {
      &HelpSubItems[00],
      HELP_SUBS_LEFT, SAFE_HEIGHT * 01 + SUB_YOFFSET,
      HELP_SUBS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&HelpSubItemText[01],
      NULL,
      0,
      NULL,
      0,
      },

   /* "Multiple Windows" */
      {
      &HelpSubItems[01],
      HELP_SUBS_LEFT, SAFE_HEIGHT * 02 + SUB_YOFFSET,
      HELP_SUBS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&HelpSubItemText[02],
      NULL,
      0,
      NULL,
      0,
      },

   /* "Text Colors" */
      {
      &HelpSubItems[02],
      HELP_SUBS_LEFT, SAFE_HEIGHT * 03 + SUB_YOFFSET,
      HELP_SUBS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&HelpSubItemText[03],
      NULL,
      0,
      NULL,
      0,
      },

   /* "Display Priority" */
      {
      &HelpSubItems[03],
      HELP_SUBS_LEFT, SAFE_HEIGHT * 04 + SUB_YOFFSET,
      HELP_SUBS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&HelpSubItemText[04],
      NULL,
      0,
      NULL,
      0,
      },

   /* "Copy and Paste" */
      {
      &HelpSubItems[04],
      HELP_SUBS_LEFT, SAFE_HEIGHT * 05 + SUB_YOFFSET,
      HELP_SUBS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&HelpSubItemText[05],
      NULL,
      0,
      NULL,
      0,
      },

   /* "Keyboard" */
      {
      &HelpSubItems[05],
      HELP_SUBS_LEFT, SAFE_HEIGHT * 06 + SUB_YOFFSET,
      HELP_SUBS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&HelpSubItemText[06],
      NULL,
      0,
      NULL,
      0,
      },

   };



struct MenuItem FreezeSubItems[FREEZE_SUBS_COUNT] =
   {
   /* "On" */
      {
      NULL,
      FREEZE_SUBS_LEFT, SAFE_HEIGHT * 00 + SUB_YOFFSET,
      FREEZE_SUBS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&FreezeSubItemText[00],
      NULL,
      0,
      NULL,
      0,
      },

   /* "Off" */
      {
      &FreezeSubItems[00],
      FREEZE_SUBS_LEFT, SAFE_HEIGHT * 01 + SUB_YOFFSET,
      FREEZE_SUBS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&FreezeSubItemText[01],
      NULL,
      0,
      NULL,
      0,
      },

   };


struct MenuItem LocaleSubItems[LOCALE_SUBS_COUNT] =
   {
   /* "Own Screen" */
      {
      NULL,
      LOCALE_SUBS_LEFT, SAFE_HEIGHT * 00 + SUB_YOFFSET,
      LOCALE_SUBS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&LocaleSubItemText[00],
      NULL,
      0,
      NULL,
      0,
      },

   /* "Shared Screen" */
      {
      &LocaleSubItems[00],
      LOCALE_SUBS_LEFT, SAFE_HEIGHT * 01 + SUB_YOFFSET,
      LOCALE_SUBS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&LocaleSubItemText[01],
      NULL,
      0,
      NULL,
      0,
      },

   };


struct MenuItem SetCursorBlinkRateSubItems[CURSOR_SUBS_COUNT] =
   {
   /* "1" */
      {
      NULL,
      CURSOR_SUBS_LEFT, SAFE_HEIGHT * 00 + SUB_YOFFSET,
      CURSOR_SUBS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&SetCursorBlinkRateSubItemText[00],
      NULL,
      0,
      NULL,
      0,
      },

   /* "2" */
      {
      &SetCursorBlinkRateSubItems[00],
      CURSOR_SUBS_LEFT, SAFE_HEIGHT * 01 + SUB_YOFFSET,
      CURSOR_SUBS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&SetCursorBlinkRateSubItemText[01],
      NULL,
      0,
      NULL,
      0,
      },

   /* "3" */
      {
      &SetCursorBlinkRateSubItems[01],
      CURSOR_SUBS_LEFT, SAFE_HEIGHT * 02 + SUB_YOFFSET,
      CURSOR_SUBS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&SetCursorBlinkRateSubItemText[02],
      NULL,
      0,
      NULL,
      0,
      },

   /* "4" */
      {
      &SetCursorBlinkRateSubItems[02],
      CURSOR_SUBS_LEFT, SAFE_HEIGHT * 03 + SUB_YOFFSET,
      CURSOR_SUBS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&SetCursorBlinkRateSubItemText[03],
      NULL,
      0,
      NULL,
      0,
      },

   };



struct MenuItem NumberOfTextColorsSubItems[DEPTH_SUBS_COUNT] =
   {
   /* "1 (2 colors)" */
      {
      NULL,
      DEPTH_SUBS_LEFT, SAFE_HEIGHT * 00 + SUB_YOFFSET,
      DEPTH_SUBS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&NumberOfTextColorsSubItemText[00],
      NULL,
      0,
      NULL,
      0,
      },

   /* "2 (4 colors)" */
      {
      &NumberOfTextColorsSubItems[00],
      DEPTH_SUBS_LEFT, SAFE_HEIGHT * 01 + SUB_YOFFSET,
      DEPTH_SUBS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&NumberOfTextColorsSubItemText[01],
      NULL,
      0,
      NULL,
      0,
      },

   /* "3 (8 colors)" */
      {
      &NumberOfTextColorsSubItems[01],
      DEPTH_SUBS_LEFT, SAFE_HEIGHT * 02 + SUB_YOFFSET,
      DEPTH_SUBS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&NumberOfTextColorsSubItemText[02],
      NULL,
      0,
      NULL,
      0,
      },

   /* "4 (16 colors)" */
      {
      &NumberOfTextColorsSubItems[02],
      DEPTH_SUBS_LEFT, SAFE_HEIGHT * 03 + SUB_YOFFSET,
      DEPTH_SUBS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&NumberOfTextColorsSubItemText[03],
      NULL,
      0,
      NULL,
      0,
      },

   };


struct MenuItem DisplayPrioritySubItems[PRIORITY_SUBS_COUNT] =
   {
   /* "+ 10" */
      {
      NULL,
      PRIORITY_SUBS_LEFT, SAFE_HEIGHT * 00 + SUB_YOFFSET,
      PRIORITY_SUBS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&DisplayPrioritySubItemText[00],
      NULL,
      0,
      NULL,
      0,
      },

   /* "+ 5" */
      {
      &DisplayPrioritySubItems[00],
      PRIORITY_SUBS_LEFT, SAFE_HEIGHT * 01 + SUB_YOFFSET,
      PRIORITY_SUBS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&DisplayPrioritySubItemText[01],
      NULL,
      0,
      NULL,
      0,
      },

   /* "0" */
      {
      &DisplayPrioritySubItems[01],
      PRIORITY_SUBS_LEFT, SAFE_HEIGHT * 02 + SUB_YOFFSET,
      PRIORITY_SUBS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&DisplayPrioritySubItemText[02],
      NULL,
      0,
      NULL,
      0,
      },

   /* "- 5" */
      {
      &DisplayPrioritySubItems[02],
      PRIORITY_SUBS_LEFT, SAFE_HEIGHT * 03 + SUB_YOFFSET,
      PRIORITY_SUBS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&DisplayPrioritySubItemText[03],
      NULL,
      0,
      NULL,
      0,
      },

   /* "- 10" */
      {
      &DisplayPrioritySubItems[03],
      PRIORITY_SUBS_LEFT, SAFE_HEIGHT * 04 + SUB_YOFFSET,
      PRIORITY_SUBS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&DisplayPrioritySubItemText[04],
      NULL,
      0,
      NULL,
      0,
      },

   };


struct MenuItem ProjectItems[PROJECT_ITEMS_COUNT] =
   {
   /* "Close" */
      {
      NULL,
      PROJECT_ITEMS_LEFT, SAFE_HEIGHT * 5,
      PROJECT_ITEMS_WIDTH, SAFE_HEIGHT,
      (COMMSEQ | ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&ProjectItemText[0],
      NULL,
      'q',
      NULL,
      0,
      },

   /* "About" */
      {
      &ProjectItems[0],
      PROJECT_ITEMS_LEFT, SAFE_HEIGHT * 4,
      PROJECT_ITEMS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&ProjectItemText[1],
      NULL,
      0,
      NULL,
      0,
      },

   /* "ADJUST KEY TIMING" */
      {
      &ProjectItems[1],
      PROJECT_ITEMS_LEFT, SAFE_HEIGHT * 3,
      PROJECT_ITEMS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&ProjectItemText[2],
      NULL,
      0,
      NULL,
      0,
      },

   /* "Help" */
      {
      &ProjectItems[2],
      PROJECT_ITEMS_LEFT, SAFE_HEIGHT * 2,
      PROJECT_ITEMS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&ProjectItemText[3],
      NULL,
      0,
      &HelpSubItems[HELP_SUBITEMS_COUNT - 1],
      0,
      },

   /* "Restore Settings" */
      {
      &ProjectItems[3],
      PROJECT_ITEMS_LEFT, SAFE_HEIGHT * 1,
      PROJECT_ITEMS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&ProjectItemText[4],
      NULL,
      0,
      NULL,
      0,
      },

   /* "Save Settings" */
      {
      &ProjectItems[4],
      PROJECT_ITEMS_LEFT, SAFE_HEIGHT * 0,
      PROJECT_ITEMS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&ProjectItemText[5],
      NULL,
      0,
      NULL,
      0,
      },

   };


struct MenuItem EditItems[EDIT_ITEMS_COUNT] =
   {
   /* "Paste" */
      {
      NULL,
      EDIT_ITEMS_LEFT, SAFE_HEIGHT * 01,
      EDIT_ITEMS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP | COMMSEQ),
      0,
      (APTR)&EditItemText[00],
      NULL,
      '.',
      NULL,
      0,
      },

   /* "Copy" */
      {
      &EditItems[00],
      EDIT_ITEMS_LEFT, SAFE_HEIGHT * 00,
      EDIT_ITEMS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&EditItemText[01],
      NULL,
      0,
      NULL,
      0,
      },

   };



struct MenuItem DisplayItems[DISPLAY_ITEMS_COUNT] =
   {
   /* "Interlace" */
      {
      NULL,
      DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 12,
      DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&DisplayItemText[0],
      NULL,
      0,
      &FreezeSubItems[INTERLACE_SUBS_COUNT - 1],
      0,
      },

   /* "Set Display Task Priority" */
      {
      &DisplayItems[0],
      DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 11,
      DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&DisplayItemText[1],
      NULL,
      0,
      &DisplayPrioritySubItems[PRIORITY_SUBS_COUNT - 1],
      0,
      },

   /* "Number of Text Colors" */
      {
      &DisplayItems[1],
      DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 10,
      DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&DisplayItemText[2],
      NULL,
      0,
      &NumberOfTextColorsSubItems[DEPTH_SUBS_COUNT - 1],
      0,
      },

   /* "Refresh Display" */
      {
      &DisplayItems[2],
      DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 9,
      DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP | COMMSEQ),
      0,
      (APTR)&DisplayItemText[3],
      NULL,
      'r',
      NULL,
      0,
      },

   /* "Open Another Window" */
      {
      &DisplayItems[3],
      DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 8,
      DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP | COMMSEQ),
      0,
      (APTR)&DisplayItemText[4],
      NULL,
      'w',
      NULL,
      0,
      },

   /* "Set Cursor Blink Rate" */
      {
      &DisplayItems[4],
      DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 7,
      DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&DisplayItemText[5],
      NULL,
      0,
      &SetCursorBlinkRateSubItems[CURSOR_SUBS_COUNT - 1],
      0,
      },

   /* "Color" */
      {
      &DisplayItems[5],
      DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 6,
      DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP | COMMSEQ),
      0,
      (APTR)&DisplayItemText[6],
      NULL,
      'c',
      NULL,
      0,
      },

   /* "Mouse Owner" */
/*???      {*/
/*???      &DisplayItems[6],*/
/*???      DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 06,*/
/*???      DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,*/
/*???      (ITEMTEXT | ITEMENABLED | HIGHCOMP | COMMSEQ),*/
/*???      0,*/
/*???      (APTR)&DisplayItemText[7],*/
/*???      NULL,*/
/*???      'm',*/
/*???      NULL,*/
/*???      0,*/
/*???      },*/

   /* "Window Locale" */
      {
      &DisplayItems[6],
      DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 05,
      DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&DisplayItemText[7],
      NULL,
      0,
      &LocaleSubItems[LOCALE_SUBS_COUNT - 1],
      0,
      },

   /* "Freeze" */
      {
      &DisplayItems[7],
      DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 04,
      DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&DisplayItemText[8],
      NULL,
      0,
      &FreezeSubItems[FREEZE_SUBS_COUNT - 1],
      0,
      },

   /* "Hide Border" */
      {
      &DisplayItems[8],
      DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 03,
      DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP | COMMSEQ),
      0,
      (APTR)&DisplayItemText[9],
      NULL,
      'h',
      NULL,
      0,
      },

   /* "Show Border" */
      {
      &DisplayItems[9],
      DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 02,
      DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP | COMMSEQ),
      0,
      (APTR)&DisplayItemText[10],
      NULL,
      'b',
      NULL,
      0,
      },

   /* "Small-Size Window" */
      {
      &DisplayItems[10],
      DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 01,
      DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP | COMMSEQ),
      0,
      (APTR)&DisplayItemText[11],
      NULL,
      's',
      NULL,
      0,
      },

   /* "Full-Size Window" */
      {
      &DisplayItems[11],
      DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 00,
      DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP | COMMSEQ),
      0,
      (APTR)&DisplayItemText[12],
      NULL,
      'f',
      NULL,
      0,
      },

   };



struct Menu MenuHeaders[MENU_HEADERS_COUNT] =
   {
   /* "Display" Menu Header */
      {
      NULL,                   /* NextMenu */
      DISPLAY_MENU_LEFT, 0,      /* Left, dummy Top */
      MENU_HEADER_WIDTH, 0,      /* Width, dummy Height */
      MENUENABLED,            /* Flags */
      "Display",            /* MenuName */
      &DisplayItems[DISPLAY_ITEMS_COUNT - 1],            /* FirstItem */
      0, 0, 0, 0,            /* mysterious variables */
      },
   /* "Edit" Menu Header */
      {
      &MenuHeaders[0],      /* NextMenu */
      EDIT_MENU_LEFT, 0,      /* Left, dummy Top */
      EDIT_HEADER_WIDTH, 0,      /* Width, dummy Height */
      MENUENABLED,            /* Flags */
      "Edit",            /* MenuName */
      &EditItems[EDIT_ITEMS_COUNT - 1], /* FirstItem */
      0, 0, 0, 0,            /* mysterious variables */
      },
   /* "Project" Menu Header */
      {
      &MenuHeaders[1],      /* NextMenu */
      PROJECT_MENU_LEFT, 0,      /* Left, dummy Top */
      MENU_HEADER_WIDTH, 0,      /* Width, dummy Height */
      MENUENABLED,            /* Flags */
      "Project",            /* MenuName */
      &ProjectItems[PROJECT_ITEMS_COUNT - 1], /* FirstItem */
      0, 0, 0, 0,            /* mysterious variables */
      },
   };


