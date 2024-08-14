/* dispatch.c
 *
 */

#include "amigaguidebase.h"

//#define PROF
#include <internal/prof.h>

/*****************************************************************************/

/* This used to be -1, but then that was before the days of DTA_Sync */
#define	DEF_TOP		0

/*****************************************************************************/

/* These are attributes that we want to pass to the embedded object */
Tag embed_attrs[] =
{
    DTA_TopVert,
    DTA_TopHoriz,
    TAG_END,
};

/* These are attributes that CAN NOT be passed to embedded objects */
Tag nembed_attrs[] =
{
    DTA_Domain,

    ICA_TARGET,
    ICA_MAP,
    TAG_END,
};

/*****************************************************************************/

/* Trigger methods we support */
struct DTMethod tm[] =
{
    {"Contents",	"CONTENTS",	STM_CONTENTS},
    {"Index",		"INDEX",	STM_INDEX},
    {"Retrace",		"RETRACE",	STM_RETRACE},
    {"Browse <",	"PREVIOUS",	STM_BROWSE_PREV},
    {"Browse >",	"NEXT",		STM_BROWSE_NEXT},

    {NULL,},
};

/*****************************************************************************/

/* Methods we support */
ULONG m[] =
{
    OM_NEW,
    OM_GET,
    OM_SET,
    OM_UPDATE,
    OM_DISPOSE,

    GM_LAYOUT,
    GM_HITTEST,
    GM_GOACTIVE,
    GM_HANDLEINPUT,
    GM_RENDER,

    DTM_CLEARSELECTED,

    DTM_PRINT,
    DTM_COPY,
    DTM_WRITE,

    DTM_GOTO,
    DTM_TRIGGER,
    DTM_REMOVEDTOBJECT,
    DTM_FRAMEBOX,

    ~0,
};

/*****************************************************************************/

/* TAB, SPACE, COMMA, ... */
UBYTE delimArray[] = "	 *-,<>()[];\"";

/*****************************************************************************/

BOOL initializeAG (struct AGLib * ag, Class * cl, Object * o, struct opSet * msg)
{
    struct ClientData *cd = INST_DATA (cl, o);	/* Object data */
    LONG line, column;
    BPTR lock = NULL;				/* Lock on directory */
    STRPTR name;				/* Where are we going? */
    UWORD i;
    BPTR fh;

    /* Initialize the variables */
    NewList ((struct List *) &cd->cd_HistoryList);
    NewList ((struct List *) &cd->cd_LineList);
    NewList ((struct List *) &cd->cd_XRefList);

    /* Establish defaults */
    cd->cd_WordDelim = delimArray;

    if (LocaleBase)
	cd->cd_Locale = OpenLocale (NULL);

    /* Get the help group */
    cd->cd_HelpGroup  = GetTagData (AGA_HelpGroup,GetUniqueID (), msg->ops_AttrList);

    /* Initialize the active number */
    cd->cd_ActiveNum = 1;

    /* Get the name of the database */
    name   = (STRPTR) GetTagData (DTA_Name, NULL, msg->ops_AttrList);

    /* Get the current position information */
    line   = GetTagData (DTA_TopVert,  DEF_TOP, msg->ops_AttrList);
    column = GetTagData (DTA_TopHoriz, DEF_TOP, msg->ops_AttrList);

    /* Disable all the buttons */
    for (i = 0; i < MAX_CONTROLS; i++)
	cd->cd_Controls[i].c_Flags |= CF_DISABLED;

    /* This is only to be used to obtain attributes from the embedded objects and
     * nothing else! */
    if (cd->cd_Glue = newobject (ag, ag->ag_ModelClass, NULL,
				 DBA_Class,		(ULONG) cl,
				 DBA_Object,		(ULONG) o,
				 DBA_ClientData,	(ULONG) cd,
				 TAG_DONE))
    {
	/* This frame is use for the control buttons */
	if (cd->cd_Frame = newobject (ag, NULL, FRAMEICLASS,
				      IA_EdgesOnly, FALSE,
				      IA_FrameType, FRAME_BUTTON,
				      TAG_DONE))
	{
	    /* Every database is an object */
	    if (cd->cd_Database = cd->cd_CurrentDatabase = newobject (ag, ag->ag_DatabaseClass, NULL,
								      DBA_Name,		name,
								      DBA_ClientData,	cd,
								      TAG_DONE))
	    {
		/* Where are we going? */
		name = (STRPTR) GetTagData (DTA_NodeName, NULL, msg->ops_AttrList);
		name = (name) ? name : "Main";

		GetAttr (DTA_Handle, o, (ULONG *) & fh);
		if (fh)
		{
		    lock = ParentOfFH (fh);
		}

		/* Set the current node */
		if (cd->cd_CurrentNode = ObtainNode (ag, cl, o, name, line, column))
		{
		    /* Set all the attributes */
		    setAGAttrs (ag, cl, o, (struct opSet *) msg);

		    /* Create a process to monitor this object */
		    if (cd->cd_Process = createnewproc (ag,
							NP_Output, Output (),
							NP_CloseOutput, FALSE,
							NP_Input, Input (),
							NP_CloseInput, FALSE,
							NP_Cli, TRUE,
							NP_Name, PROC_NAME,
							NP_CommandName, PROC_NAME,
							NP_StackSize, 8096L,
							NP_Entry, ObjectHandler,
							NP_Priority, 0L,
							NP_CurrentDir, lock,
							TAG_DONE))
		    {
			/* Initialize the message */
			cd->cd_Message.sm_Message.mn_Node.ln_Type = NT_MESSAGE;
			cd->cd_Message.sm_Message.mn_Length = sizeof (struct StartupMsg);
			cd->cd_Message.sm_Message.mn_ReplyPort = &((struct Process *)FindTask(NULL))->pr_MsgPort;
			cd->cd_Message.sm_AG     = ag;
			cd->cd_Message.sm_Class  = cl;
			cd->cd_Message.sm_Object = o;

			/* Show that we are operational */
			cd->cd_Going = TRUE;

			/* Send the information to the process */
			PutMsg (&(cd->cd_Process->pr_MsgPort), &cd->cd_Message);

			/* Wait until the new process starts */
			WaitPort (cd->cd_Message.sm_Message.mn_ReplyPort);

			/* Pull the message */
			GetMsg (cd->cd_Message.sm_Message.mn_ReplyPort);

			/* No error return value */
			if (cd->cd_Message.sm_Return == 0)
			    return (TRUE);

			/* Set the error return value */
			SetIoErr (cd->cd_Message.sm_Return);
		    }
		}
	    }
	}
	else
	    SetIoErr (ERROR_NO_FREE_STORE);
    }
    else
	SetIoErr (ERROR_NO_FREE_STORE);

    return (FALSE);
}

