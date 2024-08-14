/*** frbuttonclass.c *******************************************************
 *
 * frbuttonclass.c -- framed command button gadget class
 *
 *  $Id: frbuttonclass.c,v 40.0 94/02/15 18:03:39 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, 1990 Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#include "intuall.h"
#include "classusr.h"
#include "classes.h"
#include "imageclass.h"
#include "gadgetclass.h"

/*****************************************************************************/

#define D(x)	;
#define DL(x)	;

/*****************************************************************************/

/* transparent base class */
#define G(o)	((struct Gadget *)(o))

/* jimm's peculiarities */
#define testf(v,f) ((v) & (f))
#define setf(v,f)	((v) |= (f))
#define clearf(v,f)	((v) &= ~(f))

/*****************************************************************************/

/* this useful to convert a point structure to a 32 bit vanilla
 * data type if you want to pass it via libcall to lattice.
 * probably can fudge it some other way ...
 */
/* #define POINTLONG( pt )	(((pt).X<<16) + (pt).Y) */
#define POINTLONG( pt )	(*( (ULONG *)  &(pt) ))

/*****************************************************************************/

extern UBYTE *FrButtonClassName;
extern UBYTE *ButtonGClassName;

#define MYCLASSID	FrButtonClassName
#define SUPERCLASSID	ButtonGClassName

/*****************************************************************************/

struct localData
{
    /* this is the offset of the contents, relative (0,0) of the gadget and its frame */
    struct
    {
	UWORD X;
	UWORD Y;
    } lod_Offset;
    ULONG lod_Flags;

#define TBDF_USEFRAME	(1<<0)
};

/*****************************************************************************/

/* show enclosed text label in correct color, or more general label object.
 * always use textpen for now */
static void displayContents (struct RastPort *rp, Object * o, struct DrawInfo *drinfo, WORD offsetx, WORD offsety, LONG state)
{
    void *label;
    LONG textpen;

    if (label = (void *) G (o)->GadgetText)
    {
	/*  maybe use a different pen if selected? */
	if (state == IDS_SELECTED || state == IDS_INACTIVESELECTED)
	{
	    textpen = FILLTEXTPEN;
	}
	else
	{
	    textpen = TEXTPEN;
	}
	SetABPenDrMd (rp, drinfo->dri_Pens[textpen], 0, JAM1);

	switch (G (o)->Flags & LABELMASK)
	{
	    case LABELITEXT:	/* intuitext	*/
		SetDrMd (rp, JAM1);
		printIText (rp, label, offsetx, offsety, FALSE);
		break;

	    case LABELSTRING:	/* (UBYTE *) */
		Move (rp, offsetx, offsety);
		Text (rp, label, strlen (label));
		break;

	    case LABELIMAGE:	/* some image object, the extensible future	*/
		DrawImageState (rp, label, offsetx, offsety, state, drinfo);
		break;
	}
    }
}

/*****************************************************************************/

/* This class has become the "button with label which is rendered by overdrawing some frame" class. */
static void getContentsExtent (Object * o, struct DrawInfo *drinfo, struct IBox *contents)
{
    UBYTE *label;
    struct RastPort rport;
    struct TextExtent textent;

#if 1
    /* maybe look at some flags to handle other types of text someday */
    if (label = (UBYTE *) G (o)->GadgetText)
    {
	InitRastPort (&rport);
	if (drinfo && drinfo->dri_Font)
	{
	    SetFont (&rport, drinfo->dri_Font);
	}

	switch (G (o)->Flags & LABELMASK)
	{
	    case LABELITEXT:	/* intuitext	*/
		ITextLayout (&rport, G (o)->GadgetText, contents, FALSE, 0, 0);
		DL (dumpBox ("getContentsExtent", contents));
		break;
	    case LABELSTRING:	/* (UBYTE *) */
		TextExtent (&rport, label, strlen (label), &textent);
		rectToBox (&textent.te_Extent, contents);
		break;

	    case LABELIMAGE:	/* some image object	*/
		/* just copy over the image's box	*/
		*contents = *((struct IBox *)
			      &(((struct Image *) G (o)->GadgetText)->LeftEdge));
		break;
	}
    }
    else
    {
	contents->Left = contents->Top =
	    contents->Width = contents->Height = 0;
    }
#else
    /* maybe look at some flags to handle other types of text someday */
    if (label = (UBYTE *) G (o)->GadgetText)
    {
	InitRastPort (&rport);
	if (drinfo && drinfo->dri_Font)
	{
	    SetFont (&rport, drinfo->dri_Font);
	}
	TextExtent (&rport, label, strlen (label), &textent);
	rectToBox (&textent.te_Extent, contents);
    }
    else
    {
	contents->Left = contents->Top =
	    contents->Width = contents->Height = 0;
    }
#endif
}

