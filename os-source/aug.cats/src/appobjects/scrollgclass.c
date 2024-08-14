/* scrollgclass.c
 * slider with arrows, compatible with the GadTools scroller.
 */

#include "appobjectsbase.h"

#define	DB(x)	x

#define	G(x)	((struct Gadget *)(x))

struct modelObjData
{
    LONG lod_Total;
    LONG lod_Visible;
    LONG lod_Top;
    ULONG lod_ID;
};

#define	LSIZE	(sizeof(struct modelObjData))

Class *initScrollModelClass (VOID);
ULONG freeScrollModelClass (Class * cl);
ULONG __saveds dispatchScrollModel (Class * cl, Object * o, Msg msg);
ULONG setScrollModelAttrs (Class * cl, Object * o, struct opSet * msg);
ULONG getScrollModelAttr (Class * cl, Object * o, struct opGet * msg);

Class *initScrollModelClass (VOID)
{
    ULONG hookEntry ();
    Class *cl;

    if (cl = MakeClass (NULL, MODELCLASS, NULL, LSIZE, 0))
    {
	/* initialize the cl_Dispatcher Hook */
	cl->cl_Dispatcher.h_Entry = hookEntry;
	cl->cl_Dispatcher.h_SubEntry = dispatchScrollModel;
    }

    return (cl);
}

ULONG freeScrollModelClass (Class * cl)
{
    return ((ULONG) FreeClass (cl));
}

ULONG __saveds dispatchScrollModel (Class * cl, Object * o, Msg msg)
{
    struct modelObjData *lod = INST_DATA (cl, o);
    struct opSet *ops = (struct opSet *) msg;
    ULONG retval = 0L;
    Object *newobj;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    /* Have the superclass create the object */
	    if (newobj = (Object *) DSM (cl, o, msg))
	    {
		/* Get the local object data */
		lod = INST_DATA (cl, newobj);

		/* Get the gadget ID */
		lod->lod_ID = GetTagData (GA_ID, NULL, ops->ops_AttrList);

		/* Initialize instance data */
		setScrollModelAttrs (cl, newobj, ops);
	    }
	    retval = (ULONG) newobj;
	    break;

	case OM_GET:
	    retval = (ULONG) getScrollModelAttr (cl, o, (struct opGet *) msg);
	    break;

	case OM_SET:
	case OM_UPDATE:
	    if (!DoSuperMethod (cl, o, ICM_CHECKLOOP))
	    {
#if 0
		LONG otopv = lod->lod_Top;
		LONG ovisv = lod->lod_Visible;
		LONG ototv = lod->lod_Total;
#endif
		ULONG topt = PGA_Top;
		ULONG vist = PGA_Visible;
		ULONG tott = PGA_Total;
		ULONG nmflag = 0L;

		/*
		 * let the superclass see whatever it wants from OM_SET, such
		 * as ICA_TARGET, ICA_MAP, and so on.  For OM_NOTIFY, however,
		 * we control all traffic and issue notification specifically
		 * for the attributes we're interested in.
		 */
		if (msg->MethodID == OM_SET)
		{
		    DSM (cl, o, msg);
		}
		else
		{
		    /* these flags aren't present in the message of OM_SET	 */
		    nmflag = ((struct opUpdate *) msg)->opu_Flags;
		}

		/* change 'em, only if changed (or if a "non-interim" message. */
		if (setScrollModelAttrs (cl, o, ops) || !(nmflag & OPUF_INTERIM))
		{
#if 0
		    if (otopv == lod->lod_Top)
			topt = TAG_IGNORE;
		    if (ovisv == lod->lod_Visible)
			vist = TAG_IGNORE;
		    if (ototv == lod->lod_Total)
			tott = TAG_IGNORE;
#endif

		    /*
		     * Pass along GInfo, if any, so gadgets can redraw
		     * themselves.  Pass along opu_Flags, so that the
		     * application will know the difference between and
		     * interim message and a final message
		     */
		    notifyAttrChanges (o,
				       ops->ops_GInfo,
				       (nmflag & OPUF_INTERIM),
				       GA_ID, lod->lod_ID,
				       topt, lod->lod_Top,
				       vist, lod->lod_Visible,
				       tott, lod->lod_Total,
				       TAG_END);
		}
	    }
	    break;

	default:
	    retval = (ULONG) DSM (cl, o, msg);
	    break;
    }

    return (retval);
}

