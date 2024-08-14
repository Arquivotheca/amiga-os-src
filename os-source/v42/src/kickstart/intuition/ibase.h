#ifndef INTUITION_INTUITIONBASE_H
#define INTUITION_INTUITIONBASE_H


/*** ibase.h *********************************************************
 *
 *  the IntuitionBase structure and supporting structures (INTERNAL ONLY)
 *
 *  $Id: ibase.h,v 40.0 94/02/15 18:06:21 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga Computer, Inc.
 *  Copyright (c) Commodore-Amiga Computer, Inc.
 ****************************************************************************/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

/* */

#ifndef EXEC_LIBRARIES_H
#include <exec/libraries.h>
#endif

#ifndef GRAPHICS_GFXBASE_H
#include <graphics/gfxbase.h>
#endif

#ifndef GRAPHICS_SPRITE_H
#include <graphics/sprite.h>
#endif

#ifndef INTUITION_INTUITION_H
#include <intuition/intuition.h>
#endif

#ifndef UTILITY_HOOKS_H
#include <utility/hooks.h>
#endif

#ifndef DEVICES_INPUTEVENT_H
#include <devices/inputevent.h>
#endif


#ifndef INTUITION_CGHOOKS_H
#include "cghooks.h"
#endif

#ifndef INTUITION_SGHOOKS_H
#include "sghooks.h"
#endif


#ifndef EXEC_INTERRUPTS_H
#include <exec/interrupts.h>
#endif

#ifndef INTUITION_IPREFS_H
#include "iprefs.h"
#endif

#ifndef INTUITION_PREFERENCES_H
#include <intuition/preferences.h>
#endif





#ifndef IESUBCLASS_NEWTABLET
/*	ie_EventAddress points to struct IEPointerNewTablet */
#define IESUBCLASS_NEWTABLET       0x03

/* The ie_EventAddress of an IECLASS_NEWPOINTERPOS event of subclass
 * IESUBCLASS_NEWTABLET points at an IENewTablet structure.
 *
 *
 * IEQUALIFIER_RELATIVEMOUSE is not supported for IESUBCLASS_NEWTABLET.
 */

struct IENewTablet
{
    /* Pointer to a hook you wish to be called back through, in
     * order to handle scaling.  You will be provided with the
     * width and height you are expected to scale your tablet
     * to, perhaps based on some user preferences.
     * If NULL, the tablet's specified range will be mapped directly
     * to that width and height for you, and you will not be
     * called back.
     */
    struct Hook *ient_CallBack;

    /* Post-scaling coordinates and fractional coordinates.
     * DO NOT FILL THESE IN AT THE TIME THE EVENT IS WRITTEN!
     * Your driver will be called back and provided information
     * about the width and height of the area to scale the
     * tablet into.  It should scale the tablet coordinates
     * (perhaps based on some preferences controlling aspect
     * ratio, etc.) and place the scaled result into these
     * fields.  The ient_ScaledX and ient_ScaledY fields are
     * in screen-pixel resolution, but the origin ( [0,0]-point )
     * is not defined.  The ient_ScaledXFraction and
     * ient_ScaledYFraction fields represent sub-pixel position
     * information, and should be scaled to fill a UWORD fraction.
     */
    UWORD ient_ScaledX, ient_ScaledY;
    UWORD ient_ScaledXFraction, ient_ScaledYFraction;

    /* Current tablet coordinates along each axis: */
    ULONG ient_TabletX, ient_TabletY;

    /* Maximum tablet range along each axis: */
    ULONG ient_RangeX, ient_RangeY;

    /* Pointer to tag-list of additional tablet attributes.
     * See <intuition/intuition.h> for the tag values.
     */
    struct TagItem *ient_TagList;
};


#endif /* temporary definition until devices/inputevent.* learns */






/* these are the display modes for which we have corresponding parameter
 *  settings in the config arrays
 */
#define DMODECOUNT	0x0002	/* how many modes there are */
#define HIRESPICK	0x0000
#define LOWRESPICK	0x0001

