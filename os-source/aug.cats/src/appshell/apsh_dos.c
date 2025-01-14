/* apsh_dos.c
 * Copyright (C) 1990 Commodore-Amiga, Inc.
 * written by David N. Junod
 *
 * DOS command shell message handling routines
 *
 * $Id: apsh_dos.c,v 1.4 1992/09/07 17:54:18 johnw Exp johnw $
 *
 * $Log: apsh_dos.c,v $
 * Revision 1.4  1992/09/07  17:54:18  johnw
 * Minor changes.
 *
 * Revision 1.1  91/12/12  14:50:24  davidj
 * Initial revision
 * 
 * Revision 1.1  90/07/02  11:20:37  davidj
 * Initial revision
 *
 *
 */

#include "appshell_internal.h"
#include <dos/dostags.h>

#define	DB(x)	;
#define	DS(x)	;

#define	APSH_SNAP_NAME	"CMDSHELL"

struct Window *GetConsoleWindow (BPTR cons);
VOID InitWinPos (struct AppInfo *ai, struct DOSInfo * md);
STRPTR PrepareWindowSpec (struct AppInfo *, struct DOSInfo * md);
void *kprintf (void *,...);

/* our handler-specific functions */
struct Funcs dos_funcs[] =
{
    {"CmdShell", CMDShellFunc, CMDShellID, "OPEN/s,CLOSE/s,TITLE/k,SNAPSHOT/s,ACTIVATE/s,FRONT/s,BACK/s,WINDOW/K", 8L, APSHF_SYSTEM,},
    {"System",   SystemFunc,   SystemID,   "ASYNC/S,PUBSCREEN/S,HOSTPORT/S,COMMAND/F", 4L, APSHF_LOCKON | APSHF_SYSTEM,},
    {NULL, NO_FUNCTION}		/* end of array */
};

ULONG ParseCmdLine (STRPTR line, STRPTR * argv);

