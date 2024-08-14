
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
 * 22 Feb 86   =RJ Mical=  Created this file
 *
 * *************************************************************************/


#define EGLOBAL_CANCEL TRUE /* this prevents eglobal.c from being included */
#include "zaphod.h"



													   
/* === System Variables ================================================== */

#ifndef RELEASE
UBYTE Version[] = "Zaphod Version 2.0 Gamma 1, 20 September 1988";
#endif
#ifdef RELEASE
UBYTE Version[] = "Zaphod Version 2.0, 20 September 1988";
#endif

struct	Preferences *SavePrefs = NULL;

struct	IntuitionBase *IntuitionBase = NULL;
struct	GfxBase *GfxBase = NULL;
struct	LayersBase *LayersBase = NULL;
struct	JanusBase *JanusBase = NULL;
struct	JanusAmiga *JanusAmigaBA = NULL;
struct	JanusAmiga *JanusAmigaWA = NULL;
struct	DiskfontBase *DiskfontBase = NULL;
UBYTE	*NormalFont = NULL;
UBYTE	*UnderlineFont = NULL;
UBYTE	*FontData = NULL;
struct	Remember *GlobalKey = NULL;
SHORT	GlobalScreenHeight = 200;
UBYTE	SubsysName[] = "Zaphod";
UBYTE	MonoActiveTitle[] = " PC Monochrome Display ";
UBYTE	MonoInactiveTitle[] = " PC Monochrome Display (Inactive) ";
UBYTE	MonoFreezeTitle[] = " PC Monochrome Display (=Frozen=) ";
UBYTE	ColorActiveTitle[] = " PC Color Display  ";
UBYTE	ColorInactiveTitle[] = " PC Color Display (Inactive) ";
UBYTE	ColorFreezeTitle[] = " PC Color Display (=Frozen=) ";
UBYTE	ScreenTitle[] = "PC Display Screen";
UBYTE	ScreenUnsharedTitle[] = "PC Display Screen (=Not Shared=)";
UBYTE	OldSidecarSettingsFile[] = "SYS:Sidecar/SidecarSettings.Table";
UBYTE	SidecarSettingsFile[] = "SYS:PC/System/SidecarSettings.Table";
UBYTE	OldSidecarKeysFile[] = "SYS:Sidecar/SidecarKeys.Table";
UBYTE	SidecarKeysFile[] = "SYS:PC/System/SidecarKeys.Table";
UBYTE	OldScanCodeFile[] = "SYS:Sidecar/ScanCode.Table";
UBYTE	ScanCodeFile[] = "SYS:PC/System/ScanCode.Table";
UBYTE	*AllSetPlane;
UBYTE	*AllClearPlane;
SHORT	AuxToolUsers = 0;
BOOL	AbandonBlink = FALSE;
BOOL	DeadenBlink = FALSE;
struct	DisplayList ZaphodDisplay =
	{
	NULL,
		{
		NULL,
		0,
		0,
		},
	};
struct	SetupSig *PCKeySig = NULL;
SHORT	DisplayPriority = 0;
SHORT	CursorPriority = CURSORPRIORITY_OFFSET;
SHORT	ColorIntenseIndex;
struct	TextAttr SafeFont =
	{
	(UBYTE *)"topaz.font",
	TOPAZ_EIGHTY,		/* equals 8 */
	0,
	FPB_ROMFONT,
	};
struct	TextAttr PCFont =
	{
	(UBYTE *)"pcfont.font",
	8,
	FS_NORMAL,				/* equal 0 */
	FPB_DISKFONT,
	};
struct	MsgPort *AuxToolsFinalPort = NULL;
struct	IntuiText OKText =
	{
	AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
	AUTOLEFTEDGE, AUTOTOPEDGE,
	&SafeFont,
	(UBYTE *)"OK", 
	NULL,
	};
SHORT	DefaultColorTextDepth = 4;
SHORT	NullPointer[] = 
	{
	0, 0, 0, 0, 0
	};
LONG	(*KeyQueuer)() = NULL;
/*???SHORT	*PCMouseFlags = NULL;*/
/*???SHORT	*PCMouseX = NULL;*/
/*???SHORT	*PCMouseY = NULL;*/



/* === Keycode Translation Stuff ========================================= */
UBYTE	PCRawKeyTable[128];

