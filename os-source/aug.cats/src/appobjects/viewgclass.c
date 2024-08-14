/* viewclass.c --- view gadget class
 * Copyright (C) 1991 Commodore-Amiga, Inc.
 * All Rights Reserved Worldwide
 * Written by David N. Junod
 *
 */

#include "appobjectsbase.h"
#include "appobjects_protos.h"

#define	DB(x)	;

#define	G(x)	((struct Gadget *)(x))
#define	IM(x)	((struct Image *)(x))
#define IEQUALIFIER_SHIFT	(IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT)

extern VOID NewList (struct List *);
extern struct Library *GfxBase, *IntuitionBase, *UtilityBase;

/* Public ListView class */
#define MYCLASSID	"viewgclass"
#define SUPERCLASSID	GADGETCLASS

/* lvmclass.c */
Class *initViewGClass (VOID);
ULONG freeViewGClass (Class * cl);
ULONG __saveds dispatchViewG (Class * cl, Object * o, Msg msg);
ULONG initializeViewG (Class * cl, Object * o, struct opSet * msg);
ULONG setViewGAttrs (Class * cl, Object * o, struct opSet * msg);
ULONG getViewGAttr (Class * cl, Object * o, struct opGet * msg);
ULONG renderViewG (Class * cl, Object * o, struct gpRender * gpr);
ULONG ClearSelected (Class * cl, Object * o, struct gpInput *, LONG line);
ULONG handleViewG (Class * cl, Object * o, struct gpInput * msg);
VOID layoutViewG (Class * cl, Object * o, struct gpRender * gpr, BOOL force);
struct Column *GetColumnNode (struct List * list, Object * o, LONG num);
ULONG DisposeColumns (struct List * list, VOID * msg);
LONG CountEntries (struct List * list);

struct Column
{
    struct Node cn_Node;	/* Embedded node */
    Object *cn_Object;		/* The column image */
    Object *cn_Control;		/* Column control gadget */
};

#define	CSIZE	(sizeof(struct Column))

struct localObjData
{
    struct IBox lod_Domain;	/* Domain of the gadget */
    struct List lod_List;	/* Column list */
    struct LVSpecialInfo lod_SI;/* Embedded SpecialInfo */

    Object *lod_Frame;		/* The frame for the gadget */
    ULONG lod_Flags;		/* See defines below (and appobjectsbase) */

    struct Column *lod_Drag;	/* Drag column */
    struct Column *lod_PDrag;	/* Column before */
    WORD lod_Min;		/* Minimum drag position */
    WORD lod_Max;		/* Maximum drag position */
    WORD lod_NewColumn;		/* New drag position */
    WORD lod_OldColumn;		/* Old drag position */
    WORD lod_OldDomain;
    WORD lod_Offset;

    UWORD lod_TH;		/* Title height */
    UWORD lod_CH;		/* Control height */

    struct
    {
	WORD V;
	WORD H;
    } lod_FMargins;		/* Margins for frame */


    WORD lod_Anchor;		/* Anchor point */
    WORD lod_Selected;		/* Selected line */
};

#define	LODF_DRAWN		0x00000001
#define	LODF_MOVE		0x00000002

static struct TagItem act_bools[] =
{
    {GA_EndGadget,	ENDGADGET},
    {GA_Immediate,	GADGIMMEDIATE},
    {GA_RelVerify,	RELVERIFY},
    {GA_FollowMouse,	FOLLOWMOUSE},
    {TAG_END}
};

static struct TagItem view_bools[] =
{
    {AOLV_MultiSelect,	LVSF_MULTISELECT},
    {AOLV_ReadOnly,	LVSF_READONLY},
    {AOLV_Borderless,	LVSF_BORDERLESS},
    {AOLV_Sizable,	LVSF_SIZABLE},
    {TAG_END}
};

#define	LODSIZE	(sizeof(struct localObjData))

Class *initViewGClass (VOID)
{
    Class *cl = NULL;
    ULONG hookEntry ();

    if (cl = MakeClass (MYCLASSID, SUPERCLASSID, NULL, LODSIZE, 0))
    {
	/* initialize the cl_Dispatcher Hook */
	cl->cl_Dispatcher.h_Entry = hookEntry;
	cl->cl_Dispatcher.h_SubEntry = dispatchViewG;

	/* Make the class public */
	AddClass (cl);
    }

    return (cl);
}

ULONG freeViewGClass (Class * cl)
{
    return ((ULONG) FreeClass (cl));
}

