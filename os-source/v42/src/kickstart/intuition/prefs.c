
/*** prefs.c *****************************************************************
 *
 *  the intuition preferences routines
 *
 *  $Id: prefs.c,v 40.0 94/02/15 17:31:10 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1985, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#include "intuall.h"

#include "preferences.h"

#ifndef DEVICES_INPUT_H
#include <devices/input.h>
#endif

/*****************************************************************************/

#define D(x)		;
#define DCOLORS(x)	;

/*****************************************************************************/

#define VIEWINITYMINREQ		\
(((SHORT) &((struct Preferences *)0)->ViewInitY)+(sizeof (WORD)))

#define POINTERMATRIXMINREQ \
(((SHORT) ((struct Preferences *)0)->PointerMatrix)+POINTERSIZE*(sizeof (USHORT)))

/*****************************************************************************/

struct Preferences DefaultPreferences =
{
 /* the default font height */
    TOPAZ_SIXTY,		/* BYTE FontHeight; default is low-res */

 /* the default printer port usage */
    PARALLEL_PRINTER,		/* UBYTE PrinterPort; */

 /* default baud rate */
    BAUD_9600,			/* USHORT BaudRate */

 /* various timing rates */
    {KEYREPSEC, KEYREPMIC,},	/* struct timeval KeyRptSpeed; */
    {KEYDELSEC, KEYDELMIC,},	/* struct timeval KeyRptDelay; */
    {MOUSEDBLSEC, MOUSEDBLMIC,},/* struct timeval DoubleClick; */

 /* Intuition Pointer data */
    {
	0x0000, 0x0000,		/* USHORT PointerMatrix[POINTERSIZE];  */

	0xC000, 0x4000,
	0x7000, 0xB000,
	0x3C00, 0x4C00,
	0x3F00, 0x4300,

	0x1FC0, 0x20C0,
	0x1FC0, 0x2000,
	0x0F00, 0x1100,
	0x0D80, 0x1280,

	0x04C0, 0x0940,
	0x0460, 0x08A0,
	0x0020, 0x0040,
	0x0000, 0x0000,

	0x0000, 0x0000,
	0x0000, 0x0000,
	0x0000, 0x0000,
	0x0000, 0x0000,

	0x0000, 0x0000,
    },
    POINTERX,			/* BYTE XOffset; */
    POINTERY,			/* BYTE YOffset; */
    COLOR17,			/* USHORT color17; */
    COLOR18,			/* USHORT color18; */
    COLOR19,			/* USHORT color19; */
    POINTERTICKS,		/* USHORT PointerTicks; */

 /* Workbench Screen colors */
    COLOR0,			/* USHORT color0; */
    COLOR1,			/* USHORT color1; */
    COLOR2,			/* USHORT color2; */
    COLOR3,			/* USHORT color3; */

 /* positioning data for the Intuition View */
    VIEWX,			/* USHORT ViewXOffset; */
    VIEWY,			/* USHORT ViewYOffset; */
    0, 0,			/* WORD ViewInitX, ViewInitY */

    ENABLECLI,			/* BOOL EnableCLI; */

 /* printer configurations */
    PRINTERTYPE,		/* USHORT PrinterType; */

 /** jimm: 11/18/85: changed to non-null default name **/
    {"generic"},		/* UBYTE PrinterFilename[FILENAME_SIZE]; */

 /* print format and quality configurations */
    PRINTPITCH,			/* BYTE PrintPitch; */
    PRINTQUALITY,		/* BYTE PrintQuality; */
    PRINTSPACING,		/* BYTE PrintSpacing; */
    PRINTLEFTMARGIN,		/* UWORD PrintLeftMargin; */
    PRINTRIGHTMARGIN,		/* UWORD PrintRightMargin; */
    PRINTINVERSE,		/* BYTE PrintInverse; */
    PRINTASPECT,		/* BYTE PrintAspect; */
    PRINTSHADE,			/* BYTE PrintShade; */
    PRINTTHRESHOLD,		/* WORD PrintThreshold; */

 /* print paper descriptors */
    PAPERSIZE,			/* BYTE PaperSize; */
    PAPERLENGTH,		/* UWORD PaperLength; */
    PAPERTYPE,			/* BYTE PaperType; */

 /* Serial device settings: These are six nibble-fields in three bytes */
 /* (these look a little strange so the defaults will map out to zero) */

