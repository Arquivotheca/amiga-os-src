/* actiongclass.c
 * framed command button gadget class
 *
 * Confidential Information: Commodore-Amiga, Inc.
 * Copyright (C) 1989, 1990 Commodore-Amiga, Inc.
 * All Rights Reserved
 *
 */

#include "appobjectsbase.h"
#include "appobjects_protos.h"
#include <hardware/blit.h>

#define G(o)		((struct Gadget *)(o))
#define	MAX_INS		10
#define POINTLONG(pt)	(*((ULONG *)&(pt)))

#define	DB(x)	;

/* actiongclass.c */
Class *initActionGClass (VOID);
BOOL freeActionGClass (Class * cl);
ULONG __saveds dispatchActionG (Class * cl, Object * o, Msg msg);
ULONG renderActionG (Class * cl, Object * o, struct gpRender * msg);
ULONG setActionGAttrs (Class * cl, Object * o, struct opSet * msg);
ULONG getActionGAttrs (Class * cl, Object * o, struct opGet * msg);
VOID getContentsExtent (Object * o, struct DrawInfo * drinfo, struct IBox * contents);
VOID displayContents (struct RastPort * rp, Object * o, struct DrawInfo * drinfo, WORD offsetx, WORD offsety, int state);
VOID computeDomain (Class *, Object *, struct gpRender *, struct IBox *, struct IBox *, ULONG);
VOID computeBox (Class *, Object *, struct gpRender *, struct IBox *);
ULONG PointInBox (struct IBox * point, struct IBox * box);
WORD aTextExtent (struct RastPort * rp, STRPTR string, LONG count, struct TextExtent * te);
UWORD printIText (struct RastPort * rp, struct IntuiText * itext, WORD left, WORD top, WORD pens);
UWORD aText (struct RastPort * rp, STRPTR label, WORD left, WORD top);

#define MYCLASSID	"actiongclass"
#define SUPERCLASSID	"buttongclass"

struct localObjData
{
    struct IBox lod_Domain;	/* Domain of the gadget */
    struct IBox lod_Constraint;	/* Domain constraints */
    struct IBox lod_Nominal;	/* Nominal size */
    ULONG lod_Flags;		/* See defines below (and appobjectsbase) */
    Object *lod_Frame;		/* Class allocated frame */
    struct TextAttr *lod_TAttr;	/* Text attribute for label */

    struct
    {
	UWORD X;
	UWORD Y;
    } lod_LabelOffset;		/* Label offset */

    struct
    {
	WORD V;
	WORD H;
    } lod_Margins;		/* Margins for frame */

    UWORD lod_Keystroke;	/* Keystroke assigned to gadget */
};

/* Label defines frame */
#define TBDF_USEFRAME	0x10000000

/* Size is relative */
#define	TBDF_FLOATING	0x20000000

Class *initActionGClass (VOID)
{
    ULONG hookEntry ();
    Class *cl;

    if (cl = MakeClass (MYCLASSID, SUPERCLASSID, NULL, sizeof (struct localObjData), 0L))
    {
	/* Fill in the callback hook */
	cl->cl_Dispatcher.h_Entry = hookEntry;
	cl->cl_Dispatcher.h_SubEntry = dispatchActionG;

	/* Make the class public */
	AddClass (cl);
    }

    return (cl);
}

BOOL freeActionGClass (Class * cl)
{
    return ((BOOL) FreeClass (cl) );
}

