/* apsh_arexx.c
 * Copyright (C) 1990 Commodore-Amiga, Inc.
 * written by David N. Junod
 *
 * AREXX message handling routines
 *
 * $Id: apsh_arexx.c,v 1.3 1992/09/07 17:48:29 johnw Exp johnw $
 *
 * $Log: apsh_arexx.c,v $
 * Revision 1.3  1992/09/07  17:48:29  johnw
 * Various minor changes.
 *
 * Revision 1.2  92/01/23  11:02:36  johnw
 * This version is cleaned up for use from V37.4 Includes
 * 
 * Revision 1.1  91/12/12  14:49:33  davidj
 * Initial revision
 *
 * Revision 1.1  90/07/02  11:16:56  davidj
 * Initial revision
 *
 *
 */
#include <rexx/storage.h>	/* off of ARexx disk */
#include <rexx/rxslib.h>	/* off of ARexx disk */

#include "appshell_internal.h"

VOID RXSFunc (struct AppInfo * ai, STRPTR args, struct TagItem * tl);

/* our handler-specific functions */
struct Funcs arexx_funcs[] =
{
    {"RX", RXFunc, RXID, "ASYNC/S,COMMAND/F", 2L, APSHF_SYSTEM,},
    {"RXS",RXSFunc,RXSID,"ASYNC/S,STRING/F",  2L, APSHF_SYSTEM,},
    {NULL, NO_FUNCTION}		/* end of array */
};

