/**************************************************************************
**
** monitor.c    - NIPC Monitor functions
**
** Copyright 1992, Commodore-Amiga, Inc.
**
** $Id: monitor.c,v 1.5 92/04/10 00:39:36 kcd Exp $
**
***************************************************************************

/*------------------------------------------------------------------------*/

#include "nipcinternal.h"
#include "externs.h"


#include <clib/exec_protos.h>
#include "nipcinternal_protos.h"

#include <pragmas/exec_pragmas.h>

/*------------------------------------------------------------------------*/

struct NIPCMonitorInfo *__asm StartMonitor(void)
{
	register struct NBase *nb = gb;
    ULONG *uptr;
    Forbid();
    if (!gb->MonitorInfo.nmi_Monitor)
    {
        nb->MonitorInfo.nmi_Monitor = FindTask(NULL);
        if (nb->MonitorInfo.nmi_SigBitNum = AllocSignal(-1L))
        {
            for (uptr = &gb->MonitorInfo.nmi_IPIn; uptr <= &gb->MonitorInfo.nmi_UDPBytesOut; uptr++)
                *uptr = 0L;
            nb->MonitorInfo.nmi_AvailChipStart = AvailMem(MEMF_CHIP);
            nb->MonitorInfo.nmi_AvailFastStart = AvailMem(MEMF_FAST);
            Permit();
            return &gb->MonitorInfo;
        }
        else
        {
            gb->MonitorInfo.nmi_Monitor = NULL;
            Permit();
            return NULL;
        }
    }
    else
    {
        Permit();
        return NULL;
    }
}

/*------------------------------------------------------------------------*/

void __asm StopMonitor(void)
{
	register struct NBase *nb = gb;
    Forbid();
    if (nb->MonitorInfo.nmi_Monitor == FindTask(NULL))
    {
        FreeSignal(nb->MonitorInfo.nmi_SigBitNum);
        gb->MonitorInfo.nmi_Monitor = NULL;
    }
    Permit();
}
/*------------------------------------------------------------------------*/
