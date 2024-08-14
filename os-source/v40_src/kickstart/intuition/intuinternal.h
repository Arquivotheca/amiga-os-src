

#ifndef INTUITION_INTUINTERNAL_H
#define INTUITION_INTUINTERNAL_H TRUE

/*** intuinternal.h **********************************************************
 *
 *  intuition internals
 *
 *  $Id: intuinternal.h,v 38.29 92/12/09 18:11:56 peter Exp $
 *  Confidential Information: Commodore-Amiga Computer, Inc.
 *  Copyright (c) Commodore-Amiga Computer, Inc.
 ****************************************************************************/

#include "ibase.h"

#ifndef UTILITY_HOOKS_H
#include <utility/hooks.h>
#endif

#ifndef EXEC_ALERTS_H
#include <exec/alerts.h>
#endif

#define	SIMPLESPRITES	1	/* glory, but with a loophole */

/* Macro to strip out TAG_USER from tag value, in order to save space. */
#define GetUserTagData(tag,def,list) GetTagDataUser((tag)&~(TAG_USER),def,list)
#define GetUserTagData0(tag,list) GetTagDataUser0((tag)&~(TAG_USER),list)

/* both vudu	*/
#define copyBox( from, to ) (getImageBox( (from), (to) ) )

struct RomGadget {
    BYTE	l, t, w, h;
    UBYTE	relflags;	/* rest of flags GADGHCOMP, GADGIMAGE, 
				 * except in special cases
				 */
    UBYTE	type;
    struct Image *image;
};


/* ======================================================================== */
/* === Preferences Definitions ============================================ */
/* ======================================================================== */

/* jimm: july 27 1986: MoveSprite error back in. */
#define SPRITEERROR	(-1)

/* these are the defines for the Intuition default Pointer */
#define ROMPOINTER_HEIGHT	16
#define ROMPOINTER_WIDTH	16
#define ROMPOINTER_HOTX		(0+(SPRITEERROR))
#define ROMPOINTER_HOTY		0
#define ROMPOINTER_XRESN	POINTERXRESN_LORES
#define ROMPOINTER_YRESN	POINTERYRESN_DEFAULT
#define ROMPOINTER_WORDWIDTH	1

/* and these are the defines for the Intuition default Busy Pointer! */
#define ROMBUSYPOINTER_HEIGHT	16
#define ROMBUSYPOINTER_WIDTH	16
#define ROMBUSYPOINTER_HOTX	(-5+(SPRITEERROR))
#define ROMBUSYPOINTER_HOTY	0
#define ROMBUSYPOINTER_XRESN	POINTERXRESN_LORES
#define ROMBUSYPOINTER_YRESN	POINTERYRESN_DEFAULT
#define ROMBUSYPOINTER_WORDWIDTH	1

/* This is the private struct MousePointer definition */

struct MousePointer
{
    /* The first five fields constitute the "transparent base class"
     * of pointerclass objects.  struct Window contains an embedded
     * instance of the TBC, which we use.  However, the details of
     * the TBC are **** INTUITION PRIVATE ****.
     * Use of other fields (xmpt_) of the MousePointer requires assurance
     * that we have a full legitimate MousePointer.  This is done by
     * testing mpt_Width for equality with MPT_CUSTOMWIDTH.
     */
    struct BitMap *mpt_BitMap;	/* For non-extended MousePointers, this is
				 * a pointer to old-style SpriteData.  For
				 * extended MousePointers, this field points
				 * to bitmap containing sprite imagery.
				 * Width/Height are also there?
				 */
    UBYTE mpt_Height;		/* Sprite height */
    UBYTE mpt_Width;		/* Sprite width */
    BYTE mpt_XOffset;		/* Hotspot x-offset */
    BYTE mpt_YOffset;		/* Hotspot y-offset */

    WORD xmpt_WordWidth;	/* Sprite width in words */
    WORD xmpt_PointerXRes;	/* Intended resolution (see POINTERXRESN_ flags) */
    WORD xmpt_PointerYRes;	/* Intended resolution (see POINTERYRESN_ flags) */
};

