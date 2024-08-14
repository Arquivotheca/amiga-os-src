
/* *** global.c *************************************************************
 * 
 * This file contains the global variable definitions for the Zaphod Library.
 * A copy of this file is kept in eglobal.c, which has all of these variables
 * declared as external.  Any addition to this file should be accompanied
 * by an addition in that file.
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 22 Feb 86   =RJ Mical=  Created this file
 *
 * *************************************************************************/


/*#define EGLOBAL_CANCEL TRUE*/ /* this prevents eglobal.c from being included */
/*#include "zaphod.h"*/



                                          
/* === System Variables ================================================== */

/*struct Preferences   *SavePrefs     = NULL;*/

struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase       *GfxBase       = NULL;

struct LayersBase    *LayersBase    = NULL;
struct JanusBase     *JanusBase     = NULL;
struct JanusAmiga    *JanusAmigaBA  = NULL;
struct JanusAmiga    *JanusAmigaWA  = NULL;

UBYTE  *NormalFont           = NULL;
UBYTE  *UnderlineFont        = NULL;
UBYTE  *FontData             = NULL;
struct Remember *GlobalKey   = NULL;

/*SHORT  GlobalScreenHeight    = 200L;*/
/*UBYTE  SubsysName[]          = "Zaphod";*/

UBYTE  MonoActiveTitle[]     = " PC Monochrome Display ";
UBYTE  MonoInactiveTitle[]   = " PC Monochrome Display (Inactive) ";
UBYTE  MonoFreezeTitle[]     = " PC Monochrome Display (=Frozen=) ";
UBYTE  ColorActiveTitle[]    = " PC Color Display  ";
UBYTE  ColorInactiveTitle[]  = " PC Color Display (Inactive) ";
UBYTE  ColorFreezeTitle[]    = " PC Color Display (=Frozen=) ";
UBYTE  ScreenTitle[]         = "PC Display Screen";
UBYTE  ScreenUnsharedTitle[] = "PC Display Screen (=Not Shared=)";
UBYTE  SidecarSettingsFile[] = "SYS:PC/System/SidecarSettings.Table";

/*UBYTE  SidecarKeysFile[]     = "SYS:PC/System/SidecarKeys.Table";*/
/*UBYTE  ScanCodeFile[]        = "SYS:PC/System/ScanCode.Table";*/

UBYTE  *AllSetPlane;
UBYTE  *AllClearPlane;
SHORT  AuxToolUsers          = 0L;
BOOL   AbandonBlink          = FALSE;
BOOL   DeadenBlink           = FALSE;

struct  DisplayList ZaphodDisplay =
   {
      NULL,
      {
         NULL,
         0L,
         0L,
      },
   };

struct SetupSig *PCKeySig = NULL;
SHORT  DisplayPriority    = 0L;
SHORT  CursorPriority     = CURSORPRIORITY_OFFSET;
SHORT  ColorIntenseIndex;

struct   TextAttr SafeFont =
   {
      (UBYTE *)"topaz.font",
      TOPAZ_EIGHTY,      /* equals 8 */
      0L,
      FPB_ROMFONT,
   };

struct   TextAttr PCFont =
   {
      (UBYTE *)"pcfont.font",
      8L,
      FS_NORMAL,            /* equal 0 */
      /*   FPB_DISKFONT,*/
      0L,
   };

struct MsgPort *AuxToolsFinalPort = NULL;

#if 1
struct   IntuiText OKText =
   {
      AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
      AUTOLEFTEDGE, AUTOTOPEDGE,
      &SafeFont,
      (UBYTE *)"OK", 
      NULL,
   };
#endif

SHORT DefaultColorTextDepth = 4L;

#if 0
SHORT   NullPointer[] = 
   {
      0L, 0L, 0L, 0L, 0L
   };
#endif

LONG (*KeyQueuer)() = NULL;

#if 0
/* === Keycode Translation Stuff ========================================= */
UBYTE PCRawKeyTable[128];

UBYTE PCAltCode          = 0;
UBYTE PCCapsLockCode     = 0;
UBYTE PCCtrlCode         = 0;
UBYTE PCLeftShiftCode    = 0;
UBYTE PCNumLockCode      = 0;
UBYTE PCPlusCode         = 0;
UBYTE PCPtrScrCode       = 0;
UBYTE PCRightShiftCode   = 0;
UBYTE PCScrollLockCode   = 0;
UBYTE PCTildeCode        = 0;
UBYTE PCBarCode          = 0;
UBYTE PCBackDashCode     = 0;
UBYTE PCBackSlashCode    = 0;
UBYTE PCKeypad0Code      = 0xFF;
UBYTE PCKeypad1Code      = 0xFF;
UBYTE PCKeypad2Code      = 0xFF;
UBYTE PCKeypad3Code      = 0xFF;
UBYTE PCKeypad4Code      = 0xFF;
UBYTE PCKeypad5Code      = 0xFF;
UBYTE PCKeypad6Code      = 0xFF;
UBYTE PCKeypad7Code      = 0xFF;
UBYTE PCKeypad8Code      = 0xFF;
UBYTE PCKeypad9Code      = 0xFF;
UBYTE PCKeypadDotCode    = 0xFF;

UBYTE AmigaCapsLockCode  = 0;
UBYTE AmigaLeftAltCode   = 0;
UBYTE AmigaNCode         = 0;
UBYTE AmigaPCode         = 0;
UBYTE AmigaPlusCode      = 0;
UBYTE AmigaRightAltCode  = 0;
UBYTE AmigaSCode         = 0;
UBYTE AmigaPrtScrCode    = 0;
UBYTE AmigaTildeBackDash = 0;
UBYTE AmigaBarBackSlash  = 0;
UBYTE AmigaCursorUp      = 0xFF;
UBYTE AmigaCursorLeft    = 0xFF;
UBYTE AmigaCursorRight   = 0xFF;
UBYTE AmigaCursorDown    = 0xFF;
UBYTE AmigaDelCode       = 0xFF;
#endif


/* === Display Variables ================================================= */
struct Image HorizImage, VertImage;

struct   PropInfo HorizInfo =
   {
      AUTOKNOB | FREEHORIZ, /* Flags */
      0L, 0L,                 /* Pots */
      0xFFFF, 0xFFFF,       /* Dummy body initializations */
      0L,0L,0L,0L,0L,0L,          /* Variables maintained by Intuition */
   };

struct   PropInfo VertInfo =
   {
      AUTOKNOB | FREEVERT, /* Flags */
      0L, 0L,                /* Pots */
      0xFFFF, 0xFFFF,      /* Dummy body initializations */
      0L,0L,0L,0L,0L,0L,         /* Variables maintained by Intuition */
   };

struct   Gadget HorizGadget =
   {
      NULL,              /* NextGadget */
      0L, -8L,             /* LeftEdge, TopEdge (relative) */
      -16L, 9L,            /* Width (relative), Height */
      GADGHNONE | GADGIMAGE | 
	  GRELBOTTOM | GRELWIDTH, /* Flags */
      BOTTOMBORDER | GADGIMMEDIATE | 
	  RELVERIFY | FOLLOWMOUSE, /* Activation */
      PROPGADGET,        /* Gadget Type */
      (APTR)&HorizImage, /* Dummy image for proportional gadget */
      NULL,              /* SelectRender */
      NULL,              /* Text */
      NULL,              /* MutualExclude */
      (APTR)&HorizInfo,  /* Special info for the proportional gadget */
      HORIZ_GADGET,      /* ID */
      NULL,              /* UserData */
   };

