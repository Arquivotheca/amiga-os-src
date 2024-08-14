/* process.c
 *
 */

#include "amigaguidebase.h"

/*****************************************************************************/

/* This used to be -1, but then that was before the days of DTA_Sync */
#define	DEF_TOP		0

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

static Object *pr_gotoMethod (struct AGLib * ag, Class * cl, Object * o, STRPTR name, LONG line, LONG column, LONG act)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
    struct ClientData *cd = INST_DATA (cl, o);
    LONG oline, ocolumn, oact;
    struct DatabaseData *dbd;
    struct NodeData *nd;
    struct History *hs;
    BOOL newNode;
    Object *nob;
    Object *cob;
    ULONG msize;

    BOOL busy = FALSE;

    /* First see if we can really do this */
    while (si->si_Flags & (DTSIF_LAYOUTPROC | DTSIF_PRINTING))
    {
	/* Delay two seconds */
	busy = TRUE;
	Delay (100);
    }

    /* Wait just a little longer if we had been printing */
    if (busy)
	Delay (100);

    /* Show that we are busy */
    cd->cd_Flags |= AGF_BUSY;

    /* Get the current settings */
    cob     = cd->cd_CurrentNode;
    oline   = si->si_TopVert;
    ocolumn = si->si_TopHoriz;
    oact    = cd->cd_ActiveNum;

    /* Tell them to go busy */
    notifyAttrChanges (ag, o, cd->cd_Window, cd->cd_Requester, NULL,
		       GA_ID,		G (o)->GadgetID,
		       DTA_Busy,	TRUE,
		       DTA_Title,	LVOGetAGString (ag, TITLE_LOADING),
		       TAG_DONE);

    /* See if we can find the node */
    if (nob = ObtainNode (ag, cl, o, name, line, column))
    {
	/* Remember what they are set to */
	line   = si->si_TopVert;
	column = si->si_TopHoriz;

	cd->cd_Flags |= AGF_SYNC;

	/* See if this is a new node or location */
	/* This used to JUST look at cob==nob */
	newNode = TRUE;
	DB (kprintf ("cob=%08lx, nob=%08lx : %ld %ld : %ld %ld\n", cob, nob, line, oline, column, ocolumn));
	if ((cob == nob) && (line == oline) && (column == ocolumn))
	    newNode = FALSE;

	/* Is this a new node? */
	if (newNode && !(cd->cd_Flags & AGF_RETRACE))
	{
	    /* Build the complete name */
	    nd    = INST_DATA (ag->ag_NodeClass, cob);
	    dbd   = INST_DATA (ag->ag_DatabaseClass, nd->nd_Database);
	    sprintf (cd->cd_WorkName, "%s/%s", dbd->dbd_Name, nd->nd_Name);
	    msize = sizeof (struct History) + strlen (cd->cd_WorkName) + 1;

	    /* Create a history node */
	    if (hs = AllocVec (msize, MEMF_PUBLIC))
	    {
		/* Initialize the fields of the history record */
		hs->hs_Name = MEMORY_FOLLOWING (hs);
		strcpy (hs->hs_Name, cd->cd_WorkName);
		hs->hs_Node.ln_Name = hs->hs_Name;
		hs->hs_Object       = cob;
		hs->hs_CurLine      = oline;
		hs->hs_CurColumn    = ocolumn;
		hs->hs_ActiveKey    = oact;

		/* Add the history node to the list */
		AddTail ((struct List *) &cd->cd_HistoryList, &hs->hs_Node);
		DB (kprintf ("add %08lx, %ld, %ld to history\n", cob, oline, ocolumn));
	    }
	}

	/* Clear the retrace flag */
	cd->cd_Flags &= ~AGF_RETRACE;

	/* Release the current node */
	ReleaseNode (ag, cl, o, cob);

	/* Get a pointer to the node data */
	nd  = INST_DATA (ag->ag_NodeClass, nob);

	/* Set the current node and database */
	cd->cd_CurrentDatabase = nd->nd_Database;
	cd->cd_CurrentNode = nob;
	cd->cd_ActiveNum = act;

	/* Layout the new node */
	ObtainSemaphore (&si->si_Lock);
	si->si_Flags |= DTSIF_LAYOUT;
	si->si_Flags &= ~DTSIF_NEWSIZE;
	layoutMethod (ag, cl, o, 1);
	si->si_Flags &= ~(DTSIF_LAYOUT | DTSIF_NEWSIZE);
	ReleaseSemaphore (&(si->si_Lock));

	/* This is the way we have to have it */
	si->si_TopVert  = line;
	si->si_TopHoriz = column;
    }
    else
    {
	/* Tell about the error and the fact that we aren't busy anymore */
	notifyAttrChanges (ag, o, cd->cd_Window, cd->cd_Requester, NULL,
			   GA_ID,		G (o)->GadgetID,
			   DTA_Busy,		FALSE,
			   DTA_ErrorLevel,	cd->cd_ErrorLevel,
			   DTA_ErrorNumber,	cd->cd_ErrorNumber,
			   DTA_ErrorString,	cd->cd_ErrorString,
			   TAG_DONE);

	/* Show that we aren't busy */
	cd->cd_Flags &= ~AGF_BUSY;
    }
    return (nob);
}

