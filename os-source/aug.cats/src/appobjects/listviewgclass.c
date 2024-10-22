/* lvclass.c --- ListView gadget class
 * Copyright (C) 1991 Commodore-Amiga, Inc.
 * All Rights Reserved Worldwide
 * Written by David N. Junod
 *
 */

#include "appobjectsbase.h"
#include "appobjects_protos.h"

#define	G(x)	((struct Gadget *)(x))
#define	SI(x)	((struct LVSpecialInfo *)(x))

extern VOID NewList (struct List *);
extern struct Library *GfxBase, *IntuitionBase, *UtilityBase;

/* Public ListView class */
#define MYCLASSID	"listviewgclass"
#define SUPERCLASSID	"lgroupgclass"

Class *initListViewMClass (VOID);
ULONG freeListViewMClass (Class * cl);

/* lvmclass.c */
Class *initListViewGClass (VOID);
ULONG freeListViewGClass (Class * cl);
ULONG __saveds dispatchListViewG (Class * cl, Object * o, Msg msg);
ULONG initializeListViewG (Class * cl, Object * o, struct opSet * msg);
ULONG getListViewGAttrs (Class * cl, Object * o, struct opGet * msg);
ULONG setListViewGAttrs (Class * cl, Object * o, struct opSet * msg);
ULONG renderListViewG (Class * cl, Object * o, struct gpRender * gpr);
ULONG handleListViewG (Class * cl, Object * o, struct gpInput * msg);
VOID layoutListViewG (Class * cl, Object * o, struct gpRender * gpr, BOOL);

#define	OBJ_GLUE	0
#define	OBJ_VSCROLL	1
#define	OBJ_HSCROLL	2
#define	OBJ_VIEW	3
#define	MAX_OBJ		4

struct localObjData
{
    struct IBox lod_Domain;		/* Domain of the gadget */
    ULONG lod_Flags;			/* See defines below (and appobjectsbase) */
    Object *lod_Obj[MAX_OBJ];		/* Components */
    UWORD lod_VSize;			/* Size of vertical scroller */
    UWORD lod_HSize;			/* Size of horizontal scroller */
    struct LVSpecialInfo *lod_SI;	/* Pointer to view's SpecialInfo */
};

#define	LODSIZE	(sizeof(struct localObjData))

static struct TagItem modelgad[] =
{
    {GA_ID, TAG_IGNORE},

    {AOLV_TotalVert, AOLV_TotalVert},
    {AOLV_VisibleVert, AOLV_VisibleVert},
    {AOLV_TopVert, AOLV_TopVert},

    {AOLV_TotalHoriz, AOLV_TotalHoriz},
    {AOLV_VisibleHoriz, AOLV_VisibleHoriz},
    {AOLV_TopHoriz, AOLV_TopHoriz},

    {AOLV_SpecialInfo, AOLV_SpecialInfo},

    {AOLV_UnitWidth, AOLV_UnitWidth},
    {AOLV_UnitHeight, AOLV_UnitHeight},
    {GTLV_Selected, GTLV_Selected},
    {GTLV_Labels, GTLV_Labels},
    {AOLV_List, AOLV_List},

    {AOLV_View, AOLV_View},
    {AOLV_LabelHeight, AOLV_LabelHeight},
    {AOLV_ControlHeight, AOLV_ControlHeight},

    {TAG_END,}
};

static Tag view[] =
{
    AOLV_Sizable,
    AOLV_Draggable,
    AOLV_Borderless,
    AOLV_Freedom,
    AOLV_UnitWidth,
    AOLV_UnitHeight,
    AOLV_ReadOnly,
    AOLV_MultiSelect,
    AOLV_SpecialInfo,
    AOLV_TextAttr,
    GTLV_Selected,
    AOLV_TopVert,
    AOLV_VisibleVert,
    AOLV_TotalVert,
    AOLV_TopHoriz,
    AOLV_VisibleHoriz,
    AOLV_TotalHoriz,
    GTLV_Labels,
    AOLV_List,
    AOLV_Position,
    AOLV_ColumnWidth,
    AOLV_Title,
    AOLV_Justification,
    AOLV_FieldOffset,
    AOLV_FieldType,
    AOLV_ControlHeight,
    AOLV_LabelHeight,
    AOLV_View,
    TAG_DONE,
};

