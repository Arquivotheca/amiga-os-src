/* lvmclass.c --- ListView model class
 * Copyright (C) 1991 Commodore-Amiga, Inc.
 * All Rights Reserved Worldwide
 * Written by David N. Junod
 *
 */

#include "appobjectsbase.h"
#include "appobjects_protos.h"

#define	DB(x)	;

#define	COL	FALSE
#define	G(x)	((struct Gadget *)(x))

VOID NewList (struct List *);

extern struct Library *GfxBase, *IntuitionBase, *UtilityBase;

/* Private model class */
Class *initListViewMClass (VOID);
ULONG freeListViewMClass (Class * cl);
ULONG __saveds dispatchListViewM (Class * cl, Object * o, Msg msg);
ULONG setListViewMAttrs (Class * cl, Object * o, struct opSet * msg);
ULONG getListViewMAttr (Class * cl, Object * o, struct opGet * msg);

struct modelObjData
{
    struct List *lod_List;	/* Pointer to list of structures */
    WORD lod_UnitWidth;		/* Width of each horizontal unit */
    WORD lod_UnitHeight;	/* Height of each vertical unit */
    LONG lod_TopVert;		/* Vertical top */
    LONG lod_VisibleVert;	/* Number of visible vertical units */
    LONG lod_TotalVert;		/* Total number of vertical units */
    LONG lod_TopHoriz;		/* Horizontal top */
    LONG lod_VisibleHoriz;	/* Number of visible horizontal units */
    LONG lod_TotalHoriz;	/* Total number of horizontal units */
    LONG lod_Selected;		/* Select item */
    struct IBox lod_View;	/* View box */
    struct LVSpecialInfo *lod_SI;
    ULONG lod_ID;
    LONG lod_FromID;
    UWORD lod_Flags;		/* */
};

#define	LODF_SELECT		0x0001

#define	LSIZE	(sizeof(struct modelObjData))

Class *initListViewMClass (VOID)
{
    ULONG __saveds dispatchListViewM ();
    ULONG hookEntry ();
    Class *cl;

    if (cl = MakeClass (NULL, MODELCLASS, NULL, LSIZE, 0))
    {
	cl->cl_Dispatcher.h_Entry = hookEntry;
	cl->cl_Dispatcher.h_SubEntry = dispatchListViewM;
    }
    return (cl);
}

ULONG freeListViewMClass (Class * cl)
{

    return ((ULONG) FreeClass (cl));
}

