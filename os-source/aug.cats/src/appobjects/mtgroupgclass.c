/*
 * mtgroupgclass.c multiline text gadget group class
 */

#include "appobjectsbase.h"

#define G(o) 	((struct Gadget *) (o))

/* public class */
#define MYCLASSID	"mtextggclass"
#define SUPERCLASSID	"lgroupgclass"

/* mygroupgclass.c */
Class *initMTGroupClass (VOID);
ULONG freeMTGroupClass (Class * cl);
ULONG __saveds dispatchMTGroup (Class * cl, Object * o, Msg msg);
ULONG renderGroupG (Class * cl, Object * o, struct gpRender * gpr);
ULONG initMTGroupObject (Class * cl, Object * o, struct opSet * msg);
ULONG getMTGroupAttrs (Class * cl, Object * o, struct opGet * msg);
ULONG setMTGroupAttrs (Class * cl, Object * o, struct opSet * msg);
VOID layoutGroup (Class * cl, Object * o, struct gpRender * gpr, BOOL);
BOOL compareRect (struct IBox * b1, struct IBox * b2);

#define	SCROLL_WIDTH	16

struct localObjData
{
    struct IBox lod_Domain;	/* Domain of the gadget */

    /* Other objects allocated */
    Object *lod_Model;
    Object *lod_Frame;
    struct Gadget *lod_MText;
    struct Gadget *lod_Scroller;

    struct
    {
	WORD V;
	WORD H;
    } lod_Margins;		/* Margins for frame */
};

Class *
initMTGroupClass (VOID)
{
    struct LibBase *base = ((struct ExtLibrary *) AppObjectsBase)->el_Base;
    ULONG __saveds dispatchMTGroup ();
    ULONG hookEntry ();
    Class *cl;

    if (cl = MakeClass (MYCLASSID, SUPERCLASSID, NULL,
			sizeof (struct localObjData), 0))
    {
	/* initialize the cl_Dispatcher Hook */
	cl->cl_Dispatcher.h_Entry = hookEntry;
	cl->cl_Dispatcher.h_SubEntry = dispatchMTGroup;
	cl->cl_Dispatcher.h_Data = (VOID *) 0xFACE;	/* unused */

	/* remember the model class */
	cl->cl_UserData = (ULONG) base->lb_Classes[CGTC_MTMODEL];

	/* Make the class public */
	AddClass (cl);
    }

    return (cl);
}

ULONG
freeMTGroupClass (Class * cl)
{

    return ((ULONG) FreeClass (cl));
}

