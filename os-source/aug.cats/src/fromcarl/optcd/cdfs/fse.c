/***********************************************************************
****************                                        ****************
****************        -=  CDTV FILE SYSTEM  =-        ****************
****************                                        ****************
************************************************************************
***                                                                  ***
***   Written by Carl Sassenrath for Commodore International, Ltd.   ***
*** Copyright (C) 1992 Commodore Electronics Ltd. All rights reserved***
***                                                                  ***
***         Confidential and Proprietary System Source Code          ***
***                                                                  ***
***********************************************************************/
/*
 * FSE.c:	FileSystem Events manager.  Available so that OptCD
 *		can monitor CD access and layout a more optimal file
 *		order.
 *
 * Leo L. Schwab					2123
 * Kenneth Yeast					2708 - 2721
 */
#include "main.h"
#include "fse.h"

extern void	InitSemaphore();
extern ULONG	AttemptSemaphore();
extern void	ReleaseSemaphore();
extern void	*Allocate();
extern void	Deallocate();
extern void	*GetMsg();
extern void	*FindTask();

/***********************************************************************
************************************************************************
***
***	Internal routines.
***
************************************************************************
***********************************************************************/

#define	ARG_SIZE	sizeof( ULONG )


/***********************************************************************
***
***  SetupFSE
***
***	Internally initialize the FileSystem Events system.
***
***********************************************************************/

void SetupFSE()
	{
	InitSemaphore( &FSELock );
	FSECollect = FALSE;
	}


/***********************************************************************
***
***  AllocFSE
***
***	Procure a FileSystem Event.
***
***********************************************************************/

struct Message *AllocFSE( Size )
REG ULONG Size;
	{
	REG struct Message *Msg;

	Size += sizeof( struct Message );

	/* force up to 8 byte boundary */
	Size = ( ( Size + 7 ) & ~7 );

	Forbid();
	for ( ;; )
		{
		/*
		 * Try and get one from the pool.
		 */
		if ( Msg = Allocate( &FSEPool, Size ) )
			{
			Msg->mn_Length = Size;
			break;
			}

		/*
		 * Pool empty; attempt to reclaim a message.
		 */
		if ( Msg = GetMsg( &FSEPort ) )
			{
			/*
			 * Ooo!  Just what we need.
			 */
			if ( Msg->mn_Length == Size )
				break;
			/*
			 * Wrong size.  Pour back into pool and try again.
			 */
			Deallocate( &FSEPool, Msg, (LONG) Msg->mn_Length );
			}
		}
	Permit();

	Msg->mn_Node.ln_Type = NT_MESSAGE;

	return( Msg );
	}


/***********************************************************************
***
***  FSE
***
***	Transmit a FileSystemEvent.
***
***********************************************************************/

void FSE( FSEType )		/*  varargs  */
LONG FSEType;
	{
	REG struct Message *Msg;
	REG LONG	   Size;
	REG UBYTE	   *MsgData;
	REG ULONG	   *ArgPtr;

	/* check if we are monitoring and if it's a type we are monitoring */
	if ( ( ! FSECollect ) || ( ! ( FSEType & FSEMonitor ) ) )
		return;

	ArgPtr = ( (ULONG *) &FSEType + 1 );

	/* Determine data lengths to be put into message */
	switch ( FSEType )
		{
		case ( FSEF_PACKETS ):
			Size = ( ARG_SIZE * 4 );
			break;

		case ( FSEF_FILE ):
		case ( FSEF_LOW_READ ):
			Size = ( ARG_SIZE * 2 );
			break;

		case ( FSEF_DIR_NOCACHE ):
		case ( FSEF_DIR_CACHE ):
		case ( FSEF_BLOCK_NOCACHE ):
		case ( FSEF_BLOCK_CACHE ):
		default:
			Size = ARG_SIZE;
			break;
		}

	/* get a message from the pool */
	if ( ! ( Msg = AllocFSE( Size + sizeof( UWORD ) ) ) )
		return;

	/* Store type */
	MsgData = (UBYTE *) ( Msg + 1 );
	*( (UWORD *) MsgData ) = (UWORD) FSEType;
	MsgData += sizeof( UWORD );

	/* Store data */
	CopyMem( ArgPtr, MsgData, Size );

	/* Make it available on the port */
	PutMsg( &FSEPort, Msg );
	}