/*****************************************************************************/

/* Inquire attribute of an object */
ULONG getAGAttr (struct AGLib * ag, Class * cl, Object * o, struct opGet * msg)
{
    extern struct MsgPort *ARexxPort (struct AGLib *ag, struct ARexxContext * rc);
    struct ClientData *cd = INST_DATA (cl, o);
    struct NodeData *nd;

    /* See if we should be trying to get the attribute from an embedded object */
    if (cd->cd_CurrentNode && TagInArray (msg->opg_AttrID, embed_attrs))
    {
	nd = INST_DATA (ag->ag_NodeClass, cd->cd_CurrentNode);
	if (nd->nd_Object)
	    return DoMethodA (nd->nd_Object, msg);
    }

    switch (msg->opg_AttrID)
    {
	case AGA_Reserved3:
	    *msg->opg_Storage = (ULONG) cd->cd_SIPCPort;
	    break;

	case AGA_ARexxPort:
	    *msg->opg_Storage = (ULONG) ARexxPort (ag, cd->cd_ARexxContext);
	    break;

	case DTA_TriggerMethods:
	    *msg->opg_Storage = (ULONG) tm;
	    break;

	case DTA_Methods:
	    *msg->opg_Storage = (ULONG) m;
	    break;

	case DTA_TextAttr:
	    *msg->opg_Storage = (ULONG) cd->cd_TextAttr;
	    break;

	default:
	    return (DoSuperMethodA (cl, o, msg));
    }

    return (1L);
}

/*****************************************************************************/

