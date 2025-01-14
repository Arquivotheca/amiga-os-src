/* apsh_sipc.c
 * Copyright (C) 1990 Commodore-Amiga, Inc.
 * written by David N. Junod
 *
 * Simple IPC message handling routines
 *
 * $Id: apsh_sipc.c,v 1.4 1992/09/07 17:58:55 johnw Exp johnw $
 *
 * $Log: apsh_sipc.c,v $
 * Revision 1.4  1992/09/07  17:58:55  johnw
 * MInor changes.
 *
 * Revision 1.1  91/12/12  14:53:48  davidj
 * Initial revision
 * 
 * Revision 1.1  90/07/02  11:37:44  davidj
 * Initial revision
 *
 *
 */

#include "appshell_internal.h"

void kprintf (void *,...);

#define	DB(x)	;
#define	DS(x)	;
#define	DR(x)	;
#define	DI(x)	;

BOOL send_sipc_command (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl);

/****** appshell.library/setup_sipcA **************************************
*
*   NAME
*	setup_sipcA - Initializes the SIPC message handler.
*
*   SYNOPSIS
*	mh = setup_sipcA (ai, attrs)
*	D0                D0  D1
*
*	struct MsgHandler *mh;
*	struct AppInfo *ai;
*	struct TagItem *attrs;
*
*   FUNCTION
*	This is the low-level function used to initialize the Simple
*	InterProcess Communications message handler.
*
*	NOTE: This call should not be called directly by the application,
*	but should invoked by using the APSH_AddSIPC_UI tag.
*
*	Valid tagitems to use at INITIALIZATION are:
*
*	    APSH_Port, <name>
*	    Used to specify a name for the SIPC port.  Defaults to
*	    APSH_BaseName plus _SIPC.
*
*	    APSH_Status, <flags>
*	    Where flags can be:
*
*		APSHP_INACTIVE
*		When set, then the port will be initialized, but
*		will fail all messages received.
*
*		APSHP_SINGLE
*		When set, will not add a use count to the end of the
*		message port name.
*
*	    APSH_SIPCOK, <func id>
*	    Function to execute when a SIPC message returns successfully.
*
*	    APSH_SIPCError, <func id>
*	    Function to execute when a SIPC message returns with an error.
*
*	    APSH_AlreadyRunning, <func id>
*	    Function to execute if port (when APSHP_SINGLE specified)
*	    already exists.
*
*	In addition to the standard message handler functions, the SIPC
*	function handler provides the following functions:
*
*	    AH_SENDCMD
*	    Send a command to another AppShell application.  This function
*	    understands the following tags:
*
*		APSH_NameTag, <name>
*		Name of the destination SIPC message port.
*
*		APSH_PortAddr, <MsgPort *>
*		Address of the destination SIPC message port.
*
*		APSH_CmdID, <function ID>
*		Function ID for the destination application to invoke.
*
*		APSH_CmdData, <data>
*		Information to send to the destination application.  Data
*		can be a tag list or a block of data.
*
*		APSH_CmdDataLength, <length>
*		This is used to indicate a block of data.
*
*		APSH_CmdString, <command>
*		This is used to send a command string to another application.
*
*   EXAMPLE
*
*	STRPTR port_name;
*	struct MsgPort *mp;
*	struct TagItem *attr_list;
*	LONG data;
*	LONG data_len;
*
*	\* To send a command string to another AppShell application *\
*	HandlerFunc (ai,
*		     APSH_Handler, "SIPC",
*		     APSH_Command, AH_SENDCMD,
*		     APSH_NameTag, port_name,
*		     APSH_CmdString, "window front activate",
*		     TAG_DONE);
*
*	\* To call a function with tags *\
*	HandlerFunc (ai,
*		     APSH_Handler, "SIPC",
*		     APSH_Command, AH_SENDCMD,
*		     APSH_PortAddr, port_addr,
*		     APSH_CmdID, OpenID,
*		     APSH_CmdData, attr_list,
*		     TAG_DONE);
*
*	\* To send a block of a data *\
*	HandlerFunc (ai,
*		     APSH_Handler, "SIPC",
*		     APSH_Command, AH_SENDCMD,
*		     APSH_PortAddr, ad->ad_ProjSIPC,
*		     APSH_CmdID, SendDataID,
*		     APSH_CmdData, data,
*		     APSH_CmdDataLength, data_len,
*		     TAG_DONE);
*
*   INPUTS
*	ai	- Pointer to the application's AppInfo structure.
*	attrs	- Pointer to an array of TagItem attributes for the
*		  function.
*
*   RESULT
*	mh	- Pointer to a MsgHandler structure.
*
*   SEE ALSO
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

struct MsgHandler *setup_sipcA (struct AppInfo * ai, struct TagItem * tl)
{
    register struct MsgHandler *mh = NULL;
    register struct MHObject *mho;
    register struct SIPCInfo *md;
    register WORD cntr;
    ULONG msize, hstatus;
    STRPTR pname;
    BOOL exist = TRUE;

    /* get the activation status */
    DI (kprintf ("setup_sipcA enter\n"));
    hstatus = GetTagData (APSH_Status, NULL, tl);

    /* get the public port name for SIPC */
    strcpy (ai->ai_WorkText, ai->ai_BaseName);
    strcat (ai->ai_WorkText, "_SIPC");
    strupr (ai->ai_WorkText);
    pname = (UBYTE *) GetTagData (APSH_Port, (ULONG) ai->ai_WorkText, tl);

    if ((hstatus & P_SINGLE) && TellAppToActivate (pname))
    {
	DI (kprintf ("Told App to activate\n"));
	return (mh);
    }

    /* calculate the amount of memory that we need */
    msize = sizeof (struct MsgHandler) + sizeof (struct SIPCInfo) +
      (5L * sizeof (ULONG));

    /* allocate instance data */
    if (mh = (struct MsgHandler *) AllocVec (msize, MEMF_CLEAR | MEMF_PUBLIC))
    {
	/* Set the cache */
	ai->ai_MH[MH_SIPC] = mh;

	/* get a pointer to the object */
	mho = &(mh->mh_Header);

	/* initialize the node information */
	mho->mho_Node.ln_Type = APSH_MH_HANDLER_T;
	mho->mho_Node.ln_Pri = APSH_MH_HANDLER_P;
	mho->mho_Node.ln_Name = "SIPC";

	/* initialize the object list */
	NewList (&(mho->mho_ObjList));

	/* establish the object id */
	mho->mho_ID = APSH_SIPC_ID;

	/* get a pointer to the instance data */
	mho->mho_SysData = md = MEMORY_FOLLOWING (mh);

	/* initialize the message handler functions */
	mh->mh_NumFuncs = 5;
	mh->mh_Func = MEMORY_FOLLOWING (md);
	mh->mh_Func[APSH_MH_OPEN] = open_sipc;
	mh->mh_Func[APSH_MH_HANDLE] = handle_sipc;
	mh->mh_Func[APSH_MH_CLOSE] = close_sipc;
	mh->mh_Func[APSH_MH_SHUTDOWN] = shutdown_sipc;
	mh->mh_Func[AH_SENDCMD] = send_sipc_command;

	/* allocate room for the port name */
	msize = (strlen (pname) + 6L);

	mh->mh_PortName = AllocVec (msize, MEMF_CLEAR);
	strcpy (mh->mh_PortName, pname);

	/* Get the SIPC functions */
	md->sipc_ufunc[SIPC_OK] = GetTagData (APSH_SIPCOK, NULL, tl);
	md->sipc_ufunc[SIPC_ERROR] = GetTagData (APSH_SIPCError, NULL, tl);
	DR (kprintf("OK %ld, Error %ld\n", md->sipc_ufunc[SIPC_OK], md->sipc_ufunc[SIPC_ERROR]));

	/* disable multi-tasking for a moment */
	Forbid ();

	/* get an unique port name */
	if (hstatus & APSHP_SINGLE)
	{
	    struct MsgPort *t_mp;
	    LONG FuncID;

	    FuncID = GetTagData (APSH_AlreadyRunning, NULL, tl);

	    if (t_mp = FindPort (mh->mh_PortName))
	    {
		/* permit multi-tasking again */
		Permit ();

		/* Do they want us to tell them? */
		DI (kprintf ("Port [%s] already exists\n", mh->mh_PortName));
		if (FuncID)
		{
		    struct TagItem tg[3];

		    /* Give them enough information to work with */
		    tg[0].ti_Tag = APSH_NameTag;
		    tg[0].ti_Data = (ULONG) mh->mh_PortName;
		    tg[1].ti_Tag = (tl) ? TAG_MORE : TAG_IGNORE;
		    tg[1].ti_Data = (ULONG) tl;
		    tg[2].ti_Tag = TAG_DONE;
		    tg[2].ti_Data = NULL;

		    /* Perform the function assigned to this message. */
		    PerfFunc (ai, FuncID, NULL, tg);

		    /* Set up the error return values */
		    ai->ai_Pri_Ret = RETURN_WARN;
		    ai->ai_Sec_Ret = APSH_PORT_ACTIVE;
		    ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID, ai->ai_Sec_Ret,
					   (int) mh->mh_PortName);
		}
		else
		{
		    /* set up the error return values */
		    ai->ai_Pri_Ret = RETURN_FAIL;
		    ai->ai_Sec_Ret = APSH_PORT_ACTIVE;
		    ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID, ai->ai_Sec_Ret,
					   (int) mh->mh_PortName);
		}
	    }
	    else
	    {
		/* create our port */
		DI (kprintf ("Create port [%s]\n", mh->mh_PortName));
		if (!(mh->mh_Port = CreatePort (mh->mh_PortName, 0L)))
		{
		    /* permit multi-tasking again */
		    Permit ();

		    /* set up the error return values */
		    ai->ai_Pri_Ret = RETURN_FAIL;
		    ai->ai_Sec_Ret = APSH_CLDNT_CREATE_PORT;
		    ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID,
				       ai->ai_Sec_Ret, (int) mh->mh_PortName);
		}
		else
		{
		    /* permit multi-tasking again */
		    Permit ();
		}
	    }
	}
	else
	{
	    /* initialize variables */
	    exist = TRUE;
	    cntr = 1;

	    while (exist)
	    {
		/* create a name with our base name and a number */
		sprintf (mh->mh_PortName, "%s_%ld", pname, (LONG) cntr);

		/* see if someone has already taken this name */
		if (!FindPort (mh->mh_PortName))
		    exist = FALSE;

		cntr++;
	    }

	    /* create our port */
	    if (!(mh->mh_Port = CreatePort (mh->mh_PortName, 0L)))
	    {
		/* permit multi-tasking again */
		Permit ();

		/* set up the error return values */
		ai->ai_Pri_Ret = RETURN_FAIL;
		ai->ai_Sec_Ret = APSH_CLDNT_CREATE_PORT;
		ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID, ai->ai_Sec_Ret,
					   (int) mh->mh_PortName);
	    }
	    else
	    {
		/* permit multi-tasking again */
		Permit ();
	    }
	}

	if (ai->ai_Pri_Ret == RETURN_OK)
	{
	    /* set up the signal bits */
	    mh->mh_SigBits = (1L << mh->mh_Port->mp_SigBit);

	    /* open immediately? */
	    if (hstatus & APSHP_INACTIVE)
	    {
		return (mh);
	    }
	    else
	    {
		if (open_sipc (ai, mh, tl))
		{
		    return (mh);
		}
	    }
	}

	/* make a nice clean failure path */
	if (mh->mh_Port)
	{
	    DeletePort (mh->mh_Port);
	}

	/* free the port name */
	if (mh->mh_PortName)
	{
	    FreeVec ((APTR) mh->mh_PortName);
	}

	/* free the memory block */
	FreeVec ((APTR) mh);
	mh = NULL;
    }
    else
    {
	ai->ai_Pri_Ret = RETURN_FAIL;
	ai->ai_Sec_Ret = APSH_NOT_ENOUGH_MEMORY;
	ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID, ai->ai_Sec_Ret, NULL);
    }

    DI (kprintf ("setup_sipcA exit\n"));

    return (mh);
}

