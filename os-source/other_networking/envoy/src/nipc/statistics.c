/**************************************************************************
**
** statistics.c - NIPC Statistics functions
**
** Copyright 1992, Commodore-Amiga, Inc.
**
** $Id: statistics.c,v 1.11 93/09/03 17:40:29 kcd Exp $
**
***************************************************************************

/*------------------------------------------------------------------------*/

#include <exec/types.h>
#include "nipcinternal.h"
#include "externs.h"

#include <clib/exec_protos.h>
#include "nipcinternal_protos.h"

#include <pragmas/exec_pragmas.h>

/*------------------------------------------------------------------------*/

void __asm StartStats(register __a0 struct stats * statptr)
{
    register struct NBase *nb = gb;
    statptr->DevList = (struct MinList *) &gb->DeviceList;
    statptr->EntList = &gb->ANMPEntityList;
    statptr->ConnList = &gb->RDPConnectionList;
    statptr->ProList = &gb->ProtocolList;
    statptr->RouteTable = &gb->RouteInfo;

    ObtainSemaphore(&nb->RDPCLSemaphore);
    ObtainSemaphore(&nb->ANMPELSemaphore);
}

/*------------------------------------------------------------------------*/

void __asm EndStats()
{
    register struct NBase *nb = gb;
    ReleaseSemaphore(&nb->ANMPELSemaphore);
    ReleaseSemaphore(&nb->RDPCLSemaphore);
}
/*------------------------------------------------------------------------*/

struct PrivatePool
{
    struct MinList	PP_List;
    ULONG		PP_Flags;
    ULONG		PP_Size;
    ULONG		PP_Thresh;
    struct MinList	PP_Track;
};

struct PrivatePool * __stdargs LockPool(void)
{
    register struct NBase *nb = gb;
    ObtainSemaphore(&nb->PoolLock);
    return((struct PrivatePool *)nb->MemoryPool);
}

VOID __stdargs UnlockPool(void)
{
    register struct NBase *nb = gb;
    ReleaseSemaphore(&nb->PoolLock);
}

