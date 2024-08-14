
/*
 * get base address of rs485.device - DO NOT MAKE ANY GLOBAL REFERENCES HERE
 */

#ifndef RS485_H
#include "rs485.h"
#endif
#include <exec/execbase.h>

void __stdargs
*getlibbase()
{
	struct ExecBase *exb;

	exb = *(struct ExecBase **)4;
	return (void *)FindName(&exb->DeviceList, _LibName);
}
