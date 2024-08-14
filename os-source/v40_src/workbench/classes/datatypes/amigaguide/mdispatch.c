/* mdispatch.c
 *
 */

#include "amigaguidebase.h"
#include <intuition/icclass.h>

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

struct modelData
{
    Class		*md_Class;		/* Main class */
    Object		*md_PObject;		/* Main object */
    Object		*md_EObject;
    struct ClientData	*md_ClientData;		/* Main object client data */
    STRPTR		 md_WordSelect;
    ULONG		 md_Flags;

    ULONG		 md_TopVert;
    ULONG		 md_VisVert;
    ULONG		 md_TotVert;
    ULONG		 md_OTopVert;
    ULONG		 md_VertUnit;

    ULONG		 md_TopHoriz;
    ULONG		 md_VisHoriz;
    ULONG		 md_TotHoriz;
    ULONG		 md_OTopHoriz;
    ULONG		 md_HorizUnit;
};

#define	MDF_WORDSELECT	(1L<<0)

/*****************************************************************************/

ULONG notifyChanges (Object * o, VOID * ginfo, ULONG flags, ULONG tag1,...)
{
    return DoMethod (o, OM_NOTIFY, &tag1, ginfo, flags);
}

/*****************************************************************************/

ULONG setgattrs (Object * o, VOID * ginfo, ULONG tag1,...)
{
    return DoMethod (o, OM_SET, &tag1, ginfo);
}

/*****************************************************************************/

Class *initModelClass (struct AGLib * ag)
{
    Class *cl;

    if (cl = MakeClass (NULL, MODELCLASS, NULL, sizeof (struct modelData), 0L))
    {
	cl->cl_Dispatcher.h_Entry = (ULONG (*)()) ModelDispatch;
	cl->cl_UserData           = (ULONG) ag;
    }

    return (cl);
}

/*****************************************************************************/

