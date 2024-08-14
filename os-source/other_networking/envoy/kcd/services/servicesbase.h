#ifndef SERVICESBASE_H
#define SERVICESBASE_H

/*****************************************************************************/

#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/lists.h>
#include <exec/semaphores.h>
#include <exec/execbase.h>
#include <utility/tagitem.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>
#include "services.h"
#include <dos.h>

/*****************************************************************************/

struct ServicesLib
{
    struct Library           SVCS_Lib;
    APTR                     SVCS_NIPCBase;
    APTR                     SVCS_UtilityBase;
    struct ExecBase         *SVCS_SysBase;
    BPTR                     SVCS_SegList;

    /* Services Library and Services Manager Common */
    struct SignalSemaphore   SVCS_ServicesLock;
    struct MinList           SVCS_Services;
    struct SignalSemaphore   SVCS_OpenLock;
};

#ifndef SERVICES_MANAGER
#define ASM           __asm
#define REG(x)        register __ ## x

#define ServicesBase  ((struct ServicesLib *)getreg(REG_A6))
#define SysBase       ServicesBase->SVCS_SysBase
#define UtilityBase   ServicesBase->SVCS_UtilityBase
#define NIPCBase      ServicesBase->SVCS_NIPCBase

#define D_S(type,name) char a_##name[sizeof(type)+3]; \
                       type *name = (type *)((LONG)(a_##name+3) & ~3);

/*****************************************************************************/

APTR ASM FindServiceA(REG(a0) STRPTR remoteHost,
                     REG(a1) STRPTR serviceName,
                     REG(a2) APTR srcEntity,
                     REG(a3) struct TagItem *tagList);

VOID ASM LoseService(REG(a0) APTR svcEntity);

VOID ASM FlushLib(VOID);

/*****************************************************************************/

#endif /* SERVICES_MANAGER */

kprintf(STRPTR,...);
sprintf(STRPTR,...);

#endif /* SERVICESBASE_H */
