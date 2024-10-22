/* aframeiclass.c
 * embossed frame image class
 *
 */

#include "appobjectsbase.h"

/* frameiclass.c */
Class *initAFrameIClass ( VOID );
ULONG freeAFrameIClass ( Class *cl );
ULONG __saveds dispatchAFrameI ( Class *cl , Object *o , Msg msg );
static ULONG frameBox ( Class *cl , Object *o , struct impFrameBox *msg );
static ULONG drawAFrameI ( Class *cl , Object *o , struct impDraw *msg );
static ULONG setAFrameIAttrs ( Class *cl , Object *o , struct opSet *msg );
static ULONG getAFrameIAttrs ( Class *cl , Object *o , struct opGet *msg );
static VOID interiorBox ( struct RastPort *rp , struct IBox *b , WORD xw , WORD yw , WORD pen, LONG state );
static VOID embossedBoxTrim (struct RastPort *,struct IBox *, WORD, WORD, WORD, WORD);

struct localObjectData
{
    UWORD lod_Flags;
    UWORD lod_VWidth;		/* Vertical bar width */
    UWORD lod_HWidth;		/* Horizontal bar width */
    struct
    {
	UWORD V;
	UWORD H;
    } lod_Margins;
};

#define LODF_RECESSED	(0x0001)
#define LODF_DOUBLE	(0x0002)
#define LODF_NOFILL	(0x0004)
#define	LODF_BORDER	(0x0008)

struct TagItem AFrameIFlagsMap[] =
{
    {IA_RECESSED,	LODF_RECESSED},
    {IA_DOUBLEEMBOSS,	LODF_DOUBLE},
    {IA_EDGESONLY,	LODF_NOFILL},
    {CGTA_InBorder,	LODF_BORDER},
    {TAG_END}
};

#define INSTANCESIZE		(sizeof (struct localObjectData))
#define DEFAULTLINEWIDTH	(1)
#define LHWIDTH			(1)
#define	LVWIDTH			(2)
#define IM(o)			((struct Image *)(o))
#define POINTLONG(pt)		(*((ULONG *)&(pt)))
#define MYCLASSID		"aframeiclass"
#define SUPERCLASSID		(IMAGECLASS)

Class *initAFrameIClass(VOID)
{
    Class *MakeClass();
    ULONG hookEntry();
    Class *cl;

    /* Try making the class */
    if (cl = MakeClass (MYCLASSID, SUPERCLASSID, NULL, INSTANCESIZE, 0))
    {
	/* Initialize the cl_Dispatcher hook */
	cl->cl_Dispatcher.h_Entry = hookEntry;
	cl->cl_Dispatcher.h_SubEntry = dispatchAFrameI;

	/* Make the class public */
	AddClass (cl);
    }

    /* Return a pointer to the now public class */
    return (cl);
}

ULONG freeAFrameIClass(Class *cl)
{
    return ((ULONG)FreeClass(cl));
}

ULONG __saveds dispatchAFrameI (Class *cl, Object *o, Msg msg)
{
    struct localObjectData *lod;
    ULONG retval;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    /* let superclass do it's creation routine first	 */
	    if (retval = DSM (cl, o, msg))
	    {
		struct TagItem *attrs = ((struct opSet *) msg)->ops_AttrList;
		struct DrawInfo *drinfo;

		/* Get a pointer to the instance data */
		lod = INST_DATA (cl, (Object *) retval);

		/* Get a pointer to the drawinfo */
		drinfo = (struct DrawInfo *) GetTagData (GA_DRAWINFO, NULL, attrs);

		/* Set the default line width */
		lod->lod_VWidth = LVWIDTH;
		lod->lod_HWidth = LHWIDTH;

		/* See if we have a DrawInfo with resolution information */
		if (drinfo)
		{
		    UWORD x = drinfo->dri_Resolution.X;
		    UWORD y = drinfo->dri_Resolution.Y;
		    UWORD xw = (y / x);

		    /* Figure out a smarter width */
		    lod->lod_VWidth = MAX (xw, 1);
		    lod->lod_HWidth = 1;
		}

		/* init my instance data	 */
		setAFrameIAttrs (cl, (Object *) retval, (struct opSet *) msg);
	    }
	    break;

	case OM_GET:
	    retval = getAFrameIAttrs (cl, o, (struct opGet *) msg);
	    break;

	case OM_SET:
	    /* let the superclass see the atts */
	    retval = DSM (cl, o, msg);

	    /* set the ones I care about */
	    retval += setAFrameIAttrs (cl, o, (struct opSet *) msg);
	    break;

	case IM_DRAW:		/* draw with state */
	case IM_DRAWFRAME:	/* special case of draw	 */
	    retval = drawAFrameI (cl, o, (struct impDraw *) msg);
	    break;

	case IM_FRAMEBOX:
	    retval = frameBox (cl, o, (struct impFrameBox *) msg);
	    break;

	default:
	    retval = (ULONG) DSM (cl, o, msg);
	    break;
    }

    return (retval);
}