BOOL open_sipc (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    register struct MHObject *mho = &(mh->mh_Header);

    mho->mho_Status |= APSHF_OPEN;
    return (TRUE);
}

BOOL handle_sipc (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * ptl)
{
    struct MHObject *mho = &(mh->mh_Header);
    struct SIPCInfo *md = mho->mho_SysData;
    ULONG FuncID = NO_FUNCTION;
    struct TagItem tg[5], *tl;
    struct SIPCMessage *msg;

    /* set up the Tag list for passing */
    tg[0].ti_Tag = APSH_SIPCData;
    tg[1].ti_Tag = APSH_SIPCDataLength;
    tg[2].ti_Tag = TAG_IGNORE;
    tg[3].ti_Tag = TAG_IGNORE;
    tg[4].ti_Tag = TAG_DONE;

    /* Set the active message handler ID */
    ai->ai_ActvMH = mho->mho_ID;

    /* Pull the messages from the port */
    while (msg = (struct SIPCMessage *) GetMsg (mh->mh_Port))
    {
	/* Set the active message */
	ai->ai_ActvMsg = (struct Message *) msg;

	if (msg->sipc_Msg.mn_Node.ln_Type == NT_REPLYMSG)
	{
	    /* this is a reply to one of our own messages */
	    if (msg->sipc_Pri_Ret == RETURN_OK)
	    {
		/* handle a positive return */
		FuncID = md->sipc_ufunc[SIPC_OK];
		DR (kprintf("Return OK, %ld\n", FuncID));
	    }
	    else
	    {
		/* handle a negative return */
		FuncID = md->sipc_ufunc[SIPC_ERROR];
		DR (kprintf("Return Error, %ld\n", FuncID));
	    }

	    if (FuncID)
	    {
		/* Perform the function assigned to this message. */
		PerfFunc (ai, FuncID, NULL, ptl);
	    }

	    /* free the message */
	    FreeVec ((APTR) msg);

	    /* Decrement the outstanding message counter */
	    mh->mh_Outstanding--;
	    ai->ai_NumCmds--;
	}
	else
	{
	    /* Pass the AppInfo */
	    tg[2].ti_Tag = (msg->sipc_Extens1) ? APSH_AppHandle : TAG_IGNORE;
	    tg[2].ti_Data = (ULONG) msg->sipc_Extens1;

	    /* this is a new message */
	    if (msg->sipc_DType != APSH_SDT_Command)
	    {
		/* Get the function number assigned to this message */
		FuncID = msg->sipc_Type;

		/* make sure that we're capable of calling the function */
		if ((FuncID != NO_FUNCTION) &&
		    (mho->mho_Status & APSHF_OPEN) &&
		    !(mho->mho_Status & APSHF_DISABLED))
		{
		    if (msg->sipc_DType == APSH_SDT_TagList)
		    {
			/* they passed us a tag list */
			tg[0].ti_Tag = TAG_IGNORE;
			tg[1].ti_Tag = TAG_IGNORE;
			tg[3].ti_Tag = TAG_MORE;
			tg[3].ti_Data = (ULONG) msg->sipc_Data;
			tl = tg;

			DB (kprintf ("Func %ld SDT_TagList 0x%lx\n", FuncID, msg->sipc_Data));
		    }
		    else if (msg->sipc_DType == APSH_SDT_Data)
		    {
			/* they passed us a pointer to data */
			tg[0].ti_Data = (ULONG) msg->sipc_Data;
			tg[1].ti_Data = msg->sipc_DSize;
			tl = tg;

			DB (kprintf ("Func %ld SDT_Data 0x%lx\n", FuncID, msg->sipc_Data));
		    }
		    else
		    {
			tg[0].ti_Tag = TAG_IGNORE;
			tg[1].ti_Tag = TAG_IGNORE;
			tg[3].ti_Tag = TAG_MORE;
			tg[3].ti_Data = (ULONG) ptl;
			tl = tg;

			DB (kprintf ("Func %ld : Unknown type\n", FuncID));
		    }

		    /* Perform the function assigned to this message. */
		    PerfFunc (ai, FuncID, NULL, tl);
		}
		else
		{
		    DB (kprintf ("no command assign to message\n"));
		}
	    }
	    else
	    {
		tg[0].ti_Tag = TAG_IGNORE;
		tg[1].ti_Tag = TAG_IGNORE;
		tg[3].ti_Tag = TAG_MORE;
		tg[3].ti_Data = (ULONG) ptl;
		tl = tg;

		/* they sent a command string */
		DB (kprintf ("SDT_Command %s\n", msg->sipc_Data));
		PerfFunc (ai, NULL, (STRPTR) msg->sipc_Data, tl);
	    }

	    /* check to see if this was an activate message */
	    if (FuncID == ActivateID)
	    {
		FreeVec ((APTR) msg);
	    }
	    else
	    {
		/* Set the return values */
		msg->sipc_Pri_Ret = ai->ai_Pri_Ret;
		msg->sipc_Sec_Ret = ai->ai_Sec_Ret;
#if 0
		ai->ai_Pri_Ret = RETURN_FAIL;
		ai->ai_Sec_Ret = APSH_NOT_ENOUGH_MEMORY;
		ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID, ai->ai_Sec_Ret, NULL);
#endif
		ReplyMsg ((struct Message *) msg);
	    }
	}
    }

    /* Clear the current message */
    ai->ai_ActvMsg = NULL;
    ai->ai_ActvMH = NULL;

    return (TRUE);
}

