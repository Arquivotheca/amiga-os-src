
/* ***************************************************************************
 * 
 * Menu Headers for the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY	Name		Description
 * -------	------		--------------------------------------------
 * 3 Apr 86	=RJ Mical=	Created this file
 *
 * **************************************************************************/


#define EGLOBAL_CANCEL TRUE /* this prevents eglobal.c from being included */
#include "zaphod.h"

extern struct TextAttr SafeFont;


#define MENU_HEADER_WIDTH	    86
#define MENU_HEADER_LEFT	    6

#define PROJECT_MENU_LEFT	    MENU_HEADER_LEFT
#define DISPLAY_MENU_LEFT	    PROJECT_MENU_LEFT + MENU_HEADER_WIDTH

#define PROJECT_ITEMS_WIDTH	    132
#define DISPLAY_ITEMS_WIDTH	    222
#define DISPLAY_ITEMS_LEFT	    -9

#define INFO_SUBITEMS_WIDTH	    (16 * 8) + 2
#define INFO_SUBITEMS_LEFT	    (6 * 8) + 2

#define BORDER_SUBITEMS_COUNT	    2	      
#define BORDER_SUBITEMS_WIDTH	    (6 * 8) + 2
#define BORDER_SUBITEMS_LEFT	    -(BORDER_SUBITEMS_WIDTH) + 12

#define SIZE_SUBITEMS_COUNT	    2
#define SIZE_SUBITEMS_WIDTH	   (10 * 8) + 2
#define SIZE_SUBITEMS_LEFT	   -(SIZE_SUBITEMS_WIDTH) + 12

#define CURSOR_SUBITEMS_COUNT	     4
#define CURSOR_SUBITEMS_WIDTH	     (5 * 8) + 2
#define CURSOR_SUBITEMS_LEFT	     -(CURSOR_SUBITEMS_WIDTH) + 12

#define DEPTH_SUBITEMS_COUNT	    4
#define DEPTH_SUBITEMS_WIDTH	    (9 * 8) + 2
#define DEPTH_SUBITEMS_LEFT	    -(DEPTH_SUBITEMS_WIDTH) + 12

#define PRIORITY_SUBITEMS_COUNT     5
#define PRIORITY_SUBITEMS_WIDTH     (6 * 8) + 2
#define PRIORITY_SUBITEMS_LEFT	    -(PRIORITY_SUBITEMS_WIDTH) + 12

#define INTERLACE_SUBITEMS_COUNT    2
#define INTERLACE_SUBITEMS_WIDTH    (5 * 8) + 2
#define INTERLACE_SUBITEMS_LEFT     -(INTERLACE_SUBITEMS_WIDTH) + 12



struct IntuiText InfoSubItemText[] =
    {
    /* "Display Priority" */	
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	"Display Priority",
	NULL,
	},

    /* "Multiple Windows" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	"Multiple Windows",
	NULL,
	},

    /* "Text Colors" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	"Text Colors",
	NULL,
	},

    /* "Window Borders" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	"Window Borders",
	NULL,
	},

    /* "Keyboard" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	"Keyboard",
	NULL,
	},

    /* "Credits" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	"Credits",
	NULL,
	},

    };


struct IntuiText SizeSubItemText[] =
    {
    /* "Full Size" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	" Full Size",
	NULL,
	},

    /* "Small Size" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	"Small Size",
	NULL,
	},

    };


struct IntuiText BorderSubItemText[] =
    {
    /* "Show" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	" Show",
	NULL,
	},

    /* "Hide" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	" Hide",
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
	" 1/2",
	NULL,
	},

    /* "2" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	"  1",
	NULL,
	},

    /* "3" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	"  2",
	NULL,
	},

    /* "4" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	"  4",
	NULL,
	},

    };


struct IntuiText DepthofTextDisplaySubItemText[] =
    {
    /* "1 (2 colors)" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	" 2 Colors",
	NULL,
	},

    /* "2 (4 colors)" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	" 4 Colors",
	NULL,
	},

    /* "3 (8 colors)" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	" 8 Colors",
	NULL,
	},

    /* "4 (16 colors)" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	"16 Colors",
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
	" + 10",
	NULL,
	},

    /* "+ 5" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	"  + 5",
	NULL,
	},

    /* "0" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	"    0",
	NULL,
	},

    /* "- 5" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	"  - 5",
	NULL,
	},

    /* "- 10" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	" - 10",
	NULL,
	},

    };


struct IntuiText InterlaceSubItemText[] =
    {
    /* "On" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	"  On",
	NULL,
	},

    /* "Off" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	" Off",
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
	"Close",
	NULL,
	},

    /* "Info" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	"Info",
	NULL,
	},

    /* "Restore Settings" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	"Restore Settings",
	NULL,
	},

    /* "Save Settings" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	"Save Settings",
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
	"  Interlace",
	NULL,
	},

    /* "Set Display Task Priority" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	"  Set Display Task Priority",
	NULL,
	},

    /* "Depth of Text Display" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	"  Depth of Text Display",
	NULL,
	},

    /* "Refresh Display" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	"  Refresh Display",
	NULL,
	},

    /* "Open Another Window" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	"  Open Another Window",
	NULL,
	},

    /* "Set Cursor Blink Rate" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	"  Set Cursor Blink Rate",
	NULL,
	},

    /* "Color" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	"  Color",
	NULL,
	},

    /* "Border" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	"  Border",
	NULL,
	},

    /* "Size" */
	{
	0, 1,
	0,
	1, 1,
	&SafeFont,
	"  Size",
	NULL,
	},

    };



