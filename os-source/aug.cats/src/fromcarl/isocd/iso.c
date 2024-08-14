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
***	ISO.c
***
***  PURPOSE:
***
***	Logic for creating ISO9660 image.
***
***  HISTORY:
***
***	0.01 0308 C. Sassenrath	Created.
***	0.02 1630 C. Sassenrath	Updated.
***	0.03 2210 C. Sassenrath	Revisited.
***	0.04 2504 Ken Yeast	Changed to match current standards
***				(headers and other meaningless stuff)
***				Crammed in my coding style and ansi stuff
***	0.05 2505 Ken Yeast	DIRNODE changing
***	0.06 2507 Ken Yeast	Changing list methodology
***	0.07 2510 Ken Yeast	OK, now let's implement some of these
***				"advantages"
***				DeterminePTSize
***				CreateImage logic changed
***	0.08 2511 Ken Yeast	CreatePath,MakePath removed
***				CreatePathTable re-written
***				WritePVD
***				FindTM moved here
***				DeterminePositions
***				WriteEntries
***				Protection vs. file open on CTRL-C
***				MakePathName
***				WriteFiles removed
***				WriteDir re-written
***				PathTable now uses pointer to DirEntry
***	0.09 2512 Ken Yeast	Fixed SizeDir
***				FirstEntry may be NULL
***	0.10 2513 Ken Yeast	NumPVD
***				WriteHeader
***				Threw DrawProgress in
***	0.11 2514 Ken Yeast	ET_HEADERS
***				ET_EMPTY used for buffer at end
***	0.12 2518 Ken Yeast	SetISODate
***	0.13 2521 Ken Yeast	Preserve RootNode string
***				Ability to abort building
***				Finish
***	0.14 2526 Ken Yeast	ISO Options added
***	0.15 2527 Ken Yeast	Usage of FlushFile changed slightly
***				(for more accurate Progress Bar)
***				SetCDFSOptions
***				FindSpecials removed
***				SetRootName
***				UnSetRootName
***	0.16 2528 Ken Yeast	Rootdir gets date
***				Parent dir ".." is now correct
***				SetISODirRecord
***				SetISOPVDDate
***				PrepTitle
***				TotalBytes was not being reset
***				FileBufSize
***	0.17 2529 Ken Yeast	Progress bar moved to within AbortWin
***				Changed IDPrep
***	0.18 2608 Ken Yeast	VerifyImage
***	0.19 2609 Ken Yeast	Fixed bug in SetISODate (day not always correct)
***	0.20 2618 Ken Yeast	ISO dir deep check for LoadFile (w/MakePathName)
***	0.21 2619 Ken Yeast	Shows bytes written
***				Handles write errors
***	0.22 2623 Ken Yeast	Dates are all correct - remember that CDFS is wrong!
***				FlushFile before Write:Empty! (last files would be corrupt)
***				SetRootPath, optimization
***	0.23 2625 Ken Yeast	Fix to handle zero length file (WriteFile/LocateEntries
***				Thanks to Chris Henry and his unbelievable help!
***	     2626 Ken Yeast	Option in SetISODate to (sortof) fix CDFS bug
***	0.24 2727 Ken Yeast	Provide all corresponding Intel numbers
***				IntelLong
***				IntelWord
***				AddAltPathTable for reverse byte systems
***				Version #'s (";1") for file names
***	0.25 2728 Ken Yeast	InitAbortWin using FontReq, also FlushFile
***				ShowProgress
***				InitShowProgress
***	0.26 2819 Ken Yeast	Progress stuff changed to handle as gadget
***				Also bytes text
***				(interface changes)
***				InitOurAbort
***	0.27 2821 Ken Yeast	InitInterface changed (Tags)
***	0.28 2917 Ken Yeast	Fixed bug in CreateImage (if exit while create)
***	0.29 2a02 Ken Yeast	CopyStrUp was not upper casing international
***				characters properly (signed vs. unsigned)
***
************************************************************************/


/***********************************************************************
***
***  Functions
***
***	CheckPVD
***	CopyStrUp
***	CreateImage
***	CreatePathTable
***	DeterminePositions
***	DeterminePTSize
***	Finish
***	FlushFile
***	InitISO
***	InitOurAbort
***	InitShowProgress
***	IntelLong
***	IntelWord
***	LoadStr
***	LocateEntries
***	MakePathName
***	QuitISO
***	SetCDFSOptions
***	SetISODate
***	SetISODirRecord
***	SetISOPVDDate
***	SetRootName
***	SetRootPath
***	ShowProgress
***	SizeDir
***	UnSetRootName
***	VerifyImage
***	WriteDir
***	WriteEntries
***	WriteFile
***	WriteHeader
***	WritePVD
***	WriteTVD
***
************************************************************************/


#include <work/standard.h>

#include <exec/lists.h>
#include <dos/dos.h>
#include <dos/dosextens.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>

#include <string.h>

#include <work/Support.h>
#include <work/Core.h>
#include <work/Memory.h>
#include <work/Interface.h>
#include <work/Custom.h>
#include <work/Dos.h>

#include <string.h>

#include "ISOCD.h"
#include "iso.h"


/***********************************************************************
***
***  External Dependencies
***
***********************************************************************/

IMPORT INTERFACE *Interface;

IMPORT CSTR	PrepTitle;
IMPORT BOOL	VerboseFlag;
IMPORT LONG	TotalBytes;
IMPORT LONG	CDSize;

// ISO 9660 Compatibility Options
IMPORT FLAG	AddAltPathTable;
IMPORT FLAG	AddVersion;
IMPORT FLAG	UpperCaseNames;

IMPORT CHAR	ImageName[];
IMPORT LONG	FileBufSize;

IMPORT STATE	TooDeepForISO;

IMPORT STATE	FixCDFSDateBug;

// ISO
IMPORT LONG	NumPVD;
IMPORT LONG	BaseSector;
IMPORT CHAR	IDVol[];
IMPORT CHAR	IDSet[];
IMPORT CHAR	IDPub[];
IMPORT CHAR	IDPrep[];
IMPORT CHAR	IDApp[];

// CDFS
IMPORT LONG	CacheData;
IMPORT LONG	CacheDir;
IMPORT LONG	PoolLock;
IMPORT LONG	PoolFile;
IMPORT LONG	Retries;
IMPORT LONG	DirectReadOn;
IMPORT LONG	FastSearch;
IMPORT LONG	TradeMark;
IMPORT LONG	Speed;
IMPORT DIRNODE	*TMFileNode;
IMPORT CHAR	TMFileName[];

IMPORT LONG	Dirs;
IMPORT PATHTABLE *PathTable;

IMPORT INTERFACE *Interface;


/***********************************************************************
***
***  Definitions
***
***********************************************************************/

#define	DIRFLAG_DIR		2

#define	OC( a, b )		( ( (a) << 8 ) + (b) )

#define	OPT_CACHE_DATA		OC( 'C', 'R' )
#define	OPT_CACHE_DIR		OC( 'C', 'D' )
#define	OPT_POOL_LOCK		OC( 'P', 'L' )
#define	OPT_POOL_FILE		OC( 'P', 'F' )
#define	OPT_RETRIES		OC( 'R', 'C' )
#define	OPT_DIRECT_READ		OC( 'D', 'R' )
#define	OPT_FAST_SEARCH		OC( 'F', 'S' )
#define	OPT_SPEED		OC( 'S', 'I' )
#define	OPT_TRADE_MARK		OC( 'T', 'M' )

#define	STORE_NUM( a, n, d, s )	if ( (n) != (d) )		\
					{			\
					if ( (n) < 1 )		\
						(n) = 1;	\
								\
					*(a)++ = (s);		\
					*(a)++ = 2;		\
					*(a)++ = (n);		\
					}

#define	STORE_BOOL( a, b, s )	if ( b )			\
					{			\
					*(a)++ = (s);		\
					*(a)++ = 0;		\
					}

// Gadget defines of our choosing, to be used as GadgetID's:
enum AGadgets
	{
	GAD_A_MSG,
	GAD_A_BYTES,
	GAD_A_PROGRESS,
	GAD_A_ABORT,
	MAX_A_GAD
	};


/***********************************************************************
***
***  Functions
***
************************************************************************/

PRIVATE void	CopyStrUp( UCHAR *, UCHAR *, LONG );
PRIVATE STATUS	CreatePathTable( FLAG );
PRIVATE void	DeterminePTSize( void );
PRIVATE void	Finish( void );
PRIVATE STATUS	FlushFile( void );
PRIVATE STATUS	InitOurAbort( INTERFACE * );
PRIVATE void	InitShowProgress( void );
PRIVATE ULONG	IntelLong( ULONG );
PRIVATE UWORD	IntelWord( UWORD );
PRIVATE void	LoadStr( CSTR, CSTR, LONG );
PRIVATE LONG	LocateEntries( MLIST *, LONG );
PRIVATE void	SetCDFSOptions( PVD * );
PRIVATE void	SetISODirRecord( DIREC *, DIRNODE *, UBYTE, UBYTE );
PRIVATE void	SetISOPVDDate( LONG, UBYTE * );
PRIVATE void	ShowProgress( void );
PRIVATE LONG	SizeDir( LONG );
PRIVATE STATUS	WriteDir( DIRNODE * );
PRIVATE STATUS	WriteEntries( MLIST *, CSTR );
PRIVATE STATUS	WriteFile( DIRNODE *, CSTR );
PRIVATE STATUS	WriteHeader( void );
PRIVATE STATUS	WritePVD( void );
PRIVATE STATUS	WriteTVD( void );


/***********************************************************************
***
***  Module Variables
***
************************************************************************/

PRIVATE LONG	NextBuf			= 0;
PRIVATE LONG	PathSector		= 0;
PRIVATE LONG	AltPathSector		= 0;
PRIVATE LONG	PathTableSize		= 0;
PRIVATE UBYTE	*FileBuf		= NULL;

PRIVATE LONG	LastPercent		= 0;
PRIVATE LONG	TotalSizeDiv		= 0;

PRIVATE UBYTE	DirBuf[ SECTOR_SIZE + 4 ];

PRIVATE LONG	PathEntryNum		= 0;
PRIVATE LONG	Level			= 0;
PRIVATE LONG	WritePaths		= 0;

PRIVATE STATE	Int_FixCDFSDateBug	= FALSE;

PRIVATE DIRNODE *RootNode		= NULL;

PRIVATE CSTR	OldRootStr		= NULL;

PRIVATE CSTR	RootPath		= NULL;
PRIVATE STATE	IsRootColon		= FALSE;

PRIVATE BPTR	FilePtr			= NULL;

// Abort
PRIVATE INTERFACE *AbortWin		= NULL;

PRIVATE UBYTE UpperChars[ 256 ] =
	{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 
	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 
	0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 
	0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 
	0x60, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 
	0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 
	0x58, 0x59, 0x5a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 
	0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, 
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 
	0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 
	0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 
	0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 
	0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 
	0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 
	0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 
	0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 
	0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 
	0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 
	0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 
	0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 
	0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xf7, 
	0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xff, 
	};


/***********************************************************************
***
***  CheckPVD
***
***********************************************************************/

FLAG CheckPVD(
	UBYTE *Buffer
	)
	{
	PVD *p = (PVD *) Buffer;

	if ( ( p->VolDescType == PRIM_VOL_DESC ) &&
	     ( strncmp( p->StdIdent, STD_IDENT, ( sizeof( STD_IDENT ) - 1 ) ) == 0 ) &&
	     ( p->VolDescVers == 1 ) )
		return( TRUE );
	else
		return( FALSE );
	}


/***********************************************************************
***
***  CopyStrUp
***
***********************************************************************/

PRIVATE void CopyStrUp(
	UCHAR *dest,
	UCHAR *src,
	LONG Length
	)
	{
	LONG i;

	SPAN( i,Length )
		*dest++ = UpperChars[ *src++ ];

	*src = NIL;
	}


/***********************************************************************
***
***  CreateImage
***
***	A) Determine positions of all entries, including PathTable
***	B) Write entries, which can be PathTable, File, Dir, etc.
***	   First are the headers.
***
***********************************************************************/

STATUS CreateImage(
	MLIST *MasterDirList,
	CSTR Path
	)
	{
	CSTR Err = NULL;

	TotalBytes = 0;

	// CDFS Date bug
	if ( FixCDFSDateBug )
		Int_FixCDFSDateBug = TRUE;

	// Open files
	if ( OpenImage() )
		goto Failed;

	// Requester for Abort
	if ( InitOurAbort( Interface ) )
		ERR( Err, "Cannot obtain requester" );

	// Everything must be located
	if ( DeterminePositions( MasterDirList ) )
		goto Failed;

	InitShowProgress();

	// WRITE
	if ( WriteEntries( MasterDirList, Path ) )
		goto Failed;

	DrawProgress( AbortWin, GAD_A_PROGRESS, 100 );

	Finish();
	return( OK );

Failed:
	if ( AbortWin )
		DrawProgress( AbortWin, GAD_A_PROGRESS, 0 );

	Finish();

	if ( Err )
		TellUser( Interface, Err );

	return( ERROR );
	}


/***********************************************************************
***
***  CreatePathTable
***
***********************************************************************/

PRIVATE STATUS CreatePathTable(
	FLAG StoreAlt
	)
	{
	REG PTE *p;
	REG DIRNODE *dn;
	LONG PathLength;
	LONG NameLength;
	REG LONG i;

	SPAN( i, Dirs )
		{
		if ( CheckAbortWin( AbortWin ) )
			return( ERROR );

		dn = PathTable[ i ].DirEntry;
		NameLength = strlen( dn->Next.ln_Name );
		PathLength = ( ( sizeof( PTE ) - 1 + NameLength ) & ~1 );

		p = (PTE *) ( FileBuf + NextBuf );
		p->LenDirIdent	= NameLength;

		if ( StoreAlt )
			{
			p->Extent	= IntelLong( dn->Position );
			p->ParentDirNum	= IntelWord( PathTable[ i ].Parent );
			}
		else
			{
			p->Extent	= dn->Position;
			p->ParentDirNum	= PathTable[ i ].Parent;
			}

		CopyStrUp( (UCHAR *) p->DirIdent, (UCHAR *) dn->Next.ln_Name, NameLength );

		NextBuf += PathLength;
		}

	NextBuf = ( ( NextBuf + SECTOR_SIZE - 1 ) & ~( SECTOR_SIZE - 1 ) );

	if ( FlushFile() )
		return( ERROR );

	if ( VerboseFlag )
		Print( "PathTableSize = %ld\n", PathTableSize );

	return( OK );
	}


/***********************************************************************
***
***  DeterminePositions
***
***	Image is laid out as follows:
***		BaseSector	
***		-		16 sectors of blank
***		PVD		1 sector/PVD (NumPVD)
***		...
***		TVD		1 sector
***		Entries		N sectors of PT's, DIR's, FILE's, EMPTY's,
***		...		etc.  Preferably with PT's and CDTV_TM in
***				the beginning.
***		PADDING		END_PAD sectors
***
***********************************************************************/

STATUS DeterminePositions(
	MLIST *MasterDirList
	)
	{
	LONG Base;
	LONG NextSector;

	// skip sys area & vol descriptors
	NextSector = Base = ( BaseSector + 16 + NumPVD + 1 );

	// Find RootNode before setting RootNode name
	if ( ! ( RootNode = FindFile( MasterDirList, ROOT_STR ) ) )
		{
		TellUser( Interface, "Damaged listing" );
		return( ERROR );
		}

	SetRootName();

	// Scan main list, determining positions
	// Will also determine PT positions
	if ( ( NextSector = LocateEntries( MasterDirList, NextSector ) ) == -1 )
		return( ERROR );

	// Total CD Image Size
	CDSize = ( NextSector - BaseSector );

	return( OK );
	}


/***********************************************************************
***
***  DeterminePTSize
***
***********************************************************************/

PRIVATE void DeterminePTSize(
	void
	)
	{
	LONG Offset = ( sizeof( PTE ) - 1 );
	LONG i;

	PathTableSize = 0;

	SPAN( i, Dirs )
		PathTableSize += ( ( Offset + strlen( PathTable[ i ].DirEntry->Next.ln_Name ) ) & ~1 );
	}


/***********************************************************************
***
***  Finish
***
***********************************************************************/

PRIVATE void Finish(
	void
	)
	{
	CloseImage();

	// Correct name of RootNode
	UnSetRootName();

	// Can use this function (Our AbortWin is close enough)
	QuitAbortWin( AbortWin ), AbortWin = NULL;

	Int_FixCDFSDateBug = FALSE;
	}


/***********************************************************************
***
***  FlushFile
***
***********************************************************************/

PRIVATE STATUS FlushFile(
	void
	)
	{
	// Write out file buffer and clear memory for re-use
	if ( WriteOut( FileBuf, NextBuf ) )
		return( ERROR );

	ClearMemory( (LONG *) FileBuf, FileBufSize );

	// Graph progress and note bytes written
	ShowProgress();

	NextBuf = 0;

	return( OK );
	}


/***********************************************************************
***
***  InitISO
***
***********************************************************************/

void InitISO(
	void
	)
	{
	// !!! update mem.c
	FileBuf = (CSTR) AllocSysMem( FileBufSize + 16 );
	ASSERT( FileBuf, "No memory for file buffer" );
	}


/***********************************************************************
***
***  InitOurAbort
***
***********************************************************************/

PRIVATE STATUS InitOurAbort(
	INTERFACE *Interface
	)
	{
	// Abort Window
	PRIVATE TAG	TagW_Abort[]		=
		{
		{ WA_Width,	320		},
		{ WA_Height,	80		},
		{ WA_IDCMP,	ABORT_IDCMP	},
		{ WA_Flags,	REQ_WFLAGS	},
		{ WA_Title,	(ULONG) "ISOCD"	},
		TAG_DONE
		};
	//	ID, Kind, X, Y, SizeX, SizeY, Flags, TitleText, ReqTags, ActionTags
	PRIVATE GAD_REQ AbortGadTable[]		=
		{
		// InActive
{ GAD_A_MSG,	G_TEXT,	       24, 16,  0, 0, ( RIGHT | HIGH ), "Building ISO Image:           bytes", NULL, NULL },
{ GAD_A_BYTES,	G_TEXT,	      184, 16,  0, 0, ( RIGHT | HIGH ), NULL, NULL, NULL },
{ GAD_A_PROGRESS,G_C_PROGRESS, 10, 30,300, 6, 0, NULL, NULL, NULL },

		// Active
{ GAD_A_ABORT,	G_BUTTON,     135, 50, 50, 4, 0, "Abort", NULL, NULL },
		};

	// Now ask for it
	if ( AbortWin = InitInterface( NULL, TagW_Abort, NULL, AbortGadTable, MAX_A_GAD, Interface->IFontReq, NULL ) )
		return( OK );
	else
		return( ERROR );
	}


/***********************************************************************
***
***  InitShowProgress
***
***********************************************************************/

PRIVATE void InitShowProgress(
	void
	)
	{
	// For Progress box
	LastPercent	= 0;
	TotalSizeDiv	= ( ( ( CDSize - NumPVD - 1 ) * SECTOR_SIZE ) / 100 );
	DrawProgress( AbortWin, GAD_A_PROGRESS, 0 );
	}


/***********************************************************************
***
***  IntelLong
***
***	Motorola Long (bytes):	1 2 3 4
***	Intel Long (bytes):	4 3 2 1
***
***********************************************************************/

PRIVATE ULONG IntelLong(
	ULONG MotorolaLong
	)
	{
	REG UBYTE *Ptr = (UBYTE *) &MotorolaLong;
	REG UBYTE Byte;

	// Swap outside bytes
	Byte		= *Ptr;
	*Ptr		= *( Ptr + 3 );
	*( Ptr + 3 )	= Byte;

	// Swap inside bytes
	Byte		= *( Ptr + 1 );
	*( Ptr + 1 )	= *( Ptr + 2 );
	*( Ptr + 2 )	= Byte;

	return( MotorolaLong );
	}


/***********************************************************************
***
***  IntelWord
***
***********************************************************************/

PRIVATE UWORD IntelWord(
	UWORD MotorolaWord
	)
	{
	REG UBYTE *Ptr = (UBYTE *) &MotorolaWord;
	REG UBYTE Byte;

	// Swap bytes
	Byte		= *Ptr;
	*Ptr		= *( Ptr + 1 );
	*( Ptr + 1 )	= Byte;

	return( MotorolaWord );
	}


/***********************************************************************
***
***  LoadStr
***
***********************************************************************/

PRIVATE void LoadStr(
	CSTR buf,
	CSTR name,
	LONG Length
	)
	{
	LONG i = 0;

	// check for overflow
	while ( *name )
		*buf++ = *name++, i++;

	// NOTE: what's this?
	while ( i < Length )
		*buf++, i++;
	}


/***********************************************************************
***
***  LocateEntries
***
***	Locate all files and dirs.
***
***********************************************************************/

PRIVATE LONG LocateEntries(
	MLIST *DList,
	LONG NextSector
	)
	{
	DIRNODE *dn;
	LONG Size;

	PathTableSize = 0;

	FOR_EACH_NODE( DList, dn )
		{
		// Can use this function (Our AbortWin is close enough)
		if ( CheckAbortWin( AbortWin ) )
			return( -1 );

		dn->Position = NextSector;

		switch ( IS_MASK( dn ) )
			{
			case ( ET_CDTV_TM ):
				if ( ( ! TMFileName[ 0 ] ) || ( ! TradeMark ) )
					break;

				//    \\\ FALL THROUGH ///

			case ( ET_FILE ):
			case ( ET_EMPTY ):
				// For now, empty files will waste one sector
				// to preserve "correctness"
				// Call Alan Purdle and check ISO-9660!
				// (SECTORS_USED could be changed to handle this...)
				if ( dn->Size == 0 )
					NextSector++;
				else
					NextSector += SECTORS_USED( dn->Size );
				break;

			case ( ET_DIR ):
				Size = SizeDir( dn->Extent );
				dn->Size = ( Size * SECTOR_SIZE );
				NextSector += Size;
				break;

			case ( ET_PATHTABLE ):
				// Only needs doing once if we have an Alt PathTable
				if ( ! PathTableSize )
					{
					DeterminePTSize();
					PathSector = NextSector;
					}
				else
					AltPathSector = NextSector;

				dn->Size = PathTableSize;
				NextSector += SECTORS_USED( dn->Size );
				break;

			case ( ET_HEADERS ):
				// Internal joke
				// This isn't really controlled by the user
				// We ALWAYS write headers first!
				break;
			}
		}

	return( NextSector );
	}


/***********************************************************************
***
***  MakePathName
***
***	
***		RootPath = "sys:"
***		IsColon = TRUE;
***		DNode->Next.ln_Name = "File"
***		// Remember (Offset - 1)
***
***			 PathTable[  0 ]<---------	DirEntry->Next.ln_Name <IGNORE>
***			 PathTable[  1 ].Parent	 |
***			 PathTable[  2 ].Parent	 |
***			 PathTable[  3 ].Parent	 |
***			 PathTable[  4 ].Parent	 |
***			 PathTable[  5 ].Parent	 |
***			 PathTable[  6 ].Parent<--	DirEntry->Next.ln_Name "prefs"
***			 PathTable[  7 ].Parent	 |
***			 PathTable[  8 ].Parent	 |
***			 PathTable[  9 ].Parent	 |
***			 PathTable[ 10 ].Parent	 |
***			 PathTable[ 11 ].Parent<--	DirEntry->Next.ln_Name "env-archive"
***			 PathTable[ 12 ].Parent	 |
***			 PathTable[ 13 ].Parent	 |
***	DNode->Extent--->PathTable[ 14 ].Parent--|	DirEntry->Next.ln_Name "sys"
***			 PathTable[ 15 ].Parent
***	<Etc.>
***
***		GIVES:
***			sys:prefs/env-archive/sys/File
***
***	if ( DNode == PathTable[ ( DNode->Extent - 1 ) ].DirEntry )
***		then only the PathName is needed (not the file)
***	(used by CheckList)
***
***********************************************************************/

void MakePathName(
	CSTR NewPath,
	DIRNODE *DNode
	)
	{
	// REMEMBER: this assumes there are never more that MAX_DEEP levels
	PRIVATE CSTR BuildList[ ( MAX_DEEP + 1 ) ];
	REG LONG Extent;
	REG LONG i;

	// Remember to set with SetRootPath!
	strcpy( NewPath, RootPath );
	if ( ! IsRootColon )
		strcat( NewPath, "/" );

	// Walk PathTable backwards
	Extent	= ( DNode->Extent - 1 );
	i	= 0;

	// If Dir, then backup one first
	if ( DNode == PathTable[ Extent ].DirEntry )
		Extent = ( PathTable[ Extent ].Parent - 1 );

	while ( Extent > 0 )
		{
		BuildList[ i++ ] = PathTable[ Extent ].DirEntry->Next.ln_Name;
		Extent = ( PathTable[ Extent ].Parent - 1 );
		}

	// Check ISO depth
	if ( i >= ISO_DEEP )
		TooDeepForISO = TRUE;

	// Add in path names
	if ( i > 0 )
		for ( i--; i > -1; i-- )
			{
			strcat( NewPath, BuildList[ i ] );
			strcat( NewPath, "/" );
			}

	// Now the actual file name
	strcat( NewPath, DNode->Next.ln_Name );
	}


/***********************************************************************
***
***  QuitISO
***
***********************************************************************/

void QuitISO(
	void
	)
	{
	// Can use this function (Our AbortWin is close enough)
	QuitAbortWin( AbortWin ), AbortWin = NULL;

	if ( FilePtr )
		Close( FilePtr );

	if ( FileBuf )
		FreeSysMem( FileBuf ), FileBuf = NULL;
	}


/***********************************************************************
***
***  SetCDFSOptions
***
***********************************************************************/

PRIVATE void SetCDFSOptions(
	PVD *p
	)
	{
	struct TMStruct *tm;
	UWORD *au = (UWORD *) ( &p->ApplicationUse[ 1 ] );

	STORE_NUM( au, CacheData, DEFAULT_CACHE_DATA, OPT_CACHE_DATA );
	STORE_NUM( au, CacheDir,  DEFAULT_CACHE_DIR,  OPT_CACHE_DIR  );
	STORE_NUM( au, PoolLock,  DEFAULT_POOL_LOCK,  OPT_POOL_LOCK  );
	STORE_NUM( au, PoolFile,  DEFAULT_POOL_FILE,  OPT_POOL_FILE  );
	STORE_NUM( au, Retries,   DEFAULT_RETRIES,    OPT_RETRIES    );
	STORE_BOOL( au, DirectReadOn,	OPT_DIRECT_READ	);
	STORE_BOOL( au, FastSearch,	OPT_FAST_SEARCH	);
	STORE_BOOL( au, Speed,		OPT_SPEED	);

	// Since TradeMark is oddball, I store it last.
	if ( TradeMark && TMFileNode && TMFileName[ 0 ] )
		{
		*au++ = OPT_TRADE_MARK;
		*au++ = sizeof( struct TMStruct );
		tm = (struct TMStruct *) au;
		tm->Size	 = TMFileNode->Size;
		tm->Sectors[ 0 ] = TMFileNode->Position;
		}
	}


/***********************************************************************
***
***  SetISODate
***
***	Date is # seconds since 1/1/1978
***	This algorithm goes through a few hoops getting the year in order
***	to avoid floating point math.  I am assuming this is faster.
***
***	REMEMBER: Not valid after 2400 A.D.  But then, ISO9660 fails after
***	2155 A.D.
***
***	NOTE: CDFS Bug:
***		Interprets date one day too much.  This correction handles
***		everything but leap days and the days before and after.
***		(also on non-leap years).  Too be honest, I'm rushed to
***		wrap this (2626) and cannot think of a better solution
***		right now.
***
***********************************************************************/

void SetISODate(
	LONG Date,
	UBYTE *RecTime
	)
	{
	PRIVATE UBYTE MonthDays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	REG LONG Years_1978;
	REG LONG Days;
	REG LONG Leaps;
	REG SHORT i;
	FLAG IsLeapYear;

	// CDFS Bug
	if ( Int_FixCDFSDateBug && ( Date > SECS_DAY ) )
		Date -= SECS_DAY;

	// Year = ( Date / ( 365.25 * SECS_YEAR ) );
	// Iterate once to remove the "0.25"'s (Leaps), then again for real year
	Years_1978 = ( Date / SECS_YEAR );

	// Remove all leap days in those years
	IsLeapYear = ( ( ( Years_1978 + 2 ) % 4 ) == 0 );
	Leaps = ( ( Years_1978 + 2 ) / 4 );

	// Don't remove this year's leap day (it may not actually be here!)
	if ( IsLeapYear )
		Leaps--;

	Date -= ( Leaps * SECS_DAY );

	// Now determine real year
	Years_1978 = ( Date / SECS_YEAR );

	IsLeapYear = ( ( ( Years_1978 + 2 ) % 4 ) == 0 );

	// Remove the years
	Date -= ( Years_1978 * SECS_YEAR );

	RecTime[ 0 ] = ( Years_1978 + 78 );

	Days = ( Date / SECS_DAY );

	// remove days
	Date = ( Date % SECS_DAY );

	if ( Date )
		Days++;

	if ( ! Days )
		Days++;

	// Month (1-12)
	RecTime[ 1 ] = 0;
	SPAN( i, 12 )
		{
		RecTime[ 1 ]++;
		if ( Days <= MonthDays[ i ] )
			break;
		else
			{
			Days -= MonthDays[ i ];

			// LY and Feb
			if ( IsLeapYear && ( i == 1 ) )
				if ( Days == 1 )
					{
					Days = 29;
					break;
					}
				else
					Days--;
			}
		}

	// Day (1-31)
	RecTime[ 2 ] = Days;

	// Hour (0-23)
	RecTime[ 3 ] = ( Date / SECS_HOUR );
	Date = ( Date % SECS_HOUR );

	// Minute (0-59)
	RecTime[ 4 ] = ( Date / 60 );

	// Seconds (0-59)
	RecTime[ 5 ] = ( Date % 60 );

	// GMT (Currently ignored by us)
	RecTime[ 6 ] = 0;
	}


/***********************************************************************
***
***  SetISODirRecord
***
***********************************************************************/

PRIVATE void SetISODirRecord(
	DIREC *dr,
	DIRNODE *dn,
	UBYTE RecSize,
	UBYTE LenID
	)
	{
	dr->LenDirRec	   = RecSize;

	if ( IS_DIR( dn ) )
		dr->FileFlags = DIRFLAG_DIR;

	dr->LenFileIdent   = LenID;
	dr->ExtentX	   = IntelLong( dn->Position );
	dr->Extent	   = dn->Position;
	dr->LenDataX	   = IntelLong( dn->Size );
	dr->LenData	   = dn->Size;

	SetISODate( dn->Date, dr->RecTime );
	}


/***********************************************************************
***
***  SetISOPVDDate
***
***	The Date in the PVD is formatted differently than the one
***	in the DIREC!  Stupid...
***
***********************************************************************/

PRIVATE void SetISOPVDDate(
	LONG Date,
	UBYTE *Buffer
	)
	{
	UBYTE RecTime[ 7 ];

	// Cheap way to interpret date
	SetISODate( Date, RecTime );

	// sprintf will put a NIL at Buffer[ 16 ], but that's OK
	sprintf( Buffer, "%4d%02d%02d%02d%02d%02d00",
		 ( 1900 + RecTime[ 0 ] ),
		 RecTime[ 1 ], RecTime[ 2 ], RecTime[ 3 ], 
		 RecTime[ 4 ], RecTime[ 5 ] );
	}


/***********************************************************************
***
***  SetRootName
***
***	ISO cannot swallow "<Root Dir>", (chuckle).
***
***********************************************************************/

void SetRootName(
	void
	)
	{
	if ( RootNode )
		{
		OldRootStr = RootNode->Next.ln_Name;
		RootNode->Next.ln_Name = "\001";
		}
	}


/***********************************************************************
***
***  SetRootPath
***
***********************************************************************/

void SetRootPath(
	CSTR NewRootPath
	)
	{
	RootPath    = NewRootPath;
	IsRootColon = ( RootPath[ strlen( RootPath ) - 1 ] == ':' );
	}


/***********************************************************************
***
***  ShowProgress
***
***********************************************************************/

PRIVATE void ShowProgress(
	void
	)
	{
	PRIVATE CHAR Buffer[ 16 ];
	REG LONG Percent;

	// Percentage bar
	TotalBytes += NextBuf;
	Percent = ( TotalBytes / TotalSizeDiv );
	if ( Percent > LastPercent )
		{
		DrawProgress( AbortWin, GAD_A_PROGRESS, Percent );
		LastPercent = Percent;
		}

	// Print bytes written
	sprintf( Buffer, "%9d", TotalBytes );
	ShowGadget( AbortWin, GAD_A_BYTES, Buffer );
	}


/***********************************************************************
***
***  SizeDir
***
***	Return the size in sectors required to store the directory.
***	Simply walks down each of the NextInDir's from the PathTable entry.
***	Now considers the version number garbage.
***
***********************************************************************/

PRIVATE INT SizeDir(
	LONG Extent
	)
	{
	DIRNODE *dn;
	INT Length;
	INT Size;
	INT Sectors = 0;

	// default entries (. ..)
	Size = ( 2 * DIREC_SIZE );

	// Path Table has start of file trail
	if ( dn = PathTable[ ( Extent - 1 ) ].FirstEntry )
		{
		// Walk list
		do
			{
			Length = ( ( DIREC_SIZE + strlen( dn->Next.ln_Name ) ) & ~1 );

			// Compensate for version ";1" if needed (stays even)
			if ( AddVersion && IS_FILE( dn ) )
				Length += 2;

			Size += Length;
			if ( Size >= SECTOR_SIZE )
				{
				Sectors++;

				// overflow full entry
				Size = ( ( Size > SECTOR_SIZE ) ? Length : 0 );
				}
			} while ( dn = dn->NextInDir );
		}

	// leftover
	if ( Size )
		Sectors++;

	return( Sectors );
	}


/***********************************************************************
***
***  UnSetRootName
***
***********************************************************************/

void UnSetRootName(
	void
	)
	{
	if ( OldRootStr )
		{
		if ( RootNode )
			RootNode->Next.ln_Name = OldRootStr;

		OldRootStr = NULL;
		}
	}


/***********************************************************************
***
***  VerifyImage
***
***	Will check a list against an ISO image partition/file
***
***********************************************************************/

STATUS VerifyImage(
	MLIST *MasterDirList,
	CSTR Path
	)
	{
	return( OK );
	}


/***********************************************************************
***
***  WriteDir
***
***	Directories:
***		"."		Itself
***		".."		Parent dir
***		"File0"		Etc.
***
***	From PathTable, walk down list of NextInDir's for entire directory
***
***********************************************************************/

PRIVATE STATUS WriteDir(
	DIRNODE *dn
	)
	{
	REG PATHTABLE *PT;
	REG DIREC *dr;
	REG LONG Length;
	REG LONG Size;
	REG LONG nLength;

	ClearMemory( (int *) DirBuf, SECTOR_SIZE );

	PT = &PathTable[ ( dn->Extent - 1 ) ];

	// "." describes self
	dr = (DIREC *) DirBuf;
	SetISODirRecord( dr, dn, DIREC_SIZE, 1 );
	dr->FileIdent[ 0 ] = 0;

	// ".." describes parent
	dr   = (DIREC *) ( DirBuf + DIREC_SIZE );
	SetISODirRecord( dr, PathTable[ ( PT->Parent - 1 ) ].DirEntry, DIREC_SIZE, 1 );
	dr->FileIdent[ 0 ] = 1;

	// default entries (. ..)
	Size = ( 2 * DIREC_SIZE );

	// If there is anything in dir, write it.
	if ( dn = PT->FirstEntry )
		do
			{
			// Can use this function (Our AbortWin is close enough)
			if ( CheckAbortWin( AbortWin ) )
				return( ERROR );

			dr = (DIREC *) ( DirBuf + Size );

			nLength = strlen( dn->Next.ln_Name );

			// version changes file length
			if ( AddVersion && IS_FILE( dn ) )
				nLength += 2;

			Length  = ( ( DIREC_SIZE + nLength ) & ~1 );

			Size += Length;
			if ( Size > SECTOR_SIZE )
				{
				if ( WriteOut( DirBuf, SECTOR_SIZE ) )
					return( ERROR );

				ClearMemory( (LONG *) DirBuf, SECTOR_SIZE );

				// check for error
				Size = Length;
				dr   = (DIREC *) DirBuf;
				}

			SetISODirRecord( dr, dn, Length, nLength );
			strcpy( dr->FileIdent, dn->Next.ln_Name );

			// add version for file if needed
			if ( AddVersion && IS_FILE( dn ) )
				strcat( dr->FileIdent, ";1" );

			// Flush a SECTOR
			if ( Size == SECTOR_SIZE )
				{
				if ( WriteOut( DirBuf, SECTOR_SIZE ) )
					return( ERROR );

				ClearMemory( (LONG *) DirBuf, SECTOR_SIZE );
				Size = 0;
				}
			} while ( dn = dn->NextInDir );

	// Spill leftover
	if ( Size )
		if ( WriteOut( DirBuf, SECTOR_SIZE ) )
			return( ERROR );

	return( OK );
	}


/***********************************************************************
***
***  WriteEntries
***
***********************************************************************/

PRIVATE STATUS WriteEntries(
	MLIST *DList,
	CSTR RootPath
	)
	{
	PRIVATE CHAR NewPath[ ( FILENAME_LEN * 2 ) ];
	REG DIRNODE *dn;
	FLAG StoreAlt = FALSE;

	SetRootPath( RootPath );

	// Not technically necessary, but I feel better...
	NextBuf = 0;

	// Write everything out...
	FOR_EACH_NODE( DList, dn )
		{
		// Can use this function (Our AbortWin is close enough)
		if ( CheckAbortWin( AbortWin ) )
			return( ERROR );

		if ( VerboseFlag )
			Print( "Writing " );

		switch ( IS_MASK( dn ) )
			{
			case ( ET_FILE ):
				MakePathName( NewPath, dn );

				if ( VerboseFlag )
					Print( "File:  [%6ld %6ld] %s\n", dn->Size, dn->Position, NewPath );

				if ( WriteFile( dn, NewPath ) )
					return( ERROR );
				break;

			case ( ET_CDTV_TM ):
				if ( TradeMark && TMFileName[ 0 ] )
					{
					if ( VerboseFlag )
						Print( "TM    :[%6ld %6ld] %s\n",
							dn->Size, dn->Position, TMFileName );

					if ( WriteFile( dn, TMFileName ) )
						return( ERROR );
					}
				break;

			case ( ET_DIR ):
				if ( FlushFile() )
					return( ERROR );

				if ( VerboseFlag )
					Print( "Dir:   [%6ld %6ld] %s\n",
						dn->Size, dn->Position, dn->Next.ln_Name );

				if ( WriteDir( dn ) )
					return( ERROR );
				break;

			case ( ET_PATHTABLE ):
				if ( FlushFile() )
					return( ERROR );

				if ( VerboseFlag )
					Print( "PathTable\n" );

				if ( CreatePathTable( StoreAlt ) )
					return( ERROR );

				// Second Path Table is in reverse byte order
				StoreAlt = TRUE;
				break;

			case ( ET_EMPTY ):
				if ( FlushFile() )
					return( ERROR );

				NextBuf = ( SECTORS_USED( dn->Size ) * SECTOR_SIZE );

				if ( VerboseFlag )
					Print( "Empty: [%6ld %6ld]\n", NextBuf, dn->Position );

				if ( FlushFile() )
					return( ERROR );
				break;

			case ( ET_HEADERS ):
				if ( VerboseFlag )
					Print( "Headers\n" );

				// MUST ALWAYS BE FIRST!!!!!!
				if ( WriteHeader() )
					return( ERROR );
				break;
			}
		}

	return( OK );
	}


/***********************************************************************
***
***  WriteFile
***
***********************************************************************/

PRIVATE STATUS WriteFile(
	DIRNODE *dn,
	CSTR path
	)
	{
	REG LONG Size;
	REG LONG retn;

	if ( ! ( FilePtr = Open( path, MODE_OLDFILE ) ) )
		{
		Print( "Cannot open %s", path );
		TellUser( Interface, "Open failed" );
		return( ERROR );
		}

	for ( EVER )
		{
		// Can use this function (Our AbortWin is close enough)
		if ( CheckAbortWin( AbortWin ) )
			return( ERROR );

		Size = ( FileBufSize - NextBuf );
		retn = Read( FilePtr, ( FileBuf + NextBuf ), Size );

		// Is this OK????
		if ( retn < 1 )
			retn = 1;

		NextBuf += retn;

		if ( NextBuf >= FileBufSize )
			if ( FlushFile() )
				return( ERROR );

		// Read was short; thus, entire file processed
		if ( Size != retn )
			break;
		}

	// Round off to sector boundary
	NextBuf = ( ( NextBuf + SECTOR_SIZE - 1 ) & ~( SECTOR_SIZE - 1 ) );
	if ( NextBuf >= FileBufSize )
		if ( FlushFile() )
			return( ERROR );

	Close( FilePtr );
	FilePtr = NULL;

	return( OK );
	}


/***********************************************************************
***
***  WriteHeader
***
***********************************************************************/

PRIVATE STATUS WriteHeader(
	void
	)
	{
	LONG i;

	// empty sys mem area
	NextBuf = ( 16 * SECTOR_SIZE );
	if ( FlushFile() )
		return( ERROR );

	// Write out headers
	SPAN( i, NumPVD )
		if ( WritePVD() )
			return( ERROR );

	if ( WriteTVD() )
		return( ERROR );

	// If there is a split file, no need for it to be left open.
	CloseSplit();

	return( OK );
	}


/***********************************************************************
***
***  WritePVD
***
***********************************************************************/

PRIVATE STATUS WritePVD(
	void
	)
	{
	CHAR IDBuffer[ 129 ];
	PVD *p = (PVD *) DirBuf;
	DIREC *dr;
	struct DateStamp Today;

	// Can use this function (Our AbortWin is close enough)
	if ( CheckAbortWin( AbortWin ) )
		return( ERROR );

	ClearMemory( (LONG *) DirBuf, SECTOR_SIZE );

	p->VolDescType = PRIM_VOL_DESC;

	strcpy( p->StdIdent, STD_IDENT );

	p->VolDescVers = 1;

	strcpy( p->SysIdent, SYS_IDENT );

	LoadStr( p->VolIdent, IDVol, 32 );

	p->VolSizeX	   = IntelLong( CDSize );
	p->VolSize	   = CDSize;
	p->VolSetSizeX	   = IntelWord( 1 );
	p->VolSetSize	   = 1;
	p->VolSeqNumX	   = IntelWord( 1 );
	p->VolSeqNum	   = 1;
	p->BlockSizeX	   = IntelWord( SECTOR_SIZE );
	p->BlockSize	   = SECTOR_SIZE;
	p->PathTableSizeX  = IntelLong( PathTableSize );
	p->PathTableSize   = PathTableSize;

	// Alternate ISO PathTable for reverse byte order machines
	if ( AddAltPathTable )
		{
		p->LPathTable[ 0 ] = IntelLong( AltPathSector );
		p->LPathTable[ 1 ] = IntelLong( AltPathSector );
		}

	p->MPathTable[ 0 ] = PathSector;
	p->MPathTable[ 1 ] = PathSector;

	if ( IDSet[ 0 ] )
		LoadStr( p->VolSetIdent, IDSet, 128 );

	if ( IDPub[ 0 ] )
		LoadStr( p->PublisherIdent, IDPub, 128 );

	// Attach our name to PreparerID
	strcpy( IDBuffer, IDPrep );
	strcat( IDBuffer, PrepTitle );
	LoadStr( p->DataPrepIdent, IDBuffer, 128 );

	if ( IDApp[ 0 ] )
		LoadStr( p->ApplicatIdent, IDApp, 128 );

	// Today's date
	TodaysDate( &Today );
	SetISOPVDDate( JUL_SEC( Today ), p->VolCreateTime );

	p->FileStructVers = 1;

	// Root Dir location, etc.
	dr = &p->RootDir;
	SetISODirRecord( dr, RootNode, DIREC_SIZE, 1 );
	dr->FileIdent[ 0 ] = 0;

	// CDFS uses ApplicationUse section
	SetCDFSOptions( p );

	return( WriteOut( DirBuf, SECTOR_SIZE ) );
	}


/***********************************************************************
***
***  WriteTVD
***
***********************************************************************/

PRIVATE STATUS WriteTVD(
	void
	)
	{
	PVD *p = (PVD *) DirBuf;

	ClearMemory( (LONG *) DirBuf, SECTOR_SIZE );

	p->VolDescType = TERM_VOL_DESC;

	strcpy( p->StdIdent, STD_IDENT );

	p->VolDescVers = 1;

	return( WriteOut( DirBuf, SECTOR_SIZE ) );
	}