BOOL close_sipc (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    register struct MHObject *mho = &(mh->mh_Header);

    mho->mho_Status &= ~APSHF_OPEN;
    return (TRUE);
}

BOOL shutdown_sipc (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    if (mh)
    {
	DB (kprintf ("%ld outstanding\n", mh->mh_Outstanding));

	/* Clear the cache */
	ai->ai_MH[MH_SIPC] = NULL;

	/* should reply to all messages before deleting!!!! */
	Remove ((struct Node *) mh);

	/* make sure there is a message port */
	if (mh->mh_Port)
	{
	    RemoveMsgPort (mh->mh_Port);
	}

	/* free the port name */
	if (mh->mh_PortName)
	    FreeVec ((APTR) mh->mh_PortName);

	/* free the message handler data */
	FreeVec ((APTR) mh);
    }
    return (TRUE);
}

BOOL send_sipc_command (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    ULONG type = 0L, dlen = 0L, dtype = 0L;
    struct SIPCMessage *msg = NULL;
    struct TagItem *tg1, *tg2;
    struct MsgPort *mp = NULL;
    BOOL foundport = FALSE;
    STRPTR name = NULL;
    APTR data;

    DB (kprintf ("send_sipc_command\n"));

    /* Get the destination */
    if ((tg1 = FindTagItem (APSH_NameTag, tl)) ||
	(tg2 = FindTagItem (APSH_PortAddr, tl)))
    {
	if (tg1)
	{
	    name = (STRPTR) tg1->ti_Data;
	}
	else if (tg2)
	{
	    mp = (struct MsgPort *) tg2->ti_Data;
	}

	/* Command type */
	type = GetTagData (APSH_CmdID, NULL, tl);

	/* Command data */
	if (data = (APTR) GetTagData (APSH_CmdData, NULL, tl))
	{
	    /* Was a length provided? */
	    if (dlen = GetTagData (APSH_CmdDataLength, NULL, tl))
	    {
		dtype = APSH_SDT_Data;
	    }
	    else
	    {
		dtype = APSH_SDT_TagList;
	    }
	}
	/* Command string */
	else if (data = (APTR) GetTagData (APSH_CmdString, NULL, tl))
	{
	    dtype = APSH_SDT_Command;
	}

	/* Allocate a message */
	if (msg = (struct SIPCMessage *)
	    AllocVec (sizeof (struct SIPCMessage), MEMF_PUBLIC | MEMF_CLEAR))
	{
	    /* Fill out the Exec message */
	    msg->sipc_Msg.mn_Node.ln_Type = NT_MESSAGE;
	    msg->sipc_Msg.mn_Length = sizeof (struct SIPCMessage);
	    msg->sipc_Msg.mn_ReplyPort = mh->mh_Port;

	    /* Fill in the SIPC message portion */
	    msg->sipc_Type = type;
	    msg->sipc_Data = data;
	    msg->sipc_DSize = dlen;
	    msg->sipc_DType = dtype;
	    msg->sipc_Extens1 = ai;

	    if (name)
	    {
		if (!(foundport = SafePutToPort ((struct Message *) msg, name)))
		{
		    DB (kprintf ("%s: unable to send cmd\n", ai->ai_AppName));
		    FreeVec ((APTR) msg);
		}
		else
		{
		    /* Increment the outstanding message counter */
		    mh->mh_Outstanding++;
		    ai->ai_NumCmds++;

		    DB (kprintf ("%s: sent command\n", ai->ai_AppName));
		}
	    }
	    else if (mp)
	    {
		DB (kprintf ("%s: sent command\n", ai->ai_AppName));

		PutMsg (mp, (struct Message *) msg);

		/* Increment the outstanding message counter */
		mh->mh_Outstanding++;
		ai->ai_NumCmds++;

		foundport = TRUE;
	    }
	}
    }

    return (foundport);
}