struct MenuItem InfoSubItems[INFO_SUBITEMS_COUNT] =
    {
    /* "Display Priority" */
	{
	NULL,
	INFO_SUBITEMS_LEFT, SAFE_HEIGHT * 00,
	INFO_SUBITEMS_WIDTH, SAFE_HEIGHT,
	(ITEMTEXT | ITEMENABLED | HIGHCOMP),
	0,
	(APTR)&InfoSubItemText[00],
	NULL,
	0,
	NULL,
	0,
	},

    /* "Multiple Windows" */
	{
	&InfoSubItems[00],
	INFO_SUBITEMS_LEFT, SAFE_HEIGHT * 01,
	INFO_SUBITEMS_WIDTH, SAFE_HEIGHT,
	(ITEMTEXT | ITEMENABLED | HIGHCOMP),
	0,
	(APTR)&InfoSubItemText[01],
	NULL,
	0,
	NULL,
	0,
	},

    /* "Text Colors" */
	{
	&InfoSubItems[01],
	INFO_SUBITEMS_LEFT, SAFE_HEIGHT * 02,
	INFO_SUBITEMS_WIDTH, SAFE_HEIGHT,
	(ITEMTEXT | ITEMENABLED | HIGHCOMP),
	0,
	(APTR)&InfoSubItemText[02],
	NULL,
	0,
	NULL,
	0,
	},

    /* "Window Borders" */
	{
	&InfoSubItems[02],
	INFO_SUBITEMS_LEFT, SAFE_HEIGHT * 03,
	INFO_SUBITEMS_WIDTH, SAFE_HEIGHT,
	(ITEMTEXT | ITEMENABLED | HIGHCOMP),
	0,
	(APTR)&InfoSubItemText[03],
	NULL,
	0,
	NULL,
	0,
	},

    /* "Keyboard" */
	{
	&InfoSubItems[03],
	INFO_SUBITEMS_LEFT, SAFE_HEIGHT * 04,
	INFO_SUBITEMS_WIDTH, SAFE_HEIGHT,
	(ITEMTEXT | ITEMENABLED | HIGHCOMP),
	0,
	(APTR)&InfoSubItemText[04],
	NULL,
	0,
	NULL,
	0,
	},

    /* "Credits" */
	{
	&InfoSubItems[04],
	INFO_SUBITEMS_LEFT, SAFE_HEIGHT * 05,
	INFO_SUBITEMS_WIDTH, SAFE_HEIGHT,
	(ITEMTEXT | ITEMENABLED | HIGHCOMP),
	0,
	(APTR)&InfoSubItemText[05],
	NULL,
	0,
	NULL,
	0,
	},

    };