struct   Gadget VertGadget =
   {
      &HorizGadget,      /* NextGadget */
      -18L, 10L,           /* LeftEdge (relative), TopEdge */
      19L, -9L -9L,         /* Width, Height (relative) */
      GADGHNONE | GADGIMAGE | 
	  GRELRIGHT | GRELHEIGHT, /* Flags */
      RIGHTBORDER | GADGIMMEDIATE | 
	  RELVERIFY | FOLLOWMOUSE, /* Activation */
      PROPGADGET,        /* Gadget Type */
      (APTR)&VertImage,  /* Dummy image for proportional gadget */
      NULL,              /* SelectRender */
      NULL,              /* Text */
      NULL,              /* MutualExclude */
      (APTR)&VertInfo,   /* Special info for the proportional gadget */
      VERT_GADGET,       /* ID */
      NULL,              /* UserData */
   };

USHORT   TextOnePlaneColors[2] =
   {
      LOW_BLUE,
      HIGH_RED | HIGH_GREEN | HIGH_BLUE,
   };

#if 0
USHORT   TextTwoPlaneColors[4] =
   {
      LOW_BLUE,
      LOW_RED | LOW_GREEN,
      LOW_RED |  LOW_BLUE,
      HIGH_RED | HIGH_GREEN | HIGH_BLUE,
   };
#endif
USHORT   TextTwoPlaneColors[4] =
   {
      LOW_BLUE,
      HIGH_RED | HIGH_GREEN,
      HIGH_RED | HIGH_BLUE,
      HIGH_RED | HIGH_GREEN | HIGH_BLUE,
   };

USHORT   TextThreePlaneColors[8] =
   {
      0L,
      HIGH_BLUE,
      HIGH_GREEN,
      HIGH_GREEN | HIGH_BLUE,
      HIGH_RED,
      HIGH_RED | HIGH_BLUE,
      HIGH_RED | HIGH_GREEN          ,
      HIGH_RED | HIGH_GREEN | HIGH_BLUE,
   };

USHORT   TextFourPlaneColors[16] =
   {
/* Used to be this simple, but it looks awful, so I'm taking over, boys .. */
/*
 *  0,
 *                      LOW_BLUE,
 *            LOW_GREEN          ,
 *            LOW_GREEN |  LOW_BLUE,
 *   LOW_RED                      ,
 *   LOW_RED |                LOW_BLUE,
 *   LOW_RED |      LOW_GREEN          ,
 *   LOW_RED |      LOW_GREEN |  LOW_BLUE,
 *
 *   MIN_RED |      MIN_GREEN |  MIN_BLUE,
 *
 *                     HIGH_BLUE,
 *            HIGH_GREEN          ,
 *            HIGH_GREEN | HIGH_BLUE,
 *  HIGH_RED                      ,
 *  HIGH_RED |               HIGH_BLUE,
 *  HIGH_RED | HIGH_GREEN          ,
 *  HIGH_RED | HIGH_GREEN | HIGH_BLUE,
 */

   0x000,
   0x00B,
   0x0B0,
   0x0BB,
   0xB00,
   0xB0B,
   0xB80,
   0xBBB,

   0x666,
   0x00F,
   0x0F0,
   0x0FF,
   0xF00,
   0xF0F,
   0xFF0,
   0xFFF,

/*
   0x200,
   0x05E,
   0x3C3,
   0x0CC,
   0xC02,
   0xC5C,
   0xCB0,
   0xCCD,
   0x333,
   0x1AF,
   0x5F5,
   0x3FF,
   0xF32,
   0xF8F,
   0xFD1,
   0xFFF,
*/
   };


USHORT   HighGraphicsColors[2] =
   {
     0L,
     LOW_RED | LOW_GREEN | LOW_BLUE,
   };

USHORT   LowGraphicsColors[2][2][4] =
   {
      {
         /* Low Intensity Palettes */
         {
            /* Palette 0 */
            0L,
            LOW_GREEN,
            LOW_RED,
            LOW_RED | LOW_GREEN,
         },

         {
            /* Palette 1 */
            0L,
            LOW_GREEN | LOW_BLUE,
            LOW_RED            | LOW_BLUE,
            LOW_RED | LOW_GREEN | LOW_BLUE,
         },
      },

      {
         /* High Intensity Palettes */
         {
            /* Palette 0 */
            0L,
            HIGH_GREEN,
            HIGH_RED,
            HIGH_RED | HIGH_GREEN,
         },

         {
            /* Palette 1 */
            0L,
            HIGH_GREEN | HIGH_BLUE,
            HIGH_RED | HIGH_BLUE,
            HIGH_RED | HIGH_GREEN | HIGH_BLUE,
         },
      },
   };

struct   NewScreen NewScreen =
   {
   0, 0,                 /* LeftEdge, TopEdge */
   0, STDSCREENHEIGHT, 0,/* Width, Height, Depth */
   0, 1,                 /* Detail/BlockPens */
   NULL,                 /*ViewPort Modes (must set/clear HIRES as needed) */
   CUSTOMSCREEN,
   &SafeFont,            /* Font */
   (UBYTE *)"PC Display Screen",
   NULL,                 /* Gadgets */
   NULL,                 /* CustomBitMap */
   };

struct   NewWindow NewWindow =
   {
   0, 0,                    /* LeftEdge, TopEdge */
   320, 200,                /* Width, Height */
   -1, -1,                  /* Detail/BlockPens */
   NULL,                    /* IDCMP Flags filled in later */
   WINDOWSIZING | WINDOWDRAG | 
   WINDOWDEPTH | WINDOWCLOSE | 
   SIZEBRIGHT | SIZEBBOTTOM  | 
   SMART_REFRESH | ACTIVATE  | 
   REPORTMOUSE,
   &VertGadget,             /* FirstGadget */
   NULL,                    /* Checkmark */
   NULL,                    /* WindowTitle */
   NULL,                    /* Screen */
   NULL,                    /* SuperBitMap */
   96, 30,                  /* MinWidth, MinHeight */
   640, ZAPHOD_HEIGHT,      /* MaxWidth, MaxHeight */
   CUSTOMSCREEN,
   };


/* === Clip Area Variables =============================================== */
SHORT ClipStartX,   ClipStartY;
SHORT ClipCurrentX, ClipCurrentY;



/* === Assorted Task Variables =========================================== */
struct Task *MainTask      = NULL;
struct Task *CursorTask    = NULL;
struct Task *InputTask     = NULL;

ULONG  CursorSuicideSignal = NULL;
ULONG  SL_SuicideSignal    = NULL;
ULONG  InputSuicideSignal  = NULL;