#define EVENTMAX 10		/* size of event array */

/* Number of possible sprite pointer sizes, namely 16-bit, 32-bit, 64-bit */
#define POINTER_SIZES	3

/* these are the system Gadget defines */
#define RESCOUNT	2
#define HIRESGADGET	0
#define LOWRESGADGET	1

#define GADGETCOUNT	8
#define UPFRONTGADGET	0
/* #define DOWNBACKGADGET	1 */	/* obsolete */
#define ZOOMGADGET 	1
#define SIZEGADGET	2
#define CLOSEGADGET	3
#define DRAGGADGET	4	/* no image */
#define SUPFRONTGADGET	5
#define SDOWNBACKGADGET	6	/* not used */
#define SDRAGGADGET	7	/* no image */

/* jimm: 1/10/86: Intuition Locking */
/* Let me say it again: don't even think about using this information
 * in a program.
 */
#define ISTATELOCK	0	/* Intuition() not re-entrant		*/
#define	LAYERINFOLOCK	1	/* dummy lock used to check protocol	*/
#define GADGETSLOCK	2	/* gadget lists, refresh, flags		*/
#define LAYERROMLOCK	3	/* (dummy) for lock layerrom		*/
#define IBASELOCK	4	/* protexts IBase pointers and lists	*/
#define	VIEWLOCK	5	/* access to ViewLord			*/
#define VIEWCPRLOCK	6	/* Dummy for GfxBase ActiViewCprSemaphore */
#define RPLOCK		7	/* use of IBase->RP			*/

#define NUMILOCKS	8

/* Here are some other locks that aren't a normal part of the Intuition
 * locking scheme.  We put them here because it allows us to init them
 * along with the other locks.
 */
#define CLASSLOCK	NUMILOCKS+0	/* protection for class list	*/
#define ALERTLOCK	NUMILOCKS+1	/* protection against alert nesting */
#define NUMOTHERLOCKS	2

/** Intuition hook defines (in array IIHooks)	**/
#define IHOOK_SEDIT	0
#define IHOOK_STRINGG	1
#define IHOOK_PROPG	2
#define IHOOK_BOOLG	3

#define NUM_IHOOKS	4


/* ======================================================================== */
/* === Intuition Messages ================================================= */
/* ======================================================================== */

/* Standard Internal form of IntuiMessage */
struct IIntuiMessage
{
    struct IntuiMessage	iim_IntuiMessage;
    struct TabletData	*iim_TabletData;
    ULONG		iim_PrevKeys;
};

/* Internal form of IntuiMessage for WA_TabletMessages windows */
struct TIIntuiMessage
{
    struct IntuiMessage	iim_IntuiMessage;
    struct TabletData	*iim_TabletData;
    ULONG		iim_PrevKeys;
    struct TabletData	tiim_TabletDataInstance;
};

/* Convenient cast to allow access to all fields of a TIIntuiMessage */
#define IIMSG(imsg) ((struct TIIntuiMessage *)(imsg))


/* ======================================================================== */
/* === Gadget "Per List" Environment ====================================== */
/* ======================================================================== */
/*
 * this structure encapsulates a GadgetInfo plus enough cached
 * information to switch between normal and g00 gadgets in
 * the same list.
 */
struct GListEnv {
    struct GadgetInfo	ge_GInfo;

    /* two possible layers for gadgets in a window list:
     * window layer or g00 (border) layer.   All other lists use
     * just ge_Layer.
     */
    struct Layer	*ge_Layer;
    struct Layer	*ge_GZZLayer;	/* border layer for g00 windows */

    /* likewise, two different domains */
    struct IBox		ge_Domain;
    struct IBox		ge_GZZdims;
};

/* IBase->WBColors is as follows:
 * [0..3] - the first four colors of a screen
 * [4..7] - the _last_ four colors of a screen
 * [8..11] - the three pointer colors
 */

