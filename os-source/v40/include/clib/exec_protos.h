#ifndef  CLIB_EXEC_PROTOS_H
#define  CLIB_EXEC_PROTOS_H

/*
**	$Id: exec_protos.h,v 39.15 93/10/01 09:19:58 darren Exp $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef  EXEC_TASKS_H
#include <exec/tasks.h>
#endif
/*------ misc ---------------------------------------------------------*/
ULONG Supervisor( unsigned long (*userFunction)() );
/*------ special patchable hooks to internal exec activity ------------*/
/*------ module creation ----------------------------------------------*/
void InitCode( unsigned long startClass, unsigned long version );
void InitStruct( APTR initTable, APTR memory, unsigned long size );
struct Library *MakeLibrary( APTR funcInit, APTR structInit,
	unsigned long (*libInit)(), unsigned long dataSize,
	unsigned long segList );
void MakeFunctions( APTR target, APTR functionArray,
	unsigned long funcDispBase );
struct Resident *FindResident( UBYTE *name );
APTR InitResident( struct Resident *resident, unsigned long segList );
/*------ diagnostics --------------------------------------------------*/
void Alert( unsigned long alertNum );
void Debug( unsigned long flags );
/*------ interrupts ---------------------------------------------------*/
void Disable( void );
void Enable( void );
void Forbid( void );
void Permit( void );
ULONG SetSR( unsigned long newSR, unsigned long mask );
APTR SuperState( void );
void UserState( APTR sysStack );
struct Interrupt *SetIntVector( long intNumber, struct Interrupt *interrupt );
void AddIntServer( long intNumber, struct Interrupt *interrupt );
void RemIntServer( long intNumber, struct Interrupt *interrupt );
void Cause( struct Interrupt *interrupt );
/*------ memory allocation --------------------------------------------*/
APTR Allocate( struct MemHeader *freeList, unsigned long byteSize );
void Deallocate( struct MemHeader *freeList, APTR memoryBlock,
	unsigned long byteSize );
APTR AllocMem( unsigned long byteSize, unsigned long requirements );
APTR AllocAbs( unsigned long byteSize, APTR location );
void FreeMem( APTR memoryBlock, unsigned long byteSize );
ULONG AvailMem( unsigned long requirements );
struct MemList *AllocEntry( struct MemList *entry );
void FreeEntry( struct MemList *entry );
/*------ lists --------------------------------------------------------*/
void Insert( struct List *list, struct Node *node, struct Node *pred );
void AddHead( struct List *list, struct Node *node );
void AddTail( struct List *list, struct Node *node );
void Remove( struct Node *node );
struct Node *RemHead( struct List *list );
struct Node *RemTail( struct List *list );
void Enqueue( struct List *list, struct Node *node );
struct Node *FindName( struct List *list, UBYTE *name );
/*------ tasks --------------------------------------------------------*/
APTR AddTask( struct Task *task, APTR initPC, APTR finalPC );
void RemTask( struct Task *task );
struct Task *FindTask( UBYTE *name );
BYTE SetTaskPri( struct Task *task, long priority );
ULONG SetSignal( unsigned long newSignals, unsigned long signalSet );
ULONG SetExcept( unsigned long newSignals, unsigned long signalSet );
ULONG Wait( unsigned long signalSet );
void Signal( struct Task *task, unsigned long signalSet );
BYTE AllocSignal( long signalNum );
void FreeSignal( long signalNum );
LONG AllocTrap( long trapNum );
void FreeTrap( long trapNum );
/*------ messages -----------------------------------------------------*/
void AddPort( struct MsgPort *port );
void RemPort( struct MsgPort *port );
void PutMsg( struct MsgPort *port, struct Message *message );
struct Message *GetMsg( struct MsgPort *port );
void ReplyMsg( struct Message *message );
struct Message *WaitPort( struct MsgPort *port );
struct MsgPort *FindPort( UBYTE *name );
/*------ libraries ----------------------------------------------------*/
void AddLibrary( struct Library *library );
void RemLibrary( struct Library *library );
struct Library *OldOpenLibrary( UBYTE *libName );
void CloseLibrary( struct Library *library );
APTR SetFunction( struct Library *library, long funcOffset,
	unsigned long (*newFunction)() );
void SumLibrary( struct Library *library );
/*------ devices ------------------------------------------------------*/
void AddDevice( struct Device *device );
void RemDevice( struct Device *device );
BYTE OpenDevice( UBYTE *devName, unsigned long unit,
	struct IORequest *ioRequest, unsigned long flags );
