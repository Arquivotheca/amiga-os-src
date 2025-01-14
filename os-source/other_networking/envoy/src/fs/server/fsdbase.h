#ifndef FSDBASE_H
#define FSDBASE_H

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

struct FSDSvc
{
    struct Library           FSD_Lib;
    struct Library          *FSD_DOSBase;
    struct Library          *FSD_NIPCBase;
    struct ExecBase         *FSD_SysBase;
    struct Library          *FSD_UtilityBase;
    struct Library          *FSD_IntuitionBase;
    struct Library          *FSD_IFFParseBase;
    struct Library          *FSD_AccountsBase;
    APTR                     FSD_Entity;
    BPTR                     FSD_SegList;
    struct List              FSD_Mounts;
    struct MinList           FSD_Current;
    struct MinList	     FSD_ExAllList;
    struct SignalSemaphore   FSD_CurrentLock;
    struct SignalSemaphore   FSD_OpenLock;
    UBYTE                    FSD_Mode;
    UBYTE                    FSD_Filler;
    UBYTE		     FSD_ExallBuffer[400];
};


#define ASM           __asm
#define REG(x)        register __ ## x

#define FSDBase       ((struct FSDSvc *)getreg(REG_A6))
#define SysBase       FSDBase->FSD_SysBase
#define DOSBase       FSDBase->FSD_DOSBase
#define UtilityBase   FSDBase->FSD_UtilityBase
#define NIPCBase      FSDBase->FSD_NIPCBase
#define IntuitionBase FSDBase->FSD_IntuitionBase
#define IFFParseBase  FSDBase->FSD_IFFParseBase
#define AccountsBase  FSDBase->FSD_AccountsBase
#define Mounts        FSDBase->FSD_Mounts
#define ExAllList     FSDBase->FSD_ExAllList
#define ExallBuffer   FSDBase->FSD_ExallBuffer

/*****************************************************************************/

ULONG ASM StartService(REG(a0) struct TagItem *t);

VOID ASM Server(REG(a0) STRPTR userName,
                REG(a1) STRPTR password,
                REG(a2) STRPTR entityName);

VOID ASM FlushLib(VOID);

/*****************************************************************************/

/* Etc. */
extern void NewList(struct List *l);
extern void kprintf(char *fmt, ...);

/* fs.c */

void KeepRecordLock(struct MountedFSInfo *m, BPTR fh, ULONG pos, ULONG len);
void NukeRecordLock(struct MountedFSInfo *m, BPTR fh, ULONG pos, ULONG len);
void KeepEtc(ULONG type, APTR data, struct MountedFSInfo *m);
void NukeEtc(APTR data, struct MountedFSInfo *m);
extern void KeepLock(APTR thelock, struct MountedFSInfo *m);
extern void KeepFH(APTR thefh, struct MountedFSInfo *m);
extern void NukeLock(APTR thelock, struct MountedFSInfo *m);
extern void NukeFH(APTR thefh, struct MountedFSInfo *m);
extern void __asm CleanupDeadMount(register __a0 struct MountedFSInfo *m);
extern BOOL BadStructure(UBYTE *);
BOOL LoadConfig(struct List *dlist);
ULONG PermFromLock(struct FileLock *l);
UWORD UIDFromLock(struct FileLock *l);
UWORD GIDFromLock(struct FileLock *l);
ULONG BestPermFromLockX(struct FileLock *l, struct MountedFSInfo *m, ULONG *flags, ULONG *map);
ULONG BestPermFromLock(struct FileLock *l, struct MountedFSInfo *m);
ULONG BestPermX(struct FileInfoBlock *f, struct MountedFSInfo *m, ULONG *flags);
ULONG BestPerm(struct FileInfoBlock *f, struct MountedFSInfo *m);

/* dopackets.c */
extern void DoDosPacket(struct Transaction *t, struct MountedFSInfo *mi);
extern BOOL GNameFromLock(ULONG lock, char * buffer, ULONG len);
extern void TackPathLock(struct FileLock *f,struct Transaction *t);
extern void TackPathFH(ULONG l, struct FileHandle *f, struct Transaction *t);

/* Defines for the Mode variable in the library base */
#define FSDMODE_NORMAL      0
#define FSDMODE_LOCKED      1



#endif /* FSDBASE */