BOOL SafePutToPort (struct Message * message, STRPTR name)
{
    struct MsgPort *port;

    Forbid ();

    port = FindPort (name);
    if (port)
	PutMsg (port, message);

    Permit ();

    return ((BOOL) port);	/* If zero, the port has gone away */
}

/*** Somewhat private functions *********************************************/

BOOL SendSIPCMessage (STRPTR name, ULONG type, VOID * data)
{
    struct SIPCMessage *msg;
    BOOL foundport = FALSE;

    if (msg = (struct SIPCMessage *)
	AllocVec (sizeof (struct SIPCMessage), MEMF_PUBLIC | MEMF_CLEAR))
    {
	msg->sipc_Msg.mn_Node.ln_Type = NT_MESSAGE;
	msg->sipc_Msg.mn_Length = sizeof (struct SIPCMessage);
	msg->sipc_Msg.mn_ReplyPort = NULL;
	msg->sipc_Type = type;
	msg->sipc_Data = data;

	/* send the message if possible */
	if (!(foundport = SafePutToPort ((struct Message *) msg, name)))
	{
	    FreeVec ((APTR) msg);
	}
    }

    return (foundport);
}

/* send a message to a private port --- no guarantees */
BOOL SendSIPCMessageP (struct MsgPort * port, struct MsgPort * reply,
		        ULONG type, VOID * data)
{
    struct SIPCMessage *msg;
    BOOL retval = FALSE;

    if (port)
    {
	if (msg = (struct SIPCMessage *)
	    AllocVec (sizeof (struct SIPCMessage), MEMF_PUBLIC | MEMF_CLEAR))
	{
	    msg->sipc_Msg.mn_Node.ln_Type = NT_MESSAGE;
	    msg->sipc_Msg.mn_Length = sizeof (struct SIPCMessage);
	    msg->sipc_Msg.mn_ReplyPort = reply;
	    msg->sipc_Type = type;

	    /* Are they sending data */
	    if (data)
	    {
		msg->sipc_DType = APSH_SDT_Data;
		msg->sipc_Data = data;
	    }

	    /* send the message */
	    DB (kprintf ("PutMsg to 0x%lx from 0x%lx (data 0x%lx)\n", port, reply, data));
	    PutMsg (port, (struct Message *) msg);

	    retval = TRUE;
	}
    }
    return (retval);
}