static Tag vprop[] =
{
    AOLV_TopVert,
    AOLV_VisibleVert,
    AOLV_TotalVert,
    GA_DrawInfo,
    TAG_DONE,
};

static Tag hprop[] =
{
    AOLV_TopHoriz,
    AOLV_VisibleHoriz,
    AOLV_TotalHoriz,
    GA_DrawInfo,
    TAG_DONE,
};

static struct TagItem vpropm[] =
{
    {AOLV_TopVert, PGA_Top},
    {AOLV_VisibleVert, PGA_Visible},
    {AOLV_TotalVert, PGA_Total},
    {GA_ID, TAG_IGNORE},
    TAG_DONE,
};

static struct TagItem hpropm[] =
{
    {AOLV_TopHoriz, PGA_Top},
    {AOLV_VisibleHoriz, PGA_Visible},
    {AOLV_TotalHoriz, PGA_Total},
    {GA_ID, TAG_IGNORE},
    TAG_DONE,
};

static struct TagItem mpropv[] =
{
    {PGA_Top, AOLV_TopVert},
    {PGA_Visible, TAG_IGNORE},
    {PGA_Total, TAG_IGNORE},
    {GA_ID, TAG_IGNORE},
    TAG_DONE,
};

static struct TagItem mproph[] =
{
    {PGA_Top, AOLV_TopHoriz},
    {PGA_Visible, TAG_IGNORE},
    {PGA_Total, TAG_IGNORE},
    {GA_ID, TAG_IGNORE},
    TAG_DONE,
};

/* These are attributes that the model understands */
static Tag model_attrs[] =
{
    AOLV_TotalVert,
    AOLV_VisibleVert,
    AOLV_TopVert,

    AOLV_TotalHoriz,
    AOLV_VisibleHoriz,
    AOLV_TopHoriz,

    AOLV_SpecialInfo,

    AOLV_UnitWidth,
    AOLV_UnitHeight,
    GTLV_Selected,
    GTLV_Labels,
    AOLV_List,

    AOLV_View,
    AOLV_LabelHeight,
    AOLV_ControlHeight,

    ICA_TARGET,
    ICA_MAP,
    TAG_END,
};

static struct TagItem activation_bools[] =
{
    {GA_EndGadget, ENDGADGET},
    {GA_Immediate, GADGIMMEDIATE},
    {GA_RelVerify, RELVERIFY},
    {GA_FollowMouse, FOLLOWMOUSE},
    {TAG_END}
};

Class *initListViewGClass (VOID)
{
    ULONG hookEntry ();
    Class *sm, *cl;

    if (sm = initListViewMClass ())
    {
	if (cl = MakeClass (MYCLASSID, SUPERCLASSID, NULL, LODSIZE, 0))
	{
	    /* initialize the cl_Dispatcher Hook */
	    cl->cl_Dispatcher.h_Entry = hookEntry;
	    cl->cl_Dispatcher.h_SubEntry = dispatchListViewG;

	    /* Remember our support model class */
	    cl->cl_UserData = (ULONG) sm;

	    /* Make the class public */
	    AddClass (cl);

	    return (cl);
	}

	freeListViewMClass (sm);
    }

    return (NULL);
}

ULONG freeListViewGClass (Class * cl)
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