#define WBCOLOR_FIRST4	0	/* Base index for first four colors */
#define WBCOLOR_LAST4	4	/* Base index for last four colors */
#define WBCOLOR_POINTER	8	/* Base index for three pointer colors */

#define WBCOLOR_NUMBER	11	/* Number of entries */

/* ======================================================================== */
/* === IntuitionBase ====================================================== */
/* ======================================================================== */

struct Point
{
    WORD X;
    WORD Y;
};

struct LongPoint
{
    LONG LX;
    LONG LY;
};


/* IntuitionBase should never be directly modified by programs  */
/* even a little bit, guys/gals; do you hear me?	*/

/* IntuitionBase needs its fields shuffled for alignment and
 * grouping reasons.  I'm not doing it now because I want
 * the current version of Wack to still be valid for Intuition
 * from Kickstart 39.106 (3.00).
 */
struct IntuitionBase
{
    struct Library LibNode;

    struct View ViewLord;

    struct Window *ActiveWindow;
    struct Screen *ActiveScreen;

				/* poll of uses on 8/23/89:
				 *   window draggie box
				 *   menu resolution
				 *   menu rendering (swapbits)
				 *   accelerator debug
				 *   set by setScreen/setWindow
				 *   screen gadget beep
				 *   cleared if closed
				 */

    struct Screen *FirstScreen; /* for linked list of all screens	*/

    ULONG Flags;		/* definitions in intuinternal.h	*/

    WORD	MouseY, MouseX;
    ULONG	Seconds;	/* timestamp of most recent input event	*/
    ULONG	Micros;

    /***** ***** ***** ***** ***** ***** ***** ***** *****/
    /***** ***** ***** PRIVATE DATA HERE ***** ***** *****/
    /***** ***** ***** ***** ***** ***** ***** ***** *****/

    /* Peter 10-Jan-91:  To fix Tetris, we recreate V34's SimpleSprite
     * pointer.  It is at $3E8(IBase).  Private is at $50(IBase).
     * Peter 30-Jan-91:  To fix TV*Text, we recreate V34's MaxDisplayHeight
     * and MaxDisplayWidth fields.  They're at 0x542(IBase) and 0x546(IBase)
     * Locale needs a pointer to the "System Request" string.  We'll
     * re-use the 1.3 "SysBase" pointer, which is a good idea because
     * it's safe to re-use and it's near the beginning of IntuitionBase,
     * meaning nothing needs to be recompiled and our future options
     * aren't screwed by a fixed location in the middle of shifting
     * fields.  IMPORTANT: SysReqTitle can't be moved!!!
     */
    /* SpecialSpriteKill is a kludge for CDGS to safely keep Intuition
     * from using its own sprite.  When non-zero, Intuition doesn't
     * call MoveSprite() or ChangeExtSprite().
     */
#define PRIVATE_IBOFFSET	( 0x50 )
#define SYSBASE_OFFSET		( 0x60 - PRIVATE_IBOFFSET)
#define	TETRISSPRITE_OFFSET	( 0x3E8 - PRIVATE_IBOFFSET )
#define MAXDISPLAYHEIGHT_OFFSET	( 0x542 - PRIVATE_IBOFFSET )
#define TOTALPRIVATE		( 1300 )

    UBYTE	Private[ SYSBASE_OFFSET ];
    STRPTR	SysReqTitle;
    STRPTR	WBScreenTitle;
    ULONG	SpecialSpriteKill;
    UBYTE	Private2[ TETRISSPRITE_OFFSET - SYSBASE_OFFSET - 12 ];
    struct SimpleSprite *TetrisSprite;
    UBYTE	Private3[ MAXDISPLAYHEIGHT_OFFSET - TETRISSPRITE_OFFSET - 4 ];
    WORD	MaxDisplayHeight;
    WORD	Private4;
    WORD	MaxDisplayWidth;
    UBYTE	Private5[ TOTALPRIVATE - MAXDISPLAYHEIGHT_OFFSET - 6 ];


