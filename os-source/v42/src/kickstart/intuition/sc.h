
/*** sc.h ****************************************************************
 *
 *  internal version of screens.h
 *
 *  $Id: sc.h,v 40.0 94/02/15 17:29:31 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga Computer, Inc.
 *  Copyright (c) Commodore-Amiga Computer, Inc.
 *
 ****************************************************************************
 * CONFIDENTIAL and PROPRIETARY
 * Copyright (C) 1985, COMMODORE-AMIGA, INC.
 * All Rights Reserved
 ****************************************************************************/

#include "screens.h"

/** Screen structure with private extensions **/

/* macro to extend (cast) a screen pointer to XScreen */
#define XSC(s)	( (struct XScreen *) (s) )

/* The SpriteFactor.X value is exaggerated by four because
 * AA supports sprites whose horizontal resolution can be 1/4, 1/2,
 * double, quadruple, or the same as the screen's resolution.
 *
 * The SpriteFactor.Y value is exaggerated by two because AA supports
 * sprites whose vertical resolution can be half or double the
 * screen's resolution.
 */
#define SPRITEMULT_X	4
#define SPRITEMULT_Y	2

struct XScreen	{
    struct Screen	xs_Screen;

    /**************************************************/
    /**** Data below this point are SYSTEM PRIVATE ****/
    /**************************************************/

    struct ViewPortExtra *VPExtra;
			/* in the graphics associative cache	*/
			/* DisplayClip is in screen resolution coords */

    struct Rectangle	DClip;	
			/* in native screen coords		*/
    
    struct  PubScreenNode	*PSNode;

    /* what the screen is supposed to be (uncoerced)		*/
    struct MonitorSpec		*NaturalMSpec;
    CPTR			NaturalDRecord;
    ULONG			NaturalDisplayID;

    struct { WORD X; WORD Y;}	ScaleFactor;	/* for -to-mouse xform	*/
    ULONG			DProperties;	/* always current	*/

    /* We remember the screen's PixelSpeed and the screen's default
     * SpriteResolution.X when we figure the coerced mode
     */
    struct { WORD X; WORD Y;}	SpriteRes;

    UWORD			PixelSpeed;
    UWORD			VPModeStash;	/* secret copy to compare */

    struct TAttrBuff		TAttrBuff;	/* for snapshot		*/
    struct ColorMap		*ColorMap;	/* stash		*/
    /* SpriteFactor.X is now exaggerated by a factor of four, so we
     * can scale down too.  SpriteFactor.Y is exaggerated by a factor
     * of two, for the same reason.
     */
    struct { WORD X; WORD Y;}	SpriteFactor;	/* pixels-per-sprite pxl*/

    /* ZZZ: GADGETCOUNT from ibase.h */
    struct Image		*SysImages[ 8 ];
	
    struct DrawInfo		SDrawInfo;	/* embedded struct 	*/
    ULONG			PrivateFlags;
    ULONG			NaturalDProperties;
    struct { WORD X; WORD Y;}		VPOffsetStash;
    UWORD			SPens[ NUMDRIPENS + 1 ];
					/* terminated pen array	*/
    struct Layer_Info		*ClipLayer_Info;
    struct Layer		*ClipLayer;

    /* These fields are used if this is a parent screen.
     * Family is an exec list containing nodes as follows:
     * - For a parent screen, the list contains nodes for each child
     *   and the parent itself (in depth order).
     * - For a non-attached screen, the list contains the node for the
     *   screen itself.
     * - For a child-screen, the list is empty (the screen's own node
     *   will be found on its parent's Family list).
     */
    struct MinList		Family;		/* List of attached screens incl. self */

    /* These fields are used if this is a child screen: */
    struct MinNode		ChildNode;	/* Node on parent's list */
    struct Screen		*ParentScreen;	/* This screen's parent */
    /* Offset of child->TopEdge with respect to parentOriginY() */
    WORD			ParentYOffset;

    /* Replaces SaveColor0: 32-bit color values for screen beeping.
     * These must be together, in red/green/blue order:
     */
    ULONG			SaveRed;
    ULONG			SaveGreen;
    ULONG			SaveBlue;

    /* For custom screens, sc_BitMap was always a copy of some real bitmap.
     * Now, when Intuition allocates the bitmap (non-CUSTOMBITMAP screens)
     * it allocates a whole BitMap (not just the planes).  We keep a pointer
     * to the real bitmap here.  In such cases, RealBitMap is subject to
     * change due to double-buffering.  For CUSTOMBITMAP screens, RealBitMap
     * will be NULL.
     */
    struct BitMap		*RealBitMap;

