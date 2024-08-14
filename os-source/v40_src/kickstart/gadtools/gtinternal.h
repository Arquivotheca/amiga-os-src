#ifndef GADTOOLS_GTINTERNAL_H
#define GADTOOLS_GTINTERNAL_H

/* gtinternal.h
 *
 * Internal structures and #defines for Gadget Toolkit.
 *
 * Copyright 1989-1992, Commodore-Amiga, Inc.
 * All Rights Reserved.
 *
 * $Id: gtinternal.h,v 39.17 93/05/06 17:03:28 vertex Exp $
 *
 */

/*------------------------------------------------------------------------*/

#ifndef INTUITION_SGHOOKS_H
#include <intuition/sghooks.h>
#endif

#ifndef LIBRARIES_GADTOOLS_H
#include <libraries/gadtools.h>
#endif

#ifndef EXEC_LIBRARIES_H
#include <exec/libraries.h>
#endif

/*------------------------------------------------------------------------*/

/* Debug macros: */

#ifdef DEBUGGING
#define DP(x)	kprintf x
#define D(x)	x
#else
#define DP(x)
#define D(x)
#endif

#ifdef MAXDEBUG
#define MP(x)	kprintf x
#define M(x)	x
#else
#define MP(x)
#define M(x)
#endif

#define POKE(value) {ULONG *x = 0;*x = value;}

VOID kprintf(STRPTR,...);


/*------------------------------------------------------------------------*/

/* Our library base */

struct GadToolsLib
{
    struct Library gtb_Library;
    UWORD gtb_Pad;
    struct Library *gtb_SysBase;
    struct Library *gtb_UtilityBase;
    struct GfxBase *gtb_GfxBase;
    struct IntuitionBase *gtb_IntuitionBase;
    struct Library *gtb_LayersBase;
/* cached match function pointers
 * ORDER MATTERS: MATCHES UTILITY LIBRARY ORDERING
 */
    void *gtb_SMult32;
    void *gtb_UMult32;
    void *gtb_SDivMod32;
    void *gtb_UDivMod32;
    Class *gtb_GTButtonIClass;
    struct Hook gtb_PaletteGHook;
};

/* For reference:
 * 	SysBase		$24(a6)
 *	UtilityBase:	$28(a6)
 *	GfxBase:	$2c(a6)
 *	UtilityBase:	$30(a6)
 */

/*------------------------------------------------------------------------*/

#define ASM    __asm
#define REG(x) register __ ## x

/*------------------------------------------------------------------------*/


/* Space-saving macros */

#define getGATagData(tag,def,list) getTagDataGA((tag)&~(GA_Dummy),def,list)

#define getGTTagData(tag,def,list) getTagDataGT((tag)&~(GT_TagBase),def,list)

#define getSTRINGTagData(tag,def,list) getTagDataSTRING((tag)&~(STRINGA_Dummy),def,list)

#define findGTTagItem(tag,list) findTagItemGT((tag)&~(GT_TagBase),list)

/*------------------------------------------------------------------------*/

/* Memory utility macros: */
#define MEMORY_FOLLOWING(ptr) ((void *)((ptr)+1))
#define MEMORY_N_FOLLOWING(ptr,n)  ((void *)( ((ULONG)ptr) + n ))

/*------------------------------------------------------------------------*/

#define GADTOOL_TYPE	0x0100	/* For Gadget->GadgetType */

/* These tags have private meanings: */
#define GT_ExtraSize	GT_Private0	/* internal use in building on generics */

#define SKETCH_KIND	10	/* OBSOLETE... */

/*------------------------------------------------------------------------*/

/* Alignment macros: */
#define GRANULARITY	4
#define ALIGNPAD(x)	((GRANULARITY-1) - ((x-1) % GRANULARITY))
#define ALIGNSIZE(x)	((x) + ALIGNPAD(x))

/*------------------------------------------------------------------------*/

#define FRAMETYPE_MASK		0x0000FFFF
#define FRAMETYPE_RECESSED	0x80000000

/*------------------------------------------------------------------------*/

/* A CycleBorder is the glyph for the cycle gadget, expressed as
 * a Border
 */

#define CB_COUNT	16	/* 16 pairs */

struct CycleBorder
{
    struct Border cb_Border;
    WORD cb_Points[CB_COUNT*2];
};

/*------------------------------------------------------------------------*/

/* Stuff for private GTButtonIClass */

/* Steal some unused imageclass tags */
#define MYIA_IText	IA_APattern