ULONG __saveds dispatchListViewM (Class * cl, Object * o, Msg msg)
{
    struct modelObjData *lod = INST_DATA (cl, o);
    ULONG retval = 0L;
    Object *newobj;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    if (newobj = (Object *) DSM (cl, o, msg))
	    {
		lod = INST_DATA (cl, newobj);
		lod->lod_ID = GetTagData (GA_ID, NULL, ((struct opSet *) msg)->ops_AttrList);
		setListViewMAttrs (cl, newobj, (struct opSet *) msg);
	    }
	    retval = (ULONG) newobj;
	    break;

	case OM_GET:
	    retval = (ULONG) getListViewMAttr (cl, o, (struct opGet *) msg);
	    break;

	case OM_SET:
	case OM_UPDATE:
	    if (!DoSuperMethod (cl, o, ICM_CHECKLOOP))
	    {
		ULONG topvt;
		LONG otopv;
		ULONG visvt;
		LONG ovisv;
		ULONG totvt;
		LONG ototv;
		ULONG topht;
		LONG otoph;
		ULONG visht;
		LONG ovish;
		ULONG totht;
		LONG ototh;
		ULONG unitwidtht;
		LONG ounitwidth;
		ULONG unitheightt;
		LONG ounitheight;
		ULONG listt;
		LONG olist;
		ULONG selt;
		ULONG nmflag = NULL;

		unitwidtht = AOLV_UnitWidth;
		ounitwidth = (LONG) lod->lod_UnitWidth;

		unitheightt = AOLV_UnitHeight;
		ounitheight = (LONG) lod->lod_UnitHeight;

		listt = AOLV_List;
		olist = (ULONG) lod->lod_List;

		topvt = AOLV_TopVert;
		otopv = lod->lod_TopVert;

		visvt = AOLV_VisibleVert;
		ovisv = lod->lod_VisibleVert;

		totvt = AOLV_TotalVert;
		ototv = lod->lod_TotalVert;

		topht = AOLV_TopHoriz;
		otoph = lod->lod_TopHoriz;

		visht = AOLV_VisibleHoriz;
		ovish = lod->lod_VisibleHoriz;

		totht = AOLV_TotalHoriz;
		ototh = lod->lod_TotalHoriz;

		/*
		 * let the superclass see whatever it wants from OM_SET, such
		 * as ICA_TARGET, ICA_MAP, and so on.  For OM_NOTIFY, however,
		 * we control all traffic and issue notification specifically
		 * for the attributes we're interested in.
		 */
		if (msg->MethodID == OM_SET)
		    DSM (cl, o, msg);
		else
		    nmflag = ((struct opUpdate *) msg)->opu_Flags;

		/* change 'em, only if changed (or if a "non-interim" message. */
		if (setListViewMAttrs (cl, o, (struct opSet *) msg) || !(nmflag & OPUF_INTERIM))
		{
		    selt = TAG_IGNORE;
		    if (lod->lod_Flags & LODF_SELECT)
		    {
			selt = GTLV_Selected;
			lod->lod_Flags &= ~LODF_SELECT;
		    }

		    if (nmflag & OPUF_INTERIM)
		    {
			if (ounitwidth == (LONG) lod->lod_UnitWidth)
			{
			    unitwidtht = TAG_IGNORE;
			}

			if (ounitheight == (LONG) lod->lod_UnitHeight)
			{
			    unitheightt = TAG_IGNORE;
			}

			if (olist == (ULONG) lod->lod_List)
			{
			    listt = TAG_IGNORE;
			}


			if (otopv == lod->lod_TopVert)
			{
			    topvt = TAG_IGNORE;
			}

			if (ovisv == lod->lod_VisibleVert)
			{
			    visvt = TAG_IGNORE;
			}

			if (ototv == lod->lod_TotalVert)
			{
			    totvt = TAG_IGNORE;
			}


			if (otoph == lod->lod_TopHoriz)
			{
			    topht = TAG_IGNORE;
			}

			if (ovish == lod->lod_VisibleHoriz)
			{
			    visht = TAG_IGNORE;
			}

			if (ototh == lod->lod_TotalHoriz)
			{
			    totht = TAG_IGNORE;
			}
		    }

		    /* Let everyone know about the changes */
		    notifyAttrChanges (o,
				       ((struct opSet *) msg)->ops_GInfo,
				       (nmflag & OPUF_INTERIM),
				       GA_ID, lod->lod_ID,
				       totvt, lod->lod_TotalVert,
				       topvt, lod->lod_TopVert,
				       visvt, lod->lod_VisibleVert,
				       totht, lod->lod_TotalHoriz,
				       topht, lod->lod_TopHoriz,
				       visht, lod->lod_VisibleHoriz,
				       unitheightt, (LONG) lod->lod_UnitHeight,
				       unitwidtht, (LONG) lod->lod_UnitWidth,
				       listt, (ULONG) lod->lod_List,
				       selt, lod->lod_Selected,
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

ULONG setListViewMAttrs (Class * cl, Object * o, struct opSet * msg)
{
    struct modelObjData *lod = INST_DATA (cl, o);
    struct TagItem *tags = msg->ops_AttrList;
    struct GadgetInfo *gi = msg->ops_GInfo;
    struct TagItem *tstate;
    struct List *newlist;
    struct TagItem *tag;
    ULONG retval = 0L;
    ULONG tidata;
    LONG newvtop;
    LONG newhtop;
    BOOL newbox = FALSE;

    lod->lod_FromID = GetTagData (GA_ID, -1L, tags);
    DB (kprintf ("GA_ID=%ld\n", lod->lod_FromID));

    newvtop = lod->lod_TopVert;
    newhtop = lod->lod_TopHoriz;
    newlist = lod->lod_List;

    tstate = tags;
    while (tag = NextTagItem (&tstate))
    {
	DB (kprintf ("0x%lx : 0x%lx : %ld\n", o, tag->ti_Tag, tag->ti_Data));
	tidata = tag->ti_Data;
	switch (tag->ti_Tag)
	{
	    case AOLV_SpecialInfo:
		lod->lod_SI = (struct LVSpecialInfo *) tidata;
		break;

	    case GTLV_Labels:
	    case AOLV_List:
		newlist = (struct List *) tidata;
		break;

	    case GTLV_Selected:
		DB (kprintf ("GTLV_Selected    : %ld\n", tidata));
		lod->lod_Flags |= LODF_SELECT;
		lod->lod_Selected = tidata;
		retval = 1;
		break;

	    case AOLV_View:
		/* Copy it over */
		lod->lod_View = *((struct IBox *) tidata);
		newbox = TRUE;
		retval = 1;
		break;

		/* Vertical Scrollbar */

	    case AOLV_TopVert:
		newvtop = tidata;
		break;

	    case AOLV_VisibleVert:
		lod->lod_VisibleVert = tidata;
		retval = 1L;
		break;

	    case AOLV_TotalVert:
		lod->lod_TotalVert = tidata;
		retval = 1L;
		break;

	    case AOLV_UnitHeight:
		lod->lod_UnitHeight = (WORD) tidata;
		retval = 1;
		break;

		/* Horizontal Scrollbar */

	    case AOLV_TopHoriz:
		newhtop = tidata;
		break;

	    case AOLV_VisibleHoriz:
		lod->lod_VisibleHoriz = tidata;
		retval = 1L;
		break;

	    case AOLV_TotalHoriz:
		lod->lod_TotalHoriz = tidata;
		retval = 1L;
		break;

	    case AOLV_UnitWidth:
		lod->lod_UnitWidth = (WORD) tidata;
		retval = 1;
		break;

	    default:
		break;
	}
    }

    if ((lod->lod_UnitHeight == -1) &&
	(gi && gi->gi_DrInfo && gi->gi_DrInfo->dri_Font))
    {
	lod->lod_UnitHeight = (WORD) gi->gi_DrInfo->dri_Font->tf_YSize + 1;
	newbox = TRUE;
    }

    if (newlist != lod->lod_List)
    {
	if ((lod->lod_List = newlist) && (lod->lod_List != (struct List *) ~ 0))
	{
	    /* Count total number of lines in list */
	    lod->lod_TotalVert = CountEntries (newlist);
	    newbox = TRUE;
	}
	retval = 1;
    }

    if (newbox)
    {
	if (lod->lod_UnitHeight > 0)
	{
	    lod->lod_VisibleVert =
	      MAX (1, (lod->lod_View.Height / (LONG) lod->lod_UnitHeight));
	}

	if (lod->lod_UnitWidth > 0)
	{
	    lod->lod_VisibleHoriz =
	      MAX (1, (lod->lod_View.Width / (LONG) lod->lod_UnitWidth));
	}
    }

    /* limit lod_TopVert to lod_TotalVert-lod_VisibleVert */
    if ((newvtop > (lod->lod_TotalVert - lod->lod_VisibleVert)) &&
	(lod->lod_TotalVert >= 0) && (lod->lod_VisibleVert >= 0))
    {
	newvtop = lod->lod_TotalVert - lod->lod_VisibleVert;
    }

    if (newvtop < 0)
    {
	newvtop = 0;
    }

    if (lod->lod_TopVert != newvtop)
    {
	lod->lod_TopVert = newvtop;
	retval = 1L;
    }

    /* limit lod_TopHoriz to lod_TotalHoriz-lod_VisibleHoriz */
    if ((newhtop > (lod->lod_TotalHoriz - lod->lod_VisibleHoriz)) &&
	(lod->lod_TotalHoriz >= 0) && (lod->lod_VisibleHoriz >= 0))
    {
	newhtop = lod->lod_TotalHoriz - lod->lod_VisibleHoriz;
    }

    if (newhtop < 0)
    {
	newhtop = 0;
    }

    if (lod->lod_TopHoriz != newhtop)
    {
	lod->lod_TopHoriz = newhtop;
	retval = 1L;
    }
    DB (kprintf ("\n"));

    return (retval);
}

ULONG getListViewMAttr (Class * cl, Object * o, struct opGet * msg)
{
    struct modelObjData *lod = INST_DATA (cl, o);
    ULONG retval = 1L;

    switch (msg->opg_AttrID)
    {
	case GTLV_Selected:
	    *msg->opg_Storage = lod->lod_Selected;
	    break;

	case AOLV_TopVert:
	    *msg->opg_Storage = lod->lod_TopVert;
	    break;

	case AOLV_TotalVert:
	    *msg->opg_Storage = lod->lod_TotalVert;
	    break;

	case AOLV_VisibleVert:
	    *msg->opg_Storage = lod->lod_VisibleVert;
	    break;

	case AOLV_TopHoriz:
	    *msg->opg_Storage = lod->lod_TopHoriz;
	    break;

	case AOLV_TotalHoriz:
	    *msg->opg_Storage = lod->lod_TotalHoriz;
	    break;

	case AOLV_VisibleHoriz:
	    *msg->opg_Storage = lod->lod_VisibleHoriz;
	    break;

	default:
	    /* I don't recognize this one, let the superclass try */
	    retval = (ULONG) DSM (cl, o, msg);
	    break;
    }

    return (retval);
}
