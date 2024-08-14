#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <intuition/intuition.h>


#define	REGIST	REGISTER



VOID	CloseSharedPortWindow(window)
REGIST	struct	Window *window;
/* If more than one window is sharing an IDCMP message port, you 
 * should get rid of any queued messages addressed to this window 
 * before freeing the window, else the messages will still 
 * be linked into the message queued while the closing of the window 
 * causes the messages' RAM to be returned back to the system free pool.  
 * From code by Neil Katin, modified by Jim Mackraz, turned back into 
 * English with comments added by RJ.
 */
{
	REGIST	struct	IntuiMessage *message, *nextmessage;
	REGIST	struct	MsgPort *port;

	/* Do most of this under the safe veil of Forbid() */
	Forbid();

	/* Initializations */
	port = window->UserPort;
	message = (struct IntuiMessage *)port->mp_MsgList.lh_Head;

	/* While we're not looking at the last (the anchor) message 
	 * of this port, if the message was addressed to this 
	 * window then unlink it and reply to it.  
	 */
	while ( nextmessage = (struct IntuiMessage *)
			message->ExecMessage.mn_Node.ln_Succ )
		{
		if (message->IDCMPWindow == window)
			{
			Remove(message);
			ReplyMsg(message);
			}
		message = nextmessage;
		}

	/* Now do the Intuition-approved technique of unlinked usage 
	 * of the shared message port.  Henceforth this window 
	 * can receive no more messages
	 */
	window->UserPort = NULL;
	ModifyIDCMP(window, NULL);

	/* Now that our window can no longer receive unattended messages, 
	 * it's safe to turn the system back on.
	 */
	Permit();

	/* Lastly, close that puppy */
	CloseWindow(window);
}