    /* --------------- base vectors ----------------------------------- */
    /* DO MOVE THESE OFFSETS WITHOUT ADJUSTING EQUATES IN IWORK.ASM
     * this is automatically handled by standalone program offsets.c
     */
    APTR		SysBase;
    struct GfxBase	*GfxBase;
    APTR		LayersBase;
    APTR		UtilityBase;
    APTR		KeymapBase;
    struct Task		*InputDeviceTask;

    /* ---------------- Intuition State Machine ----------------------- */
    UWORD		CurrentState;
    void		(*GadgetReturn)(LONG);
    void		(*SDragReturn)(void);
    void		(*WSDReturn)(void);
    void		(*VerifyReturn)(LONG);
    void		(*DMRReturn)(LONG);
    struct InputToken	*InputToken;
    struct InputEvent	IECopy;		/* copy of recent real input event */

    /**** look at end for more ISM stuff ***/

    /* ---------------- Pools and Queues ------------------------------- */

    /* input token lists	*/
    struct List		TokenQueue;	/* accumulated input tokens	*/
    struct List		DeferredQueue;	/* tokens for next time		*/
    struct List		ITFreeList;	/* pool				*/

    /* input node lists		*/
    struct List		IEFoodList;	/* being sent along to handlers	*/
    struct List		IEQueue;	/* queueing up for next time	*/
    struct List		IEFreeList;	/* pool				*/
    struct List		IECloneList;	/* allocated copies being used */

    /* --------------- Local State ------------------------------------ */

    struct Point	ShiftBack;	/* menu magic return shift	*/
    ULONG		StartSecs;	/* measure double clicks */
    ULONG		StartMicros;

    /* ---------------- Menu Rendering and Operation ------------------ */
    /* menu/item/sub number of current display */
    USHORT		MenuDrawn;
    /* menu/item/sub number of selected (and highlights)*/
    USHORT		MenuSelected;
    USHORT		OptionList;		/* menu selection	*/

    struct RastPort	MenuRPort;
    struct ClipRect	ItemCRect;	/* for the item's display	*/
    LONG		BSZero1;	/* layers assumes this is defined */
    struct ClipRect	SubCRect;	/* for the subitem's display	*/
    LONG		BSZero2;	/* layers assumes this is defined */

    /* ---------------- Input Device Interface ------------------------ */
    struct IOStdReq	InputRequest;
    struct Interrupt	InputInterrupt;

    struct timerequest	IOExcess;

    /* ---------------- Active Gadget Information --------------------- */
    struct Gadget	*ActiveGadget;
    struct GListEnv	GadgetEnv;
    struct Hook		*ActiveHook;	/* hook of active gadget	*/

    /* cached by prop/bool/string GoActive hooks	*/
    struct IBox		ActiveGBox;	/* cached relativity box	*/

    /* cache for active prop gadget	*/
    struct Point	KnobOffset;
    struct PGX		ActivePGX;	/* container, newknob		*/

    /* cache for active string gadget	*/
    struct StringExtend	*ActiveSEx;
    struct KeyMap	*KeyMap;
#define LONGBSIZE	(15)
    /* workbuffer for LONGINT gadgets	*/
    UBYTE		LongBuff[ LONGBSIZE ];

    /* ----------- Intuition's Rendering for Gadgets, Titles, ... ----- */
    /* This will be allocated on init */
    struct RastPort	*RP;
    struct Region	*OldClipRegion;		/* locks with RPort */
    struct Point    	OldScroll;		/* user's Scroll_X/Y*/

    /* ----------- Frame Rendering for Window Size/Drag --------------- */
    struct IBox		IFrame;	/* window frame for sizing/dragging	*/
    void   		(*frameChange)(struct Point); /* function to change IFrame	*/

    /* points to either pos or dims of ChangeBox	*/
    struct Point	*SDDelta;
    struct Point	FirstPt;	/* point from which s/d started */
    struct Point	OldPt;		/* previous point for s/d	*/
    struct IBox		ChangeBox;	/* parameter buffer for sizeDrag */
    UWORD		SizeDrag;	/*  WSD_SIZE or WSD_DRAG	*/

