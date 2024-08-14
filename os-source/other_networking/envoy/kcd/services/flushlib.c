/*
** $Id: flushlib.c,v 37.7 92/06/09 15:17:43 kcd Exp $
**
** Module to free our Library base stuff
**
*/

/*--------------------------------------------------------------------------*/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef EXEC_LISTS_H
#include <exec/lists.h>
#endif

#ifndef EXEC_SEMAPHORES_H
#include <exec/semaphores.h>
#endif

#ifndef EXEC_MEMORY_H
#include <exec/memory.h>
#endif

#ifndef ENVOY_NIPC_H
#include <envoy/nipc.h>
#endif

#include "services.h"
#include "servicesbase.h"
#include "servicesinternal.h"
#include <envoy/nipc.h>

#include <dos.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/nipc_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/nipc_pragmas.h>
#include <pragmas/utility_pragmas.h>

/*--------------------------------------------------------------------------*/

VOID ASM FlushLib()
{
    struct Service *svc;
    struct ServicesLib *S_Lib = ServicesBase;

    while(svc = (struct Service *) RemHead((struct List *)&S_Lib->SVCS_Services))
    {
        DBMSG(("FlushLib(): Freeing Service: %lx\n",svc));
        FreeMem(svc,sizeof(struct Service));
    }
}