/* GTButtonIClass looks at the Border->BackPen for one of these: */
#define DESIGNSHINE	1	/* use SHINEPEN (SHADOWPEN if IDS_SELECTED) */
#define DESIGNSHADOW	2	/* use SHADOWPEN (SHINEPEN if IDS_SELECTED) */
#define DESIGNTEXT	3	/* use TEXTPEN (FILLTEXTPEN if IDS_SELECTED) */

/*------------------------------------------------------------------------*/

/* Trim (inclusive of border thickness) that should be allowed
 * around an object for the border:
 */
#define LEFTTRIM	4
#define LRTRIM		8
#define TOPTRIM		2
#define TBTRIM		4

/* Border thickness: */
#define BEVELXSIZE	2
#define BEVELYSIZE	1

/*------------------------------------------------------------------------*/

#define PLACETEXT_MASK (PLACETEXT_LEFT | PLACETEXT_RIGHT | PLACETEXT_ABOVE \
	| PLACETEXT_BELOW | PLACETEXT_IN)

/*------------------------------------------------------------------------*/

/* All gadgets allocated through CreateGadget() are actually
 * SpecialGadgets, which have a few extra fields.  These extra fields
 * are hidden from the casual user.
 */

struct SpecialGadget
{
    struct ExtGadget sg_Gadget;	/* The actual gadget */
    struct ExtGadget *sg_Parent;   /* Parent if part of composite gadget */
    BOOL (*sg_EventHandler)(struct ExtGadget *, struct IntuiMessage *);
				/* Routine to handle gadget events */
    void (*sg_Refresh)(struct ExtGadget *, struct Window *, BOOL);
				/* Routine to handle refresh */
    void (*sg_SetAttrs)(struct ExtGadget *, struct Window *,
	struct TagItem *);	/* Routine to set gadget attributes */
    struct GetTable *sg_GetTable;
    ULONG sg_Flags;		/* See below */
};

#define SG_MOUSEMOVE		1   /* This gadget cares about MOUSEMOVEs */
#define SG_CONTEXT		2   /* This is the context gadget */
#define SG_INTUITICKS	        4   /* This gadget cares about INTUITICKs */
#define SG_EXTRAFREE_DISPOSE	8   /* Call DisposeObject() on this gadget's image when freeing it */
#define SG_EXTRAFREE_CLOSEFONT  16  /* Call closeFont() on this gadget's StringInfo.Extension.Font */
#define SG_EXTRAFREE_DISPOSE_LV 32  /* Call closeFont() on a listview's font */
#define SG_MOUSEBUTTONS         64  /* This gadget cares about IDCMP_MOUSEBUTTONS */

#define SGAD(gad) ((struct SpecialGadget *)gad)

/*------------------------------------------------------------------------*/

/* Used internally as modifiable IntuiMessages */

struct QuasiMessage
{
    struct ExtIntuiMessage qm_IMessage;
    struct IntuiMessage *qm_OrigMessage;
    struct ExtGadget *qm_ContextGadget;
};

/*------------------------------------------------------------------------*/

/* Context gadget: */

struct XContext
{
    struct SpecialGadget xct_Gadget;
    /* Instance data: */
    struct ExtGadget *ctid_ActiveGadget;
    struct QuasiMessage ctid_QuasiMessage;
    BYTE ctid_QuasiUsed;
    BYTE ctid_DelayedFree;
};

#define CTID(g)	((struct XContext *)(g))

#define CONTEXT_IDATA_SIZE (sizeof(struct XContext)-sizeof(struct SpecialGadget))


/*------------------------------------------------------------------------*/

/* Instance data for checkbox gadget: */

struct XCheckbox
{
    struct SpecialGadget cbid_Gadget;
    /* Instance data: */
    WORD cbid_Checked;
};

#define CHECKBOX_IDATA_SIZE	(sizeof(struct XCheckbox)-sizeof(struct SpecialGadget))

#define CBID(g)	((struct XCheckbox *)(g))

/*------------------------------------------------------------------------*/

/* ListView gadgets: */

/* The instance data for the ListView gadget: */

struct XListView
{
    struct SpecialGadget xlv_Gadget;

    /* instance data */
    struct List      *lvid_Labels;
    WORD              lvid_Top;
    WORD              lvid_OldTop;
    WORD              lvid_Total;
    WORD	      lvid_Visible;
    UWORD	      lvid_ItemHeight;
    UWORD             lvid_Selected;
    STRPTR            lvid_SelectedName;

    struct ExtGadget *lvid_ListGad;
    struct ExtGadget *lvid_Scroller;
    struct ExtGadget *lvid_DisplayGad;