/****** appshell.library/setup_dosA ****************************************
*
*   NAME
*	setup_dosA - Initializes the Command Shell message handler.
*
*   SYNOPSIS
*	mh = setup_dosA (ai, attrs)
*	D0               D0  D1
*
*	struct MsgHandler *mh;
*	struct AppInfo *ai;
*	struct TagItem *attrs;
*
*   FUNCTION
*	This is the low-level function used to initialize the Command
*	Shell message handler.
*
*	NOTE: This call should not be called directly by the application,
*	but should invoked by using the APSH_AddCmdShell_UI tag.
*
*	Valid tagitems to use at INITIALIZATION are:
*
*	    APSH_Status, <flags>
*	    Where flags can be:
*
*		APSHP_INACTIVE
*		When set, then the command shell will be initialized, but
*		will remain closed until opened explicitly by the user.
*
*	    APSH_CloseMsg, <ID>
*	    Text ID of the message displayed when attempting to close the
*	    command shell.  Defaults to "Waiting for macro return".
*
*	    APSH_CMDWindow, <ID>
*	    Text ID of the console window spec.  A reasonable default spec
*	    is provided.
*
*	    APSH_Prompt, <ID>
*	    Text ID of the text to used for a prompt.  "Cmd> " is the default
*	    prompt.
*
*	    APSH_CMDTitle
*	    Used to specify the text ID of title for the Command Shell.  Defaults to
*	    <basename> Command Shell.
*
*	    APSH_ParentWindow
*	    APSH_CMDParent (obsolete, don't use).
*	    Used to specify the text ID the parent window of the Command Shell.
*	    Should be the main project window of the application.  Using this tag
*	    helps the AppShell keep the Command Shell near the application when the
*	    user is using a virtual screen.  Not fully implemented yet.
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

struct MsgHandler *setup_dosA (struct AppInfo * ai, struct TagItem * tl)
{
    register struct MsgHandler *mh;
    register struct MHObject *mho;
    register struct DOSInfo *md;
    register struct StandardPacket *stdp;
    ULONG msize, hstatus, id;
    STRPTR prompt;

    /* compute the size of our memory block */
    msize = sizeof (struct MsgHandler)
      + sizeof (struct DOSInfo)
      + sizeof (struct StandardPacket)
      + (4L * sizeof (ULONG));

    /* allocate message handler memory block */
    if (mh = (struct MsgHandler *)
	AllocVec (msize, MEMF_CLEAR | MEMF_PUBLIC))
    {
	/* Set the cache */
	ai->ai_MH[MH_DOS] = mh;

	/* get a pointer on the object node */
	mho = &(mh->mh_Header);

	/* initialize the message handler node structure */
	mho->mho_Node.ln_Type = APSH_MH_HANDLER_T;
	mho->mho_Node.ln_Pri = (APSH_MH_HANDLER_P + 2);
	mho->mho_Node.ln_Name = "DOS";

	/* initialize the message handler object list */
	NewList (&(mho->mho_ObjList));

	/* set the message handler base ID */
	mho->mho_ID = APSH_DOS_ID;

	/* set up the message handler data area */
	mho->mho_SysData = md = MEMORY_FOLLOWING (mh);
	md->dmsg = stdp = MEMORY_FOLLOWING (md);
	stdp->sp_Msg.mn_Node.ln_Name = (UBYTE *) & (stdp->sp_Pkt);
	stdp->sp_Pkt.dp_Link = &(stdp->sp_Msg);

	/* set up the message handler function table */
	mh->mh_NumFuncs = 4;
	mh->mh_Func = MEMORY_FOLLOWING (stdp);
	mh->mh_Func[APSH_MH_OPEN] = open_dos;
	mh->mh_Func[APSH_MH_HANDLE] = handle_dos;
	mh->mh_Func[APSH_MH_CLOSE] = close_dos;
	mh->mh_Func[APSH_MH_SHUTDOWN] = shutdown_dos;

	/* get the activation status */
	hstatus = GetTagData (APSH_Status, NULL, tl);

	/* Get the closing message */
	if (id = GetTagData (APSH_CloseMsg, NULL, tl))
	{
	    md->closing_msg = GetText (ai, APSH_USER_ID, id, NULL);
	}
	else
	{
	    md->closing_msg =
		GetText (ai, APSH_MAIN_ID, APSH_WAITING_FOR_MACRO, NULL);
	}

	/* Get the window specification */
	if (id = GetTagData (APSH_CMDWindow, NULL, tl))
	{
	    md->winspec = GetText (ai, APSH_USER_ID, id, NULL);
	}
	else
	{
	    md->winspec = GetText (ai, APSH_MAIN_ID, APSH_CMDSHELL_WIN, NULL);
	}

	/* remember the position asked for */
	InitWinPos (ai, md);

	/* Get the command shell title */
	if (id = GetTagData (APSH_CMDTitle, NULL, tl))
	{
	    prompt = GetText (ai, APSH_USER_ID, id, NULL);
	}
	else
	{
	    prompt =
		PrepText (ai, APSH_MAIN_ID, APSH_CMDSHELL_TITLE, (int)ai->ai_BaseName);
	}
	strcpy (md->di_Title, prompt);

	/* Get the command prompt */
	if (id = GetTagData (APSH_Prompt, NULL, tl))
	{
	    prompt = GetText (ai, APSH_USER_ID, id, NULL);
	}
	else
	{
	    prompt = GetText (ai, APSH_MAIN_ID, APSH_CMDSHELL_PROMPT, NULL);
	}
	strcpy (md->aprompt, prompt);

	/* make backups of the default task information */
	Forbid();
	md->old_CIS = ai->ai_Process->pr_CIS;
	md->old_COS = ai->ai_Process->pr_COS;
	md->old_ConsoleTask = ai->ai_Process->pr_ConsoleTask;
	Permit();
	md->packet_out = FALSE;

	/* Mark shell window as being closed */
	md->dstatus = AS_CLOSED;

	/* set up a port for DOS replys */
	if (mh->mh_Port = CreatePort (NULL, NULL))
	{
	    mh->mh_SigBits = (1L << mh->mh_Port->mp_SigBit);

	    /* add our specific functions to the function table */
	    AddFuncEntries (ai, dos_funcs);

	    /* open immediately? */
	    if (hstatus & APSHP_INACTIVE)
	    {
		return (mh);
	    }
	    else if (open_dos (ai, mh, tl))
	    {
		return (mh);
	    }

	    /* clean failure path */
	    DeletePort (mh->mh_Port);
	}
	else
	{
	    ai->ai_Sec_Ret = APSH_CLDNT_CREATE_PORT;
	    ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID, ai->ai_Sec_Ret, (int)"DOS");
	}

	/* clean failure path */
	FreeVec ((APTR) mh);
    }
    else
    {
	ai->ai_Sec_Ret = APSH_NOT_ENOUGH_MEMORY;
	ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID, ai->ai_Sec_Ret, NULL);
    }

    /* If we're here, it's a failure */
    ai->ai_Pri_Ret = RETURN_FAIL;

    return (NULL);
}

