#ifndef JANUS_JFUNCS_H
#define JANUS_JFUNCS_H
/***************************************************************************
 * (PC side file)
 *
 * JFuncs.h - Include file for janus function type checking
 *
 * 06-07-89 - Bill Koester - Created this file
 *
 ***************************************************************************/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef LINT_ARGS
UBYTE	AddService();
APTR	AllocJanusMem();
APTR  AllocJRemember();
APTR  AllocServiceMem();
VOID  AttachJRemember();
UBYTE	CallAmiga();
UBYTE	CallService();
UBYTE	CheckAmiga();
UBYTE	DeleteService();
UBYTE	FreeJanusMem();
VOID  FreeJRemember();
VOID	FreeServiceMem();
UBYTE	GetBase();
UBYTE	GetService();
int	j_file_transfer();
VOID	JanusInitLock();
VOID	JanusLock();
BOOL	JanusLockAttempt();
VOID	JanusUnlock();
UBYTE	LockServiceData();
UBYTE	ReleaseService();
UBYTE	SetParam();
UBYTE	UnLockServiceData();
UBYTE	WaitAmiga();
VOID	WaitService();
#else
UBYTE	AddService(struct ServiceData **,ULONG,USHORT,USHORT,USHORT,ULONG,USHORT);
APTR	AllocJanusMem(ULONG,ULONG);
APTR  AllocJRemember(RPTR *,USHORT,USHORT);
APTR  AllocServiceMem(struct ServiceData *,USHORT,USHORT);
VOID  AttachJRemember(RPTR *,RPTR *);
UBYTE	CallAmiga(UBYTE);
UBYTE	CallService(struct ServiceData *);
UBYTE	CheckAmiga(UBYTE);
UBYTE	DeleteService(struct ServiceData *);
UBYTE	FreeJanusMem(APTR);
VOID  FreeJRemember(RPTR *,BOOL);
VOID  FreeServiceMem(struct ServiceData *,APTR);
UBYTE	GetBase(UBYTE,UWORD *,UWORD *,UWORD *);
UBYTE	GetService(struct ServiceData **,ULONG,UWORD,ULONG,UWORD);
int	j_file_transfer(char *,char *,int,int,unsigned char *,int);
VOID	JanusInitLock(UBYTE *);
VOID	JanusLock(UBYTE *);
BOOL	JanusLockAttempt(UBYTE *);
VOID	JanusUnlock(UBYTE *);
UBYTE	LockServiceData(struct ServiceData *);
UBYTE	ReleaseService(struct ServiceData *);
UBYTE	SetParam(UBYTE,UWORD);
UBYTE	UnlockServiceData(struct ServiceData *);
UBYTE	WaitAmiga(UBYTE);
VOID	WaitService(struct ServiceData *);
#endif

#endif