/****** appshell.library/setup_arexxA **************************************
*
*   NAME
*	setup_arexxA - Initializes the ARexx message handler.
*
*   SYNOPSIS
*	mh = setup_arexxA (ai, tags)
*	D0		D0  D1
*
*	struct MsgHandler *mh;
*	struct AppInfo *ai;
*	struct TagItem *tags;
*
*   FUNCTION
*	This is the low-level function used to initialize the ARexx message
*	hander.
*
*	NOTE: This call should not be called directly by the application,
*	but should invoked by using the APSH_AddARexx_UI tag.
*
*	Valid tagitems to use at INITIALIZATION are:
*
*	    APSH_ARexxError, <id>
*	    ID of the function to call whenever an ARexx message returns
*	    with an error.
*
*	    APSH_ARexxOK, <id>
*	    ID of the function to call whenever an ARexx message returns
*	    successfully.
*
*	    APSH_Extens, <extension>
*	    Used to specify the default macro file extension to use.
*	    Defaults to .REXX
*
*	    APSH_Port, <name>
*	    Used to specify a name for the ARexx port.  Defaults to
*	    APSH_BaseName.
*
*	    APSH_Status, <flags>
*	    Where flags can be:
*
*		APSHP_INACTIVE
*		When set, then the ARexx port will be initialized, but
*		will fail all messages received.
*
*		APSHP_SINGLE
*		When set, will not add a use count to the end of the
*		ARexx port name.
*
*	In addition to the standard message handler functions, the ARexx
*	function handler provides the following functions:
*
*	    AH_SENDCMD
*	    Send a command to ARexx.
*
*   EXAMPLE
*
*	\* To send an command to ARexx *\
*	struct MsgHandler *mh;
*	STRPTR cmd;
*
*	cmd = "\"ADDRESS COMMAND DIR\"";
*
*	\* Get a pointer to the ARexx message handler *\
*	if (mh = HandlerData (ai, APSH_Handler, "AREXX", TAG_DONE))
*	{
*	    \* Send the command to ARexx *\
*	    retval = HandlerFunc (ai,
*				  APSH_Handler, "AREXX",
*				  APSH_Command, AH_SENDCMD,
*				  APSH_CmdString, cmd,
*				  TAG_DONE);
*	}
*
*
*   INPUTS
*	ai	- Pointer to the application's AppInfo structure.
*	tags	- Pointer to an array of TagItem attributes for the
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

struct MsgHandler *setup_arexxA (struct AppInfo * ai, struct TagItem * tl)
{
    register struct MsgHandler *mh;
    register struct ARexxInfo *md;
    register struct MHObject *mho;
    register WORD cntr = 1;
    BOOL exist = TRUE;
    ULONG msize, hstatus;
    STRPTR pname = NULL;
    LONG sname;

    /* make sure that RexxSysBase is opened */
    if (RexxSysBase)
    {
	/* get the opening status */
	hstatus = GetTagData (APSH_Status, NULL, tl);

	/* get the public port name */
	pname = (UBYTE *) GetTagData (APSH_Port, (ULONG) ai->ai_BaseName, tl);

	/* Is the PORTNAME switch used? */
	if ((sname = WhichOption (ai, "portname/k")) >= 0)
	{
	    /* Was the option specified? */
	    if (ai->ai_Options[sname])
	    {
		/* Get the port name */
		pname = (STRPTR) ai->ai_Options[sname];

		/* Specified name implies single useage */
		hstatus |= APSHP_SINGLE;
	    }
	}

	/* how much memory do we need */
	msize = sizeof (struct MsgHandler) + sizeof (struct ARexxInfo)
	  + (5L * sizeof (ULONG));

	/* allocate a block of memory for the ARexx message handler */
	if (mh = (struct MsgHandler *)
	    AllocVec (msize, MEMF_CLEAR | MEMF_PUBLIC))
	{
	    /* Cache the pointer */
	    ai->ai_MH[MH_AREXX] = mh;

	    /* get a pointer to object node */
	    mho = &(mh->mh_Header);

	    /* set up the node portion of the message handler */
	    mho->mho_Node.ln_Type = APSH_MH_HANDLER_T;
	    mho->mho_Node.ln_Pri = APSH_MH_HANDLER_P;
	    mho->mho_Node.ln_Name = "AREXX";

	    /* initialize the message handler object list */
	    NewList (&(mho->mho_ObjList));

	    /* establish the base ID for the message handler */
	    mho->mho_ID = APSH_AREXX_ID;

	    /* point the message data at the right place */
	    mho->mho_SysData = md = MEMORY_FOLLOWING (mh);

	    /* Build up the attributes for sending a command */
	    md->ari_Tags[1].ti_Tag = TAG_DONE;
	    md->ari_Tags[1].ti_Data = NULL;

	    /* point the handler function table at the right place */
	    mh->mh_Func = MEMORY_FOLLOWING (md);
	    mh->mh_NumFuncs = 5;

	    /* set up the message handler function table */
	    mh->mh_Func[APSH_MH_OPEN] = open_arexx;
	    mh->mh_Func[APSH_MH_HANDLE] = handle_arexx;
	    mh->mh_Func[APSH_MH_CLOSE] = close_arexx;
	    mh->mh_Func[APSH_MH_SHUTDOWN] = shutdown_arexx;
	    mh->mh_Func[AH_SENDCMD] = send_rexx_command;

	    /* point the port name at the allocated memory */
	    msize = (strlen (pname) + 6L);
	    if (mh->mh_PortName = AllocVec (msize, MEMF_CLEAR))
	    {
		strcpy (mh->mh_PortName, pname);
		strupr (mh->mh_PortName);

		/* get the macro file extension */
		md->ari_Extens = (UBYTE *)
		    GetTagData (APSH_Extens, (ULONG) ".rexx", tl);

		/* determine ARexx command ERROR handler */
		md->ari_UFunc[ARX_MACRO_ERROR] =
		    GetTagData (APSH_ARexxError, NULL, tl);

		/* determine ARexx command OK handler */
		md->ari_UFunc[ARX_MACRO_GOOD] =
		    GetTagData (APSH_ARexxOK, NULL, tl);

		/* forbid multi-tasking while we try to get a port */
		Forbid ();

		/* get an unique port name */
		if (hstatus & APSHP_SINGLE)
		{
		    if (FindPort (mh->mh_PortName))
		    {
			/* permit multi-tasking again */
			Permit ();

			/* set up error values */
			ai->ai_Pri_Ret = RETURN_FAIL;
			ai->ai_Sec_Ret = APSH_PORT_ACTIVE;
			ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID,
					       ai->ai_Sec_Ret, (int) "ARexx");
		    }
		    else
		    {
			/* create our ARexx port */
			if (!(mh->mh_Port = CreatePort (mh->mh_PortName, 0L)))
			{
			    /* permit multi-tasking again */
			    Permit ();

			    /* set up error values */
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
		/* allow multiple ports */
		else
		{
		    while (exist)
		    {
			/* create a name with our base name and a number */
			sprintf (mh->mh_PortName, "%s.%ld",
				 pname, (LONG) cntr);

			/* Make sure the name is upper-case */
			strupr (mh->mh_PortName);

			/* see if someone has already taken this name */
			if (!FindPort (mh->mh_PortName))
			    exist = FALSE;

			cntr++;
		    }

		    /* create our ARexx port */
		    if (!(mh->mh_Port = CreatePort (mh->mh_PortName, 0L)))
		    {
			/* permit multi-tasking again */
			Permit ();

			/* set up error values */
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

		/* Successful so far? */
		if (ai->ai_Pri_Ret == RETURN_OK)
		{
		    /* set up the signal bits */
		    mh->mh_SigBits = (1L << mh->mh_Port->mp_SigBit);

		    /* add handler functions to the table */
		    AddFuncEntries (ai, arexx_funcs);

		    /* open immediately? */
		    if (hstatus & APSHP_INACTIVE)
		    {
			return (mh);
		    }
		    else if (open_arexx (ai, mh, tl))
		    {
			return (mh);
		    }
		}

		/* make a nice clean failure path */
		if (mh->mh_Port)
		{
		    RemoveMsgPort (mh->mh_Port);
		}

		/* free the port name */
		FreeVec ((APTR) mh->mh_PortName);
	    }
	    else
	    {
		ai->ai_Sec_Ret = APSH_NOT_ENOUGH_MEMORY;
		ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID, ai->ai_Sec_Ret, NULL);
	    }

	    /* free the memory block */
	    FreeVec ((APTR) mh);
	}
	else
	{
	    ai->ai_Sec_Ret = APSH_NOT_ENOUGH_MEMORY;
	    ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID, ai->ai_Sec_Ret, NULL);
	}
    }
    else
    {
	ai->ai_Sec_Ret = APSH_NOT_AVAILABLE;
	ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID, ai->ai_Sec_Ret,
				   (int) "rexxsyslib.library");
    }

    /* If we're here, it's a failure */
    ai->ai_Pri_Ret = RETURN_FAIL;

    return (NULL);
}

