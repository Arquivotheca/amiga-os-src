/* apsh_hyper.c
 * Copyright (C) 1990 Commodore-Amiga, Inc.
 * written by David N. Junod
 *
 */

#include "appshell_internal.h"

#define	DB(x)	;

void kprintf(void *, ...);

/* Hyper message handling routines */
BOOL handle_hyper (struct AppInfo *, struct MsgHandler *, struct TagItem *);
BOOL shutdown_hyper (struct AppInfo *, struct MsgHandler *, struct TagItem *);
struct MsgHandler *setup_hyperA (struct AppInfo *, struct TagItem *);

struct MsgHandler *setup_hyperA (struct AppInfo * ai, struct TagItem * tl)
{
    register struct MsgHandler *mh;
    register struct MHObject *mho;
    ULONG msize;

    /* calculate size of memory block */
    msize = sizeof (struct MsgHandler) + (4L * sizeof (ULONG));

    /* allocate the memory block */
    if (mh = (struct MsgHandler *) AllocVec (msize, MEMF_CLEAR | MEMF_PUBLIC))
    {
	struct NewAmigaGuide *nag = &(ai->ai_NewHyper);

	/* get a pointer to the embedded object header */
	mho = &(mh->mh_Header);

	/* initialize the object header */
	mho->mho_Node.ln_Type = APSH_MH_HANDLER_T;
	mho->mho_Node.ln_Pri = (APSH_MH_HANDLER_P + 2);
	mho->mho_Node.ln_Name = "AMIGAGUIDE";

	/* initialize the object list */
	NewList (&(mho->mho_ObjList));

	/* set up the ID */
	mho->mho_ID = APSH_MAIN_ID;

	/* number of functions in handler */
	mh->mh_NumFuncs = 4;
	mh->mh_Func = MEMORY_FOLLOWING (mh);
	mh->mh_Func[APSH_MH_OPEN] = NULL;
	mh->mh_Func[APSH_MH_HANDLE] = handle_hyper;
	mh->mh_Func[APSH_MH_CLOSE] = NULL;
	mh->mh_Func[APSH_MH_SHUTDOWN] = shutdown_hyper;

	/* Set up the base name */
	nag->nag_BaseName = ai->ai_BaseName;

	/* Set up the file name */
	sprintf (ai->ai_WorkText, "PROGDIR:%s.guide", ai->ai_BaseName);
	msize = strlen (ai->ai_WorkText) + 1;
	if (nag->nag_Name = (STRPTR) AllocVec (msize, MEMF_CLEAR))
	{
	    strcpy (nag->nag_Name, ai->ai_WorkText);
	}

	/* Set up the port name */
	sprintf (ai->ai_WorkText, "%s_HELP", ai->ai_BaseName);
	msize = strlen (ai->ai_WorkText) + 1;
	if (nag->nag_ClientPort = (STRPTR) AllocVec (msize, MEMF_CLEAR))
	{
	    strcpy (nag->nag_ClientPort, ai->ai_WorkText);
	}

	/* Set the screen name */
	nag->nag_PubScreen = ai->ai_ScreenName;

	/* Clear the context table */
	nag->nag_Context = NULL;

	/* Open AmigaGuide system */
	ai->ai_HyperText = OpenAmigaGuideAsync (nag, NULL);

	/* Port has to be already established */
	mh->mh_SigBits = AmigaGuideSignal (ai->ai_HyperText);
    }
    else
    {
	ai->ai_Pri_Ret = RETURN_FAIL;
	ai->ai_Sec_Ret = APSH_NOT_ENOUGH_MEMORY;
	ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID, ai->ai_Sec_Ret, NULL);
    }

    return (mh);
}

STRPTR err_type[] =
{
    "NO ERROR",
    "HTERR_NOT_ENOUGH_MEMORY",
    "HTERR_CANT_OPEN_DATABASE",
    "HTERR_CANT_FIND_NODE",
    "HTERR_CANT_OPEN_NODE",
    "HTERR_CANT_OPEN_WINDOW",
    "HTERR_INVALID_COMMAND",
    "HTERR_CANT_COMPLETE",
    "HTERR_PORT_CLOSED",
    "HTERR_CANT_CREATE_PORT",
    NULL
};

STRPTR msg_type[] =
{
    "<unknown>",
    "StartupMsgID",
    "LoginToolID",
    "LogoutToolID",
    "ShutdownMsgID",
    "ActivateToolID",
    "DeactivateToolID",
    "ActiveToolID",
    "InactiveToolID",
    "ToolStatusID",
    "ToolCmdID",
    "ToolCmdReplyID",
    "ShutdownToolID",
    NULL
};

VOID display_msg (struct AmigaGuideMsg *msg)
{
    LONG type, err;

    type = msg->agm_Type - StartupMsgID + 1;
    err = msg->agm_Sec_Ret - HTERR_NOT_ENOUGH_MEMORY + 1;

    if (err < 0)
	err = 0;

    if (type < 0)
	type = 0;

    kprintf ("%s returns %s\n", msg_type[type], err_type[err]);
}

BOOL handle_hyper (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    struct HyperMsg *msg;

    DB (kprintf ("handle_hyper enter\n"));

    /* Get each hmsg message */
    while (msg = GetAmigaGuideMsg (ai->ai_HyperText))
    {
	DB (display_msg(msg));

	/* Reply to the message */
	ReplyAmigaGuideMsg (msg);
    };

    DB (kprintf ("handle_hyper exit\n"));

    return (TRUE);
}

BOOL shutdown_hyper (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    struct NewAmigaGuide *nag = &(ai->ai_NewHyper);

    if (ai->ai_HyperText)
    {
	DB (kprintf ("CloseAmigaGuide 0x%lx\n", ai->ai_HyperText));
	CloseAmigaGuide (ai->ai_HyperText);
    }

    DB (kprintf ("Free client port name 0x%lx\n", nag->nag_ClientPort));
    FreeVec (nag->nag_ClientPort);

    DB (kprintf ("Free name 0x%lx\n", nag->nag_Name));
    FreeVec (nag->nag_Name);

    DB (kprintf ("shutdown_hyper exit\n"));
    return (TRUE);
}