ULONG __saveds dispatchViewG (Class * cl, Object * o, Msg msg)
{
    struct localObjData *lod = INST_DATA (cl, o);
    struct Column *cn;
    ULONG retval = 0L;
    Object *tmpo;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    /* Have the superclass create the base object */
	    if (tmpo = (Object *) DSM (cl, o, msg))
	    {
		/* Initialize the complete set */
		if (!(initializeViewG (cl, tmpo, (struct opSet *) msg)))
		{
		    /* free what's there if failure	 */
		    CoerceMethod (cl, tmpo, OM_DISPOSE);
		    tmpo = NULL;
		}
	    }
	    retval = (ULONG) tmpo;
	    break;

	case OM_GET:
	    retval = getViewGAttr (cl, o, (struct opGet *) msg);
	    break;

	case OM_UPDATE:
	case OM_SET:
	    retval = DSM (cl, o, msg);
	    retval = MAX (retval, setViewGAttrs (cl, o, (struct opSet *) msg));

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
		    gpr.gpr_Redraw = retval;

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
	    layoutViewG (cl, o, (struct gpRender *) msg, FALSE);
	    retval = (ULONG) renderViewG (cl, o, (struct gpRender *) msg);
	    break;

	case GM_GOACTIVE:
	case GM_HANDLEINPUT:
	    retval = handleViewG (cl, o, (struct gpInput *) msg);
	    break;

	case GM_GOINACTIVE:
	    G (o)->Flags &= ~(SELECTED | ACTIVEGADGET);
	    if (!(lod->lod_SI.si_Flags & LVSF_MULTISELECT))
	    {
		lod->lod_SI.si_Flags &= ~LVSF_HIGHLIGHT;
		renderViewG (cl, o, (struct gpRender *) msg);
	    }
            notifyAttrChanges (o, ((struct gpRender *) msg)->gpr_GInfo, NULL,
                               GA_ID, G (o)->GadgetID,
                               GTLV_Selected, (LONG) lod->lod_Selected,
                               TAG_DONE);
	    break;

	case LV_ADDCOLUMN:
	    /* Allocate a column */
	    if (cn = (struct Column *) AllocVec (CSIZE, MEMF_CLEAR))
	    {
		/* Remember the objects */
		cn->cn_Object = ((struct opAddColumn *) msg)->opac_Object;
		cn->cn_Control = ((struct opAddColumn *) msg)->opac_Control;

		/* Add the column to our list */
		AddTail ((struct List *) & (lod->lod_List), (struct Node *) cn);

		/* Let the column know about the list particulars */
		SetAttrs (cn->cn_Object, AOLV_SpecialInfo, &(lod->lod_SI), TAG_DONE);

		/* Indicate success */
		retval = 1L;
	    }
	    break;

	case LV_REMCOLUMN:
	    /* Find the node in the list */
	    if (cn = GetColumnNode (&lod->lod_List, ((struct opAddMember *) msg)->opam_Object, -1))
	    {
		/* Remove the node */
		Remove ((struct Node *) cn);

		/* Free it */
		FreeVec (cn);
	    }
	    break;

	case OM_DISPOSE:
	    /* Send the message to all columns */
	    DisposeColumns (&lod->lod_List, msg);

	    /* Dispose of the frame if there is one */
	    if (lod->lod_Frame)
		DM (lod->lod_Frame, msg);

	    /* Dispose of view gadget itself */
	    DSM (cl, o, msg);

	    retval = 1L;
	    break;

	case GM_HITTEST:
	    if (lod->lod_SI.si_Flags & LVSF_READONLY)
		return (0L);

	    /* Pass it on to the superclass */
	default:
	    retval = (ULONG) DSM (cl, o, msg);
	    break;

    }

    return (retval);
}

/* Return TRUE if I could do everything needed to initialize one of my
 * objects. */
ULONG initializeViewG (Class * cl, Object * o, struct opSet * ops)
{
    struct LVSpecialInfo *si = (struct LVSpecialInfo *) G (o)->SpecialInfo;
    struct localObjData *lod = INST_DATA (cl, o);
    struct TagItem *tags = ops->ops_AttrList;
    struct Gadget *g = G (o);
    UWORD orient;

    /* Initialize column list */
    NewList (&lod->lod_List);

    /* Set up the special info record */
    si = &(lod->lod_SI);
    g->SpecialInfo = si;

    /* Provide reasonable defaults */
    si->si_UnitWidth = si->si_UnitHeight = 1;
    lod->lod_Selected = -1;

    /* Get the activation flags */
    si->si_Flags = PackBoolTags (si->si_Flags, tags, view_bools);

    /* Get the orientation of the scroller */
    orient = (UWORD) GetTagData (AOLV_Freedom, (ULONG) LVSF_VERTICAL, tags);
    si->si_Flags |= orient;

    /* Allocate the frame */
    if (!(si->si_Flags & LVSF_BORDERLESS))
    {
	/* Allocate the frame */
	if (lod->lod_Frame = NewObject (NULL, "aframeiclass",
					IA_EdgesOnly, TRUE,
					IA_Recessed, (LONG) (si->si_Flags & LVSF_READONLY),
					TAG_DONE))
	{
	    /* Get the margins of the frame */
	    if (!(DoMethod (lod->lod_Frame, OM_GET,
			    IA_LINEWIDTH, &(lod->lod_FMargins))))
	    {
		/* Provide a default */
		lod->lod_FMargins.V = 1;
		lod->lod_FMargins.H = 1;
	    }
	}
	else
	{
	    /* Show that we were unable to allocate the frame */
	    return (FALSE);
	}
    }

    /* Preset the domain */
    lod->lod_Domain.Left = NOT_SET;
    lod->lod_Domain.Top = NOT_SET;
    lod->lod_Domain.Width = NOT_SET;
    lod->lod_Domain.Height = NOT_SET;

    /* Set the attributes */
    setViewGAttrs (cl, o, ops);

    return (1L);
}