/*o***** appshell.library/OpenSIPC ***********************************
*
*   NAME
*	OpenSIPC - Open a SIPC data transmission stream
*
*   SYNOPSIS
*	handle = OpenSIPC (ai, name, attrs);
*
*	APTR handle;
*	struct AppInfo *ai;
*	STRPTR name;
*	struct TagItem *attrs;
*
*   FUNCTION
*	This function attempts to register a direct SIPC data transmission
*	stream with the named port.
*
*   EXAMPLE
*
*	APTR handle;
*	STRPTR name;
*
*	handle = OpenSIPC(ai, "appshell_debugger", NULL);
*
*	SIPCPrintf(handle, "opening %s for input\n", name);
*
*	handle = CloseSIPC(handle, NULL);
*
*   INPUTS
*
*   RESULT
*
*   SEE ALSO
*	SIPCPrintf(), CloseSIPC()
*
*********************************************************************
*
* Created:  28-May-90, David N. Junod
*
*/

APTR OpenSIPC (struct AppInfo * ai, STRPTR name, struct TagItem * tl)
{
    struct SIPCHandle *sh;
    struct SIPCMessage *msg;
    ULONG msize;
    BOOL fail = FALSE;

    /* how much memory do we need */
    msize = sizeof (struct SIPCHandle) + strlen (name) + 2L;

    /* allocate the SIPC handle */
    if ((sh = (struct SIPCHandle *) AllocVec (msize, MEMF_CLEAR | MEMF_PUBLIC)) &&
	(sh->sh_Buffer = (STRPTR) AllocVec (512L, MEMF_CLEAR | MEMF_PUBLIC)))
    {
	/* only type currently supported... */
	sh->sh_Type = APSH_SDT_Text;

	/* initialize the AppInfo pointer */
	sh->sh_AI = ai;

	/* make a copy of the name */
	sh->sh_Name = MEMORY_FOLLOWING (sh);
	strcpy (sh->sh_Name, name);

	if (sh->sh_Port = CreatePort (NULL, NULL))
	{
	    /* initialize the message portion of the handler */
	    msg = &(sh->sh_Msg);
	    msg->sipc_Msg.mn_Node.ln_Type = NT_MESSAGE;
	    msg->sipc_Msg.mn_Length = sizeof (struct SIPCMessage);
	    msg->sipc_Msg.mn_ReplyPort = sh->sh_Port;
	    msg->sipc_Type = LoginID;
	    msg->sipc_DType = APSH_SDT_Data;
	    msg->sipc_Data = sh;
	    msg->sipc_DSize = sizeof (struct SIPCHandle);

	    /* send the message if possible */
	    if (SafePutToPort ((struct Message *) msg, name))
	    {
		/* wait for the reply */
		WaitPort (sh->sh_Port);

		/* get the connection response */
		msg = (struct SIPCMessage *) GetMsg (sh->sh_Port);

		/* do we have a connection? */
		if (msg->sipc_Pri_Ret == RETURN_OK)
		{
		    /* they are supposed to return their address */
		    sh->sh_Dest = (struct MsgPort *) msg->sipc_Data;
		}
		else
		{
#if 0
		    printf ("/* other party refused to talk to us */\n");
#endif
		    fail = TRUE;
		}
	    }
	    else
	    {
#if 0
		printf ("/* unable to send the message to the named party */\n");
#endif
		fail = TRUE;
	    }

	}			/* end of CreatePort */
	else
	{
	    /* unable to create a message port */
	    fail = TRUE;
	}

    }				/* end of AllocVec SIPCHandle */
    else
	fail = TRUE;

    if (fail)
    {
	if (sh)
	{
	    /* we're done with the reply port */
	    if (sh->sh_Port)
		DeletePort (sh->sh_Port);

	    /* free the SIPCPrintf buffer */
	    if (sh->sh_Buffer)
		FreeVec ((APTR) sh->sh_Buffer);

	    /* free the SIPCHandle */
	    FreeVec ((APTR) sh);
	    sh = NULL;
	}

    }				/* end of if fail */


    return ((APTR) sh);
}

