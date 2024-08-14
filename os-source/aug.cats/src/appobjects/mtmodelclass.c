/* mtmodelclass.c
 * model for multiline text gadget
 */

#include "appobjectsbase.h"

Class *initMTModClass (VOID);
ULONG freeMTModClass (Class *cl);
ULONG __saveds dispatchMTMod (Class *cl, Object *o, Msg msg);
ULONG setMTModAttrs (Class *cl, Object *o, struct opSet *msg);
ULONG getMTModAttr (Class *cl, Object *o, struct opGet *msg);
ULONG notifyAttrChanges (Object *o, VOID *ginfo, ULONG flags, ULONG tag1, ...);

/* private class */
#define MYCLASSID	(NULL)

/* if condition is false, replace tag with TAG_IGNORE */
#define XTAG( expr, tagid ) ((expr)? (tagid): TAG_IGNORE)

#define SUPERCLASSID	MODELCLASS

struct localObjData
{
    LONG lod_Top;
    LONG lod_Total;
    LONG lod_Visible;
    STRPTR lod_TextVal;
};

Class *initMTModClass (VOID)
{
    ULONG __saveds dispatchMTMod ();
    Class *MakeClass ();
    ULONG hookEntry ();
    Class *cl;

    if (cl = MakeClass (MYCLASSID,SUPERCLASSID, NULL,sizeof (struct localObjData),0))
    {
	cl->cl_Dispatcher.h_Entry = hookEntry;
	cl->cl_Dispatcher.h_SubEntry = dispatchMTMod;
    }
    return (cl);
}

ULONG freeMTModClass (Class *cl)
{
    return ((ULONG)FreeClass (cl));
}

ULONG __saveds dispatchMTMod (Class *cl, Object *o, Msg msg)
{
    struct localObjData *lod;
    Object *newobj;
    ULONG nmf = 0;
    ULONG intf;

    lod = INST_DATA (cl, o);

    switch (msg->MethodID)
    {
	/* use superclass defaults for everything else */
	case OM_NEW:
	    if (newobj = (Object *) DSM (cl, o, msg))
	    {
		/* initialize instance data (they start life as 0) */
		setMTModAttrs (cl, newobj, (struct opSet *) msg);
	    }
	    return ((ULONG) newobj);

	case OM_GET:
	    return ((ULONG) getMTModAttr (cl, o, (struct opGet *) msg));

	case OM_SET:
	case OM_UPDATE:
	    if (!DoSuperMethod (cl, o, ICM_CHECKLOOP))
	    {
		/* let the superclass see whatever it wants from OM_SET, such
		 * as ICA_TARGET, ICA_MAP, and so on.  For OM_NOTIFY, however,
		 * we control all traffic and issue notification specifically
		 * for the attributes we're interested in. */
		if (msg->MethodID == OM_SET)
		{
		    DSM (cl, o, msg);
		}
		else
		{
		    /* these flags aren't present in the message of OM_SET */
		    nmf = ((struct opUpdate *) msg)->opu_Flags;
		}

		/* I'll be wanting to know this is an "interim" message or a
		 * final report (which I always want to send, even if the value
		 * of lod_Top hasn't changed). */
		intf = nmf & OPUF_INTERIM;

		/* change 'em, only if changed (or if a "non-interim" message. */
		if (setMTModAttrs (cl, o, (struct opSet *) msg) || !intf)
		{
		    /* Pass along GInfo, if any, so gadgets can redraw
		     * themselves.  Pass along opu_Flags, so that the
		     * application will know the difference between and interim
		     * message and a final message */
		    notifyAttrChanges (o, ((struct opSet *) msg)->ops_GInfo,
				       intf,
				       PGA_Total, lod->lod_Total,
				       PGA_Top, lod->lod_Top,
				       PGA_Visible, lod->lod_Visible,
#if 0
				       STRINGA_TextVal, lod->lod_TextVal,
#endif
				       TAG_END);
		}
	    }
	    break;

	case OM_NOTIFY:
	case OM_DISPOSE:
	default:
	    return ((ULONG) DSM (cl, o, msg));
    }
    return (1L);
}

ULONG setMTModAttrs (Class *cl, Object *o, struct opSet *msg)
{
    struct localObjData *lod = INST_DATA (cl, o);
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
	    case STRINGA_TextVal:
		if (lod->lod_TextVal != (STRPTR) tidata)
		{
		    lod->lod_TextVal = (STRPTR) tidata;
		    retval = 1L;
		}
		break;

	    case PGA_Visible:
		if (lod->lod_Visible != tidata)
		{
		    lod->lod_Visible = (LONG) tidata;
		    retval = 1L;
		}
		break;

	    case PGA_Total:
		if (lod->lod_Total != tidata)
		{
		    lod->lod_Total = (LONG) tidata;
		    retval = 1L;
		}
		break;

	    case CGTA_Increment:
		newval++;
		break;

	    case CGTA_Decrement:
		newval--;
		break;
	}
    }

    /* Validity check */
    if (lod->lod_Visible == 0)
    {
	lod->lod_Visible = 1;
	retval = 1L;
    }

    /* Validity check */
    if (lod->lod_Total == 0)
    {
	lod->lod_Total = 1;
	retval = 1L;
    }

    /* look at "absolute" setting last */
    newval = GetTagData (PGA_Top, newval, tags);

    /* limit lod_Top to lod_Total-lod_Visible */
    if (newval > (lod->lod_Total - lod->lod_Visible))
	newval = lod->lod_Total - lod->lod_Visible;

    if (newval < 0)
	newval = 0;

    if (lod->lod_Top != newval)
    {
	lod->lod_Top = newval;
	retval = 1L;
    }

    return (retval);
}

ULONG getMTModAttr (Class *cl, Object *o, struct opGet *msg)
{
    struct localObjData *lod;

    lod = INST_DATA (cl, o);

    switch (msg->opg_AttrID)
    {
	case STRINGA_TextVal:
	    *msg->opg_Storage = (ULONG) lod->lod_TextVal;
	    break;

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
	    return ((ULONG)DSM (cl, o, msg));
    }
    return (1L);
}