/* This handles the layout of all the components of the listview */
VOID layoutViewG (Class * cl, Object * o, struct gpRender * gpr, BOOL force)
{
    struct LVSpecialInfo *si = (struct LVSpecialInfo *) G (o)->SpecialInfo;
    struct localObjData *lod = INST_DATA (cl, o);
    struct IBox obox = *(&(lod->lod_Domain));
    struct IBox *box = &(lod->lod_Domain);
    struct List *list = &lod->lod_List;
    UWORD vm = lod->lod_FMargins.V;
    UWORD hm = lod->lod_FMargins.H;
    struct Image *im;
    UWORD vheight;
    UWORD vwidth;
    struct Node *node;
    SHORT ty;
    SHORT tx = vm;
    SHORT width;
    SHORT perc;
    ULONG tagt = TAG_IGNORE;
    LONG total;
    ULONG tagv = TAG_IGNORE;
    LONG visv = 0L;
    ULONG tagh = TAG_IGNORE;
    LONG vish = 0L;
    BOOL notify = FALSE;

    /* Compute the domain of the entire gadget */
    computeDomain (cl, o, gpr, box, NULL, NULL);

    /* Make sure we should change things... */
    if (force || !(compareRect (&obox, box)))
    {
	/* Get the size of the view */
	vheight = box->Height - lod->lod_CH - lod->lod_TH - (hm * 2);
	vwidth = box->Width - (vm * 2);
	ty = lod->lod_TH + hm;

	/* Set up the view rectangle */
	si->si_View.Left = box->Left + tx;
	si->si_View.Top = box->Top + ty;
	si->si_View.Width = vwidth;
	si->si_View.Height = vheight;

	/* Make sure there are entries in the list */
	if (list && (list->lh_TailPred != (struct Node *) list))
	{
	    for (node = list->lh_Head; node->ln_Succ; node = node->ln_Succ)
	    {
		/* Cast it to a column */
		im = IM (((struct Column *) node)->cn_Object);
		perc = (SHORT) ((im->ImageData) ? im->ImageData : 100);
		width = (vwidth * perc / 100) - vm;

		/* Fill in the frame particular information */
		if (node == list->lh_Head)
		{
		    im->LeftEdge = tx;
		    im->Width = width + vm;
		}
		else if (node == list->lh_TailPred)
		{
		    im->LeftEdge = tx + vm;
#if 1
		    im->Width = vwidth - tx;
#else
		    im->Width = width + vm - 1;
#endif
		}
		else
		{
		    im->LeftEdge = tx + vm;
		    im->Width = width;
		}

		im->TopEdge = ty;
		im->Height = vheight;

		tx += (width + vm);
	    }
	}

	/* Make sure we have a list */
	if (si->si_List && (si->si_List != (struct List *) ~ 0))
	{
	    /* Count total number of lines in list */
	    total = CountEntries (si->si_List);

	    /* See if it has a vertical scrollbar */
	    if ((si->si_Flags & LVSF_VERTICAL) &&
		(total != si->si_TotalVert))
	    {
		/* Set the total vertical */
		tagt = AOLV_TotalVert;
		notify = TRUE;
	    }
	}
    }

    /* See if it has horizontial scrollbar */
    if (si->si_Flags & LVSF_HORIZONTAL)
    {
	/* Compute the visible portion */
	vish = MAX (1, (si->si_View.Width / (LONG) si->si_UnitWidth));
	vish = MIN (si->si_TotalHoriz, vish);
	if (si->si_VisibleHoriz != vish)
	{
	    tagh = AOLV_VisibleHoriz;
	    si->si_VisibleHoriz = vish;
	    notify = TRUE;
	}
    }

    /* See if it has a vertical scrollbar */
    if (si->si_Flags & LVSF_VERTICAL)
    {
	/* Compute the visible portion */
	visv = MAX (1, (si->si_View.Height / (LONG) si->si_UnitHeight));
	visv = MIN (si->si_TotalVert, visv);
	if (si->si_VisibleVert != visv)
	{
	    tagv = AOLV_VisibleVert;
	    si->si_VisibleVert = visv;
	    notify = TRUE;
	}
    }

    if (notify)
    {
	/* Tell people about the size */
	notifyAttrChanges (o, gpr->gpr_GInfo, NULL,
			   GA_ID, G (o)->GadgetID,
			   tagv, visv,
			   tagh, vish,
			   tagt, total,
			   TAG_DONE);
    }
}