/*****************************************************************************/

static ULONG pr_triggerMethod (struct AGLib * ag, Class * cl, Object * o, struct dtTrigger * msg)
{
    struct ClientData *cd = INST_DATA (cl, o);
    struct _Object *rob = NULL;
    struct NodeData *nd = NULL;
    LONG line = 0, column = 0;
    struct DatabaseData *dbd;
    STRPTR mainName = "Main";
    struct _Object *ob;
    struct History *hs;
    struct List *list;
    ULONG retval = 0;
    LONG act = 1;
    STRPTR name;

    dbd = INST_DATA (ag->ag_DatabaseClass, cd->cd_CurrentDatabase);
    list = (struct List *) &dbd->dbd_NodeList;

    if (cd->cd_CurrentNode)
    {
	nd = INST_DATA (ag->ag_NodeClass, cd->cd_CurrentNode);
	ob = _OBJECT (cd->cd_CurrentNode);
    }

    name = NULL;

    switch (msg->dtt_Function)
    {
	case STM_CONTENTS:
	    if (nd)
	    {
		if (nd->nd_TOC)
		    name = nd->nd_TOC;
		else if (Stricmp (nd->nd_Name, mainName) != 0)
		    name = mainName;
	    }
	    else
		name = mainName;
	    break;

	case STM_INDEX:
	    if (dbd->dbd_Index)
		name = dbd->dbd_Index;
	    break;

	case STM_HELP:
	    if (dbd->dbd_Help)
		name = dbd->dbd_Help;
	    else
		name = MASTER_HELPNODE;
	    break;

	case STM_RETRACE:
	    if (hs = (struct History *) RemTail ((struct List *) &cd->cd_HistoryList))
	    {
#if 0
		while (hs->hs_Object == cd->cd_CurrentNode)
		{
		    FreeVec (hs);
		    hs = (struct History *) RemTail ((struct List *) &cd->cd_HistoryList);
		}
#endif

		if (hs)
		{
		    /* Get the current object */
		    strcpy (cd->cd_WorkName, hs->hs_Name);
		    name   = cd->cd_WorkName;
		    line   = hs->hs_CurLine;
		    column = hs->hs_CurColumn;
		    act    = hs->hs_ActiveKey;

		    cd->cd_Flags |= AGF_RETRACE;
		    FreeVec (hs);
		}
	    }
	    else
		DisplayBeep (NULL);
	    break;

	case STM_BROWSE_PREV:
	    if (nd)
	    {
		if (nd->nd_Prev)
		    name = nd->nd_Prev;
		else if (ob != (struct _Object *) list->lh_Head)
		    rob = (struct _Object *) ob->o_Node.mln_Pred;
	    }
	    break;

	case STM_BROWSE_NEXT:
	    if (nd)
	    {
		if (nd->nd_Next)
		    name = nd->nd_Next;
		else if (ob != (struct _Object *) list->lh_TailPred)
		    rob = (struct _Object *) ob->o_Node.mln_Succ;
	    }
	    break;

	case STM_NEXT_FIELD:
	    r_activateLink (ag, cl, o, cd, 1);
	    break;

	case STM_PREV_FIELD:
	    r_activateLink (ag, cl, o, cd, -1);
	    break;
    }

    if (rob)
    {
	nd = INST_DATA (ag->ag_NodeClass, BASEOBJECT (rob));
	name = nd->nd_Name;
    }

    if (name)
    {
	pr_gotoMethod (ag, cl, o, name, line, column, act);
	retval = 1L;
    }

    return (retval);
}