    struct Hook      *lvid_CallBack;
    struct Hook       lvid_DefaultCallBack;

    struct DrawInfo  *lvid_DrawInfo;
    struct TextFont  *lvid_Font;

    BOOL              lvid_ReadOnly;
    BOOL              lvid_AllowSelections;
    UWORD             lvid_MaxPen;
    UWORD             lvid_TickCount;
};

#define LVID(g)	((struct XListView *)(g))

#define LISTVIEW_IDATA_SIZE (sizeof(struct XListView)-sizeof(struct SpecialGadget))

/*------------------------------------------------------------------------*/

/* Mutually exclusive gadgets: */

/* The dummy gadget's instance data: */

struct XMX
{
    struct SpecialGadget xmx_Gadget;
    /* Instance data: */
    struct ExtGadget *mxid_ActiveGadget;	/* The active one */
    struct ExtGadget *mxid_FirstGadget;	/* The first one */
    WORD mxid_NumGadgets;               /* Total number of gadgets */
    WORD mxid_Active;			/* Number of the active gadget */
};

#define MXID(g)	((struct XMX *)(g))

#define MX_IDATA_SIZE	(sizeof(struct XMX)-sizeof(struct SpecialGadget))

/*------------------------------------------------------------------------*/

/* Instance data for simple number display: */
/* Shared with Text display routines, so it must match XText
 * field-for-field (with extensions allowed)
 */

struct XNumber
{
    struct SpecialGadget nmid_Gadget;	     /* Must match XText */

    /* instance data */
    struct IntuiText     nmid_IText;	     /* Must match XText */
    struct Rectangle     nmid_Extent;	     /* Must match XText */
    UWORD                nmid_Justification; /* Must match XText */
    UWORD                nmid_TextWidth;
    STRPTR               nmid_Format;
    LONG                 nmid_Number;
};

#define NMID(g)	((struct XNumber *)(g))

#define NUMBER_IDATA_SIZE	(sizeof(struct XNumber)-sizeof(struct SpecialGadget))


/* Instance data for simple text display: */

struct XText
{
    struct SpecialGadget txid_Gadget;

    /* instance data */
    struct IntuiText     txid_IText;
    struct Rectangle     txid_Extent;
    UWORD                txid_Justification;
    UWORD                txid_TextWidth;
};

#define TXID(g)	((struct XText *)(g))

#define TEXT_IDATA_SIZE	(sizeof(struct XText)-sizeof(struct SpecialGadget))

/*------------------------------------------------------------------------*/

/* Cycle gadgets: */

/* The instance data for the Cycle gadget: */

struct XCycle
{
    struct SpecialGadget cyid_Gadget;
    /* Instance data: */
    STRPTR *cyid_Labels;
    UWORD cyid_MaxLabel;
    WORD cyid_Active;
    struct IntuiText cyid_IText;
    struct CycleBorder cyid_CycleBorder;
    struct Border cyid_DarkStroke;
    struct Border cyid_LightStroke;
    WORD cyid_StrokePoints[4];
};

#define CYID(g)	((struct XCycle *)(g))

#define CYCLE_IDATA_SIZE	(sizeof(struct XCycle)-sizeof(struct SpecialGadget))

#define CYCLEGLYPHWIDTH	20

/*------------------------------------------------------------------------*/

/* Instance data for palette gadget: */

struct XPalette
{
    struct SpecialGadget xp_Gadget;
    /* Instance data follows: */
    UWORD paid_XCount;		/* Number of colors per row */
    UWORD paid_EachWidth;	/* Width of each color rectangle */
    UWORD paid_EachHeight;	/* Height of each color rectangle */
    UWORD paid_Count;		/* Number of color rectangles */
    UWORD paid_SelectedBox;	/* Active color box */
    UWORD paid_Color;            /* # of currently selected color */
    UWORD paid_ColorOffset;	/* 1st color to use in palette */
    UBYTE *paid_ColorTable;     /* Table of pens to use */
    struct DrawInfo *paid_DrawInfo; /* Cached pointer */
    UWORD paid_BoxBackup;     /* To support undoing the selection with the RMB */
    BOOL paid_Indicator;
};

#define PALETTE_IDATA_SIZE	(sizeof(struct XPalette)-sizeof(struct SpecialGadget))

#define PAID(g)	((struct XPalette *)(g))


/*------------------------------------------------------------------------*/

/* Instance data for scroller gadget: */

