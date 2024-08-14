/* talk.c
 *
 */

#include "graffiti.h"

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

#define	AS_GOING	0
#define	AS_OPEN		1

/*****************************************************************************/

void TalkFunc (struct GlobalData * gd, STRPTR text, ULONG len)
{
    struct Window *win;

    if (OpenTalkWindow (gd))
    {
	/* Is the window open? */
	if (gd->gd_Output == NULL)
	{
	    if (gd->gd_OutputMemory.Width == 0)
	    {
		gd->gd_OutputMemory.Height = 100;

		/* Calculate the window size */
		if (win = gd->gd_ConsoleWindow)
		{
		    gd->gd_OutputMemory.Left = win->LeftEdge;
		    gd->gd_OutputMemory.Top = win->TopEdge + win->Height;
		    gd->gd_OutputMemory.Width = win->Width;
		}
		else
		{
		    gd->gd_OutputMemory.Left = gd->gd_Window->LeftEdge;
		    gd->gd_OutputMemory.Top = gd->gd_Window->TopEdge + gd->gd_Window->Height;
		    gd->gd_OutputMemory.Width = 440;
		}
	    }

	    /* Create the window spec */
	    sprintf (gd->gd_Message, "CON:%d/%d/%d/%d/%s: Listen/NOCLOSE/INACTIVE/SCREEN%s",
		     gd->gd_OutputMemory.Left, gd->gd_OutputMemory.Top,
		     gd->gd_OutputMemory.Width, gd->gd_OutputMemory.Height,
		     gd->gd_Server, gd->gd_ScreenName);

	    /* Open the window */
	    if (gd->gd_Output = Open (gd->gd_Message, MODE_NEWFILE))
	    {
		gd->gd_OutputWindow = GetConsoleWindow (gd, gd->gd_Output);
	    }
	}

	if (gd->gd_Output)
	{
	    Write (gd->gd_Output, text, len);
	}
    }
}

/*****************************************************************************/

BOOL OpenTalkWindow (struct GlobalData * gd)
{
    struct FileHandle *cfh;

    if (gd->gd_Console)
	return TRUE;

    if (gd->gd_ConsoleMemory.Width == 0)
    {
	gd->gd_ConsoleMemory.Left = gd->gd_Window->LeftEdge;
	gd->gd_ConsoleMemory.Top = gd->gd_Window->TopEdge + gd->gd_Window->Height;
	gd->gd_ConsoleMemory.Width = 440;
	gd->gd_ConsoleMemory.Height = 100;
    }

    if (gd->gd_CPort = CreateMsgPort ())
    {
	if (gd->gd_CPacket = AllocVec (sizeof (struct StandardPacket), MEMF_CLEAR))
	{
	    gd->gd_CPacket->sp_Msg.mn_Node.ln_Name = (UBYTE *) & gd->gd_CPacket->sp_Pkt;
	    gd->gd_CPacket->sp_Pkt.dp_Link = &gd->gd_CPacket->sp_Msg;

	    /* Open the console window */
	    sprintf (gd->gd_Message, "CON:%d/%d/%d/%d/%s: Talk/CLOSE/SCREEN%s",
		     gd->gd_ConsoleMemory.Left, gd->gd_ConsoleMemory.Top,
		     gd->gd_ConsoleMemory.Width, gd->gd_ConsoleMemory.Height,
		     gd->gd_Server, gd->gd_ScreenName);
	    if (gd->gd_Console = Open (gd->gd_Message, MODE_NEWFILE))
	    {
		gd->gd_ConsoleWindow = GetConsoleWindow (gd, gd->gd_Console);

		cfh = (struct FileHandle *) (gd->gd_Console << 2);

		strcpy (gd->gd_CPrompt, "> ");
		gd->gd_CPromptLen = strlen (gd->gd_CPrompt);

		gd->gd_OCIS = gd->gd_Process->pr_CIS;
		gd->gd_OCOS = gd->gd_Process->pr_COS;
		gd->gd_ConsoleTask = gd->gd_Process->pr_ConsoleTask;

		gd->gd_Process->pr_CIS = gd->gd_Console;
		gd->gd_Process->pr_COS = gd->gd_Console;
		gd->gd_Process->pr_ConsoleTask = cfh->fh_Type;

		gd->gd_CStatus = AS_OPEN;

		Write (gd->gd_Console, gd->gd_CPrompt, gd->gd_CPromptLen);

		SendReadPacket (gd, gd->gd_CPacket, gd->gd_Console, gd->gd_CPort, gd->gd_CBuff);
		return TRUE;
	    }
	    FreeVec (gd->gd_CPacket);
	    gd->gd_CPacket = NULL;
	}
	DeleteMsgPort (gd->gd_CPort);
	gd->gd_CPort = NULL;
    }

    return FALSE;
}

/*****************************************************************************/