static ULONG getAFrameIAttrs (Class *cl, Object *o, struct opGet *msg)
{
    struct localObjectData *lod = INST_DATA (cl, o);

    switch (msg->opg_AttrID)
    {
	case IA_LINEWIDTH:
	    *msg->opg_Storage = POINTLONG (lod->lod_Margins);
	    break;

	/* Let the superclass try */
	default:
	    return ((ULONG) DSM (cl, o, msg));
    }

    return (1L);
}

static ULONG setAFrameIAttrs (Class *cl, Object *o, struct opSet *msg)
{
    struct TagItem *tags = msg->ops_AttrList;
    struct localObjectData *lod = INST_DATA (cl, o);
    struct TagItem *tstate;
    struct TagItem *tag;
    ULONG tidata;

    /* Set up flags */
    lod->lod_Flags = PackBoolTags (lod->lod_Flags, tags, AFrameIFlagsMap);

    /* process rest */
    tstate = tags;
    while (tag = NextTagItem (&tstate))
    {
	tidata = tag->ti_Data;
	switch (tag->ti_Tag)
	{
	    case IA_LINEWIDTH:
		{
		    Point *margin = (Point *)tidata;

		    lod->lod_VWidth = margin->x;
		    lod->lod_HWidth = margin->y;
		}
		break;
	}
    }

    /* Set up the margins */
    lod->lod_Margins.V = lod->lod_VWidth;
    lod->lod_Margins.H = lod->lod_HWidth;
    if (lod->lod_Flags & LODF_DOUBLE)
    {
	lod->lod_Margins.V *= 2;
	lod->lod_Margins.H *= 2;
    }

    return (1L);
}

/*
 * return surrounding box, with a little bit of space.
 * If caller wants more space, he can pass me a larger
 * box
 */
static ULONG frameBox (Class *cl, Object *o, struct impFrameBox *msg)
{
    struct IBox *fbox;

    fbox = msg->imp_FrameBox;

    if (!(msg->imp_FrameFlags & FRAMEF_SPECIFY))
    {
	struct localObjectData *lod = INST_DATA (cl, o);
	UWORD h_fill, v_fill;

	*fbox = *msg->imp_ContentsBox;
	h_fill = (lod->lod_HWidth << 1);
	v_fill = (lod->lod_VWidth << 1);

	fbox->Width += (v_fill << 1);
	fbox->Height += (h_fill << 1);

	fbox->Left -= v_fill;
	fbox->Top -= h_fill;
    }
    else
    {
	/* use given dimensions and center */
	fbox->Left = msg->imp_ContentsBox->Left -
	  (fbox->Width - msg->imp_ContentsBox->Width) / 2;
	fbox->Top = msg->imp_ContentsBox->Top -
	  (fbox->Height - msg->imp_ContentsBox->Height) / 2;
    }

    return (1L);
}