UBYTE	PCAltCode = 0;
UBYTE	PCCapsLockCode = 0;
UBYTE	PCCtrlCode = 0;
UBYTE	PCLeftShiftCode = 0;
UBYTE	PCNumLockCode = 0;
UBYTE	PCPlusCode = 0;
UBYTE	PCPtrScrCode = 0;
UBYTE	PCRightShiftCode = 0;
UBYTE	PCScrollLockCode = 0;
UBYTE	PCTildeCode = 0;
UBYTE	PCBarCode = 0;
UBYTE	PCBackDashCode = 0;
UBYTE	PCBackSlashCode = 0;
UBYTE	PCKeypad0Code = 0xFF;
UBYTE	PCKeypad1Code = 0xFF;
UBYTE	PCKeypad2Code = 0xFF;
UBYTE	PCKeypad3Code = 0xFF;
UBYTE	PCKeypad4Code = 0xFF;
UBYTE	PCKeypad5Code = 0xFF;
UBYTE	PCKeypad6Code = 0xFF;
UBYTE	PCKeypad7Code = 0xFF;
UBYTE	PCKeypad8Code = 0xFF;
UBYTE	PCKeypad9Code = 0xFF;
UBYTE	PCKeypadDotCode = 0xFF;

UBYTE	AmigaCapsLockCode = 0;
UBYTE	AmigaLeftAltCode = 0;
UBYTE	AmigaNCode = 0;
UBYTE	AmigaPCode = 0;
UBYTE	AmigaPlusCode = 0;
UBYTE	AmigaRightAltCode = 0;
UBYTE	AmigaSCode = 0;
UBYTE	AmigaPrtScrCode = 0;
UBYTE	AmigaTildeBackDash = 0;
UBYTE	AmigaBarBackSlash = 0;
UBYTE	AmigaCursorUp = 0xFF;
UBYTE	AmigaCursorLeft = 0xFF;
UBYTE	AmigaCursorRight = 0xFF;
UBYTE	AmigaCursorDown = 0xFF;
UBYTE	AmigaDelCode = 0xFF;



/* === Display Variables ================================================= */
struct	Image HorizImage, VertImage;
struct	PropInfo HorizInfo =
	{
	AUTOKNOB | FREEHORIZ, /* Flags */
	0, 0,			/* Pots */
	0xFFFF, 0xFFFF, /* Dummy body initializations */
	0,0,0,0,0,0,	/* Variables maintained by Intuition */
	};
struct	PropInfo VertInfo =
	{
	AUTOKNOB | FREEVERT, /* Flags */
	0, 0,			/* Pots */
	0xFFFF, 0xFFFF, /* Dummy body initializations */
	0,0,0,0,0,0,	/* Variables maintained by Intuition */
	};
struct	Gadget HorizGadget =
	{
	NULL,			/* NextGadget */
	0, -8,			/* LeftEdge, TopEdge (relative) */
	-16, 9,			/* Width (relative), Height */
	GADGHNONE | GADGIMAGE | GRELBOTTOM | GRELWIDTH, /* Flags */
	BOTTOMBORDER | GADGIMMEDIATE | RELVERIFY | FOLLOWMOUSE, /* Activation */
	PROPGADGET,	 /* Gadget Type */
	(APTR)&HorizImage, /* Dummy image for proportional gadget */
	NULL,			/* SelectRender */
	NULL,			/* Text */
	NULL,			/* MutualExclude */
	(APTR)&HorizInfo,  /* Special info for the proportional gadget */
	HORIZ_GADGET,   /* ID */
	NULL,			/* UserData */
	};
struct	Gadget VertGadget =
	{
	&HorizGadget,   /* NextGadget */
	-18, 10,			/* LeftEdge (relative), TopEdge */
	19, -9 -9,			/* Width, Height (relative) */
	GADGHNONE | GADGIMAGE | GRELRIGHT | GRELHEIGHT, /* Flags */
	RIGHTBORDER | GADGIMMEDIATE | RELVERIFY | FOLLOWMOUSE, /* Activation */
	PROPGADGET,	 /* Gadget Type */
	(APTR)&VertImage,  /* Dummy image for proportional gadget */
	NULL,			/* SelectRender */
	NULL,			/* Text */
	NULL,			/* MutualExclude */
	(APTR)&VertInfo,   /* Special info for the proportional gadget */
	VERT_GADGET,	/* ID */
	NULL,			/* UserData */
	};

USHORT	TextOnePlaneColors[2] =
	{
							 LOW_BLUE,
	HIGH_RED | HIGH_GREEN | HIGH_BLUE,
	};

USHORT	TextTwoPlaneColors[4] =
	{
							 LOW_BLUE,
	 LOW_RED |		LOW_GREEN			 ,
	 LOW_RED				  |  LOW_BLUE,
	HIGH_RED | HIGH_GREEN | HIGH_BLUE,
	};