/*****************************************************************************/

static void renderFramedB (struct RastPort *rp, struct GadgetInfo *gi, Object * o, struct localData *lod, LONG redraw_mode)
{
    LONG state;

    if (redraw_mode == GREDRAW_TOGGLE)
	return;

    state = gadgetDrawState (G (o), gi, FALSE);

    /* draw frame */
    SendMessage (G (o)->GadgetRender, IM_DRAWFRAME, rp,
		 *((ULONG *) & G (o)->LeftEdge),
		 state,
		 gi->gi_DrInfo,
		 *((ULONG *) & G (o)->Width));	/* frame dimensions */

    displayContents (rp, o, gi->gi_DrInfo,
		     G (o)->LeftEdge + lod->lod_Offset.X,
		     G (o)->TopEdge + lod->lod_Offset.Y, state);
}

/*****************************************************************************/

static LONG setFramedBAttrs (Class * cl, Object * o, struct opSet *msg)
{
    struct localData *lod = INST_DATA (cl, o);
    struct TagItem *tags = msg->ops_AttrList;
    struct TagItem *tstate;
    struct TagItem *tag;
    ULONG tidata;

    LONG refresh = 0;
    struct DrawInfo *drinfo;
    struct IBox newframe;

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

		/* if forced dimension change, then
	 * center text, recalc offsets using FRAMEF_SPECIFY
	 */
	    case GA_WIDTH:
		refresh = 1;
		newframe.Width = tidata;
		break;
	    case GA_HEIGHT:
		refresh = 1;
		newframe.Height = tidata;
		break;
		/* could also reset for a new GA_IMAGE here, if
	 * you wanted
	 */
	}
    }

    /* fix up changes which require some work	*/
    if (refresh)
    {
	/* get new offsets from the specified frame dimensions	*/

	if (lod->lod_Flags & TBDF_USEFRAME)
	{
	    struct IBox contents;

	    /* really need DrawInfo, if layout change	*/
	    drinfo = msg->ops_GInfo ? msg->ops_GInfo->gi_DrInfo : NULL;
	    DL (printf ("drawinfo from ginfo %lx %lx\n", msg->ops_GInfo, drinfo));

	    /* override with tags version */
	    drinfo = (struct DrawInfo *) GetUserTagData (GA_DRAWINFO, (ULONG) drinfo, tags);
	    DL (printf ("drawinfo from tags %lx\n", drinfo));

	    /* get contents box again */
	    getContentsExtent (o, drinfo, &contents);

	    SendMessage (G (o)->GadgetRender, IM_FRAMEBOX, &contents,
			 &newframe, drinfo, FRAMEF_SPECIFY);

	    lod->lod_Offset.X = -newframe.Left;
	    lod->lod_Offset.Y = -newframe.Top;
	    DL (dumpPt ("offsets at SET", lod->lod_Offset));
	}

	/* superclass knows about the new dimensions, and I don't
	 * have any reason to disagree, so no need to
	 * override the values now.
	 */
    }

    return (refresh);
}

/*****************************************************************************/