struct MenuItem SizeSubItems[SIZE_SUBITEMS_COUNT] =
    {
    /* "Full Size" */
	{
	NULL,
	SIZE_SUBITEMS_LEFT, SAFE_HEIGHT * 00,
	SIZE_SUBITEMS_WIDTH, SAFE_HEIGHT,
	(ITEMTEXT | ITEMENABLED | HIGHCOMP),
	0,
	(APTR)&SizeSubItemText[00],
	NULL,
	0,
	NULL,
	0,
	},

    /* "Small Size" */
	{
	&SizeSubItems[00],
	SIZE_SUBITEMS_LEFT, SAFE_HEIGHT * 01,
	SIZE_SUBITEMS_WIDTH, SAFE_HEIGHT,
	(ITEMTEXT | ITEMENABLED | HIGHCOMP),
	0,
	(APTR)&SizeSubItemText[01],
	NULL,
	0,
	NULL,
	0,
	},

    };


struct MenuItem BorderSubItems[BORDER_SUBITEMS_COUNT] =
    {
    /* "Show" */
	{
	NULL,
	BORDER_SUBITEMS_LEFT, SAFE_HEIGHT * 00,
	BORDER_SUBITEMS_WIDTH, SAFE_HEIGHT,
	(ITEMTEXT | ITEMENABLED | HIGHCOMP),
	0,
	(APTR)&BorderSubItemText[00],
	NULL,
	0,
	NULL,
	0,
	},

    /* "Hide" */
	{
	&BorderSubItems[00],
	BORDER_SUBITEMS_LEFT, SAFE_HEIGHT * 01,
	BORDER_SUBITEMS_WIDTH, SAFE_HEIGHT,
	(ITEMTEXT | ITEMENABLED | HIGHCOMP),
	0,
	(APTR)&BorderSubItemText[01],
	NULL,
	0,
	NULL,
	0,
	},

    };


struct MenuItem SetCursorBlinkRateSubItems[CURSOR_SUBITEMS_COUNT] =
    {
    /* "1" */
	{
	NULL,
	CURSOR_SUBITEMS_LEFT, SAFE_HEIGHT * 00,
	CURSOR_SUBITEMS_WIDTH, SAFE_HEIGHT,
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
	CURSOR_SUBITEMS_LEFT, SAFE_HEIGHT * 01,
	CURSOR_SUBITEMS_WIDTH, SAFE_HEIGHT,
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
	CURSOR_SUBITEMS_LEFT, SAFE_HEIGHT * 02,
	CURSOR_SUBITEMS_WIDTH, SAFE_HEIGHT,
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
	CURSOR_SUBITEMS_LEFT, SAFE_HEIGHT * 03,
	CURSOR_SUBITEMS_WIDTH, SAFE_HEIGHT,
	(ITEMTEXT | ITEMENABLED | HIGHCOMP),
	0,
	(APTR)&SetCursorBlinkRateSubItemText[03],
	NULL,
	0,
	NULL,
	0,
	},

    };



struct MenuItem DepthofTextDisplaySubItems[DEPTH_SUBITEMS_COUNT] =
    {
    /* "1 (2 colors)" */
	{
	NULL,
	DEPTH_SUBITEMS_LEFT, SAFE_HEIGHT * 00,
	DEPTH_SUBITEMS_WIDTH, SAFE_HEIGHT,
	(ITEMTEXT | ITEMENABLED | HIGHCOMP),
	0,
	(APTR)&DepthofTextDisplaySubItemText[00],
	NULL,
	0,
	NULL,
	0,
	},

    /* "2 (4 colors)" */
	{
	&DepthofTextDisplaySubItems[00],
	DEPTH_SUBITEMS_LEFT, SAFE_HEIGHT * 01,
	DEPTH_SUBITEMS_WIDTH, SAFE_HEIGHT,
	(ITEMTEXT | ITEMENABLED | HIGHCOMP),
	0,
	(APTR)&DepthofTextDisplaySubItemText[01],
	NULL,
	0,
	NULL,
	0,
	},

    /* "3 (8 colors)" */
	{
	&DepthofTextDisplaySubItems[01],
	DEPTH_SUBITEMS_LEFT, SAFE_HEIGHT * 02,
	DEPTH_SUBITEMS_WIDTH, SAFE_HEIGHT,
	(ITEMTEXT | ITEMENABLED | HIGHCOMP),
	0,
	(APTR)&DepthofTextDisplaySubItemText[02],
	NULL,
	0,
	NULL,
	0,
	},

    /* "4 (16 colors)" */
	{
	&DepthofTextDisplaySubItems[02],
	DEPTH_SUBITEMS_LEFT, SAFE_HEIGHT * 03,
	DEPTH_SUBITEMS_WIDTH, SAFE_HEIGHT,
	(ITEMTEXT | ITEMENABLED | HIGHCOMP),
	0,
	(APTR)&DepthofTextDisplaySubItemText[03],
	NULL,
	0,
	NULL,
	0,
	},

    };


