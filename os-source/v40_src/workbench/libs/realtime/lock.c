
#include <exec/semaphores.h>

#include <clib/exec_protos.h>

#include <pragmas/exec_pragmas.h>

#include "realtimebase.h"


/*****************************************************************************/


APTR ASM LIBLockRealTime(REG(d0) ULONG lockType,
			 REG(a6) struct RealTimeLib *RealTimeBase)
{
    if (lockType == RT_CONDUCTORS)
    {
	ObtainSemaphore(&RealTimeBase->rtl_ConductorLock);
	return(&RealTimeBase->rtl_ConductorLock);
    }

    return(NULL);
}


/*****************************************************************************/


VOID ASM LIBUnlockRealTime(REG(a0) struct SignalSemaphore *lockHandle,
			   REG(a6) struct RealTimeLib *RealTimeBase)
{
    if (lockHandle)
        ReleaseSemaphore(lockHandle);
}