ULONG setScrollModelAttrs (Class * cl, Object * o, struct opSet * msg)
{
    struct modelObjData *lod = INST_DATA (cl, o);
    struct TagItem *tags = msg->ops_AttrList;
    struct TagItem *tstate;
    struct TagItem *tag;
    ULONG retval = 0L;
    ULONG tidata;
    LONG newval;

    /* start with original value */
    newval = lod->lod_Top;

    /* Process tags */
    tstate = tags;
    while (tag = NextTagItem (&tstate))
    {
	tidata = tag->ti_Data;
	switch (tag->ti_Tag)
	{
	    case GTSC_Total:
	    case PGA_Total:
		if (lod->lod_Total != tidata)
		{
		    lod->lod_Total = tidata;
		    retval = 1L;
		}
		break;

	    case GTSC_Visible:
	    case PGA_Visible:
		if (lod->lod_Visible != tidata)
		{
		    lod->lod_Visible = tidata;
		    retval = 1L;
		}
		break;

	    case GTSC_Top:
	    case PGA_Top:
		newval = tidata;
		break;

	    case CGTA_Decrement:
		newval--;
		break;

	    case CGTA_Increment:
		newval++;
		break;
	}
    }

    /* Validity check */
    if (lod->lod_Visible <= 0)
    {
	lod->lod_Visible = 1;
	retval = 1L;
    }

    /* Validity check */
    if (lod->lod_Total <= 0)
    {
	lod->lod_Total = 1;
	retval = 1L;
    }

    /* limit lod_Top to lod_Total-lod_Visible */
    if (newval > (lod->lod_Total - lod->lod_Visible))
    {
	newval = lod->lod_Total - lod->lod_Visible;
    }

    if (newval < 0)
    {
	newval = 0;
    }

    if (lod->lod_Top != newval)
    {
	lod->lod_Top = newval;
	retval = 1L;
    }

    return (retval);
}

ULONG getScrollModelAttr (Class * cl, Object * o, struct opGet * msg)
{
    struct modelObjData *lod = INST_DATA (cl, o);
    ULONG retval = 1L;


    switch (msg->opg_AttrID)
    {
	case PGA_Top:
	    *msg->opg_Storage = lod->lod_Top;
	    break;

	case PGA_Total:
	    *msg->opg_Storage = lod->lod_Total;
	    break;

	case PGA_Visible:
	    *msg->opg_Storage = lod->lod_Visible;
	    break;

	default:
	    /* I don't recognize this one, let the superclass try */
	    retval = (ULONG) DSM (cl, o, msg);
	    break;
    }

    return (retval);
}

/*-------------------------------------------------------------------------*/

/* public class */
#define MYCLASSID	"scrollgclass"
#define SUPERCLASSID	"lgroupgclass"

/* scrollgclass.c */
Class *initScrollGClass (VOID);
ULONG freeScrollGClass (Class * cl);
ULONG __saveds dispatchScrollG (Class * cl, Object * o, Msg msg);
ULONG initScrollGObject (Class * cl, Object * o, struct opSet * msg);
ULONG getScrollGAttrs (Class * cl, Object * o, struct opGet * msg);
ULONG setScrollGAttrs (Class * cl, Object * o, struct opSet * msg);
VOID layoutScrollG (Class * cl, Object * o, struct gpRender * gpr, BOOL);

