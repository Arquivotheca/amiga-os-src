/* getamigaguidemsglvo.c
 *
 */

#include "amigaguidebase.h"

/****** amigaguide.library/GetAmigaGuideMsg ********************************
*
*   NAME
*	GetAmigaGuideMsg - Receive async AmigaGuide message.    (V34)
*
*   SYNOPSIS
*	msg = GetAmigaGuideMsg (handle);
*	d0			 a0
*
*	struct AmigaGuideMsg *GetAmigaGuideMsg (AMIGAGUIDECONTEXT);
*
*   FUNCTION
*	This function returns a SIPC message from the AmigaGuide system,
*	if there is a message available.
*
*   INPUTS
*	handle - Handle to a AmigaGuide system.
*
*   EXAMPLE
*
*	AMIGAGUIDECONTEXT handle;
*	struct AmigaGuideMsg *agm;
*
*	\* get a AmigaGuide message *\
*	while (agm = GetAmigaGuideMsg(handle))
*	{
*	    \* process the event *\
*	    switch (agm->agm_Type)
*	    {
*		case ToolCmdReplyID:	\* a command has completed *\
*		    if (agm->agm_Pri_Ret)
*		    {
*			\* An error occurred, the reason is in agm_Sec_Ret.
*			 * The command string is in agm_Data
*			 *\
*		    }
*		    break;
*
*		case ToolStatusID:	\* status message *\
*		    if (agm->agm_Pri_Ret)
*		    {
*			\* an error occurred, the reason is in agm_Sec_Ret *\
*		    }
*		    break;
*
*		default:
*		    break;
*	    }
*
*	    \* reply to the AmigaGuide message *\
*	    ReplyAmigaGuideMsg(agm);
*	}
*
*   RETURNS
*	msg	- Pointer to a SIPC message or NULL if no message was
*		  available.
*
*   SEE ALSO
*	OpenAmigaGuideAsyncA(), AmigaGuideSignal(), ReplyAmigaGuideMsg()
*
**********************************************************************
*
* Created:  03-Aug-90, David N. Junod
*
*/

struct AmigaGuideMsg * ASM GetAmigaGuideMsgLVO (REG (a6) struct AmigaGuideLib *ag, REG (a0) struct Client * cl)
{
    struct AmigaGuideMsg *agm = NULL;

    /* make sure we have a client */
    if (cl && cl->cl_AsyncPort)
    {
	/* get the message */
	if (agm = (struct AmigaGuideMsg *) GetMsg (cl->cl_AsyncPort))
	{
	    switch (agm->agm_Type)
	    {
		case ToolCmdID:
		    if (agm->agm_Msg.mn_Node.ln_Type == NT_REPLYMSG)
			agm->agm_Type = ToolCmdReplyID;
		    break;
#if 0
		case StartupMsgID:
		    cl->cl_ChildPort = agm->agm_Data;
		    break;
#endif
	    }
	}
    }

    /* return any extra messages */
    return (agm);
}
