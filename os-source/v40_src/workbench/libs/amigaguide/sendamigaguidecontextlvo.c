/* sendamigaguidecontextlvo.c
 *
 */

#include "amigaguidebase.h"

#define	DB(x)	;

/****** amigaguide.library/SendAmigaGuideContextA ***************************
*
*   NAME
*	SendAmigaGuideContextA - Align an AmigaGuide system on the context ID.
*                                                               (V34)
*   SYNOPSIS
*	success = SendAmigaGuideContextA (handle, attrs);
*	d0				  a0	  d0
*
*	BOOL SendAmigaGuideContextA (AMIGAGUIDECONTEXT, struct TagItem *);
*
*	success = SendAmigaGuideContext (handle, tag1, ...);
*
*	BOOL SendAmigaGuideContext (AMIGAGUIDECONTEXT, Tag, ...);
*
*   FUNCTION
*	This function is used to send a message to an AmigaGuide system to
*	align it on the current context ID.
*
*	This function effectively does a:
*
*	    SendAmigaGuideCmd(handle 'LINK ContextArray[contextID]', NULL);
*
*   INPUTS
*	handle - Handle to an AmigaGuide system.
*	future - Future expansion, must be set to NULL for now.
*
*   EXAMPLE
*
*	struct IntuiMessage *imsg;
*
*	...
*
*	case RAWKEY:
*	    switch (imsg->Code)
*	    {
*		case 95:
*		    \* bring up help on a particular subject *\
*		    SendAmigaGuideContext(handle, NULL);
*		    break;
*		...
*	    }
*	    break;
*
*	...
*
*   RETURNS
*	success	- Returns TRUE if the message was sent, otherwise returns
*	    FALSE.
*
*   SEE ALSO
*	SetAmigaGuideContextA(), SendAmigaGuideCmdA()
*
**********************************************************************
*
* Created:  03-Aug-90, David N. Junod
*
*/

LONG ASM SendAmigaGuideContextALVO (REG (a6) struct AmigaGuideLib *ag, REG (a0) struct Client * cl, REG (d0) struct TagItem *attrs)
{
#if 1
    return (SendAmigaGuideCmd (ag, cl, NULL, AGA_Context, cl->cl_ContextID, TAG_DONE) );
#else
    LONG max = 0L;

    /* make sure we have a client */
    DB (kprintf ("SendAmigaGuideContextALVO (0x%lx)\n", cl));
    if (cl && cl->cl_Context)
    {
	if (!(cl->cl_ContextID & AGFC_SYSGADS))
	{
	    while (cl->cl_Context[max])
		max++;

	    if (cl->cl_ContextID < max)
	    {
		sprintf (cl->cl_WorkText, "LINK %s", cl->cl_Context[cl->cl_ContextID]);
		DB (kprintf ("send %s\n", cl->cl_WorkText));
		return (SendAmigaGuideCmdALVO (ag, cl, cl->cl_WorkText, NULL));
	    }
	}
    }
    DB (kprintf ("couldn't send\n"));
    return FALSE;
#endif
}
