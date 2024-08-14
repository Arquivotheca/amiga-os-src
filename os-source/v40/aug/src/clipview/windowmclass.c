/* windowmclass.c
 *
 */

#include "clipview.h"

/*****************************************************************************/

struct modelObjData
{
    LONG lod_TotVert;
    LONG lod_VisVert;
    LONG lod_TopVert;

    LONG lod_TotHoriz;
    LONG lod_VisHoriz;
    LONG lod_TopHoriz;

    ULONG lod_ID;
};

/*****************************************************************************/

ULONG notifyAttrChanges (struct GlobalData * gd, Object * o, VOID * ginfo, ULONG flags, ULONG tag1,...)
{
    return DoMethod (o, OM_NOTIFY, &tag1, ginfo, flags);
}

/*****************************************************************************/

Class *initWindowMClass (struct GlobalData * gd)
{
    extern ULONG hookEntry ();
    Class *cl;

    if (cl = MakeClass (NULL, MODELCLASS, NULL, sizeof (struct modelObjData), 0L))
    {
	cl->cl_Dispatcher.h_Entry = dispatchWindowMClass;
	cl->cl_UserData = (ULONG) gd;
    }

    return (cl);
}

/*****************************************************************************/

ULONG freeWindowMClass (Class * cl)
{
    struct GlobalData *gd = (struct GlobalData *) cl->cl_UserData;

    return ((ULONG) FreeClass (cl));
}

/*****************************************************************************/

ULONG ASM dispatchWindowMClass (REG (a0) Class * cl, REG (a2) Object * o, REG (a1) Msg msg)
{
    struct GlobalData *gd = (struct GlobalData *) cl->cl_UserData;
    struct modelObjData *lod = INST_DATA (cl, o);
    struct opSet *ops = (struct opSet *) msg;
    ULONG retval = 0L;
    Object *newobj;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    /* Have the superclass create the object */
	    if (newobj = (Object *) DoSuperMethodA (cl, o, msg))
	    {
		/* Get the local object data */
		lod = INST_DATA (cl, newobj);
		lod->lod_TopVert = lod->lod_TopHoriz = -1;

		/* Get the gadget ID */
		lod->lod_ID = GetTagData (GA_ID, NULL, ops->ops_AttrList);

		/* Initialize instance data */
		setWindowMClassAttrs (gd, cl, newobj, ops);
	    }
	    retval = (ULONG) newobj;
	    break;

	case OM_GET:
	    retval = (ULONG) getWindowMClassAttr (gd, cl, o, (struct opGet *) msg);
	    break;

	case OM_SET:
	case OM_UPDATE:
	    if (!DoSuperMethod (cl, o, ICM_CHECKLOOP))
	    {
		ULONG topvt = DTA_TopVert,	otopv = lod->lod_TopVert;
		ULONG visvt = DTA_VisibleVert,	ovisv = lod->lod_VisVert;
		ULONG totvt = DTA_TotalVert,	ototv = lod->lod_TotVert;
		ULONG topht = DTA_TopHoriz,	otoph = lod->lod_TopHoriz;
		ULONG visht = DTA_VisibleHoriz,	ovish = lod->lod_VisHoriz;
		ULONG totht = DTA_TotalHoriz,	ototh = lod->lod_TotHoriz;
		ULONG nmflag = 0L;

		/*
		 * let the superclass see whatever it wants from OM_SET, such
		 * as ICA_TARGET, ICA_MAP, and so on.  For OM_NOTIFY, however,
		 * we control all traffic and issue notification specifically
		 * for the attributes we're interested in.
		 */
		if (msg->MethodID == OM_SET)
		{
		    DoSuperMethodA (cl, o, msg);
		}
		else
		{
		    /* these flags aren't present in the message of OM_SET	 */
		    nmflag = ((struct opUpdate *) msg)->opu_Flags;
		}

		/* change 'em, only if changed (or if a "non-interim" message. */
		if ((retval = setWindowMClassAttrs (gd, cl, o, ops)) || !(nmflag & OPUF_INTERIM))
		{
		    if (!(FindTagItem (WOA_Sync, ops->ops_AttrList) || FindTagItem (DTA_Sync, ops->ops_AttrList)))
		    {
			if (otopv == lod->lod_TopVert)
			    topvt = TAG_IGNORE;
			if (ovisv == lod->lod_VisVert)
			    visvt = TAG_IGNORE;
			if (ototv == lod->lod_TotVert)
			    totvt = TAG_IGNORE;
			if (otoph == lod->lod_TopHoriz)
			    topht = TAG_IGNORE;
			if (ovish == lod->lod_VisHoriz)
			    visht = TAG_IGNORE;
			if (ototh == lod->lod_TotHoriz)
			    totht = TAG_IGNORE;
		    }

		    /*
		     * Pass along GInfo, if any, so gadgets can redraw
		     * themselves.  Pass along opu_Flags, so that the
		     * application will know the difference between and
		     * interim message and a final message
		     */
		    notifyAttrChanges (gd, o, ops->ops_GInfo, (nmflag & OPUF_INTERIM),
				       GA_ID, lod->lod_ID,
				       topvt, lod->lod_TopVert,
				       visvt, lod->lod_VisVert,
				       totvt, lod->lod_TotVert,
				       topht, lod->lod_TopHoriz,
				       visht, lod->lod_VisHoriz,
				       totht, lod->lod_TotHoriz,
				       TAG_END);
		}
	    }
	    break;

	default:
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    break;
    }

    return (retval);
}