struct localObjData
{
    struct IBox lod_Domain;	/* Domain of the gadget */
    UWORD lod_Flags;		/* See defines below (and appobjectsbase) */
    WORD lod_W, lod_H;		/* Arrow size */

    /* Other objects allocated */
    Object *lod_Glue;
    struct Image *lod_UpImage;
    struct Image *lod_DnImage;
    struct Gadget *lod_Slider;
    struct Gadget *lod_UpArrow;
    struct Gadget *lod_DnArrow;

    USHORT lod_ID;		/* GadgetID */
};

#define	LODSIZE	(sizeof(struct localObjData))

Class *initScrollModelClass (VOID);
ULONG freeScrollModelClass (Class * cl);

Class *initScrollGClass (VOID)
{
    Class *sm, *cl = NULL;
    ULONG hookEntry ();

    if (sm = initScrollModelClass ())
    {
	if (cl = MakeClass (MYCLASSID, SUPERCLASSID, NULL, LODSIZE, 0))
	{
	    /* initialize the cl_Dispatcher Hook */
	    cl->cl_Dispatcher.h_Entry = hookEntry;
	    cl->cl_Dispatcher.h_SubEntry = dispatchScrollG;

	    /* Remember our support model class */
	    cl->cl_UserData = (ULONG) sm;

	    /* Make the class public */
	    AddClass (cl);
	}
	else
	{
	    /* Cleanup */
	    freeScrollModelClass (sm);
	}
    }

    return (cl);
}

ULONG freeScrollGClass (Class * cl)
{
    ULONG retval = NULL;
    Class *sm;

    if (cl)
    {
	sm = (Class *) cl->cl_UserData;

	/* Free the gadget class */
	if (retval = (ULONG) FreeClass (cl))
	{
	    /* Free our model class */
	    FreeClass (sm);
	}
    }

    return (retval);
}

ULONG __saveds dispatchScrollG (Class * cl, Object * o, Msg msg)
{
    struct localObjData *lod = INST_DATA (cl, o);
    ULONG retval;
    LONG top;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    if (retval = DSM (cl, o, msg))
	    {
		if (!(initScrollGObject (cl, (Object *)retval, (struct opSet *)msg)))
		{
		    /* Free what's there if failure */
		    CoerceMethod (cl, (Object *) retval, OM_DISPOSE);
		    return (NULL);
		}
	    }
	    break;

	case OM_GET:
	    retval = getScrollGAttrs (cl, o, (struct opGet *) msg);
	    break;

	case OM_UPDATE:
	case OM_SET:
	    /* Lay out the gadget */
	    layoutScrollG (cl, o, (struct gpRender *)msg, FALSE);

	    /* Set the attributes */
	    retval = setScrollGAttrs (cl, o, (struct opSet *)msg);

	    /* See if we should refresh */
	    if (retval && (OCLASS (o) == cl))
	    {
		struct RastPort *rp;

		/* Get a pointer to the rastport */
		if (rp = ObtainGIRPort (((struct opSet *) msg)->ops_GInfo))
		{
		    struct gpRender gpr;

		    /* Force a redraw */
		    gpr.MethodID = GM_RENDER;
		    gpr.gpr_GInfo = ((struct opSet *) msg)->ops_GInfo;
		    gpr.gpr_RPort = rp;
		    gpr.gpr_Redraw = retval; /* GREDRAW_REDRAW; */

		    /* Redraw... */
		    DM (o, &gpr);

		    /* Release the temporary rastport */
		    ReleaseGIRPort (rp);
		}

		/* Clear the return value */
		retval = 0L;
	    }
	    break;

	case GM_RENDER:
	    /* Lay out the gadget */
	    layoutScrollG (cl, o, (struct gpRender *) msg, FALSE);

	    /* Render the gadget's graphics */
	    retval = (ULONG) DSM (cl, o, msg);

	    /* Ghost the gadget if needed */
	    GhostRectangle (o, (struct gpRender *)msg, &(lod->lod_Domain));
	    break;

	case GM_GOACTIVE:
	case GM_HANDLEINPUT:
	    retval = (ULONG) DSM (cl, o, msg);
	    GetAttr (PGA_Top, o, (ULONG *)&top);
	    *(((struct gpInput *) msg)->gpi_Termination) = (0xFFFFL & top);
	    break;

	case OM_DISPOSE:
	    DM (lod->lod_Glue, msg);
	    DM ((Object *) lod->lod_UpImage, msg);
	    DM ((Object *) lod->lod_DnImage, msg);
	default:
	    retval = (ULONG) DSM (cl, o, msg);
	    break;
    }
    return (retval);
}