ULONG getViewGAttr (Class * cl, Object * o, struct opGet * msg)
{
    struct localObjData *lod = INST_DATA (cl, o);
    struct LVSpecialInfo *si = &(lod->lod_SI);
    ULONG retval = 1L;

    switch (msg->opg_AttrID)
    {
	case GTLV_Selected:
	    *msg->opg_Storage = (ULONG) lod->lod_Selected;
	    break;

	case AOLV_View:
	    *msg->opg_Storage = (ULONG) & (si->si_View);
	    break;

	case AOLV_TopVert:
	    *msg->opg_Storage = si->si_TopVert;
	    break;

	case AOLV_TotalVert:
	    *msg->opg_Storage = si->si_TotalVert;
	    break;

	case AOLV_VisibleVert:
	    *msg->opg_Storage = si->si_VisibleVert;
	    break;

	case AOLV_TopHoriz:
	    *msg->opg_Storage = si->si_TopHoriz;
	    break;

	case AOLV_TotalHoriz:
	    *msg->opg_Storage = si->si_TotalHoriz;
	    break;

	case AOLV_VisibleHoriz:
	    *msg->opg_Storage = si->si_VisibleHoriz;
	    break;

	default:
	    /* I don't recognize this one, let the superclass try */
	    retval = (ULONG) DSM (cl, o, msg);
	    break;
    }

    return (retval);
}

ULONG setViewGAttrs (Class * cl, Object * o, struct opSet * ops)
{
    struct localObjData *lod = INST_DATA (cl, o);
    struct LVSpecialInfo *si = &(lod->lod_SI);
    struct GadgetInfo *gi = ops->ops_GInfo;
    struct TagItem *tstate;
    struct TagItem *tag;
    BOOL layout = FALSE;
    ULONG retval = 0;
    ULONG tidata;

    /* Set the starting attribute */
    tstate = ops->ops_AttrList;

    /* Get the activation flags */
    G (o)->Activation = PackBoolTags (G (o)->Activation, tstate, act_bools);

    /* Process rest */
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
		retval = IDSM_REDRAW;
		break;

	    case GTLV_Selected:
		/* This has to be this way or Multiline select won't fucking work. */
		if (lod->lod_Selected != (WORD) tidata)
		{
		    lod->lod_Selected = (WORD) tidata;
		    ClearSelected (cl, o, NULL, tidata);
		    retval = IDSM_REDRAW;
		}
		break;

	    case AOLV_UnitHeight:
		if ((tidata == -1) && gi)
		{
		    si->si_UnitHeight = gi->gi_DrInfo->dri_Font->tf_YSize + 1;
		}
		else
		{
		    si->si_UnitHeight = (UWORD) tidata;
		}
		retval = IDSM_REDRAW;
		layout = TRUE;
		break;

	    case AOLV_UnitWidth:
		si->si_UnitWidth = (UWORD) tidata;
		retval = IDSM_REDRAW;
		layout = TRUE;
		break;

	    case AOLV_TopVert:
		si->si_TopVert = tidata;
		retval = IDSM_UPDATE;
		break;

	    case AOLV_TopHoriz:
		si->si_TopHoriz = tidata;
		retval = IDSM_REDRAW;
		break;

	    case AOLV_VisibleVert:
		si->si_VisibleVert = tidata;
		retval = IDSM_REDRAW;
		break;

	    case AOLV_VisibleHoriz:
		si->si_VisibleHoriz = tidata;
		retval = IDSM_REDRAW;
		break;

	    case AOLV_TotalVert:
		si->si_TotalVert = tidata;
		retval = IDSM_REDRAW;
		layout = TRUE;
		break;

	    case AOLV_TotalHoriz:
		si->si_TotalHoriz = tidata;
		retval = IDSM_REDRAW;
		layout = TRUE;
		break;

	    case GTLV_Labels:
	    case AOLV_List:
		si->si_List = (struct List *) tidata;
		retval = IDSM_REDRAW;
		layout = TRUE;
		break;

	    case AOLV_ControlHeight:
		lod->lod_CH = (UWORD) tidata;
		retval = IDSM_REDRAW;
		layout = TRUE;
		break;

	    case AOLV_LabelHeight:
		lod->lod_TH = (UWORD) tidata;
		retval = IDSM_REDRAW;
		layout = TRUE;
		break;
	}
    }

    /* Do we need to layout again? */
    if (layout)
    {
	lod->lod_Domain.Left = NOT_SET;
	lod->lod_Domain.Top = NOT_SET;
	lod->lod_Domain.Width = NOT_SET;
	lod->lod_Domain.Height = NOT_SET;
    }

    return (retval);
}