void CloseDevice( struct IORequest *ioRequest );
BYTE DoIO( struct IORequest *ioRequest );
void SendIO( struct IORequest *ioRequest );
struct IORequest *CheckIO( struct IORequest *ioRequest );
BYTE WaitIO( struct IORequest *ioRequest );
void AbortIO( struct IORequest *ioRequest );
/*------ resources ----------------------------------------------------*/
void AddResource( APTR resource );
void RemResource( APTR resource );
APTR OpenResource( UBYTE *resName );
/*------ private diagnostic support -----------------------------------*/
/*------ misc ---------------------------------------------------------*/
APTR RawDoFmt( UBYTE *formatString, APTR dataStream, void (*putChProc)(),
	APTR putChData );
ULONG GetCC( void );
ULONG TypeOfMem( APTR address );
ULONG Procure( struct SignalSemaphore *sigSem,
	struct SemaphoreMessage *bidMsg );
void Vacate( struct SignalSemaphore *sigSem,
	struct SemaphoreMessage *bidMsg );
struct Library *OpenLibrary( UBYTE *libName, unsigned long version );
/*--- functions in V33 or higher (Release 1.2) ---*/
/*------ signal semaphores (note funny registers)----------------------*/
void InitSemaphore( struct SignalSemaphore *sigSem );
void ObtainSemaphore( struct SignalSemaphore *sigSem );
void ReleaseSemaphore( struct SignalSemaphore *sigSem );
ULONG AttemptSemaphore( struct SignalSemaphore *sigSem );
void ObtainSemaphoreList( struct List *sigSem );
void ReleaseSemaphoreList( struct List *sigSem );
struct SignalSemaphore *FindSemaphore( UBYTE *sigSem );
void AddSemaphore( struct SignalSemaphore *sigSem );
void RemSemaphore( struct SignalSemaphore *sigSem );
/*------ kickmem support ----------------------------------------------*/
ULONG SumKickData( void );
/*------ more memory support ------------------------------------------*/
void AddMemList( unsigned long size, unsigned long attributes, long pri,
	APTR base, UBYTE *name );
void CopyMem( APTR source, APTR dest, unsigned long size );
void CopyMemQuick( APTR source, APTR dest, unsigned long size );
/*------ cache --------------------------------------------------------*/
/*--- functions in V36 or higher (Release 2.0) ---*/
void CacheClearU( void );
void CacheClearE( APTR address, unsigned long length, unsigned long caches );
ULONG CacheControl( unsigned long cacheBits, unsigned long cacheMask );
/*------ misc ---------------------------------------------------------*/
APTR CreateIORequest( struct MsgPort *port, unsigned long size );
void DeleteIORequest( APTR iorequest );
struct MsgPort *CreateMsgPort( void );
void DeleteMsgPort( struct MsgPort *port );
void ObtainSemaphoreShared( struct SignalSemaphore *sigSem );
/*------ even more memory support -------------------------------------*/
APTR AllocVec( unsigned long byteSize, unsigned long requirements );
void FreeVec( APTR memoryBlock );
/*------ V39 Pool LVOs...*/
APTR CreatePool( unsigned long requirements, unsigned long puddleSize,
	unsigned long threshSize );
void DeletePool( APTR poolHeader );
APTR AllocPooled( APTR poolHeader, unsigned long memSize );
void FreePooled( APTR poolHeader, APTR memory, unsigned long memSize );
/*------ misc ---------------------------------------------------------*/
ULONG AttemptSemaphoreShared( struct SignalSemaphore *sigSem );
void ColdReboot( void );
void StackSwap( struct StackSwapStruct *newStack );
/*------ task trees ---------------------------------------------------*/
void ChildFree( APTR tid );
void ChildOrphan( APTR tid );
void ChildStatus( APTR tid );
void ChildWait( APTR tid );
/*------ future expansion ---------------------------------------------*/
APTR CachePreDMA( APTR address, ULONG *length, unsigned long flags );
void CachePostDMA( APTR address, ULONG *length, unsigned long flags );
/*------ New, for V39*/
/*--- functions in V39 or higher (Release 3) ---*/
/*------ Low memory handler functions*/
void AddMemHandler( struct Interrupt *memhand );
void RemMemHandler( struct Interrupt *memhand );
/*------ Function to attempt to obtain a Quick Interrupt Vector...*/
ULONG ObtainQuickVector( APTR interruptCode );
#endif   /* CLIB_EXEC_PROTOS_H */