struct XScroller
{
    struct SpecialGadget scid_Gadget;
    /* Instance data: */
    WORD scid_Top;			/* First one you want displayed */
    WORD scid_Total;			/* total elements in list */
    WORD scid_Visible;			/* Number visible at one time */
    UWORD scid_TickCount;
    ULONG scid_Flags;			/* See below */

    struct ExtGadget *scid_ListView;	/* PRIVATE kludge pointer back
					 * to ListView if I'm in one
					 */
    struct ExtGadget *scid_Prop;		/* Pointer to prop gadget */
    struct ExtGadget *scid_Up;		/* Pointer to up arrow gadget */
    struct ExtGadget *scid_Down;		/* Pointer to down arrow gadget */
    UWORD *scid_Body, *scid_Pot;

    struct PropInfo scid_PropInfo;
    struct Image    scid_PropImage;
};

#define SCROLLER_IDATA_SIZE	(sizeof(struct XScroller)-sizeof(struct SpecialGadget))

#define SCID(g)	((struct XScroller *)(g))

/* Scroller flags include some common ones with sliders,
 * plus the scroller-specific SC_ARROWS.
 */

#define GTPROP_VERTICAL		0x00000001 /* Vert. prop */
#define GTPROP_GADGETDOWN	0x00000002 /* Guaranteed msg upon GADGETDOWN */
#define GTPROP_GADGETUP		0x00000004 /* Guaranteed msg upon GADGETUP */
#define SC_ARROWS		0x00000008 /* Has arrows */

/*------------------------------------------------------------------------*/

/* Instance data for slider gadget: */

struct XSlider
{
    struct SpecialGadget slid_Gadget;
    /* Instance data: */
    WORD slid_Min;		/* Minimum level */
    WORD slid_Max;		/* Maximum level */
    WORD slid_Level;		/* Current level */
    ULONG slid_Flags;		/* See scroller flags */
    LONG (*slid_DispFunc)();	/* Callback for number recalc. */

    UWORD *slid_Body;		/* Pointer to Vert or HorizBody */
    UWORD *slid_Pot;		/* Pointer to Vert or HorizPot */
    struct IntuiText *slid_IText;	/* IntuiText to render label/level */
    UBYTE *slid_LevelFormat;	/* Format String for level */
    struct ExtGadget *slid_Prop;

    struct PropInfo slid_PropInfo;
    struct Image    slid_PropImage;

    UWORD           slid_MaxPixelLen;   /* maximum room used up by level display */
    UBYTE           slid_Justification;

    struct Rectangle slid_Extent;   /* space occupied by slider level display */
};

#define SLIDER_IDATA_SIZE	(sizeof(struct XSlider)-sizeof(struct SpecialGadget))

#define SLID(g)	((struct XSlider *)(g))

/* Slider flags are the three GTPROP_ flags defined under scrollers. */

/*------------------------------------------------------------------------*/

/* Instance data for a string gadget: */

struct XString
{
    struct SpecialGadget stid_Gadget;
    /* Instance data: */
    struct StringInfo stid_StringInfo;
    struct StringExtend stid_Sex;
};

#define STID(g)	((struct XString *)(g))

#define STRING_IDATA_SIZE	(sizeof(struct XString)-sizeof(struct SpecialGadget))

/*------------------------------------------------------------------------*/


struct ImageLink
{
    struct ImageLink *il_Next;
    struct Image     *il_Image;
    ULONG             il_Type;  /* CHECKIMAGE or MXIMAGE */
};


/*------------------------------------------------------------------------*/

/* All the visual information we need can be found in this private
 * (hence extensible) structure:
 */

struct VisualInfo
{
    struct Screen    *vi_Screen;		/* The screen in question */
    struct TextFont  *vi_ScreenFont;	/* opened copy of screen font */
    struct DrawInfo  *vi_DrawInfo;	/* DrawInfo structure */
    struct ImageLink *vi_Images;	/* Images */
    UBYTE             vi_textPen;	/* Shorthand */
    UBYTE             vi_backgroundPen;	/* Shorthand */
};

#define VI(vi)	((struct VisualInfo *)vi)

/*------------------------------------------------------------------------*/

/* For scroller arrows: */
struct ArrowBorder
{
    struct Border Border1;
    struct Border Border2;
    WORD Points[8];
};

/* Arrow directions: */
#define ARROW_UP	0
#define ARROW_DOWN	1
#define ARROW_LEFT	2
#define ARROW_RIGHT	3

/* Number of INTUITICKS to ignore before first repeat: */
#define ARROW_SKIPTICKS	4

/*------------------------------------------------------------------------*/

#define NM_TRUETYPE(type)	((type) & ~MENU_IMAGE)

