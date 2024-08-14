#ifndef GADTOOLS_GTINTERNAL_H
#define GADTOOLS_GTINTERNAL_H

/*  gtinternal.h
**
*   Internal structures and #defines for Gadget Toolkit.
*
*   Copyright 1989, Commodore-Amiga, Inc.
*   All Rights Reserved.
*
*   $Id: gtinternal.h,v 36.6 90/05/30 13:51:03 peter Exp $
*
*   $Log:	gtinternal.h,v $
*   Revision 36.6  90/05/30  13:51:03  peter
*   Defined GTST_EditHook to be GT_Reserved0.
*   
*   Revision 36.5  90/05/15  14:16:29  peter
*   Added Extent rectangle to instance data for text and number display gadgets.
*   
*   Revision 36.4  90/05/03  16:30:46  peter
*   Added NM_TRUETYPE() macro for custom imagery in menus.
*   
*   Revision 36.3  90/04/02  16:07:04  peter
*   Added #define GADTOOL_TYPE.
*   
*   Revision 36.2  90/03/31  06:38:51  peter
*   Added NWAYGLYPHWIDTH define.
*   
*   Revision 36.1  90/03/16  14:06:04  peter
*   RCS Clean up.
*   
*
*/

/*------------------------------------------------------------------------*/

#ifndef LIBRARIES_GADTOOLS_H
#include <libraries/gadtools.h>
#endif

/*------------------------------------------------------------------------*/

/*  Debug macros: */

#ifdef DEBUGGING
#define DP(x)	kprintf x
#define D(x)	x
void kprintf(char *, ...);
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

/*------------------------------------------------------------------------*/

/*  Memory utility macros: */
#define MEMORY_FOLLOWING(ptr) ((void *)((ptr)+1))
#define MEMORY_N_FOLLOWING(ptr,n)  ((void *)( ((ULONG)ptr) + n ))

/*------------------------------------------------------------------------*/

#define GADTOOL_TYPE	0x0100	/*  For Gadget->GadgetType */

/*  These tags have private meanings: */
#define GT_ExtraSize	GT_Private0	/* internal use in building on generics */
#define GTST_EditHook	GT_Reserved0	/* Private use by ASL */

#define GT_AUTOGADGET	0xFFFF	/*  Reserved GadgetID for gadgets generated
				    for you (i.e. implicitly) Not yet used!!!*/

/*------------------------------------------------------------------------*/

/*  Alignment macros: */
#define GRANULARITY	4
#define ALIGNPAD(x)	((GRANULARITY-1) - ((x-1) % GRANULARITY))
#define ALIGNSIZE(x)	((x) + ALIGNPAD(x))

/*------------------------------------------------------------------------*/

/*  A BevelBox is the pretty bevelled box ever-present in the Toolkit: */

#define BB_TRIMCOUNT	5	/*  5 pairs */
#define BB_SHADOWCOUNT	5	/*  5 pairs */

struct BevelBox
    {
    struct Border bb_TrimBorder;
    WORD bb_TrimPoints[BB_TRIMCOUNT*2];
    struct Border bb_ShadowBorder;
    WORD bb_ShadowPoints[BB_SHADOWCOUNT*2];
    };

/*  BevelBox can be one of two kinds: */

#define BB_RAISED	(TRUE)
#define BB_RECESSED	(FALSE)

/*------------------------------------------------------------------------*/

/*  Trim (inclusive of border thickness) that should be allowed
    around an object for the border: */
#define LEFTTRIM	4
#define LRTRIM		8
#define TOPTRIM		2
#define TBTRIM		4

/*  Border thickness: */
#define BEVELXSIZE	2
#define BEVELYSIZE	1

/*------------------------------------------------------------------------*/

#define PLACETEXT_MASK (PLACETEXT_LEFT | PLACETEXT_RIGHT | PLACETEXT_ABOVE \
	| PLACETEXT_BELOW | PLACETEXT_IN)

/*------------------------------------------------------------------------*/

/*  All gadgets allocated through CreateGadget() are actually
    SpecialGadgets, which have a few extra fields.  These extra fields
    are hidden from the casual user by virtue of being ahead of the
    actual gadget structure, to where the result of CreateGadget()
    points.  Any of those extra fields may safely be NULL: */

