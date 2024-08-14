/*
**  S2_onevent()
*/

#ifndef RS485_H
#include "rs485.h"
#endif

/*
 * process onevent request packet.  This request may blocks, since the driver
 * does not retain state about the various error conditions this request is
 * interested in.  ONLINE or OFFLINE may return immediately, however.
 */
int S2_onevent(iob)
	struct IOSana2Req *iob;
{
	if(iob->ios2_WireError > S2EVENT_SOFTWARE){
		iob->ios2_Req.io_Error = S2ERR_BAD_ARGUMENT;
		iob->ios2_WireError = S2WERR_BAD_EVENT;
		return COMPLETE;
	}
	if( (iob->ios2_WireError & S2EVENT_ONLINE) && UNIT(iob)->state == ONLINE)
	{
		iob->ios2_WireError = S2EVENT_ONLINE;
		return COMPLETE;
	}
	if( (iob->ios2_WireError & S2EVENT_OFFLINE) && UNIT(iob)->state == OFFLINE)
	{
		iob->ios2_WireError = S2EVENT_OFFLINE;
		return COMPLETE;
	}

	iob->ios2_Req.io_Flags &= ~SANA2IOF_QUICK;
	AddTail(&UNIT(iob)->misc_q, &iob->ios2_Req.io_Message.mn_Node);
	return QUEUED;
}

/*
 * search events queue for any requests waiting for specified event, and return
 * then to sender.
 */
void wakeup(struct rs485Unit *up, ULONG event)
{
struct IOSana2Req *iob, *next;

	iob = (struct IOSana2Req *)(up->misc_q.lh_Head);
	while(iob->ios2_Req.io_Message.mn_Node.ln_Succ)
	{
		next = (struct IOSana2Req *)iob->ios2_Req.io_Message.mn_Node.ln_Succ;
		if(iob->ios2_Req.io_Command == S2_ONEVENT &&
		   event & iob->ios2_WireError)
		{
			iob->ios2_Req.io_Error = 0;
			iob->ios2_WireError = event & iob->ios2_WireError;
			Remove(&iob->ios2_Req.io_Message.mn_Node);
			ReplyMsg(&iob->ios2_Req.io_Message);
		}
		iob = next;
	}
}