ULONG __saveds dispatchActionG (Class * cl, Object * o, Msg msg)
{
    struct localObjData *lod;
    struct RastPort *rp;
    ULONG retval = 0L;
    register WORD i;
    Object *newobj;

    lod = INST_DATA (cl, o);

    switch (msg->MethodID)
    {
	case OM_NEW:
	    if (newobj = (Object *) DSM (cl, o, msg))
	    {
		struct TagItem *attrs = ((struct opSet *) msg)->ops_AttrList;
		struct TagItem ins[MAX_INS];
		struct DrawInfo *drinfo;
		struct TagItem *tag;
		WORD use = 0;
		Object *io;

		drinfo = (struct DrawInfo *) GetTagData (GA_DRAWINFO, NULL, attrs);

		/* Prefill insert table */
		for (i = 0; i < MAX_INS; i++)
		    ins[i].ti_Tag = TAG_IGNORE;

		/* End the table */
		ins[(MAX_INS - 1)].ti_Tag = TAG_DONE;

		/* Get the object's instance data */
		lod = INST_DATA (cl, newobj);

		/* Preset the constraints */
		lod->lod_Constraint.Left = NOT_SET;
		lod->lod_Constraint.Top = NOT_SET;
		lod->lod_Constraint.Width = NOT_SET;
		lod->lod_Constraint.Height = NOT_SET;

		/* Get a pointer to the frame */
		io = (Object *) G (newobj)->GadgetRender;

		/* See if we're display only */
		if (tag = FindTagItem (CGTA_DisplayOnly, attrs))
		{
		    if (tag->ti_Data)
		    {
			G (newobj)->Flags |= GADGHNONE;
			lod->lod_Flags |= CGTF_READONLY;

			/* Fill in tag values for frame */
			ins[use].ti_Tag = IA_RECESSED;
			ins[use++].ti_Data = TRUE;
		    }
		}

		/* See if we're borderless */
		if (GetTagData (CGTA_Borderless, NULL, attrs))
		{
		    lod->lod_Flags |= CGTF_BORDERLESS;
		}

		/* See if we need to make a frame */
		if (io == NULL)
		{
		    Point margins;

		    /* See if the gadget is in the border */
		    if ((G(newobj)->Activation) & 0x00F0)
		    {
			ins[use].ti_Tag = CGTA_InBorder;
			ins[use++].ti_Data = TRUE;
		    }

		    if (lod->lod_Flags & CGTF_BORDERLESS)
		    {
			/* Set to null margins */
			margins.x = 0;
			margins.y = 0;

			ins[use].ti_Tag = IA_LINEWIDTH;
			ins[use].ti_Data = (LONG) & margins;
		    }
		    else
		    {
			/* See if we have a DrawInfo structure */
			if (drinfo)
			{
			    ins[use].ti_Tag = GA_DRAWINFO;
			    ins[use++].ti_Data = (ULONG)drinfo;
			}

			/* See if they want double embossing */
			if (tag = FindTagItem (IA_DOUBLEEMBOSS, attrs))
			{
			    ins[use].ti_Tag = IA_DOUBLEEMBOSS;
			    ins[use++].ti_Data = tag->ti_Data;
			}

			/* See if they want filling */
			if (tag = FindTagItem (IA_EDGESONLY, attrs))
			{
			    ins[use].ti_Tag = IA_EDGESONLY;
			    ins[use++].ti_Data = tag->ti_Data;
			}

			/* See if they want it recessed */
			if (tag = FindTagItem (IA_RECESSED, attrs))
			{
			    ins[use].ti_Tag = IA_RECESSED;
			    ins[use++].ti_Data = tag->ti_Data;
			}

			/* See if they specified a line width */
			if (tag = FindTagItem (IA_LINEWIDTH, attrs))
			{
			    ins[use].ti_Tag = IA_LINEWIDTH;
			    ins[use].ti_Data = tag->ti_Data;
			}
		    }

		    /* Create a frame */
		    io = lod->lod_Frame = NewObjectA (NULL, "aframeiclass", ins);
		}

		/* See if we can inquire an image attribute */
		if (io && (((struct Image *) io)->Depth < 0))
		{
		    /* Get the margins of the frame */
		    DoMethod (io, OM_GET, IA_LINEWIDTH, &(lod->lod_Margins));
		}

		/* frame our contents, if any ... */
		if (G (newobj)->GadgetText && io)
		{
		    struct IBox framebox;

		    /* Get the bounding rectangle of the label */
		    getContentsExtent (newobj, drinfo, &(lod->lod_Nominal));

		    /* and if our image understands IM_FRAMEBOX	 */
		    if (DoMethod (io, IM_FRAMEBOX,
				  (LONG) & (lod->lod_Nominal), (LONG) & framebox, (LONG) drinfo, 0L))
		    {
			ULONG tagt1 = GA_WIDTH, tagd1;
			ULONG tagt2 = GA_HEIGHT, tagd2;
			struct TagItem *tag;

			/* use the frame dimensions and offset	 */
			lod->lod_Flags |= TBDF_USEFRAME;

			/* Get the width */
			tagd1 = GetTagData (GA_WIDTH, (LONG) framebox.Width, attrs);
			framebox.Width = (WORD) tagd1;

			/* Get the height */
			tagd2 = GetTagData (GA_HEIGHT, (LONG) framebox.Height, attrs);
			framebox.Height = (WORD) tagd2;

			/* See if we're using relative width */
			if (tag = FindTagItem (GA_RELWIDTH, attrs))
			{
			    tagt1 = GA_RELWIDTH;
			    tagd1 = tag->ti_Data;
			}

			/* See if we're using relative height */
			if (tag = FindTagItem (GA_RELHEIGHT, attrs))
			{
			    tagt2 = GA_RELHEIGHT;
			    tagd2 = tag->ti_Data;
			}

			/* Set our attributes to match */
			SetSuperAttrs (cl, newobj,
				       tagt1, tagd1,
				       tagt2, tagd2,
				       TAG_END);

			/*
			 * when we draw the frame at 0,0, we need to offset the
			 * text contents in the opposite direction.
			 */
			lod->lod_LabelOffset.X = -framebox.Left;
			lod->lod_LabelOffset.Y = -framebox.Top;
		    }
		}

		/* Set the attributes */
		setActionGAttrs (cl, newobj, (struct opSet *) msg);
	    }
	    retval = (ULONG) newobj;
	    break;

	case OM_GET:
	    retval = getActionGAttrs (cl, o, (struct opGet *) msg);
	    break;

	case OM_UPDATE:
	case OM_SET:
	    /* Do the superclass first */
	    retval = DSM (cl, o, msg);

	    /* Call our set routines */
	    retval += setActionGAttrs (cl, o, (struct opSet *) msg);

	    /* See if we should refresh */
	    if (retval && (OCLASS (o) == cl))
	    {
		/* Get a pointer to the rastport */
		if (rp = ObtainGIRPort (((struct opSet *) msg)->ops_GInfo))
		{
		    struct gpRender gpr = {NULL};

		    /* Force a redraw */
		    gpr.MethodID = GM_RENDER;
		    gpr.gpr_GInfo = ((struct opSet *) msg)->ops_GInfo;
		    gpr.gpr_RPort = rp;
		    gpr.gpr_Redraw = GREDRAW_REDRAW;

		    /* Redraw... */
		    DM (o, &gpr);

		    /* Release the temporary rastport */
		    ReleaseGIRPort (rp);
		}

		/* Clear the return value */
		retval = 0L;
	    }
	    break;

	case GM_HITTEST:
	    /* ReadOnly gadgets can't get hit */
	    if ((lod->lod_Flags & CGTF_READONLY) ||
		(lod->lod_Flags & CGTF_TRANSPARENT))
	    {
		/* We can never be hit */
	    }
	    else
	    {
		struct IBox *box = &(lod->lod_Domain);
		struct impHitTest imp =	{NULL};
		struct IBox no_constraint;
		struct Image *im;
		WORD ox, oy;

		/*
		 * Get the domain without constraints, because RELRIGHT and
		 * RELBOTTOM would be thrown off by the constraints
		 */
		computeDomain (cl, o, (struct gpRender *) msg, &no_constraint, NULL, NULL);

		/* Calculate the relative constrained top-left corner */
		ox = box->Left - no_constraint.Left;
		oy = box->Top - no_constraint.Top;

		/*
		 * Set up the mouse coordinates. Remember that X,Y is relative
		 * to the rectangle of the gadget
		 */
		imp.imp_Point.X = ((struct gpHitTest *) msg)->gpht_Mouse.X - ox;
		imp.imp_Point.Y = ((struct gpHitTest *) msg)->gpht_Mouse.Y - oy;

		/* Get the frame image */
		if (!(im = (struct Image *) G (o)->GadgetRender))
		{
		    im = (struct Image *) lod->lod_Frame;
		}

		if (im && im->Depth < 0)
		{
		    /* Ask custom boopsi image if it was hit */
		    imp.MethodID = IM_HITFRAME;
		    imp.imp_Dimensions.Width = box->Width;
		    imp.imp_Dimensions.Height = box->Height;

		    /* Hit? */
		    retval = DM ((Object *)im, &imp);
		}
		else if (im && im->Depth >= 0)
		{
		    /* Ask old-style image */
		    retval =
		      PointInBox ((struct IBox *) & imp.imp_Point,
				  (struct IBox *) & im->LeftEdge);
		}
	    }
	    break;

	case GM_RENDER:
	    /* Compute the domain of the box */
	    computeDomain (cl, o, (struct gpRender *) msg,
			   &(lod->lod_Domain), &(lod->lod_Constraint),
			   lod->lod_Flags);

	    /* Render the button */
	    renderActionG (cl, o, (struct gpRender *) msg);

	    break;

	case OM_DISPOSE:
	    /* See if we have a frame to dispose */
	    if (lod->lod_Frame)
	    {
		/* Delete our frame */
		DisposeObject (lod->lod_Frame);
	    }

	    /* Pass it up to the superclass */
	    retval = (ULONG) DSM (cl, o, msg);
	    break;

	case GM_HANDLEINPUT:
	    if (G(o)->Activation & RELVERIFY)
	    {
		retval = (ULONG) DSM (cl, o, msg);
	    }
	    else
	    {
		retval = GMR_NOREUSE;
	    }
	    break;

	/* Let the superclass handle everything else */
	default:
	    retval = (ULONG) DSM (cl, o, msg);
	    break;
    }

    return (retval);
}

