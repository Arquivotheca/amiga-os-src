
/* ***************************************************************************
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
 * HISTORY	Name		Description
 * -------	------		--------------------------------------------
 * 22 Feb 86	=RJ Mical=	Created this file
 *
 * **************************************************************************/


#define EGLOBAL_CANCEL TRUE /* this prevents eglobal.c from being included */
#include "zaphod.h"



						       
/* === System Variables ==================================================== */
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase = NULL;
struct LayersBase *LayersBase = NULL;
struct JanusBase *JanusBase = NULL;
struct DiskfontBase *DiskfontBase = NULL;
UBYTE *NormalFont = NULL;
UBYTE *UnderlineFont = NULL;
UBYTE *FontData = NULL;
struct Remember *GlobalKey = NULL;
UBYTE SubsysName[] = "Zaphod";
UBYTE MonoActiveTitle[] = " PC Monochrome Display ";
UBYTE MonoInactiveTitle[] = " PC Monochrome Display (Inactive) ";
UBYTE ColorActiveTitle[] = " PC Color Display  ";
UBYTE ColorInactiveTitle[] = " PC Color Display (Inactive) ";
UBYTE SidecarSettingsFile[] = ":Sidecar/SidecarSettings.Table";
UBYTE SidecarKeysFile[] = ":Sidecar/SidecarKeys.Table";
UBYTE AllSetPlane[80 * CHAR_HEIGHT];
UBYTE AllClearPlane[80 * CHAR_HEIGHT];
struct TextAttr PCFont =
    {
    "pcfont.font",
    8,
    FS_NORMAL,
    FPB_DISKFONT,
    };
SHORT AuxToolUsers = 0;
BOOL AbandonBlink = FALSE;
struct DisplayList ZaphodDisplay =
    {
    NULL,
	{
	NULL,
	0,
	0,
	},
    };
struct SetupSig *PCKeySig = NULL;
SHORT CursorPriority = -1;
SHORT DisplayPriority = 0;
SHORT ColorIntenseIndex;
struct TextAttr SafeFont =
    {
    "topaz.font",
    TOPAZ_EIGHTY,
    0,
    0,
    };
struct MsgPort *AuxToolsFinalPort = NULL;
struct IntuiText OKText =
    {
    AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
    AUTOLEFTEDGE, AUTOTOPEDGE,
    &SafeFont,
    "OK", 
    NULL,
    };
SHORT DefaultColorTextDepth = 4;



/* === Keycode Translation Stuff ========================================== */
BYTE PCRawKeyTable[128];
BYTE PCAltCode = 0;
BYTE PCCapsLockCode = 0;
BYTE PCCtrlCode = 0;
BYTE PCLeftShiftCode = 0;
BYTE PCNumLockCode = 0;
BYTE PCPlusCode = 0;
BYTE PCPtrScrCode = 0;
BYTE PCRightShiftCode = 0;
BYTE PCScrollLockCode = 0;

BYTE AmigaCapsLockCode = 0;
BYTE AmigaLeftAltCode = 0;
BYTE AmigaNCode = 0;
BYTE AmigaPCode = 0;
BYTE AmigaPlusCode = 0;
BYTE AmigaRightAltCode = 0;
BYTE AmigaSCode = 0;
	    
 



/* === Display Variables =================================================== */
struct Image HorizImage, VertImage;
struct PropInfo HorizInfo =
    {
    AUTOKNOB | FREEHORIZ, /* Flags */
    0, 0,	    /* Pots */
    0xFFFF, 0xFFFF, /* Dummy body initializations */
    0,0,0,0,0,0,    /* Variables maintained by Intuition */
    };
struct PropInfo VertInfo =
    {
    AUTOKNOB | FREEVERT, /* Flags */
    0, 0,	    /* Pots */
    0xFFFF, 0xFFFF, /* Dummy body initializations */
    0,0,0,0,0,0,    /* Variables maintained by Intuition */
    };
struct Gadget HorizGadget =
    {
    NULL,	    /* NextGadget */
    0, -8,	    /* LeftEdge, TopEdge (relative) */
    -16, 9,	    /* Width (relative), Height */
    GADGHNONE | GADGIMAGE | GRELBOTTOM | GRELWIDTH, /* Flags */
    BOTTOMBORDER | RELVERIFY, /* Activation */
    PROPGADGET,     /* Gadget Type */
    (APTR)&HorizImage, /* Dummy image for proportional gadget */
    NULL,	    /* SelectRender */
    NULL,	    /* Text */
    NULL,	    /* MutualExclude */
    (APTR)&HorizInfo,  /* Special info for the proportional gadget */
    HORIZ_GADGET,   /* ID */
    NULL,	    /* UserData */
    };