static ULONG drawAFrameI (Class *cl, Object *o, struct impDraw *msg)
{
    struct localObjectData *lod = INST_DATA (cl, o);
    struct IBox box;
    UWORD *pens;		/* pen spec array */
    UWORD ulpen;		/* upper left	 */
    UWORD lrpen;		/* lower right	 */
    UWORD fillpen;		/* filled area	 */
    UWORD tmppen;

    /* let's be sure that we were passed a DrawInfo	 */
    pens = (msg->imp_DrInfo) ? msg->imp_DrInfo->dri_Pens : NULL;

    /* get Image.Left/Top/Width/Height */
    box = *IM_BOX (IM (o));
    box.Left += msg->imp_Offset.X;
    box.Top += msg->imp_Offset.Y;

    if (msg->MethodID == IM_DRAWFRAME)
    {
	box.Width = msg->imp_Dimensions.Width;
	box.Height = msg->imp_Dimensions.Height;
    }

    switch (msg->imp_State)
    {
	case IDS_SELECTED:
	case IDS_INACTIVESELECTED:
	    ulpen = pens ? pens[shadowPen] : 2;
	    lrpen = pens ? pens[shinePen] : 1;
	    if (lod->lod_Flags & LODF_BORDER)
		fillpen = pens ? pens[backgroundPen] : 0;
	    else
		fillpen = pens ? pens[hifillPen] : 3;
	    break;

	case IDS_INACTIVENORMAL:
	    ulpen = pens ? pens[shinePen] : 2;
	    lrpen = pens ? pens[shadowPen] : 1;
	    fillpen = pens ? pens[backgroundPen] : 0;
	    break;

	case IDS_NORMAL:
	case IDS_DISABLED:
	default:
	    ulpen = pens ? pens[shinePen] : 2;
	    lrpen = pens ? pens[shadowPen] : 1;

	    /* Is frame in the border? */
	    if (lod->lod_Flags & LODF_BORDER)
		fillpen = pens ? pens[hifillPen] : 0;
	    else
		fillpen = pens ? pens[backgroundPen] : 0;

	    break;
    }

    /* See if it is pushed in */
    if (lod->lod_Flags & LODF_RECESSED)
    {
	/* swap pens	 */
	tmppen = ulpen;
	ulpen = lrpen;
	lrpen = tmppen;
    }

    /* Draw the box */
    embossedBoxTrim (msg->imp_RPort, &box, lod->lod_VWidth, lod->lod_HWidth, ulpen, lrpen);

    /* Clear the interior */
    if (!(lod->lod_Flags & LODF_NOFILL) &&
	!(lod->lod_Flags & LODF_DOUBLE))
    {
	interiorBox (msg->imp_RPort, &box, lod->lod_VWidth, lod->lod_HWidth, fillpen, msg->imp_State);
    }

    if (lod->lod_Flags & LODF_DOUBLE)
    {
	/* swap pens	 */
	tmppen = ulpen;
	ulpen = lrpen;
	lrpen = tmppen;

	/* Adjust the box dimensions */
	box.Left += lod->lod_VWidth;
	box.Top += lod->lod_HWidth;
	box.Width -= (2 * lod->lod_VWidth);
	box.Height -= (2 * lod->lod_HWidth);

	/* Draw the box */
	embossedBoxTrim (msg->imp_RPort, &box, lod->lod_VWidth, lod->lod_HWidth, ulpen, lrpen);

	/* Clear the interior */
	if (!(lod->lod_Flags & LODF_NOFILL))
	{
	    interiorBox (msg->imp_RPort, &box, lod->lod_VWidth, lod->lod_HWidth, fillpen, msg->imp_State);
	}
    }

    if ((msg->imp_State == IDS_DISABLED) ||
	(msg->imp_State == IDS_INACTIVEDISABLED))
    {
	interiorBox (msg->imp_RPort, &box, lod->lod_VWidth, lod->lod_HWidth, pens[shadowPen], msg->imp_State);
    }

    return (1L);
}

/* fill region centered in a box */
static VOID interiorBox (struct RastPort *rp, struct IBox *b,
			 WORD xw, WORD yw, WORD pen, LONG state)
{
    if ((b->Width > (xw << 1)) && (b->Height > (yw << 1)))
    {
	UWORD pattern[] = {0x4444, 0x1111, 0x4444, 0x1111};

	rp->Mask = -1;
	BNDRYOFF (rp);
	SetAPen (rp, pen);

	/* set the ghosting pattern */
	if ((state == IDS_DISABLED) || (state == IDS_INACTIVEDISABLED))
	{
	    SetDrMd (rp, JAM1);
	    SetAfPt (rp, pattern, 2);
	}
	else
	{
	    SetDrMd (rp, JAM2);
	    SetAfPt (rp, NULL, 0);
	}

	/* Fill the region */
	RectFill (rp,
		  (b->Left + xw),
		  (b->Top + yw),
		  (b->Left + b->Width - xw - 1),
		  (b->Top + b->Height - yw - 1));

	/* Turn of pattern fill */
	SetAfPt (rp, NULL, 0);
    }
}

static VOID embossedBoxTrim (
	struct RastPort *rp,
	struct IBox *b,
	WORD vthick, WORD hthick,
	WORD ulpen, WORD lrpen)
{
    register WORD i, j;

    /* Knock the corners off if we're slender */
    j = (vthick == 1) ? 1 : 0;

    /* Draw the vertical lines */
    for (i = 0; i < vthick; i++)
    {
	/* Left side */
	SetAPen (rp, ulpen);
	Move (rp, (b->Left + i), b->Top);
	Draw (rp, (b->Left + i), (b->Top + b->Height - i - 1 - j));

	/* Right side */
	SetAPen (rp, lrpen);
	Move (rp, (b->Left + b->Width - i - 1), (b->Top + i + j));
	Draw (rp, (b->Left + b->Width - i - 1), (b->Top + b->Height - 1));
	j = 0;
    }

    /* Draw the horizontal lines */
    for (i = 0; i < hthick; i++)
    {
	/* Top side */
	SetAPen (rp, ulpen);
	Move (rp, b->Left, (b->Top + i));
	Draw (rp, (b->Left + b->Width - i - 2), (b->Top + i));

	/* Bottom side */
	SetAPen (rp, lrpen);
	Move (rp, (b->Left + i + 1), (b->Top + b->Height - i - 1));
	Draw (rp, (b->Left + b->Width - 1), (b->Top + b->Height - i - 1));
    }
}