/* Inquire attributes of an object */
ULONG getActionGAttrs (Class * cl, Object * o, struct opGet * msg)
{
    struct localObjData *lod = INST_DATA (cl, o);

    switch (msg->opg_AttrID)
    {
	/* Positioning information */
	case CGTA_LabelInfo:
	    *msg->opg_Storage = (ULONG) (lod->lod_Flags & LIBITS);
	    break;
	case CGTA_FrameInfo:
	    *msg->opg_Storage = (ULONG) (lod->lod_Flags & FIBITS);
	    break;

	/* Trigger keystroke */
	case CGTA_Keystroke:
	    *msg->opg_Storage = (ULONG) lod->lod_Keystroke;
	    break;

	/* Constraint information */
	case CGTA_MinWidth:
	    *msg->opg_Storage = (ULONG) lod->lod_Constraint.Left;
	    break;
	case CGTA_MinHeight:
	    *msg->opg_Storage = (ULONG) lod->lod_Constraint.Top;
	    break;
	case CGTA_MaxWidth:
	    *msg->opg_Storage = (ULONG) lod->lod_Constraint.Width;
	    break;
	case CGTA_MaxHeight:
	    *msg->opg_Storage = (ULONG) lod->lod_Constraint.Height;
	    break;
	case CGTA_Constraint:
	    *msg->opg_Storage = (ULONG) & (lod->lod_Constraint);
	    break;
	case CGTA_Nominal:
	    *msg->opg_Storage = (ULONG) & (lod->lod_Nominal);
	    break;

	/* Let the superclass try */
	default:
	    return ((ULONG) DSM (cl, o, msg));
    }

    return (1L);
}