BOOL open_dos (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    register struct MHObject *mho = &(mh->mh_Header);
    register struct DOSInfo *md = mho->mho_SysData;
    register struct FileHandle *handle;
    STRPTR line = NULL;

    if (!md->acons)
    {
	/* prepare the window spec for the right parameters */
	line = PrepareWindowSpec (ai, md);

	if (md->acons = Open (line, MODE_NEWFILE))
	{
	    /* get a pointer to the console window */
	    if (md->di_Win = GetConsoleWindow (md->acons))
	    {
		/* Set the window title */
		SetWindowTitles (md->di_Win, md->di_Title, (UBYTE *)(-1));
	    }

	    /* clear the command count */
	    md->di_NumCmds = 0L;

	    /* Set the standard input/output to this console window */
	    ai->ai_Process->pr_CIS = md->acons;
	    ai->ai_Process->pr_COS = md->acons;

	    /* Convert the BPTR to a C pointer */
	    handle = (struct FileHandle *) (md->acons << 2);

	    /* Set the console task in case they open a window off ours */
	    ai->ai_Process->pr_ConsoleTask = (APTR) handle->fh_Type;

	    /* Display the prompt */
	    DisplayPrompt (mh);
	    md->dstatus = AS_OPEN;

	    /* send a packet out for user input */
	    if (!md->packet_out)
	    {
		/* send a packet to DOS asking for user keyboard input */
		DB (kprintf ("send_read_packet\n"));
		send_read_packet (md->dmsg, md->acons, mh->mh_Port, md->buff);
		md->packet_out = TRUE;
	    }

	    /* set our status */
	    mho->mho_Status |= APSHF_OPEN;
	}
	else
	{
	    DB (kprintf("Couldn't open %s\n", line));
	}
    }

    /* free the temporary buffer */
    if (line)
    {
	DB (kprintf ("FreeVec 0x%lx\n", line));
	FreeVec ((APTR) line);
    }

    DB (kprintf("return 0x%lx\n", md->acons));
    return ((BOOL) ((md->acons) ? TRUE : FALSE));
}

/* handle DOS messages */
BOOL handle_dos (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    register struct MHObject *mho = &(mh->mh_Header);
    register struct DOSInfo *md = mho->mho_SysData;

    /* Set the active message handler ID */
    ai->ai_ActvMH = mho->mho_ID;

    if (GetMsg (mh->mh_Port))
    {
	/* not out any more */
	md->packet_out = FALSE;

	/* If the shell CLOSE gadget was hit, or ^\ */
	if (md->dmsg->sp_Pkt.dp_Res1 == 0)
	{
	    md->dstatus = AS_GOING;
	}
	else
	{
	    /* Mark the end of the command string */
	    md->buff[md->dmsg->sp_Pkt.dp_Res1 - 1] = 0;

	    /* See if anything was entered */
	    if (strlen (md->buff) > 0)
	    {
		STRPTR ot;

		/* Perform the function */
		PerfFunc (ai, NULL, md->buff, tl);

#if 0
		/* If it was an unknown command, pass it to ARexx */
		if (ai->ai_Sec_Ret == APSH_UNKNOWN_COMMAND)
		{
		    struct MsgHandler *mh;

		    /* See if we are using ARexx */
		    if (mh = HandlerData (ai, APSH_Handler, "AREXX", TAG_DONE))
		    {
			/* Clear the error return values */
			ai->ai_Pri_Ret = NULL;
			ai->ai_Sec_Ret = NULL;
			ai->ai_TextRtn = NULL;

			/* Pass it to ARexx */
			HandlerFunc (ai,
				     APSH_Handler, "AREXX",
				     APSH_Command, AH_SENDCMD,
				     APSH_CmdString, md->buff,
				     TAG_DONE);
		    }
		}
#endif

		/* Do we have a text return? */
		if (ot = ai->ai_TextRtn)
		{
		    /* Display the text on the console */
		    Write (md->acons, (APTR)ot, (LONG) strlen (ot));
		    Write (md->acons, (APTR)"\n", 1L);

		    /* Backup the error text */
		    strcpy (ai->ai_LastText, ot);
		}
#if 0
		/* Do we have error values? */
		else if (ai->ai_Pri_Ret)
		{
		    ot = ai->ai_WorkText;
		    sprintf (ai->ai_WorkText, "Primary: %ld, Secondary: %d\n",
			ai->ai_Pri_Ret, ai->ai_Sec_Ret);
		    Write (md->acons, (APTR)ot, (LONG) strlen (ot));
		}
#endif

		/* Save the last error */
		ai->ai_LastError = ai->ai_Sec_Ret;

		/* Clear the error information now that it's been handled */
		ai->ai_TextRtn = NULL;
		ai->ai_Pri_Ret = NULL;
		ai->ai_Sec_Ret = NULL;

		/* Display the prompt */
		DisplayPrompt (mh);
	    }
	    else
	    {
		/* they entered a blank line, let's end the shell */
		md->dstatus = AS_GOING;
	    }
	}

	/* send out a packet for user input */
	if (!md->packet_out && md->dstatus >= AS_OPEN)
	{
	    /* send a packet to DOS asking for user keyboard input */
	    send_read_packet (md->dmsg, md->acons, mh->mh_Port, md->buff);
	    md->packet_out = TRUE;
	}

	/* Half-hearted attempt at closing the shell */
	close_dos (ai, mh, tl);
    }

    /* Clear the active message */
    ai->ai_ActvMsg = NULL;
    ai->ai_ActvMH = NULL;

    return (TRUE);
}

