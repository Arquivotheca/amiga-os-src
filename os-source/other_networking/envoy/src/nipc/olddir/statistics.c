/**************************************************************************
**
** statistics.c - NIPC Statistics functions
**
** Copyright 1992, Commodore-Amiga, Inc.
**
** $Id: statistics.c,v 1.9 92/05/19 14:39:48 gregm Exp $
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