/*****************************************************************************/

ULONG setWindowMClassAttrs (struct GlobalData *gd, Class * cl, Object * o, struct opSet * msg)
{
    struct modelObjData *lod = INST_DATA (cl, o);
    struct TagItem *tags = msg->ops_AttrList;
    struct TagItem *tstate;
    struct TagItem *tag;
    ULONG retval = 0L;
    ULONG tidata;

    /* Process tags */
    tstate = tags;
    while (tag = NextTagItem (&tstate))
    {
	tidata = tag->ti_Data;
	switch (tag->ti_Tag)
	{
	    case WOA_Sync:
	    case DTA_Sync:
		retval = 1L;
		break;

	    case WOA_TopVert:
		if (lod->lod_TopVert != tidata)
		{
		    lod->lod_TopVert = tidata;
		    retval = 1L;
		}
		break;

	    case WOA_VisVert:
		if (lod->lod_VisVert != tidata)
		{
		    lod->lod_VisVert = tidata;
		    retval = 1L;
		}
		break;

	    case WOA_TotVert:
		if (lod->lod_TotVert != tidata)
		{
		    lod->lod_TotVert = tidata;
		    retval = 1L;
		}
		break;

	    case WOA_DecVert:
		lod->lod_TopVert--;
		break;

	    case WOA_IncVert:
		lod->lod_TopVert++;
		break;

	    case WOA_TopHoriz:
		if (lod->lod_TopHoriz != tidata)
		{
		    lod->lod_TopHoriz = tidata;
		    retval = 1L;
		}
		break;

	    case WOA_VisHoriz:
		if (lod->lod_VisHoriz != tidata)
		{
		    lod->lod_VisHoriz = tidata;
		    retval = 1L;
		}
		break;

	    case WOA_TotHoriz:
		if (lod->lod_TotHoriz != tidata)
		{
		    lod->lod_TotHoriz = tidata;
		    retval = 1L;
		}
		break;

	    case WOA_DecHoriz:
		lod->lod_TopHoriz--;
		break;

	    case WOA_IncHoriz:
		lod->lod_TopHoriz++;
		break;
	}
    }

    if (lod->lod_TopHoriz < 0)
	lod->lod_TopHoriz = 0;
    if (lod->lod_TopHoriz > lod->lod_TotHoriz - lod->lod_VisHoriz)
	lod->lod_TopHoriz = lod->lod_TotHoriz - lod->lod_VisHoriz;

    if (lod->lod_TopVert < 0)
	lod->lod_TopVert = 0;
    if (lod->lod_TopVert > lod->lod_TotVert - lod->lod_VisVert)
	lod->lod_TopVert = lod->lod_TotVert - lod->lod_VisVert;

    return (retval);
}

/*****************************************************************************/

ULONG getWindowMClassAttr (struct GlobalData *gd, Class * cl, Object * o, struct opGet * msg)
{
    struct modelObjData *lod = INST_DATA (cl, o);
    ULONG retval = 1L;


    switch (msg->opg_AttrID)
    {
	case WOA_TopVert:
	    *msg->opg_Storage = lod->lod_TopVert;
	    break;

	case WOA_VisVert:
	    *msg->opg_Storage = lod->lod_VisVert;
	    break;

	case WOA_TotVert:
	    *msg->opg_Storage = lod->lod_TotVert;
	    break;

	case WOA_TopHoriz:
	    *msg->opg_Storage = lod->lod_TopHoriz;
	    break;

	case WOA_VisHoriz:
	    *msg->opg_Storage = lod->lod_VisHoriz;
	    break;

	case WOA_TotHoriz:
	    *msg->opg_Storage = lod->lod_TotHoriz;
	    break;

	default:
	    /* I don't recognize this one, let the superclass try */
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    break;
    }

    return (retval);
}
