/* apsh_tool.c
 * Copyright (C) 1990 Commodore-Amiga, Inc.
 * written by David N. Junod
 *
 * multi-threaded process handling
 *
 * $Id: apsh_tool.c,v 1.4 1992/09/07 18:00:08 johnw Exp johnw $
 *
 * $Log: apsh_tool.c,v $
 * Revision 1.4  1992/09/07  18:00:08  johnw
 * Minor changes.
 *
 * Revision 1.1  91/12/12  14:54:46  davidj
 * Initial revision
 * 
 * Revision 1.1  90/07/02  11:40:48  davidj
 * Initial revision
 *
 *
 */

#include "appshell_internal.h"

void kprintf (void *,...);

#define	DB(x)	;
#define	DE(x)	;

/****** appshell.library/setup_toolA ****************************************
*
*   NAME
*	setup_toolA - Initializes the tool message handler.
*
*   SYNOPSIS
*	mh = setup_toolA (ai, attrs)
*	D0                D0  D1
*
*	struct MsgHandler *mh;
*	struct AppInfo *ai;
*	struct TagItem *attrs;
*
*   FUNCTION
*	This is the low-level function used to initialize the tool message
*	handler.  This message handler is responsible for maintaining the
*	initialization & shutdown of asynchronous processes within an
*	application.
*
*	NOTE: This call should not be called directly by the application,
*	but should invoked by using the APSH_AddTool_UI tag.
*
*	Valid tagitems to use at OPEN time:
*
*	    APSH_Tool, <name>
*	    Name to give to the process that is spawned.  Required.
*
*	    APSH_ToolData, <data>
*	    Data to pass to the new process.  Passes a pointer to the
*	    AppInfo structure as default.  If you are starting a new AppShell
*	    application, then a tag list should be passed.
*
*	    APSH_ToolAddr, <function>
*	    Pointer to the function to spawn as a new process.  Defaults to
*	    HandleAppAsync.
*
*	    APSH_ToolStack, <stack size>
*	    Stack size to allocate for the new process.  Defaults to 4096
*	    bytes.
*
*	    APSH_ToolPri, <priority>
*	    Priority to run the new process at.  Defaults to zero.
*
*   EXAMPLE
*
*	\* Start a project as a new process *\
*	VOID NewProject (struct AppInfo *ai, STRPTR cmd, struct TagItem *attrs)
*	{
*	    extern struct TagItem Cloned_App[];
*	    struct MsgHandler *mh;
*	    struct TagItem *tag;
*
*	    \* Search for the tag to store our SIPC address in *\
*	    if (tag = FindTagItem (APSH_PortAddr, Cloned_App))
*	    {
*		\* See if we have a SIPC port *\
*		if (mh = HandlerData (ai, APSH_Handler, "SIPC", TAG_DONE))
*		{
*		    \* Get the address of our SIPC port *\
*		    tag->ti_Data = (ULONG) mh->mh_Port;
*		}
*	    }
*
*	    \* Start the new process *\
*           HandlerFunc (ai,
*			 APSH_Handler,	"TOOL",
*			 APSH_Command,	APSH_MH_OPEN,
*			 APSH_Tool,	"MiniBuild_Project",
*			 APSH_ToolData, Cloned_App,
*			 TAG_DONE);
*	}
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

struct MsgHandler *setup_toolA (struct AppInfo * ai, struct TagItem * tl)
{
    register struct MsgHandler *mh;
    register struct MHObject *mho;
    register struct ToolInfo *md;
    ULONG msize, hstatus;

    /* calculate the size of our memory block */
    msize = sizeof (struct MsgHandler) +
      sizeof (struct ToolInfo) +
      (4L * sizeof (ULONG));

    /* allocate our memory block */
    if (mh = (struct MsgHandler *) AllocVec (msize, MEMF_PUBLIC | MEMF_CLEAR))
    {
	ai->ai_MH[MH_TOOL] = mh;

	/* get a pointer to the object structure */
	mho = &(mh->mh_Header);

	/* set up our object node */
	mho->mho_Node.ln_Type = APSH_MH_HANDLER_T;
	mho->mho_Node.ln_Pri = (APSH_MH_HANDLER_P + 2);
	mho->mho_Node.ln_Name = "TOOL";

	/* initialize our object list */
	NewList (&(mho->mho_ObjList));

	/* set up our ID */
	mho->mho_ID = APSH_TOOL_ID;

	/* initialize our data area */
	mho->mho_SysData = md = MEMORY_FOLLOWING (mh);
	InitSemaphore (&(md->ti_Sem));

	/* set up message handler function array */
	mh->mh_NumFuncs = 4;
	mh->mh_Func = MEMORY_FOLLOWING (md);
	mh->mh_Func[APSH_MH_OPEN] = open_tool;
	mh->mh_Func[APSH_MH_HANDLE] = handle_tool;
	mh->mh_Func[APSH_MH_CLOSE] = close_tool;
	mh->mh_Func[APSH_MH_SHUTDOWN] = shutdown_tool;

	/* get activation status */
	hstatus = GetTagData (APSH_Status, NULL, tl);

	/* create tool reply port */
	if (mh->mh_Port = CreatePort (NULL, NULL))
	{
	    /* remember the signal bits */
	    mh->mh_SigBits = (1L << mh->mh_Port->mp_SigBit);

	    /* open immediately? */
	    if (hstatus & APSHP_INACTIVE)
	    {
		return (mh);
	    }
	    else
	    {
		if (open_tool (ai, mh, tl))
		{
		    return (mh);
		}
	    }

	    /* make a nice clean failure path */
	    if (mh->mh_Port)
	    {
		DeletePort (mh->mh_Port);
		mh->mh_Port = NULL;
	    }
	}
	else
	{
	    ai->ai_Pri_Ret = RETURN_FAIL;
	    ai->ai_Sec_Ret = APSH_CLDNT_CREATE_PORT;
	    ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID, ai->ai_Sec_Ret,
				       (int) "Tool");
	}

	FreeVec ((APTR) mh);
	mh = NULL;
    }
    else
    {
	ai->ai_Pri_Ret = RETURN_FAIL;
	ai->ai_Sec_Ret = APSH_NOT_ENOUGH_MEMORY;
	ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID, ai->ai_Sec_Ret, NULL);
    }

    return (mh);
}