/* Set attributes of an object */
ULONG setActionGAttrs (Class * cl, Object * o, struct opSet * msg)
{
    struct TagItem *tags = msg->ops_AttrList;
    struct TagItem *tstate;
    struct TagItem *tag;
    ULONG tidata;
    struct localObjData *lod;
    struct DrawInfo *drinfo;
    struct IBox newframe;
    ULONG refresh = 0L, reframe = 0L, extent = 0L;

    lod = INST_DATA (cl, o);

    /* only interested in Width/Height fields */
    newframe.Width = G (o)->Width;
    newframe.Height = G (o)->Height;

    /* process rest */
    tstate = tags;
    while (tag = NextTagItem (&tstate))
    {
	tidata = tag->ti_Data;
	switch (tag->ti_Tag)
	{
	    case GA_WIDTH:
		extent = reframe = refresh = 1L;
		newframe.Width = tidata;
		break;

	    case GA_HEIGHT:
		extent = reframe = refresh = 1L;
		newframe.Height = tidata;
		break;

	    /* Set the 'size is relative' flag */
	    case GA_RELWIDTH:
	    case GA_RELHEIGHT:
		lod->lod_Flags |= TBDF_FLOATING;
		refresh = 1L;
		break;

	    /* We need to refresh if this changes... */
	    case GA_TEXT:
	    case GA_INTUITEXT:
	    case GA_LABELIMAGE:
		extent = refresh = 1L;
		break;

	    /* Set the label information */
	    case CGTA_LabelInfo:
		lod->lod_Flags &= ~(LIBITS | LABEL_HIGHLIGHT);
		lod->lod_Flags |= (tidata & (LIBITS | LABEL_HIGHLIGHT));
		refresh = 1L;
		break;

	    /* Set the frame information */
	    case CGTA_FrameInfo:
		lod->lod_Flags &= ~FIBITS;
		lod->lod_Flags |= (tidata & FIBITS);
		refresh = 1L;
		break;

	    /* Set the frame constraints */
	    case CGTA_MinWidth:
		lod->lod_Constraint.Left = (WORD) tidata;
		refresh = 1L;
		break;
	    case CGTA_MinHeight:
		lod->lod_Constraint.Top = (WORD) tidata;
		refresh = 1L;
		break;
	    case CGTA_MaxWidth:
		lod->lod_Constraint.Width = (WORD) tidata;
		refresh = 1L;
		break;
	    case CGTA_MaxHeight:
		lod->lod_Constraint.Height = (WORD) tidata;
		refresh = 1L;
		break;
	    case CGTA_Constraint:
		lod->lod_Constraint = *((struct IBox *) tidata);
		refresh = 1L;
		break;

	    case CGTA_Transparent:
		if (tidata)
		    lod->lod_Flags |= CGTF_TRANSPARENT;
		else
		    lod->lod_Flags &= ~CGTF_TRANSPARENT;
		break;

	    case CGTA_TextAttr:
		lod->lod_TAttr = (struct TextAttr *) tidata;
		refresh = 1L;
		break;

	    case GA_SELECTED:
	    case GA_DISABLED:
		refresh = 1L;
		break;
	}
    }

    /* Validate constraints */
    {
	struct IBox *b = &(lod->lod_Constraint);
	WORD vm = (lod->lod_Margins.V * 2);
	WORD hm = (lod->lod_Margins.H * 2);

	/* Don't let the minimum size get smaller than the absolute minimum */
	if ((b->Left != NOT_SET) && (b->Left < vm))
	    b->Left = vm;
	if ((b->Top != NOT_SET) && (b->Top < hm))
	    b->Top = hm;

	/* Don't let the maximum width get smaller than the minimum width */
	if (b->Width != NOT_SET)
	{
	    if (b->Left == NOT_SET)
	    {
		if (b->Width < vm)
		    b->Width = vm;
	    }
	    else
	    {
		if (b->Width < b->Left)
		    b->Width = b->Left;
	    }
	}

	/* Don't let the maximum height get smaller than the minimum height */
	if (b->Height != NOT_SET)
	{
	    if (b->Top == NOT_SET)
	    {
		if (b->Height < vm)
		    b->Height = vm;
	    }
	    else
	    {
		if (b->Height < b->Top)
		    b->Height = b->Top;
	    }
	}
    }

    /* See if we need to get the nominal size */
    if (extent)
    {
	/* really need DrawInfo, if layout change	 */
	drinfo = msg->ops_GInfo ? msg->ops_GInfo->gi_DrInfo : NULL;

	/* override with tags version */
	drinfo = (struct DrawInfo *)
	  GetTagData (GA_DRAWINFO, (ULONG) drinfo, tags);

	/* get contents box again */
	getContentsExtent (o, drinfo, &(lod->lod_Nominal));
    }

    /* See if we need to reframe the contents */
    if (reframe)
    {
	/* get new offsets from the specified frame dimensions	 */
	if (lod->lod_Flags & TBDF_USEFRAME)
	{
	    struct Image *im;

	    if (!(im = (struct Image *) G (o)->GadgetRender))
	    {
		im = (struct Image *) lod->lod_Frame;
	    }

	    /* Adjust the frame box */
	    DoMethod ((Object *) im, IM_FRAMEBOX, &(lod->lod_Nominal),
		      &newframe, drinfo, FRAMEF_SPECIFY);

	    lod->lod_LabelOffset.X = -newframe.Left;
	    lod->lod_LabelOffset.Y = -newframe.Top;
	}
    }

    return (refresh);
}

