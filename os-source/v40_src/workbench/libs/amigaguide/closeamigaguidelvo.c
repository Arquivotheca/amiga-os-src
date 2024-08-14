/* closeamigaguidelvo.c
 *
 */

#include "amigaguidebase.h"

#define	DB(x)	;

/****** amigaguide.library/CloseAmigaGuide *****************************
*
*    NAME
*	CloseAmigaGuide - Close a AmigaGuide client.            (V34)
*
*    SYNOPSIS
*	CloseAmigaGuide (handle);
*			  a0
*
*	VOID CloseAmigaGuide (AMIGAGUIDECONTEXT);
*
*    FUNCTION
*	Closes a synchronous, or asynchronous, AmigaGuide client.
*
*	This function will also close all windows that were opened for
*	the client.
*
*    INPUTS
*	handle - Handle to an AmigaGuide client.
*
*    SEE ALSO
*	OpenAmigaGuideA(), OpenAmigaGuideAsyncA()
*
**********************************************************************
*
* Created:  03-Aug-90, David N. Junod
*
*/

VOID ASM CloseAmigaGuideLVO (REG (a6) struct AmigaGuideLib *ag, REG (a0) struct Client * cl)
{
    DB (kprintf ("CloseAmigaGuide : enter\n"));

    if (cl->cl_ChildPort)
    {
	struct AmigaGuideMsg *agm;

	/* Remove all the outstanding messages */
	DB (kprintf ("remove outstanding messages\n"));
	while (agm = GetAmigaGuideMsgLVO (ag, cl))
	    ReplyAmigaGuideMsgLVO (ag, agm);

	/* Allocate a message to send */
	DB (kprintf ("send shutdown\n"));
	if (agm = AllocPVec (ag, ag->ag_MemoryPool, AGMSIZE))
	{
	    /* Initialize the message */
	    agm->agm_Type		 = ShutdownToolID;
	    agm->agm_Msg.mn_Node.ln_Type = NT_MESSAGE;
	    agm->agm_Msg.mn_ReplyPort    = cl->cl_AsyncPort;
	    agm->agm_Msg.mn_Length       = AGMSIZE;

	    /* Send the message */
	    PutMsg (cl->cl_ChildPort, (struct Message *) agm);

	    /* Wait for the reply */
	    WaitPort (cl->cl_AsyncPort);
	}

	/* Remove all the outstanding messages */
	DB (kprintf ("remove outstanding messages\n"));
	while (agm = GetAmigaGuideMsgLVO (ag, cl))
	    ReplyAmigaGuideMsgLVO (ag, agm);

	/* Wait until they are done */
	DB (kprintf ("wait for completion\n"));
	while (!(cl->cl_Flags & AGF_DONE))
	    Delay (5);
    }

    DB (kprintf ("free client\n"));
    FreeClient (ag, cl);

    DB (kprintf ("CloseAmigaGuide : exit\n"));
}