ULONG __saveds dispatchListViewG (Class * cl, Object * o, Msg msg)
{
    struct localObjData *lod;
    ULONG retval;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    if (retval = DSM (cl, o, msg))
	    {
		/* Initialize the complete set */
		if (!(initializeListViewG (cl, (Object *) retval, (struct opSet *) msg)))
		{
		    /* free what's there if failure	 */
		    CoerceMethod (cl, (Object *) retval, OM_DISPOSE);
		    return (NULL);
		}
	    }
	    break;

	case OM_GET:
	    retval = getListViewGAttrs (cl, o, (struct opGet *) msg);
	    break;

	case OM_UPDATE:
	case OM_SET:
	    /* Configure the gadget */
	    layoutListViewG (cl, o, (struct gpRender *) msg, FALSE);

	    retval = setListViewGAttrs (cl, o, (struct opSet *) msg);

	    /* See if we should refresh */
	    if (retval && (OCLASS (o) == cl))
	    {
		struct RastPort *rp;

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

	case LV_ADDCOLUMN:
	case LV_REMCOLUMN:
	    lod = INST_DATA (cl, o);
	    retval = 0L;
	    if (lod->lod_Obj[OBJ_VIEW])
		retval = DM (lod->lod_Obj[OBJ_VIEW], msg);
	    break;

	case GM_RENDER:
	    /* Configure the gadget */
	    layoutListViewG (cl, o, (struct gpRender *) msg, FALSE);

	default:
	    retval = (ULONG) DSM (cl, o, msg);
	    break;
    }

    return (retval);
}

/* Return TRUE if I could do everything needed to initialize one of my objects. */
ULONG initializeListViewG (Class * cl, Object * o, struct opSet * msg)
{
    struct localObjData *lod = INST_DATA (cl, o);
    Class *mo = (Class *) cl->cl_UserData;
    struct LVSpecialInfo *si;
    struct TagItem *tags;
    ULONG retval = FALSE;
    UWORD orient;
    Object *ic;
    Point marg;
    LONG id;

    if (tags = CloneTagItems (msg->ops_AttrList))
    {
	id = GetTagData (GA_ID, NULL, tags);
	if (lod->lod_Obj[OBJ_GLUE] = NewObject (mo, NULL, GA_ID, id, TAG_DONE))
	{
	    lod->lod_HSize = 9;
	    lod->lod_VSize = 16;
	    marg.x = 2;
	    marg.y = 1;
	    if (FilterTagItems (tags, view, TAGFILTER_AND) >= 0)
	    {
		if (lod->lod_Obj[OBJ_VIEW] = NewObject (NULL, (UBYTE *) "viewgclass",
							GA_ID, id,
							ICA_TARGET, lod->lod_Obj[OBJ_GLUE],
							TAG_MORE, tags,
							TAG_END))
		{
		    lod->lod_SI = SI (G (lod->lod_Obj[OBJ_VIEW])->SpecialInfo);
		    G (o)->SpecialInfo = si = lod->lod_SI;
		    SetAttrs (lod->lod_Obj[OBJ_GLUE], AOLV_SpecialInfo, si, TAG_DONE);
		    DoMethod (o, OM_ADDMEMBER, (LONG) lod->lod_Obj[OBJ_VIEW]);
		    if (ic = (Object *) NewObject (NULL, (UBYTE *) ICCLASS,
						   ICA_TARGET, lod->lod_Obj[OBJ_VIEW],
						   ICA_MAP, modelgad,
						   TAG_END))
		    {
			DoMethod (lod->lod_Obj[OBJ_GLUE], OM_ADDMEMBER, (LONG) ic);
		    }
		}
		RefreshTagItemClones (tags, msg->ops_AttrList);
	    }

	    if (!lod->lod_Obj[OBJ_VIEW])
		return (FALSE);

	    /* Get the orientation of the scroller */
	    orient = (UWORD) GetTagData (AOLV_Freedom, (ULONG) LVSF_VERTICAL, tags);
	    si->si_Flags |= orient;

	    if ((orient & LVSF_VERTICAL) && FilterTagItems (tags, vprop, TAGFILTER_AND) >= 0)
	    {
		MapTags (tags, vpropm, 1);
		if (lod->lod_Obj[OBJ_VSCROLL] = NewObject (NULL, (UBYTE *) "scrollgclass",
							   PGA_Freedom, FREEVERT,
							   GA_ID, 11010L,
							   ICA_TARGET, lod->lod_Obj[OBJ_GLUE],
							   ICA_MAP, mpropv,
							   IA_LINEWIDTH, (ULONG) & marg,
							   TAG_MORE, tags,
							   TAG_END))
		{
		    DoMethod (o, OM_ADDMEMBER, (LONG) lod->lod_Obj[OBJ_VSCROLL]);
		    if (ic = (Object *) NewObject (NULL, (UBYTE *) ICCLASS,
						   ICA_TARGET, lod->lod_Obj[OBJ_VSCROLL],
						   ICA_MAP, vpropm,
						   TAG_END))
		    {
			DoMethod (lod->lod_Obj[OBJ_GLUE], OM_ADDMEMBER, (LONG) ic);
		    }
		}
		RefreshTagItemClones (tags, msg->ops_AttrList);
	    }

	    if ((orient & LVSF_HORIZONTAL) && FilterTagItems (tags, hprop, TAGFILTER_AND) >= 0)
	    {
		MapTags (tags, hpropm, 1);
		if (lod->lod_Obj[OBJ_HSCROLL] = NewObject (NULL, (UBYTE *) "scrollgclass",
							   PGA_Freedom, FREEHORIZ,
							   GA_ID, 11020L,
							   ICA_TARGET, lod->lod_Obj[OBJ_GLUE],
							   ICA_MAP, mproph,
							   IA_LINEWIDTH, (ULONG) & marg,
							   TAG_MORE, tags,
							   TAG_END))
		{
		    DoMethod (o, OM_ADDMEMBER, (LONG) lod->lod_Obj[OBJ_HSCROLL]);
		    if (ic = (Object *) NewObject (NULL, (UBYTE *) ICCLASS,
						   ICA_TARGET, lod->lod_Obj[OBJ_HSCROLL],
						   ICA_MAP, hpropm,
						   TAG_END))
		    {
			DoMethod (lod->lod_Obj[OBJ_GLUE], OM_ADDMEMBER, (LONG) ic);
		    }
		}
		RefreshTagItemClones (tags, msg->ops_AttrList);
	    }
	    lod->lod_Domain.Left = NOT_SET;
	    lod->lod_Domain.Top	= NOT_SET;
	    lod->lod_Domain.Width = NOT_SET;
	    lod->lod_Domain.Height = NOT_SET;
	    setListViewGAttrs (cl, o, msg);
	    retval = TRUE;
	}
	FreeTagItems (tags);
    }
    return (retval);
}