BOOL close_dos (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    register struct MHObject *mho = &(mh->mh_Header);
    register struct DOSInfo *md = mho->mho_SysData;

    if (md->dstatus == AS_GOING)
    {
	/* User has indicated that he wants to close the command shell. */
	md->dstatus = AS_CLOSING;
	Write (md->acons, (APTR) md->closing_msg, (LONG) strlen (md->closing_msg));
    }

    if (md->dstatus == AS_CLOSING && !ai->ai_NumCmds)
    {
	if (md->acons)
	{
	    /* safely restore the default information */
	    Forbid();
	    ai->ai_Process->pr_CIS = md->old_CIS;
	    ai->ai_Process->pr_COS = md->old_COS;
	    ai->ai_Process->pr_ConsoleTask = md->old_ConsoleTask;
	    Permit();

	    /* remember the window position */
	    if (md->di_Win)
	    {
		md->di_NW.LeftEdge = md->di_Win->LeftEdge;
		md->di_NW.TopEdge = md->di_Win->TopEdge;
		md->di_NW.Width = md->di_Win->Width;
		md->di_NW.Height = md->di_Win->Height;
	    }

	    /* close the console window */
	    Close (md->acons);
	}

	mho->mho_Status &= ~APSHF_OPEN;

	md->dstatus = AS_CLOSED;
	md->acons = NULL;
	md->di_Win = NULL;
    }
    return ((BOOL) ((md->acons) ? TRUE : FALSE));
}

BOOL shutdown_dos (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    register struct MHObject *mho = &(mh->mh_Header);
    register struct DOSInfo *md = mho->mho_SysData;

    if (mh)
    {
	/* Clear the cache */
	ai->ai_MH[MH_DOS] = NULL;

	if (md)
	{
	    if (md->acons)
		md->dstatus = AS_GOING;
	    close_dos (ai, mh, tl);
	}

	/* delete the message port */
	if (mh->mh_Port)
	    RemoveMsgPort (mh->mh_Port);

	/* remove the message handler from the list */
	Remove ((struct Node *) mh);

	/* free our data */
	FreeVec ((APTR) mh);
    }

    return (TRUE);
}

VOID DisplayPrompt (struct MsgHandler * mh)
{
    register struct MHObject *mho = &(mh->mh_Header);
    register struct DOSInfo *md = mho->mho_SysData;

    Write (md->acons, (APTR) md->aprompt, (LONG) strlen (md->aprompt));
}