BOOL open_arexx (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    /* open the ARexx port */
    mh->mh_Header.mho_Status |= APSHF_OPEN;

    return (TRUE);
}

/* ARexx message processing */
BOOL handle_arexx (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    register struct MHObject *mho = &(mh->mh_Header);
    register struct ARexxInfo *md = mho->mho_SysData;
    register struct RexxMsg *msg;	/* incoming ARexx messages */

    /* Set the active message handler ID */
    ai->ai_ActvMH = mho->mho_ID;

    /* handle all of our messages */
    while (msg = (struct RexxMsg *) GetMsg (mh->mh_Port))
    {
	/* Set the active message */
	ai->ai_ActvMsg = (struct Message *) msg;

	if (msg->rm_Node.mn_Node.ln_Type == NT_REPLYMSG)
	{
	    /* This is a reply to a previous message */
	    if (msg->rm_Result1)
	    {
		if (md->ari_UFunc[ARX_MACRO_ERROR])
		{
		    PerfFunc (ai, md->ari_UFunc[ARX_MACRO_ERROR], NULL, tl);
		}
	    }
	    else if (md->ari_UFunc[ARX_MACRO_GOOD])
	    {
		PerfFunc (ai, md->ari_UFunc[ARX_MACRO_GOOD], NULL, tl);
	    }

	    /* delete the argument that we originally sent */
	    DeleteArgstring (msg->rm_Args[0]);

	    /* delete the extended message */
	    DeleteRexxMsg (msg);

	    /* decrement the count of outstanding messages */
	    ai->ai_NumCmds--;
	    mh->mh_Outstanding--;
	}
	else
	{
	    /* We have received a command/function message */
	    execute_command (msg, ai, mh, tl);
	}
    }

    /* Clear the active message */
    ai->ai_ActvMsg = NULL;
    ai->ai_ActvMH = NULL;

    return (TRUE);
}

BOOL close_arexx (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    /* close the ARexx port */
    mh->mh_Header.mho_Status &= ~APSHF_OPEN;

    return (TRUE);
}

