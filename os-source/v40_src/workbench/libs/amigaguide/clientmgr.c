/* clientmsg.c
 * Client Allocation/Deallocation
 *
 */

#include "amigaguidebase.h"

/*****************************************************************************/

struct Client *AllocClient (struct AmigaGuideLib *ag, struct NewAmigaGuide *nag, struct TagItem *attrs)
{
    struct Client *cl;
    ULONG msize;

    msize = sizeof (struct Client) + ((nag->nag_ClientPort) ? (strlen (nag->nag_ClientPort) + 1) : 0);
    if (cl = AllocVec (msize, MEMF_CLEAR))
    {
	/* Initialize the lists */
	NewList (&cl->cl_WindowList);

	/* Initialize the variables */
	cl->cl_AG         = ag;
	cl->cl_NAG        = nag;
	cl->cl_Flags      = nag->nag_Flags;
	cl->cl_Context    = nag->nag_Context;
	cl->cl_ParentProc = (struct Process *) FindTask (NULL);
	cl->cl_HelpGroup  = GetTagData (AGA_HelpGroup, GetUniqueID (), attrs);

	/* Process the tag list */
	SetAmigaGuideAttrsALVO (ag, cl, attrs);

	/* Create a memory pool */
	if (cl->cl_MemoryPool = CreatePool (MEMF_PUBLIC | MEMF_CLEAR, 1024, 1024))
	{
	    return (cl);
	}
	else
	{
	    SetIoErr (ERR_NOT_ENOUGH_MEMORY);
	}

	FreeVec (cl);
    }
    else
    {
	SetIoErr (ERR_NOT_ENOUGH_MEMORY);
    }

    return (NULL);
}

/*****************************************************************************/

VOID FreeClient (struct AmigaGuideLib *ag, struct Client *cl)
{
    struct AmigaGuideMsg *agm;

    if (cl)
    {
	/* Do we have an asynchronous process? */
	if (cl->cl_AsyncPort)
	{
	    /* Carefully remove all the messages from the message list */
	    while (agm = (struct AmigaGuideMsg *) GetMsg(cl->cl_AsyncPort))
		ReplyAmigaGuideMsgLVO (ag, agm);
	    DeleteMsgPort (cl->cl_AsyncPort);
	}

	if (cl->cl_FR)
	    FreeAslRequest (cl->cl_FR);

	/* Delete the memory pool */
	DeletePool (cl->cl_MemoryPool);

	/* Free our instance data */
	FreeVec (cl);
    }
}