struct SpecialGadget
    {
    struct MinNode sg_Node;	/*  Parent keeps a MinList of kids */
    struct Gadget *sg_Parent;   /*  Parent if part of composite gadget */
    BOOL (*sg_EventHandler)(struct Gadget *, struct IntuiMessage *);
				/*  Routine to handle gadget events */
    void (*sg_Refresh)(struct Window *, struct Gadget *, BOOL);
				/*  Routine to handle refresh */
    void (*sg_ExtraFree)(struct Gadget *);
				/*  Routine to do any extra freeing stuff */
    void (*sg_SetAttrs)(struct Gadget *, struct Window *, struct Requester *,
	struct TagItem *);	/*  Routine to set gadget attributes */
    ULONG sg_Flags;		/*  See below */
    struct Gadget sg_Gadget;	/*  The actual gadget */
    };

#define SG_MOUSEMOVE	1	/*  This gadget cares about MOUSEMOVEs */
#define SG_CONTEXT	2	/*  This is the context gadget */
#define SG_INTUITICKS	4	/*  This gadget cares about INTUITICKs */

#define SG_EXTRASIZE (sizeof(struct SpecialGadget) - sizeof(struct Gadget))

/*  I can use this to go from a Gadget to a SpecialGadget.
    Of course, the reverse is simply (&sg->sg_Gadget) */
#define SG_FROM_GAD(gad) ((struct SpecialGadget *) ( ((ULONG)gad) - SG_EXTRASIZE ))

/*------------------------------------------------------------------------*/

/*  Checkbox gadgets: */

/*  The gadget's instance data: */

struct CheckBoxIData
    {
    struct Image *cbid_CheckImage;
    };


#define CBID(sg)	((struct CheckBoxIData *)MEMORY_FOLLOWING(sg))

/*  Dimesions of checkbox: */
#define CHECKBOXWIDTH	26
#define CHECKBOXHEIGHT	11


/*------------------------------------------------------------------------*/

/*  Context gadget: */

struct ContextIData
    {
    struct Gadget *ctid_ActiveGadget;
    };

#define CTID(sg)	((struct ContextIData *)MEMORY_FOLLOWING(sg))

/*------------------------------------------------------------------------*/

/*  ListView gadgets: */

/*  The instance data for the ListView gadget: */

struct ListViewIData
    {
    struct List *lvid_Labels;
    WORD lvid_Top;
    WORD lvid_Total;
    ULONG lvid_Flags;			/*  See below */
    struct Node *lvid_TopLabel;		/*  Pointer to top label displayed */
    struct Gadget *lvid_TopGadget;	/*  Pointer to top Gadget */
    WORD lvid_Width;			/*  Of frame, less Scroller */
/*  lvid_Height is always derived as EachHeight*Count + TBTRIM */
    WORD lvid_InterSpace;
    WORD lvid_EachHeight;    		/*  Resulting height of each line */
    WORD lvid_Count;			/*  Resulting number of lines */
    struct TextAttr *lvid_TextAttr;	/*  Original TextAttr structure */
    struct Gadget *lvid_Scroller;	/*  Scroller gadget */
    struct Gadget *lvid_DisplayGad;	/*  String or text gadget for display
					    of current selection */
    WORD lvid_Selected;			/*  Ordinal number of the selected
					    one */
    struct VisualInfo *lvid_VisualInfo;
    };

/*  lvid_Flags: */
#define LV_READONLY	0x00000001	/*  Read-only list */
#define LV_STRINGDISP	0x00000002	/*  display gadget is a string gad */
#define LV_DEFERREFRESH	0x00000004	/*  defer refreshing since list is
					    detached for client use */

#define LVID(sg)	((struct ListViewIData *)MEMORY_FOLLOWING(sg))

/*  Instance data for each line gadget of the listview: */

struct ListLineIData
    {
    WORD llid_MaxX;			/*  Highest pixel rendered into */
    };

#define LLID(sg)	((struct ListLineIData *)MEMORY_FOLLOWING(sg))

/*------------------------------------------------------------------------*/

/*  Mutually exclusive gadgets: */

/*  The dummy gadget's instance data: */

struct MXIData
    {
    /*  I don't believe I need to keep labels in the IData!!! */
    STRPTR *mxid_Labels;
    struct SpecialGadget *mxid_ActiveSG;
    struct MinList mxid_List;
    struct Image *mxid_Image;
    ULONG mxid_Flags;
    UWORD mxid_Active;
    };

