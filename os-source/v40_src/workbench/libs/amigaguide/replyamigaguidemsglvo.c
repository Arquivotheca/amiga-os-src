/* replyamigaguidemsglvo.c
 *
 */

#include "amigaguidebase.h"

/****** amigaguide.library/ReplyAmigaGuideMsg ******************************
*
*   NAME
*	ReplyAmigaGuideMsg - Reply to an AmigaGuide message.    (V34)
*
*   SYNOPSIS
*	ReplyAmigaGuideMsg ( msg );
*			     a0
*
*	VOID ReplyAmigaGuideMsg (struct AmigaGuideMsg *msg);
*
*   FUNCTION
*	This function is used to reply to an AmigaGuide SIPC message.
*
*   INPUTS
*	msg - Pointer to a SIPC message returned by a previous call to
*	    GetAmigaGuideMsg().
*
*   SEE ALSO
*	OpenAmigaGuideAsyncA(), AmigaGuideSignal(), GetAmigaGuideMsg()
*
**********************************************************************
*
* Created:  03-Aug-90, David N. Junod
*
*/

VOID ASM ReplyAmigaGuideMsgLVO (REG (a6) struct AmigaGuideLib *ag, REG (a0) struct AmigaGuideMsg * agm)
{
    if (agm)
    {
	/* process tool message types */
	switch (agm->agm_Type)
	{
	    case StartupMsgID:
	    case ShutdownMsgID:
		break;

	    default:
		if (agm->agm_Msg.mn_Node.ln_Type == NT_MESSAGE)
		{
		    /* reply to the message */
		    ReplyMsg ((struct Message *) agm);
		}
		else if (agm->agm_Msg.mn_Node.ln_Type == NT_REPLYMSG)
		{
		    /* Return the message to the pool */
		    FreePVec (ag, ag->ag_MemoryPool, agm);
		}
		break;
	}
    }
}
