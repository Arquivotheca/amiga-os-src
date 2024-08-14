/* apsh_clonesipc.c
 * Copyright (C) 1990 Commodore-Amiga, Inc.
 * written by David N. Junod
 *
 * control module for cloned AppShells
 *
 * $Id: apsh_clonesipc.c,v 1.4 1992/09/07 17:49:57 johnw Exp johnw $
 *
 * $Log: apsh_clonesipc.c,v $
 * Revision 1.4  1992/09/07  17:49:57  johnw
 * Minor changes.
 *
 * Revision 1.1  91/12/12  14:49:50  davidj
 * Initial revision
 * 
 * Revision 1.1  90/07/02  11:17:23  davidj
 * Initial revision
 *
 *
 */

#include "appshell_internal.h"

/* SIPC message handling routines */
BOOL handle_apsh_sipc (struct AppInfo *, struct MsgHandler *, struct TagItem *);
BOOL shutdown_apsh_sipc (struct AppInfo *, struct MsgHandler *, struct TagItem *);
struct MsgHandler *setup_apsh_sipcA (struct AppInfo *, struct TagItem *);

struct MsgHandler *setup_apsh_sipcA (struct AppInfo * ai, struct TagItem * tl)
{
    register struct MsgHandler *mh;
    register struct MHObject *mho;
    ULONG msize;

    /* calculate size of memory block */
    msize = sizeof (struct MsgHandler) + (4L * sizeof (ULONG));

    /* allocate the memory block */
    if (mh = (struct MsgHandler *) AllocVec (msize, MEMF_CLEAR | MEMF_PUBLIC))
    {
	/* get a pointer to the embedded object header */
	mho = &(mh->mh_Header);

	/* initialize the object header */
	mho->mho_Node.ln_Type = APSH_MH_HANDLER_T;
	mho->mho_Node.ln_Pri = APSH_MH_HANDLER_P;
	mho->mho_Node.ln_Name = "SIPC";

	/* initialize the object list */
	NewList (&(mho->mho_ObjList));

	/* set up the ID */
	mho->mho_ID = APSH_MAIN_ID;

	/* number of functions in handler */
	mh->mh_NumFuncs = 4;
	mh->mh_Func = MEMORY_FOLLOWING (mh);
	mh->mh_Func[APSH_MH_OPEN] = NULL;
	mh->mh_Func[APSH_MH_HANDLE] = handle_apsh_sipc;
	mh->mh_Func[APSH_MH_CLOSE] = NULL;
	mh->mh_Func[APSH_MH_SHUTDOWN] = shutdown_apsh_sipc;

	/* Port has to be already established */
	if (mh->mh_Port = (struct MsgPort *) GetTagData (APSH_Port, NULL, tl))
	{
	    mh->mh_SigBits = (1L << mh->mh_Port->mp_SigBit);
	    return (mh);
	}
	else
	{
	    ai->ai_Pri_Ret = RETURN_FAIL;
	    ai->ai_Sec_Ret = APSH_NO_PORT;
	    ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID, ai->ai_Sec_Ret, NULL);
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

BOOL handle_apsh_sipc (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    struct SIPCMessage *msg;

    /* Get each SIPC message */
    while (msg = (struct SIPCMessage *) GetMsg (mh->mh_Port))
    {
	switch (msg->sipc_Type)
	{
	case ShutdownToolID:
	    ai->ai_Done = TRUE;
	    break;

	case ActivateToolID:
	    PerfFunc (ai, ActivateID, NULL, NULL);
	    break;

	case DeactivateToolID:
	    PerfFunc (ai, DeActivateID, NULL, NULL);
	    break;

	default:
	    break;
	}

	/* Reply to the message */
	ReplyMsg ((struct Message *) msg);
    };

    return (TRUE);
}

BOOL shutdown_apsh_sipc (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    Remove ((struct Node *) mh);
    FreeVec ((APTR) mh);

    return (TRUE);
}