static VOID __saveds StartUpTool (VOID)
{
    struct Process *proc = (struct Process *) FindTask (NULL);
    struct MsgPort *port, *sipc_port;
    struct ToolMessage *msg;
    struct ToolFuncNode *tfn;
    struct SignalSemaphore *ss = NULL;
    struct Message *tmp;
    BPTR lock;

    port = &(proc->pr_MsgPort);

    WaitPort (port);

    /* get the tool information */
    msg = (struct ToolMessage *) GetMsg (port);
    tfn = msg->tm_TFN;

    /* create an SIPC port for activation/shutdown */
    if (sipc_port = msg->tm_SIPC = CreatePort (NULL, NULL))
    {
	/* reply with the newly created port address added to the msg */
	ReplyMsg ((struct Message *) msg);

	/* wait for the handler to reply back to me */
	WaitPort (sipc_port);

	/* get the message off the port */
	msg = (struct ToolMessage *) GetMsg (sipc_port);
	tfn = msg->tm_TFN;

	/* get a lock on the current directory */
	lock = CurrentDir (tfn->tfn_Lock);

	/* Make sure we're the ones specifying the program directory */
	if (proc->pr_HomeDir)
	{
	    UnLock (proc->pr_HomeDir);
	}

	/* Set up the PROGDIR */
	if (proc->pr_HomeDir = DupLock (tfn->tfn_Lock))
	{
	    /* call the function */
	    (*(tfn->tfn_Func)) (tfn->tfn_Data, sipc_port);
	    DB (kprintf("tool complete\n"));

	    /* Release the home directory */
	    UnLock (proc->pr_HomeDir);
	    proc->pr_HomeDir = NULL;
	}

	/* go back to the default directory */
	lock = CurrentDir (lock);

	/* obtain semaphore */
	ObtainSemaphore (ss = msg->tm_Sem);

	/* clear out all the messages */
	DB (kprintf("Clear out messages\n"));
	while (tmp = GetMsg (sipc_port))
	{
	    /* set error code... */
	    DE (kprintf("replymsg 0x%lx\n", tmp));
	    ReplyMsg (tmp);
	}
	DB (kprintf("done clearing messages\n"));

	/* okay we're done with port, delete it */
	DeletePort (sipc_port);
	sipc_port = msg->tm_SIPC = NULL;
    }

    /* indicate that this is the tool shutdown message */
    msg->tm_Type = ShutdownMsgID;

    /* fall out... */
    Forbid ();
    DE (kprintf("ReplyMsg 0x%lx\n", msg));
    ReplyMsg ((struct Message *) msg);

    /* okay, unlock the semaphore */
    if (ss)
    {
	DE (kprintf("ReleaseSemaphore 0x%lx\n", ss));
	ReleaseSemaphore (ss);
    }

    DE (kprintf("exiting %s\n", (FindTask(NULL))->tc_Node.ln_Name));
}