Tag propf[] =
{
    GTSC_Top,
    PGA_Top,
    GTSC_Total,
    PGA_Total,
    GTSC_Visible,
    PGA_Visible,
    GTSC_Overlap,
    PGA_Freedom,
    PGA_Borderless,
    GA_RightBorder,
    GA_LeftBorder,
    GA_TopBorder,
    GA_BottomBorder,
    GA_DrawInfo,
    IA_LineWidth,
};

Tag butf[] =
{
    GA_RightBorder,
    GA_LeftBorder,
    GA_TopBorder,
    GA_BottomBorder,
    GA_DrawInfo,
    IA_LineWidth,
};

/* Translate from up arrow attr. to model attr. */
static struct TagItem uparrow2model[] =
{
    {GA_ID, CGTA_Decrement},
    {TAG_END,}
};

/* Translate from down arrow attr. to model attr. */
static struct TagItem downarrow2model[] =
{
    {GA_ID, CGTA_Increment},
    {TAG_END,}
};

static struct TagItem model2gad[] =
{
    {GA_ID,		TAG_IGNORE},
    {GTSC_Top,		GTSC_Top},
    {PGA_Top,		PGA_Top},
    {GTSC_Total,	GTSC_Total},
    {PGA_Total,		PGA_Total},
    {GTSC_Visible,	GTSC_Visible},
    {PGA_Visible,	PGA_Visible},
    {GTSC_Overlap,	GTSC_Overlap},
    {PGA_Freedom,	PGA_Freedom},
    {PGA_Borderless,	PGA_Borderless},
    {GA_RightBorder,	GA_RightBorder},
    {GA_LeftBorder,	GA_LeftBorder},
    {GA_TopBorder,	GA_TopBorder},
    {GA_BottomBorder,	GA_BottomBorder},
    {GA_DrawInfo,	GA_DrawInfo},
    {IA_LineWidth,	IA_LineWidth},
    {TAG_END,}
};

/*
 * Return TRUE if I could do everything needed to initialize one of my
 * objects.
 */
