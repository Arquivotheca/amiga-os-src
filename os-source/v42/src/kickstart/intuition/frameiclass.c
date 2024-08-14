/*** frameiclass.c *******************************************************
 *
 *  frameiclass.c -- embossed frame image class.
 *
 *  $Id: frameiclass.c,v 40.0 94/02/15 18:03:20 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1990, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#include "intuall.h"
#include "classusr.h"
#include "classes.h"
#include "imageclass.h"
#include "newlook.h"

#include <graphics/gfxmacros.h>
#include <utility/pack.h>

/*****************************************************************************/

#define GETATTRS	FALSE	/* don't spend the space	*/

/*****************************************************************************/

#define D(x)	;
#define DI(x)	;

/*****************************************************************************/

struct localData
{
    UWORD lod_Flags;
    UWORD lod_FrameType;

#define LODF_RECESSED	(0x0001)
#define LODF_NOFILL	(0x0002)
};

#define INSTANCESIZE	(sizeof (struct localData))

/*****************************************************************************/

#define STROKEWIDTH	(1)	/* Regular thickness of vertical lines */
#define FAT_STROKEWIDTH	(2)	/* Thickness of non-FRAME_DEFAULT vertical lines */
#define STROKEHEIGHT	(1)	/* Regular thickness of horizontal lines */

#define IM(o)	((struct Image *)(o))	/* transparent base class */

/*****************************************************************************/

extern UBYTE *FrameIClassName;
extern UBYTE *ImageClassName;

#define MYCLASSID	FrameIClassName
#define SUPERCLASSID	ImageClassName

/*****************************************************************************/

ULONG frameiPackTable[] =
{
    PACK_STARTTABLE (IA_Dummy),
    PACK_WORDBIT (IA_Dummy, IA_Recessed, localData, lod_Flags, PKCTRL_BIT, LODF_RECESSED),
    PACK_WORDBIT (IA_Dummy, IA_EdgesOnly, localData, lod_Flags, PKCTRL_BIT, LODF_NOFILL),
    PACK_ENTRY (IA_Dummy, IA_FrameType, localData, lod_FrameType, PKCTRL_UWORD),
    PACK_ENDTABLE,
};

/*****************************************************************************/

/* Data values for IA_FrameType (recognized by FrameIClass, defined
 * in imageclass.h):
 *
 * FRAME_DEFAULT:  The standard V37-type frame, which has
 *	thin edges.
 * FRAME_BUTTON:  Standard button gadget frames, having thicker
 *	sides and nicely edged corners.
 * FRAME_RIDGE:  A ridge such as used by standard string gadgets.
 *	You can recess the ridge to get a groove image.
 * FRAME_ICONDROPBOX: A broad ridge which is the standard imagery
 *	for areas in AppWindows where icons may be dropped.
 *
 * Now we have some "properties" tables for each kind of frame-type.
 *
 * StrokeWidth[] is the width of vertical strokes used to draw the
 *	frame, as a function of lod_FrameType.  (Some frames have
 *	multiple strokes, which we count up later).
 *
 * STROKEHEIGHT is the height of the horizontal strokes used to draw
 *	the frame.  It's constant, so we skip having the array.
 *
 * InsetBox[], if non-zero, is the number of stroke-thicknesses
 *	to inset the inner box by, allowing composite frames like
 *	FRAME_RIDGE and FRAME_ICONDROPBOX.
 *
 * WidthPad[] is the amount of padding we need to add to the contents
 *	width to get a comfortable frame fitting around those contents.
 *	This is actually a _derived_ property, but it takes less ROM to
 *	store it precalculated, and we're worried about that right now.
 *	WidthPad[ frametype ] is defined as:
 *	    ( ( InsetBox[ frametype ] + 2 ) * StrokeWidth[ frametype ] ) << 1
 *	The "+2" adds twice the frame thickness, once to get inside the
 *	inner frame, and once to leave a little trim around the contents.
 *	The net result is doubled for (left+right).
 *
 * HeightPad[] is the amount of padding we need to add to the contents
 *	height to get a comfortable frame fitting around those contents.
 *	This is also a derived property, defined as:
 *	    ( ( InsetBox[ frametype ] + 2 ) * STROKEHEIGHT ) << 1;
 */

