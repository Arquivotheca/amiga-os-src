/* windowmclass.c
 *
 */

#include "amigaguidebase.h"

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

struct modelObjData
{
    LONG	 lod_TotVert;
    LONG	 lod_VisVert;
    LONG	 lod_TopVert;

    LONG	 lod_TotHoriz;
    LONG	 lod_VisHoriz;
    LONG	 lod_TopHoriz;

    ULONG	 lod_ID;
};

/*****************************************************************************/

ULONG notifyAttrChanges (struct AmigaGuideLib * ag, Object * o, VOID * ginfo, ULONG flags, ULONG tag1,...)
{
    return DoMethod (o, OM_NOTIFY, &tag1, ginfo, flags);
}

/*****************************************************************************/

Class *initWindowMClass (struct AmigaGuideLib * ag)
{
    extern ULONG hookEntry ();
    Class *cl;

    if (cl = MakeClass (NULL, MODELCLASS, NULL, sizeof (struct modelObjData), 0L))
    {
	cl->cl_Dispatcher.h_Entry = (ULONG (*)()) dispatchWindowMClass;
	cl->cl_UserData = (ULONG) ag;
    }

    return (cl);
}

/*****************************************************************************/

ULONG freeWindowMClass (Class * cl)
{
    struct AmigaGuideLib *ag = (struct AmigaGuideLib *) cl->cl_UserData;
    return ((ULONG) FreeClass (cl));
}

/*****************************************************************************/

ULONG ASM dispatchWindowMClass (REG (a0) Class * cl, REG (a2) Object * o, REG (a1) Msg msg)
{
    struct AmigaGuideLib *ag = (struct AmigaGuideLib *) cl->cl_UserData;
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

		/* Get the gadget ID */
		lod->lod_ID = GetTagData (GA_ID, NULL, ops->ops_AttrList);

		/* Initialize instance data */
		setWindowMClassAttrs (ag, cl, newobj, ops);
	    }
	    retval = (ULONG) newobj;
	    break;

	case OM_GET:
	    retval = (ULONG) getWindowMClassAttr (ag, cl, o, (struct opGet *) msg);
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
		if ((retval = setWindowMClassAttrs (ag, cl, o, ops)) || !(nmflag & OPUF_INTERIM))
		{
		    if (!(FindTagItem (DTA_Sync, ops->ops_AttrList) || FindTagItem (WOA_Sync, ops->ops_AttrList)))
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
		    notifyAttrChanges (ag, o, ops->ops_GInfo, (nmflag & OPUF_INTERIM),
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

ULONG setWindowMClassAttrs (struct AmigaGuideLib *ag, Class * cl, Object * o, struct opSet * msg)
{
    struct modelObjData *lod = INST_DATA (cl, o);
    struct TagItem *tags = msg->ops_AttrList;
    struct TagItem *tstate;
    struct TagItem *tag;
    ULONG retval = 0L;
    LONG newHoriz;
    ULONG tidata;
    LONG newVert;

    /* start with original value */
    newHoriz = lod->lod_TopHoriz;
    newVert = lod->lod_TopVert;

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

	    case WOA_TotVert:
#if 0
		lod->lod_TotVert = tidata;
		retval = 1L;
#else
		if (lod->lod_TotVert != tidata)
		{
		    lod->lod_TotVert = tidata;
		    retval = 1L;
		}
#endif
		break;

	    case WOA_VisVert:
#if 0
		lod->lod_VisVert = tidata;
		retval = 1L;
#else
		if (lod->lod_VisVert != tidata)
		{
		    lod->lod_VisVert = tidata;
		    retval = 1L;
		}
#endif
		break;

	    case WOA_TopVert:
		newVert = tidata;
		break;

	    case WOA_DecVert:
		newVert--;
		break;

	    case WOA_IncVert:
		newVert++;
		break;

	    case WOA_TotHoriz:
#if 0
		lod->lod_TotHoriz = tidata;
		retval = 1L;
#else
		if (lod->lod_TotHoriz != tidata)
		{
		    lod->lod_TotHoriz = tidata;
		    retval = 1L;
		}
#endif
		break;

	    case WOA_VisHoriz:
#if 0
		lod->lod_VisHoriz = tidata;
		retval = 1L;
#else
		if (lod->lod_VisHoriz != tidata)
		{
		    lod->lod_VisHoriz = tidata;
		    retval = 1L;
		}
#endif
		break;

	    case WOA_TopHoriz:
		newHoriz = tidata;
		break;

	    case WOA_DecHoriz:
		newHoriz--;
		break;

	    case WOA_IncHoriz:
		newHoriz++;
		break;
	}
    }

    /* VERTICAL Validity check */
    if (lod->lod_VisVert <= 0)
    {
	lod->lod_VisVert = 1;
	retval = 1L;
    }
    if (lod->lod_TotVert <= 0)
    {
	lod->lod_TotVert = 1;
	retval = 1L;
    }
    if (newVert > (lod->lod_TotVert - lod->lod_VisVert))
	newVert = lod->lod_TotVert - lod->lod_VisVert;
    if (newVert < 0)
	newVert = 0;
    if (lod->lod_TopVert != newVert)
    {
	lod->lod_TopVert = newVert;
	retval = 1L;
    }

    /* HORIZONTAL Validity check */
    if (lod->lod_VisHoriz <= 0)
    {
	lod->lod_VisHoriz = 1;
	retval = 1L;
    }
    if (lod->lod_TotHoriz <= 0)
    {
	lod->lod_TotHoriz = 1;
	retval = 1L;
    }
    if (newHoriz > (lod->lod_TotHoriz - lod->lod_VisHoriz))
	newHoriz = lod->lod_TotHoriz - lod->lod_VisHoriz;
    if (newHoriz < 0)
	newHoriz = 0;
    if (lod->lod_TopHoriz != newHoriz)
    {
	lod->lod_TopHoriz = newHoriz;
	retval = 1L;
    }

    return (retval);
}

/*****************************************************************************/

ULONG getWindowMClassAttr (struct AmigaGuideLib *ag, Class * cl, Object * o, struct opGet * msg)
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
	    retval = (ULONG) DoSuperMethodA (cl, o, (Msg) msg);
	    break;
    }

    return (retval);
}