    /* Menu imagery:  menu checkmark and menu Amiga-key in two sizes */
    struct Image	*MenuImage[2][RESCOUNT];

    /* --- Preferences Section ---------------------------------------------- */

    LONG DoubleSeconds, DoubleMicros; /* for testing double-click timeout */

    struct Preferences *Preferences;

    /* ----------------- Workbench Interface ---------------------- */
    struct MsgPort *WBPort;

    struct Screen *HitScreen;	/* set by hitScreen() routine */

    /* ---------------------- Intuition Arbitration ---------------------- */
    struct SignalSemaphore	ISemaphore[NUMILOCKS+NUMOTHERLOCKS];

    /* ---------------------- Display Stuff ------------------------------- */
    struct MonitorSpec *ActiveMonitorSpec;
    struct ViewExtra *ViewExtra;
    WORD ViewInitX, ViewInitY;	/* View initial offsets at startup   */

    /* copies of gfxbase original display defs	*/
    UWORD	OrigNDRows;
    UWORD	OrigNDCols;

    /* ---------------------- Mouse Motion ------------------------------- */
    /* multiply ie_X/Y by this	*/
    /* this is also "ticks per hires pixel"		*/
#define MOUSESCALEX		(22)

    /* Peter 28-Nov-90:  Rather than the Y-ticks per pixel being exactly
     * constant, we fudge it a little for compatibility and common sense,
     * i.e. we adapt it to PAL and VGA, too.
     */
#define MOUSESCALEY		(26)
#define NTSC_MOUSESCALEY	(26)
#define PAL_MOUSESCALEY		(22)
#define VGA_MOUSESCALEY		(22)

    struct Point	EffectiveScaleFactor;
			/* MOUSESCALE divided by PointerTicks	*/

    /* Pointer to last screen passed to tablet callback hook */
    struct Screen	*LastTabletScreen;

    struct Rectangle	MouseLimits;
    SHORT 		HoldMinYMouse;

    /* Peter 3-Apr-91: Screen relative mouse limits to solve
     * autoscroll weirdness, and ScrollFreedom variable.
     */
    struct Rectangle	ScreenMouseLimits;
    UWORD		ScrollFreedom;

    /* Peter 3-Apr-91: IESUBCLASS_TABLET events need to know the
     * mouse's true limits, unconstrained by sizing, dragging, or
     * menu operations.
     */
    struct Rectangle	FreeMouseLimits;


    UWORD 		CursorSteady;	/* for accelerating pointer movement */
    UWORD 		CursorInc;

    /* --------------- public screen stuff ----------------------------	*/
    struct List		PubScreenList;	/* protected by LOCKIBASE()	*/
    struct PubScreenNode	*DefaultPubScreen;
    UWORD		PubScreenFlags; /* PubScreenFlags in screens.h	*/