SHORT  TopBorderOn         = NTSC_TOP_ONHEIGHT;
SHORT  TopBorderOff        = NTSC_TOP_OFFHEIGHT;
/*BBBSHORT   BottomBorderOn = NTSC_BOTTOM_ONHEIGHT;*/
/*BBBSHORT   BottomBorderOff = NTSC_BOTTOM_OFFHEIGHT;*/




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

#if 0
#define EGLOBAL_CANCEL TRUE /* this prevents eglobal.c from being included */
#include "zaphod.h"
#endif

/*extern struct TextAttr SafeFont;*/


#define MENU_HEADER_WIDTH      80
#define MENU_HEADER_LEFT       6
#define EDIT_HEADER_WIDTH      (MENU_HEADER_WIDTH - 3 * CHARACTER_WIDTH)

#define PROJECT_MENU_LEFT      MENU_HEADER_LEFT
#define DISPLAY_MENU_LEFT      PROJECT_MENU_LEFT + MENU_HEADER_WIDTH
#define    EDIT_MENU_LEFT      DISPLAY_MENU_LEFT + MENU_HEADER_WIDTH

#define PROJECT_ITEMS_WIDTH    140+(3*8)
#define PROJECT_ITEMS_LEFT     0
#define    EDIT_ITEMS_WIDTH    MENU_HEADER_WIDTH + 18
#define    EDIT_ITEMS_LEFT     (-3 - 12)
#define DISPLAY_ITEMS_WIDTH    230
#define DISPLAY_ITEMS_LEFT     (-9)

#define SUB_YOFFSET            -2

#define      HELP_SUBS_WIDTH   (16 * 8) + 2
#define      HELP_SUBS_LEFT    (6 * 8) + 2

#define    BORDER_SUBS_COUNT   2
#define    BORDER_SUBS_WIDTH   (6 * 8) + 2
#define    BORDER_SUBS_LEFT    -(BORDER_SUBS_WIDTH) + 12

#define      SIZE_SUBS_COUNT   2
#define      SIZE_SUBS_WIDTH   (10 * 8) + 2
#define      SIZE_SUBS_LEFT    -(SIZE_SUBS_WIDTH) + 12

#define    FREEZE_SUBS_COUNT   2
#define    FREEZE_SUBS_WIDTH   (4 * 8) + 2 + CHECKWIDTH
#define    FREEZE_SUBS_LEFT    (DISPLAY_ITEMS_WIDTH) - 12-45

#define    CURSOR_SUBS_COUNT   4
#define    CURSOR_SUBS_WIDTH   (4 * 8) + 2 + CHECKWIDTH
#define    CURSOR_SUBS_LEFT    (DISPLAY_ITEMS_WIDTH) - 12-53+8

#define     DEPTH_SUBS_COUNT   4
#define     DEPTH_SUBS_WIDTH   (9 * 8) + 2 + CHECKWIDTH
#define     DEPTH_SUBS_LEFT    (DISPLAY_ITEMS_WIDTH) - 12-85

#define    LOCALE_SUBS_COUNT   2
#define    LOCALE_SUBS_WIDTH   (10 * 8) + 2 + CHECKWIDTH
#define    LOCALE_SUBS_LEFT    (DISPLAY_ITEMS_WIDTH) - 12-93

#define  PRIORITY_SUBS_COUNT   5
#define  PRIORITY_SUBS_WIDTH   (5 * 8) + 2 + CHECKWIDTH
#define  PRIORITY_SUBS_LEFT    (DISPLAY_ITEMS_WIDTH) - 12-61+8

#define INTERLACE_SUBS_COUNT   2
#define INTERLACE_SUBS_WIDTH   (5 * 8) + 2 + CHECKWIDTH
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

struct IntuiText LocaleSubItemText[] =
   {
   /* "Own Screen" */
      {
      0, 1,
      0,
      1+CHECKWIDTH, 1,
      &SafeFont,
      (UBYTE *)"Own Screen",
      NULL,
      },

   /* "Shared Screen" */
      {
      0, 1,
      0,
      1+CHECKWIDTH, 1,
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
      1+CHECKWIDTH, 1,
      &SafeFont,
      (UBYTE *)"1/2",
      NULL,
      },

   /* "2" */
      {
      0, 1,
      0,
      1+CHECKWIDTH, 1,
      &SafeFont,
      (UBYTE *)" 1",
      NULL,
      },

   /* "3" */
      {
      0, 1,
      0,
      1+CHECKWIDTH, 1,
      &SafeFont,
      (UBYTE *)" 2",
      NULL,
      },

   /* "4" */
      {
      0, 1,
      0,
      1+CHECKWIDTH, 1,
      &SafeFont,
      (UBYTE *)" 4",
      NULL,
      },

   };


struct IntuiText NumberOfTextColorsSubItemText[] =
   {
   /* "1 (2 colors)" */
      {
      0, 1,
      0,
      1+CHECKWIDTH, 1,
      &SafeFont,
      (UBYTE *)" 2 Colors",
      NULL,
      },

   /* "2 (4 colors)" */
      {
      0, 1,
      0,
      1+CHECKWIDTH, 1,
      &SafeFont,
      (UBYTE *)" 4 Colors",
      NULL,
      },

   /* "3 (8 colors)" */
      {
      0, 1,
      0,
      1+CHECKWIDTH, 1,
      &SafeFont,
      (UBYTE *)" 8 Colors",
      NULL,
      },

   /* "4 (16 colors)" */
      {
      0, 1,
      0,
      1+CHECKWIDTH, 1,
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
      1+CHECKWIDTH, 1,
      &SafeFont,
      (UBYTE *)"+ 10",
      NULL,
      },

   /* "+ 5" */
      {
      0, 1,
      0,
      1+CHECKWIDTH, 1,
      &SafeFont,
      (UBYTE *)" + 5",
      NULL,
      },

   /* "0" */
      {
      0, 1,
      0,
      1+CHECKWIDTH, 1,
      &SafeFont,
      (UBYTE *)"   0",
      NULL,
      },

   /* "- 5" */
      {
      0, 1,
      0,
      1+CHECKWIDTH, 1,
      &SafeFont,
      (UBYTE *)" - 5",
      NULL,
      },

   /* "- 10" */
      {
      0, 1,
      0,
      1+CHECKWIDTH, 1,
      &SafeFont,
      (UBYTE *)"- 10",
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
      (UBYTE *)"About...",
      NULL,
      },

   /* "ADJUST KEY TIMING" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"Adjust Key Timing...",
      NULL,
      },

   /* "Help" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"Help               \273",
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
#if 0
   /* "Copy" */
      {
      0, 1,
      0,
      1, 1,
      &SafeFont,
      (UBYTE *)"Copy",
      NULL,
      },
#endif
   };