BOOL shutdown_arexx (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{

    /* Clear the cache */
    ai->ai_MH[MH_AREXX] = NULL;

    /* remove the message handler node from the list */
    Remove ((struct Node *) mh);

    /* delete the message port */
    if (mh->mh_Port)
    {
	DeletePort (mh->mh_Port);
    }

    /* free the port name */
    if (mh->mh_PortName)
	FreeVec ((APTR) mh->mh_PortName);

    /* free the memory block */
    FreeVec ((APTR) mh);
    mh = NULL;

    return (TRUE);
}

/* This is very inefficient SHIT---copied from ARexx disk */
BOOL send_rexx_command (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    register struct MHObject *mho = &(mh->mh_Header);
    register struct ARexxInfo *md = mho->mho_SysData;
    register struct MsgPort *rexxport;	/* this will be rexx's port */
    struct RexxMsg *rexx_command_message;	/* this is the message */
    BOOL StringFile = FALSE;
    UBYTE *cmdstr;

    /* get the command string */
    if (cmdstr = (UBYTE *) GetTagData (APSH_ARexxString, NULL, tl))
    {
	/* Indicate that we're working with a string file */
	StringFile = TRUE;
    }
    else
    {
	cmdstr = (UBYTE *) GetTagData (APSH_CmdString, NULL, tl);
    }

    /* lock things temporarily */
    Forbid ();

    /* if ARexx is not active, just return FALSE */
    if (!(rexxport = FindPort (RXSDIR)))
    {
	Permit ();

	/* set up error values */
	ai->ai_Pri_Ret = RETURN_FAIL;
	ai->ai_Sec_Ret = APSH_NOT_AVAILABLE;
	ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID, ai->ai_Sec_Ret, (int) RXSDIR);

	return (FALSE);
    }

    /* allocate a message packet for our command */
    /* note that this is a very important call.  Much flexibility is */
    /* available to you here by using multiple host port names, etc. */
    if (!(rexx_command_message = CreateRexxMsg (mh->mh_Port,
						md->ari_Extens,
						mh->mh_Port->mp_Node.ln_Name)))
    {
	Permit ();

	/* set up error values */
	ai->ai_Pri_Ret = RETURN_FAIL;
	ai->ai_Sec_Ret = APSH_NOT_AVAILABLE;
	ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID, ai->ai_Sec_Ret, (int) RXSDIR);

	return (FALSE);
    }

    /* create an argument string and install it in the message */
    if (!(rexx_command_message->rm_Args[0] =
	  CreateArgstring (cmdstr, strlen (cmdstr))))
    {
	DeleteRexxMsg (rexx_command_message);
	Permit ();

	/* set up error values */
	ai->ai_Pri_Ret = RETURN_FAIL;
	ai->ai_Sec_Ret = APSH_NOT_AVAILABLE;
	ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID, ai->ai_Sec_Ret, (int) RXSDIR);

	return (FALSE);
    }

    /* indicate that message is a command */
    rexx_command_message->rm_Action = RXCOMM | (StringFile?(1L << RXFB_STRING):0);

    /* send the message */
    PutMsg (rexxport, (struct Message *) rexx_command_message);

    /* Start multitasking back up */
    Permit ();

    /* increment the number of outstanding messages */
    ai->ai_NumCmds++;
    mh->mh_Outstanding++;

    return (TRUE);
}

VOID execute_command (struct RexxMsg * rmsg, struct AppInfo * ai,
		       struct MsgHandler * mh, struct TagItem * tl)
{
    register struct MHObject *mho = &(mh->mh_Header);
    UBYTE *cmd = ARG0 (rmsg);

    /* Don't perform function if we are suspended */
    if ((mho->mho_Status & APSHF_OPEN) &&
	!(mho->mho_Status & APSHF_DISABLED))
    {
	PerfFunc (ai, NULL, cmd, tl);
    }
    else
    {
	ai->ai_Pri_Ret = RETURN_FAIL;
	ai->ai_Sec_Ret = APSH_DISABLED;
	ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID, ai->ai_Sec_Ret, (int) cmd);
    }

    /* reply to the message */
    reply_rexx_command (rmsg, ai, mh, tl);
}

/*
extern LONG __asm CheckRexxMsg (register __a0 struct RexxMsg *);
extern LONG __asm SetRexxVar (register __a0 struct RexxMsg *,
                              register __a1 UBYTE *,
                              register __d0 UBYTE *,
                              register __d1 LONG);
*/

/* Replys to a ARexx message */
VOID reply_rexx_command (struct RexxMsg * rmsg, struct AppInfo * ai,
			  struct MsgHandler * mh, struct TagItem * tl)
{

    /* Save the last error */
    if (ai->ai_Pri_Ret && ai->ai_Sec_Ret)
    {
	/* save the last error */
	ai->ai_LastError = ai->ai_Sec_Ret;

	/* Do we have any text? */
	if (ai->ai_TextRtn)
	{
	    strcpy (ai->ai_LastText, ai->ai_TextRtn);
	}

	/* Make sure it's a message that can handle getting a variable */
	if (CheckRexxMsg ((struct Message *) rmsg))
	{
	    UBYTE es[10];

	    /* Build the RVI name */
	    sprintf (ai->ai_TempText, "%s.LASTERROR", ai->ai_BaseName);

	    /* Must be in uppercase for the macro to see it */
	    strupr (ai->ai_TempText);

	    /* Build the value string */
	    sprintf (es, "%ld", ai->ai_Sec_Ret);

	    /* Set the RVI */
	    SetRexxVar ((struct Message *) rmsg,
	                ai->ai_TempText,
	                es,
	                strlen (es));
	}
    }