/*****************************************************************************/

static ULONG pr_initProcess (struct AGLib *ag, Class *cl, Object *o, struct ClientData *cd)
{
    ULONG retval = ERROR_NO_FREE_STORE;

    /* Create our communication port */
    if (cd->cd_SIPCPort = CreateMsgPort ())
    {
	/* Initialize the ARexx port */
	if (ag->ag_RexxSysBase)
	    cd->cd_ARexxContext = InitARexx (ag, cd->cd_ARexxPortName, "guide", FALSE);

	/* Indicate success */
	retval = 0;
    }

    return (retval);
}

/*****************************************************************************/

#undef	SysBase

/*****************************************************************************/

void ObjectHandler (void)
{
    struct ExecBase *SysBase = (*((struct ExecBase **) 4));
    struct DTSpecialInfo *si;
    LONG error = RETURN_OK;
    UBYTE parseBuffer[512];
    UBYTE workBuffer[512];
    struct StartupMsg *sm;
    struct ClientData *cd;
    BOOL changed = FALSE;
    struct RexxMsg *rmsg;
    struct SIPCMsg *sim;
    STRPTR argv[MAXARG];
    struct NodeData *nd;
    STRPTR text = NULL;
    struct Process *pr;
    BOOL parse = FALSE;
    APTR oldptr = NULL;
    LONG line, column;
    struct AGLib *ag;
    STRPTR nodename;
    ULONG tagsync;
    UWORD cmdofs;
    BOOL notify;
    ULONG sigr;
    ULONG sigp;
    ULONG argc;
    Class *cl;
    Object *o;
    Msg msg;

    /* Find out who we are */
    pr = (struct Process *) FindTask (NULL);

    /* Get the startup message */
    WaitPort (&pr->pr_MsgPort);
    sm = (struct StartupMsg *) GetMsg (&pr->pr_MsgPort);

    /* Pull all the information from the startup message */
    ag = sm->sm_AG;
    cl = sm->sm_Class;
    o  = sm->sm_Object;
    cd = INST_DATA (cl, o);
    si = (struct DTSpecialInfo *) G (o)->SpecialInfo;

    /* Try initializing the process */
    sm->sm_Return = pr_initProcess (ag, cl, o, cd);

    /* Reply to the message */
    ReplyMsg ((struct Message *) sm);

    /* Show that we are running */
    cd->cd_Flags |= AGF_RUNNING;

    /* Continue if no errors */
    if (sm->sm_Return == 0)
    {
	/* Get the ARexx signal bit */
	sigp = ARexxSignal (ag, cd->cd_ARexxContext);

	/* Clear the notify flag */
	notify = FALSE;

	/* Keep going until we are told to stop */
	while (cd->cd_Going)
	{
	    /* Wait for something to happen */
	    sigr = Wait ((1L << cd->cd_SIPCPort->mp_SigBit) | sigp | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_E);

	    /*****************/
	    /* ARexx message */
	    /*****************/
	    if (cd->cd_ARexxContext && (sigr & sigp))
	    {
		/* Get the ARexx message */
		rmsg = GetARexxMsg (ag, cd->cd_ARexxContext);

		/* Make sure it is something we can handle */
		if (rmsg == REXX_RETURN_ERROR)
		{
		}
		else if (rmsg)
		{
		    struct DatabaseData *dbd;
		    UBYTE tmpBuffer[20];
		    struct dtTrigger dtt;

		    /* Copy the message */
		    strcpy (parseBuffer, ARG0 (rmsg));

		    /* See if we have something that has a return value */
		    if (Stricmp (parseBuffer, "getnodecount") == 0)
		    {
			/* Get a pointer to the database object */
			dbd = INST_DATA (ag->ag_DatabaseClass, cd->cd_Database);

			/* Make the return value */
			sprintf (tmpBuffer, "%ld", dbd->dbd_NumNodes);
			text = tmpBuffer;
		    }
		    else if (Stricmp (parseBuffer, "next") == 0)
		    {
			dtt.dtt_Function = STM_BROWSE_NEXT;
			if (pr_triggerMethod (ag, cl, o, &dtt))
			    notify = TRUE;
		    }
		    else if (Stricmp (parseBuffer, "previous") == 0)
		    {
			dtt.dtt_Function = STM_BROWSE_PREV;
			if (pr_triggerMethod (ag, cl, o, &dtt))
			    notify = TRUE;
		    }
		    else if (Stricmp (parseBuffer, "retrace") == 0)
		    {
			dtt.dtt_Function = STM_RETRACE;
			if (pr_triggerMethod (ag, cl, o, &dtt))
			    notify = TRUE;
		    }
		    else if (Stricmp (parseBuffer, "print") == 0)
		    {
			union printerIO *pio;
			struct MsgPort *mp;

			if (mp = CreateMsgPort ())
			{
			    if (pio = (union printerIO *) CreateIORequest (mp, sizeof (union printerIO)))
			    {
				if (OpenDevice ("printer.device", 0, (struct IORequest *) pio, 0) == 0)
				{
				    /* Dump the node */
				    dumpNode (ag, 255, pio, o, cd);

				    /* Form Feed */
				    pio->ios.io_Length = 2;
				    pio->ios.io_Data = (APTR) "\n";
				    pio->ios.io_Command = CMD_WRITE;
				    DoIO ((struct IORequest *) pio);

				    CloseDevice ((struct IORequest *) pio);
				}
				DeleteIORequest ((struct IORequest *) pio);
			    }
			    DeleteMsgPort (mp);
			}
		    }
		    else
			parse = TRUE;
		    cmdofs = 0;

		    /* Reply the message */
		    ReplyARexxMsg (ag, cd->cd_ARexxContext, rmsg, text, error);
		    text = NULL;
		}
	    }

	    if (sigr & SIGBREAKF_CTRL_E)
	    {
		/* Remove from the window */
		if (changed)
		    pr->pr_WindowPtr = oldptr;
		changed = FALSE;
	    }

	    /* Was that something a ^C */
	    if (sigr & SIGBREAKF_CTRL_C)
	    {
		cd->cd_Going = FALSE;
	    }
	    else
	    {
		/* Pull all the control information */
		while (sim = (struct SIPCMsg *) GetMsg (cd->cd_SIPCPort))
		{
		    switch (sim->sm_Type)
		    {
			case SIPCT_COMMAND:
			    strcpy (parseBuffer, sim->sm_Data);
			    parse = TRUE;
			    cmdofs = 1;
			    break;

			case SIPCT_MESSAGE:
			    msg = (Msg) sim->sm_Data;
			    switch (msg->MethodID)
			    {
				case DTM_GOTO:
				    line   = GetTagData (DTA_TopVert,  DEF_TOP, ((struct dtGoto *)msg)->dtg_AttrList);
				    column = GetTagData (DTA_TopHoriz, DEF_TOP, ((struct dtGoto *)msg)->dtg_AttrList);
				    nodename = ((struct dtGoto *)msg)->dtg_NodeName;
				    if (pr_gotoMethod (ag, cl, o, nodename, line, column, 1))
					notify = TRUE;
				    break;

				case DTM_TRIGGER:
				    {
					struct dtTrigger *dtt = (struct dtTrigger *) msg;

					if (dtt->dtt_Function == STM_COMMAND)
					{
					    strcpy (parseBuffer, dtt->dtt_Data);
					    parse = TRUE;
					    cmdofs = 0;
					}
					else if (dtt->dtt_Function == STM_ACTIVATE_FIELD)
					{
					    if (cd->cd_ActiveLine)
					    {
						strcpy (parseBuffer, cd->cd_ActiveLine->ln_Data);
						parse = TRUE;
						cmdofs = 1;
					    }
					}
					else if (pr_triggerMethod (ag, cl, o, dtt))
					    notify = TRUE;
				    }

				    break;

				case GM_LAYOUT:
				    if (!changed)
				    {
					changed = TRUE;
					oldptr = pr->pr_WindowPtr;
					pr->pr_WindowPtr = cd->cd_Window;
				    }

				    if (cd->cd_CurrentNode)
				    {
					ObtainSemaphore (&si->si_Lock);
					do
					{
					    si->si_Flags &= ~DTSIF_NEWSIZE;
					    layoutMethod (ag, cl, o, 0);
					} while (si->si_Flags & DTSIF_NEWSIZE);

					si->si_Flags &= ~(DTSIF_LAYOUT | DTSIF_NEWSIZE);
					ReleaseSemaphore (&(si->si_Lock));
					notify = TRUE;
				    }
				    break;

				default:
				    break;
			    }
			    break;

			case SIPCT_QUIT:
			    /* Time to quit! */
			    cd->cd_Going = FALSE;
			    break;

			default:
			    break;
		    }

		    /* Do we have a command string to parse? */
		    if (parse)
		    {
			/* Parse it */
			if ((argc = ParseString (ag, parseBuffer, argv, MAXARG)) > 1)
			{
			    /* Link to a node? */
			    if ((Stricmp (argv[cmdofs], "link") == 0) || (Stricmp (argv[cmdofs], "alink") == 0))
			    {
				line = column = DEF_TOP;
				if (argc > cmdofs + 2)
				{
				    StrToLong (argv[cmdofs+2],(LONG *)&line);
				    if (line)
					line--;
				    if (argc > cmdofs + 3)
				    {
					StrToLong (argv[cmdofs+3],(LONG *)&column);
					if (column)
					    column--;
				    }
				}
				nodename = argv[cmdofs+1];
				if (pr_gotoMethod (ag, cl, o, nodename, line, column, 1))
				    notify = TRUE;
			    }
			    else if (Stricmp (argv[cmdofs], "beep") == 0)
			    {
				DisplayBeep (NULL);
			    }
			    else if (Stricmp (argv[cmdofs], "close") == 0)
			    {
				/* Time to quit! */
				cd->cd_Going = FALSE;
			    }
#if 0
			    else if (Stricmp (argv[cmdofs], "next") == 0)
			    {
			    }
			    else if (Stricmp (argv[cmdofs], "previous") == 0)
			    {
			    }
#endif
			    else if (Stricmp (argv[cmdofs], "quit") == 0)
			    {
				/* Time to quit! */
				cd->cd_Going = FALSE;
			    }
			    else if (Stricmp (argv[cmdofs], "rxs") == 0)
			    {
				if (argc > cmdofs + 1)
				{
				    if (!(SendARexxMsg (ag, cd->cd_ARexxContext, argv[cmdofs+1], TRUE)))
				    {
					notifyAttrChanges (ag, o, cd->cd_Window, cd->cd_Requester, NULL,
							   GA_ID,		G (o)->GadgetID,
							   DTA_ErrorLevel,	RETURN_WARN,
							   DTA_ErrorNumber,	DTERROR_COULDNT_SEND_MESSAGE,
							   DTA_ErrorString,	(ULONG)argv[cmdofs+1],
							   TAG_DONE);
				    }
				}
			    }
			    else if (Stricmp (argv[cmdofs], "rx") == 0)
			    {
				if (argc > cmdofs + 1)
				{
				    if (!(SendARexxMsg (ag, cd->cd_ARexxContext, argv[cmdofs+1], FALSE)))
				    {
					notifyAttrChanges (ag, o, cd->cd_Window, cd->cd_Requester, NULL,
							   GA_ID,		G (o)->GadgetID,
							   DTA_ErrorLevel,	RETURN_WARN,
							   DTA_ErrorNumber,	DTERROR_COULDNT_SEND_MESSAGE,
							   DTA_ErrorString,	(ULONG)argv[cmdofs+1],
							   TAG_DONE);
				    }
				}
			    }
			    else if (Stricmp (argv[cmdofs], "system") == 0)
			    {
				STRPTR cmd = argv[cmdofs + 1];
				BPTR fh = NULL;

				/* Do they need a console? */
				if ((argc > 2) && (Stricmp (argv[cmdofs + 1], "CONSOLE")) == 0)
				{
				    /* Build the console string */
				    strcpy (workBuffer, "CON:0/0/320/50/AmigaGuide/CLOSE/AUTO/WAIT");

				    /* Open the console */
				    fh = Open (workBuffer, MODE_NEWFILE);

				    /* Command is in next field */
				    cmd = argv[cmdofs + 2];
				}

				/* Add a stack to the command */
				sprintf (workBuffer, "stack 40000\n%s", cmd);

				/* Execute the command */
				Execute (workBuffer, NULL, fh);

				if (fh)
				    Close (fh);
			    }
			}
			parse = FALSE;
		    }

		    /* Free the message */
		    FreeVec (sim);
		}

		if (notify)
		{
		    tagsync = (cd->cd_Flags & AGF_SYNC) ? DTA_Sync : TAG_IGNORE;
		    cd->cd_Flags &= ~AGF_SYNC;

		    /* Tell someone about the new information. */
		    nd = INST_DATA (ag->ag_NodeClass, cd->cd_CurrentNode);
		    notifyAttrChanges (ag, o, cd->cd_Window, cd->cd_Requester, NULL,
				       GA_ID,		G (o)->GadgetID,
				       DTA_TopVert,	si->si_TopVert,
				       DTA_VisibleVert,	si->si_VisVert,
				       DTA_TotalVert,	si->si_TotVert,
				       DTA_NominalVert,	si->si_TotVert,
				       DTA_TopHoriz,	si->si_TopHoriz,
				       DTA_VisibleHoriz,si->si_VisHoriz,
				       DTA_TotalHoriz,	si->si_TotHoriz,
				       DTA_NominalHoriz,si->si_TotHoriz,
				       DTA_Title,	nd->nd_Title,
				       DTA_Busy,	FALSE,
				       tagsync,		TRUE,
				       TAG_DONE);

		    /* Show that we aren't busy */
		    cd->cd_Flags &= ~AGF_BUSY;

		    notify = FALSE;
		}
	    }
	}

	/* Free the ARexx context */
	FreeARexx (ag, cd->cd_ARexxContext);

	/* Clear the SIPC port */
	while (sim = (struct SIPCMsg *) GetMsg (cd->cd_SIPCPort))
	    FreeVec (sim);

	/* Delete our message port */
	DeleteMsgPort (cd->cd_SIPCPort);
    }

    /* Exit now */
    Forbid ();

    /* Show that we aren't running anymore */
    cd->cd_Flags &= ~AGF_RUNNING;
}