BOOL open_tool (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    struct MHObject *mho = &(mh->mh_Header);
    struct ToolInfo *md = mho->mho_SysData;
    struct ToolFuncNode *tfn = NULL;
    struct MsgPort *proc = NULL;
    struct List *list = NULL;
    UBYTE *name;
    BPTR lock;

    if (mh)
    {
	/* make it as being opened */
	mho->mho_Status |= APSHF_OPEN;

	list = &(mho->mho_ObjList);

	/* get the name of the tool */
	if (name = (UBYTE *) GetTagData (APSH_Tool, NULL, tl))
	{
	    /* Allocate the function node */
	    if (tfn = (struct ToolFuncNode *)
		AllocVec (sizeof (struct ToolFuncNode) + (LONG) strlen (name),
			  MEMF_CLEAR | MEMF_PUBLIC))
	    {
		struct TagItem *asc;

		/* copy the name of the process */
		strcpy (tfn->tfn_Node.ln_Name = tfn->tfn_Name, name);

		/* get the process information */
		tfn->tfn_Data = (VOID *)
		  GetTagData (APSH_ToolData, (ULONG) ai, tl);

		/* Get the address of the tool */
		if (asc = FindTagItem (APSH_ToolAddr, tl))
		{
		    tfn->tfn_Func = (F_PTR) asc->ti_Data;
		}
		else
		{
		    tfn->tfn_Func = (F_PTR) HandleAppAsync;
		}

		tfn->tfn_Stack = GetTagData (APSH_ToolStack, 4096L, tl);
		tfn->tfn_Priority = GetTagData (APSH_ToolPri, 0L, tl);

		/* setup the message portion */
		tfn->tfn_Msg.tm_Msg.mn_Node.ln_Type = NT_MESSAGE;
		tfn->tfn_Msg.tm_Msg.mn_ReplyPort = mh->mh_Port;
		tfn->tfn_Msg.tm_Msg.mn_Length = sizeof (struct ToolMessage);
		tfn->tfn_Msg.tm_Type = StartupMsgID;
		tfn->tfn_Msg.tm_TFN = tfn;
		tfn->tfn_Msg.tm_SIPC = NULL;
		tfn->tfn_Msg.tm_Sem = &(md->ti_Sem);

		/* setup the segment list portion */
		tfn->tfn_SegList.SegSize = sizeof (struct CodeHdr);
		tfn->tfn_SegList.JumpInstr = 0x4EF9;
		tfn->tfn_SegList.Function = (APTR) StartUpTool;

		/* setup the process' default directory */
		lock = CurrentDir (NULL);
		tfn->tfn_Lock = DupLock (lock);
		lock = CurrentDir (lock);

		/* not done yet... hasn't started yet! */
		tfn->tfn_Key = FALSE;

		/* fork the process */
		if (proc =
		    CreateProc (tfn->tfn_Node.ln_Name,
				tfn->tfn_Priority,
				TO_BPTR (&(tfn->tfn_SegList)),
				tfn->tfn_Stack))
		{
		    PutMsg (proc, &(tfn->tfn_Msg.tm_Msg));

		    AlphaEnqueue (list, (struct Node *) tfn);

		    return (TRUE);
		}
		else
		    NotifyUser (ai, "Unable to start tool", NULL);

		FreeVec ((APTR) tfn);
	    }
	}
    }
    return (TRUE);
}