VOID send_read_packet (struct StandardPacket * dos_message,
		        BPTR console_fh,
		        struct MsgPort * dos_reply_port,
		        UBYTE * buff)
{
    struct FileHandle *file_handle;

    /* change a BPTR to a REAL pointer */
    file_handle = (struct FileHandle *) (console_fh << 2);

    /* setup the packet for reading */
    dos_message->sp_Pkt.dp_Arg1 = file_handle->fh_Arg1;
    dos_message->sp_Pkt.dp_Arg2 = (LONG) buff;
    dos_message->sp_Pkt.dp_Arg3 = BUFFLEN;
    dos_message->sp_Pkt.dp_Type = ACTION_READ;
    dos_message->sp_Pkt.dp_Port = dos_reply_port;
    dos_message->sp_Msg.mn_ReplyPort = dos_reply_port;

    /* now send it */
    PutMsg (file_handle->fh_Type, (struct Message *) dos_message);
}

/* return the window for a given console */
struct Window *GetConsoleWindow (BPTR cons)
{
    struct MsgPort *mp = NULL, *tmp;
    struct FileHandle *fh;
    struct FindWindow *fwin = NULL;
    struct Window *win = NULL;
    struct DosPacket *dp;

    if (cons && (mp = CreatePort (NULL, NULL)))
    {
	/* convert to a C pointer */
	fh = (struct FileHandle *) (cons << 2);

	/* Set the console task in case they open a window off ours */
	tmp = (struct MsgPort *) fh->fh_Type;

	/* Allocate memory for the FindWindow structure */
	if (fwin = (struct FindWindow *)
	    AllocVec (sizeof (struct FindWindow), MEMF_PUBLIC | MEMF_CLEAR))
	{
	    /* Initialize the packet */
	    fwin->FW_Pack.sp_Msg.mn_Node.ln_Name = (char *) &fwin->FW_Pack.sp_Pkt;
	    fwin->FW_Pack.sp_Pkt.dp_Link = (struct Message *) fwin;
	    fwin->FW_Pack.sp_Pkt.dp_Port = mp;
	    fwin->FW_Pack.sp_Pkt.dp_Type = ACTION_DISK_INFO;
	    fwin->FW_Pack.sp_Pkt.dp_Arg1 = ((LONG) & fwin->FW_Info) >> 2;

	    /* Now send the packet, and wait for it to return */
	    PutMsg (tmp, (struct Message *) fwin);

	    /* wait for the message to come back */
	    WaitPort (mp);

	    /*
	     * Our program has been replied to, so get the reply message, which
	     * is the packet that we sent, with the InfoData structure filled
	     * in.
	     */
	    GetMsg (mp);

	    dp = &(fwin->FW_Pack.sp_Pkt);
	    if (dp->dp_Res1 != DOSFALSE)
	    {
		win = (struct Window *) (&(fwin->FW_Info))->id_VolumeNode;
	    }

	    /* free the packet */
	    FreeVec (fwin);
	}
	else
	{
	    DB (kprintf("Couldn't allocate memory\n"));
	}

	/* delete our temporary message port */
	RemoveMsgPort (mp);
    }
    else
    {
	DB (kprintf("Couldn't create port, or cons (0x%lx) == NULL\n", cons));
    }

    /* return the window pointer */
    DB (kprintf("GetConsoleWindow returns 0x%lx\n", win));
    return (win);
}

#define QUOTE  34

/* remember ',' is the decimal point in some countries */
#define isbreak(c)  ((c=='/') || (c==':'))

ULONG ParseCmdLine (STRPTR line, STRPTR * argv)
{
    register int i;
    register STRPTR *pargv;
    ULONG argc;

    /* clear the work areas */
    argc = 0L;
    for (i = 0; i < MAXARG; i++)
	argv[i] = NULL;

    /* parse the arguments */
    while (argc < MAXARG)
    {
	while (isbreak (*line))
	    line++;
	if (*line == '\0')
	    break;
	pargv = &argv[(UWORD) (argc++)];
	if (*line == QUOTE)
	{
	    *pargv = ++line;	/* ptr inside quoted string */
	    while ((*line != '\0') && (*line != QUOTE))
		line++;
	    if (*line == '\0')
	    {
		return (argc);
	    }
	    else
	    {
		*line++ = '\0';	/* terminate arg */
	    }
	}
	else
	    /* non-quoted arg */
	{
	    *pargv = line;
	    while ((*line != '\0') && (!isbreak (*line)))
		line++;
	    if (*line == '\0')
		break;
	    else
		*line++ = '\0';	/* terminate arg */
	}
    }				/* while */
    return (argc);
}