#define MXID(sg)	((struct MXIData *)MEMORY_FOLLOWING(sg))

/*  MX Gadget width and height are dependent on the supplied imagery: */
#define MXGAD_WIDTH	17
#define MXGAD_HEIGHT	9

/*------------------------------------------------------------------------*/

/*  Instance data for simple number display: */
/*  Shared with Text display routines, so it must match TextIData
    field-for-field (with extensions allowed) */
struct NumberIData
    {
    struct VisualInfo *nmid_VisualInfo;
    struct IntuiText nmid_IText;
    struct Rectangle nmid_Extent;

    UBYTE nmid_Buffer[20];
    };

#define NMID(sg)	((struct NumberIData *)MEMORY_FOLLOWING(sg))

/*------------------------------------------------------------------------*/

/*  NWay gadgets: */

/*  The instance data for the NWay gadget: */

struct NWayIData
    {
    STRPTR *nwid_Labels;
    UWORD nwid_MaxLabel;
    UWORD nwid_Active;
    struct IntuiText nwid_IText;
    struct BitMap nwid_GlyphBM;
    struct VisualInfo *nwid_VisualInfo;
    };

#define NWID(sg)	((struct NWayIData *)MEMORY_FOLLOWING(sg))

#define NWAYGLYPHWIDTH	20

/*------------------------------------------------------------------------*/

/*  Instance data for palette gadget: */

struct PaletteIData
    {
    UBYTE paid_Color;		/*  Active color */
    UBYTE paid_ColorOffset;	/*  1st color to use in palette */
    WORD paid_IndicLeft;	/*  Dimensions of indicator area */
    WORD paid_IndicTop;
    WORD paid_IndicWidth;
    WORD paid_IndicHeight;
    WORD paid_SelectLeft;	/*  Dimensions of select area */
    WORD paid_SelectTop;
    WORD paid_SelectWidth;
    WORD paid_SelectHeight;
    struct VisualInfo *paid_VisualInfo;
    struct Gadget *paid_FirstGadget;
    UWORD paid_Count;
    };

#define PAID(sg)	((struct PaletteIData *)MEMORY_FOLLOWING(sg))

/*------------------------------------------------------------------------*/

/*  Instance data for scroller gadget: */

struct ScrollerIData
    {
    WORD sc_Top;			/*  First one you want displayed */
    WORD sc_Total;			/*  total elements in list */
    WORD sc_Visible;			/*  Number visible at one time */
    UWORD sc_TickCount;
    ULONG sc_Flags;			/*  See gadtools.h */

    struct Gadget *sc_ListView;		/*  PRIVATE kludge pointer back
					    to ListView if I'm in one */
    struct Gadget *sc_Prop;		/*  Pointer to prop gadget */
    struct Gadget *sc_Up;		/*  Pointer to up arrow gadget */
    struct Gadget *sc_Down;		/*  Pointer to down arrow gadget */
    UWORD *sc_Body, *sc_Pot;
    struct VisualInfo *sc_VisualInfo;
    };

#define SCID(sg)	((struct ScrollerIData *)MEMORY_FOLLOWING(sg))

/*  Scroller flags: */
#define SC_VERTICAL	0x00000020	/*  Vert. scroller */
#define SC_GADGETDOWN	0x00000040	/*  Guaranteed msg upon GADGETDOWN */
#define SC_GADGETUP	0x00000080	/*  Guaranteed msg upon GADGETUP */
#define SC_ARROWS	0x00000100	/*  Has arrows */

/*------------------------------------------------------------------------*/

/*  Instance data for sketch gadget: */

struct SketchIData
    {
    struct RastPort *skid_RastPort;
    UWORD skid_bmWidth;		/*  Width of BitMap */
    UWORD skid_bmHeight;	/*  Height of BitMap */
    UWORD skid_MagWidth;	/*  Width of each magnified pixel */
    UWORD skid_MagHeight;	/*  Height of each magnified pixel */
    UWORD skid_FirstColor;	/*  First color register to use in sketch */
    UWORD skid_Color;		/*  Current drawing color */
    UWORD skid_LastX;		/*  x-coord of last pixel drawn */
    UWORD skid_LastY;		/*  y-coord of last pixel drawn */
    UWORD skid_LastColor;	/*  color of last pixel drawn */
    ULONG skid_Flags;		/*  See below */
    UWORD skid_HotSpotX;	/*  X-coordinate of HotSpot */
    UWORD skid_HotSpotY;	/*  Y-coordinate of HotSpot */
    UWORD skid_HSColor;		/*  Color to use for HotSpot */
    UWORD skid_HSXThick;	/*  Hot spot outline x-thickness */
    UWORD skid_HSYThick;	/*  Hot spot outline y-thickness */
    };

