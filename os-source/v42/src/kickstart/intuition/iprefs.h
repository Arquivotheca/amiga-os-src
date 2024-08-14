#ifndef INTUITION_IPREFS_H
#define INTUITION_IPREFS_H

/*** iprefs.h ****************************************************************
 *
 *  private version of data structures for new Intuition prefs
 *
 *  $Id: iprefs.h,v 40.0 94/02/15 17:37:00 davidj Exp $
 *
 ****************************************************************************
 * CONFIDENTIAL and PROPRIETARY
 * Copyright (C) 1989, COMMODORE-AMIGA, INC.
 * All Rights Reserved
 ****************************************************************************/

#ifndef LIBRARIES_DISKFONT_H
#include <libraries/diskfont.h>
#endif

#ifndef INTUITION_SCREENS_H
#include <intuition/screens.h>
#endif

/* Different new preferences chunks recognized by Intuition	*/
#define IP_SCREENMODE	(1)
#define IP_FONT		(2)
#define IP_OVERSCAN	(3)
#define IP_ICONTROL	(4)
#define IP_POINTER	(5)
#define IP_PALETTE	(6)
#define IP_NEWPOINTER	(7)
#define IP_NEWPALETTE	(8)
#define IP_PENS		(9)

/* different Intuition system fonts	*/
#define IPF_SYSFONT	(0)
#define IPF_SCREENFONT	(1)

/* interface data */
/* same as internal format, for now	*/


/* Data for IP_SCREENMODE: */

struct IScreenModePrefs
{
    ULONG			ism_DisplayID;
    UWORD			ism_Width;
    UWORD			ism_Height;
    UWORD			ism_Depth;
    UWORD			ism_Control;
};

/* ism_Control flags	*/
#define WBAUTOSCROLL	(0x0001)
#define WBINTERLEAVED	(0x0002)


/* Data for IP_FONT: */
struct TAttrBuff
{
    struct TextAttr	tab_TAttr;
    UBYTE		tab_NameBuff[ MAXFONTNAME ];
};

struct IFontPrefs
{
    struct TAttrBuff	ifp_TABuff;
    struct TextFont	*ifp_Font;		/* result of MY open	*/
    UWORD		ifp_SysFontType;	/* this font is this?	*/
};


/* Data for IP_OVERSCAN: */
/* stored in graphics database: no IBase copy	*/
struct IOverscanPrefs
{
    ULONG			ios_DisplayID;	/* display id	*/
    Point			ios_ViewPos;	/* View origin in ViewRes */
    Point			ios_Text;	/* TxtOScan in DisplayID-res */
    struct Rectangle		ios_Standard;   /* StdOScan in DisplayID-res */
};


/* Data for IP_ICONTROL: */

struct IIControlPrefs
{
    UWORD iic_TimeOut;			/* Verify timeout */
    WORD  iic_MetaDrag;			/* Meta drag mouse event */
    ULONG iic_ICFlags;			/* IControl flags (see above) */
    UBYTE iic_WBtoFront;		/* CKey: WB to front */
    UBYTE iic_FrontToBack;		/* CKey: front screen to back */
    UBYTE iic_ReqTrue;			/* CKey: Requester TRUE */
    UBYTE iic_ReqFalse;			/* CKey: Requester FALSE */
};

/* Data for IP_POINTER: */

struct IPointerPrefs
{
    UWORD	*ipp_PointerData;
    UWORD	ipp_Height;	
    /* size of the *active* part of the pointer; the
     * data that must be presented is actually += 2 rows
     */

    WORD	ipp_HotSpotX;
    WORD	ipp_HotSpotY;
    UWORD	*ipp_AttachedData;	/* if non-NULL, attach sprites */
    ULONG	ipp_Reserved[4];
};

/* Data for IP_NEWPOINTER: */

struct INewPointerPrefs
{
    struct BitMap *inp_BitMap;
    WORD	inp_HotSpotX;
    WORD	inp_HotSpotY;
    WORD	inp_WordWidth;
    WORD	inp_XResolution;
    WORD	inp_YResolution;
    WORD	inp_Type;
    LONG	inp_Flags;	/* None currently defined */

#define NPT_DEFAULT	0
#define NPT_BUSY	1
};

struct IPenPrefs
{
    UWORD	ipn_NumPens;	/* How many pens are being passed, including ~0 */
    UWORD	ipn_PenType;	/* See PENTYPE_ defines below */
    ULONG	ipn_Reserved;	/* For future use */
    UWORD	ipn_FirstPen;
#define PENTYPE_FOURCOLORPENS	0	/* Pen array to use for four color screens */
#define PENTYPE_EIGHTCOLORPENS	1	/* Pen array to use for eight+ color screens */
};

#endif /* INTUITION_IPREFS_H */