/*o***** appshell.library/SIPCPrintf *********************************
*
*   NAME
*	SIPCPrintf - send the formatted data stream to the SIPC handle
*
*   SYNOPSIS
*	return = SIPCPrintf(handle, arg1, ...);
*
*	APTR return;
*	APTR handle;
*	APTR arg1;
*
*   FUNCTION
*	This function will send the formatted data stream to the
*	requested SIPC handle.
*
*	This function allows us to talk to another application.  The
*	only feedback that we get is an occasional nod.  If the other
*	party decides to no longer listen to us, then they politely
*	wait until we finish saying something before they tell us
*	goodbye.  If we want, we can still talk to them after they
*	leave without hurting anyone.
*
*   INPUTS
*
*   RESULT
*
*   BUGS
*
*   SEE ALSO
*
*********************************************************************
*
* Created:  28-May-90, David N. Junod
*
*/

APTR SIPCPrintf (APTR ash, APTR data,...)
{
    struct SIPCHandle *sh = (struct SIPCHandle *) ash;
    struct SIPCMessage *msg;

    if (sh && data)
    {
	/* prepare the data */
	strcpy (sh->sh_Buffer, data);

	/* initialize the message portion of the handler */
	msg = &(sh->sh_Msg);
	msg->sipc_Type = DataStreamID;
	msg->sipc_DType = APSH_SDT_Data;
	msg->sipc_Data = sh->sh_Buffer;
	msg->sipc_DSize = strlen (sh->sh_Buffer);

	/* send the data transmission */
	PutMsg (sh->sh_Dest, (struct Message *) msg);

	/* wait for the reply */
	WaitPort (sh->sh_Port);

	/* get the connection response */
	msg = (struct SIPCMessage *) GetMsg (sh->sh_Port);

	/* do they still want to talk to us? */
	if (msg->sipc_Pri_Ret != RETURN_OK)
	{
	    /* we're done with the reply port */
	    if (sh->sh_Port)
		DeletePort (sh->sh_Port);
	    sh->sh_Port = NULL;

	    /* free the SIPCPrintf buffer */
	    if (sh->sh_Buffer)
		FreeVec ((APTR) sh->sh_Buffer);

	    /* free the SIPCHandle */
	    FreeVec ((APTR) sh);
	    sh = NULL;
	}
    }

    return ((APTR) sh);
}

