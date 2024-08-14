/*
 * S2_getglobalstats()
 */

#ifndef RS485_H
#include "rs485.h"
#endif

int S2_getglobalstats(iob)
	struct IOSana2Req *iob;
{
	if(!iob->ios2_StatData){
		iob->ios2_Req.io_Error = S2ERR_BAD_ARGUMENT;
		iob->ios2_WireError = S2WERR_NULL_POINTER;
	} else {
		memcpy(iob->ios2_StatData, &UNIT(iob)->stats, sizeof(UNIT(iob)->stats));
	}
	return COMPLETE;
}