ULONG renderViewG (Class * cl, Object * o, struct gpRender * msg)
{
    struct LVSpecialInfo *si = (struct LVSpecialInfo *) G (o)->SpecialInfo;
    struct Window *win = msg->gpr_GInfo->gi_Window;
    struct localObjData *lod = INST_DATA (cl, o);
    struct IBox *box = &(lod->lod_Domain);
    struct List *list = &lod->lod_List;
    UWORD vm = lod->lod_FMargins.V;
    UWORD hm = lod->lod_FMargins.H;
    struct impDrawColumn imp;
    BOOL refresh = FALSE;
    struct RastPort *rp;
    struct Node *node;
    struct Image *im;
    UWORD vheight;
    UWORD vwidth;
    UWORD *pens;

    /* Get a pointer to the rastport */
    if (rp = ObtainGIRPort (msg->gpr_GInfo))
    {
	/* This must be done if the image class does any clipping */
	if (win->Flags & WINDOWREFRESH)
	{
	    refresh = TRUE;
	    EndRefresh (win, FALSE);
	}

	SetDrMd (rp, COMPLEMENT);
	SetAPen (rp, 0);

	if (lod->lod_Flags & LODF_DRAWN)
	{
	    Move (rp, box->Left + lod->lod_OldColumn, box->Top + lod->lod_TH + 1);
	    Draw (rp, box->Left + lod->lod_OldColumn, box->Top + box->Height - lod->lod_TH - lod->lod_CH - 2);
	    Move (rp, box->Left + lod->lod_OldColumn + 1, box->Top + lod->lod_TH + 1);
	    Draw (rp, box->Left + lod->lod_OldColumn + 1, box->Top + box->Height - lod->lod_TH - lod->lod_CH - 2);
	    lod->lod_Flags &= ~LODF_DRAWN;
	}

	if (lod->lod_Drag && ((G(o)->Flags & GFLG_SELECTED)))
	{
	    Move (rp, box->Left + lod->lod_NewColumn, box->Top + lod->lod_TH + 1);
	    Draw (rp, box->Left + lod->lod_NewColumn, box->Top + box->Height - lod->lod_TH - lod->lod_CH - 2);
	    Move (rp, box->Left + lod->lod_NewColumn + 1, box->Top + lod->lod_TH + 1);
	    Draw (rp, box->Left + lod->lod_NewColumn + 1, box->Top + box->Height - lod->lod_TH - lod->lod_CH - 2);
	    lod->lod_OldColumn = lod->lod_NewColumn;
	    lod->lod_Flags |= LODF_DRAWN;
	}
	else
	{
	    SetDrMd (rp, JAM2);

	    if (lod->lod_Flags & LODF_MOVE)
	    {
		SetAPen (rp, msg->gpr_GInfo->gi_DrInfo->dri_Pens[BACKGROUNDPEN]);
		RectFill (rp,
			  box->Left + lod->lod_OldDomain - 4,
			  box->Top + lod->lod_TH + 1,
			  box->Left + lod->lod_OldDomain + 1,
			  box->Top + box->Height - lod->lod_TH - lod->lod_CH - 2);
		lod->lod_Flags &= ~LODF_MOVE;
	    }

	    /* Construct a 'fake' message */
	    imp.MethodID = IM_DRAWFRAME;
	    imp.imp_RPort = rp;
	    imp.imp_DrInfo = msg->gpr_GInfo->gi_DrInfo;
	    imp.imp_State = IDS_NORMAL;
	    imp.imp_Offset.X = box->Left;
	    imp.imp_Offset.Y = box->Top + lod->lod_TH;
	    imp.imp_Dimensions.Width = box->Width;
	    imp.imp_Dimensions.Height = box->Height - lod->lod_TH - lod->lod_CH;
	    imp.imp_Mode = IDSM_TOGGLE;

	    /* Let's be sure that we were passed a DrawInfo */
	    pens = imp.imp_DrInfo->dri_Pens;

	    /* Get the size of the view */
	    vheight = box->Height - (hm * 2);
	    vwidth = box->Width - (vm * 2);

	    /* See if we're doing a full draw */
	    if (msg->MethodID == GM_RENDER)
	    {
		if (msg->gpr_Redraw == GREDRAW_REDRAW)
		{
		    /* Do we have a frame to draw */
		    if (im = IM (lod->lod_Frame))
		    {
			/* Draw the frame */
			DM ((Object *) im, &imp);

			/* Make sure there are entries in the list */
			if (list && (list->lh_TailPred != (struct Node *) list))
			{
			    SHORT ty = box->Top + hm;
			    SHORT by = box->Top + box->Height - 1 - hm;
			    SHORT dark = pens[SHADOWPEN];
			    SHORT lite = pens[SHINEPEN];
			    SHORT tx = box->Left;
			    SHORT lx;

			    /* Which way do the bars go */
			    if (si->si_Flags & LVSF_READONLY)
			    {
				dark = lite;
				lite = pens[SHADOWPEN];
			    }

			    for (node = list->lh_Head; node->ln_Succ; node = node->ln_Succ)
			    {
				/* Don't do one for the first column */
				if (node != list->lh_Head)
				{
				    /* Cast it to an image */
				    im = IM (((struct Column *) node)->cn_Object);

				    /* Get the left edge */
				    lx = tx + im->LeftEdge - vm;

				    SetAPen (rp, dark);
				    Move (rp, lx, ty);
				    Draw (rp, lx, by);

				    lx++;

				    SetAPen (rp, lite);
				    Move (rp, lx, ty);
				    Draw (rp, lx, by);
				}
			    }
#if 0
			    lx = tx + im->LeftEdge + im->Width;
			    SetAPen (rp, pens[BACKGROUNDPEN]);
			    Move (rp, lx, ty);
			    Draw (rp, lx, by);
			    lx++;
			    Move (rp, lx, ty);
			    Draw (rp, lx, by);
#endif
			}
		    }
		}

		/* Make it carry through to the image */
		imp.imp_Mode = msg->gpr_Redraw;
	    }

	    if (list && (list->lh_TailPred != (struct Node *) list))
	    {
		/* Set the state */
		if (G (o)->Flags & GADGDISABLED)
		    imp.imp_State = IDS_DISABLED;
		else if (G (o)->Flags & SELECTED)
		    imp.imp_State = IDS_SELECTED;

		/* Now do each column */
		imp.MethodID = LV_DRAWCOLUMN;
		imp.imp_Anchor.X = si->si_Anchor.X;
		imp.imp_Anchor.Y = si->si_Anchor.Y;
		imp.imp_Over.X = si->si_Over.X;
		imp.imp_Over.Y = si->si_Over.Y;

		for (node = list->lh_Head; node->ln_Succ; node = node->ln_Succ)
		{
		    im = IM (((struct Column *) node)->cn_Object);
		    imp.imp_Dimensions.Width = im->Width;
		    imp.imp_Dimensions.Height = im->Height;
		    DM ((Object *) im, &imp);
		}
	    }
	}

	if (refresh)
	    BeginRefresh (win);
	ReleaseGIRPort (rp);
    }
    return (1L);
}

