
/* ***************************************************************************
 * 
 * Info Routines --  Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY	Name		Description
 * -------	------		--------------------------------------------
 * 23 Apr 86	=RJ Mical=	Created this file (right at the end!)
 *
 * **************************************************************************/

#include "zaphod.h"


#define EXTRA_TOP   6
#define EXTRA_LEFT  6

#define INFO_TEXT_COUNT 16

#define HEADER_START	(AUTOTOPEDGE + 4)
#define TEXT_START	(HEADER_START + SAFE_HEIGHT + 4)
#define TEXT_LEFT	(AUTOLEFTEDGE + EXTRA_LEFT)
#define INFO_HEIGHT	(TEXT_START + (INFO_TEXT_COUNT * SAFE_HEIGHT) + 27)


struct IntuiText InfoText[INFO_TEXT_COUNT] =
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
	&InfoText[00],
	},

	{
	AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
	TEXT_LEFT, TEXT_START + SAFE_HEIGHT * 01,
	&SafeFont,
	NULL,
	&InfoText[01],
	},

	{
	AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
	TEXT_LEFT, TEXT_START + SAFE_HEIGHT * 02,
	&SafeFont,
	NULL,
	&InfoText[02],
	},

	{
	AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
	TEXT_LEFT, TEXT_START + SAFE_HEIGHT * 03,
	&SafeFont,
	NULL,
	&InfoText[03],
	},

	{
	AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
	TEXT_LEFT, TEXT_START + SAFE_HEIGHT * 04,
	&SafeFont,
	NULL,
	&InfoText[04],
	},

	{
	AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
	TEXT_LEFT, TEXT_START + SAFE_HEIGHT * 05,
	&SafeFont,
	NULL,
	&InfoText[05],
	},

	{
	AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
	TEXT_LEFT, TEXT_START + SAFE_HEIGHT * 06,
	&SafeFont,
	NULL,
	&InfoText[06],
	},

	{
	AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
	TEXT_LEFT, TEXT_START + SAFE_HEIGHT * 07,
	&SafeFont,
	NULL,
	&InfoText[07],
	},

	{
	AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
	TEXT_LEFT, TEXT_START + SAFE_HEIGHT * 08,
	&SafeFont,
	NULL,
	&InfoText[08],
	},

	{
	AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
	TEXT_LEFT, TEXT_START + SAFE_HEIGHT * 09,
	&SafeFont,
	NULL,
	&InfoText[09],
	},

	{
	AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
	TEXT_LEFT, TEXT_START + SAFE_HEIGHT * 10,
	&SafeFont,
	NULL,
	&InfoText[10],
	},

	{
	AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
	TEXT_LEFT, TEXT_START + SAFE_HEIGHT * 11,
	&SafeFont,
	NULL,
	&InfoText[11],
	},

	{
	AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
	TEXT_LEFT, TEXT_START + SAFE_HEIGHT * 12,
	&SafeFont,
	NULL,
	&InfoText[12],
	},

	{
	AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
	TEXT_LEFT, TEXT_START + SAFE_HEIGHT * 13,
	&SafeFont,
	NULL,
	&InfoText[13],
	},

	{
	AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
	TEXT_LEFT, TEXT_START + SAFE_HEIGHT * 14,
	&SafeFont,
	NULL,
	&InfoText[14],
	},

    };



UBYTE *InfoStrings[INFO_SUBITEMS_COUNT][INFO_TEXT_COUNT] =
    {
	{
	/* Creators */
	"The Sidecar was created by:",
	"",
	"     Torsten Burgdorff",
	"     Bob \"Kodiak\" Burns",
	"     Phil Geyer",
	"     Neil Katin",
	"     Bill Kolb",
	"     =Robert J. Mical=",
	"     Dieter Prei\337",
	"     Burkhard Rosin",
	"     Wilfried Rusniok",
	"     Joachim Siedentopf",
	"     Frank-Thomas Ullmann",
	"     Heinz Ullrich",
	"",
	"",
	},

	{
	/* "Keyboard" */
	"Using the Keyboard:",
	"Send input to your IBM-PC programs",
	"through the Amiga keyboard.",
	"",
	"There are four PC keys that aren't",
	"on the Amiga keyboard.  But by",
	"typing control sequences you are",
	"able to transmit these keys:",
	"",
	"      PC KEY  SIDECAR EQUIVALENT",
	" -----------  ------------------",
	"    Num lock  Right AMIGA and N",
	" Scroll Lock  Right AMIGA and S",
	"                or the HELP Key",
	"    Ptr Sc *  Right AMIGA and P",
	" + on Keypad  Right AMIGA and +",
	},

	{
	/* "Borders" */
	"About Window Borders:",
	"The window border has gadgets that",
	"you use to size and position your",
	"window.  You can choose to have the",
	"window borders be visible or hidden.",
	"When the borders are hidden, all 80",
	"columns and 25 lines are visible.",
	"",
	"You can make the borders appear and",
	"disappear by double-clicking in the",
	"window, or by using the BORDER menu",
	"option.  Also, when you size the",
	"window to the full size of the",
	"screen, the window borders vanish",
	"automatically.",
	"",
	},

	{
	/* "Number of Text Colors" */
	"Number of Text Colors:",
	"",
	"PC Mono text can appear in up to 4",
	"colors,  PC Color text in up to 16",
	"colors.  You can elect to have text",
	"drawn in fewer colors if you want",
	"better performance and are willing",
	"to sacrifice some of the colors.",
	"",
	"",
	"The speed improvement from 16-color",
	"to 8-color text is significant.",
	"",
	"",
	"",
	"",
	},

	{
	/* "Multiple Windows" */
	"About Multiple Windows:",
	"",
	"",
	"You can open multiple windows in",
	"any display mode.  When you have",
	"multiple windows of a display, you",
	"can freeze the contents of a window",
	"by selecting another window of that",
	"display type.  You open another",
	"window by selecting the menu option",
	"\"OPEN ANOTHER WINDOW\".",
	"",
	"",
	"",
	"",
	"",
	},

	{
	/* "Display Priority" */
	"About Display Priority:",
	"All of the Amiga programs run at a",
	"certain \"priority\".  The Sidecar",
	"display task runs at the normal",
	"priority.",
	"",
	"When the PC display is changing a",
	"lot, the Sidecar display emulator",
	"will try to consume a lot of the",
	"system time and resources.",
	"",
	"To make the Sidecar performance",
	"improve, select a higher priority.",
	"To slow the Sidecar down and let",
	"other Amiga software run better,",
	"select a lower priority.",
	},

    };



struct IntuiText NextText =
    {
    AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
    AUTOLEFTEDGE, AUTOTOPEDGE,
    &SafeFont,
    "Next Page",
    NULL,
    };



Info(infoNumber, display)
SHORT infoNumber;
struct Display *display;
{
    struct Window *window;
    SHORT i;
    BOOL hitMeAgain;

    if (display)
	{
	if (display->FirstWindow)
	    window = display->FirstWindow->DisplayWindow;
	}
    else window = NULL;

    do
	{
	for (i = 0; i < INFO_TEXT_COUNT; i++)
	    InfoText[i].IText = InfoStrings[infoNumber][i];

	hitMeAgain = AutoRequest(window, &InfoText[INFO_TEXT_COUNT - 1], 
		&NextText, &OKText, 0, 0, 320, INFO_HEIGHT);

	infoNumber--;
	if (infoNumber < 0) infoNumber = INFO_SUBITEMS_COUNT - 1;
	}
    while (hitMeAgain);

}




