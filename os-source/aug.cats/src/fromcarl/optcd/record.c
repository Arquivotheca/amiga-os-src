/************************************************************************
**********                                                     **********
**********                     CDTV Tools                      **********
**********                     ----------                      **********
**********                                                     **********
**********           Copyright (C) Pantaray Inc. 1992          **********
**********               Ukiah, CA  707-462-4878               **********
**********                All Rights Reserved.                 **********
**********                                                     **********
*************************************************************************
***
***  MODULE:
***
***	Record.c
***
***  PURPOSE:
***
***	Record FSE events to file or screen.
***
***  QUOTE:
***
***  HISTORY:
***
***	0.01 2717 Ken Yeast	Moved here from OptCD.c
***	0.02 2720 Ken Yeast	Cleaned up, file output only
***	0.03 2721 Ken Yeast	Changed operation of FSE
***	0.04 2728 Ken Yeast	InitAbortWin using FontReq
***	0.05 2802 Ken Yeast	Changed to be CLI based
***	0.06 2805 Ken Yeast	Small optimizations
***				Reports code instead of flag
***
************************************************************************/


/***********************************************************************
***
***  Functions
***
***	HandleFSE
***	InitRecord
***	Record
***	QuitRecord
***
************************************************************************/


#include <work/Standard.h>
#include <work/System.h>

#include <exec/memory.h>

#include <string.h>
#include <dos.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include <work/Support.h>
#include <work/Core.h>
#include <work/Dos.h>

#include "OptCD.h"
#include <work/fse.h>
#include "cdfs_protos.h"
#include "cdfs_pragmas.h"


/***********************************************************************
***
***  Definitions
***
***********************************************************************/

#define	CDFS_VERSION		26


/***********************************************************************
***
***  Function Prototypes
***
***********************************************************************/

PRIVATE STATUS	_ASM HandleFSE( _A0 struct MsgPort * );


/***********************************************************************
***
***  Global Variables
***
***********************************************************************/

PUBLIC struct Library	*CDFSBase	= NULL;


/***********************************************************************
***
***  Module Variables
***
***********************************************************************/

PRIVATE FILE	*StatFile		= NULL;


/***********************************************************************
***
***  HandleFSE
***
***********************************************************************/

PRIVATE STATUS _ASM HandleFSE(
	_A0 struct MsgPort *FSEPort
	)
	{
	PRIVATE LONG Args[ 5 ];
	REG struct Message *Msg;
	REG UWORD *Data;
	REG UWORD FSEType;
	REG UWORD FSECode;
	CSTR Err = NULL;

	while ( Msg = GetMsg( FSEPort ) )
		{
		// Get info from message and reply
		Data = (UWORD *) ( Msg + 1 );
		FSEType = *Data;
		CopyMem( ( Data + 1 ), Args, ( 4 * sizeof( ULONG ) ) );
		ReplyFSE( Msg );

		switch ( FSEType )
			{
			case ( FSEF_PACKETS ):
				FSECode = FSEB_PACKETS;
				break;

			case ( FSEF_DIR_NOCACHE ):
				FSECode = FSEB_DIR_NOCACHE;
				break;

			case ( FSEF_DIR_CACHE ):
				FSECode = FSEB_DIR_CACHE;
				break;

			case ( FSEF_BLOCK_NOCACHE ):
				FSECode = FSEB_BLOCK_NOCACHE;
				break;

			case ( FSEF_BLOCK_CACHE ):
				FSECode = FSEB_BLOCK_CACHE;
				break;

			case ( FSEF_FILE ):
				FSECode = FSEB_FILE;
				break;

			case ( FSEF_LOW_READ ):
				FSECode = FSEB_LOW_READ;
				break;

			default:
				ERR( Err, "Unrecognized message" );
			}

		switch ( FSECode )
			{
			case ( FSEB_PACKETS ):
				// Argx is useless in a file...
				if ( fprintf( StatFile,
					      "%ld %ld %08lx %08lx %08lx\n",
					      FSECode,
					      Args[ 0 ],
					      Args[ 1 ],
					      Args[ 2 ],
					      Args[ 3 ] ) < 31 )
					ERR( Err, "Failed to write statistics file" );
				break;

			case ( FSEB_DIR_NOCACHE ):
			case ( FSEB_DIR_CACHE ):
			case ( FSEB_BLOCK_NOCACHE ):
			case ( FSEB_BLOCK_CACHE ):
				if ( fprintf( StatFile,
					      "%ld %ld\n",
					      FSECode,
					      Args[ 0 ] ) < 4 )
					ERR( Err, "Failed to write statistics file" );
				break;

			case ( FSEB_FILE ):
			case ( FSEB_LOW_READ ):
				if ( fprintf( StatFile,
					      "%ld %ld %ld\n",
					      FSECode,
					      Args[ 0 ],
					      Args[ 1 ] ) < 6 )
					ERR( Err, "Failed to write statistics file" );
				break;
			}
		}

	return( OK );

Failed:
	if ( Err )
		PrintLine( Err );

	return( ERROR );
	}


/***********************************************************************
***
***  InitRecord
***
***********************************************************************/

void InitRecord(
	void
	)
	{
	CDFSBase = OpenLib( "cdfs.library", CDFS_VERSION );
	}


/***********************************************************************
***
***  Record
***
***********************************************************************/

void Record(
	CSTR StatFileName,
	ULONG FSEMonitor,
	ULONG BufferLength
	)
	{
	ULONG FSELength			= ( BufferLength * KILO );
	UBYTE *FSEMemory		= NULL;
	CSTR Err			= NULL;
	REG struct MsgPort *FSEPort	= NULL;
	LONG WaitBits;

	if ( ! ( StatFile = fopen( StatFileName, "a" ) ) )
		ERR( Err, "Cannot open Statistics file" );

	if ( ! ( FSEMemory = AllocMem( FSELength, MEMF_CLEAR ) ) )
		ERR( Err, "Not enough memory for messages" );

	if ( ! ( FSEPort = InitFSE( FSEMemory, FSELength, FSEMonitor ) ) )
		ERR( Err, "Cannot get FSE port" );

	WaitBits = ( ( 1L << FSEPort->mp_SigBit ) | SIGBREAKF_CTRL_C );

	Print( "Recording...\n" );

	// "Record"
	StartFSE();

	for ( EVER )
		{
		if ( Wait( WaitBits ) & SIGBREAKF_CTRL_C )
			break;

		if ( HandleFSE( FSEPort ) )
			break;
		}

	StopFSE();

	// Any remaining messages
	(void) HandleFSE( FSEPort );

Failed:
	if ( Err )
		PrintLine( Err );

	if ( FSEPort )
		QuitFSE();

	if ( FSEMemory )
		FreeMem( FSEMemory, FSELength );

	if ( StatFile )
		{
		fclose( StatFile ), StatFile = NULL;

		if ( SizeOfFile( StatFileName ) == 0 )
			DeleteFile( StatFileName );
		}
	}


/***********************************************************************
***
***  QuitRecord
***
***********************************************************************/

void QuitRecord(
	void
	)
	{
	CloseLib( CDFSBase );
	}
