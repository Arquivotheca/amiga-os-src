
/*
 * CMD_rdwr - read/write processing; sanity checks are performed here, so that
 * user gets immediate feedback about a bad i/o request.
 *
 * Generic checks: iob must have legal packet type, unit must be ONLINE, and
 * node ID must have been set to non zero value.
 *
 * Write specific sanity check - packet must:
 *   if SANA2IOF_RAW the packet + CNT + ZERO bytes must fit in 512 byte page
 *   else length of packet + length of type field + SID + DID + CNT + ZERO < 512 page
 */

#ifndef RS485_H
#include "rs485.h"
#endif

#define DTL(iob) ((iob)->ios2_DataLength)

int CMD_rdwr(struct IOSana2Req *iob)
{
ULONG actual;

/*
**  checktype() no longer needed because of changes to spec.
**
**	if(!checktype(iob)){
**		return COMPLETE;
**	}
*/
	if(UNIT(iob)->state != ONLINE || SUB_REG(UNIT(iob)) == 0){
		iob->ios2_Req.io_Error = S2ERR_BAD_STATE;
		iob->ios2_WireError = S2WERR_UNIT_OFFLINE;
		return COMPLETE;
	}

	switch(iob->ios2_Req.io_Command){
	case CMD_WRITE:
	case S2_BROADCAST:
		if(DTL(iob)==0 || ((iob->ios2_Req.io_Flags & SANA2IOF_RAW) && DTL(iob)<3)){
			iob->ios2_Req.io_Error = S2ERR_BAD_ARGUMENT;
			return COMPLETE;
		}

		actual = DTL(iob) + 2;  /* len of pkt + 2 bytes (count/zero) */
		if(!(iob->ios2_Req.io_Flags & SANA2IOF_RAW))
		{
			actual += + 2;
		}

		if(actual > 512){
			iob->ios2_Req.io_Error = S2ERR_MTU_EXCEEDED;
			return COMPLETE;
		}

		iob->ios2_Req.io_Flags &= ~SANA2IOF_QUICK;
		AddTail(&UNIT(iob)->write_q, &iob->ios2_Req.io_Message.mn_Node);
		startxmit(UNIT(iob));
		break;

	case CMD_READ:
	case S2_READORPHAN:
		iob->ios2_Req.io_Flags &= ~SANA2IOF_QUICK;
		AddTail(&UNIT(iob)->read_q, &iob->ios2_Req.io_Message.mn_Node);
		break;
	}

	return QUEUED;
}