/***********************************************************************
************************************************************************
***
***	Client Routines.
***
************************************************************************
***********************************************************************/


/***********************************************************************
***
***  ReplyFSE
***
***	Return an FSE to the free pool.
***
***********************************************************************/

void ReplyFSE( Msg )
struct Message	*Msg;
	{
	Forbid();
		Deallocate( &FSEPool, Msg, (LONG) Msg->mn_Length );
	Permit();
	}


/***********************************************************************
***
***  InitFSE
***
***	Procure the use of the FSE port.  Fails if the port is already
***	in use.  Pass it the memory to be used and the length.  Remember,
***	Allocate REQUIRES the memory it uses to be block aligned.
***
***********************************************************************/

struct MsgPort *InitFSE( Mem, Length, Monitor )
UBYTE *Mem;
ULONG Length;
ULONG Monitor;
	{
	struct MemChunk	*FirstMemChunk;

	/* Simple proofing */
	if ( ( ! Mem ) || ( Length == 0 ) )
		return( NULL );

	if ( ! AttemptSemaphore( &FSELock ) )
		return( NULL );

	/* Check for 8 byte alignment, try to fit even if it isn't */
	if ( (ULONG) Mem & 7 )
		{
		Debug0( "FSE Memory is not aligned: 0x%8lx\n", Mem );

		Mem = (UBYTE *) ( ( (ULONG) Mem + 7 ) & ~7 );
		Length -= 8;
		}

	FirstMemChunk = (struct MemChunk *) Mem;

	FSEPool.mh_Node.ln_Type = NT_MEMORY;
	FSEPool.mh_First	= (struct MemChunk *) Mem;
	FSEPool.mh_Lower	= (APTR) FirstMemChunk;
	FSEPool.mh_Upper	= (APTR) ( Length + (ULONG) FirstMemChunk );
	FSEPool.mh_Free		= Length;

	/*  Set up first chunk in the freelist */
	FirstMemChunk->mc_Next	= NULL;
	FirstMemChunk->mc_Bytes	= Length;

	FSEPort.mp_Node.ln_Type	= NT_MSGPORT;
	FSEPort.mp_Flags	= PA_IGNORE;
	NewList( &FSEPort.mp_MsgList );

	if ( ( FSEPort.mp_SigBit = AllocSignal( -1 ) ) == -1 )
		goto Failed;

	FSEPort.mp_SigTask = FindTask( NULL );
	FSEPort.mp_Flags   = PA_SIGNAL;
	FSEMonitor	   = Monitor;
	FSECollect	   = FALSE;

	return( &FSEPort );

Failed:
	ReleaseSemaphore( &FSELock );
	return( NULL );
	}


/***********************************************************************
***
***  QuitFSE
***
***	Release the use of the FSE system.  Defends against non-owners
***	trying to release it.
***
***********************************************************************/

void QuitFSE()
	{
	/*
	 * Check to see if the guy trying to release it actually has the
	 * right to do so.
	 */
	if ( AttemptSemaphore( &FSELock ) )
		{
		FSECollect = FALSE;
		FSEMonitor = 0;

		/* what about pending messages? */
		Forbid();
			FSEPort.mp_Flags   = PA_IGNORE;
			FSEPort.mp_SigTask = NULL;
		Permit();

		if ( FSEPort.mp_SigBit != -1 )
			FreeSignal( FSEPort.mp_SigBit ), FSEPort.mp_SigBit = -1;

		/*
		 * Yup, it's his.  Release our attempt, then release
		 * the original Semaphore.  A little hokey, but hey,
		 * shock the monkey.
		 */
		ReleaseSemaphore( &FSELock );
		ReleaseSemaphore( &FSELock );
		}
	}