/* In a couple of places, we need an old MousePointer, which is just
 * the part which is embedded in the Window structure, starting
 * with the Pointer field.
 */

struct OldMousePointer
{
    struct BitMap *mpt_BitMap;	/* A pointer to old-style SpriteData. */
    UBYTE mpt_Height;		/* Sprite height */
    UBYTE mpt_Width;		/* Sprite width */
    BYTE mpt_XOffset;		/* Hotspot x-offset */
    BYTE mpt_YOffset;		/* Hotspot y-offset */
};

/* All mouse pointers from pointerclass have mpt_Width equal to this
 * special value:
 */
#define MPT_CUSTOMWIDTH	0xFF

/* Internally, this constant is used to denote the default busy pointer */
#define BUSYPOINTER	((struct MousePointer *)1)

/* This is the timeout for the delayed pointer activation */
#define DEFERREDPOINTERCOUNT	3	/* ticks */





/* various timing rates */
#define KEYREPSEC	0	/* struct timeval KeyRptSpeed == 0.05 Secs */
#define KEYREPMIC 	 50000	/* struct timeval KeyRptSpeed */
#define KEYDELSEC	0	/* struct timeval KeyRptDelay == 0.60 Secs */
#define KEYDELMIC	600000	/* struct timeval KeyRptDelay */
#define MOUSEDBLSEC	1	/* struct timeval DoubleClick == 1.50 Secs */
#define MOUSEDBLMIC	500000	/* struct timeval DoubleClick */

/* Intuition Pointer defaults */
#define POINTERX (ROMPOINTER_HOTX)		/* BYTE XOffset; */
#define POINTERY (ROMPOINTER_HOTY)		/* BYTE YOffset; */
#define COLOR17	0xE44		/* Red:   USHORT color17; */
#define COLOR18	0x000		/* Black: USHORT color18; */
#define COLOR19	0xEEC		/* Cream: USHORT color19; */
#define POINTERTICKS 1		/* USHORT PointerTicks; */

/* Workbench Screen colors, colors 0 to 3 and ~3 to ~0 */
#define COLOR0  0xAAA		/* Gray */
#define COLOR1  0x000		/* Black */
#define COLOR2  0xFFF		/* White */
#define COLOR3  0x68B		/* Blue */
#define COLORN3 0xE44		/* Red */
#define COLORN2 0x5D5		/* Green */
#define COLORN1 0x04D		/* Blue */
#define COLORN0 0xE90		/* Orange */


/* positioning data for the Intuition View */
#define VIEWX 0			/* USHORT XScreenOffset; */
#define VIEWY 0			/* USHORT YScreenOffset; */

/* miscellaneous */
#define ENABLECLI FALSE		/* BOOL EnableCLI; is FALSE */

/* printer configurations */
#define PRINTERTYPE EPSON	/* USHORT PrinterType; */

/* print format and quality configurations */
#define PRINTPITCH PICA		/* BYTE PrintPitch; of 10 cps */
#define PRINTQUALITY DRAFT	/* BYTE PrintQuality; is draft */
#define PRINTSPACING SIX_LPI	/* BYTE PrintSpacing;  is 6 lpi */
#define PRINTLEFTMARGIN 5	/* FLOAT PrintLeftMargin; */
#define PRINTRIGHTMARGIN 75	/* FLOAT PrintRightMargin; */
#define PRINTINVERSE 0		/* BYTE PrintInverse; is positive */
#define PRINTASPECT 0		/* BYTE PrintAspect; is horizontal */
#define PRINTSHADE 1		/* BYTE PrintShade; is black & white */
#define PRINTTHRESHOLD 2	/* WORD PrintThreshold; is middle */

/* print paper descriptors */
#define PAPERSIZE N_TRACTOR	/* BYTE PaperSize; is U.S. Letter */
#define PAPERWIDTH 9.5		/* FLOAT PaperWidth; */
#define PAPERLENGTH 66		/* FLOAT PaperLength; */
#define PAPERTYPE FANFOLD	/* BYTE PaperType; is fanfold */

/* default name for the disk-based Preferences file */
#define	PREF_FILE	".prefs"


/* ======================================================================== */
/* ======================================================================== */

