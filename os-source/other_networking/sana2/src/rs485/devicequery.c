/*
 * S2_devicequery() - return information about this device
 */

#ifndef RS485_H
#include "rs485.h"
#endif

#define	LINERATE	2500000
#define QUERYFORMAT	0
#define DEVICELEVEL	0
#define ADDRSIZE	8

int S2_devicequery(struct IOSana2Req *iob)
{
struct Sana2DeviceQuery st;

	if(!iob->ios2_StatData)
	{
		iob->ios2_Req.io_Error = S2ERR_BAD_ARGUMENT;
		iob->ios2_WireError = S2WERR_NULL_POINTER;
		return COMPLETE;
	}
	st.SizeAvailable = ((struct Sana2DeviceQuery *)iob->ios2_StatData)->SizeAvailable;
	st.SizeSupplied = min(sizeof(st), st.SizeAvailable);
	st.DevQueryFormat = QUERYFORMAT;
	st.DeviceLevel = DEVICELEVEL;
	st.AddrFieldSize = ADDRSIZE;
	st.MTU = RS485_MTU;
	st.BPS = LINERATE;
	st.HardwareType = S2WireType_Arcnet;
	memcpy(iob->ios2_StatData, &st, st.SizeSupplied);
	return COMPLETE;
}