#if 0
ULONG ClearSelected (Class * cl, Object * o, struct gpInput * msg, LONG line)
{
    struct LVSpecialInfo *si = (struct LVSpecialInfo *) G (o)->SpecialInfo;
    struct List *list = si->si_List;
    struct Node *node;
    LONG cnt;

    /* Make sure there are entries in the list */
    if (list && (list->lh_TailPred != (struct Node *) list))
    {
	for (node = list->lh_Head, cnt = 0; node->ln_Succ; node = node->ln_Succ, cnt++)
	{
	    if (cnt == line)
		node->ln_Type |= LNF_SELECTED;
	    else
		node->ln_Type &= ~LNF_SELECTED;
	}
    }
    renderViewG (cl, o, (struct gpRender *) msg);

    return (0L);
}
#else
ULONG ClearSelected (Class * cl, Object * o, struct gpInput * msg, LONG line)
{
    struct LVSpecialInfo *si = (struct LVSpecialInfo *) G (o)->SpecialInfo;
    struct List *list = si->si_List;
    struct Node *node;
    LONG display;
    ULONG ch = 0;
    LONG cnt;

    /* Make sure there are entries in the list */
    if (list && (list != (struct List *) ~0) && (list->lh_TailPred != (struct Node *) list))
    {
	for (node = list->lh_Head, cnt = display = 0; node->ln_Succ; node = node->ln_Succ, cnt++)
	{
	    if (cnt == line)
	    {
		node->ln_Type |= LNF_SELECTED;
	    }
	    else if (node->ln_Type & LNF_SELECTED)
	    {
		node->ln_Type &= ~LNF_SELECTED;
		ch++;

		/* Update the display? */
		if (msg && (cnt >= si->si_TopVert) && (display < si->si_VisibleVert))
		{
		    si->si_Over.Y = cnt;
		    renderViewG (cl, o, (struct gpRender *) msg);
		}
	    }
	    if (cnt >= si->si_TopVert)
		display++;
	}
    }

    return (ch);
}
#endif

ULONG SelectAnchor (Class * cl, Object * o, struct gpInput * msg, UWORD sel)
{
    struct LVSpecialInfo *si = (struct LVSpecialInfo *) G (o)->SpecialInfo;
    LONG ty = (LONG) MIN (si->si_Anchor.Y, si->si_Over.Y);
    LONG by = (LONG) MAX (si->si_Anchor.Y, si->si_Over.Y);
    struct List *list = si->si_List;
    struct Node *node;
    UBYTE flag;
    LONG cnt;

    /* Make sure there are entries in the list */
    if (list && (list->lh_TailPred != (struct Node *) list))
    {
	for (node = list->lh_Head, cnt = 0; node->ln_Succ; node = node->ln_Succ, cnt++)
	{
	    flag = node->ln_Type & LNF_TEMPORARY;
	    node->ln_Type &= ~(LNF_TEMPORARY | LNF_REFRESH);

	    if ((cnt >= ty) && (cnt <= by))
	    {
		if (sel == 1)
		    node->ln_Type |= LNF_SELECTED;
		else if (sel == 2)
		    node->ln_Type |= LNF_TEMPORARY;
	    }
	    if (flag != (node->ln_Type & LNF_TEMPORARY))
		node->ln_Type |= LNF_REFRESH;
	}
    }

    return (1);
}

struct Column *hitColumnBoundary (struct localObjData * lod, struct Gadget * g, WORD mx)
{
    struct LVSpecialInfo *si = (struct LVSpecialInfo *) g->SpecialInfo;
    struct List *list = &lod->lod_List;
    struct Node *node;
    struct Image *im;