    /* set an error code */
    if (ai->ai_Pri_Ret == RETURN_OK &&
	(rmsg->rm_Action & (1L << RXFB_RESULT)))
    {
	ai->ai_Sec_Ret = (ai->ai_TextRtn) ?
	  (LONG) CreateArgstring (ai->ai_TextRtn, strlen (ai->ai_TextRtn)) :
	  (LONG) NULL;
    }
    else
    {
	/* can't have a secondary result field */
	ai->ai_Sec_Ret = NULL;
    }

    /* set the result fields */
    rmsg->rm_Result1 = ai->ai_Pri_Ret;
    rmsg->rm_Result2 = ai->ai_Sec_Ret;

    /* send the ARexx message back */
    ReplyMsg ((struct Message *) rmsg);

    /* Clear the error information now that it's been handled */
    ai->ai_TextRtn = NULL;
    ai->ai_Pri_Ret = NULL;
    ai->ai_Sec_Ret = NULL;
}

/****** appshell.library/__RX ************************************************
*
*   NAME
*	RX - Execute an ARexx macro.
*
*   SYNOPSIS
*	RXID		Function ID
*
*   FUNCTION
*	Allows access to ARexx functions that may have otherwise been
*	rerouted by the application.  Should only be used if there are name
*	conflicts between an application function and an ARexx function.
*
*	As a string command line:
*
*	    RX
*	    ASYNC/S, CMD/F
*
*		ASYNC	Send the command to the asynchronous port.
*
*		CMD	Command to execute.
*
*	As a TagItem attribute list:
*
*	    APSH_CmdString, <command>
*		where <command> is a command line to pass to ARexx.
*
*	This function is implemented by the ARexx message handler.
*
*   BUGS
*	ASYNC is not implemented.
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

VOID RXFunc (struct AppInfo * ai, STRPTR args, struct TagItem * tl)
{
    struct MsgHandler *mh;

    if (mh = HandlerData (ai, APSH_Handler, "AREXX", TAG_DONE))
    {
	register struct MHObject *mho = &(mh->mh_Header);
	register struct ARexxInfo *md = mho->mho_SysData;
	struct Funcs *f;
	STRPTR cmd;

	if (f = (struct Funcs *) GetTagData (APSH_FuncEntry, NULL, tl))
	{
	    cmd = (STRPTR) f->fe_Options[1];
	}
	else
	{
	    cmd = (STRPTR) GetTagData (APSH_CmdString, NULL, tl);
	}

	/* Make sure we have a command to execute */
	if (cmd)
	{
	    /* Set the command */
	    md->ari_Tags[0].ti_Tag = APSH_CmdString;
	    md->ari_Tags[0].ti_Data = (LONG) cmd;

	    /* Send the command */
	    send_rexx_command (ai, mh, md->ari_Tags);
	}
    }
}

/****** appshell.library/__RXS ************************************************
*
*   NAME
*	RXS - Execute an ARexx string file.
*
*   SYNOPSIS
*	RXSID		Function ID
*
*   FUNCTION
*	Allows access to ARexx functions that may have otherwise been
*	rerouted by the application.  Should only be used if there are name
*	conflicts between an application function and an ARexx function.
*
*	As a string command line:
*
*	    RXS
*	    ASYNC/S, STRING/F
*
*		ASYNC	Send the command to the asynchronous port.
*
*		STRING	String file to execute.
*
*	As a TagItem attribute list:
*
*	    APSH_CmdString, <command>
*		where <command> is a command line to pass to ARexx.
*
*	This function is implemented by the ARexx message handler.
*
*   BUGS
*	ASYNC is not implemented.
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

VOID RXSFunc (struct AppInfo * ai, STRPTR args, struct TagItem * tl)
{
    struct MsgHandler *mh;

    if (mh = HandlerData (ai, APSH_Handler, "AREXX", TAG_DONE))
    {
	register struct MHObject *mho = &(mh->mh_Header);
	register struct ARexxInfo *md = mho->mho_SysData;
	struct Funcs *f;
	STRPTR cmd;

	if (f = (struct Funcs *) GetTagData (APSH_FuncEntry, NULL, tl))
	{
	    cmd = (STRPTR) f->fe_Options[1];
	}
	else
	{
	    cmd = (STRPTR) GetTagData (APSH_CmdString, NULL, tl);
	}

	/* Make sure we have a command to execute */
	if (cmd)
	{
	    /* Set the command */
	    md->ari_Tags[0].ti_Tag = APSH_ARexxString;
	    md->ari_Tags[0].ti_Data = (LONG) cmd;

	    /* Send the command */
	    send_rexx_command (ai, mh, md->ari_Tags);
	}
    }
}
