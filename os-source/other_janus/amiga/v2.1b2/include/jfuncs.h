#ifndef JANUS_JFUNCS_H
#define JANUS_JFUNCS_H
/************************************************************************
 * (Amiga side file)
 *
 * JFuncs.h - Include file for janus function type checking
 *
 * 07-26-88 - Bill Koester - Created this file
 * 05-16-89 - Bill Koester - Fixed GetJanusStart prototype to GSS(void)
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
VOID     JanusInitLock();
VOID     JanusLock();
BOOL     JanusLockAttempt();
APTR     JanusMemBase();
USHORT   JanusMemToOffset();
ULONG    JanusMemType();
APTR     JanusOffsetToMem();
VOID     JanusUnlock();
VOID     JBCopy();
VOID     LockServiceData();
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
VOID     UnlockServiceData();
#else
SHORT    AddService(struct ServiceData **,ULONG,USHORT,USHORT,USHORT,SHORT,USHORT);
APTR     AllocJanusMem(ULONG,ULONG);
APTR     AllocJRemember(RPTR *,USHORT,USHORT);
APTR     AllocServiceMem(struct ServiceData *,USHORT,USHORT);
VOID     AttachJRemember(RPTR *,RPTR *);
VOID     CallService(struct ServiceData *);
ULONG    CheckJanusInt(ULONG);
VOID     CleanupJanusSig(struct SetupSig *);
VOID     DeleteService(struct ServiceData *);
VOID     FreeJanusMem(APTR,ULONG);
VOID     FreeJRemember(RPTR *,BOOL);
VOID     FreeServiceMem(struct ServiceData *,APTR);
APTR     GetJanusStart(void);
SHORT    GetParamOffset(ULONG);
SHORT    GetService(struct ServiceData **,ULONG,USHORT,SHORT,USHORT);
VOID     JanusInitLock(UBYTE *);
VOID     JanusLock(UBYTE *);
BOOL     JanusLockAttempt(UBYTE *);
APTR     JanusMemBase(ULONG);
USHORT   JanusMemToOffset(APTR);
ULONG    JanusMemType(APTR);
APTR     JanusOffsetToMem(USHORT,USHORT);
VOID     JanusUnlock(UBYTE *);
VOID     JBCopy(APTR,APTR,ULONG);
VOID     LockServiceData(struct ServiceData *);
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
VOID     UnlockServiceData(struct ServiceData *);
#endif
#ifdef PRAGMAS
/*------ Janus Functions -----------------------------------------------*/
#pragma libcall JanusBase SetJanusHandler 1e 9002
#pragma libcall JanusBase SetJanusEnable 24 1002
#pragma libcall JanusBase SetJanusRequest 2a 1002
#pragma libcall JanusBase SendJanusInt 30 1
#pragma libcall JanusBase CheckJanusInt 36 1
#pragma libcall JanusBase AllocJanusMem 3c 1002
#pragma libcall JanusBase FreeJanusMem 42 902
#pragma libcall JanusBase JanusMemBase 48 1
#pragma libcall JanusBase JanusMemType 4e 1
#pragma libcall JanusBase JanusMemToOffset 54 1
#pragma libcall JanusBase GetParamOffset 5a 1
#pragma libcall JanusBase SetParamOffset 60 1002
#pragma libcall JanusBase GetJanusStart 66 0
#pragma libcall JanusBase SetupJanusSig 6c 321004
#pragma libcall JanusBase CleanupJanusSig 72 801
#pragma libcall JanusBase JanusLock 78 801
#pragma libcall JanusBase JanusUnlock 7e 801
#pragma libcall JanusBase JBCopy 84 9803
/*------ V2.0 additions ------------------------------------------------*/
/*pragma libcall JanusBase AddService 8a 43210807*/
/*pragma libcall JanusBase GetService 90 3210805*/
#pragma libcall JanusBase CallService 96 801
#pragma libcall JanusBase ReleaseService 9c 801
#pragma libcall JanusBase DeleteService a2 801
#pragma libcall JanusBase JanusOffsetToMem a8 1002
#pragma libcall JanusBase TranslateJanusPtr ae 802
#pragma libcall JanusBase MakeBytePtr b4 801
#pragma libcall JanusBase MakeWordPtr ba 801
#pragma libcall JanusBase AllocJRemember c0 10803
#pragma libcall JanusBase FreeJRemember c6 802
#pragma libcall JanusBase AttachJRemember cc 9802
#pragma libcall JanusBase AllocServiceMem d2 10803
#pragma libcall JanusBase FreeServiceMem d8 9802
#pragma libcall JanusBase JanusLockAttempt de 801
#pragma libcall JanusBase LockServiceData e4 801
#pragma libcall JanusBase UnlockServiceData ea 801
#pragma libcall JanusBase JanusInitLock f0 801
#endif
#endif