/* This structure gives us the easiest way to extract the original
 * Left value for an Custom Image menu item:
 */
struct ImageMenuItem
{
    struct MenuItem imi_MenuItem;
    void *imi_UserData;
    struct Image imi_Image;
    WORD imi_OrigLeft;
};

/*------------------------------------------------------------------------*/

/* These macros are used to generate the GT_GetGadgetAttrs tables.
 *
 * Each gadget kind has a getattr table interpreted as follows:
 *
 * Each entry is a byte-pair:
 *
 * UBYTE 1 = describes the attribute tag or is a terminator
 *	Attributes are expressed as ( attribute tag - GT_TagBase )
 *	With the special values of ~0 and ~1 as terminator.  ~1 not only
 *	terminates, but requests that GA_Disabled be recognized.
 *
 * UBYTE 2 = describes the attribute offset of the attribute, partitioned into
 *	( size:1 | offset: 7 )
 * 	Where "offset" is the offset in WORDs from the end of the Gadget
 *	structure to the attribute in the instance data.  The "size" bit
 *	is zero for a WORD-sized attribute, and one for a LONG.
 *
 * We could steal one offset bit to distinguish signed vs. non.  For now,
 * we take advantage of the fact that all attributes are either signed words
 * or long (sign extension is not an issue with longs).
 *
 * ATTR_OFFSET returns the offset of a particular attribute in the
 * instance data.  The result is the number of words past the
 * SpecialGadget structure where the attribute can be found.
 *
 * GETTABLE_ATTR generates a table entry based on a tag, a structure-cast,
 * a member name, and one of the attribute types.  If the attribute-offset
 * is out of range, illegal code will be generated, so you can notice
 * at compile time that something's wrong.
 */

#define WORD_ATTR	0x00
#define LONG_ATTR	0x80

#define MAX_OFFSET	0x7F

#define ATTR_OFFSET( cast, member ) ((&cast(0)->member - sizeof(struct SpecialGadget))/sizeof(WORD))

void CauseCompilerError(void);

/* These special attribute codes are the highest possible bytes.  An
 * attribute table always ends with one of these.
 */
#define ATTR_END	255
#define ATTR_DISABLED	254

/* This macro generates a table signifying the end of the table.
 */
#define GETTABLE_END		{ATTR_END,0}


/* This macro generates a table entry for the supplied GadTools
 * "attr"ibute.  The table entry says to look at the "member" offset
 * into the "cast" structure, for a variable of the specified "type".
 */
#define GETTABLE_ATTR( attr, cast, member, type ) \
	{ ATTR_OFFSET(cast,member) > MAX_OFFSET ? CauseCompilerError() : \
	(attr) - GT_TagBase, (type)|ATTR_OFFSET(cast,member) }

/* These macros define table entries to look for the GA_Disabled
 * attribute.  Either of these macros (as well as GETTABLE_END)
 * also define the end of the attribute table.
 *
 * If the actual gadget the caller receives can be tested for
 * the GFLG_DISABLED property, then use GETTABLE_DISABLED.
 * Certain gadget kinds have a dummy gadget, and a member gadget
 * (eg. SLID(gad)->slid_Prop for the slider) needs to be tested
 * instead.  For that type, use GETTABLE_DISABLED_MEMBER(SLID,slid_Prop).
 */

#define GETTABLE_DISABLED	{ATTR_DISABLED,0}
#define GETTABLE_DISABLED_MEMBER( cast, member ) \
	GETTABLE_ATTR( GT_TagBase+ATTR_DISABLED, cast, member, LONG_ATTR )

#define REAL_OFFSET(descriptor) (((descriptor)&MAX_OFFSET)*2+sizeof(struct SpecialGadget))

/* The following are the non-standard attributes (i.e. those not simply
 * stuck in the instance data:
 *
 * GA_Disabled - OK, since handled separately anyways
 *
 * GTIN_Number - If we embed StringInfo, then OK as inid_StringInfo.LongInt.
 *
 * GTST_String - If we embed StringInfo, then OK as stid_StringInfo.Buffer.
 *
 * GTTX_Text - OK, as txid_IText.IText
 *
 * GTMX_Active - mxid_ActiveGadget->UserData
 */

struct GetTable
{
    UBYTE gtab_tag;
    UBYTE gtab_descriptor;
};

/*------------------------------------------------------------------------*/


#define IGNORE_VISUALINFO (VOID *)~0

/*------------------------------------------------------------------------*/


#endif /* !GADTOOLS_GTINTERNAL_H */