BOOL handle_tool (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    struct MHObject *mho = &(mh->mh_Header);
    struct ToolMessage *msg;
    struct ToolFuncNode *tfn;

    /* Set the active message handler ID */
    ai->ai_ActvMH = mho->mho_ID;

    /* Pull each message */
    while (msg = (struct ToolMessage *) GetMsg (mh->mh_Port))
    {
	/* Set the current message */
	ai->ai_ActvMsg = (struct Message *) msg;

	/* get the complete information for the tool */
	tfn = msg->tm_TFN;

	/* process tool message types */
	switch (msg->tm_Type)
	{
	    case StartupMsgID:	/* startup tool message */
		if (msg->tm_SIPC)
		{
		    /* send the message right back */
		    msg->tm_Type = ActivateToolID;
		    PutMsg (msg->tm_SIPC, (struct Message *) msg);
		}
		break;

	    case ShutdownToolID:	/* replied shutdown message */
		FreeVec ((APTR) msg);
		break;

	    case ShutdownMsgID:/* shutdown tool message */
		/* unlock the directory */
		if (tfn->tfn_Lock)
		    UnLock (tfn->tfn_Lock);

		/* remove the node */
		Remove ((struct Node *) tfn);
		FreeVec ((APTR) tfn);
	}
    }

    /* Clear the current settings */
    ai->ai_ActvMsg = NULL;
    ai->ai_ActvMH = NULL;

    return (TRUE);
}

BOOL close_tool (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    register struct MHObject *mho = &(mh->mh_Header);

    mho->mho_Status &= ~APSHF_OPEN;
    return (TRUE);
}

BOOL shutdown_tool (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    register struct MHObject *mho = &(mh->mh_Header);
    register struct ToolInfo *md = mho->mho_SysData;
    register struct List *list;
    register struct Node *node, *nxtnode;
    register struct ToolFuncNode *tfn;
    register struct ToolMessage *tm;

    if (mh)
    {
	/* obtain semaphore */
	ObtainSemaphore (&(md->ti_Sem));

	handle_tool (ai, mh, tl);

	/* send the ShutdownTool message to all tools */
	list = &(mho->mho_ObjList);
	if (list->lh_TailPred != (struct Node *) list)
	{
	    node = list->lh_Head;
	    while (nxtnode = node->ln_Succ)
	    {
		tfn = (struct ToolFuncNode *) node;
		tm = &(tfn->tfn_Msg);

		/* send the shutdown message */
		SendSIPCMessageP (tm->tm_SIPC, mh->mh_Port, ShutdownToolID, NULL);

		/* go to the next node */
		node = nxtnode;
	    }
	}

	/* release the semaphore */
	ReleaseSemaphore (&(md->ti_Sem));

	/* close all tools */
	while (list->lh_TailPred != (struct Node *) list)
	{
	    WaitPort (mh->mh_Port);

	    handle_tool (ai, mh, tl);
	}

	ai->ai_MH[MH_TOOL] = NULL;

	/* delete the tool message port */
	if (mh->mh_Port)
	    RemoveMsgPort (mh->mh_Port);

	/* remove the message handler data */
	Remove ((struct Node *) mh);
	FreeVec ((APTR) mh);
    }
    return (TRUE);
}