VOID getContentsExtent (Object * o, struct DrawInfo * drinfo, struct IBox * contents)
{
    UBYTE *label;
    struct RastPort rport;
    struct TextExtent textent;

    /* maybe look at some flags to handle other types of text someday */
    if (label = (UBYTE *) G (o)->GadgetText)
    {
	/* Set up a temporary RastPort to contain a font */
	InitRastPort (&rport);
	if (drinfo && drinfo->dri_Font)
	{
	    SetFont (&rport, drinfo->dri_Font);
	}

	switch (G (o)->Flags & LABELMASK)
	{
		/* IntuiText */
	    case LABELITEXT:
		ITextLayout (&rport, G (o)->GadgetText, -1, contents, FALSE, 0, 0);
		break;

		/* STRPTR */
	    case LABELSTRING:
		aTextExtent (&rport, label, strlen (label), &textent);
		rectToBox ((struct IBox *) & textent.te_Extent, (struct IBox *) contents);
		break;

		/* Image */
	    case LABELIMAGE:
		*contents = *((struct IBox *)
			  & (((struct Image *) G (o)->GadgetText)->LeftEdge));
		break;
	}
    }
    else
    {
	contents->Left = contents->Top = 0;
	contents->Width = contents->Height = 0;
    }
}

ULONG renderActionG (Class * cl, Object * o, struct gpRender * msg)
{
    ULONG retval = 0L;

    /* See if we need to draw */
    if (msg->gpr_Redraw != GREDRAW_TOGGLE)
    {
	struct localObjData *lod = INST_DATA (cl, o);
	struct GadgetInfo *gi = msg->gpr_GInfo;
	struct IBox *box = &(lod->lod_Domain);
	struct RastPort *rp = msg->gpr_RPort;
	WORD vm = lod->lod_Margins.V;
	WORD hm = lod->lod_Margins.H;
	struct Image *frame;
	VOID *label;
	LONG state;

	/* Get the button's visual state */
	state = GetVisualState (G (o), gi);

	/* Get the button's frame */
	if (!(frame = (struct Image *) G (o)->GadgetRender))
	{
	    /* No user provided frame, so use the one we allocated */
	    frame = (struct Image *) lod->lod_Frame;
	}

	/* See if we have a frame to render */
	if (frame)
	{
	    /* See if we're using a custom image for the frame */
	    if (frame->Depth < 0)
	    {
		/* Build a draw message */
		struct impDraw imp = {NULL};

		imp.MethodID = IM_DRAWFRAME;
		imp.imp_RPort = rp;
		imp.imp_Offset.X = box->Left;
		imp.imp_Offset.Y = box->Top;
		imp.imp_State = state;
		imp.imp_DrInfo = gi->gi_DrInfo;
		imp.imp_Dimensions.Width = box->Width;
		imp.imp_Dimensions.Height = box->Height;

		/* Draw the button's frame */
		DM ((Object *) frame, &imp);
	    }
	    /* See if it is an old-style image */
	    else if (frame->Depth >= 0)
	    {
		DrawImageState (rp, frame,
				box->Left, box->Top, state, gi->gi_DrInfo);
	    }
	}

	/* See if we have a label to display */
	DB (kprintf("label=0x%lx\n", (LONG)G(o)->GadgetText));
	if (label = (VOID *) G (o)->GadgetText)
	{
	    struct IBox *extent = &(lod->lod_Nominal);
	    struct DrawInfo *drinfo = gi->gi_DrInfo;
	    struct Window *win = gi->gi_Window;
	    struct Region *new_r = NULL, *old_r = NULL;
	    WORD dw = (box->Width - (vm + 1 ));
	    WORD dh = (box->Height - (hm + 1));
	    WORD tx = (box->Left + vm);
	    WORD ty = (box->Top + hm);
	    struct RastPort *drp = rp;
	    struct IBox framebox;
	    BOOL refresh = FALSE;
	    WORD textpen;
	    WORD offsetx;
	    WORD offsety;

	    /* Preset some of the information */
	    framebox.Width = extent->Width;
	    framebox.Height = extent->Height;

	    /* See if we can get frame information */
	    DB (kprintf("framebox\n"));
	    if ((frame->Depth < 0) &&
		(DoMethod ((Object *) frame, IM_FRAMEBOX,
			(LONG) extent, (LONG) & framebox, (LONG) drinfo, 0L)))
	    {
		tx = (box->Left + (-framebox.Left) + 1);
		ty = (box->Top + (-framebox.Top) + 1);
	    }

	    /* Offset at top-left corner */
	    offsetx = tx;
	    offsety = ty;

	    /* Handle the horizontal justification */
	    if (lod->lod_Flags & LABEL_HCENTER)
		offsetx = tx + ((dw - framebox.Width) / 2) + 1;
	    else if (lod->lod_Flags & LABEL_RIGHT)
		offsetx = tx + (dw - framebox.Width) + 1;

	    /* Handle the vertical justification */
	    if (lod->lod_Flags & LABEL_VCENTER)
		offsety = ty + ((dh - framebox.Height) / 2);
	    else if (lod->lod_Flags & LABEL_BOTTOM)
		offsety = ty + (dh - framebox.Height);

	    /* Do we need to install a region? */
	    DB (kprintf("install region\n"));
	    if ((extent->Width > dw) ||
		(extent->Height > dh) ||
		(offsetx <= (box->Left + vm)) ||
		(offsety <= (box->Top + hm)))
	    {
		if (new_r = (struct Region *) NewRegion ())
		{
		    struct Rectangle rect;

		    rect.MinX = (box->Left + vm);
		    rect.MinY = (box->Top + hm);
		    rect.MaxX = (box->Left + box->Width - (vm + 1));
		    rect.MaxY = (box->Top + box->Height - (hm + 1));

		    OrRectRegion (new_r, &rect);

		    if (win->Flags & WINDOWREFRESH)
		    {
			refresh = TRUE;
			EndRefresh (win, FALSE);
		    }

		    old_r = InstallClipRegion (rp->Layer, new_r);
		}
	    }

	    /* maybe use a different pen if selected? */
	    textpen = TEXTPEN;
	    if (state == IDS_SELECTED || state == IDS_INACTIVESELECTED)
	    {
		textpen = FILLTEXTPEN;
	    }

	    /* Set the drawing mode and text pen */
	    SetDrMd (drp, JAM1);
	    SetAPen (drp, drinfo->dri_Pens[textpen]);

	    DB (kprintf ("display label\n"));
	    switch (G (o)->Flags & LABELMASK)
	    {
		    /* IntuiText */
		case LABELITEXT:
		    DB (kprintf ("labelitext\n"));
		    printIText (drp, label, offsetx, offsety, FALSE);
		    break;

		    /* STRPTR */
		case LABELSTRING:
		    DB (kprintf ("labelstring\n"));
		    aText (drp, label, offsetx, offsety);
		    break;

		    /* Image */
		case LABELIMAGE:
		    DB (kprintf("labelimage\n"));
		    {
			struct Image *im = (struct Image *) label;

			if (im->Depth >= 0)
			{
			    /* Old-style image ... */
			    DB (kprintf("drawImage rp=0x%lx im=0x%lx dri=0x%lx\n", drp, im, drinfo));
			    drawImage (drp, im, offsetx, offsety, state, drinfo);
			}
			else
			{
#if 1
			    /* Build a draw message */
			    struct impDraw imp = {NULL};

			    /* Prep the message */
			    imp.MethodID = IM_DRAWFRAME;
			    imp.imp_RPort = drp;
			    imp.imp_State = state;
			    imp.imp_DrInfo = drinfo;
			    imp.imp_Offset.X = (LONG) (box->Left + vm);
			    imp.imp_Offset.Y = (LONG) (box->Top + hm);
			    imp.imp_Dimensions.Width = (LONG) (box->Width - (2 * vm));
			    imp.imp_Dimensions.Height = (LONG) (box->Height - (2 * hm));

			    /* Draw the text */
			    DB (kprintf("drawframe\n"));
			    DM ((Object *) im, &imp);
#else
			    /* Custom image... */
			    DrawImageState (drp, im, offsetx, offsety, state, drinfo);
#endif
			}
		    }
		    break;
	    }

	    DB (kprintf ("restore clip region\n"));
	    if (new_r)
	    {
		InstallClipRegion (rp->Layer, old_r);

		if (refresh)
		{
		    BeginRefresh (win);
		}

		DisposeRegion (new_r);
	    }
	}

	/* Show success... */
	retval = 1L;
    }

    return (retval);
}
