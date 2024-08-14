/* apsh_msghandle.c
 * Copyright (C) 1990 Commodore-Amiga, Inc.
 * written by David N. Junod
 *
 * message handler initialization and shutdown
 *
 * $Id: apsh_msghandle.c,v 1.4 1992/09/07 17:57:32 johnw Exp johnw $
 *
 * $Log: apsh_msghandle.c,v $
 * Revision 1.4  1992/09/07  17:57:32  johnw
 * Minor changes.
 *
 * Revision 1.1  91/12/12  14:52:49  davidj
 * Initial revision
 * 
 * Revision 1.1  90/07/02  11:37:04  davidj
 * Initial revision
 *
 *
 */

#include "appshell_internal.h"

#define	DB(x)	;

void kprintf (void *,...);

BOOL AddMsgHandlerA (struct AppInfo * ai, struct TagItem * tl)
{
    struct MsgHandler *(*func) (struct AppInfo *, struct TagItem *);
    struct MsgHandler *mh = NULL;
    BOOL retval = FALSE, needed;

    /* is this handler optional or required */
    needed = (BOOL) GetTagData (APSH_Rating, NULL, tl);

    /* get address of initialization routine */
    func = (VOID *) GetTagData (APSH_Setup, NULL, tl);

    /* only continue if we have a function to call */
    if (func)
    {
	/* call the creation routine */
	mh = (*(func)) (ai, tl);

	/* see if successful */
	if (mh)
	{
	    /* insert the node into the list */
	    Enqueue (&(ai->ai_MsgList), (struct Node *) mh);

	    /* keep track of the signal bits */
	    ai->ai_SigBits |= mh->mh_SigBits;

	    /* success! */
	    retval = TRUE;
	}

    }

    /* return true if optional, otherwise return actual */
    return ((BOOL) ((needed) ? retval : TRUE));
}

BOOL AddIntMsgHandlerA (struct AppInfo * ai, struct MsgHandler * mh,
			 struct TagItem * tl)
{
    BOOL retval = FALSE, needed;

    /* is this handler optional or required */
    needed = (BOOL) GetTagData (APSH_Rating, NULL, tl);

    /* see if successful */
    if (mh)
    {
	/* insert the node into the list */
	Enqueue (&(ai->ai_MsgList), (struct Node *) mh);

	/* keep track of the signal bits */
	ai->ai_SigBits |= mh->mh_SigBits;

	/* success! */
	retval = TRUE;
    }

    /* return true if optional, otherwise return actual */
    return ((BOOL) ((needed) ? retval : TRUE));
}

/* shutdown all the handlers */
VOID DelMsgHandlers (struct AppInfo * ai)
{
    struct Node *node, *nxtnode;
    struct MsgHandler *mh;
    struct MHObject *mho;
    struct List *list;

    if (ai)
    {
	/* get a pointer to the message handler list */
	list = &(ai->ai_MsgList);

	/* make sure that there is a list */
	if (list->lh_TailPred != (struct Node *) list)
	{
	    node = list->lh_Head;
	    while (nxtnode = node->ln_Succ)
	    {
		mh = (struct MsgHandler *) node;
		mho = &(mh->mh_Header);

		/* call the shutdown routine */
		DB (kprintf ("message handler %s\n", mho->mho_Node.ln_Name));
		(*mh->mh_Func[APSH_MH_SHUTDOWN]) (ai, mh, NULL);

		/* get a pointer to the next node */
		node = nxtnode;
	    }
	}
    }
}
