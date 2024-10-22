/* apsh_dispatch.c
 * Copyright (C) 1990 Commodore-Amiga, Inc.
 * written by David N. Junod
 *
 * Function dispatcher
 *
 * $Id: apsh_dispatch.c,v 1.4 1992/09/07 17:54:03 johnw Exp johnw $
 *
 * $Log: apsh_dispatch.c,v $
 * Revision 1.4  1992/09/07  17:54:03  johnw
 * MInor changes.
 *
 * Revision 1.1  91/12/12  14:50:11  davidj
 * Initial revision
 *
 * Revision 1.1  90/07/02  11:19:38  davidj
 * Initial revision
 *
 *
 */

#include "appshell_internal.h"

void kprintf (void *,...);

#define	DB(x)	;
#define	DN(x)	;
#define	DI(x)	;
#define	DS(x)	;
#define	DE(x)	;
#define	DC(x)	;
#define	DH(x)	;

/* process messages */
struct AppInfo *ProcessMsgs (struct AppInfo * ai)
{
    struct MsgHandler *mh;
    struct MHObject *mho;
    struct List *list;
    struct Node *node;
    ULONG sig_rcvd;

    /* establish list pointer */
    list = &(ai->ai_MsgList);

    /* Process messages until user signals that they are done. */
    /* loop until done and no messages outstanding */
    while (!ai->ai_Done || ai->ai_NumCmds)
    {
	/* wait for a message to come in */
	DB (kprintf ("Wait 0x%lx\n", ai->ai_SigBits));
	sig_rcvd = Wait (ai->ai_SigBits);

	/* do special signal processing */
	if (sig_rcvd & SIGBREAKF_CTRL_C)
	{
	    ULONG FuncID;

	    /* get the function ID for the ^C signal */
	    FuncID = ai->ai_Funcs[APSH_SIGC];

	    /* make sure we dispatch to something... */
	    if (FuncID == NO_FUNCTION)
	    {
		FuncID = QuitID;
	    }

	    /* dispatch the function */
	    PerfFunc (ai, FuncID, NULL, NULL);
	}

	if (sig_rcvd & SIGBREAKF_CTRL_D)
	{
	    PerfFunc (ai, ai->ai_Funcs[APSH_SIGD], NULL, NULL);
	}

	if (sig_rcvd & SIGBREAKF_CTRL_E)
	{
	    PerfFunc (ai, ai->ai_Funcs[APSH_SIGE], NULL, NULL);
	}

	if (sig_rcvd & SIGBREAKF_CTRL_F)
	{
	    PerfFunc (ai, ai->ai_Funcs[APSH_SIGF], NULL, NULL);
	}


	/* display error/text message */
	if (ai->ai_TextRtn)
	{
	    NotifyUser (ai, NULL, NULL);

	    /* Save the last error */
	    ai->ai_LastError = ai->ai_Sec_Ret;
	    strcpy (ai->ai_LastText, ai->ai_TextRtn);

	    /* Clear the error information */
	    ai->ai_TextRtn = NULL;
	    ai->ai_Pri_Ret = NULL;
	    ai->ai_Sec_Ret = NULL;
	}

	/* process signals */
	if (list->lh_TailPred != (struct Node *) list)
	{
	    node = list->lh_Head;
	    while (node->ln_Succ)
	    {
		mh = (struct MsgHandler *) node;
		mho = &(mh->mh_Header);

		/* is this the handler that the message is for? */
		if (node->ln_Type == APSH_MH_HANDLER_T &&
		    (mh->mh_SigBits & sig_rcvd))
		{
		    /* call the message handler */
		    DB (kprintf("Handle [%s]\n", mh->mh_Header.mho_Node.ln_Name));
		    (*mh->mh_Func[APSH_MH_HANDLE]) (ai, mh, TAG_DONE);

		    /* display error/text message */
		    if (ai->ai_TextRtn)
		    {
			/* show the message to the user */
			NotifyUser (ai, NULL, NULL);

			/* save the last error */
			ai->ai_LastError = ai->ai_Sec_Ret;
			strcpy (ai->ai_LastText, ai->ai_TextRtn);

			/* clear the error information */
			ai->ai_TextRtn = NULL;
			ai->ai_Pri_Ret = NULL;
			ai->ai_Sec_Ret = NULL;
		    }
		}

		/* get the next node */
		node = node->ln_Succ;
	    }
	}
    }