ULONG __saveds dispatchMTGroup (Class * cl, Object * o, Msg msg)
{
    struct localObjData *lod = INST_DATA (cl, o);
    Object *newobj;
    ULONG retval;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    if (newobj = (Object *) DSM (cl, o, msg))
	    {
		lod = INST_DATA (cl, newobj);

		if (initMTGroupObject (cl, newobj, (struct opSet *) msg))
		{
		    /* Get the margins of the frame */
		    DoMethod (lod->lod_Frame, OM_GET, IA_LINEWIDTH, &(lod->lod_Margins));

		    /* Allow the first layout to work */
		    lod->lod_Domain.Left = NOT_SET;
		    lod->lod_Domain.Top = NOT_SET;
		    lod->lod_Domain.Width = NOT_SET;
		    lod->lod_Domain.Height = NOT_SET;

		    /* Set the attributes */
		    setMTGroupAttrs (cl, newobj, (struct opSet *) msg);
		}
		else
		{
		    /* free what's there if failure	 */
		    CoerceMethod (cl, newobj, OM_DISPOSE);
		    newobj = NULL;
		}
	    }

	    retval = (ULONG) newobj;
	    break;

	case OM_GET:
	    retval = getMTGroupAttrs (cl, o, (struct opGet *) msg);
	    break;

	case OM_UPDATE:
	case OM_SET:
	    retval = setMTGroupAttrs (cl, o, (struct opSet *) msg);

	    /* See if we should refresh */
	    if (retval && (OCLASS (o) == cl))
	    {
		struct RastPort *rp;

		/* Get a pointer to the rastport */
		if (rp = ObtainGIRPort (((struct opSet *) msg)->ops_GInfo))
		{
		    struct gpRender gpr =
		    {NULL};

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

	case GM_RENDER:
	    /* Configure the gadget */
	    layoutGroup (cl, o, (struct gpRender *) msg, FALSE);

	    /* Render the gadget's graphics */
	    retval = renderGroupG (cl, o, (struct gpRender *) msg);
	    break;

	case OM_DISPOSE:
	    /* dispose of components	 */
	    DM (lod->lod_Model, msg);
	    DM (lod->lod_Frame, msg);

	    /* dispose self and component gadgets */
	    retval = (ULONG) DSM (cl, o, msg);
	    break;

	    /* Pass it on to the superclass */
	default:
	    retval = (ULONG) DSM (cl, o, msg);
	    break;

    }

    return (retval);
}

enum gadgetids
{
    gSlider = 1,
    gString,
    gGroup,
};

Tag string_attrs[] =
{
    STRINGA_MaxChars,
    STRINGA_Buffer,
    STRINGA_UndoBuffer,
    STRINGA_WorkBuffer,
    STRINGA_TextVal,
    STRINGA_BufferPos,
    STRINGA_DispPos,
    STRINGA_AltKeyMap,
    STRINGA_Font,
    STRINGA_Pens,
    STRINGA_ActivePens,
    STRINGA_EditHook,
    CGTA_HighPens,
    CGTA_DisplayOnly,
    CGTA_PrevField,
    CGTA_NextField,
    GA_TabCycle,
    TAG_END,
};

struct TagItem string2model[] =
{
    {CGTA_Visible, PGA_Visible},
    {CGTA_Total, PGA_Total},
    {CGTA_Top, PGA_Top},
    {TAG_DONE,}
};

struct TagItem model2string[] =
{
    {PGA_Visible, CGTA_Visible},
    {PGA_Total, CGTA_Total},
    {PGA_Top, CGTA_Top},
    {TAG_DONE,}
};

/*
 * Return TRUE if I could do everything needed to initialize one of my
 * objects.
 */
ULONG initMTGroupObject (Class * cl, Object * o, struct opSet * msg)
{
    struct localObjData *lod = INST_DATA (cl, o);
    struct TagItem *tags = msg->ops_AttrList;
    struct TagItem *string_tags;
    struct DrawInfo *drinfo;
    ULONG retval = FALSE;
    Point margins;
    Object *ic;

    drinfo = (struct DrawInfo *) GetTagData (GA_DrawInfo, NULL, tags);

    /* Allocate a model */
    if (lod->lod_Model = NewObject ((struct IClass *) cl->cl_UserData, NULL, TAG_END))
    {
	/* Set the margins */
	margins.x = 2;
	margins.y = 1;

	/* Create the border */
	if (lod->lod_Frame = NewObject (NULL, "aframeiclass",
					IA_EDGESONLY, TRUE,
					IA_DOUBLEEMBOSS, TRUE,
					IA_LINEWIDTH, (ULONG) &margins,
					GA_DrawInfo, drinfo,
					TAG_END))
	{
	    /* Clone the string tags */
	    if (string_tags = CloneTagItems (tags))
	    {
		ULONG tagm = TAG_IGNORE;

		/* Just get the multiline text attributes */
		if (FilterTagItems (string_tags, string_attrs, TAGFILTER_AND))
		{
		    tagm = TAG_MORE;
		}

		/* Create the scroller */
		if (lod->lod_Scroller = (struct Gadget *)
		      NewObject (NULL, "scrollgclass",
				ICA_TARGET, lod->lod_Model,
				GA_Width, SCROLL_WIDTH,
				GA_DrawInfo, drinfo,
				GA_ID, 200,
				PGA_Freedom, FREEVERT,
				PGA_Top, 0L,
				PGA_Visible, 4L,
				PGA_Total, 4L,
				PGA_Borderless, FALSE,
				TAG_END))
		{
		    /* add it to the group ... */
		    DoMethod (o, OM_ADDMEMBER, (LONG) lod->lod_Scroller);

		    /* ... and get an ic for the scroll gadget */
		    ic = (Object *) NewObject (NULL, ICCLASS,
					       ICA_TARGET, lod->lod_Scroller,
					       TAG_END);

		    DoMethod (lod->lod_Model, OM_ADDMEMBER, (LONG) ic);

		/* Create the gadget */
		if (lod->lod_MText = (struct Gadget *)
		    NewObject (NULL, "mtextgclass",
				ICA_TARGET, lod->lod_Model,
				ICA_MAP, string2model,
				GA_DrawInfo, drinfo,
				GA_ID, 100,
				tagm, string_tags,
				TAG_END))
		{
		    /* Set the special information field to point to our info */
		    G (o)->SpecialInfo = lod->lod_MText->SpecialInfo;

		    /* Show that we honor a string extension */
		    G (o)->Activation |= STRINGEXTEND;

		    /* Add the gadget to the ourself */
		    DoMethod (o, OM_ADDMEMBER, (LONG) lod->lod_MText);

		    /* Create the interconnect glue */
		    ic = (Object *) NewObject (NULL, ICCLASS,
					       ICA_TARGET, lod->lod_MText,
					       ICA_MAP, model2string,
					       TAG_END);

		    /* Add the glue */
		    DoMethod (lod->lod_Model, OM_ADDMEMBER, (LONG) ic);

		    /* Tell who the master of this group is */
		    SetAttrs (o, CGTA_Master, lod->lod_MText, TAG_DONE);

		    retval = TRUE;
		}
		}

		/* Free clone */
		FreeTagItems (string_tags);

	    }
	}
    }

    return (retval);
}

/* These are attributes that the model understands */
Tag model_attrs[] =
{
    CGTA_Total,
    CGTA_Visible,
    CGTA_Top,
    ICA_TARGET,
    ICA_MAP,
    TAG_END,
};

ULONG getMTGroupAttrs (Class * cl, Object * o, struct opGet * msg)
{
    struct localObjData *lod = INST_DATA (cl, o);

    /* See if it is a request for a model attribute */
    if (TagInArray (msg->opg_AttrID, model_attrs))
    {
	return (DM (lod->lod_Model, msg));
    }

    /* See if it is a direct request to the text gadget */
    if (TagInArray (msg->opg_AttrID, string_attrs))
    {
	/* Ask the text gadget for the attribute */
	return ((ULONG) DM((Object *)lod->lod_MText, msg));
    }

    /* Send it to the group class */
    return ((ULONG) DSM (cl, o, msg));
}

/*
 * This function is called for OM_NEW, OM_SET, and OM_UPDATE. What we do is
 * delegate the processing of all the attributes that our model will
 * understand to it, including the target/map.  These last means that the
 * model will be directly connected to the outside world instead of directly
 * our gadget itself.
 *
 * In the case of OM_NEW, I've already created the model with no attributes
 * passed, now I have to convert the OM_NEW message into an OM_SET message so
 * we don't go create another model object.
 *
 * Also, since the superclass has already seen the OM_NEW message and
 * attributes, we don't pass it along again here, in that case.
 */

ULONG setMTGroupAttrs (Class * cl, Object * o, struct opSet * msg)
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

    /* Handle model attributes first */
    if (FilterTagItems (msg->ops_AttrList, model_attrs, TAGFILTER_AND))
    {

	/*
	 * Pass along same message, with filtered attrlist This message will be
	 * mapped and propagated to all component gadgets, who will refresh
	 * themselves if needed.  There's no additional need to refresh the
	 * group gadget.
	 *
	 * For the case of OM_NEW, we have to convert to OM_SET, since we've
	 * previously created the thing.
	 */

	if (msg->MethodID == OM_NEW)
	{
	    DoMethod (lod->lod_Model, OM_SET, (LONG) msg->ops_AttrList, (LONG) msg->ops_GInfo);
	}
	else
	{

	    /*
	     * pass along OM_SET or OM_UPDATE message as is (with mapped tags).
	     */
	    DM (lod->lod_Model, msg);
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
	/* re-clone, without re-allocating */
	RefreshTagItemClones (msg->ops_AttrList, origtags);
	if (FilterTagItems (msg->ops_AttrList, string_attrs, TAGFILTER_AND))
	{
	    /* Perhaps there is a need to refresh the group gadget now. */
	    retval += (ULONG) DM ((Object *)lod->lod_MText, msg);
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
	    case CGTA_DisplayOnly:
		if (tidata)
		{
		    SetAttrs (lod->lod_Frame,
			      IA_DOUBLEEMBOSS, FALSE,
			      IA_RECESSED, TRUE,
			      TAG_DONE);
		}
		else
		{
		    SetAttrs (lod->lod_Frame,
			      IA_DOUBLEEMBOSS, TRUE,
			      IA_RECESSED, FALSE,
			      TAG_DONE);
		}
		/* Get the margins of the frame */
		DoMethod (lod->lod_Frame, OM_GET, IA_LINEWIDTH, &(lod->lod_Margins));
		retval = 1;
		break;

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

VOID layoutGroup (Class * cl, Object * o, struct gpRender * gpr, BOOL force)
{
    struct localObjData *lod = INST_DATA (cl, o);
    struct GadgetInfo *gi = gpr->gpr_GInfo;
    struct IBox obox = *(&(lod->lod_Domain));
    struct IBox *box = &(lod->lod_Domain);

    /* Compute the domain of the entire gadget */
    computeDomain (cl, o, gpr, box, NULL, NULL);

    /* Make sure we should change things... */
    if (force || !(compareRect (&obox, box)))
    {
	LONG tx = (LONG) box->Left;
	LONG ty = (LONG) box->Top;
	LONG w  = (LONG) box->Width;
	LONG h  = (LONG) box->Height;
	LONG aw, vm, hm;

	aw = (LONG) SCROLL_WIDTH;	/* lod->lod_Scroller->Width; */
	vm = (LONG) lod->lod_Margins.V;
	hm = (LONG) lod->lod_Margins.H;

	/* Set the proportional slider rectangle */
	SetGadgetAttrs (lod->lod_Scroller, gi->gi_Window, gi->gi_Requester,
		  GA_LEFT, (tx + w - aw),
		  GA_TOP, ty,
		  GA_WIDTH, aw,
		  GA_HEIGHT, h,
		  TAG_DONE);

	/* Set the multi-line text gadget rectangle */
	SetGadgetAttrs (lod->lod_MText, gi->gi_Window, gi->gi_Requester,
		  GA_LEFT, (tx + vm + (vm / 2)),
		  GA_TOP, (ty + hm + (hm / 2)),
		  GA_WIDTH, (w - aw - (vm * 2L) - (vm / 2)),
		  GA_HEIGHT, (h - (hm * 2L) - (hm / 2) - 1),
		  TAG_DONE);
    }
}

ULONG
renderGroupG (Class * cl, Object * o, struct gpRender * gpr)
{
    struct localObjData *lod = INST_DATA (cl, o);
    struct IBox *box = &(lod->lod_Domain);
    struct RastPort *rp = gpr->gpr_RPort;
    struct impDraw imp;
    LONG state;

    /* Clear the whole block */
    if ((gpr->MethodID == GM_RENDER) &&
	(gpr->gpr_Redraw == GREDRAW_REDRAW))
    {
	/* Ghosting is done with the shadow pen */
	SetAPen (rp, gpr->gpr_GInfo->gi_DrInfo->dri_Pens[BACKGROUNDPEN]);

	/* Allow background to show through */
	SetDrMd (rp, JAM1);

	/* Overlay the ghosting over the gadget visuals */
	RectFill (rp, box->Left, box->Top,
		  (box->Left + box->Width - 1),
		  (box->Top + box->Height - 1));
    }

    /* Let the superclass do it's stuff first */
    DSM (cl, o, gpr);

    /* Get the gadget's visual state */
    state = GetVisualState (G (o), gpr->gpr_GInfo);

    /* Build up the common part of the message */
    imp.imp_RPort = rp;
    imp.imp_State = state;
    imp.imp_DrInfo = gpr->gpr_GInfo->gi_DrInfo;

    /* Build up the frame portion of the message */
    imp.MethodID = IM_DRAWFRAME;
    imp.imp_Offset.X = box->Left;
    imp.imp_Offset.Y = box->Top;
    imp.imp_Dimensions.Width = (box->Width - SCROLL_WIDTH);
    imp.imp_Dimensions.Height = box->Height;

    /* Send the message */
    DM (lod->lod_Frame, &imp);

    if (state == IDS_DISABLED)
    {
	UWORD pattern[] = {0x4444, 0x1111, 0x4444, 0x1111};

	/* Ghosting is done with the shadow pen */
	SetAPen (rp, gpr->gpr_GInfo->gi_DrInfo->dri_Pens[SHADOWPEN]);

	/* Allow background to show through */
	SetDrMd (rp, JAM1);

	/* Turn on the special ghosting pattern */
	SetAfPt (rp, pattern, 2);

	/* Overlay the ghosting over the gadget visuals */
	RectFill (rp, box->Left, box->Top,
		  (box->Left + box->Width - 1),
		  (box->Top + box->Height - 1));
    }

    return (1L);
}

BOOL compareRect (struct IBox * b1, struct IBox * b2)
{

    if ((b1->Left == b2->Left) &&
	(b1->Top == b2->Top) &&
	(b1->Width == b2->Width) &&
	(b1->Height == b2->Height))
    {
	return (TRUE);
    }
    else
    {
	return (FALSE);
    }
}