/* Set attributes of an object */
ULONG setAGAttrs (struct AGLib * ag, Class * cl, Object * o, struct opSet * msg)
{
    struct ClientData *cd = INST_DATA (cl, o);
    struct TagItem *tags = msg->ops_AttrList;
    struct TagItem *origtags;
    struct TagItem *tstate;
    struct TagItem *tag;
    struct NodeData *nd;
    ULONG refresh = 0L;
    ULONG tidata;

    nd = INST_DATA (ag->ag_NodeClass, cd->cd_CurrentNode);
    if (nd->nd_Object)
    {
	/* Set aside the tags so that we can properly send them out */
	origtags = msg->ops_AttrList;
	msg->ops_AttrList = CloneTagItems (origtags);

	/* See if we have anything to send to the embedded object */
	if (FilterTagItems (msg->ops_AttrList, embed_attrs, TAGFILTER_AND))
	    DoMethodA (nd->nd_Object, msg);

	/* Free clone and restore original */
	FreeTagItems (msg->ops_AttrList);
	msg->ops_AttrList = origtags;
    }

    /* process rest */
    tstate = tags;
    while (tag = NextTagItem (&tstate))
    {
	tidata = tag->ti_Data;
	switch (tag->ti_Tag)
	{
	    /* This attribute is just here for debugging.  Take it out soon */
	    case TDTA_WordSelect:
		break;

	    case TDTA_WordDelim:
		cd->cd_WordDelim = (UBYTE *) tidata;
		break;

	    case DTA_TextAttr:
		cd->cd_TextAttr = (struct TextAttr *) tidata;
		if (cd->cd_Font)
		    CloseFont (cd->cd_Font);
		cd->cd_Font = OpenFont (cd->cd_TextAttr);

	    case DTA_VisibleVert:
	    case DTA_TotalVert:
	    case DTA_VisibleHoriz:
	    case DTA_TotalHoriz:
		refresh = 1L;
		break;

	    case DTA_Sync:
		if (tidata)
		    refresh = 1L;
		break;

	    case DTA_ARexxPortName:
		cd->cd_ARexxPortName = (STRPTR) tidata;
		break;

	    case DTA_ControlPanel:
		if (tidata)
		    cd->cd_Flags |= AGF_CONTROLS;
		else
		    cd->cd_Flags &= ~AGF_CONTROLS;
		break;
	}
    }
    return (refresh);
}

/*****************************************************************************/