BOOL HandleTalkEvents (struct GlobalData * gd)
{
    ULONG len;

    if (GetMsg (gd->gd_CPort))
    {
	gd->gd_PacketOut = FALSE;

	if (gd->gd_CPacket->sp_Pkt.dp_Res1 == 0)
	{
	    gd->gd_CStatus = AS_GOING;
	}
	else
	{
	    gd->gd_CBuff[gd->gd_CPacket->sp_Pkt.dp_Res1 - 1] = 0;
	    if (len = strlen (gd->gd_CBuff))
	    {
		if (gd->gd_CSBuff = AllocVec (len + 1 + strlen (gd->gd_User) + 14, MEMF_CLEAR))
		{
		    /* Build the output string */
		    sprintf (gd->gd_CSBuff, "\033[1m%s\033[0m> %s\n", gd->gd_User, gd->gd_CBuff);

		    /* Display the message locally */
		    if (gd->gd_SEntity == NULL)
			TalkFunc (gd, gd->gd_CSBuff, strlen (gd->gd_CSBuff));

		    /* Got input, now transmit it */
		    BroadCastTalk (gd, gd->gd_CSBuff);

		    FreeVec (gd->gd_CSBuff);
		}

		/* Display the prompt */
		Write (gd->gd_Console, gd->gd_CPrompt, gd->gd_CPromptLen);
	    }
	    else
	    {
		gd->gd_CStatus = AS_GOING;
	    }
	}

	if (gd->gd_CStatus >= AS_OPEN)
	{
	    SendReadPacket (gd, gd->gd_CPacket, gd->gd_Console, gd->gd_CPort, gd->gd_CBuff);
	}
	else
	{
	    CloseTalkWindow (gd);
	}
    }

    return FALSE;
}

/*****************************************************************************/

void CloseTalkWindow (struct GlobalData * gd)
{

    if (gd->gd_Output)
    {
	gd->gd_OutputMemory = *((struct IBox *) & gd->gd_OutputWindow->LeftEdge);

	Close (gd->gd_Output);
	gd->gd_Output = NULL;
	gd->gd_OutputWindow = NULL;
    }

    if (gd->gd_Console)
    {
	gd->gd_ConsoleMemory = *((struct IBox *) & gd->gd_ConsoleWindow->LeftEdge);

	gd->gd_Process->pr_CIS = gd->gd_OCIS;
	gd->gd_Process->pr_COS = gd->gd_OCOS;
	gd->gd_Process->pr_ConsoleTask = gd->gd_ConsoleTask;

	Close (gd->gd_Console);

	DeleteMsgPort (gd->gd_CPort);

	FreeVec (gd->gd_CPacket);

	gd->gd_CPort = NULL;
	gd->gd_CPacket = NULL;
	gd->gd_Console = NULL;
	gd->gd_ConsoleWindow = NULL;
    }
}

/*****************************************************************************/

VOID SendReadPacket (struct GlobalData * gd, struct StandardPacket * sp, BPTR fh, struct MsgPort * rport, UBYTE * buff)
{
    struct FileHandle *cfh;

    /* change a BPTR to a REAL pointer */
    cfh = (struct FileHandle *) (fh << 2);

    /* setup the packet for reading */
    sp->sp_Pkt.dp_Arg1 = cfh->fh_Arg1;
    sp->sp_Pkt.dp_Arg2 = (LONG) buff;
    sp->sp_Pkt.dp_Arg3 = BUFFLEN;
    sp->sp_Pkt.dp_Type = ACTION_READ;
    sp->sp_Pkt.dp_Port = rport;
    sp->sp_Msg.mn_ReplyPort = rport;

    /* now send it */
    PutMsg (cfh->fh_Type, (struct Message *) sp);
    gd->gd_PacketOut = TRUE;
}

/*****************************************************************************/

struct FindWindow
{
    struct StandardPacket fw_Pack;
    struct InfoData fw_Info;
};

/*****************************************************************************/

/* return the window for a given console */
struct Window *GetConsoleWindow (struct GlobalData * gd, BPTR cons)
{
    struct Window *win = NULL;
    struct MsgPort *mp, *tmp;
    struct FindWindow *fw;
    struct FileHandle *fh;
    struct DosPacket *dp;

    if (cons && (mp = CreateMsgPort ()))
    {
	/* convert to a C pointer */
	fh = (struct FileHandle *) (cons << 2);

	/* Set the console task in case they open a window off ours */
	tmp = (struct MsgPort *) fh->fh_Type;

	/* Allocate memory for the FindWindow structure */
	if (fw = (struct FindWindow *) AllocVec (sizeof (struct FindWindow), MEMF_CLEAR))
	{
	    /* Initialize the packet */
	    fw->fw_Pack.sp_Msg.mn_Node.ln_Name = (char *) &fw->fw_Pack.sp_Pkt;
	    fw->fw_Pack.sp_Pkt.dp_Link = (struct Message *) fw;
	    fw->fw_Pack.sp_Pkt.dp_Port = mp;
	    fw->fw_Pack.sp_Pkt.dp_Type = ACTION_DISK_INFO;
	    fw->fw_Pack.sp_Pkt.dp_Arg1 = ((LONG) & fw->fw_Info) >> 2;

	    /* Now send the packet, and wait for it to return */
	    PutMsg (tmp, (struct Message *) fw);

	    /* wait for the message to come back */
	    WaitPort (mp);

	    /*
	     * Our program has been replied to, so get the reply message, which is the packet that we sent, with the InfoData structure filled in.
	     */
	    GetMsg (mp);

	    /* Get the window pointer */
	    dp = &(fw->fw_Pack.sp_Pkt);
	    if (dp->dp_Res1 != DOSFALSE)
		win = (struct Window *) (&(fw->fw_Info))->id_VolumeNode;

	    /* free the packet */
	    FreeVec (fw);
	}

	/* delete our temporary message port */
	DeleteMsgPort (mp);
    }

    /* return the window pointer */
    return (win);
}