/*o***** appshell.library/CloseSIPC ***********************************
*
*   NAME
*	CloseSIPC - Closes a SIPC data transmission stream
*
*   SYNOPSIS
*	return = CloseSIPC (handle, attrs);
*
*	APTR return;
*	APTR handle;
*	struct TagItem *attrs;
*
*   FUNCTION
*	Informs the other party that we no longer need to talk to
*	them.
*
*   INPUTS
*
*   RESULT
*
*   BUGS
*
*   SEE ALSO
*
*********************************************************************
*
* Created:  28-May-90, David N. Junod
*
*/

APTR CloseSIPC (APTR ash, struct TagItem * tl)
{
    struct SIPCHandle *sh = (struct SIPCHandle *) ash;
    struct SIPCMessage *msg;

    if (sh)
    {
	/* initialize the message portion of the handler */
	msg = &(sh->sh_Msg);
	msg->sipc_Type = LogoutID;
	msg->sipc_DType = APSH_SDT_Data;
	msg->sipc_Data = sh;
	msg->sipc_DSize = sizeof (struct SIPCHandle);

	/* send the logout message */
	PutMsg (sh->sh_Dest, (struct Message *) msg);

	/* wait for the reply */
	WaitPort (sh->sh_Port);

	/* get the connection response */
	msg = (struct SIPCMessage *) GetMsg (sh->sh_Port);

	/* ... we don't really do anything with the reply ... */

	/* we're done with the reply port */
	if (sh->sh_Port)
	    DeletePort (sh->sh_Port);
	sh->sh_Port = NULL;

	/* free the SIPCPrintf buffer */
	if (sh->sh_Buffer)
	    FreeVec ((APTR) sh->sh_Buffer);

	/* free the SIPCHandle */
	FreeVec ((APTR) sh);
	sh = NULL;
    }

    /* return a nice clean NULL */
    return ((APTR) NULL);
}