ULONG frameMethod (struct AGLib * ag, Class * cl, Object * o, struct dtFrameBox * msg)
{
    struct ClientData *cd = INST_DATA (cl, o);
    struct TextFont *tfont = NULL;
    struct DTSpecialInfo *si;
    struct TextFont *font;
    struct FrameInfo *fri;
    struct NodeData *nd;
    ULONG retval = 1L;

    if (cd->cd_CurrentNode)
    {
	nd = INST_DATA (ag->ag_NodeClass, cd->cd_CurrentNode);

	if (nd->nd_Object)
	{
	    return DoMethodA (nd->nd_Object, msg);
	}
    }

    if ((msg->dtf_FrameFlags & FRAMEF_SPECIFY) && o)
    {
	si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
	fri = msg->dtf_ContentsInfo;
	if (cd->cd_CurrentNode && nd->nd_Font)
	    font = nd->nd_Font;
	else if (cd->cd_Font)
	    font = cd->cd_Font;
	else
	    tfont = font = OpenFont (fri->fri_Screen->Font);

	cd->cd_TotVert  = si->si_TotVert  = cd->cd_LineHeight * 24;
	cd->cd_TotHoriz = si->si_TotHoriz = font->tf_XSize * 80;

	if (tfont)
	    CloseFont (tfont);
    }

    return (retval);
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

ULONG ASM Dispatch (REG (a0) Class * cl, REG (a2) Object * o, REG (a1) Msg msg)
{
    struct AGLib *ag = (struct AGLib *) cl->cl_UserData;
    struct ClientData *cd = INST_DATA (cl, o);
    struct NodeData *nd = NULL;
    struct DTSpecialInfo *si;
    struct RastPort *rp;
    struct gpRender gpr;
    ULONG retval = 0L;
    struct Node *node;
    Object *newobj;

    ONTIMER(0);

    if (cd->cd_CurrentNode)
	nd = INST_DATA (ag->ag_NodeClass, cd->cd_CurrentNode);

    switch (msg->MethodID)
    {
	case OM_NEW:
	    if (newobj = (Object *) DoSuperMethodA (cl, o, msg))
	    {
		BOOL success = FALSE;
		ULONG errNumber;
		ULONG poolsize;

		cd = INST_DATA (cl, newobj);

		/* Start out with controls by default */
		cd->cd_Flags |= AGF_CONTROLS;

		/* Say we have scrollraster stuff */
		EG (newobj)->MoreFlags |= GMORE_SCROLLRASTER;

		poolsize = sizeof (struct Line) * 100;
		if (cd->cd_Pool = CreatePool (MEMF_CLEAR | MEMF_PUBLIC, poolsize, poolsize))
		{
		    if (initializeAG (ag, cl, newobj, (struct opSet *) msg))
		    {
			if (cd->cd_Region = NewRegion ())
			    success = TRUE;
			else
			    SetIoErr (ERROR_NO_FREE_STORE);
		    }
		}
		else
		    SetIoErr (ERROR_NO_FREE_STORE);

		if (!success)
		{
		    errNumber = IoErr ();

		    CoerceMethod (cl, newobj, OM_DISPOSE);
		    newobj = NULL;

		    SetIoErr (errNumber);
		}
	    }
	    retval = (ULONG) newobj;
	    break;

	case OM_GET:
	    retval = getAGAttr (ag, cl, o, (struct opGet *) msg);
	    break;

	case OM_SET:
	case OM_UPDATE:
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    retval += setAGAttrs (ag, cl, o, (struct opSet *) msg);
	    if (retval && (OCLASS (o) == cl))
	    {
		/* Get a pointer to the rastport */
		if (rp = ObtainGIRPort (((struct opSet *) msg)->ops_GInfo))
		{
		    /* Force a redraw */
		    gpr.MethodID   = GM_RENDER;
		    gpr.gpr_GInfo  = ((struct opSet *) msg)->ops_GInfo;
		    gpr.gpr_RPort  = rp;
		    gpr.gpr_Redraw = GREDRAW_UPDATE;
		    DoMethodA (o, &gpr);

		    /* Release the temporary rastport */
		    ReleaseGIRPort (rp);
		}
		/* Clear the return value */
		retval = 0L;
	    }
	    break;

	case DTM_GOTO:
	    retval = sam (ag, cd, msg, sizeof (struct dtGoto));
	    break;

	case DTM_TRIGGER:
	    retval = sam (ag, cd, msg, sizeof (struct dtTrigger));
	    break;

	case DTM_PROCLAYOUT:
	case GM_LAYOUT:
	    /* We need to cache the window & requester pointers */
	    if (((struct gpLayout *) msg)->gpl_Initial)
	    {
		/* Copy the GadgetInfo structure */
		cd->cd_GInfo = *(((struct gpLayout *)msg)->gpl_GInfo);
		cd->cd_Window    = ((struct gpLayout *) msg)->gpl_GInfo->gi_Window;
		cd->cd_Requester = ((struct gpLayout *) msg)->gpl_GInfo->gi_Requester;

		if (cd->cd_Window)
		{
		    if (cd->cd_Window->RPort)
			cd->cd_RPort = *(cd->cd_Window->RPort);
		    else
			memset (&cd->cd_RPort, 0, sizeof (struct RastPort));
		}
	    }

	    /* We have to pass this one up to our superclass */
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);

	    si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
	    if (si->si_Flags & DTSIF_LAYOUT)
	    {
		si->si_Flags |= DTSIF_NEWSIZE;
		retval = 1L;
	    }
	    else
	    {
		/* Show that we are busy */
		cd->cd_Flags |= AGF_BUSY;

		notifyAttrChanges (ag, o, cd->cd_Window, cd->cd_Requester, NULL,
				   GA_ID, G (o)->GadgetID,
				   DTA_Busy, TRUE,
				   TAG_DONE);

		si->si_Flags |= DTSIF_LAYOUT;
		sam (ag, cd, msg, sizeof (struct gpLayout));
	    }
	    break;

	case DTM_REMOVEDTOBJECT:
	    /* Do we have a control process running */
	    if ((cd->cd_Flags & AGF_RUNNING) && cd->cd_Process)
	    {
		/* Tell the control process to go away */
		Signal (cd->cd_Process, SIGBREAKF_CTRL_E);

		/* Wait until they go away */
		si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
		while (si->si_Flags & (DTSIF_LAYOUT | DTSIF_PRINTING))
		    Delay (3);

		cd->cd_Window = NULL;
		cd->cd_Requester = NULL;
	    }
	    break;

	case DTM_FRAMEBOX:
	    retval = frameMethod (ag, cl, o, (struct dtFrameBox *) msg);
	    break;

	case GM_HITTEST:
	    if (!(cd->cd_Flags & AGF_BUSY))
		retval = propagateHit (ag, cl, o, (struct gpHitTest *)msg);
	    break;

	case GM_GOACTIVE:
	case GM_HANDLEINPUT:
	    retval = handleInput (ag, cl, o, (struct gpInput *) msg);
	    break;

	case GM_GOINACTIVE:
	    retval = goinactive (ag, cl, o, (struct gpInput *) msg);
	    break;

	case GM_RENDER:
	    retval = renderMethod (ag, cl, o, (struct gpRender *) msg);
	    break;

	case DTM_CLEARSELECTED:
	    ((struct DTSpecialInfo *) (G (o)->SpecialInfo))->si_Flags &= ~DTSIF_HIGHLIGHT;

	case DTM_SELECT:
	    if (nd->nd_Object)
	    {
		struct DTSpecialInfo *esi;

		esi = (struct DTSpecialInfo *) G(nd->nd_Object)->SpecialInfo;
		si  = (struct DTSpecialInfo *) G (o)->SpecialInfo;

		retval = DoMethodA (nd->nd_Object, msg);
		si->si_Flags = esi->si_Flags;
	    }
	    else
	    {
		struct dtSelect *dts = (struct dtSelect *) msg;

		/* Get a pointer to the rastport */
		if (rp = ObtainGIRPort (dts->dts_GInfo))
		{
		    /* Toggle the currently selected line */
		    gpr.MethodID   = GM_RENDER;
		    gpr.gpr_GInfo  = dts->dts_GInfo;
		    gpr.gpr_RPort  = rp;
		    gpr.gpr_Redraw = GREDRAW_TOGGLE;
		    DoMethodA (o, &gpr);

		    /* Release the temporary rastport */
		    ReleaseGIRPort (rp);
		}
		/* Clear the return value */
		retval = 0L;
	    }
	    break;

	case DTM_WRITE:
	case DTM_COPY:
	case DTM_PRINT:
	    if (nd->nd_Object)
	    {
		struct DTSpecialInfo *esi;

		esi = (struct DTSpecialInfo *) G(nd->nd_Object)->SpecialInfo;
		si  = (struct DTSpecialInfo *) G (o)->SpecialInfo;

		retval = DoMethodA (nd->nd_Object, msg);
		si->si_Flags = esi->si_Flags;
	    }
	    else
	    {
		switch (msg->MethodID)
		{
		    case DTM_WRITE:
			retval = writeMethod (ag, cl, o, (struct dtWrite *) msg);
			break;

		    case DTM_COPY:
			retval = copyMethod (ag, cl, o, (struct dtGeneral *) msg);
			break;

		    case DTM_PRINT:
			retval = dumpNode (ag, 255, ((struct dtPrint *)msg)->dtp_PIO, o, cd);
			break;
		}
	    }
	    break;

	case OM_DISPOSE:
	    /* Show that it's time to go away */
	    cd->cd_Flags |= AGF_DISPOSE;

	    /* Do we have a control process running */
	    if ((cd->cd_Flags & AGF_RUNNING) && cd->cd_Process)
	    {
		/* Tell the control process to go away */
		Signal (cd->cd_Process, SIGBREAKF_CTRL_C);

		/* Wait until they go away */
		while (cd->cd_Flags & AGF_RUNNING)
		    Delay (2);
	    }

	    /* Free the history list */
	    while (node = RemHead ((struct List *) &cd->cd_HistoryList))
		FreeVec (node);

	    /* Delete the line list */
	    DeletePool (cd->cd_Pool);

	    /* Close the font */
	    if (cd->cd_Font)
		CloseFont (cd->cd_Font);

	    /* Release the current node */
	    ReleaseNode (ag, cl, o, cd->cd_CurrentNode);

	    /* Get rid of the database */
	    if (cd->cd_Database)
		DoMethod (cd->cd_Database, OM_DISPOSE);

	    /* Get rid of the button's frame */
	    if (cd->cd_Frame)
		DoMethod (cd->cd_Frame, OM_DISPOSE);

	    /* Get rid of the glue */
	    if (cd->cd_Glue)
		DoMethod (cd->cd_Glue, OM_DISPOSE);

	    if (LocaleBase)
		CloseLocale (cd->cd_Locale);

	    if (cd->cd_Region)
		DisposeRegion (cd->cd_Region);

	    /* Let the superclass handle everything else */
	default:
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    break;
    }

    OFFTIMER(0);

    return (retval);
}

/*****************************************************************************/

Class *initClass (struct AGLib * ag)
{
    Class *cl;

    if (cl = MakeClass (AMIGAGUIDEDTCLASS, DATATYPESCLASS, NULL, sizeof (struct ClientData), 0L))
    {
	cl->cl_Dispatcher.h_Entry = (ULONG (*)()) Dispatch;
	cl->cl_UserData = (ULONG) ag;
	AddClass (cl);
    }

    return (cl);
}