    struct { WORD X; WORD Y;}	InitialScaleFactor;

    WORD LastColor;

    /* Offset of child->LeftEdge with respect to parentOriginX() */
    WORD ParentXOffset;

    /* The BitMap copy is used to ascertain when the screen's embedded
     * bitmap has been poked.  In V39, we used to compare the embedded
     * copy to the RealBitMap, but we found that DirectorII pokes
     * the RealBitMap (via screen.RastPort->BitMap).  When we found
     * this, we did the wrong thing, namely switching to use the
     * embedded BitMap.  Now we compare against this guaranteed-correct
     * copy.
     */
    struct BitMap sc_BitMapCopy;
};

/* Private screen flags:
 * PSF_SCREENFONT - means that one of the new SA_SysFont specifiers was
 *	used, so I'm to use the vanilla font for windows
 *	(i.e. GfxBase->DefaultFont), instead of the screen font.
 * PSF_DELAYBEEP - Set by DisplayBeep(), cleared the first time through
 *	doTiming().  This is to ensure that the beep lasts at least one
 *	vblank.
 * PSF_DRAGGABLE - Marks a draggable screen (all except those having
 *	{SA_Draggable,FALSE}).
 * PSF_EXCLUSIVE - Marks a screen made exclusive via {SA_Exclusive,TRUE}.
 * PSF_INCOMPATIBLE - Marks a screen which is exclusive by virtue of
 *	being MCOMPAT_NOBODY, according to the graphics database.
 * PSF_CHILDFRONT - When an existing child is attached to a newly opening
 *	parent, we set this flag if the attachment was done with
 *	SA_FrontChild, and clear this flag if the attachment was done
 *	with SA_BackChild.  We test this flag later to act on the request.
 * PSF_REMAKEVPORT - Set by MakeScreen() to inform rethinkVPorts() that
 *	this screen needs its copper-lists remade.
 * PSF_NEWSCREENOFFSETS - Set by coerceScreenGrunt() to indicate that the
 *	screen Left/Top should be respected over the ViewPort Dx/DyOffset
 *	because a coercion change moved the Left/Top.  (Ordinarily, unless
 *	a MEGAFORCE remake happens, we respect the ViewPort coordinates
 *	over the screen coordinates in order to support old overscan
 *	techniques.
 * PSF_SETFONT - Set by OpenScreen() after it successfully SetFont()s into
 *	the screen's RastPort.  This informs closeScreenCommon() that it's
 *	correct to CloseFont() that font.  Without this, certain OpenScreen()
 *	failures mean that cSC() would incorrectly CloseFont() the
 *	GfxBase->DefaultFont, which is what InitRastPort() placed in
 *	the screen RastPort.Font field
 * PSF_MINIMIZEISG - Corresponds to the state of SA_MinimizeISG, which
 *	is a V40 tag requesting that CalcISG() no longer ensure that
 *	the ISG is at least six laced lines.  Useful to get best appearance
 *	of your screen.
 */
#define PSF_SCREENFONT		(0x00000001)
#define PSF_DELAYBEEP		(0x00000002)
#define PSF_DRAGGABLE		(0x00000004)
#define PSF_EXCLUSIVE		(0x00000008)
#define PSF_INCOMPATIBLE	(0x00000010)
#define PSF_CHILDFRONT		(0x00000020)


#define PSF_NEWSCREENOFFSETS	(0x00000040)
#define PSF_REMAKEVPORT		(0x00000080)
#define PSF_SETFONT		(0x00000100)
#define PSF_MINIMIZEISG		(0x00000200)

/* Private Screen->Flags value meaning that this screen is hidden on account
 * of having a mode incompatible with the frontmost screen.
 */
#define SCREENHIDDEN	0x2000

/* Special private value to SA_LikeWorkbench tag used by the Workbench
 * screen only.
 */
#define LIKEWB_WORKBENCH	(~0)

/* Given a pointer to a ChildNode, return a pointer to the screen. */
#define SCREENFROMNODE(n)	((struct Screen *)( ((ULONG)n) - (ULONG)(&(((struct XScreen *)0)->ChildNode))))

/* cast to ExtNewScreen for use with Extension field 	*/
#define XNS(ns)	( (struct ExtNewScreen *) (ns) )

struct XScreenBuffer
{
	struct ScreenBuffer xsb_ScreenBuffer;	/* Instance of ScreenBuffer */
	ULONG sb_Flags;				/* Private flags, see below */
};

#define SB_FREEBITMAP	16	/* Marks bitmaps I alloc, so I know to free */

/* Macro to get at the private fields */
#define XSB(sb)	((struct XScreenBuffer *)(sb))