    if (si->si_Flags & LVSF_SIZABLE)
    {
        if (mx < 0)
            return (NULL);
        mx += 3;
        if (list->lh_TailPred != (struct Node *) list)
        {
            for (node = list->lh_Head; node->ln_Succ; node = node->ln_Succ)
            {
                im = IM (((struct Column *) node)->cn_Object);
                if ((mx >= im->LeftEdge - 1) && (mx <= im->LeftEdge + 2))
                {
                    lod->lod_PDrag = (struct Column *)node->ln_Pred;
                    lod->lod_Min = ((struct Image *)lod->lod_PDrag->cn_Object)->LeftEdge + 4;
                    lod->lod_Max = im->LeftEdge + im->Width - 4;

                    lod->lod_OldDomain = im->LeftEdge;
                    lod->lod_Offset = mx - im->LeftEdge;
                    return ((struct Column *) node);
                }
            }
        }
    }
    return (NULL);
}

ULONG handleViewG (Class * cl, Object * o, struct gpInput * msg)
{
    struct LVSpecialInfo *si = (struct LVSpecialInfo *) G (o)->SpecialInfo;
    struct localObjData *lod = INST_DATA (cl, o);
    struct InputEvent *ie = msg->gpi_IEvent;
    struct GadgetInfo *gi = msg->gpi_GInfo;
    struct IBox *box = &(lod->lod_Domain);
    ULONG retval = GMR_MEACTIVE;
    BOOL refresh = FALSE;
    BOOL select = FALSE;
    struct Column *cn;
    WORD column = 0;
    LONG mx, my;
    WORD line;
    LONG adj;

    /* Cache the mouse coordinates */
    mx = msg->gpi_Mouse.X - 1;
    my = msg->gpi_Mouse.Y - 1;

    /* Get the selected line */
    line = (WORD) (si->si_TopVert + (my / si->si_UnitHeight));

    /* Make sure it's a valid line */
    if (my < 0)
    {
	line = -1;
	if (si->si_Flags & LVSF_DRAGGING)
	    line = si->si_TopVert;
    }
    else if (line >= (si->si_TopVert + si->si_VisibleVert))	/* Added the = */
    {
	line = -1;
	if (si->si_Flags & LVSF_DRAGGING)
	    line = si->si_TopVert + si->si_VisibleVert - 1;	/* Added the - 1 */
    }

    if ((mx < 0) || (mx > box->Width))
    {
	column = -1;
	line = -1;
    }

    /* Determine what to do... */
    switch (ie->ie_Class)
    {
	    /* Mouse button or movement */
	case IECLASS_RAWMOUSE:
	    switch (ie->ie_Code)
	    {
		case SELECTDOWN:
		    refresh = TRUE;
		    G (o)->Flags |= SELECTED;
		    si->si_Anchor.X = (LONG) column;
		    si->si_Anchor.Y = (LONG) line;
		    if (lod->lod_Drag = hitColumnBoundary (lod, G (o), (WORD) mx))
			lod->lod_NewColumn = lod->lod_OldColumn = (WORD)mx;
		    else
		    {
			/* Are we a MultiSelect list view? */
			if ((si->si_Flags & LVSF_MULTISELECT) && (si->si_List != (struct List *) ~ 0))
			{
			    /* Get the current line */
			    if (cn = GetColumnNode (si->si_List, NULL, line))
			    {
				if (!(ie->ie_Qualifier & IEQUALIFIER_SHIFT))
				    ClearSelected (cl, o, msg, (LONG)line);
				si->si_Flags |= LVSF_DRAGGING;
			    }
			    else
				retval = GMR_NOREUSE;
			}
			select = TRUE;
		    }
		    break;

		    /* Select button released */
		case SELECTUP:
		    if (lod->lod_Drag)
		    {
			WORD perc, total, dif, vwidth;
			struct Image *im;

			dif = lod->lod_OldDomain + lod->lod_Offset - lod->lod_NewColumn - 3;
			if (dif != 0)
			{
			    lod->lod_Flags |= LODF_MOVE;
			    vwidth = box->Width - (lod->lod_FMargins.V * 2);
                            im = (struct Image *)lod->lod_PDrag->cn_Object;
                            total = (WORD) im->ImageData;
                            im->Width -= dif;
                            perc = ((im->Width + lod->lod_FMargins.V) * 100) / vwidth;
                            im->ImageData = (UWORD *) perc;

                            im = (struct Image *)lod->lod_Drag->cn_Object;
                            total += (WORD) im->ImageData;
                            im->LeftEdge = lod->lod_NewColumn;
                            im->Width += dif;
                            perc = total - perc;
                            im->ImageData = (UWORD *)perc;
			}
		    }
		    else if (si->si_Flags & LVSF_MULTISELECT)
		    {
			/* Select the selected items */
			SelectAnchor (cl, o, msg, 1);
			line = -1;
		    }
		    else if ((si->si_Over.Y != si->si_Anchor.Y))
			line = -1;
		    retval = (GMR_NOREUSE | GMR_VERIFY);
		    break;

		/* Menu (ABORT) button pressed */
		case MENUDOWN:
		    if (si->si_Flags & LVSF_MULTISELECT)
			SelectAnchor (cl, o, msg, 0);

		    /* This event ends it */
		    retval = GMR_NOREUSE;
		    break;

		/* Continue giving me input */
		default:
		    if (lod->lod_Drag)
		    {
			WORD np;

			np = (WORD)mx + lod->lod_Offset;

			if (np < 4)
			    np = 4;
			if (np > box->Width - 4)
			    np = box->Width - 4;
			if (np < lod->lod_Min)
			    np = lod->lod_Min;
			if (np > lod->lod_Max)
			    np = lod->lod_Max;

			if (lod->lod_NewColumn != np)
			{
			    lod->lod_NewColumn = np;
			    refresh = TRUE;
			}
		    }
		    else
			refresh = select = TRUE;
		    break;
	    }
	    break;

	    /* IntuiTick */
	case IECLASS_TIMER:
	    mx = msg->gpi_Mouse.X;
	    my = msg->gpi_Mouse.Y;

	    if (lod->lod_Drag)
		break;

	    if (mx <= 0)
	    {
		if (si->si_TopHoriz > 0)
		{
		    adj = si->si_TopHoriz - 1;
		    notifyAttrChanges (o,
				       gi,
				       OPUF_INTERIM,
				       GA_ID, G (o)->GadgetID,
				       AOLV_TopHoriz, adj,
				       TAG_DONE);
		    refresh = select = TRUE;
		}
	    }
	    else if (mx > box->Width)
	    {
		if (si->si_TopHoriz < (si->si_TotalHoriz - si->si_VisibleHoriz))
		{
		    adj = si->si_TopHoriz + 1;
		    notifyAttrChanges (o,
				       gi,
				       OPUF_INTERIM,
				       GA_ID, G (o)->GadgetID,
				       AOLV_TopHoriz, adj,
				       TAG_DONE);
		    refresh = select = TRUE;
		}
	    }

	    if (my <= 0)
	    {
		/* No valid line... */
		line = -1;

		if (si->si_TopVert > 0)
		{
		    line = adj = si->si_TopVert - 1;
		    notifyAttrChanges (o,
				       gi,
				       OPUF_INTERIM,
				       GA_ID, G (o)->GadgetID,
				       AOLV_TopVert, adj,
				       TAG_DONE);
		    refresh = select = TRUE;
		}
	    }
	    else if (my > box->Height)
	    {
		/* No valid line... */
		line = -1;

		if (si->si_TopVert < (si->si_TotalVert - si->si_VisibleVert))
		{
		    adj = si->si_TopVert + 1;
		    line = adj + si->si_VisibleVert;
		    notifyAttrChanges (o,
				       gi,
				       OPUF_INTERIM,
				       GA_ID, G (o)->GadgetID,
				       AOLV_TopVert, adj,
				       TAG_DONE);
		    refresh = select = TRUE;
		}
	    }

	    break;
    }

    si->si_Over.X = (LONG) column;
    si->si_Over.Y = (LONG) line;

    if (si->si_Flags & LVSF_MULTISELECT)
    {
	if (select)
	{
	    lod->lod_Selected = line;
	    SelectAnchor (cl, o, msg, 2);
	}
    }
    else if (!lod->lod_Drag)
    {
	refresh = FALSE;
	lod->lod_Selected = line;
	if (si->si_Over.Y == si->si_Anchor.Y)
	{
	    if (!(si->si_Flags & LVSF_HIGHLIGHT))
	    {
		/* Show that it is highlighted */
		si->si_Flags |= LVSF_HIGHLIGHT;
		refresh = TRUE;
	    }
	}
	else
	{
	    if (si->si_Flags & LVSF_HIGHLIGHT)
	    {
		/* Show that it isn't highlighted */
		si->si_Flags &= ~LVSF_HIGHLIGHT;
		refresh = TRUE;
	    }
	}
    }

    if (retval & GMR_NOREUSE)
	si->si_Flags &= ~(LVSF_DRAGGING | LVSF_SELECT | LVSF_HIGHLIGHT);

    if (refresh)
	renderViewG (cl, o, (struct gpRender *) msg);
    *msg->gpi_Termination = line;
    return (retval);
}

struct Column *GetColumnNode (struct List * list, Object * o, LONG num)
{
    struct Node *node;
    LONG cnt;

    if (list && (list->lh_TailPred != (struct Node *) list))
    {
	for (node = list->lh_Head, cnt = 0; node->ln_Succ; node = node->ln_Succ, cnt++)
	{
	    if (((struct Column *)node)->cn_Object == o)
		return ((struct Column *)node);

	    if (cnt == num)
		return ((struct Column *)node);
	}
    }

    return (NULL);
}

ULONG DisposeColumns (struct List * list, VOID * msg)
{
    struct Node *node;

    while (node = RemHead (list))
    {
	DM (((struct Column *)node)->cn_Object, msg);
	FreeVec (node);
    }
    return (NULL);
}

LONG CountEntries (struct List * list)
{
    struct Node *node;
    LONG count = -1L;

    if (list && (list->lh_TailPred != (struct Node *) list))
	for (node = list->lh_Head, count = 0L; node->ln_Succ; node = node->ln_Succ, count++);

    return (count);
}