/* = global INTUITION declarations ======================================== */

/* these are the IBase->Flags definitions */
#define DRAGSELECT	0x00000002 /* currently drag-selecting in menus */
#define GADGETON	0x00000004 /* GRELEASE highlighted flag
				    * NUMERICALLY EQUAL TO GMR_GADGETHIT!!!
				    */
#define COM_SELECT	0x00000020 /* if COMMAND-SELECT is in effect */
#define COM_MENU	0x00000040 /* if COMMAND-MENU is in effect */

#define ITEMDRAWN    	0x00000080 /* if the menu window is currently drawn */
#define SUBDRAWN     	0x00000100 /* if the submenu is currently drawn */
#define GOODITEMDRAWN	0x00002000 /* the item was successfully drawn */
#define GOODSUBDRAWN	0x00004000 /* the subitem was successfully drawn */

#define HIT_BARLAYER	0x00000001 /* hitUpfront detected barlayer hit */
#define MACHINE_ISPAL	0x00000008 /* If we found PAL hardware upon init */
#define RELEASED	0x00000800 /* Double-Click button watcher flag */
#define VIRGINDISPLAY	0x00001000 /* Display not opened by anyone yet */
#define POPPEDSCREEN	0x00020000 /* autorequest screen popped	*/

#define SEENSETPREFS	0x00010000 /* seen at least one SetPrefs */
#define SEENIPOINTER	0x00040000 /* SetPrefs shouldn't set pointer */
#define SEENIPALETTE	0x00000400 /* SetPrefs shouldn't set colors */

#define INMOUSELIMITS	0x00000010 /* don't do freeMouse(),  I'm busy */
#define SCREENMLIMITS	0x00008000 /* Use screen-relative mouse-limits */

#define GAD_SELECTDOWN	0x00000200 /* Select button is down over a gadget */

#define POINTER_XDOUBLE	0x00080000 /* Double pointer horiz to preserve aspect */
#define POINTER_YDOUBLE	0x00100000 /* Double pointer vertical */

/* Unused: 0x00200000, 0x00400000,0x00800000, 0xXX000000 */

/* key codes */
#define SHIFTY (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT/*|IEQUALIFIER_CAPSLOCK*/)


/* Layers now sets two additional refresh bits for us.
 * We use the first one to track which windows have been notified
 * since the last damage, and we use the other to sniff when a boopsi
 * gadget might have caused damage through ScrollRaster().
 */
#define LAYERI_NOTIFYREFRESH	LAYERIREFRESH
#define LAYERI_GADGETREFRESH	LAYERIREFRESH2

/* --- some menu MACROS --------------------------------------------------- */
#define SETDMENU(n) IBase->MenuDrawn = \
	(IBase->MenuDrawn & 0xFFE0) | (n & 0x1F)
#define SETDITEM(n) IBase->MenuDrawn = \
	(IBase->MenuDrawn & 0xF81F) | ((n & 0x3F) << 5)
#define SETDSUB(n) IBase->MenuDrawn = \
	(IBase->MenuDrawn & 0x07FF) | ((n & 0x1F) << 11)

#define SETSMENU(n) IBase->MenuSelected = \
	(IBase->MenuSelected & 0xFFE0) | (n & 0x1F)
#define SETSITEM(n) IBase->MenuSelected = \
	(IBase->MenuSelected & 0xF81F) | ((n & 0x3F) << 5)
#define SETSSUB(n) IBase->MenuSelected = \
	(IBase->MenuSelected & 0x07FF) | ((n & 0x1F) << 11)

/* This define is for the pauses between blinks during Alerts.  This is
 * how many times Intuition reads the mouse registers before changing the
 * color of the Alert border.  Also, by wild coincidence, 6809 is also
 * the processor that Jack Haeger, Sam Dicker and I, =RJMical=, did
 * Sinistar on, with Noah Falstein and Rich Witt.  
 */
#define ALERT_COUNTDOWN 0x6809

/* I won't use this ridiculous name */
#define SwapBits SwapBitsRastPortClipRect

/* nor this */
#define sc_BitMap	BitMap	/* for Screen.BitMap	*/