/* This handles the layout of all the components of the listview */
VOID layoutListViewG (Class * cl, Object * o, struct gpRender * gpr, BOOL force)
{
    struct LVSpecialInfo *si = SI (G (o)->SpecialInfo);
    struct localObjData *lod = INST_DATA (cl, o);
    struct IBox obox = *(&(lod->lod_Domain));
    struct IBox *box = &(lod->lod_Domain);
    struct IBox vbox;
    struct GadgetInfo *gi;

    /* Make sure we have all the information we need to layout */
    if (gi = GetGadgetInfo (gpr))
    {
	/* Compute the domain of the entire gadget */
	gadgetBox (G (o), &(gi->gi_Domain), box, NULL, NULL);

	/* Make sure we should change things... */
	if (force || !(compareRect (&obox, box)))
	{
	    LONG tx = (LONG) box->Left;
	    LONG ty = (LONG) box->Top;
	    LONG w = (LONG) box->Width;
	    LONG h = (LONG) box->Height;
	    LONG vsx, vsy, vsw, vsh;
	    LONG hsx, hsy, hsw, hsh;

	    /* Default values */
	    vsy = ty;
	    hsx = tx;
	    vbox.Height = vsh = h;
	    hsw = w;

	    /* Start with the frame size */
	    vbox.Left = box->Left;
	    vbox.Top = box->Top;
	    vbox.Width = w;
	    vbox.Height = h;

	    /* See if it has horizontial scrollbar */
	    if (si->si_Flags & LVSF_HORIZONTAL)
	    {
		/* Calculate the scroller size */
		hsh = lod->lod_HSize;
		hsy = ty + h - hsh;
		vsh -= hsh;

		/* Calculate the frame size */
		vbox.Height -= hsh;
	    }

	    /* See if it has a vertical scrollbar */
	    if (si->si_Flags & LVSF_VERTICAL)
	    {
		/* Calculate the scroller size */
		vsw = lod->lod_VSize;
		vsx = tx + w - vsw;
		hsw -= vsw;

		/* Calculate the frame size */
		vbox.Width -= vsw;
	    }

	    /* Set the frame */
	    if (lod->lod_Obj[OBJ_VIEW])
	    {
		/* Tell the master about the view size */
		SetAttrs (G (lod->lod_Obj[OBJ_GLUE]),
			  AOLV_View, &vbox,
			  TAG_DONE);

		SetAttrs (G (lod->lod_Obj[OBJ_VIEW]),
			  GA_LEFT, vbox.Left,
			  GA_TOP, vbox.Top,
			  GA_WIDTH, vbox.Width,
			  GA_HEIGHT, vbox.Height,
			  TAG_DONE);
	    }

	    /* Set the vertical scroll bar */
	    if (lod->lod_Obj[OBJ_VSCROLL])
	    {
		/* Set the gadget */
		SetAttrs (G (lod->lod_Obj[OBJ_VSCROLL]),
			  GA_LEFT, vsx,
			  GA_TOP, vsy,
			  GA_WIDTH, vsw,
			  GA_HEIGHT, vsh,
			  TAG_DONE);
	    }

	    /* Set the horizontal scroll bar */
	    if (lod->lod_Obj[OBJ_HSCROLL])
	    {
		/* Set the gadget */
		SetAttrs (G (lod->lod_Obj[OBJ_HSCROLL]),
			  GA_LEFT, hsx,
			  GA_TOP, hsy,
			  GA_WIDTH, hsw,
			  GA_HEIGHT, hsh,
			  TAG_DONE);
	    }
	}
    }
}