    /* --------------- runtime debugging stuff ------------------------	*/
    UWORD		DebugFlag;	/* checked at run time		*/

#define IDF_SPRITE	(1)	/* sprite debug	*/
#define IDF_BPATTERN	(2)	/* border patrol cross hatch	*/
#define IDF_BPAUSE	(4)	/* border patrol pause		*/
#define IDF_PAUSE	(8)	/* general pause		*/
#define IDF_LOCKDEBUG	(16)	/* Lock debugging enabled
				 * (it's disabled during initIntuition())
				 */

    struct Point	debugpt;	/* mailbox for stuff		*/

    /* --------------- "new" preferences ------------------------------	*/
    /* CoolScreenMode is what the user wants.  We also keep track
     * of the last screenmode that worked, to give a reasonable
     * fallback if the user asks for something impossible:
     */
    struct IScreenModePrefs	CoolScreenMode;
    struct IScreenModePrefs	LastScreenMode;

    struct IFontPrefs	SysFontPrefs[ 2 ];
    ULONG		NewIControl;	/* new Intuition bool prefs */
    struct Hook		*EditHook;	/* first pass stringg editing	*/

    /*** new ISM stuff that I don't want to recompile for now ***/
    struct Requester	*ActiveRequester;

    struct Window	*VerifyWindow;	  /* window we're waiting for	*/
    struct IntuiMessage	*VerifyMessage;	  /* address of pending message	*/
    ULONG		VerifyClass;	  /* menu, dmr, size, ...	*/
    UWORD		VerifyCode;	/* MENUHOT or MENUWAITING or other */
    UWORD		VerifyTimeout;
    struct Window	*AllButWindow;	/* inactive MENUVERIFY window	*/
    UWORD		MenuAbort;

    /******* put these in better homes then remake	****/
    struct List		IClassList;
    struct Hook		IIHooks[ NUM_IHOOKS ];

    /* more IControl stuff	*/
    UWORD		MetaDragQual;
    UBYTE		WBtoFCode;
    UBYTE		FtoBCode;
    UBYTE		ReqTrueCode;
    UBYTE		ReqFalseCode;

    /* a kludgy little fake semaphore (spin lock)
     * used by openSysScreen
     */
    UWORD		SysScreenProtect;

    /* reply port for asynhc msgs to WBPort	*/
    struct MsgPort	WBReplyPort;

    /* cache for imageclass pointer	*/
    struct IClass	*ImageClassPtr;

    /* new improved workbench colors storage
     * We store a byte per gun, eleven colors only
     * (first four, last four, and three pointer colors)
     */
    UBYTE	WBColors[ WBCOLOR_NUMBER * 3 ];

    /* to catch and sync View offset changes: */
    WORD		TrackViewDx, TrackViewDy;

    /* The old MOUSESCALEX and MOUSESCALEY constants (representing
     * one NTSC-lace pixel in ticks) is now set by monitor type.
     * (MouseScaleX is a bit farther down in IBase)
     */
    WORD		MouseScaleY;

    /* Peter 7-Jan-91: By keeping the Workbench screen's TextAttr
     * in a place that doesn't go away when the screen is closed,
     * MicroFiche Filer works better.
     */
    struct TAttrBuff	MFF_TAttrBuff;

    /* Non-draggable child screens result in a screen other than
     * the HitScreen being dragged.  So we use this instead.
     */
    struct Screen	*DragScreen;

    /* --------------- Sprite Pointer --------------------------------- */
    /* the ActiveGroup pointer sprite definition	*/
    BYTE		AXOffset;
    BYTE		AYOffset;

    /** jimm:dale: 11/25/85, thought we'd take a chance for glory **/
    struct SimpleSprite	*SimpleSprite;

    WORD		MouseScaleX;

    /* NEW SPRITE HANDLING:
     *
     * These are handles to the MousePointer structures for
     * the user's preferred mouse-pointer and busy-pointer.
     * Intuition initializes these at the dawn of time,
     * and the user can change them through Preferences.
     *
     * Note that Intuition keeps allocated copies of the default
     * mouse pointer in each possible resolution.  As a result,
     * it doesn't need the BitMap any longer.  IPrefs does free
     * the xmpt_BitMap of the DefaultMousePointer right away.
     */
    struct MousePointer *DefaultMousePointer;
    struct MousePointer *DefaultBusyPointer;

    /* MousePointer structures such as those above are not
     * ready for display; their bitmaps need to be passed
     * to graphics.  This means there's a possibility of
     * failure.  In order to always be able to bring up
     * some sprites, we keep ready-to-display ExtSprites
     * version of the preferences mouse-pointer, in each
     * possible size:
     */
    struct ExtSprite *DefaultExtSprite[ POINTER_SIZES ];

    /* When we're using a custom pointer, we keep track
     * of its MousePointer structure here, because we need
     * to look at its flags and size, and so on.
     * If the default mouse-pointer is in use, this field is
     * NULL.
     */
    struct MousePointer *CustomMousePointer;

    /* When we're using a custom pointer, we keep track of
     * the ExtSprite we allocated for it, so that we can free
     * it when the time comes.
     */
    struct ExtSprite *CustomExtSprite;

    /* One of the POINTERXRESN_ flags from pointerclass.h */
    WORD PointerXRes;
    /* One of the POINTERYRESN_ flags from pointerclass.h */
    WORD PointerYRes;

    /* If menu-lending is in effect, we store the window to return to */
    struct Window *MenuLendingReturn;

    LONG PointerKilled;

    /* A blank sprite we allocate at the beginning, and use to blank
     * the pointer during transitions.
     */
    struct ExtSprite *BlankSprite;

    /* We store a pointer to the AllocVec()'d bitmap of the original
     * ROM-default busy pointer.
     */
    struct BitMap *BusyPointerBitMap;

    /* Pointer to Preferences pen-array.  One for four colors,
     * one for eight or more:
     */
    UWORD ScreenPens4[ NUMDRIPENS + 1];
    UWORD ScreenPens8[ NUMDRIPENS + 1];

    /* GadgetHelp testing occurs when the active window has gadget-help
     * mode (WMF_GADGETHELP) enabled.  To give good performance and feel,
     * we don't bother testing for GadgetHelp if the mouse is travelling
     * quickly.  Thus, we store the mouse coordinates noted at the last
     * timer tick (LastTimeX/Y).
     *
     * To avoid unnecessary work, we don't retest for GadgetHelp if
     * the mouse position is unchanged from the last time we tested
     * (LastHelpX/Y).
     *
     * We also store the gadget that last gave help, which can be
     * NULL or ~0 for the special help messages of "not over one of
     * the help windows" or "in a help window, not over a gadget".
     * Finally, we store the last code from from a gadget's GM_HELPTEST
     * method.  These two stored values allow us to only send a fresh
     * IDCMP_GADGETHELP message when the gadget or the GM_HELPTEST result
     * changes.
     */
    WORD LastTimeX;
    WORD LastTimeY;
    WORD LastHelpX;
    WORD LastHelpY;
    struct Gadget *HelpGadget;

    /* Move this somewhere better... */
    struct LongPoint 	LongMouse;

    LONG HelpGadgetCode;

    /* It's too complex to have MakeVPort() failure trickle up
     * through function call return codes, so we have this
     * field here (protected by VIEWCPR lock and IBASELOCK, incidentally)
     * to store any non-zero MakeVPort() result:
     */
    LONG ViewFailure;

    UBYTE OverrunX, OverrunY;

    /* There is a vulnerability in the name uniqueness check
     * for public screens.  To fix this, we need a list of
     * public screens that are in the process of being opened.
     * See pubscreen.c for full gory details.
     */
    struct List		PendingPubScreens;	/* protected by LOCKIBASE()	*/
};

/* ScrollFreedom */
#define OKSCROLL_LEFT	0x0001 /* AutoScroll may scroll left */
#define OKSCROLL_RIGHT	0x0002 /* AutoScroll may scroll right */
#define OKSCROLL_UP	0x0004 /* AutoScroll may scroll up */
#define OKSCROLL_DOWN	0x0008 /* AutoScroll may scroll down */

#define OKSCROLL_ALL	(OKSCROLL_LEFT|OKSCROLL_RIGHT|OKSCROLL_UP|OKSCROLL_DOWN)

/* NewIControl Flag values	*/
#define IC_COERCE_COLORS	(0x0001) /* = Preserve Colors? */
#define IC_COERCE_LACE		(0x0002) /* = Avoid Flicker? */
#define IC_STRINGG_CTRL		(0x0004) /* = Filter out CTRL chars in strgads? */
#define IC_DOMAGICMENU		(0x0008) /* = Menu snap? */
#define IC_MODEPROMOTION	(0x0010) /* = Promote? */
#endif
