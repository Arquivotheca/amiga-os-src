/*
**  S2_configinterface()
*/

#ifndef RS485_H
#include "rs485.h"
#endif

/*
** configinterface - start receiver.  Interrupt will occur after Enable()
**		since receiver is inhibited.  intr routine will give the
**		receiver its first buffer.
*/
int S2_configinterface(struct IOSana2Req *iob)
{
	switch(UNIT(iob)->state){
	case INITIAL:
		if(iob->ios2_SrcAddr[0] == BROADCASTADDR)
		{
			iob->ios2_Req.io_Error = S2ERR_BAD_ADDRESS;
			return COMPLETE;
		}

		Disable();
		UNIT(iob)->myaddr = iob->ios2_SrcAddr[0];
		CFG(UNIT(iob)) &= ~CFG_TXEN;
		CFG(UNIT(iob)) = (CFG(UNIT(iob)) & ~3) | SUB_NODE_ID;
		SUB_REG(UNIT(iob)) = UNIT(iob)->myaddr;
		CFG(UNIT(iob)) |= CFG_TXEN;
/* Fix Long Packet Bug Christmas present: */
                CMD(UNIT(iob)) = C_CONFIG;
/* XXX end one-liner */
   		UNIT(iob)->state = ONLINE;
		UNIT(iob)->intmask = SR_RI | SR_RECON;
		SETINTMASK(UNIT(iob), UNIT(iob)->intmask);
		Enable();

		resetstats(UNIT(iob));
		WAKEUP(UNIT(iob), S2EVENT_ONLINE);
		break;

	case ONLINE:
	case OFFLINE:
	default:
		iob->ios2_Req.io_Error = S2ERR_BAD_STATE;
		iob->ios2_WireError = S2WERR_IS_CONFIGURED;
		break;
	}

	return COMPLETE;
}