struct IntuiText DisplayItemText[DISPLAY_ITEMS_COUNT] =
   {
   /* "Interlace" */
      {
      0, 1,
      0,
      1+CHECKWIDTH, 1,
      &SafeFont,
      (UBYTE *)"Interlace",
      NULL,
      },

   /* "Set Display Task Priority" */
      {
      0, 1,
      0,
      1+CHECKWIDTH, 1,
      &SafeFont,
      (UBYTE *)"Display Task Priority    \273",
      NULL,
      },

   /* "Number of Text Colors" */
      {
      0, 1,
      0,
      1+CHECKWIDTH, 1,
      &SafeFont,
      (UBYTE *)"Number of Text Colors    \273",
      NULL,
      },

   /* "Refresh Display" */
      {
      0, 1,
      0,
      1+CHECKWIDTH, 1,
      &SafeFont,
      (UBYTE *)"Refresh Display",
      NULL,
      },

   /* "Open Another Window" */
      {
      0, 1,
      0,
      1+CHECKWIDTH, 1,
      &SafeFont,
      (UBYTE *)"Open Another Window",
      NULL,
      },

   /* "Set Cursor Blink Rate" */
      {
      0, 1,
      0,
      1+CHECKWIDTH, 1,
      &SafeFont,
      (UBYTE *)"Cursor Blink Rate        \273",
      NULL,
      },

   /* "Color" */
      {
      0, 1,
      0,
      1+CHECKWIDTH, 1,
      &SafeFont,
      (UBYTE *)"Color...",
      NULL,
      },

   /* "Window Locale" */
      {
      0, 1,
      0,
      1+CHECKWIDTH, 1,
      &SafeFont,
      (UBYTE *)"Workbench Screen",
      NULL,
      },

   /* "Window Freeze" */
      {
      0, 1,
      0,
      1+CHECKWIDTH, 1,
      &SafeFont,
      (UBYTE *)"Window Freeze",
      NULL,
      },
#if 0
   /* "Hide Border" */
      {
      0, 1,
      0,
      1+CHECKWIDTH, 1,
      &SafeFont,
      (UBYTE *)"Hide Border",
      NULL,
      },
#endif
   /* "Show Border" */
      {
      0, 1,
      0,
      1+CHECKWIDTH, 1,
      &SafeFont,
      (UBYTE *)"Window Borders",
      NULL,
      },

   /* "Small-Size Window" */
      {
      0, 1,
      0,
      1+CHECKWIDTH, 1,
      &SafeFont,
      (UBYTE *)"Small-Size Window",
      NULL,
      },

   /* "Full-Size Window" */
      {
      0, 1,
      0,
      1+CHECKWIDTH, 1,
      &SafeFont,
      (UBYTE *)"Full-Size Window",
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


struct MenuItem SetCursorBlinkRateSubItems[CURSOR_SUBS_COUNT] =
   {
   /* "1" */
      {
      NULL,
      CURSOR_SUBS_LEFT, SAFE_HEIGHT * 00 + SUB_YOFFSET,
      CURSOR_SUBS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP |CHECKIT),
      1+2+4,
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
      (ITEMTEXT | ITEMENABLED | HIGHCOMP |CHECKIT),
      1+2+8,
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
      (ITEMTEXT | ITEMENABLED | HIGHCOMP |CHECKIT),
      1+4+8,
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
      (ITEMTEXT | ITEMENABLED | HIGHCOMP |CHECKIT),
      2+4+8,
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
      (ITEMTEXT | ITEMENABLED | HIGHCOMP |CHECKIT),
      1+2+4,
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
      (ITEMTEXT | ITEMENABLED | HIGHCOMP |CHECKIT),
      1+2+8,
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
      (ITEMTEXT | ITEMENABLED | HIGHCOMP |CHECKIT),
      1+4+8,
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
      (ITEMTEXT | ITEMENABLED | HIGHCOMP |CHECKIT),
      2+4+8,
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
      (ITEMTEXT | ITEMENABLED | HIGHCOMP |CHECKIT),
      1+2+4+8,
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
      (ITEMTEXT | ITEMENABLED | HIGHCOMP |CHECKIT),
      1+2+4+16,
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
      (ITEMTEXT | ITEMENABLED | HIGHCOMP |CHECKIT),
      1+2+8+16,
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
      (ITEMTEXT | ITEMENABLED | HIGHCOMP |CHECKIT),
      1+4+8+16,
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
      (ITEMTEXT | ITEMENABLED | HIGHCOMP |CHECKIT),
      2+4+8+16,
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
      'Q',
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
      EDIT_ITEMS_LEFT, SAFE_HEIGHT * 00,
      EDIT_ITEMS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP | COMMSEQ),
      0,
      (APTR)&EditItemText[00],
      NULL,
      'V',
      NULL,
      0,
      },
   };



struct MenuItem DisplayItems[DISPLAY_ITEMS_COUNT] =
   {
   /* "Interlace" 0 */
      {
      NULL,
      DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 11,
      DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
      (COMMSEQ | ITEMTEXT | ITEMENABLED | HIGHCOMP | MENUTOGGLE | CHECKIT),
      0,
      (APTR)&DisplayItemText[0],
      NULL,
      'I',
      /*&FreezeSubItems[INTERLACE_SUBS_COUNT - 1],*/
	  NULL,
      0,
      },

   /* "Set Display Task Priority"  1*/
      {
      &DisplayItems[0],
      DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 10,
      DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&DisplayItemText[1],
      NULL,
      0,
      &DisplayPrioritySubItems[PRIORITY_SUBS_COUNT - 1],
      0,
      },

   /* "Number of Text Colors" 2*/
      {
      &DisplayItems[1],
      DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 9,
      DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&DisplayItemText[2],
      NULL,
      0,
      &NumberOfTextColorsSubItems[DEPTH_SUBS_COUNT - 1],
      0,
      },

   /* "Refresh Display" 3*/
      {
      &DisplayItems[2],
      DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 8,
      DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP | COMMSEQ),
      0,
      (APTR)&DisplayItemText[3],
      NULL,
      'R',
      NULL,
      0,
      },

   /* "Open Another Window" 4*/
      {
      &DisplayItems[3],
      DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 7,
      DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP | COMMSEQ),
      0,
      (APTR)&DisplayItemText[4],
      NULL,
      'W',
      NULL,
      0,
      },

   /* "Set Cursor Blink Rate" 5*/
      {
      &DisplayItems[4],
      DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 6,
      DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&DisplayItemText[5],
      NULL,
      0,
      &SetCursorBlinkRateSubItems[CURSOR_SUBS_COUNT - 1],
      0,
      },

   /* "Color" 6*/
      {
      &DisplayItems[5],
      DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 5,
      DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP | COMMSEQ),
      0,
      (APTR)&DisplayItemText[6],
      NULL,
      'C',
      NULL,
      0,
      },

   /* "Window Locale" 7*/
      {
      &DisplayItems[6],
      DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 4,
      DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
      (MENUTOGGLE | CHECKIT | ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&DisplayItemText[7],
      NULL,
      0,
      NULL/*&LocaleSubItems[LOCALE_SUBS_COUNT - 1]*/,
      0,
      },

   /* "Freeze" 8*/
      {
      &DisplayItems[7],
      DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 03,
      DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
      (MENUTOGGLE | CHECKIT | ITEMTEXT | ITEMENABLED | HIGHCOMP),
      0,
      (APTR)&DisplayItemText[8],
      NULL,
      0,
      NULL/*&FreezeSubItems[FREEZE_SUBS_COUNT - 1]*/,
      0,
      },
   /* "Window Borders" 9*/
      {
      &DisplayItems[8],
      DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 02,
      DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
      (MENUTOGGLE | CHECKIT | ITEMTEXT | ITEMENABLED | HIGHCOMP | COMMSEQ),
      0,
      (APTR)&DisplayItemText[9],
      NULL,
      'B',
      NULL,
      0,
      },

   /* "Small-Size Window" 10*/
      {
      &DisplayItems[9],
      DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 01,
      DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP | COMMSEQ),
      0,
      (APTR)&DisplayItemText[10],
      NULL,
      'T',
      NULL,
      0,
      },

   /* "Full-Size Window" 11*/
      {
      &DisplayItems[10],
      DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 00,
      DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
      (ITEMTEXT | ITEMENABLED | HIGHCOMP | COMMSEQ),
      0,
      (APTR)&DisplayItemText[11],
      NULL,
      'F',
      NULL,
      0,
      },

   };