    0,				/* UBYTE   SerRWBits;  */
    0,				/* UBYTE   SerStopBuf; */
    0,				/* UBYTE   SerParShk;  */
    0,				/* UBYTE   LaceWB;	  */
    {0,},			/* UBYTE   Pad[ 12 ] */
    {0,},			/* UBYTE   PrtDevName[DEVNAME_SIZE]; */
    0,				/* BYTE    DefaultPrtUnit; */
    0,				/* BYTE    DefaultSerUnit; */
    0,				/* BYTE    RowSizeChange; */
    0,				/* BYTE    ColumnSizeChange; */

    0,				/* UWORD   PrintFlags */
    0,				/* UWORD   PrintMaxWidth */
    0,				/* UWORD   PrintMaxHeight */
    0,				/* UBYTE   PrintDensity */
    0,				/* UBYTE   PrintXOffset */

    0,				/* UWORD   wb_Width  */
    0,				/* UWORD   wb_Height */
    0,				/* UBYTE   wb_Depth  */

    0,				/* UBYTE ext_size;  */
};

/*****************************************************************************/

static void copyprefs (struct Preferences *from, LONG size, struct Preferences *to)
{
    struct IntuitionBase *IBase = fetchIBase ();

    if (size <= 0)
	return;

    /* bart - 08.07.87 support extended preferences structure */

    if (size > sizeof (struct Preferences))
	 size = sizeof (struct Preferences);

    CopyMem (from, to, size);

    if (size >= VIEWINITYMINREQ)
    {
	to->ViewInitX = IBase->ViewInitX;
	to->ViewInitY = IBase->ViewInitY;
    }
}

/*****************************************************************************/

static void setIBasePrefs (void)
/* sets up IBase based on the values currently in IBase->Preferences */
{
    struct Preferences *Pref;
    struct IntuitionBase *IBase = fetchIBase ();
    struct GfxBase *gb;

    /* jimm: */
    struct Screen *findWorkBench ();

    D (printf ("sIBP\n"));

    LOCKIBASE ();

    gb = IBase->GfxBase;
    Pref = IBase->Preferences;

    IBase->DoubleSeconds = Pref->DoubleClick.tv_secs;
    IBase->DoubleMicros = Pref->DoubleClick.tv_micro;

    /* cache copy of original gfx NormalDisplayRows/Cols,
     * so SizeChange can be applied every prefs time
     */
    if (!(IBase->Flags & SEENSETPREFS))
    {
	IBase->OrigNDRows = gb->NormalDisplayRows;
	IBase->OrigNDCols = gb->NormalDisplayColumns;
    }

    /*
     * wb_Width/Height will be used for workbench, but old
     * morerows stuff still specifies normal width/height
     * Peter 30-Jan-91:  Maintain V34 private IntuitionBase fields
     * MaxDisplayHeight and MaxDisplayWidth for TV*Text.
     */
    IBase->MaxDisplayHeight = (gb->NormalDisplayRows =
			       IBase->OrigNDRows + Pref->RowSizeChange) << 1;
    IBase->MaxDisplayWidth = gb->NormalDisplayColumns =
	IBase->OrigNDCols + Pref->ColumnSizeChange;

    /* Reset mouse limits for new view position	*/
    freeMouse ();

    /* determine effective mouse scale factor	*/
    IBase->EffectiveScaleFactor.X = IBase->MouseScaleX / Pref->PointerTicks;
    IBase->EffectiveScaleFactor.Y = IBase->MouseScaleY / Pref->PointerTicks;

    /* Pointer changes through old-SetPrefs now happen inside
     * syncIPrefs.
     */

    /* parameter removed by jimm, 11/4/85
     * documentation and interface require no
     * parameter.  FetchIBase() used instead.
     */
    RemakeDisplay ();
    updateMousePointer (NULL);

    if (IBase->InputRequest.io_Device)
    {
	CopyMem (&IBase->InputRequest, &IBase->IOExcess,
		 sizeof (struct timerequest));

	IBase->IOExcess.tr_node.io_Command = IND_SETTHRESH;
	IBase->IOExcess.tr_time.tv_secs = Pref->KeyRptDelay.tv_secs;
	IBase->IOExcess.tr_time.tv_micro = Pref->KeyRptDelay.tv_micro;
	DoIO (&IBase->IOExcess);
	IBase->IOExcess.tr_node.io_Command = IND_SETPERIOD;
	IBase->IOExcess.tr_time.tv_secs = Pref->KeyRptSpeed.tv_secs;
	IBase->IOExcess.tr_time.tv_micro = Pref->KeyRptSpeed.tv_micro;
	DoIO (&IBase->IOExcess);
    }
    UNLOCKIBASE ();
}

