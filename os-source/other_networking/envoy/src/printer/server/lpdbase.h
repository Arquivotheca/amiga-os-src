#ifndef LPDBASE_H
#define LPDBASE_H

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
#include <dos.h>
#include <envoy/nipc.h>

/*****************************************************************************/

struct LPDSvc
{
    struct Library           LPD_Lib;
    struct Library          *LPD_DOSBase;
    struct Library          *LPD_NIPCBase;
    struct ExecBase         *LPD_SysBase;
    struct Library          *LPD_UtilityBase;
    struct Library          *LPD_AccountsBase;
    struct Library          *LPD_IFFParseBase;
    APTR                     LPD_Entity;
    BPTR                     LPD_SegList;

    /* Printer Daemon Stuff */
    struct SignalSemaphore   LPD_SpoolLock;
    struct MinList           LPD_Spool;
    struct SignalSemaphore   LPD_IncomingLock;
    struct MinList           LPD_Incoming;
    struct SignalSemaphore   LPD_OpenLock;
    struct List              LPD_Users;
};

#define ASM           __asm
#define REG(x)        register __ ## x

#define LPDBase       ((struct LPDSvc *)getreg(REG_A6))
#define SysBase       LPDBase->LPD_SysBase
#define DOSBase       LPDBase->LPD_DOSBase
#define UtilityBase   LPDBase->LPD_UtilityBase
#define NIPCBase      LPDBase->LPD_NIPCBase
#define AccountsBase  LPDBase->LPD_AccountsBase
#define IFFParseBase  LPDBase->LPD_IFFParseBase

/*****************************************************************************/

ULONG ASM StartService(REG(a0) struct TagItem *t);

VOID ASM Server(REG(a0) STRPTR userName,
                REG(a1) STRPTR password,
                REG(a2) STRPTR entityName);

VOID ASM FlushLib(VOID);

/*****************************************************************************/

kprintf(STRPTR,...);
sprintf(STRPTR,...);

#endif /* LPDBASE */