struct Menu MenuHeaders[MENU_HEADERS_COUNT] =
   {
   /* "Edit" Menu Header */
      {
      NULL,                   /* NextMenu */
      EDIT_MENU_LEFT, 0,      /* Left, dummy Top */
      EDIT_HEADER_WIDTH, 0,      /* Width, dummy Height */
      MENUENABLED,            /* Flags */
      "Edit",            /* MenuName */
      &EditItems[EDIT_ITEMS_COUNT - 1], /* FirstItem */
      0, 0, 0, 0,            /* mysterious variables */
      },
   /* "Display" Menu Header */
      {
      &MenuHeaders[0],      /* NextMenu */
      DISPLAY_MENU_LEFT, 0,      /* Left, dummy Top */
      MENU_HEADER_WIDTH, 0,      /* Width, dummy Height */
      MENUENABLED,            /* Flags */
      "Display",            /* MenuName */
      &DisplayItems[DISPLAY_ITEMS_COUNT - 1],            /* FirstItem */
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


LONG     KeyDelaySeconds = 0L;
LONG     KeyDelayMicros = 5000L;

/***************************************************************************
 * 
 * Ascii Translation Tables  --  Zaphod Project
 *
 * Copyright (C) 1988, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY    Name         Description
 * ---------  -----------  -------------------------------------------------
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 2 Nov 88   =RJ Mical=   Created this file
 *
 **************************************************************************/

/*#include <exec/types.h>*/

/* Conversion table translates between Amiga and IBM character set 
 * PC char is index, entry is char to send to Amiga
 */
UBYTE   PCToAmigaTable[256] = 
   {
   0x00,   /* Entry 00 */
   0x01,   /* Entry 01 */
   0x02,   /* Entry 02 */
   0x03,   /* Entry 03 */
   0x04,   /* Entry 04 */
   0x05,   /* Entry 05 */
   0x06,   /* Entry 06 */
   0x07,   /* Entry 07 */
   0x08,   /* Entry 08 */
   0x09,   /* Entry 09 */
   0x0a,   /* Entry 0a */
   0x0b,   /* Entry 0b */
   0x0c,   /* Entry 0c */
   0x0d,   /* Entry 0d */
   0x0e,   /* Entry 0e */
   0x0f,   /* Entry 0f */
   0x10,   /* Entry 10 */
   0x11,   /* Entry 11 */
   0x12,   /* Entry 12 */
   0x13,   /* Entry 13 */
   0x14,   /* Entry 14 */
   0x15,   /* Entry 15 */
   0x16,   /* Entry 16 */
   0x17,   /* Entry 17 */
   0x18,   /* Entry 18 */
   0x19,   /* Entry 19 */
   0x1a,   /* Entry 1a */
   0x1b,   /* Entry 1b */
   0x1c,   /* Entry 1c */
   0x1d,   /* Entry 1d */
   0x1e,   /* Entry 1e */
   0x1f,   /* Entry 1f */
   0x20,   /* Entry 20 */
   0x21,   /* Entry 21 */
   0x22,   /* Entry 22 */
   0x23,   /* Entry 23 */
   0x24,   /* Entry 24 */
   0x25,   /* Entry 25 */
   0x26,   /* Entry 26 */
   0x27,   /* Entry 27 */
   0x28,   /* Entry 28 */
   0x29,   /* Entry 29 */
   0x2a,   /* Entry 2a */
   0x2b,   /* Entry 2b */
   0x2c,   /* Entry 2c */
   0x2d,   /* Entry 2d */
   0x2e,   /* Entry 2e */
   0x2f,   /* Entry 2f */
   0x30,   /* Entry 30 */
   0x31,   /* Entry 31 */
   0x32,   /* Entry 32 */
   0x33,   /* Entry 33 */
   0x34,   /* Entry 34 */
   0x35,   /* Entry 35 */
   0x36,   /* Entry 36 */
   0x37,   /* Entry 37 */
   0x38,   /* Entry 38 */
   0x39,   /* Entry 39 */
   0x3a,   /* Entry 3a */
   0x3b,   /* Entry 3b */
   0x3c,   /* Entry 3c */
   0x3d,   /* Entry 3d */
   0x3e,   /* Entry 3e */
   0x3f,   /* Entry 3f */
   0x40,   /* Entry 40 */
   0x41,   /* Entry 41 */
   0x42,   /* Entry 42 */
   0x43,   /* Entry 43 */
   0x44,   /* Entry 44 */
   0x45,   /* Entry 45 */
   0x46,   /* Entry 46 */
   0x47,   /* Entry 47 */
   0x48,   /* Entry 48 */
   0x49,   /* Entry 49 */
   0x4a,   /* Entry 4a */
   0x4b,   /* Entry 4b */
   0x4c,   /* Entry 4c */
   0x4d,   /* Entry 4d */
   0x4e,   /* Entry 4e */
   0x4f,   /* Entry 4f */
   0x50,   /* Entry 50 */
   0x51,   /* Entry 51 */
   0x52,   /* Entry 52 */
   0x53,   /* Entry 53 */
   0x54,   /* Entry 54 */
   0x55,   /* Entry 55 */
   0x56,   /* Entry 56 */
   0x57,   /* Entry 57 */
   0x58,   /* Entry 58 */
   0x59,   /* Entry 59 */
   0x5a,   /* Entry 5a */
   0x5b,   /* Entry 5b */
   0x5c,   /* Entry 5c */
   0x5d,   /* Entry 5d */
   0x5e,   /* Entry 5e */
   0x5f,   /* Entry 5f */
   0x60,   /* Entry 60 */
   0x61,   /* Entry 61 */
   0x62,   /* Entry 62 */
   0x63,   /* Entry 63 */
   0x64,   /* Entry 64 */
   0x65,   /* Entry 65 */
   0x66,   /* Entry 66 */
   0x67,   /* Entry 67 */
   0x68,   /* Entry 68 */
   0x69,   /* Entry 69 */
   0x6a,   /* Entry 6a */
   0x6b,   /* Entry 6b */
   0x6c,   /* Entry 6c */
   0x6d,   /* Entry 6d */
   0x6e,   /* Entry 6e */
   0x6f,   /* Entry 6f */
   0x70,   /* Entry 70 */
   0x71,   /* Entry 71 */
   0x72,   /* Entry 72 */
   0x73,   /* Entry 73 */
   0x74,   /* Entry 74 */
   0x75,   /* Entry 75 */
   0x76,   /* Entry 76 */
   0x77,   /* Entry 77 */
   0x78,   /* Entry 78 */
   0x79,   /* Entry 79 */
   0x7a,   /* Entry 7a */
   0x7b,   /* Entry 7b */
   0x7c,   /* Entry 7c */
   0x7d,   /* Entry 7d */
   0x7e,   /* Entry 7e */
   0x7f,   /* Entry 7f */
   0xc7,   /* Entry 80 */
   0xfc,   /* Entry 81 */
   0xe9,   /* Entry 82 */
   0xe2,   /* Entry 83 */
   0xe4,   /* Entry 84 */
   0xe0,   /* Entry 85 */
   0xe5,   /* Entry 86 */
   0xe7,   /* Entry 87 */
   0xea,   /* Entry 88 */
   0xeb,   /* Entry 89 */
   0xe8,   /* Entry 8a */
   0xef,   /* Entry 8b */
   0xee,   /* Entry 8c */
   0xec,   /* Entry 8d */
   0xc4,   /* Entry 8e */
   0xc5,   /* Entry 8f */
   0xc9,   /* Entry 90 */
   0xe6,   /* Entry 91 */
   0xc6,   /* Entry 92 */
   0xf4,   /* Entry 93 */
   0xf6,   /* Entry 94 */
   0xf2,   /* Entry 95 */
   0xfb,   /* Entry 96 */
   0xf9,   /* Entry 97 */
   0xff,   /* Entry 98 */
   0xd6,   /* Entry 99 */
   0xdc,   /* Entry 9a */
   0xa2,   /* Entry 9b */
   0xa3,   /* Entry 9c */
   0xa5,   /* Entry 9d */
   0x7f,   /* Entry 9e */
   0x7f,   /* Entry 9f */
   0xe1,   /* Entry a0 */
   0xed,   /* Entry a1 */
   0xf3,   /* Entry a2 */
   0xfa,   /* Entry a3 */
   0xf1,   /* Entry a4 */
   0xd1,   /* Entry a5 */
   0xaa,   /* Entry a6 */
   0xba,   /* Entry a7 */
   0xbf,   /* Entry a8 */
   0x7f,   /* Entry a9 */
   0xac,   /* Entry aa */
   0xbd,   /* Entry ab */
   0xbc,   /* Entry ac */
   0xa1,   /* Entry ad */
   0xab,   /* Entry ae */
   0xbb,   /* Entry af */
   0x7f,   /* Entry b0 */
   0x7f,   /* Entry b1 */
   0x7f,   /* Entry b2 */
   0x7f,   /* Entry b3 */
   0x7f,   /* Entry b4 */
   0x7f,   /* Entry b5 */
   0x7f,   /* Entry b6 */
   0x7f,   /* Entry b7 */
   0x7f,   /* Entry b8 */
   0x7f,   /* Entry b9 */
   0x7f,   /* Entry ba */
   0x7f,   /* Entry bb */
   0x7f,   /* Entry bc */
   0x7f,   /* Entry bd */
   0x7f,   /* Entry be */
   0x7f,   /* Entry bf */
   0x7f,   /* Entry c0 */
   0x7f,   /* Entry c1 */
   0x7f,   /* Entry c2 */
   0x7f,   /* Entry c3 */
   0x7f,   /* Entry c4 */
   0x7f,   /* Entry c5 */
   0x7f,   /* Entry c6 */
   0x7f,   /* Entry c7 */
   0x7f,   /* Entry c8 */
   0x7f,   /* Entry c9 */
   0x7f,   /* Entry ca */
   0x7f,   /* Entry cb */
   0x7f,   /* Entry cc */
   0x7f,   /* Entry cd */
   0x7f,   /* Entry ce */
   0x7f,   /* Entry cf */
   0x7f,   /* Entry d0 */
   0x7f,   /* Entry d1 */
   0x7f,   /* Entry d2 */
   0x7f,   /* Entry d3 */
   0x7f,   /* Entry d4 */
   0x7f,   /* Entry d5 */
   0x7f,   /* Entry d6 */
   0x7f,   /* Entry d7 */
   0x7f,   /* Entry d8 */
   0x7f,   /* Entry d9 */
   0x7f,   /* Entry da */
   0x7f,   /* Entry db */
   0x7f,   /* Entry dc */
   0x7f,   /* Entry dd */
   0x7f,   /* Entry de */
   0x7f,   /* Entry df */
   0x7f,   /* Entry e0 */
   0xdf,   /* Entry e1 */
   0x7f,   /* Entry e2 */
   0x7f,   /* Entry e3 */
   0x7f,   /* Entry e4 */
   0x7f,   /* Entry e5 */
   0xb5,   /* Entry e6 */
   0x7f,   /* Entry e7 */
   0x7f,   /* Entry e8 */
   0x7f,   /* Entry e9 */
   0x7f,   /* Entry ea */
   0xf0,   /* Entry eb */
   0x7f,   /* Entry ec */
   0xf8,   /* Entry ed */
   0x7f,   /* Entry ee */
   0x7f,   /* Entry ef */
   0x7f,   /* Entry f0 */
   0xb1,   /* Entry f1 */
   0x7f,   /* Entry f2 */
   0x7f,   /* Entry f3 */
   0x7f,   /* Entry f4 */
   0x7f,   /* Entry f5 */
   0xf7,   /* Entry f6 */
   0x7f,   /* Entry f7 */
   0xb0,   /* Entry f8 */
   0xb7,   /* Entry f9 */
   0x7f,   /* Entry fa */
   0x7f,   /* Entry fb */
   0x7f,   /* Entry fc */
   0xb2,   /* Entry fd */
   0xaf,   /* Entry fe */
   0x7f,   /* Entry ff */
};

/* Conversion table translates between Amiga and IBM character set
 * Amiga char is index, entry is char to send to PC
 */
UBYTE   AmigaToPCTable[256] = 
   {
   0x00,   /* Entry 00 */
   0x01,   /* Entry 01 */
   0x02,   /* Entry 02 */
   0x03,   /* Entry 03 */
   0x04,   /* Entry 04 */
   0x05,   /* Entry 05 */
   0x06,   /* Entry 06 */
   0x07,   /* Entry 07 */
   0x08,   /* Entry 08 */
   0x09,   /* Entry 09 */
   0x0a,   /* Entry 0a */
   0x0b,   /* Entry 0b */
   0x0c,   /* Entry 0c */
   0x0d,   /* Entry 0d */
   0x0e,   /* Entry 0e */
   0x0f,   /* Entry 0f */
   0x10,   /* Entry 10 */
   0x11,   /* Entry 11 */
   0x12,   /* Entry 12 */
   0x13,   /* Entry 13 */
   0x14,   /* Entry 14 */
   0x15,   /* Entry 15 */
   0x16,   /* Entry 16 */
   0x17,   /* Entry 17 */
   0x18,   /* Entry 18 */
   0x19,   /* Entry 19 */
   0x1a,   /* Entry 1a */
   0x1b,   /* Entry 1b */
   0x1c,   /* Entry 1c */
   0x1d,   /* Entry 1d */
   0x1e,   /* Entry 1e */
   0x1f,   /* Entry 1f */
   0x20,   /* Entry 20 */
   0x21,   /* Entry 21 */
   0x22,   /* Entry 22 */
   0x23,   /* Entry 23 */
   0x24,   /* Entry 24 */
   0x25,   /* Entry 25 */
   0x26,   /* Entry 26 */
   0x27,   /* Entry 27 */
   0x28,   /* Entry 28 */
   0x29,   /* Entry 29 */
   0x2a,   /* Entry 2a */
   0x2b,   /* Entry 2b */
   0x2c,   /* Entry 2c */
   0x2d,   /* Entry 2d */
   0x2e,   /* Entry 2e */
   0x2f,   /* Entry 2f */
   0x30,   /* Entry 30 */
   0x31,   /* Entry 31 */
   0x32,   /* Entry 32 */
   0x33,   /* Entry 33 */
   0x34,   /* Entry 34 */
   0x35,   /* Entry 35 */
   0x36,   /* Entry 36 */
   0x37,   /* Entry 37 */
   0x38,   /* Entry 38 */
   0x39,   /* Entry 39 */
   0x3a,   /* Entry 3a */
   0x3b,   /* Entry 3b */
   0x3c,   /* Entry 3c */
   0x3d,   /* Entry 3d */
   0x3e,   /* Entry 3e */
   0x3f,   /* Entry 3f */
   0x40,   /* Entry 40 */
   0x41,   /* Entry 41 */
   0x42,   /* Entry 42 */
   0x43,   /* Entry 43 */
   0x44,   /* Entry 44 */
   0x45,   /* Entry 45 */
   0x46,   /* Entry 46 */
   0x47,   /* Entry 47 */
   0x48,   /* Entry 48 */
   0x49,   /* Entry 49 */
   0x4a,   /* Entry 4a */
   0x4b,   /* Entry 4b */
   0x4c,   /* Entry 4c */
   0x4d,   /* Entry 4d */
   0x4e,   /* Entry 4e */
   0x4f,   /* Entry 4f */
   0x50,   /* Entry 50 */
   0x51,   /* Entry 51 */
   0x52,   /* Entry 52 */
   0x53,   /* Entry 53 */
   0x54,   /* Entry 54 */
   0x55,   /* Entry 55 */
   0x56,   /* Entry 56 */
   0x57,   /* Entry 57 */
   0x58,   /* Entry 58 */
   0x59,   /* Entry 59 */
   0x5a,   /* Entry 5a */
   0x5b,   /* Entry 5b */
   0x5c,   /* Entry 5c */
   0x5d,   /* Entry 5d */
   0x5e,   /* Entry 5e */
   0x5f,   /* Entry 5f */
   0x60,   /* Entry 60 */
   0x61,   /* Entry 61 */
   0x62,   /* Entry 62 */
   0x63,   /* Entry 63 */
   0x64,   /* Entry 64 */
   0x65,   /* Entry 65 */
   0x66,   /* Entry 66 */
   0x67,   /* Entry 67 */
   0x68,   /* Entry 68 */
   0x69,   /* Entry 69 */
   0x6a,   /* Entry 6a */
   0x6b,   /* Entry 6b */
   0x6c,   /* Entry 6c */
   0x6d,   /* Entry 6d */
   0x6e,   /* Entry 6e */
   0x6f,   /* Entry 6f */
   0x70,   /* Entry 70 */
   0x71,   /* Entry 71 */
   0x72,   /* Entry 72 */
   0x73,   /* Entry 73 */
   0x74,   /* Entry 74 */
   0x75,   /* Entry 75 */
   0x76,   /* Entry 76 */
   0x77,   /* Entry 77 */
   0x78,   /* Entry 78 */
   0x79,   /* Entry 79 */
   0x7a,   /* Entry 7a */
   0x7b,   /* Entry 7b */
   0x7c,   /* Entry 7c */
   0x7d,   /* Entry 7d */
   0x7e,   /* Entry 7e */
/*!*/   0x20,   /* Entry 7f */
/*!*/   0x20,   /* Entry 80 */
/*!*/   0x20,   /* Entry 81 */
/*!*/   0x20,   /* Entry 82 */
/*!*/   0x20,   /* Entry 83 */
/*!*/   0x20,   /* Entry 84 */
/*!*/   0x20,   /* Entry 85 */
/*!*/   0x20,   /* Entry 86 */
/*!*/   0x20,   /* Entry 87 */
/*!*/   0x20,   /* Entry 88 */
/*!*/   0x20,   /* Entry 89 */
/*!*/   0x20,   /* Entry 8a */
/*!*/   0x20,   /* Entry 8b */
/*!*/   0x20,   /* Entry 8c */
/*!*/   0x20,   /* Entry 8d */
/*!*/   0x20,   /* Entry 8e */
/*!*/   0x20,   /* Entry 8f */
/*!*/   0x20,   /* Entry 90 */
/*!*/   0x20,   /* Entry 91 */
/*!*/   0x20,   /* Entry 92 */
/*!*/   0x20,   /* Entry 93 */
/*!*/   0x20,   /* Entry 94 */
/*!*/   0x20,   /* Entry 95 */
/*!*/   0x20,   /* Entry 96 */
/*!*/   0x20,   /* Entry 97 */
/*!*/   0x20,   /* Entry 98 */
/*!*/   0x20,   /* Entry 99 */
/*!*/   0x20,   /* Entry 9a */
/*!*/   0x20,   /* Entry 9b */
/*!*/   0x20,   /* Entry 9c */
/*!*/   0x20,   /* Entry 9d */
/*!*/   0x20,   /* Entry 9e */
/*!*/   0x20,   /* Entry 9f */
/*!*/   0x20,   /* Entry a0 */
   0xad,   /* Entry a1 */
   0x9b,   /* Entry a2 */
   0x9c,   /* Entry a3 */
/*!*/   0x20,   /* Entry a4 */
   0x9d,   /* Entry a5 */
/*!*/   0x20,   /* Entry a6 */
/*!*/   0x20,   /* Entry a7 */
/*!*/   0x20,   /* Entry a8 */
/*!*/   0x20,   /* Entry a9 */
   0xa6,   /* Entry aa */
   0xae,   /* Entry ab */
   0xaa,   /* Entry ac */
/*!*/   0x20,   /* Entry ad */
/*!*/   0x20,   /* Entry ae */
   0xfe,   /* Entry af */
   0xf8,   /* Entry b0 */
   0xf1,   /* Entry b1 */
   0xfd,   /* Entry b2 */
/*!*/   0x20,   /* Entry b3 */
/*!*/   0x20,   /* Entry b4 */
   0xe6,   /* Entry b5 */
/*!*/   0x20,   /* Entry b6 */
   0xf9,   /* Entry b7 */
/*!*/   0x20,   /* Entry b8 */
/*!*/   0x20,   /* Entry b9 */
   0xa7,   /* Entry ba */
   0xaf,   /* Entry bb */
   0xac,   /* Entry bc */
   0xab,   /* Entry bd */
/*!*/   0x20,   /* Entry be */
   0xa8,   /* Entry bf */
/*!*/   0x20,   /* Entry c0 */
/*!*/   0x20,   /* Entry c1 */
/*!*/   0x20,   /* Entry c2 */
/*!*/   0x20,   /* Entry c3 */
   0x8e,   /* Entry c4 */
   0x8f,   /* Entry c5 */
   0x92,   /* Entry c6 */
   0x80,   /* Entry c7 */
/*!*/   0x20,   /* Entry c8 */
   0x90,   /* Entry c9 */
/*!*/   0x20,   /* Entry ca */
/*!*/   0x20,   /* Entry cb */
/*!*/   0x20,   /* Entry cc */
/*!*/   0x20,   /* Entry cd */
/*!*/   0x20,   /* Entry ce */
/*!*/   0x20,   /* Entry cf */
/*!*/   0x20,   /* Entry d0 */
   0xa5,   /* Entry d1 */
/*!*/   0x20,   /* Entry d2 */
/*!*/   0x20,   /* Entry d3 */
/*!*/   0x20,   /* Entry d4 */
/*!*/   0x20,   /* Entry d5 */
   0x99,   /* Entry d6 */
/*!*/   0x20,   /* Entry d7 */
/*!*/   0x20,   /* Entry d8 */
/*!*/   0x20,   /* Entry d9 */
/*!*/   0x20,   /* Entry da */
/*!*/   0x20,   /* Entry db */
   0x9a,   /* Entry dc */
/*!*/   0x20,   /* Entry dd */
/*!*/   0x20,   /* Entry de */
   0xe1,   /* Entry df */
   0x85,   /* Entry e0 */
   0xa0,   /* Entry e1 */
   0x83,   /* Entry e2 */
/*!*/   0x20,   /* Entry e3 */
   0x84,   /* Entry e4 */
   0x86,   /* Entry e5 */
   0x91,   /* Entry e6 */
   0x87,   /* Entry e7 */
   0x8a,   /* Entry e8 */
   0x82,   /* Entry e9 */
   0x88,   /* Entry ea */
   0x89,   /* Entry eb */
   0x8d,   /* Entry ec */
   0xa1,   /* Entry ed */
   0x8c,   /* Entry ee */
   0x8b,   /* Entry ef */
   0xeb,   /* Entry f0 */
   0xa4,   /* Entry f1 */
   0x95,   /* Entry f2 */
   0xa2,   /* Entry f3 */
   0x93,   /* Entry f4 */
/*!*/   0x20,   /* Entry f5 */
   0x94,   /* Entry f6 */
   0xf6,   /* Entry f7 */
   0xed,   /* Entry f8 */
   0x97,   /* Entry f9 */
   0xa3,   /* Entry fa */
   0x96,   /* Entry fb */
   0x81,   /* Entry fc */
/*!*/   0x20,   /* Entry fd */
/*!*/   0x20,   /* Entry fe */
   0x98,   /* Entry ff */
   };

BOOL JustOpened=FALSE;
struct Library *ConsoleDevice=NULL;
BOOL specialpal;

/**********************************************************/
/* FILEIO.C PCWindow save and restore settings variables. */
/**********************************************************/

struct Settings
   {
   SHORT   PresetMonoFlags;
   SHORT   PresetMonoWidth;
   SHORT   PresetMonoHeight;
   SHORT   PresetMonoTopEdge;
   SHORT   PresetMonoLeftEdge;
   SHORT   PresetMonoDepth;

   SHORT   PresetColorFlags;
   SHORT   PresetColorWidth;
   SHORT   PresetColorHeight;
   SHORT   PresetColorTopEdge;
   SHORT   PresetColorLeftEdge;
   SHORT   DefaultColorTextDepth;

   LONG    CursorSeconds;
   LONG    CursorMicros;

   LONG    DisplayPriority;

   SHORT   TextOnePlaneColors[2];
   SHORT   TextTwoPlaneColors[4];
   SHORT   TextThreePlaneColors[8];
   SHORT   TextFourPlaneColors[16];

   SHORT   HighGraphicsColors[2];
   SHORT   LowGraphicsColors[2][2][4];
   };

struct   Settings CurrentSettings;
struct   Settings LastSavedSettings;

struct   Settings DefaultSettings = {
      0L,   /* PresetMonoFlags */
	  640L, /* PresetMonoWidth */
	  200L, /* PresetMonoHeight */
      0L,   /* PresetMonoTopEdge */
      0L,   /* PresetMonoLeftEdge */
      1L,   /* PresetMonoDepth */

      0L,   /* PresetColorFlags */
	  640L, /* PresetColorWidth */
	  200L, /* PresetColorHeight */
      0L,   /* PresetColorTopEdge */
      0L,   /* PresetColorLeftEdge */
      4L,   /* PresetColorDepth */

	  DEFAULT_CURSOR_SECONDS, /* CursorSeconds */
	  DEFAULT_CURSOR_MICROS,  /* CursorMicros */

	  0L,   /* Display Priority */

      /*TextOnePlaneColors[2]*/
      {
         LOW_BLUE,
         HIGH_RED | HIGH_GREEN | HIGH_BLUE,
      },

      /*TextTwoPlaneColors[4]*/
      {
         LOW_BLUE,
         HIGH_RED | HIGH_GREEN,
         HIGH_RED | HIGH_BLUE,
         HIGH_RED | HIGH_GREEN | HIGH_BLUE,
      },

      /*TextThreePlaneColors[8]*/
      {
         0L,
         HIGH_BLUE,
         HIGH_GREEN,
         HIGH_GREEN | HIGH_BLUE,
         HIGH_RED,
         HIGH_RED | HIGH_BLUE,
         HIGH_RED | HIGH_GREEN          ,
         HIGH_RED | HIGH_GREEN | HIGH_BLUE,
      },
      /*TextFourPlaneColors[16]*/
      {
         0x000,
         0x00B,
         0x0B0,
         0x0BB,
         0xB00,
         0xB0B,
         0xB80,
         0xBBB,

         0x666,
         0x00F,
         0x0F0,
         0x0FF,
         0xF00,
         0xF0F,
         0xFF0,
         0xFFF,
      },

      /*HighGraphicsColors[2]*/
      {
         0L,
         LOW_RED | LOW_GREEN | LOW_BLUE,
      },

      /*LowGraphicsColors[2][2][4]*/
      {
         {
            /* Low Intensity Palettes */
            {
               /* Palette 0 */
               0L,
               LOW_GREEN,
               LOW_RED,
               LOW_RED | LOW_GREEN,
            },

            {
               /* Palette 1 */
               0L,
               LOW_GREEN | LOW_BLUE,
               LOW_RED            | LOW_BLUE,
               LOW_RED | LOW_GREEN | LOW_BLUE,
            },
         },

         {
            /* High Intensity Palettes */
            {
               /* Palette 0 */
               0L,
               HIGH_GREEN,
               HIGH_RED,
               HIGH_RED | HIGH_GREEN,
            },

            {
               /* Palette 1 */
               0L,
               HIGH_GREEN | HIGH_BLUE,
               HIGH_RED | HIGH_BLUE,
               HIGH_RED | HIGH_GREEN | HIGH_BLUE,
            },
         },
      }, 
   };