UBYTE StrokeWidth[] =
{
    STROKEWIDTH, FAT_STROKEWIDTH, FAT_STROKEWIDTH, FAT_STROKEWIDTH,
};

UBYTE InsetBox[] =
{
    0, 0, 1, 2,
};

UBYTE WidthPad[] =
{
    4, 8, 12, 16,
};

UBYTE HeightPad[] =
{
    4, 4, 6, 8,
};

/*****************************************************************************/

/* fill region centered in a box */
void interiorBox (struct RastPort *rp, struct IBox *b, LONG xw, LONG yw, LONG pen)
{
    BNDRYOFF (rp);
    drawBlock (rp,
	       b->Left + xw,
	       b->Top + yw,
	       b->Left + b->Width - 1 - xw,
	       b->Top + b->Height - 1 - yw,
	       pen);
}

/*****************************************************************************/

/* Since omSet() is nothing more than a call to PackStructureTags(),
 * it's smaller to take it in-line.  It's clearer to leave it as a macro.
 */
#define omSet(cl,o,msg ) \
	PackStructureTags(INST_DATA(cl,o),frameiPackTable,((struct opSet *)msg)->ops_AttrList)

/*****************************************************************************/

/* return surrounding box, with a little bit of space.
 * If caller wants more space, he can pass me a larger box */
static void imFrameBox (Class *cl, Object *o, struct impFrameBox *msg)
{
    struct localData *lod = INST_DATA (cl, o);
    struct IBox *fbox = msg->imp_FrameBox;

    D (printf ("imFrameBox height %lx\n", msg->imp_FrameBox->Height));

    if (!TESTFLAG (msg->imp_FrameFlags, FRAMEF_SPECIFY))
    {
	/* Caller wants us to size and position our box to
	 * fit comfortably around the contents.  InsetBox[] is
	 * the offset of the inner frame, if any.  To that, we
	 * add two (one to get inside the inner frame and one
	 * to add a frame's thickness of gap).  That's multiplied
	 * by the stroke width or height, and then doubled (for
	 * left+right or top+bottom).
	 */
	WORD frametype = lod->lod_FrameType;

	/* Let's size ourselves accordingly */
	*fbox = *msg->imp_ContentsBox;
	fbox->Width += WidthPad[frametype];
	fbox->Height += HeightPad[frametype];

	/* We can center ourselves by falling through to the FRAMEF_SPECIFY
	 * code...
	 */
    }
    /* Caller dictates frame size but wants us to center ourselves
     * within that box.
     */
    fbox->Left = msg->imp_ContentsBox->Left -
	(fbox->Width - msg->imp_ContentsBox->Width) / 2;
    fbox->Top = msg->imp_ContentsBox->Top -
	(fbox->Height - msg->imp_ContentsBox->Height) / 2;

    D (printf ("frame cont. height %ld query height %ld\n",
	       msg->imp_ContentsBox->Height, fbox->Height));
}

/*****************************************************************************/