USHORT	TextThreePlaneColors[8] =
	{
	0,
							HIGH_BLUE,
			   HIGH_GREEN			 ,
			   HIGH_GREEN | HIGH_BLUE,
	HIGH_RED							 ,
	HIGH_RED |					HIGH_BLUE,
	HIGH_RED | HIGH_GREEN			 ,
	HIGH_RED | HIGH_GREEN | HIGH_BLUE,
	};

USHORT	TextFourPlaneColors[16] =
	{
/* Used to be this simple, but it looks awful, so I'm taking over, boys .. */
/*
 *  0,
 *							 LOW_BLUE,
 *				LOW_GREEN			 ,
 *				LOW_GREEN |  LOW_BLUE,
 *   LOW_RED							 ,
 *   LOW_RED |					 LOW_BLUE,
 *   LOW_RED |		LOW_GREEN			 ,
 *   LOW_RED |		LOW_GREEN |  LOW_BLUE,
 *
 *   MIN_RED |		MIN_GREEN |  MIN_BLUE,
 *
 *							HIGH_BLUE,
 *			   HIGH_GREEN			 ,
 *			   HIGH_GREEN | HIGH_BLUE,
 *  HIGH_RED							 ,
 *  HIGH_RED |					HIGH_BLUE,
 *  HIGH_RED | HIGH_GREEN			 ,
 *  HIGH_RED | HIGH_GREEN | HIGH_BLUE,
 */
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
	};


USHORT	HighGraphicsColors[2] =
	{
	0,
	LOW_RED | LOW_GREEN | LOW_BLUE,
	};

USHORT	LowGraphicsColors[2][2][4] =
	{
		{
		/* Low Intensity Palettes */
			{
			/* Palette 0 */
			0,
					  LOW_GREEN,
			LOW_RED,
			LOW_RED | LOW_GREEN,
			},

			{
			/* Palette 1 */
			0,
					  LOW_GREEN | LOW_BLUE,
			LOW_RED				| LOW_BLUE,
			LOW_RED | LOW_GREEN | LOW_BLUE,
			},
		},

		{
		/* High Intensity Palettes */
			{
			/* Palette 0 */
			0,
			HIGH_GREEN,
			HIGH_RED,
			HIGH_RED | HIGH_GREEN,
			},

			{
			/* Palette 1 */
			0,
			HIGH_GREEN | HIGH_BLUE,
			HIGH_RED | HIGH_BLUE,
			HIGH_RED | HIGH_GREEN | HIGH_BLUE,
			},
		},
	};

struct	NewScreen NewScreen =
	{
	0, 0,		/* LeftEdge, TopEdge */
	0, STDSCREENHEIGHT, 0,		/* Width, Height, Depth */
	0, 1,		/* Detail/BlockPens */
	NULL,		/* ViewPort Modes (must set/clear HIRES as needed) */
	CUSTOMSCREEN,
	&SafeFont,		/* Font */
	(UBYTE *)"PC Display Screen",
	NULL,		/* Gadgets */
	NULL,		/* CustomBitMap */
	};

struct	NewWindow NewWindow =
	{
	0, 0,		/* LeftEdge, TopEdge */
	320, 200,		/* Width, Height */
	-1, -1,		/* Detail/BlockPens */
	NULL,		/* IDCMP Flags filled in later */
	WINDOWSIZING | WINDOWDRAG | WINDOWDEPTH | WINDOWCLOSE
			| SIZEBRIGHT | SIZEBBOTTOM
			| SMART_REFRESH | ACTIVATE | REPORTMOUSE,
	&VertGadget, /* FirstGadget */
	NULL,		/* Checkmark */
	NULL,		/* WindowTitle */
	NULL,		/* Screen */
	NULL,		/* SuperBitMap */
	96, 30,		/* MinWidth, MinHeight */
	640, ZAPHOD_HEIGHT,		/* MaxWidth, MaxHeight */
	CUSTOMSCREEN,
	};

SHORT	LeftOfCloseGadget = 0;


	   

/* === Clip Area Variables =============================================== */
SHORT	ClipStartX, ClipStartY;
SHORT	ClipCurrentX, ClipCurrentY;



/* === Assorted Task Variables =========================================== */
struct	Task *MainTask = NULL;
struct	Task *CursorTask = NULL;
struct	Task *InputTask = NULL;

ULONG	CursorSuicideSignal = NULL;
ULONG	SL_SuicideSignal = NULL;
ULONG	InputSuicideSignal = NULL;


SHORT	TopBorderOn = NTSC_TOP_ONHEIGHT;
SHORT	TopBorderOff = NTSC_TOP_OFFHEIGHT;
/*BBBSHORT	BottomBorderOn = NTSC_BOTTOM_ONHEIGHT;*/
/*BBBSHORT	BottomBorderOff = NTSC_BOTTOM_OFFHEIGHT;*/