/* === Default sizes for the Intuition display ============================ */
/* These are used for creating and manipulating the Intuition displays */
#if 0	/*	OBSOLETE: jimm: 1/20/88		*/
#define MAXDISPLAYHEIGHT	(IBase->MaxDisplayHeight)
#define MAXDISPLAYROWS		(IBase->MaxDisplayRow)
#define MAXDISPLAYWIDTH		(IBase->MaxDisplayWidth)
#define MAXDISPLAYCOLUMNS	(MAXDISPLAYWIDTH - 1)
#endif

#define MAXWINDOWLAYERS	10

/* IWindowDepth() takes the following flags:
 * WDEPTH_TOFRONT - Send the window to front.
 * WDEPTH_TOBACK - Send the window to back.
 * WDEPTH_INFRONTOF - Send the window in front of the reference window.
 */
#define	WDEPTH_TOFRONT 		(0)
#define WDEPTH_TOBACK  		(1)
#define WDEPTH_INFRONTOF	(2)

/* IScreenDepth() and relinkScreen() share a bunch of flags:
 * SDEPTH_TOFRONT - Bring the screen to the front of the list or its family
 * SDEPTH_TOBACK - Bring the screen to the back of the list or its family
 * SDEPTH_INFAMILY - Move the screen within the family, instead of moving
 *	the whole family.
 * SDEPTH_USERMASK - Mask to protect against user setting internal-only bits.
 * SDEPTH_DIRECTIONMASK - mask all but SDEPTH_TOFRONT/SDEPTH_TOBACK
 * SDEPTH_INITIAL - Signifies the initial linking of the screen, and
 *	asks relinkScreen() to not delink the screen first.
 *
 * These three flags are in the public includes:
 * #define SDEPTH_TOFRONT	(0)
 * #define SDEPTH_TOBACK	(1)
 * #define SDEPTH_INFAMILY	(2)
 */

#define SDEPTH_USERMASK		(SDEPTH_TOFRONT|SDEPTH_TOBACK|SDEPTH_INFAMILY)
#define SDEPTH_DIRECTIONMASK	(1)
#define SDEPTH_INITIAL		(4)


/* IMoveScreen() and screenLegalPosition() share a bunch of flags:
 * SPOS_FORCEDRAG - Caller wants to move {SA_Draggable,FALSE} screen.
 * SPOS_ABSOLUTE - Means that the coordinates are absolute, not relative.
 * SPOS_BOUNDEDBOTTOM / SPOS_UNBOUNDEDBOTTOM - screenLegalPosition() can
 *	be unbounded on the bottom.  By default, it's unbounded for
 *	child screens, since pulling a parent screen down can send a child
 *	screen far off the bottom.  These flags override the unboundedness.
 *	MoveScreen() has always been unbounded on the bottom, so it sets
 *	SPOS_UNBOUNDEDBOTTOM.  The screen dragging code uses sLP() to
 *	figure out the legal mouse range, and it must force a bounded
 *	bottom to keep the user from dragging a child screen itself off the
 *	bottom.
 * SPOS_MENUSNAPRANGE - Allow screen to move farther right than normally
 *	(up to the Text Overscan left-edge) to permit screen menu-snap.
 * SPOS_REVALIDATE - Force revalidation of legal position when un-snapping
 *	the menu.
 * SPOS_NORETHINK - Asks IMoveScreen() to skip calling RethinkDisplay().
 *	When OpenScreen() has to move child-screens into legal place,
 *	we hold off the RethinkDisplay() since the new screen's ViewPort
 *	is not linked in yet, and RethinkDisplay() is coming up anyways.
 * SPOS_USERMASK - Mask to protect against user setting internal-only bits.
 * SPOS_TYPEMASK - Mask that filters all but the exclusive choices of
 *	positioning-type.
 *
 * These two flags are in the public includes:
 * #define SPOS_RELATIVE	(0)
 * #define SPOS_ABSOLUTE	(1)
 * #define SPOS_MAKEVISIBLE	(2)
 * #define SPOS_FORCEDRAG	(4)
 */

