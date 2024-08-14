/*
 * Attempt to send queued packet on specified unit:
 *
 * try to get xmit page from com chip; leave at least one buf for rcvr
 * try to grab iob from write_q; if none, free xmit buffer & return
 * build packet header into packet header struct
 * build body of packet in transmit buffer.  See com20020 doc pg 36 for format
 * 	also, pg 37: can't send 254,255,256 byte pkts
 * 	512 byte packets have two byte count field: a zero, and a count byte
 * try to send the packet: Disable ints, check if xmit available,
 * 	assert trasmit from buffer command.  Otherwise, just mark buffer
 * 	as "HAS_XMIT_DATA" and let the interrupt driver send the buffer.
 *
 * returns: 0 if no work/buffers, 1 if sent, -1 if error
 */

#ifndef RS485_H
#include "rs485.h"
#endif

int startxmit(struct rs485Unit *up)
{
unsigned int len, offset, dataoffset;
struct TrackedType *ttp, tp;
struct IOSana2Req *iob;
struct pkthdr ph;
int bn, i;

/*
**  Clear current request if excessive negative acknowledgements for that
**  packet.  The interrupt serever will clear the buffer as if it had been
**  sent.
*/
//int dsr;
//	dsr = DSR(up);
//	if(dsr & DSR_EXCNAK)
//	{
//DPRINT(("EXCNAK!!!\n"));
//		CMD(up) = C_DIS_XMIT;
//		/*  Don't I need to free the buffer being worked on here?  */
//	}
//	 else
//	{
//DPRINT(("No EXCNAK.\n"));
//	}
//	if(dsr & DSR_TOKEN)
//		DPRINT(("Token seen.\n"));
//	else
//		DPRINT(("Token not seen.\n"));
//	if(dsr & DSR_MYRECON)
//		DPRINT(("I caused a recon.\n"));
//	else
//		DPRINT(("I haven't caused a recon.\n"));

/*
**  get transmit buffer
*/
DPRINT(("startxmit\n"));
	i = NBUF;
	bn = -1;
	Disable();
	while(--i >= 0)
	{
		if(up->bstate[i] == IS_FREE)
		{
			if(bn < 0)
			{
				up->bstate[i] = IS_BUSY;
    				bn = i;
			} else
			{
				break;
			}
		}
	}
	Enable();
	if(i < 0)
	{
		if(bn >= 0)
		{
			up->bstate[bn] = IS_FREE;
		}
		return 0;
	}
DPRINT(("got buffer %ld\n", bn));
/*
**  pull next writer packet off send queue
*/
	iob = (struct IOSana2Req *)RemHead(&up->write_q);
	if(!iob)
	{
		up->bstate[bn] = IS_FREE;
		return 0;
	}
DPRINT(("got it off send queue\n"));

/*
**  get packet from abstract data structure into contiguous memory.
**  this is an extra copy, but given that we must feed one byte at
**  a time to the COM20020, it is hardly avoidable.  and, depending
**  on the actual buffer management scheme, this could speed up things
**  anyway by allowing a very tight loop to copy the buffer out.
*/
	if(CopyFromBuff(UNIT(iob)->temp[bn], iob->ios2_Data, iob->ios2_DataLength) == FALSE)
	{
DPRINT(("Error on CopyFromBuff()\n"));
		iob->ios2_Req.io_Error = S2ERR_SOFTWARE;
		iob->ios2_WireError = S2WERR_BUFF_ERROR;
		up->bstate[bn] = IS_FREE;
		if(!(iob->ios2_Req.io_Flags & SANA2IOF_QUICK))
		{
			ReplyMsg(&iob->ios2_Req.io_Message);
		}
		return -1;
	}
DPRINT(("did CopyFromBuff (%s) %ld\n", UNIT(iob)->temp[bn], iob->ios2_DataLength));
/*
**  if it's raw, just dump it, else compute lengths/offsets from packet
*/
	if(iob->ios2_Req.io_Flags & SANA2IOF_RAW)
	{
DPRINT(("Raw send!!!\n"));
		ADDRH(up) = (bn << 1) | ADH_AUTOINC;
		ADDRL(up) = 0;
		copyout(UNIT(iob)->temp[bn], &DATA(up), iob->ios2_DataLength);
		goto xmit;
	}
DPRINT(("Not a raw send!!!\n"));
	ph.sid = SUB_REG(up);
	ph.did = (iob->ios2_Req.io_Command==CMD_WRITE)?iob->ios2_DstAddr[0]:BROADCASTADDR;
	ph.type[0] = (UBYTE)iob->ios2_PacketType;
	ph.type[1] = *(((BYTE *)&iob->ios2_PacketType)+1);
	dataoffset = 0;

	len = iob->ios2_DataLength - dataoffset;
/*
**  find stats buffer, if any
*/
	ttp = 0;
	if(!ISEMPTY(&up->trackedtype)){
		ttp = findtracked(up, iob->ios2_PacketType);
	}
	if(!ttp){
		ttp = &tp;
	}
/*
**  marshall header
*/
	ADDRH(up) = (bn << 1) | ADH_AUTOINC;
	ADDRL(up) = 0;
	DATA(up) = ph.sid;
	DATA(up) = ph.did;
	if(len > 253)
	{
		if(len < 257)
		{
			len = 257;
		}
		DATA(up) = 0;
		offset = 512 - len;
DPRINT(("Long packet.\n"));
	} else
	{
		offset = 256 - len;
	}
	offset -= (TWOBYTECODE(&ph)? 2:1);
	DATA(up) = offset;
	ADDRL(up) = offset;
	DATA(up) = ph.type[0];
	if((TWOBYTECODE(&ph)? 2:1) == 2)
	{
		DATA(up) = ph.type[1];
	}
DPRINT(("about to copyout  offset=%ld, tlength=%ld, count=%ld type=%ld\n", offset, (TWOBYTECODE(&ph)? 2:1), len, (long)ph.type[0]));
/*
**  marshall data part of packet
*/
	copyout(UNIT(iob)->temp[bn], &DATA(up), len);
/*
**  output packet to wire + do loopback, if any
*/
xmit:
	if((ph.did==BROADCASTADDR || ph.did==SUB_REG(up)) &&
	   !ISEMPTY(&up->read_q))
	{
		recv(up, bn);
	}
	Disable();
	if(SR(up) & SR_TA)
	{
		CMD(up) = C_SEND(bn);
		up->intmask |= SR_TA;
		SR(up) = up->intmask;
		up->bstate[bn] = IS_CURR_XMIT;
DPRINT(("Actually sending to %ld!!!\n", (long)ph.did));
	} else
	{
		up->bstate[bn] = HAS_XMIT_DATA;
DPRINT(("on send queue.\n"));
	}
	Enable();

/*
**  update stats, reply to message
*/
	up->stats.PacketsSent++;
	ttp->stats.PacketsSent++;
	ttp->stats.BytesSent += iob->ios2_DataLength;
	ReplyMsg(&iob->ios2_Req.io_Message);


	return 1;
}