static ULONG Dispatcher (Class * cl, Object * o, Msg msg)
{
    Object *newobj;
    struct RastPort *rp;
    LONG refresh;
    struct localData *lod;

    lod = INST_DATA (cl, o);

    switch (msg->MethodID)
    {
	case OM_NEW:
	    D (kprintf ("framebclass new\n"));
	    if (newobj = (Object *) SSM (cl, o, msg))
	    {
		lod = INST_DATA (cl, newobj);	/* want it for newobject */

		/* frame our contents, if any ... */
		if (G (newobj)->GadgetText)
		{
		    struct IBox contents;
		    struct IBox framebox;
		    struct DrawInfo *drinfo;

		    /* get the contents	*/
		    drinfo = (struct DrawInfo *) GetUserTagData0 (GA_DRAWINFO,
								  ((struct opSet *) msg)->ops_AttrList);
		    DL (printf ("frbutton OM_NEW drawinfo %lx\n", drinfo));
		    getContentsExtent (newobj, drinfo, &contents);

		    /* and if our image understands IM_FRAMEBOX	*/
		    if (SendMessage (G (newobj)->GadgetRender, IM_FRAMEBOX,
				     &contents, &framebox, drinfo, 0))
		    {
			/* use the frame dimensions and offset	*/
			lod->lod_Flags |= TBDF_USEFRAME;
			D (kprintf ("call SSA to set width/height %lx/%lx\n",
				    framebox.Width, framebox.Height));

			mySetSuperAttrs (cl, newobj,
					 GA_WIDTH, framebox.Width,
					 GA_HEIGHT, framebox.Height,
					 TAG_END);
			D (kprintf (" mySetSuperAttrs returns, gadget Height %\n"));

			/* when we draw the frame at 0,0, we
		     * need to offset the text contents
		     * in the opposite direction.
		     */
			lod->lod_Offset.X = -framebox.Left;
			lod->lod_Offset.Y = -framebox.Top;
			DL (dumpPt ("offsets at NEW", lod->lod_Offset));
		    }
		    /* ZZZ: else just fail if not a frame? */
		}

		/*  set attributes in general, which might force dims */
		setFramedBAttrs (cl, newobj, (struct opSet *) msg);
	    }
	    return ((ULONG) newobj);

	case OM_UPDATE:
	case OM_SET:
	    /* inherit atts, accumulated refresh	*/
	    refresh = SSM (cl, o, msg);
	    refresh += setFramedBAttrs (cl, o, (struct opSet *) msg);

	    /* take responsibility here for refreshing visuals
	 * if I am the "true class."  It's sort of too bad
	 * that every class has to do this itself.
	 */
	    if (refresh && (OCLASS (o) == cl))
	    {
		rp = ObtainGIRPort (((struct opSet *) msg)->ops_GInfo);
		if (rp)
		    renderFramedB (rp, ((struct opSet *) msg)->ops_GInfo, o, lod, GREDRAW_UPDATE);
		ReleaseGIRPort (rp);
		return (0);	/* don't need refresh any more */
	    }
	    /* else */
	    return ((ULONG) refresh);	/* return to subclass perhaps */

	case GM_HITTEST:
	    /* ask our frame if we have one */
	    /* even non-frames will understand this */
	    return (ULONG)(SendMessage (G (o)->GadgetRender,
				 IM_HITFRAME,
				 POINTLONG (((struct gpHitTest *) msg)->gpht_Mouse),
				 *((ULONG *) & G (o)->Width)) ? GMR_GADGETHIT : 0);

	case GM_RENDER:
	    renderFramedB (((struct gpRender *) msg)->gpr_RPort,
			   ((struct gpRender *) msg)->gpr_GInfo, o, lod,
			   ((struct gpRender *) msg)->gpr_Redraw);
	    break;

#if 0
	case OM_GET:		/* I'm not telling anyone about it yet	*/
	case GM_GOACTIVE:
	case GM_GOINACTIVE:
	case GM_HANDLEINPUT:	/* the big one	*/
#endif
	default:		/* let the superclass handle it */
	    return ((ULONG) SSM (cl, o, msg));
    }
    return (0);
}

/*****************************************************************************/

Class *initFramedButtonClass (void)
{
    return (makePublicClass (MYCLASSID, SUPERCLASSID, sizeof (struct localData), Dispatcher));
}