#define SPOS_BOUNDEDBOTTOM	(8)
#define SPOS_UNBOUNDEDBOTTOM	(16)
#define SPOS_MENUSNAPRANGE	(32)
#define SPOS_REVALIDATE		(64)
#define SPOS_NORETHINK		(128)

#define SPOS_TYPEMASK		(SPOS_MAKEVISIBLE|SPOS_ABSOLUTE|SPOS_RELATIVE)
#define SPOS_USERMASK		(SPOS_FORCEDRAG|SPOS_TYPEMASK)

/* Flags to drawGadgets() and drawEmbossedBorder():
 *
 * The drawGadgets() function takes a list of gadgets and a filter
 * flag.  Based on the value of the filter flag, each gadget in the
 * list is rendered or skipped, as follows:
 *
 * DRAWGADGETS_ALL - draw all the gadgets
 * DRAWGADGETS_BORDER - draw all gadgets that are in a window border
 * DRAWGADGETS_DAMAGE - draw all gadgets which have the GMORE_SCROLLRASTER
 *	flag set, indicating they use ScrollRaster() and are subject
 *	to damage (hence repair).
 *
 * NB: intuitionface.asm depends on DRAWGADGETS_ALL being zero.
 */
#define DRAWGADGETS_ALL		0
#define DRAWGADGETS_BORDER	1
#define DRAWGADGETS_DAMAGE	2

/* Flags for itACTIVATEWIN token:
 *
 * AWIN_NORMAL - Regular window activation
 * AWIN_INITIAL - Initial activation of a new window
 * AWIN_LENDMENU - temporary activation to lend menu action
 * AWIN_LENDMENUKEY - temporary activation to lend menu key action
 *
 */

#define AWIN_NORMAL		0
#define AWIN_INITIAL		1
#define AWIN_LENDMENU		2
#define AWIN_LENDMENUKEY	3


#if 0
#define	MAX(A, B)	(((A)<(B))?(B):(A))
#define	MIN(A, B)	(((A)>(B))?(B):(A))
#else
#define	MAX(A, B)	imax( (A), (B) )
#define	MIN(A, B)	imin( (A), (B) )
#endif
/*******************/

/* get the address of the IntuitionBase */
#if 0
#define fetchIBase()	((struct IntuitionBase *) *((ULONG *)0xD0))
struct IntuitionBase *fetchIBase();
#else
#define fetchIBase()	(IBaseDefine)
extern struct IntuitionBase *IBaseDefine;
#endif

/* converts pointer to an IBox to a Point */
#define upperLeft(A) (*((struct Point *) (A)))

#define MAX_SIGNED_WORD	32767
#define MIN_SIGNED_WORD -32768

/* She'll always be Karla to me ... */
#define KARLA	"topaz.font"
/*???#define KARLA	"Karla"*/

#define SI( gadget ) 	((struct StringInfo *) ((gadget)->SpecialInfo))
#define PI( gadget ) 	((struct PropInfo *) (gadget)->SpecialInfo)
#define BI( gadget ) 	((struct BoolInfo *) (gadget)->SpecialInfo)

#define CLEARFLAG( var, flag ) ((var) &= ~(flag))
#define SETFLAG( var, flag ) ((var) |= (flag))
#define TESTFLAG( var, flag ) ( (var) & (flag) )

/** Display Vuuduu	**/
/* D_O was 13 */
#define DENISE_OFFSET	9
#define STAND_HCLOCKS	226
#define STAND_ROWS	262

/* absolute hardware limits, hardware relative, in hi-res, lace coords	*/
#define MIN_LEFT_VISUAL	((0x81 - 64) << 1)	/* dale sez	*/
#define MIN_TOP_VISUAL	((0x2c - 0x1b + 4) << 1)

/* init values for Left/RightHardLimit, used in dclip.c		*/
/* these are in hi-res, lace coordinates, hardware relative	*/
#define	LEFT_HARD_LIMIT		(512)	/* dwistop can't be below 512/2 */
#define	RIGHT_HARD_LIMIT	(1000)

#if 0	/* these are some obsolete values	*/
#define LEFT_HARD_LIMIT 284+32
#define RIGHT_HARD_LIMIT 668
#endif