ULONG getListViewGAttrs (Class * cl, Object * o, struct opGet * msg)
{
    struct localObjData *lod = INST_DATA (cl, o);

    if (TagInArray (msg->opg_AttrID, model_attrs))
	return (DM (lod->lod_Obj[OBJ_GLUE], msg));
    return (DSM (cl, o, msg));
}

ULONG setListViewGAttrs (Class * cl, Object * o, struct opSet * msg)
{
    struct localObjData *lod = INST_DATA (cl, o);
    struct TagItem *tstate;
    struct TagItem *otags;
    struct TagItem *tag;
    ULONG retval = 0;
    ULONG tidata;

    otags = msg->ops_AttrList;
    msg->ops_AttrList = CloneTagItems (otags);

    G (o)->Activation = PackBoolTags (G (o)->Activation, otags, activation_bools);

    if (FilterTagItems (msg->ops_AttrList, model_attrs, TAGFILTER_AND))
    {
	if (msg->MethodID == OM_NEW)
	    DoMethod (lod->lod_Obj[OBJ_GLUE], OM_SET, (LONG) msg->ops_AttrList, (LONG) msg->ops_GInfo);
	else
	    DM (lod->lod_Obj[OBJ_GLUE], msg);
    }
    RefreshTagItemClones (msg->ops_AttrList, otags);

    if ((msg->MethodID != OM_NEW) && FilterTagItems (msg->ops_AttrList, model_attrs, TAGFILTER_NOT))
	retval += DSM (cl, o, msg);

    FreeTagItems (msg->ops_AttrList);
    msg->ops_AttrList = otags;

    /* Process rest */
    tstate = msg->ops_AttrList;
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