/*****************************************************************************/

/* Do not call with numcolors > 4!! */
static void setWBColorsGruntGrunt (struct Screen *screen, LONG firstcolor, LONG numcolors, LONG ibcolor)
{
    /* One long for each of red, green, blue, plus a initialization
     * LONG and a terminator.
     */
    ULONG loadtable[2 + (4) * 3];
    UBYTE *colorptr;
    ULONG *loadptr;
    LONG i;
    struct IntuitionBase *IBase = fetchIBase ();

    /* LoadRGB32() takes a table whose first word is the number
     * of colors, and whose second word is the first color to load.
     */
    *((UWORD *) loadtable) = numcolors;
    *(((UWORD *) loadtable) + 1) = firstcolor;
    loadptr = &loadtable[1];

    colorptr = &IBase->WBColors[3 * ibcolor];

    DCOLORS (printf ("sWBCGG screen %lx colorptr %lx\n", screen, colorptr));
    DCOLORS (printf ("firstcolor %lx, numcolors %lx\n", firstcolor, numcolors));
    DCOLORS (printf ("viewport at %lx\n", &screen->ViewPort));

    for (i = 0; i < 3 * numcolors; i++)
    {
	/* Move that byte into the high-byte */
	*loadptr++ = (*colorptr++) << 24;
    }
    /* Terminate that table! */
    *loadptr = 0;

    LoadRGB32 (&screen->ViewPort, loadtable);
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/*** intuition.library/GetDefPrefs ***/

struct Preferences *GetDefPrefs (struct Preferences *preferences, LONG size)
{
    /* JON PRINCE SEZ:
    ** Set the system defaults
    ** Used for total reset and loss or corruption of the data file */
    copyprefs (&DefaultPreferences, size, preferences);
    return (preferences);
}

/*****************************************************************************/

/*** intuition.library/GetPrefs ***/

struct Preferences *GetPrefs (struct Preferences *preferences, LONG size)
{
    struct IntuitionBase *IBase = fetchIBase ();

    /* NOTE FROM JON:
    ** Does this mean that the current preferences have been
    ** requested, or has the stupid user made yet another mistake
    ** One hopes for the former but unfortunately expects the
    ** latter. I just hope the argument is valid.
    ** Let's just fill it in anyway - Thats what I'm here for!
    */

    copyprefs (IBase->Preferences, size, preferences);

    return (preferences);
}

/*****************************************************************************/

/*** intuition.library/SetPrefs ***/

/* Intuitionface.asm calls here through stackSwap() */

struct Preferences *setPrefs (struct Preferences *preferences, LONG size, BOOL RealThing)
{
    struct IntuitionBase *IBase = fetchIBase ();

    D (printf ("SP: realthing: %ld\n", RealThing));

    copyprefs (preferences, size, IBase->Preferences);

    D (printf ("SetPrefs calling syncIP\n"));
    syncIPrefs (size);		/* initialize new prefs by old prefs, once */

    setIBasePrefs ();

    D (printf ("SP:sIBP returned\n"));

    if (RealThing)
	sendISM (itNEWPREFS, NULL, NULL, NULL);

    /* Certain things may only happen once */
    IBase->Flags |= SEENSETPREFS | SEENIPOINTER;

    return (preferences);
}

/*****************************************************************************/

void setWBColorsGrunt (struct Screen *screen, BOOL allcolors)
{
    UBYTE depth;

    DCOLORS (printf ("sWBCG: sc %lx\n", screen));

    if (!screen)
	return;

    depth = screen->RastPort.BitMap->Depth;

    /* Set the first four IBase colors into the first four colors of the screen */
    setWBColorsGruntGrunt (screen, 0, 4, WBCOLOR_FIRST4);

    /* If allcolors were requested, put the next four IBase
     * colors into the last four colors of the screen. */
    if ((allcolors) && (depth > 2))
    {
	setWBColorsGruntGrunt (screen, XSC (screen)->LastColor - 3, 4, WBCOLOR_LAST4);
    }

    /* And set the sprite colors */
    setWBColorsGruntGrunt (screen, 17, 3, WBCOLOR_POINTER);
}