#define SKID(sg)	((struct SketchIData *)MEMORY_FOLLOWING(sg))

#define SK_HOTSPOT	0x00000001	/* Sketch has hot-spot */
#define SK_SETHOTSPOT	0x00000002	/* Waiting for user to click the
					   HotSpot */
#define SK_POSTSETHS	0x00000004	/* Waiting for user to release the
					   mouse after setting hotspot */

/*------------------------------------------------------------------------*/

/*  Instance data for slider gadget: */

struct SliderIData
    {
    WORD sl_Min;		/*  Minimum level */
    WORD sl_Max;		/*  Maximum level */
    WORD sl_Level;		/*  Current level */
    ULONG sl_Flags;		/*  See gadtools.h */
    LONG (*sl_DispFunc)();	/*  Callback for number recalc. */

    UWORD *sl_Body;		/*  Pointer to Vert or HorizBody */
    UWORD *sl_Pot;		/*  Pointer to Vert or HorizPot */
    struct IntuiText *sl_IText;	/*  IntuiText to render label/level */
    UBYTE *sl_LevelFormat;	/*  Format String for level */
    struct VisualInfo *sl_VisualInfo;
    };

#define SLID(sg)	((struct SliderIData *)MEMORY_FOLLOWING(sg))

/*  Slider flags: */
#define SL_VERTICAL	0x00000020	/*  Vert. slider */
#define SL_GADGETDOWN	0x00000040	/*  Guaranteed msg upon GADGETDOWN */
#define SL_GADGETUP	0x00000080	/*  Guaranteed msg upon GADGETUP */
#define SL_DISPLAYLEVEL	0x00000100	/*  Display level beside slider */

/*------------------------------------------------------------------------*/

/*  Instance data for simple text display: */

struct TextIData
    {
    struct VisualInfo *txid_VisualInfo;
    struct IntuiText txid_IText;
    struct Rectangle txid_Extent;
    };

#define TXID(sg)	((struct TextIData *)MEMORY_FOLLOWING(sg))

/*------------------------------------------------------------------------*/

/*  All the visual information we need can be found in this private
    (hence extensible) structure: */

struct VisualInfo
    {
    struct Screen *vi_Screen;		/*  The screen in question */
    struct TextAttr *vi_ScreenTAttr;	/*  TextAttr of screen font */
    struct TextFont *vi_ScreenFont;	/*  opened copy of screen font */
    struct DrawInfo *vi_DrawInfo;	/*  DrawInfo structure */
    UWORD vi_textPen;			/*  Shorthand */
    UWORD vi_backgroundPen;		/*  Shorthand */
    ULONG vi_Flags;			/*  See below */
    };

#define VI(vi)	((struct VisualInfo *)vi)

#define VI_SCREENHIRES	0x00000001

/*------------------------------------------------------------------------*/

/*  For scroller arrows: */
struct ArrowBorder
    {
    struct Border Border1;
    struct Border Border2;
    WORD Points[8];
    };

/*  Arrow directions: */
#define ARROW_UP	0
#define ARROW_DOWN	1
#define ARROW_LEFT	2
#define ARROW_RIGHT	3

/*  Number of INTUITICKS to ignore before first repeat: */
#define ARROW_SKIPTICKS	4

/*------------------------------------------------------------------------*/

#define NM_TRUETYPE(type)	((type) & ~MENU_IMAGE)

/*  This structure gives us the easiest way to extract the original
    Left value for an Custom Image menu item: */
struct ImageMenuItem
    {
    struct MenuItem imi_MenuItem;
    void *imi_UserData;
    struct Image imi_Image;
    WORD imi_OrigLeft;
    };

/*------------------------------------------------------------------------*/
#endif /* !GADTOOLS_GTINTERNAL_H */