struct MenuItem DisplayPrioritySubItems[PRIORITY_SUBITEMS_COUNT] =
    {
    /* "+ 10" */
	{
	NULL,
	PRIORITY_SUBITEMS_LEFT, SAFE_HEIGHT * 00,
	PRIORITY_SUBITEMS_WIDTH, SAFE_HEIGHT,
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
	PRIORITY_SUBITEMS_LEFT, SAFE_HEIGHT * 01,
	PRIORITY_SUBITEMS_WIDTH, SAFE_HEIGHT,
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
	PRIORITY_SUBITEMS_LEFT, SAFE_HEIGHT * 02,
	PRIORITY_SUBITEMS_WIDTH, SAFE_HEIGHT,
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
	PRIORITY_SUBITEMS_LEFT, SAFE_HEIGHT * 03,
	PRIORITY_SUBITEMS_WIDTH, SAFE_HEIGHT,
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
	PRIORITY_SUBITEMS_LEFT, SAFE_HEIGHT * 04,
	PRIORITY_SUBITEMS_WIDTH, SAFE_HEIGHT,
	(ITEMTEXT | ITEMENABLED | HIGHCOMP),
	0,
	(APTR)&DisplayPrioritySubItemText[04],
	NULL,
	0,
	NULL,
	0,
	},

    };