static ULONG imDrawFrame (Class *cl, Object *o, struct impDraw *msg)
{
    struct localData *lod = INST_DATA (cl, o);
    struct IBox box;
    UWORD *pens;		/* pen spec array */
    ULONG ulpen;		/* upper left	*/
    ULONG lrpen;		/* lower right	*/
    ULONG fillpen;		/* filled area	*/
    LONG swappens;
    WORD frametype = lod->lod_FrameType;

    swappens = TESTFLAG (lod->lod_Flags, LODF_RECESSED);

    /* let's be sure that we were passed a DrawInfo	*/
    pens = (msg->imp_DrInfo) ? msg->imp_DrInfo->dri_Pens : NULL;

    box = *IM_BOX (IM (o));	/* get Image.Left/Top/Width/Height */
    box.Left += msg->imp_Offset.X;
    box.Top += msg->imp_Offset.Y;

    if (msg->MethodID == IM_DRAWFRAME)
    {
	box.Width = msg->imp_Dimensions.Width;
	box.Height = msg->imp_Dimensions.Height;
    }

    /* Default pens to use if DrawInfo pens are absent */
    ulpen = 2;			/* shine */
    lrpen = 1;			/* shadow */
    fillpen = 0;		/* background */

    if (pens)
    {
	/* Override with more sane pens */
	ulpen = pens[SHINEPEN];
	lrpen = pens[SHADOWPEN];
	fillpen = pens[BACKGROUNDPEN];
    }

    switch (msg->imp_State)
    {
	case IDS_SELECTED:
	case IDS_INACTIVESELECTED:
	    /* Swap shadow and shine, below */
	    swappens ^= LODF_RECESSED;
	    fillpen = 3;
	    if (pens)
	    {
		fillpen = pens[FILLPEN];
	    }
	    break;

#if 0
	case IDS_NORMAL:	/* doesn't use activefill in borders now */
	case IDS_DISABLED:	/* I don't have a ghosted version yet	 */
	case IDS_INACTIVENORMAL:	/* doesn't use activefill in borders now */
	default:
	    /* Pens already set up for default */
	    break;
#endif
    }

    if (swappens)
    {
	ULONG tmppen;

	/* swap pens	*/
	tmppen = ulpen;
	ulpen = lrpen;
	lrpen = tmppen;
    }

    /* Now actually draw the thing: */
    {
	LONG joins;
	LONG width = StrokeWidth[frametype];
	LONG multiplier;

	/* The default frame has a joins-type of JOINS_NONE, but
	 * FRAME_BUTTON, FRAME_RIDGE, and FRAME_ICONDROPBOX all use
	 * angled joins.
	 * NB: For space-saving reasons, embossedBoxTrim() infers
	 * hthick = 2 if joins == JOINS_ANGLED.
	 */
	joins = JOINS_NONE;
	if (frametype)
	{
	    joins = JOINS_ANGLED;
	}

	embossedBoxTrim (msg->imp_RPort, &box,
			 ulpen, lrpen, joins);

	if (multiplier = InsetBox[frametype])
	{
	    /* FRAME_RIDGE, and FRAME_ICONDROPBOX
	     * have a second ridge inside the first, whose
	     * colors are swapped.
	     */
	    ULONG tmppen;

	    /* swap pens	*/
	    tmppen = ulpen;
	    ulpen = lrpen;
	    lrpen = tmppen;

	    {
		LONG dx = width * multiplier;
		LONG dy = STROKEHEIGHT * multiplier;

		box.Left += dx;
		box.Width -= 2 * dx;
		box.Top += dy;
		box.Height -= 2 * dy;
	    }

	    embossedBoxTrim (msg->imp_RPort, &box,
			     ulpen, lrpen, joins);
	}

	/* interior */
	if (!TESTFLAG (lod->lod_Flags, LODF_NOFILL))
	{
	    interiorBox (msg->imp_RPort, &box,
			 width,	/* inset in x dim */
			 STROKEHEIGHT,	/* inset in y dim */
			 fillpen);
	}
    }
    return 0;
}

/*****************************************************************************/

static ULONG Dispatch (Class *cl, Object *o, Msg msg)
{
    Object *newobj;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    /* let superclass do it's creation routine first	*/
	    if (newobj = (Object *) SSM (cl, o, msg))
	    {
		/* init my instance data */
		omSet (cl, newobj, (struct opSet *) msg);
	    }
	    return ((ULONG) newobj);

#if GETATTRS
	case OM_GET:
	    return ((ULONG) omGet (cl, o, msg));
#endif

	case OM_SET:
	    SSM (cl, o, msg);	/* let the superclass see the atts */
	    omSet (cl, o, msg);	/* set the ones I care about */
	    return ((ULONG) 1);	/* i'm happy */

	case IM_DRAW:		/* draw with state */
	case IM_DRAWFRAME:	/* special case of draw	*/
	    return ((ULONG) imDrawFrame (cl, o, (struct impDraw *) msg));

	case IM_FRAMEBOX:
	    imFrameBox (cl, o, (struct impFrameBox *) msg);
	    return (1);		/* let him know I support this */

	default:
	    return ((ULONG) SSM (cl, o, msg));
    }
}

/*****************************************************************************/

Class *initFrameIClass (void)
{
    DI (printf ("init FrameIClass\n"));
    return (makePublicClass (MYCLASSID, SUPERCLASSID, INSTANCESIZE, Dispatch));
}
