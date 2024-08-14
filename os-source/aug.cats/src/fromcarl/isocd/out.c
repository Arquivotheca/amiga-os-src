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
***	Out.c
***
***  PURPOSE:
***
***	Writes out ISO9660 image to file OR partition
***
***  HISTORY:
***
***	0.01 0308 C. Sassenrath	Created.
***	0.02 1630 C. Sassenrath	Updated.
***	0.03 2210 C. Sassenrath	Revisited.
***	0.04 2504 Ken Yeast	Changed to match current standards
***				(headers and other meaningless stuff)
***				Crammed in my coding style and ansi stuff
***				removed FindDevice (we already have it in
***				Support.lib:Dos.c)
***				Same with CopyBStr (Support.lib:Support.c)
***				(note new usage)
***				Removed IsDevice
***				Corrected check for SplitFile opening
***				Alloc/FreeIORequest needs changing
***	0.05 2507 Ken Yeast	Changing list methodology
***	0.06 2519 Ken Yeast	OpenImage
***				CloseImage
***				ImageName
***	0.07 2526 Ken Yeast	SplitFileName
***				Replaced ForceOut with AskUser
***	0.08 2528 Ken Yeast	Protection for Image > partition
***				NotADevice
***	0.09 2617 Ken Yeast	Check for free disk space for image file
***	0.10 2619 Ken Yeast	WriteOut now doesn't Error() out, returns ERROR
***	0.11 2623 Ken Yeast	Minor changes
***				removed Leo's AbortIO check in FreeIORequest
***	0.12 2917 Ken Yeast	Check for cdtv.device (read only)
***	0.13 2b02 Ken Yeast	Entering a volume that does not exist for image
***				was not being properly trapped.
***				OpenImage and OpenDevVol slightly re-written
***
************************************************************************/


/***********************************************************************
***
***  Functions
***
***	AllocIORequest
***	CloseImage
***	CloseSplit
***	FreeIORequest
***	InitOut
***	OpenDevVol
***	OpenImage
***	QuitOut
***	WriteOut
***
************************************************************************/


#include <work/standard.h>

#include <exec/io.h>
#include <exec/lists.h>
#include <dos/dos.h>
#include <dos/filehandler.h>
#include <dos/dosextens.h>

#include <string.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include <work/Core.h>
#include <work/Support.h>
#include <work/IO.h>
#include <work/Dos.h>
#include <work/List.h>
#include <work/Interface.h>
#include <cdtv/cdtv.h>

#include "ISOCD.h"


/***********************************************************************
***
***  External Dependencies
***
***********************************************************************/

IMPORT	BOOL	VerboseFlag;
IMPORT	LONG	BaseSector;
IMPORT	CHAR	ImageName[];
IMPORT	CHAR	SplitFileName[];

IMPORT LONG	CDSize;

IMPORT INTERFACE *Interface;

IMPORT FLAG	ImageIsDevice;


/***********************************************************************
***
***  Module Functions
***
************************************************************************/

PRIVATE void	*AllocIORequest( CSTR, ULONG, ULONG, ULONG );
PRIVATE void	FreeIORequest( void * );
PRIVATE STATUS	OpenDevVol( CSTR );


/***********************************************************************
***
***  Module Variables
***
************************************************************************/

PRIVATE LONG	OutFile			= 0;
PRIVATE LONG	SplitFile		= 0;
PRIVATE LONG	MinOffset		= 0;
PRIVATE LONG	MaxOffset		= 0;
PRIVATE LONG	OutOffset		= 0;

PRIVATE struct	IOStdReq *IOReq		= NULL;

PRIVATE UBYTE	TmpBuf[ SECTOR_SIZE ];


/***********************************************************************
***
***  AllocIORequest
***
***********************************************************************/

PRIVATE void *AllocIORequest(
	CSTR devname,
	ULONG unitNumber,
	ULONG flags,
	ULONG Size
	)
	{
	REG struct IORequest *r = NULL;
	REG struct MsgPort   *m;

	if ( ! ( m = CreatePort( NULL, NULL ) ) )
	 	return( NULL );

	if ( ! ( r = CreateExtIO( m, Size ) ) )
		{
		DeletePort( m );
		return( NULL );
		}

	if ( OpenDevice( devname, unitNumber, r, flags ) )
		{
		r->io_Device = NULL;
		FreeIORequest( r );
		return( NULL );
		}

	return( r );
	}


/***********************************************************************
***
***  CloseImage
***
***********************************************************************/

void CloseImage(
	void
	)
	{
	CloseSplit();

	if ( OutFile )
		{
		Close( OutFile ), OutFile = NULL;

		if ( SizeOfFile( ImageName ) == 0 )
			DeleteFile( ImageName );
		}
	}


/***********************************************************************
***
***  CloseSplit
***
***********************************************************************/

void CloseSplit(
	void
	)
	{
	if ( SplitFile )
		{
		Close( SplitFile ), SplitFile = NULL;

		if ( SizeOfFile( SplitFileName ) == 0 )
			DeleteFile( SplitFileName );
		}
	}


/***********************************************************************
***
***  FreeIORequest
***
***********************************************************************/

PRIVATE void FreeIORequest(
	void *IOReq
	)
	{
	REG struct IORequest *r;

	if ( ! ( r = IOReq ) )
		return;

	if ( r->io_Device )
		{
// NOTE: valid?
//		if ( r->io_Message.mn_Node.ln_Succ )
//			{
//			AbortIO( r );
//			WaitIO( r );
//			}

		CloseDevice( r );
		}

	if ( r->io_Message.mn_ReplyPort )
		DeletePort( r->io_Message.mn_ReplyPort );

	DeleteExtIO( r );
	}