struct MenuItem InterlaceSubItems[INTERLACE_SUBITEMS_COUNT] =
    {
    /* "On" */
	{
	NULL,
	INTERLACE_SUBITEMS_LEFT, SAFE_HEIGHT * 00,
	INTERLACE_SUBITEMS_WIDTH, SAFE_HEIGHT,
	(ITEMTEXT | ITEMENABLED | HIGHCOMP),
	0,
	(APTR)&InterlaceSubItemText[00],
	NULL,
	0,
	NULL,
	0,
	},

    /* "Off" */
	{
	&InterlaceSubItems[00],
	INTERLACE_SUBITEMS_LEFT, SAFE_HEIGHT * 01,
	INTERLACE_SUBITEMS_WIDTH, SAFE_HEIGHT,
	(ITEMTEXT | ITEMENABLED | HIGHCOMP),
	0,
	(APTR)&InterlaceSubItemText[01],
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
	0, SAFE_HEIGHT * 03,
	PROJECT_ITEMS_WIDTH, SAFE_HEIGHT,
	(ITEMTEXT | ITEMENABLED | HIGHCOMP),
	0,
	(APTR)&ProjectItemText[00],
	NULL,
	0,
	NULL,
	0,
	},

    /* "Info" */
	{
	&ProjectItems[00],
	0, SAFE_HEIGHT * 02,
	PROJECT_ITEMS_WIDTH, SAFE_HEIGHT,
	(ITEMTEXT | ITEMENABLED | HIGHCOMP),
	0,
	(APTR)&ProjectItemText[01],
	NULL,
	0,
	&InfoSubItems[INFO_SUBITEMS_COUNT - 1],
	0,
	},

    /* "Restore Settings" */
	{
	&ProjectItems[01],
	0, SAFE_HEIGHT * 01,
	PROJECT_ITEMS_WIDTH, SAFE_HEIGHT,
	(ITEMTEXT | ITEMENABLED | HIGHCOMP),
	0,
	(APTR)&ProjectItemText[02],
	NULL,
	0,
	NULL,
	0,
	},

    /* "Save Settings" */
	{
	&ProjectItems[02],
	0, SAFE_HEIGHT * 00,
	PROJECT_ITEMS_WIDTH, SAFE_HEIGHT,
	(ITEMTEXT | ITEMENABLED | HIGHCOMP),
	0,
	(APTR)&ProjectItemText[03],
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
	DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 08,
	DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
	(ITEMTEXT | ITEMENABLED | HIGHCOMP),
	0,
	(APTR)&DisplayItemText[00],
	NULL,
	0,
	&InterlaceSubItems[INTERLACE_SUBITEMS_COUNT - 1],
	0,
	},

    /* "Set Display Task Priority" */
	{
	&DisplayItems[00],
	DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 07,
	DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
	(ITEMTEXT | ITEMENABLED | HIGHCOMP),
	0,
	(APTR)&DisplayItemText[01],
	NULL,
	0,
	&DisplayPrioritySubItems[PRIORITY_SUBITEMS_COUNT - 1],
	0,
	},

    /* "Depth of Text Display" */
	{
	&DisplayItems[01],
	DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 06,
	DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
	(ITEMTEXT | ITEMENABLED | HIGHCOMP),
	0,
	(APTR)&DisplayItemText[02],
	NULL,
	0,
	&DepthofTextDisplaySubItems[DEPTH_SUBITEMS_COUNT - 1],
	0,
	},

    /* "Refresh Display" */
	{
	&DisplayItems[02],
	DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 05,
	DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
	(ITEMTEXT | ITEMENABLED | HIGHCOMP),
	0,
	(APTR)&DisplayItemText[03],
	NULL,
	0,
	NULL,
	0,
	},

    /* "Open Another Window" */
	{
	&DisplayItems[03],
	DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 04,
	DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
	(ITEMTEXT | ITEMENABLED | HIGHCOMP),
	0,
	(APTR)&DisplayItemText[04],
	NULL,
	0,
	NULL,
	0,
	},

    /* "Set Cursor Blink Rate" */
	{
	&DisplayItems[04],
	DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 03,
	DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
	(ITEMTEXT | ITEMENABLED | HIGHCOMP),
	0,
	(APTR)&DisplayItemText[05],
	NULL,
	0,
	&SetCursorBlinkRateSubItems[CURSOR_SUBITEMS_COUNT - 1],
	0,
	},

    /* "Color" */
	{
	&DisplayItems[05],
	DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 02,
	DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
	(ITEMTEXT | ITEMENABLED | HIGHCOMP),
	0,
	(APTR)&DisplayItemText[06],
	NULL,
	0,
	NULL,
	0,
	},

    /* "Border" */
	{
	&DisplayItems[06],
	DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 01,
	DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
	(ITEMTEXT | ITEMENABLED | HIGHCOMP),
	0,
	(APTR)&DisplayItemText[07],
	NULL,
	0,
	&BorderSubItems[BORDER_SUBITEMS_COUNT - 1],
	0,
	},

    /* "Size" */
	{
	&DisplayItems[07],
	DISPLAY_ITEMS_LEFT, SAFE_HEIGHT * 00,
	DISPLAY_ITEMS_WIDTH, SAFE_HEIGHT,
	(ITEMTEXT | ITEMENABLED | HIGHCOMP),
	0,
	(APTR)&DisplayItemText[08],
	NULL,
	0,
	&SizeSubItems[SIZE_SUBITEMS_COUNT - 1],
	0,
	},

   };



struct Menu MenuHeaders[MENU_HEADERS_COUNT] =
    {
    /* "Display" Menu Header */
	{
	NULL,			/* NextMenu */
	DISPLAY_MENU_LEFT, 0,	/* Left, dummy Top */
	MENU_HEADER_WIDTH, 0,	/* Width, dummy Height */
	MENUENABLED,		/* Flags */
	"Display",	       /* MenuName */
	&DisplayItems[DISPLAY_ITEMS_COUNT - 1], 	      /* FirstItem */
	0, 0, 0, 0,		/* mysterious variables */
	},
    /* "Project" Menu Header */
	{
	&MenuHeaders[0],	/* NextMenu */
	PROJECT_MENU_LEFT, 0,	/* Left, dummy Top */
	MENU_HEADER_WIDTH, 0,	/* Width, dummy Height */
	MENUENABLED,		/* Flags */
	"Project",	       /* MenuName */
	&ProjectItems[PROJECT_ITEMS_COUNT - 1], /* FirstItem */
	0, 0, 0, 0,		/* mysterious variables */
	},
    };



