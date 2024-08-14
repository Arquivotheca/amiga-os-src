/*
** recv() - read incoming packet out of packet buffer, match with with pending
**	    read/readorphan request.  This function is called by the listener
**	    task, as well as startxmit to echo packets to local readers.
*/

#ifndef RS485_H
#include "rs485.h"
#endif


void recv(struct rs485Unit *up, int bn)
{
unsigned int count, offset;
struct IOSana2Req *iob, *oiob;
struct TrackedType *ttp, tp;
struct pkthdr ph;
int tlength;
ULONG type;

/*
**  pull header off interface into struct pkthdr
*/
DPRINT(("Recv called!!!\n"));
	ADDRH(up) = (bn << 1) | ADH_AUTOINC | ADH_RDDATA;
	ADDRL(up) = 0;
	ph.sid = DATA(up);
	ph.did = DATA(up);
	offset = DATA(up);
	if(offset == 0)
	{
		/* long packet */
		offset = DATA(up);
		count = 512 - offset;
DPRINT(("long packet -- offset=%ld, count=%ld.\n", (long)offset, (long)count));
	} else
	{
		count = 256 - offset;
	}
	ADDRL(up) = offset;
	ph.type[0] = DATA(up);
	tlength = 1;
	type = (ULONG) ph.type[0];
	if(TWOBYTECODE(&ph))
	{
		tlength = 2;
		ph.type[1] = DATA(up);
		type = *(UWORD *)ph.type;
DPRINT(("I might have the type backwards...\n"));
	}
	count -= tlength;

DPRINT(("Count=%ld  tlength=%ld  offset=%ld type=%ld\n", count, tlength, offset, (long)ph.type[0]));

/*
**  find stats buffer, if any
*/
	ttp = 0;
	if(!ISEMPTY(&up->trackedtype))
	{
		ttp = findtracked(up, type);
	}
	if(!ttp)
	{
		ttp = &tp; /* null stats buffer */
	}
/*
**  search queued requests; pull iob that wants this packet
*/
	oiob = 0;
	FORALL(&up->read_q, iob, struct IOSana2Req *)
	{
		if(iob->ios2_Req.io_Command == S2_READORPHAN)
		{
			if(!oiob)
			{
				oiob = iob;
			}
			continue;
		}
		if(iob->ios2_PacketType == type)
		{
			break;
		}
	}
	if(!iob->ios2_Req.io_Message.mn_Node.ln_Succ)
	{
		up->stats.UnknownTypesReceived++;
		if(!oiob)
		{
			ttp->stats.PacketsDropped++;
DPRINT(("bailing because type %ld has no queued requests.\n", type));
			return;
		}
		iob = oiob;
	}
	Remove(&iob->ios2_Req.io_Message.mn_Node);
/*
**  Adjust things for raw
*/
	if(iob->ios2_Req.io_Flags & SANA2IOF_RAW)
	{
DPRINT(("Eat it RAW!!!\n"));
		ADDRL(up) = 0;
		if(count > 253)
		{
			count = 512;
DPRINT(("Long packet.\n"));
		} else
		{
			count = 256;
		}
	}
	iob->ios2_DataLength = count;
/*
** push packet data field into iob
*/
	copyin(&DATA(up), UNIT(iob)->temp[bn], iob->ios2_DataLength);
DPRINT(("copyin'd (%s) %ld\n", UNIT(iob)->temp[bn], iob->ios2_DataLength));
	CopyToBuff(iob->ios2_Data, UNIT(iob)->temp[bn], iob->ios2_DataLength);
/*
**  update iob fields to reflect transaction
*/
	iob->ios2_SrcAddr[0] = ph.sid;
	iob->ios2_DstAddr[0] = ph.did;
	if(ph.did == BROADCASTADDR)
	{
		iob->ios2_Req.io_Flags |= SANA2IOF_BCAST;
	}
	iob->ios2_Req.io_Flags &= ~SANA2IOF_MCAST;
/*
**  update stats, set flags to reflect packet type
*/
	ttp->stats.BytesReceived += iob->ios2_DataLength;
	ttp->stats.PacketsReceived++;
	up->stats.PacketsReceived++;
/*
**  reply to message
*/
reply:	if(iob->ios2_Req.io_Error != S2ERR_NO_ERROR)
	{
		ttp->stats.PacketsDropped++;
	}
	ReplyMsg(&iob->ios2_Req.io_Message);
DPRINT(("recv() exiting in civilized fashion (%s)\n", UNIT(iob)->temp[bn]));
DPRINT(("DataLength=%ld SrcAddr=%ld  DstAddr=%ld\n", iob->ios2_DataLength,
(long)iob->ios2_SrcAddr[0], (long)iob->ios2_DstAddr[0]));
}