ULONG initScrollGObject (Class * cl, Object * o, struct opSet * msg)
{
    struct localObjData *lod = INST_DATA (cl, o);
    WORD border = G (o)->Activation & 0xF0;
    Class *mo = (Class *) cl->cl_UserData;
    struct TagItem *tags;
    struct TagItem *orig;
    ULONG retval = FALSE;
    UWORD orient;
    Object *ic;
    Point marg;
    ULONG tag;
    UWORD as;
    LONG ad = 0;

    /* Get the gadget ID */
    lod->lod_ID = (USHORT) GetTagData (GA_ID, NULL, msg->ops_AttrList);

    /* Backup tags */
    orig = msg->ops_AttrList;
    tags = CloneTagItems (orig);

    /* Get the orientation of the scroller */
    orient = (UWORD) GetTagData (PGA_Freedom, (ULONG) FREEVERT, tags);
    lod->lod_Flags |= orient;

    /* Default arrow height */
    as = 8;
    /* Get the size of the arrows */
    if (orient & FREEHORIZ)
    {
	as = 16;
    }

    /* Set up the margins */
    tag = IA_LINEWIDTH;
    marg.x = 2;
    marg.y = 1;

    /* Adjust for border */
    if (border)
    {
	/* Use different arrows in the border */
	ad = 10;

	/* Change the line margins to match the window margins */
	marg.x = 1;

	/* Different default sizes when in the border */
	as = 9;
	if (orient & FREEHORIZ)
	{
	    as = 18;
	}
    }

    /* See if they overwrote our defaults */
    as = (UWORD) GetTagData (GTSC_Arrows, (ULONG) as, tags);

    /* We have to have a model for this. */
    if (lod->lod_Glue = NewObject (mo, NULL,
				      GA_ID, (ULONG) lod->lod_ID,
				      TAG_DONE))
    {
	ULONG upn, dnn;

	if (lod->lod_Flags & FREEHORIZ)
	{
	    lod->lod_W = as;
	    lod->lod_H = G (o)->Height;
	    upn = LEFTIMAGE - ad;
	    dnn = RIGHTIMAGE - ad;
	}
	else
	{
	    lod->lod_W = G (o)->Width;
	    lod->lod_H = as;
	    upn = UPIMAGE - ad;
	    dnn = DOWNIMAGE - ad;
	}

	/* Create the 'up' arrow image */
	lod->lod_UpImage = (struct Image *)
	      NewObject (NULL, "appsysiclass",
			 SYSIA_Which, upn,
			 TAG_END);

	/* Create the 'down' arrow image */
	lod->lod_DnImage = (struct Image *)
	      NewObject (NULL, "appsysiclass",
			 SYSIA_Which, dnn,
			 TAG_END);

	/* Pass through slider tags */
	if (FilterTagItems (tags, propf, TAGFILTER_AND) >= 0)
	{
	    /* Create the slider gadget */
	    if (lod->lod_Slider = (struct Gadget *)
#if 0
		NewObject (NULL, PROPGCLASS,
#else
		NewObject (NULL, "slidergclass",
#endif
			   GA_ID, 12010L,
			   ICA_TARGET, lod->lod_Glue,
			   tag, (ULONG) & marg,
			   ICA_MAP, model2gad,
			   TAG_MORE, tags,
			   TAG_END))
	    {
		/* add it to the group ... */
		DoMethod (o, OM_ADDMEMBER, (LONG) lod->lod_Slider);

		/* ... and get an ic for the slider gadget */
		if (ic = (Object *) NewObject (NULL, ICCLASS,
					       ICA_TARGET, lod->lod_Slider,
					       ICA_MAP, model2gad,
					       TAG_END))
		{
		    /* ... and add it to the model gadget */
		    DoMethod (lod->lod_Glue, OM_ADDMEMBER, (LONG) ic);
		}
	    }

	    /* Refresh the tag list */
	    RefreshTagItemClones (tags, orig);
	}

	/* Pass through button tags */
	if (FilterTagItems (tags, butf, TAGFILTER_AND) >= 0)
	{
	    /* Up arrow gadget */
	    lod->lod_UpArrow = (struct Gadget *)
	      NewObject (NULL, "actiongclass",
			 GA_LabelImage, lod->lod_UpImage,
			 GA_Immediate, TRUE,
			 GA_RelVerify, TRUE,
			 GA_ID, 12020L,
			 tag, (ULONG) & marg,
			 ICA_TARGET, lod->lod_Glue,
			 ICA_MAP, uparrow2model,
			 TAG_MORE, tags,
			 TAG_END);
	    /* ... and add it to the model gadget */
	    DoMethod (o, OM_ADDMEMBER, (LONG) lod->lod_UpArrow);

	    /* Down arrow gadget */
	    lod->lod_DnArrow = (struct Gadget *)
	      NewObject (NULL, "actiongclass",
			 GA_LabelImage, lod->lod_DnImage,
			 GA_Immediate, TRUE,
			 GA_RelVerify, TRUE,
			 GA_ID, 12030L,
			 tag, (ULONG) & marg,
			 ICA_TARGET, lod->lod_Glue,
			 ICA_MAP, downarrow2model,
			 TAG_MORE, tags,
			 TAG_END);

	    /* ... and add it to the model gadget */
	    DoMethod (o, OM_ADDMEMBER, (LONG) lod->lod_DnArrow);

	    /* Refresh the tag list */
	    RefreshTagItemClones (tags, orig);
	}

	retval = TRUE;
    }

    /* Free temporary tag array */
    FreeTagItems (tags);

    /* Preset the domain */
    lod->lod_Domain.Left = lod->lod_Domain.Top = NOT_SET;
    lod->lod_Domain.Width = lod->lod_Domain.Height = NOT_SET;

    /* Set the attributes */
    setScrollGAttrs (cl, o, msg);

    return (retval);
}

VOID layoutScrollG (Class * cl, Object * o, struct gpRender * gpr, BOOL force)
{
    struct localObjData *lod = INST_DATA (cl, o);
    struct IBox obox = *(&(lod->lod_Domain));
    struct IBox *box = &(lod->lod_Domain);
    WORD border = G(o)->Activation & 0xF0;
    struct GadgetInfo *gi;

    /* Make sure we have all the information we need to layout */
    if (gi = GetGadgetInfo (gpr))
    {
	/* Compute the domain of the entire gadget */
	gadgetBox (G(o), &(gi->gi_Domain), box, NULL, NULL);

	/* Make sure we should change things... */
	if (force || !(compareRect (&obox, box)))
	{
	    LONG tx = (LONG) box->Left;
	    LONG ty = (LONG) box->Top;
	    LONG w = (LONG) box->Width;
	    LONG h = (LONG) box->Height;
	    LONG aw = (LONG) lod->lod_W;	/* UpImage->Width; */
	    LONG ah = (LONG) lod->lod_H;	/* UpImage->Height; */
	    LONG sx, sy, sw, sh;
	    LONG ux, uy, uw, uh;
	    LONG dx, dy, dw, dh;

	    if (lod->lod_Flags & FREEHORIZ)
	    {
		/* Calculate slider rectangle */
		sx = tx;
		sy = (border) ? (ty + 2) : ty;
		sh = (border) ? (h - 4) : h;
		sw = (border) ? (w - (aw * 2L) - 2) : (w - aw * 2L);

		/* Calculate the up arrow rectangle */
		ux = tx + w - (aw * 2);
		uy = ty;
		uw = aw;
		uh = h;

		/* Calculate the down arrow rectangle */
		dx = tx + w - aw;
		dy = ty;
		dw = aw;
		dh = h;
	    }
	    else
	    {
		/* Calculate slider rectangle */
		sx = (border) ? (tx + 3) : tx;
		sy = ty;
		sw = (border) ? (w - 6) : w;
		sh = (border) ? (h - (ah * 2L) - 1) : (h - (ah * 2L));

		/* Calculate up arrow */
		ux = tx;
		uy = ty + h - (ah * 2);
		uw = w;
		uh = ah;

		/* Calculate down arrow */
		dx = tx;
		dy = ty + h - ah;
		dw = w;
		dh = ah;
	    }

	    /* Set the proportional slider rectangle */
	    SetAttrs (lod->lod_Slider,
				GA_LEFT, sx,
				GA_TOP, sy,
				GA_WIDTH, sw,
				GA_HEIGHT, sh,
				TAG_DONE);

	    /* Set the up arrow rectangle */
	    SetAttrs (lod->lod_UpArrow,
				GA_LEFT, ux,
				GA_TOP, uy,
				GA_WIDTH, uw,
				GA_HEIGHT, uh,
				TAG_DONE);

	    /* Set the down arrow rectangle */
	    SetAttrs (lod->lod_DnArrow,
				GA_LEFT, dx,
				GA_TOP, dy,
				GA_WIDTH, dw,
				GA_HEIGHT, dh,
				TAG_DONE);
	}
    }
}

/* These are attributes that the model understands */
static Tag model_attrs[] =
{
    GTSC_Total,
    GTSC_Visible,
    GTSC_Top,
    PGA_Total,
    PGA_Visible,
    PGA_Top,
    GA_ID,
    ICA_TARGET,
    ICA_MAP,
    TAG_END,
};

ULONG getScrollGAttrs (Class * cl, Object * o, struct opGet * msg)
{
    struct localObjData *lod = INST_DATA (cl, o);

    /* See if it is a request for a model attribute */
    if (TagInArray (msg->opg_AttrID, model_attrs))
    {
	return (DM (lod->lod_Glue, msg));
    }

    /* Send it to the group class */
    return (DSM (cl, o, msg));
}

static struct TagItem activation_bools[] =
{
    {GA_EndGadget, ENDGADGET},
    {GA_Immediate, GADGIMMEDIATE},
    {GA_RelVerify, RELVERIFY},
    {GA_FollowMouse, FOLLOWMOUSE},
    {GA_RightBorder, RIGHTBORDER},
    {GA_LeftBorder, LEFTBORDER},
    {GA_TopBorder, TOPBORDER},
    {GA_BottomBorder, BOTTOMBORDER},
    {GA_ToggleSelect, TOGGLESELECT},
    {TAG_END}
};

ULONG setScrollGAttrs (Class * cl, Object * o, struct opSet * msg)
{
    struct localObjData *lod = INST_DATA (cl, o);
    struct TagItem *tags = msg->ops_AttrList;
    struct TagItem *origtags;
    struct TagItem *tstate;
    struct TagItem *tag;
    ULONG tidata;
    ULONG retval = 0;

    /* Set aside the tags so that we can properly send them out */
    origtags = msg->ops_AttrList;
    msg->ops_AttrList = CloneTagItems (origtags);

    /* Get the activation tags */
    G(o)->Activation = PackBoolTags (G (o)->Activation, origtags, activation_bools);

    /* Handle model attributes first */
    if (FilterTagItems (msg->ops_AttrList, model_attrs, TAGFILTER_AND))
    {
	if (msg->MethodID == OM_NEW)
	{
	    DoMethod (lod->lod_Glue, OM_SET, (LONG) msg->ops_AttrList, (LONG) msg->ops_GInfo);
	}
	else
	{
	    DM (lod->lod_Glue, msg);
	}
    }

    /* If this is a new, then pass along the attributes */
    if (msg->MethodID != OM_NEW)
    {
	/* re-clone, without re-allocating */
	RefreshTagItemClones (msg->ops_AttrList, origtags);
	if (FilterTagItems (msg->ops_AttrList, model_attrs, TAGFILTER_NOT))
	{
	    /* Perhaps there is a need to refresh the group gadget now. */
	    retval += DSM (cl, o, msg);
	}
    }

    /* free clone and restore original */
    FreeTagItems (msg->ops_AttrList);
    msg->ops_AttrList = origtags;

    /* Process rest */
    tstate = tags;
    while (tag = NextTagItem (&tstate))
    {
	tidata = tag->ti_Data;
	switch (tag->ti_Tag)
	{
	    case GA_WIDTH:
	    case GA_RELWIDTH:
		G (o)->Width = (WORD) tidata;
		break;

	    case GA_HEIGHT:
	    case GA_RELHEIGHT:
		G (o)->Height = (WORD) tidata;
		break;

	    case GA_LEFT:
	    case GA_RELRIGHT:
		G (o)->LeftEdge = (WORD) tidata;
		break;

	    case GA_TOP:
	    case GA_RELBOTTOM:
		G (o)->TopEdge = (WORD) tidata;
		break;

	    case GA_DISABLED:
		retval = 1;
		break;
	}
    }

    return (retval);
}