ULONG ASM ModelDispatch (REG (a0) Class * cl, REG (a2) Object * o, REG (a1) Msg msg)
{
    struct AGLib *ag = (struct AGLib *) cl->cl_UserData;
    struct modelData *md = INST_DATA (cl, o);
    ULONG retval = 0L;
    ULONG nmf = 0;
    ULONG intf;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    if (retval = DoSuperMethodA (cl, o, msg))
		setModelAttrs (ag, cl, (Object *) retval, (struct opSet *) msg);
	    break;

	case OM_GET:
	    retval = getModelAttr (ag, cl, o, (struct opGet *) msg);
	    break;

	case OM_SET:
	case OM_UPDATE:
#if 1
	    if (!DoSuperMethod (cl, o, ICM_CHECKLOOP))
	    {
		/* let the superclass see whatever it wants from OM_SET, such
		 * as ICA_TARGET, ICA_MAP, and so on.  For OM_NOTIFY, however,
		 * we control all traffic and issue notification specifically
		 * for the attributes we're interested in. */
		if (msg->MethodID == OM_SET)
		{
		    DoSuperMethodA (cl, o, msg);
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
		if (setModelAttrs (ag, cl, o, (struct opSet *) msg) || !intf)
		{
		    /* Pass along GInfo, if any, so gadgets can redraw
		     * themselves.  Pass along opu_Flags, so that the
		     * application will know the difference between and interim
		     * message and a final message */
		    setgattrs (md->md_PObject, ((struct opSet *) msg)->ops_GInfo,
				   DTA_TopVert,		md->md_TopVert,
				   DTA_VisibleVert,	md->md_VisVert,
				   DTA_TotalVert,	md->md_TotVert,

				   DTA_TopHoriz,	md->md_TopHoriz,
				   DTA_VisibleHoriz,	md->md_VisHoriz,
				   DTA_TotalHoriz,	md->md_TotHoriz,
				   TAG_END);
		}
	    }
#else
	    if (!DoSuperMethod (cl, o, ICM_CHECKLOOP))
	    {
		ULONG topvt = DTA_TopVert,	otopv = md->md_TopVert;
		ULONG visvt = DTA_VisibleVert,	ovisv = md->md_VisVert;
		ULONG totvt = DTA_TotalVert,	ototv = md->md_TotVert;
		ULONG topht = DTA_TopHoriz,	otoph = md->md_TopHoriz;
		ULONG visht = DTA_VisibleHoriz,	ovish = md->md_VisHoriz;
		ULONG totht = DTA_TotalHoriz,	ototh = md->md_TotHoriz;
		ULONG wordt = TAG_IGNORE;
		ULONG synct = TAG_IGNORE;

		/* let the superclass see whatever it wants from OM_SET, such
		 * as ICA_TARGET, ICA_MAP, and so on.  For OM_NOTIFY, however,
		 * we control all traffic and issue notification specifically
		 * for the attributes we're interested in. */
		if (msg->MethodID == OM_SET)
		{
		    DoSuperMethodA (cl, o, msg);
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
		if (setModelAttrs (ag, cl, o, (struct opSet *) msg) || !intf)
		{
		    if (!FindTagItem (DTA_Sync, ((struct opSet *)msg)->ops_AttrList))
		    {
			if (otopv == md->md_TopVert)
			    topvt = TAG_IGNORE;
			if (ovisv == md->md_VisVert)
			    visvt = TAG_IGNORE;
			if (ototv == md->md_TotVert)
			    totvt = TAG_IGNORE;
			if (otoph == md->md_TopHoriz)
			    topht = TAG_IGNORE;
			if (ovish == md->md_VisHoriz)
			    visht = TAG_IGNORE;
			if (ototh == md->md_TotHoriz)
			    totht = TAG_IGNORE;
		    }
		    else
		    {
			synct = DTA_Sync;
		    }

		    if (md->md_WordSelect)
			wordt = TDTA_WordSelect;

		    /* Pass along GInfo, if any, so gadgets can redraw
		     * themselves.  Pass along opu_Flags, so that the
		     * application will know the difference between and interim
		     * message and a final message */
		    notifyChanges (o, ((struct opSet *) msg)->ops_GInfo, intf,
				   topvt, md->md_TopVert,
				   visvt, md->md_VisVert,
				   totvt, md->md_TotVert,
				   topht, md->md_TopHoriz,
				   visht, md->md_VisHoriz,
				   totht, md->md_TotHoriz,
				   wordt, md->md_WordSelect,
				   synct, TRUE,
				   TAG_END);
		}
	    }
#endif

            if ((md->md_Flags & MDF_WORDSELECT) && md->md_ClientData && md->md_ClientData->cd_SIPCPort)
            {
                struct SIPCMsg *sm;
                ULONG msize;

                md->md_Flags ^= MDF_WORDSELECT;
                msize = sizeof (struct SIPCMsg) + (strlen (md->md_WordSelect) * 2) + 14;
                if (sm = AllocVec (msize, MEMF_CLEAR))
                {
                    sm->sm_Message.mn_Node.ln_Type = NT_MESSAGE;
                    sm->sm_Message.mn_Length      = msize;
                    sm->sm_Type = SIPCT_COMMAND;
                    sm->sm_Data = MEMORY_FOLLOWING (sm);
                    sprintf (sm->sm_Data, "\"%s\" Link \"%s\"", md->md_WordSelect, md->md_WordSelect);
                    PutMsg (md->md_ClientData->cd_SIPCPort, (struct Message *) sm);
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

/* Inquire attribute of an object */
ULONG getModelAttr (struct AGLib * ag, Class * cl, Object * o, struct opGet * msg)
{
    struct modelData *md = INST_DATA (cl, o);

    switch (msg->opg_AttrID)
    {
	case DTA_TopVert:
	    *msg->opg_Storage = (ULONG) md->md_TopVert;
	    break;
	case DTA_VisibleVert:
	    *msg->opg_Storage = (ULONG) md->md_VisVert;
	    break;
	case DTA_TotalVert:
	    *msg->opg_Storage = (ULONG) md->md_TotVert;
	    break;
	case DTA_VertUnit:
	    *msg->opg_Storage = (ULONG) md->md_VertUnit;
	    break;

	case DTA_TopHoriz:
	    *msg->opg_Storage = (ULONG) md->md_TopHoriz;
	    break;
	case DTA_VisibleHoriz:
	    *msg->opg_Storage = (ULONG) md->md_VisHoriz;
	    break;
	case DTA_TotalHoriz:
	    *msg->opg_Storage = (ULONG) md->md_TotHoriz;
	    break;
	case DTA_HorizUnit:
	    *msg->opg_Storage = (ULONG) md->md_HorizUnit;
	    break;

	default:
	    return (DoSuperMethodA (cl, o, msg));
    }

    return (1L);
}

/*****************************************************************************/

/* Set attributes of an object */
ULONG setModelAttrs (struct AGLib * ag, Class * cl, Object * o, struct opSet * msg)
{
    struct TagItem *tags = msg->ops_AttrList;
    struct modelData *md = INST_DATA (cl, o);
    struct TagItem *tstate;
    struct TagItem *tag;
    ULONG refresh = 0L;
    ULONG tidata;

    /* process rest */
    md->md_WordSelect = NULL;
    tstate = tags;
    while (tag = NextTagItem (&tstate))
    {
	tidata = tag->ti_Data;
	switch (tag->ti_Tag)
	{
	    case DBA_Class:
		md->md_Class = (Class *) tidata;
		break;

	    case DBA_Object:
		md->md_PObject = (Object *) tidata;
		break;

	    case DBA_EmbeddedObject:
		md->md_EObject = (Object *) tidata;
		break;

	    case DBA_ClientData:
		md->md_ClientData = (struct ClientData *) tidata;
		break;

	    case TDTA_WordSelect:
		if (md->md_WordSelect = (STRPTR) tidata)
		    md->md_Flags |= MDF_WORDSELECT;
		break;

	    case DTA_TopVert:
		md->md_TopVert  = tidata;
		refresh = 1L;
		break;
	    case DTA_VisibleVert:
		md->md_VisVert  = tidata;
		refresh = 1L;
		break;
	    case DTA_TotalVert:
		md->md_TotVert  = tidata;
		refresh = 1L;
		break;
	    case DTA_VertUnit:
		md->md_VertUnit  = tidata;
		refresh = 1L;
		break;

	    case DTA_TopHoriz:
		md->md_TopHoriz = tidata;
		refresh = 1L;
		break;
	    case DTA_VisibleHoriz:
		md->md_VisHoriz  = tidata;
		refresh = 1L;
		break;
	    case DTA_TotalHoriz:
		md->md_TotHoriz  = tidata;
		refresh = 1L;
		break;
	    case DTA_HorizUnit:
		md->md_HorizUnit  = tidata;
		refresh = 1L;
		break;
	}
    }

    if (md->md_EObject)
    {
	struct DTSpecialInfo *esi = (struct DTSpecialInfo *) G (md->md_EObject)->SpecialInfo;

	/* Get the most up to the second information possible */
	md->md_TopVert   = esi->si_TopVert;
	md->md_VisVert   = esi->si_VisVert;
	md->md_TotVert   = esi->si_TotVert;
	md->md_VertUnit  = esi->si_VertUnit;

	md->md_TopHoriz  = esi->si_TopHoriz;
	md->md_VisHoriz  = esi->si_VisHoriz;
	md->md_TotHoriz  = esi->si_TotHoriz;
	md->md_HorizUnit = esi->si_HorizUnit;
    }

    return (refresh);
}