VOID InitWinPos (struct AppInfo *ai, struct DOSInfo * md)
{
    STRPTR spec, line, argv[MAXARG];
    ULONG argc, ln;

    /* make sure that we have a enough to continue */
    if (md && (spec = md->winspec))
    {
	/* get the length of the window spec */
	ln = strlen (spec) + 2L;

	/* allocate memory, so that we can copy the window spec */
	if (line = (STRPTR) AllocVec (ln, MEMF_CLEAR))
	{
	    struct NewWindow *nw = &(md->di_NW);

	    /* copy the original into the clone */
	    strcpy (line, spec);

	    /* parse the command line */
	    argc = ParseCmdLine (line, argv);

	    /* provide some defaults */
	    nw->LeftEdge = 0;
	    nw->TopEdge = 150;
	    nw->Width = 640;
	    nw->Height = 50;

	    /* establish the initial settings */
	    if (argc >= 4L)
	    {
		nw->LeftEdge = (SHORT) atoi (argv[1]);
		nw->TopEdge = (SHORT) atoi (argv[2]);
		nw->Width = (SHORT) atoi (argv[3]);
		nw->Height = (SHORT) atoi (argv[4]);
	    }

	    /* Fake some of the need fields */
	    nw->MaxWidth = ~0;
	    nw->MaxHeight = ~0;
	    if (nw->Screen = ai->ai_Screen)
	    {
		nw->MaxWidth = ai->ai_Screen->Width;
		nw->MaxHeight = ai->ai_Screen->Height;
	    }
	    nw->Flags |= WINDOWSIZING;

	    /* Load the snapshot image */
	    nw = LoadSnapShot (nw, ai->ai_ConfigDir, APSH_SNAP_NAME);

	    /* free the temporary buffer */
	    FreeVec ((APTR) line);
	}
    }
}

STRPTR PrepareWindowSpec (struct AppInfo * ai, struct DOSInfo * md)
{
    STRPTR spec, line = NULL, dest = NULL, argv[MAXARG];
    UBYTE numb[10];
    ULONG argc, ln, i;

    /* make sure that we have a enough to continue */
    if (md && (spec = md->winspec))
    {
	/* get the length of the window spec */
	DB (kprintf("spec %s, screen %s\n", spec, ai->ai_ScreenName));
	ln = strlen (spec) + 90L;
	if (ai->ai_ScreenName)
	{
	    ln += strlen(ai->ai_ScreenName);
	}

	/* allocate memory, so that we can copy the window spec */
	DB (kprintf("allocate work space\n"));
	if ((line = (STRPTR) AllocVec (ln, MEMF_CLEAR)) &&
	    (dest = (STRPTR) AllocVec (ln, MEMF_CLEAR)))
	{
	    struct NewWindow *nw = &(md->di_NW);
	    BOOL add_screen = FALSE;

	    /* copy the original into the clone */
	    DB (kprintf("strcpy 0x%lx, %s\n", line, spec));
	    strcpy (line, spec);

	    /* parse the command line */
	    argc = ParseCmdLine (line, argv);

	    /* Do we have a screen name? */
	    if (ai->ai_ScreenName)
	    {
		add_screen = TRUE;
	    }

	    for (i = 0L; i < argc; i++)
	    {
		switch (i)
		{
		    case 0L:
			strcpy (dest, argv[i]);
			strcat (dest, ":");
			break;
		    case 1L:
			sprintf (numb, "%ld", (LONG) nw->LeftEdge);
			strcat (dest, numb);
			break;
		    case 2L:
			sprintf (numb, "/%ld", (LONG) nw->TopEdge);
			strcat (dest, numb);
			break;
		    case 3L:
			sprintf (numb, "/%ld", (LONG) nw->Width);
			strcat (dest, numb);
			break;
		    case 4L:
			sprintf (numb, "/%ld", (LONG) nw->Height);
			strcat (dest, numb);
			break;
		    default:
			if ((strnicmp(argv[i], "SCREEN", 6))==0)
			{
			    if (add_screen)
			    {
				DB (kprintf ("screen [%s]\n", argv[i]));
				strcat (dest, "/SCREEN");

				/* Add the screen name */
				strcat (dest, ai->ai_ScreenName);

				/* No more need for a screen */
				add_screen = FALSE;
			    }
			}
			else
			{
			    DB (kprintf ("[%s]\n", argv[i]));
			    strcat (dest, "/");
			    strcat (dest, argv[i]);
			}
			break;
		}		/* end of switch */
	    }			/* end of for argc */

	    /* Add the screen name to the spec */
	    if (add_screen)
	    {
		strcat (dest, "/SCREEN");
		strcat (dest, ai->ai_ScreenName);
	    }

	}			/* end of if allocate memory */

	/* free the temporary string */
	if (line)
	{
	    FreeVec (line);
	}
    }				/* end of make sure there are parameters */

    /* return the built string */
    return (dest);
}

