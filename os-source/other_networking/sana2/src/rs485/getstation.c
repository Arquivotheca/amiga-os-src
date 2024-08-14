/*
 * S2_getstationaddress() - return current station address.
 */

#ifndef RS485_H
#include "rs485.h"
#endif

/*
 * getstationaddress: if node ID reg is non zero, the network address is known,
 * otherwise a S2_CONFIGINTERFACE has not been done.
 */
S2_getstationaddress(iob)
	struct IOSana2Req *iob;
{
	memset(iob->ios2_SrcAddr, '\0', SANA2_MAX_ADDR_BYTES);
	memset(iob->ios2_DstAddr, '\0', SANA2_MAX_ADDR_BYTES);

	if(UNIT(iob)->myaddr)
	{
		iob->ios2_SrcAddr[0] = SUB_REG(UNIT(iob));
	} else {
		iob->ios2_Req.io_Error = S2ERR_BAD_STATE;
	}
	return COMPLETE;
}