/***********************************************************************
***
***  InitOut
***
***********************************************************************/

void InitOut(
	void
	)
	{
	}


/***********************************************************************
***
***  OpenDevVol
***
***********************************************************************/

PRIVATE STATUS OpenDevVol(
	CSTR devname
	)
	{
	struct	FileSysStartupMsg *fssm;
	struct	DosEnvec *de;
	LONG	cylsiz;
	LONG	blockbytes;
	char	name[ 80 ];
	CSTR	Err = NULL;

	// Check if it is available in Dos lists
	if ( ! ( fssm = FindDOSDev( devname ) ) )
		ERR( Err, "Device not available" );

	CopyBStr( fssm->fssm_Device, name );

	// Cannot write to a device controlled by cdtv.device (read only)
	if ( stricmp( name, CDTV_NAME ) == 0 )
		ERR( Err, "Read Only Volume" );

	if ( ! ( IOReq = AllocIORequest( name, fssm->fssm_Unit, fssm->fssm_Flags, sizeof( *IOReq ) ) ) )
		ERR( Err, "No IO available" );

	// Compute limits of device/partition.  Observe Reserved and PreAlloc blocks.
	// NOTE: this is also used in SimCD:Config.c, needs to be in a function
	de	   = BADDR( fssm->fssm_Environ );
	blockbytes = ( de->de_SizeBlock << 2 );
	cylsiz	   = ( blockbytes * de->de_Surfaces * de->de_BlocksPerTrack );
	MinOffset  = ( ( cylsiz * de->de_LowCyl ) + ( blockbytes * de->de_Reserved ) );
	MaxOffset  = ( ( cylsiz * ( de->de_HighCyl + 1 ) ) - ( blockbytes * de->de_PreAlloc ) - 1 );
	OutOffset  = MinOffset;

	if ( VerboseFlag )
		Print( "BlockBytes %ld CylSize %ld MinOffset %ld MaxOffset %ld\n",
			blockbytes, cylsiz, MinOffset, MaxOffset );

	// Check for a fit
	if ( ( CDSize * SECTOR_SIZE ) > ( MaxOffset - MinOffset ) )
		ERR( Err, "Partition is too small for image" );

	// Check for valid ISO vol before writing!
	if ( DoIOReq( IOReq, CMD_READ, ( OutOffset + ( 16 * SECTOR_SIZE ) ), SECTOR_SIZE, TmpBuf ) )
		ERR( Err, "SCSI read failed" );

	if ( ! CheckPVD( TmpBuf ) )
		if ( ! AskUser( Interface,
				"Overwrite your disk (%s)? (No previous ISO on it!!!)",
				"Overwrite!|Cancel", devname ) )
			goto Failed;

	if ( VerboseFlag )
		Print( "Output directed to %s unit %ld\n", name, fssm->fssm_Unit );

	return( OK );

Failed:
	if ( IOReq )
		FreeIORequest( IOReq ), IOReq = NULL;

	if ( Err )
		TellUser( Interface, Err );

	return( ERROR );
	}


/***********************************************************************
***
***  OpenImage
***
***********************************************************************/

STATUS OpenImage(
	void
	)
	{
	CSTR Err = NULL;

	// If it is a device, see if it's available, otherwise open as a file
	if ( ImageName[ ( strlen( ImageName ) - 1 ) ] == ':' )
		{
		ImageIsDevice = TRUE;
		return( OpenDevVol( ImageName ) );
		}

	ImageIsDevice = FALSE;

	// File exists?
	if ( SizeOfFile( ImageName ) )
		if ( ! AskUser( Interface, "%s exists, replace?", "Replace|Cancel", ImageName ) )
			goto Failed;

	// Is there disk space on volume?
	if ( FreeDiskSpace( ImageName ) < ( CDSize * SECTOR_SIZE ) )
		ERR( Err, "Not enough free disk space!" );

	// Open files
	if ( ! ( OutFile = Open( ImageName, MODE_NEWFILE ) ) )
		ERR( Err, "Could not open file" );

	if ( SplitFileName[ 0 ] )
		if ( ! ( SplitFile = Open( SplitFileName, MODE_NEWFILE ) ) )
			ERR( Err, "Could not open Split file" );

	return( OK );

Failed:
	if ( Err )
		TellUser( Interface, Err );

	return( ERROR );
	}

/***********************************************************************
***
***  QuitOut
***
***********************************************************************/

void QuitOut(
	void
	)
	{
	CloseImage();

	if ( IOReq )
		FreeIORequest( IOReq ), IOReq = NULL;
	}


/***********************************************************************
***
***  WriteOut
***
***********************************************************************/

STATUS WriteOut(
	CSTR Buffer,
	LONG Size
	)
	{
	CSTR Err = NULL;

	if ( Size == 0 )
		return( OK );

	if ( Size & 0x3ff )
		ERR( Err, "Non-aligned write error" );

	if ( OutFile )
		{
		if ( Write( OutFile, Buffer, Size ) != Size )
			ERR( Err, "Error in writing output file" );

		if ( SplitFile )
			if ( Write( SplitFile, Buffer, Size ) != Size )
				ERR( Err, "Error in writing split file" );
		}
	else
		{
		if ( ( OutOffset + Size ) > MaxOffset )
			ERR( Err, "Output device is full" );

		if ( VerboseFlag )
			Print( "SCSI Write %ld for %ld\n", OutOffset, Size );

		if ( DoIOReq( IOReq, CMD_WRITE, OutOffset, Size, Buffer ) )
			ERR( Err, "SCSI write failed" );

		OutOffset += Size;
		}

	return( OK );

Failed:
	if ( Err )
		TellUser( Interface, Err );

	return( ERROR );
	}