    return (ai);
}

/****** appshell.library/PerfFunc ******************************************
*
*   NAME
*	PerfFunc - The entry point to all commands in the function table.
*
*   SYNOPSIS
*	success = PerfFunc (ai, fid, cmdline, attrs)
*	D0		    D0  D1   A0       A1
*
*	BOOL success;
*	struct AppInfo *ai;
*	ULONG fid;
*	STRPTR cmdline;
*	struct TagItem *attrs;
*
*   FUNCTION
*	This is the main and only entry point for all commands in the
*	function table.  In order to respect state, such as whether the
*	function is enabled or disabled, function table commands should
*	never be called directly.
*
*	If the function isn't in the function table and the ARexx message
*	handler has been initialized, then the command is passed to ARexx.
*
*   EXAMPLE
*
*	\* how to call a function using its ID *\
*	PerfFunc (ai, QuitID, NULL, NULL);
*
*	\* how to call a function using its name *\
*	PerfFunc (ai, NULL, "QUIT", NULL);
*
*
*   INPUTS
*	ai	- Pointer to the AppInfo structure
*	fid	- ID of function to perform.
*	cmdline	- Pointer to the text command line to use to trigger the
*		  command.
*	attrs	- Pointer to the TagItem array to use to triggered the
*		  command.
*
*   RESULT
*	success	- Returns TRUE if the function was performed, FALSE if
*		  it didn't.
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

ULONG callHookPkt (struct Hook *h, VOID *obj, VOID *msg);

BOOL PerfFunc (struct AppInfo * ai, ULONG fid, STRPTR cmd, struct TagItem * tl)
{

    VOID (*func) (struct AppInfo *, STRPTR, struct TagItem *) = NULL;
    STRPTR clone = NULL, anchor = NULL;
    struct AppFunction *msg = &(ai->ai_AF);
    struct Hook *hook = &(ai->ai_Hook);
    struct FuncEntry *curfe = NULL;
    STRPTR argv[MAXARG] = {NULL};
    BOOL retval = FALSE;
    ULONG status = NULL;
    ULONG argc = 0L;

    DH (kprintf ("ai_Hook 0x%lx, hook 0x%lx\n", &(ai->ai_Hook), hook));

    /* Did they call us with a command string? */
    if (cmd)
    {
	/* Use it */
	anchor = cmd;
    }
    /* Did they call us with a function ID? */
    else if (fid)
    {
	/* Look up the function ID */
	if (curfe = GetFuncEntry (ai, NULL, fid))
	{
	    /* Use it */
	    anchor = curfe->fe_Name;
	}
	else
	{
	    DE (kprintf ("Invalid function ID\n"));
	}
    }

    /* Do we have a command to parse */
    DC (kprintf("PerfFunc [%s]\n", anchor));
    if (anchor)
    {
	/* we're learning, let's save the command in the macro file */
	if (ai->ai_Flags & APSHF_LEARN)
	{
	    LONG actual, numchars;

	    /* How many characters are we going to write? */
	    numchars = (LONG) strlen (anchor);

	    /* Write the command to the macro file */
	    actual = Write (ai->ai_MacroFH, (APTR) anchor, numchars);

	    /* Were we able to write the whole command? */
	    if (actual != numchars)
	    {
		/* error checking */
		NotifyUser (ai, "Couldn't write to macro file", NULL);
	    }
	    else
	    {
		/* write a line feed */
		Write (ai->ai_MacroFH, (APTR) "\n", 1L);
	    }
	}

	/* Non-destructively parse the command line */
	clone = BuildParseLine (anchor, &argc, argv);

	/* Compare to the function in the cache */
	if (ai->ai_CurFunc && (QStrCmpI (argv[0], ai->ai_CurFunc->fe_Name)))
	{
	    /* Use the cached function */
	    curfe = ai->ai_CurFunc;
	}
	else
	{
	    /* New function, look it up */
	    curfe = GetFuncEntry (ai, argv[0], NULL);
	}

	/* Do we have a function to execute? */
	if (curfe)
	{
	    /* Get the function information */
	    func = curfe->fe_Func;

	    status = curfe->fe_Flags;
	    curfe->fe_HitCnt++;
	    ai->ai_CurFunc = curfe;

	    /* Get a pointer to the command line */
	    if (curfe->fe_Params)
	    {
		anchor = curfe->fe_Params;
	    }
	}

	/* Is the function disabled? */
	if (status & APSHF_DISABLED)
	{
	    /* Preset the error return to warn */
	    ai->ai_Pri_Ret = RETURN_WARN;
	    ai->ai_Sec_Ret = APSH_DISABLED;
	    ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID, APSH_DISABLED,
				       (int) anchor);
	}
	/* Is the function private? */
	else if (cmd && (status & APSHF_PRIVATE))
	{
	    /* preset the error return to warn */
	    ai->ai_Pri_Ret = RETURN_WARN;
	    ai->ai_Sec_Ret = APSH_DISABLED;
	    ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID, APSH_DISABLED,
				       (int) anchor);
	}
	/* Should be able to execute the function */
	else
	{
	    /* Do we have a function to execute */
	    if (func != NO_FUNCTION)
	    {
		VOID (* __asm hEntry)(register __a0 struct Hook *, register __a2 struct AppInfo *, register __a1 struct AppFunction *);

		/* Initialize the hook */
		DH (kprintf ("Curfe 0x%lx, Hook 0x%lx\n", curfe->fe_Entry, hook->h_Entry));
		hEntry = (curfe->fe_Entry) ? curfe->fe_Entry : hook->h_Entry;
		hook->h_SubEntry = func;

		/* Set up the message */
		msg->MethodID = AI_FUNCTION;
		msg->af_CmdLine = cmd;
		msg->af_Kind = ai->ai_ActvMH;
		msg->af_Msg = ai->ai_ActvMsg;
		msg->af_FE = NULL;

		/* Tag list, but no command string */
		if (!(cmd) && tl)
		{
		    /* execute the function */
#if 1
		    DH (kprintf ("1) Entry 0x%lx : hook 0x%lx, ai 0x%lx, msg 0x%lx\n", hEntry, hook, ai, msg));
		    msg->af_Attrs = tl;
		    DH (kprintf ("-- hEntry:\t0x%lx\n", hEntry));
		    DH (kprintf ("-- h_SubEntry:\t0x%lx\n", hook->h_SubEntry));
		    (*hEntry)(hook, ai, msg);
#else
		    callHookPkt (hook, ai, msg);
		    dispatchRegs (func, ai, anchor, tl);
		    (*(func)) (ai, anchor, tl);
#endif

		    /* successful command completion */
		    retval = TRUE;
		}
		/* We have a command that needs ReadArgs */
		else if (curfe->fe_Options && anchor)
		{
		    struct RDArgs *ra = &(ai->ai_CmdRDA);
		    struct TagItem ins[2];
		    UWORD i;

		    /* Clear out anything left over */
		    for (i = 0; i < ((UWORD) curfe->fe_NumOpts); i++)
		    {
			curfe->fe_Options[i] = NULL;
		    }

		    /* Initialize the ReadArgs structures */
		    if (argc > 1)
		    {
			i = (UWORD) (argv[1] - clone);
			sprintf (ai->ai_TempText, "%s\n", &anchor[i]);
		    }
		    else
		    {
			i = 0;
			strcpy (ai->ai_TempText, "\n");
		    }
		    DE (kprintf ("Length = %ld Argc = %ld\n", (LONG) i, argc));
		    ra->RDA_Source.CS_Buffer = ai->ai_TempText;
		    ra->RDA_Source.CS_Length = strlen (ai->ai_TempText);
		    ra->RDA_Source.CS_CurChr = 0;

		    /* Run it through ReadArgs */
		    if (curfe->fe_ArgsPtr =
			ReadArgs (curfe->fe_Template, curfe->fe_Options, ra))
		    {
			msg->af_FE = &(curfe->fe_Name);
			ins[0].ti_Tag = APSH_FuncEntry;
			ins[0].ti_Data = (ULONG) & curfe->fe_Name;
			ins[1].ti_Tag = TAG_MORE;
			ins[1].ti_Data = (ULONG) tl;

			/* Execute the function */
#if 1
			DH (kprintf ("2) Entry 0x%lx : hook 0x%lx, ai 0x%lx, msg 0x%lx\n", hEntry, hook, ai, msg));
			msg->af_Attrs = ins;
			(*hEntry)(hook, ai, msg);
#else
			callHookPkt (hook, ai, msg);
			dispatchRegs (func, ai, anchor, ins);
			(*(func)) (ai, anchor, ins);
#endif
			/* Free the ReadArgs work area */
			FreeArgs (curfe->fe_ArgsPtr);

			/* Successful command completion */
			retval = TRUE;
		    }
		    /* ReadArgs error */
		    else
		    {
			/* There was a syntax error */
			DE (kprintf ("ReadArgs error (%ld) [%s] [%s] %ld\n",
				     IoErr (), &anchor[i], curfe->fe_Template, curfe->fe_Options));
			ai->ai_Pri_Ret = RETURN_FAIL;
			ai->ai_Sec_Ret = APSH_SYNTAX_ERROR;
			ai->ai_TextRtn =
			  PrepText (ai, APSH_MAIN_ID, APSH_SYNTAX_ERROR,
				    (int) curfe->fe_Template);
		    }
		}
		/* We have a command string */
		else
		{
		    /* Execute the function */
#if 1
		    DH (kprintf ("3) Entry 0x%lx : hook 0x%lx, ai 0x%lx, msg 0x%lx\n", hEntry, hook, ai, msg));
		    msg->af_Attrs = tl;
		    (*hEntry)(hook, ai, msg);
#else
		    callHookPkt (hook, ai, msg);
		    dispatchRegs (func, ai, anchor, tl);
		    (*(func)) (ai, anchor, tl);
#endif
		    /* Successful command completion */
		    retval = TRUE;
		}
	    }
	    /* No function ID, so pass it through ARexx */
	    else
	    {
#if 1
		/* Set the error return to warn */
		ai->ai_Pri_Ret = RETURN_WARN;
		ai->ai_Sec_Ret = APSH_UNKNOWN_COMMAND;
		ai->ai_TextRtn =
		  PrepText (ai, APSH_MAIN_ID, APSH_UNKNOWN_COMMAND, NULL);
#else
		struct MsgHandler *mh;

		/* see if we are using ARexx */
		if (mh = HandlerData (ai, APSH_Handler, "AREXX", TAG_DONE))
		{
		    /* wasn't an internal function, so let's pass it to ARexx */
		    retval = HandlerFunc (ai,
					  APSH_Handler, "AREXX",
					  APSH_Command, AH_SENDCMD,
					  APSH_CmdString, anchor,
					  TAG_DONE);
		}
#endif
	    }
	}

	/* Deallocate the parsed line */
	FreeParseLine (clone);
    }

    DC (kprintf("PerfFunc exit %ld\n", retval));

    return (retval);
}