/****** appshell.library/__CMDSHELL ******************************************
*
*   NAME
*	CMDSHELL - Open/Close the application command shell.
*
*   SYNOPSIS
*	CMDShellID	Function ID
*
*   FUNCTION
*	Opens a console window whereby the user can interact directly with
*	the application at a command level.  Allows the user quick access
*	to functions or macros that they may use so infrequently that they
*	don't wish to bind them to a key, menu or button.
*
*	As a string command line:
*
*	    CMDSHELL
*	    OPEN/s,CLOSE/s,TITLE/k,SNAPSHOT/s,ACTIVATE/s,FRONT/s,BACK/s,WINDOW/k
*
*		OPEN		Open the command shell.
*		CLOSE		Close the command shell.
*		TITLE		Set the title of the command shell window.
*		SNAPSHOT	Save the window size to disk.
*		ACTIVATE	Activate the command shell.
*		FRONT		Send the command shell to the front.
*		BACK		Send the command shell to the back.
*		WINDOW		Name of window to open relative to. Defaults
*				to "Main".
*
*	As a TagItem attribute list:
*
*	    APSH_Command, <command>
*		where <command> is a valid command, defaults to MH_OPEN.
*
*		    MH_OPEN	Open the command shell.
*		    MH_CLOSE	Close the command shell.
*
*	This function is implemented by the DOS message handler.
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

VOID CMDShellFunc (struct AppInfo * ai, STRPTR args, struct TagItem * tl)
{
    struct MsgHandler *mh;

    DS (kprintf("CMDShellFunc enter 0x%lx 0x%lx 0x%lx\n", ai, args, tl));
    if (mh = HandlerData (ai, APSH_Handler, "DOS", TAG_DONE))
    {
	struct MHObject *mho = &(mh->mh_Header);
	struct DOSInfo *md = mho->mho_SysData;
	ULONG cmd = NULL;
	struct Funcs *fe = NULL;

	if (tl)
	{
	    if (fe = (struct Funcs *) GetTagData (APSH_FuncEntry, NULL, tl))
	    {
		if (fe->fe_Options[0])
		{
		    cmd = APSH_MH_OPEN;
		}
		else if (fe->fe_Options[1])
		{
		    cmd = APSH_MH_CLOSE;
		}
	    }
	    else
	    {
		cmd = GetTagData (APSH_Command, APSH_MH_OPEN, tl);
	    }
	}

	/* See if we're supposed to open the command shell */
	if (cmd == APSH_MH_OPEN)
	{
	    if (md->di_Win)
	    {
		WindowToFront (md->di_Win);
		ActivateWindow (md->di_Win);
	    }
	    else
	    {
		open_dos (ai, mh, tl);
	    }
	}

	/* Make sure the window is open */
	if (md->di_Win && fe)
	{
	    /* Set the window title */
	    if (fe->fe_Options[2])
	    {
		/* Copy the title */
		strcpy (md->di_Title, (UBYTE *)fe->fe_Options[2]);

		/* Set the window title */
		SetWindowTitles (md->di_Win, md->di_Title, (UBYTE *)(-1));
	    }

	    /* Save the window snapshot */
	    if (fe->fe_Options[3])
		SaveSnapShot (md->di_Win, ai->ai_ConfigDir, APSH_SNAP_NAME);

	    /* Activate the window */
	    if (fe->fe_Options[4])
		ActivateWindow (md->di_Win);

	    /* Send the window to front */
	    if (fe->fe_Options[5])
		WindowToFront (md->di_Win);

	    /* Send the window to back */
	    if (fe->fe_Options[6])
		WindowToBack (md->di_Win);
	}

	/* See if we're supposed to close the command shell */
	if (cmd == APSH_MH_CLOSE)
	{
	    if (md->acons)
		md->dstatus = AS_GOING;
	    close_dos (ai, mh, tl);
	}
    }
}

