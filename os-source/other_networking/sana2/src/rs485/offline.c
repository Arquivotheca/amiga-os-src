/*
 * S2_OFFLINE/S2_ONLINE processing.
 */

#ifndef RS485_H
#include "rs485.h"
#endif

#include <devices/timer.h>

/*
 * return all messages on given queue with specified error(s)
 */
void
empty_q(struct List *lp, UBYTE err, ULONG werr)
{
struct IOSana2Req *p, *q;

	for(p = (struct IOSana2Req *)lp->lh_Head;
		p->ios2_Req.io_Message.mn_Node.ln_Succ;  p = q)
	{

		q = (struct IOSana2Req *)p->ios2_Req.io_Message.mn_Node.ln_Succ;
		Remove(&p->ios2_Req.io_Message.mn_Node);
		p->ios2_Req.io_Error = err;
		p->ios2_WireError = werr;
		ReplyMsg(&p->ios2_Req.io_Message);
	}
}

/*
 * reset unit statistics, set timeval in global stats to current time
 */
void resetstats(struct rs485Unit *up)
{
struct TrackedType *tp;
struct timerequest tr;
struct MsgPort *rport;

	FORALL(&up->trackedtype, tp, struct TrackedType *)
	{
		memset(&tp->stats, 0, sizeof(tp->stats));
	}
	memset(&up->stats, 0, sizeof(up->stats));
	if(OpenDevice(TIMERNAME, UNIT_VBLANK, (struct IORequest *)&tr, 0) == 0)
	{
		rport = CreatePort(0, 0);
		if(rport)
		{
			tr.tr_node.io_Message.mn_ReplyPort = rport;
			tr.tr_node.io_Command = TR_GETSYSTIME;
			DoIO((struct IORequest *)&tr);
			up->stats.LastStart = tr.tr_time;
			DeletePort(rport);
		}
		CloseDevice((struct IORequest *)&tr);
	}
}


/*
** take unit offline.  When in offline state, the driver will not touch any
** device registers, memory, etc.
*/
int
S2_offline(struct IOSana2Req *iob)
{
	if(UNIT(iob)->state != ONLINE)
	{
		iob->ios2_Req.io_Error = S2ERR_BAD_STATE;
		return COMPLETE;
	}
	empty_q(&UNIT(iob)->read_q, S2ERR_OUTOFSERVICE, S2WERR_GENERIC_ERROR);
	empty_q(&UNIT(iob)->write_q,S2ERR_OUTOFSERVICE, S2WERR_GENERIC_ERROR);
	WAKEUP(UNIT(iob), S2EVENT_OFFLINE);
	UNIT(iob)->state = OFFLINE;

	return COMPLETE;
}


/*
** take unit online.  The online state may only be entered while the unit is
** in the INITIAL state, and when there is a valid address setting.

	BROKEN!  shouldnt zero address, etc.
*/
int
S2_online(struct IOSana2Req *iob)
{
	if(UNIT(iob)->state == OFFLINE && UNIT(iob)->myaddr)
	{
		WAKEUP(UNIT(iob), S2EVENT_ONLINE);
		UNIT(iob)->state = ONLINE;
		resetstats(UNIT(iob));
	} else
	{
		iob->ios2_Req.io_Error = S2ERR_BAD_STATE;
	}
	return COMPLETE;
}