#define ratioFIXEDPART	4
/*#define RATIO(n, r) (((n) * (r)) >> ratioFIXEDPART)*/
#define RATIO(n, r) ((((n)*(r))+(1<<(ratioFIXEDPART-1)))>>ratioFIXEDPART)

#define REALBIG	(7000)	/* bigger than any window or screen */

#define ALENGTH( array )	( sizeof ( array )/sizeof ( array[0]) )


#if ONCOLORS
#define ICOLOR(c)	 (*((SHORT *)0xdff180) = (c))
#define CKCOLOR0 (0xf00)
#define CKCOLOR1 (0x000)
#define CKCOLOR2 (0x0f0)
#define CKCOLOR3 (0x000)
#define CKCOLOR4 (0x00f)
#define CKCOLOR5 (0x000)
#define CKCOLOR6 (0xfff)
#define CKCOLOR7 (0x000)
#define CKCOLOR8 (0xff0)
#define CKCOLOR9 (0x000)
#define CKCOLOR10 (0x0ff)
#define CKCOLOR11 (0x000)

#define DONECOLOR (0xfff)

#else
#define ICOLOR(c)	;
#endif

/* mask macros */
/* width in bytes of word-padded raster (from gfx.h) */
#define RASWIDTH(W) ( ((W)+15) >> 3 & 0xFFFE )

/* uword with single bit set in x pixel position */
#define MASKBIT(X)  ((UWORD) 1 << (15 - ((X) & 0x0F)))

/* pointer to word that point p lies in mask plane P of width W  */
#define MASKWORD(p, W, P) ((UWORD *)(((UWORD *) P) + ((p).Y*RASWIDTH(W))/2 + (((p).X) >> 4)))

/* finally, test if point p is in mask P of width W */
#define MASKTEST(p, W, P) ((BOOL) (*MASKWORD(p, W, P) & MASKBIT((p).X)))

/*
 * tells whether a gadget is to be highlighted by COMP or BOX
 * XOR toggling.
 */
#define ISTOGGLE( g ) ( !( (g)->Flags  & 2 ) )

/*
 * type Pointer to Function returning Unsigned long
 */
typedef ULONG	(*PFU)();

#define  TRIGGER	{kprintf("trigger\n");(* ((short *) 0x2fe) )  = 0xdead;}

#define PANIC( str )	kprintf("panic: %s\n", str ); Debug();


#define OLD_MODES	( LACE | HIRES | HAM | DUALPF | PFBA | EXTRA_HALFBRITE)

/* display ID mask to monitor part */
#define MONITOR_PART( displayid ) ((displayid) & MONITOR_ID_MASK)

#define ALLBORDERS (BORDERSNIFF|TOPBORDER|RIGHTBORDER|BOTTOMBORDER|LEFTBORDER)

/* modeVerify() and rethinkVPorts parameters */
#define NOFORCE (0)		/* don't remake all copper lists	*/
#define FORCE (1)   /* remake all copper lists, but don't sync vp dx/yoffset*/
#define MEGAFORCE (2)	/* sync vps with screen positions and remake all */

/* indexes into array passed to containerpens	*/
enum {
	CP_FILL,
	CP_PATTERN,
	CP_BORDER,


	NUMCONTAINERPENS
};

/* The frameEdge() function takes these definitions to figure out
 * which edge to draw:
 */
#define FRAMEEDGE_VERTICAL	2

#define FRAMEEDGE_TOP		0
#define FRAMEEDGE_BOTTOM	1
#define FRAMEEDGE_LEFT		( 0 | FRAMEEDGE_VERTICAL )
#define FRAMEEDGE_RIGHT		( 1 | FRAMEEDGE_VERTICAL )

/* MENUHELP stuff	*/
#define KEYCODE_HELP	(0x5f)

#ifndef IECLASS_MENUHELP
#define IECLASS_MENUHELP	(0x14)
#endif

#ifndef IECLASS_CHANGEWINDOW
#define IECLASS_CHANGEWINDOW	(0x15)
#endif

#ifndef IECLASS_GADGETHELP
#define IECLASS_GADGETHELP	(0x16)
#endif

#endif
