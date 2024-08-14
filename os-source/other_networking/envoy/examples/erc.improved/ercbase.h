#ifndef ERCBASE_H
#define ERCBASE_H
/*
**  ERC Service Common Header.  Includes, library base, prototypes, etc.
*/
#include <exec/exec.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#include <utility/tagitem.h>
#include <clib/utility_protos.h>
#include <pragmas/utility_pragmas.h>
#include <dos/dos.h>
#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>
#include <envoy/nipc.h>
#include <clib/nipc_protos.h>
#include <pragmas/nipc_pragmas.h>
#include <envoy/services.h>
#include <envoy/errors.h>
#include <string.h>
#include <clib/alib_protos.h>
/*
**  Library base and global variables
*/
struct ERCSvc
{
    struct Library           ERC_Lib;
    struct SignalSemaphore   ERC_OpenLock;
    BPTR                     ERC_SegList;
    struct Library          *ERC_DOSBase;
    struct Library          *ERC_NIPCBase;
    struct ExecBase         *ERC_SysBase;
    struct Library          *ERC_UtilityBase;
    struct Entity           *ERC_Entity;
    ULONG		     ERC_Status;
    ULONG                    ERC_SigMask;
    struct Task             *ERC_LibProc;
    struct Task             *ERC_SpawnedProc;
    ULONG 		     ERC_ResetSig;
};
#define SysBase       (ERCBase->ERC_SysBase)
#define DOSBase       (ERCBase->ERC_DOSBase)
#define UtilityBase   (ERCBase->ERC_UtilityBase)
#define NIPCBase      (ERCBase->ERC_NIPCBase)
#define Entity	      (ERCBase->ERC_Entity)
#define Status	      (ERCBase->ERC_Status)
#define SigMask	      (ERCBase->ERC_SigMask)
#define LibProc       (ERCBase->ERC_LibProc)
#define SpawnedProc   (ERCBase->ERC_SpawnedProc)
#define ResetSig      (ERCBase->ERC_ResetSig)
/*
**  Prototypes
*/
#define LIBENT        __asm  __saveds
ULONG LIBENT StartService(register __a0 STRPTR userName,
                          register __a1 STRPTR passWord,
                          register __a2 STRPTR entName,
                          register __a6 struct ERCSvc *ERCBase);
VOID __saveds Server(VOID);
VOID LIBENT FlushLib(VOID);
VOID __asm docmd(register __a0 struct Transaction *trans, 
		 register __a6 struct ERCSvc *ERCBase);
#endif /* ERCBASE */
