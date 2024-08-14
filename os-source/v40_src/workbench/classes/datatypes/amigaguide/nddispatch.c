/* nddispatch.c
 *
 */

#include "amigaguidebase.h"

/*****************************************************************************/

#define	DB(x)	;
#define	DH(x)	;

/*****************************************************************************/
#if 0
static void nd_sendCmd (struct AGLib *ag, struct ClientData *cd, STRPTR cmd)
{
    struct SIPCMsg *sm;
    ULONG msize;

    msize = sizeof (struct SIPCMsg) + strlen (cmd) + 1;
    if (sm = AllocVec (msize, MEMF_CLEAR))
    {
	sm->sm_Message.mn_Node.ln_Type = NT_MESSAGE;
	sm->sm_Message.mn_Length       = msize;
	sm->sm_Type = SIPCT_COMMAND;
	sm->sm_Data = MEMORY_FOLLOWING (sm);
	strcpy (sm->sm_Data, cmd);
	PutMsg (cd->cd_SIPCPort, (struct Message *) sm);
    }
}
#endif

/*****************************************************************************/

Class *initHNClass (struct AGLib * ag)
{
    Class *cl;

    if (cl = MakeClass (NULL, ROOTCLASS, NULL, sizeof (struct NodeData), 0L))
    {
	cl->cl_Dispatcher.h_Entry = (ULONG (*)()) HNDispatch;
	cl->cl_UserData           = (ULONG) ag;
    }

    return (cl);
}

/*****************************************************************************/

ULONG ASM HNDispatch (REG (a0) Class * cl, REG (a2) Object * o, REG (a1) Msg msg)
{
    struct AGLib *ag = (struct AGLib *) cl->cl_UserData;
    struct NodeData *nd;
    ULONG i, retval;
    Object *newobj;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    if (newobj = (Object *) DoSuperMethodA (cl, o, msg))
	    {
		if (!(initializeHN (ag, cl, newobj, (struct opSet *) msg)))
		{
		    CoerceMethod (cl, newobj, OM_DISPOSE);
		    newobj = NULL;
		}
	    }
	    retval = (ULONG) newobj;
	    break;

	case OM_SET:
	    retval = setHNAttrs (ag, cl, o, (struct opSet *) msg);
	    break;

	case OM_DISPOSE:
	    nd = INST_DATA (cl, o);

	    if (nd->nd_Font)
		CloseFont (nd->nd_Font);

	    for (i = 0; i < ND_STRINGS; i++)
		FreeVec (nd->nd_Strings[i]);

	    if ((nd->nd_Flags & HNF_FREEBUFFER) && nd->nd_Buffer)
		FreeVec (nd->nd_Buffer);
	    if (nd->nd_Object)
		DisposeDTObject (nd->nd_Object);

	    /* Let the superclass handle everything else */
	default:
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    break;
    }

    return (retval);
}

/*****************************************************************************/

BOOL initializeHN (struct AGLib * ag, Class * cl, Object * o, struct opSet * msg)
{
    struct NodeData *nd = INST_DATA (cl, o);
    struct DatabaseData *dbd;
    Object *mo;
    LONG i;

    InitSemaphore (&nd->nd_Lock);

    if (mo = (Object *) GetTagData (HNA_Database, NULL, msg->ops_AttrList))
    {
	dbd = INST_DATA (ag->ag_DatabaseClass, mo);
	nd->nd_Database = mo;
	nd->nd_Offset   = (-1);

	/* Same font as the database */
	nd->nd_TextAttr = *(&dbd->dbd_TextAttr);
	strcpy (nd->nd_FontName, dbd->dbd_FontName);
	nd->nd_TextAttr.ta_Name = nd->nd_FontName;
	nd->nd_Font = OpenFont (&nd->nd_TextAttr);

	/* Copy the default tab information */
	nd->nd_MinWidth = dbd->dbd_MinWidth;
	nd->nd_DefTabs = dbd->dbd_DefTabs;
	for (i = 0; i < MAX_TABS; i++)
	    nd->nd_Tabs[i] = dbd->dbd_Tabs[i];

	setHNAttrs (ag, cl, o, msg);

	DoMethod (mo, OM_ADDMEMBER, (LONG) o);

	return (TRUE);
    }

    return (FALSE);
}

/*****************************************************************************/

/* Set attributes of an object */
ULONG setHNAttrs (struct AGLib * ag, Class * cl, Object * o, struct opSet * msg)
{
    struct TagItem *tags = msg->ops_AttrList;
    struct NodeData *nd = INST_DATA (cl, o);
    struct DatabaseData *dbd;
    struct TagItem *tstate;
    struct TagItem *tag;
    ULONG refresh = 0L;
    ULONG tidata;
    ULONG i;

    /* process rest */
    tstate = tags;
    while (tag = NextTagItem (&tstate))
    {
	tidata = tag->ti_Data;
	switch (tag->ti_Tag)
	{
	    case HNA_Parent:
		nd->nd_Parent = (Object *) tidata;
		break;

	    case HNA_Name:
	    case HNA_Title:
	    case HNA_TOC:
	    case HNA_Previous:
	    case HNA_Next:
	    case HNA_Keywords:
	    case HNA_OnOpen:
	    case HNA_OnClose:
		i = tag->ti_Tag - HNA_Name;
		FreeVec (nd->nd_Strings[i]);
		nd->nd_Strings[i] = NULL;
		if (tidata && (nd->nd_Strings[i] = AllocVec (strlen ((STRPTR) tidata) + 3, MEMF_CLEAR)))
		    strcpy (nd->nd_Strings[i], (STRPTR) tidata);

		/* Adjust the title if the file is stale */
		if (tag->ti_Tag == HNA_Title)
		{
		    /* Are we stale or not? */
		    dbd = INST_DATA (ag->ag_DatabaseClass, nd->nd_Database);
		    if (dbd->dbd_Flags & DBDF_STALE)
			strcat (nd->nd_Strings[i], " *");
		}
		break;

	    case HNA_FileHandle:
		nd->nd_FileHandle = (BPTR) tidata;
		break;

	    case HNA_Offset:
		nd->nd_Offset = (LONG) tidata;
		break;

	    case HNA_Length:
		nd->nd_Length = (LONG) tidata;
		break;

	    case HNA_Object:
		nd->nd_Object = (Object *) tidata;
		break;
	}
    }

    return (refresh);
}
