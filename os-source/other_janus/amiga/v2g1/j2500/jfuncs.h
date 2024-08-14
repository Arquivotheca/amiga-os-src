#ifndef JANUS_JFUNCS_H
#define JANUS_JFUNCS_H
/************************************************************************
 * (Amiga side file)
 *
 * JFuncs.h - Include file for janus function type checking
 *
 * 07-26-88 - Bill Koester - Created this file
 ************************************************************************/

#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef  JANUS_SERVICES_H
#include <janus/services.h>
#endif
#ifndef  JANUS_MEMORY_H
#include <janus/memory.h>
#endif
#ifndef  JANUS_JANUSBASE_H
#include <janus/janusbase.h>
#endif

/* === Service Functions Declarations ====================================== */
/* If you want the Service functions to be typed for you, include this file
 * If you want (and your language supports) automatic type-checking 
 * of arguments, define LINT_ARGS.
 */
#ifndef LINT_ARGS
SHORT    AddService();
APTR     AllocJanusMem();
APTR     AllocJRemember();
APTR     AllocServiceMem();
VOID     AttachJRemember();
VOID     CallService();
ULONG    CheckJanusInt();
VOID     CleanupJanusSig();
VOID     DeleteService();
VOID     FreeJanusMem();
VOID     FreeJRemember();
VOID     FreeServiceMem();
APTR     GetJanusStart();
SHORT    GetParamOffset();
SHORT    GetService();
VOID     JanusLock();
BOOL     JanusLockAttempt();
APTR     JanusMemBase();
USHORT   JanusMemToOffset();
ULONG    JanusMemType();
APTR     JanusOffsetToMem();
VOID     JanusUnlock();
VOID     JBCopy();
BYTE     *MakeBytePtr();
SHORT    *MakeWordPtr();
VOID     ReleaseService();
VOID     SendJanusInt();
ULONG    SetJanusEnable();
APTR     SetJanusHandler();
ULONG    SetJanusRequest();
USHORT   SetParamOffset();
struct SetupSig *SetupJanusSig();
APTR     TranslateJanusPtr();
#else
SHORT    AddService(struct ServiceData **,ULONG,USHORT,USHORT,USHORT,SHORT,USHORT);
APTR     AllocJanusMem(ULONG,ULONG);
APTR     AllocJRemember(struct JanusRemember **,USHORT,USHORT);
APTR     AllocServiceMem(struct ServiceData *,USHORT,USHORT);
VOID     AttachJRemember(struct JanusRemember **,struct JanusRemember **);
VOID     CallService(struct ServiceData *);
ULONG    CheckJanusInt(ULONG);
VOID     CleanupJanusSig(struct SetupSig *);
VOID     DeleteService(struct ServiceData *);
VOID     FreeJanusMem(APTR,ULONG);
VOID     FreeJRemember(struct JanusRemember **,BOOL);
VOID     FreeServiceMem(struct ServiceData *,APTR);
APTR     GetJanusStart();
SHORT    GetParamOffset(ULONG);
SHORT    GetService(struct ServiceData **,ULONG,USHORT,SHORT,USHORT);
VOID     JanusLock(UBYTE *);
BOOL     JanusLockAttempt(UBYTE *);
APTR     JanusMemBase(ULONG);
USHORT   JanusMemToOffset(APTR);
ULONG    JanusMemType(APTR);
APTR     JanusOffsetToMem(USHORT,USHORT);
VOID     JanusUnlock(UBYTE *);
VOID     JBCopy(APTR,APTR,ULONG);
BYTE     *MakeBytePtr(APTR);
SHORT    *MakeWordPtr(APTR);
VOID     ReleaseService(struct ServiceData *);
VOID     SendJanusInt(ULONG);
ULONG    SetJanusEnable(ULONG,ULONG);
APTR     SetJanusHandler(ULONG,APTR);
ULONG    SetJanusRequest(ULONG,ULONG);
USHORT   SetParamOffset(ULONG,USHORT);
struct SetupSig *SetupJanusSig(USHORT,USHORT,ULONG,ULONG);
APTR     TranslateJanusPtr(APTR,USHORT);
#endif


#endif