/****** appshell.library/__SYSTEM ******************************************
*
*   NAME
*	SYSTEM - Execute a system command or application.
*
*   SYNOPSIS
*	SystemID	Function ID
*
*   FUNCTION
*	This function uses the AmigaDOS command Run to execute a command.
*
*	As a string command line:
*
*	    SYSTEM
*	    ASYNC/S,PUBSCREEN/S,HOSTPORT/S,COMMAND/F
*
*		ASYNC		Run the command asynchronously.
*
*		PUBSCREEN	Pass the name of the applications public
*				screen with the PUBSCREEN keyword.
*
*		HOSTPORT	Pass the name of the applications ARexx
*				port with the HOSTPORT keyword.
*
*	This function is implemented by the DOS message handler.
*
*********************************************************************
*
* Created:  18-Mar-91, David N. Junod
*
*/

VOID SystemFunc (struct AppInfo * ai, STRPTR args, struct TagItem * tl)
{
    struct MsgHandler *mh;
    struct TagItem tg[5];
    struct Funcs *f;
    BPTR fs = NULL;
    STRPTR cmd;
    BOOL async;

    DS (kprintf("SystemFunc enter 0x%lx 0x%lx 0x%lx\n", ai, args, tl));
    if (mh = HandlerData (ai, APSH_Handler, "DOS", TAG_DONE))
    {
        /* See if we have a parsed command */
	DS (kprintf("Find APSH_FuncEntry 0x%lx\n", tl));
        if (f = (struct Funcs *) GetTagData (APSH_FuncEntry, NULL, tl))
        {

	    /* See if we should run asynchronously. */
            if (!(async = (BOOL) f->fe_Options[0]))
	    {
		fs = Output(); /* md->acons; */
		DS (kprintf("Async 0x%lx\n", fs));
	    }

	    /* See if they gave us a command string */
            if (cmd = (STRPTR) f->fe_Options[3])
            {
		/* Copy the command into our work buffer */
		DS (kprintf("%s\n", cmd));
		strcpy (ai->ai_WorkText, cmd);

		/* See if they need our screen name */
		if (f->fe_Options[1])
		{
		    DS (kprintf("PubScreen %s\n", ai->ai_ScreenName));
		    strcat (ai->ai_WorkText, " PUBSCREEN ");
		    strcat (ai->ai_WorkText, ai->ai_ScreenName);
		}

		/* See if they need our ARexx port name */
		if (f->fe_Options[2])
		{
		    struct MsgHandler *mh = ai->ai_MH[MH_AREXX];

		    if (mh && mh->mh_PortName)
		    {
			DS (kprintf("ARexx %s\n", mh->mh_PortName));
			strcat (ai->ai_WorkText, " HOSTPORT ");
			strcat (ai->ai_WorkText, mh->mh_PortName);
		    }
		}

		/* Build the tags. */
                tg[0].ti_Tag = SYS_Asynch;
                tg[0].ti_Data = (ULONG) async;
                tg[1].ti_Tag = SYS_Input;
                tg[1].ti_Data = NULL;
                tg[2].ti_Tag = SYS_Output;
                tg[2].ti_Data = fs;
                tg[3].ti_Tag = TAG_DONE;

		/* Call the command */
		DS (kprintf("System %s\n", ai->ai_WorkText));
                if ((ai->ai_Pri_Ret = SystemTagList (ai->ai_WorkText, tg)) == -1)
                {
                    ai->ai_Pri_Ret = RETURN_FAIL;
                    ai->ai_Sec_Ret = APSH_UNKNOWN_COMMAND;
                }
            }
	    else
	    {
		DS (kprintf("No command string\n"));
	    }
        }
	else
	{
	    DS (kprintf("No FuncEntry\n"));
	}
    }
    else
    {
	DS (kprintf("Couldn't get dos handler\n"));
    }

    return;
}