struct Gadget VertGadget =
    {
    &HorizGadget,   /* NextGadget */
    -18, 10,	    /* LeftEdge (relative), TopEdge */
    19, -9 -9,	    /* Width, Height (relative) */
    GADGHNONE | GADGIMAGE | GRELRIGHT | GRELHEIGHT, /* Flags */
    RIGHTBORDER | RELVERIFY, /* Activation */
    PROPGADGET,     /* Gadget Type */
    (APTR)&VertImage,  /* Dummy image for proportional gadget */
    NULL,	    /* SelectRender */
    NULL,	    /* Text */
    NULL,	    /* MutualExclude */
    (APTR)&VertInfo,   /* Special info for the proportional gadget */
    VERT_GADGET,    /* ID */
    NULL,	    /* UserData */
    };

USHORT TextOnePlaneColors[2] =
    {
			     LOW_BLUE,
    HIGH_RED | HIGH_GREEN | HIGH_BLUE,
    };

USHORT TextTwoPlaneColors[4] =
    {
			     LOW_BLUE,
     LOW_RED |	LOW_GREEN	     ,
     LOW_RED		  |  LOW_BLUE,
    HIGH_RED | HIGH_GREEN | HIGH_BLUE,
    };

USHORT TextThreePlaneColors[8] =
    {
    0,
			    HIGH_BLUE,
	       HIGH_GREEN	     ,
	       HIGH_GREEN | HIGH_BLUE,
    HIGH_RED			     ,
    HIGH_RED |		    HIGH_BLUE,
    HIGH_RED | HIGH_GREEN	     ,
    HIGH_RED | HIGH_GREEN | HIGH_BLUE,
    };

USHORT TextFourPlaneColors[16] =
    {
/* Used to be this simple, but it looks awful, so I'm taking over, boys ... */
/*
 *  0,
 *			     LOW_BLUE,
 *		LOW_GREEN	     ,
 *		LOW_GREEN |  LOW_BLUE,
 *   LOW_RED			     ,
 *   LOW_RED |		     LOW_BLUE,
 *   LOW_RED |	LOW_GREEN	     ,
 *   LOW_RED |	LOW_GREEN |  LOW_BLUE,
 *
 *   MIN_RED |	MIN_GREEN |  MIN_BLUE,
 *
 *			    HIGH_BLUE,
 *	       HIGH_GREEN	     ,
 *	       HIGH_GREEN | HIGH_BLUE,
 *  HIGH_RED			     ,
 *  HIGH_RED |		    HIGH_BLUE,
 *  HIGH_RED | HIGH_GREEN	     ,
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


USHORT HighGraphicsColors[2] =
    {
    0,
    LOW_RED | LOW_GREEN | LOW_BLUE,
    };

USHORT LowGraphicsColors[2][2][4] =
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
	    LOW_RED		| LOW_BLUE,
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

struct NewScreen NewScreen =
    {
    0, 0,	/* LeftEdge, TopEdge */
    0, 200, 0,	/* Width, Height, Depth */
    0, 1,	/* Detail/BlockPens */
    NULL,	/* ViewPort Modes (must set/clear HIRES as needed) */
    CUSTOMSCREEN,
    &SafeFont,	/* Font */
    "PC Color Display",
    NULL,	/* Gadgets */
    NULL,	/* CustomBitMap */
    };

struct NewWindow NewWindow =
    {
    0, 0,	/* LeftEdge, TopEdge */
    320, 200,	/* Width, Height */
    -1, -1,	/* Detail/BlockPens */
    NULL,	/* IDCMP Flags filled in later */
    WINDOWSIZING | WINDOWDRAG | WINDOWDEPTH | WINDOWCLOSE
	    | SIZEBRIGHT | SIZEBBOTTOM
	    | SMART_REFRESH | ACTIVATE,
    &VertGadget, /* FirstGadget */
    NULL,	/* Checkmark */
    NULL,	/* WindowTitle */
    NULL,	/* Screen */
    NULL,	/* SuperBitMap */
    96, 30,	/* MinWidth, MinHeight */
    640, 200,	/* MaxWidth, MaxHeight */
    CUSTOMSCREEN,
    };
       

/* === Assorted Task Variables ============================================= */
struct Task *CursorTask = NULL;
struct Task *InputTask = NULL;
struct Task *SL_Task = NULL;

ULONG CursorSuicideSignal = NULL;
ULONG SL_SuicideSignal = NULL;
ULONG InputSuicideSignal = NULL;



