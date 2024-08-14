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
***	Dir.c
***
***  PURPOSE:
***
***	Will load directories and sub-directories for ISOCD
***
***  HISTORY:
***
***	0.01 0308 C. Sassenrath	Created.
***	0.02 1630 C. Sassenrath	Updated.
***	0.03 2210 C. Sassenrath	Revisited.
***	0.04 2421 Ken Yeast	Visited, changes for testing of ListView
***				List vs. MinList
***	0.05 2504 Ken Yeast	Changed to match current standards
***				(headers and other meaningless stuff)
***				Crammed in my coding style and ansi stuff
***				removed ps() and printf -> Print
***				CountNodes moved to Support.lib:List.c
***				changed to M/LIST and M/NODE's
***				added AllocDirNode/Base from Mem.c
***	0.06 2505 Ken Yeast	DIRNODE changing
***				IS/AS_DIR changed
***				LockList for locks
***				Push/PopLock
***				Locks now unlocked on CTRL-C
***				MakeCD moved out
***				FindTM
***				PrintDirList public
***	0.07 2506 Ken Yeast	Push/PopLock revised
***	0.08 2507 Ken Yeast	Changing list methodology
***	0.09 2508 Ken Yeast	rewrote ScanDir/ExpandDir
***				BuildPathTable
***				FlattenAndArrange
***				ArrangeDir
***				FreeDirLists
***	0.10 2511 Ken Yeast	Add PT node
***				DirEntry instead of Name/etc. in PathTable
***	0.11 2512 Ken Yeast	FirstEntry should be NULL if there are no
***				entries in dir list
***	0.12 2513 Ken Yeast	AddSpecials
***	0.13 2514 Ken Yeast	ET_HEADERS
***				ET_EMPTY used for buffer at end
***	0.14 2515 Ken Yeast	Create/RemoveLockList
***				Create/RemoveDirList
***				Create/RemoveDirNodes
***				Create/RemovePathTable
***				ditched FreeDirLists
***	0.15 2520 Ken Yeast	Usage of ShowPath
***				PrintDirList removed
***				Buffer size and Header size
***	0.16 2521 Ken Yeast	Ability to abort scanning
***				Finish
***				LoadFile/SaveFile moved here from File.c
***				MakeDNode
***				LoadFile working
***	0.17 2522 Ken Yeast	CheckList
***				Alloc/FreeSort
***				SortLoadDir
***				LoadDir
***				Works!!!
***				SortCmpSize
***				SortCmpDate
***				SortCmpInfo
***				SortCmpDirs
***				SortCmpExtension
***	0.18 2527 Ken Yeast	SortReverse
***				SortCmpxxxRev
***				Added TradeMark stuff
***				InitTM
***	0.19 2528 Ken Yeast	More usage of MakeDNode, some changes
***				AllocDirNode removed
***				RootDate
***	0.20 2529 Ken Yeast	ShowPath moved here, changed to output to AbortWin
***	0.21 2601 Ken Yeast	Fixed ShowPath (when used by LoadFile)
***	0.22 2603 Ken Yeast	Some changes to minimize stack use
***				FIBBuffer moved to static
***				NewPath from AllocMem instead of stack
***				Alloc/FreeNewPath
***				(May chose to move NewPath stuff back to stack)
***				Added Groups
***				Recoded sort/filters to work at the same time
***				SortFuncPtr
***				SortCmpFNone/Rev
***				SortCmpNone/Rev
***				All seems to work
***				LoadFile absorbs some code
***	0.23 2605 Ken Yeast	Load/SaveFile handles Option data as well
***	0.24 2608 Ken Yeast	Code added to handle Missing and Extra files
***				LoadDir changed
***				Will InitTM after LoadFile/etc.
***	     2609 Ken Yeast	remaining code for Missing/Extra
***				M/E dirs CANNOT be handled at this time
***				Extension sort working
***	0.25 2610 Ken Yeast	Fixed bug in LoadFile (loading <Trade Mark>)
***				AT_BEFORE_TAIL for adding extra files during LoadFile
***	0.26 2617 Ken Yeast	Dirs deep check for ISO
***	0.27 2622 Ken Yeast	<Empty Space> saves out size (and loads)
***				MakeDNode public
***				MakeDNode:AT_AFTER_HEAD added
***	0.28 2623 Ken Yeast	Usage of SetRootPath
***	0.29 2626 Ken Yeast	Removed MakeFileNode
***				MakeDNode changed, added AT_NOWHERE
***	0.30 2727 Ken Yeast	AddAltPathTable for reverse byte systems
***	0.31 2728 Ken Yeast	InitAbortWin using FontReq, also ShowPath
***				Minor optimization to ShowPath
***				InitShowPath
***				Made ShowPath window bigger
***	0.32 2807 Ken Yeast	Bug in SaveFile: if could not open file, was closing
***				NULL
***	0.33 2918 Ken Yeast	Batch option changes
***	0.34 2a29 Ken Yeast	Save state as 1 or 0 for CDFS on/off options in layout
***
************************************************************************/


/***********************************************************************
***
***  Functions
***
***	AddSpecials
***	AllocDirList
***	AllocNewPath
***	AllocSort
***	ArrangeDir
***	BuildDirList
***	BuildPathTable
***	CheckList
***	CreateDListPool
***	CreateDNodePool
***	CreateLockList
***	CreatePathTable
***	ExpandDir
***	FindFile
***	Finish
***	FlattenAndArrange
***	FreeNewPath
***	FreeSort
***	InitDir
***	InitShowPath
***	InitTM
***	LoadFile
***	LoadDir
***	MakeDNode
***	PopLock
***	PushLock
***	QuitDir
***	RemoveDListPool
***	RemoveDNodePool
***	RemoveLockList
***	RemovePathTable
***	SaveFile
***	SaveFileNode
***	ScanDir
***	ShowPath
***	SortCmpAlpha
***	SortCmpAlphaRev
***	SortCmpDate
***	SortCmpDateRev
***	SortCmpDirs
***	SortCmpDirsRev
***	SortCmpExtension
***	SortCmpExtensionRev
***	SortCmpFNone
***	SortCmpFNoneRev
***	SortCmpInfo
***	SortCmpInfoRev
***	SortCmpNone
***	SortCmpNoneRev
***	SortCmpSize
***	SortCmpSizeRev
***	SortLoadDir
***	SortScanDir
***
************************************************************************/


#include <work/standard.h>

#include <exec/lists.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>

#include <string.h>
#include <dos.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>

#include <work/Support.h>
#include <work/Core.h>
#include <work/Dos.h>
#include <work/Interface.h>
#include <work/List.h>
#include <work/Memory.h>

#include "ISOCD.h"


/***********************************************************************
***
***  External Dependencies
***
***********************************************************************/

IMPORT INTERFACE *Interface;
IMPORT STATE	Batch;

IMPORT BOOL	VerboseFlag;
IMPORT LONG	TotalBytes;
IMPORT LONG	Dirs;
IMPORT LONG	Files;
IMPORT PATHTABLE *PathTable;

// ISO 9660 Compatibility Options
IMPORT FLAG	AddAltPathTable;

// Dir Info
IMPORT LONG	MissingFiles;
IMPORT LONG	ExtraFiles;
IMPORT STATE	TooDeepForISO;

// TradeMark
IMPORT LONG	TradeMark;
IMPORT DIRNODE	*TMFileNode;
IMPORT CHAR	TMFileName[];

IMPORT CHAR	ImageName[];

// -------- Options --------
IMPORT LONG	SortOrder;
IMPORT LONG	SortReverse;
IMPORT LONG	SortGroup;
IMPORT LONG	GroupReverse;

// ISO Stuff
IMPORT CHAR	IDVol[];
IMPORT CHAR	IDSet[];
IMPORT CHAR	IDPub[];
IMPORT CHAR	IDPrep[];
IMPORT CHAR	IDApp[];
IMPORT LONG	NumPVD;
IMPORT LONG	BaseSector;
IMPORT CHAR	SplitFileName[];

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


/***********************************************************************
***
***  Definitions
***
***********************************************************************/

#define	TEMP_SORT	128
#define	MAX_LOCKS	64
#define	PATHNAME_SIZE	256

#define		SAVE_STATE( v )		( ( v ) ? 1 : 0 )

#define		READ_BUF( f, b, s )	if ( fgets( b, 256, f ) == 0 )		\
						goto Failed;			\
					(b)[ ( strlen( b ) - 1 ) ] = NIL;	\
					strcpy( s, b );



/***********************************************************************
***
***  Function Prototypes
***
***********************************************************************/

PRIVATE void	AddSpecials( MLIST *, CSTR );
PRIVATE MLIST	*AllocDirList( void );
PRIVATE CSTR	AllocNewPath( void );
PRIVATE DIRNODE	**AllocSort( LONG );
PRIVATE void	ArrangeDir( MLIST * );
PRIVATE STATUS	BuildPathTable( MLIST * );
PRIVATE STATUS	CheckList( MLIST *, CSTR );
PRIVATE void	CreateDListPool( void );
PRIVATE void	CreateDNodePool( void );
PRIVATE void	CreateLockList( void );
PRIVATE void	CreatePathTable( void );
PRIVATE STATUS	ExpandDir( CSTR, MLIST * );
PRIVATE void	Finish( void );
PRIVATE void	FlattenAndArrange( MLIST * );
PRIVATE void	FreeNewPath( CSTR * );
PRIVATE void	FreeSort( DIRNODE **, LONG );
PRIVATE void	InitShowPath( void );
PRIVATE STATUS	InitTM( MLIST *, CSTR );
PRIVATE STATUS	LoadDir( MLIST *, DIRNODE **, CSTR, FLAG *, LONG );
PRIVATE FLAG	PopLock( void );
PRIVATE void	PushLock( BPTR );
PRIVATE void	RemoveDListPool( void );
PRIVATE void	RemoveDNodePool( void );
PRIVATE void	RemoveLockList( void );
PRIVATE void	RemovePathTable( void );
PRIVATE STATUS	SaveFileNode( MNODE *, LONG );
PRIVATE void	ShowPath( CSTR );
PRIVATE int	SortCmpAlpha( DIRNODE **, DIRNODE ** );
PRIVATE int	SortCmpAlphaRev( DIRNODE **, DIRNODE ** );
PRIVATE int	SortCmpDate( DIRNODE **, DIRNODE ** );
PRIVATE int	SortCmpDateRev( DIRNODE **, DIRNODE ** );
PRIVATE int	SortCmpDirs( DIRNODE **, DIRNODE ** );
PRIVATE int	SortCmpDirsRev( DIRNODE **, DIRNODE ** );
PRIVATE int	SortCmpExtension( DIRNODE **, DIRNODE ** );
PRIVATE int	SortCmpExtensionRev( DIRNODE **, DIRNODE ** );
PRIVATE int	SortCmpFNone( DIRNODE **, DIRNODE ** );
PRIVATE int	SortCmpFNoneRev( DIRNODE **, DIRNODE ** );
PRIVATE int	SortCmpInfo( DIRNODE **, DIRNODE ** );
PRIVATE int	SortCmpInfoRev( DIRNODE **, DIRNODE ** );
PRIVATE int	SortCmpNone( DIRNODE **, DIRNODE ** );
PRIVATE int	SortCmpNoneRev( DIRNODE **, DIRNODE ** );
PRIVATE int	SortCmpSize( DIRNODE **, DIRNODE ** );
PRIVATE int	SortCmpSizeRev( DIRNODE **, DIRNODE ** );
PRIVATE DIRNODE	*SortLoadDir( DIRNODE *, int (*)( DIRNODE **, DIRNODE ** ) );
PRIVATE void	SortScanDir( MLIST *, int (*)( DIRNODE **, DIRNODE ** ) );
PRIVATE MLIST	*ScanDir( CSTR, LONG * );


/***********************************************************************
***
***  Module Variables
***
***********************************************************************/

// Push/PopLock stack
PRIVATE BPTR	*LockStack		= NULL;
PRIVATE WORD	LockMeter		= 0;

// RootNode
PRIVATE DIRNODE	*RootNode		= NULL;
PRIVATE LONG	RootDate		= 0;

// PathTable
PRIVATE LONG	PathTableSize		= 0;

// Abort
PRIVATE INTERFACE *AbortWin		= NULL;
PRIVATE BOX	AbortBox		= { 0, 0, 480, 80 };
PRIVATE struct	RastPort AbortRP;
PRIVATE WORD	AbortY			= 0;

// Sort
PRIVATE int	(*SortFuncPtr)( DIRNODE **, DIRNODE ** )	= NULL;
PRIVATE DIRNODE	*TempSort[ TEMP_SORT ];

// FIB Buffer
PRIVATE FIB	*Fib			= NULL;
PRIVATE UBYTE	FIBBuffer[ ( sizeof( FIB ) + 3 ) ];

PRIVATE LONG	Deep			= 0;

/***********************************************************************
***
***  Standard Memory Pools
***
***	These are the standard Memory Pool structures used in the
***	program: Single Nodes, Double Nodes, and Vectors.
***
***	The constants below are used to set the segment allocation
***	sizes for each of the pools.  It is also possible for the
***	program to change these sizes dynamically while running.
***
***********************************************************************/

PRIVATE struct	MemoryPool DirListPool =
	{
	NULL,
	NULL,
	0,
	0,
	sizeof( MLIST ),
	32
	};

PRIVATE struct	MemoryPool DirNodePool =
	{
	NULL,
	NULL,
	0,
	0,
	sizeof( DIRNODE ),
	256
	};


/***********************************************************************
***
***  AddSpecials
***
***	Force feed PathTable, Headers, and buffer space in for now...
***	May put Addr of PathTable in Extent
***
***********************************************************************/

PRIVATE void AddSpecials(
	MLIST *MasterList,
	CSTR RootDir
	)
	{
	// TradeMark
	TMFileNode = NULL;
	if ( TradeMark && ( InitTM( MasterList, RootDir ) == OK ) )
		TMFileNode = MakeDNode( MasterList, TM_STR, SizeOfFile( TMFileName ),
					0, ET_CDTV_TM, AT_HEAD );

	// PathTable
	MakeDNode( MasterList, PATH_STR, 0, 0, ET_PATHTABLE, AT_HEAD );

	if ( AddAltPathTable )
		MakeDNode( MasterList, PATH_STR, 0, 0, ET_PATHTABLE, AT_HEAD );

	// Pseudo for Headers (immovable)
	MakeDNode( MasterList, HEAD_STR, ( ( 16 + NumPVD + 1 ) * SECTOR_SIZE ),
		   0, ET_HEADERS, AT_HEAD );

	// Buffer space at end of image
	MakeDNode( MasterList, END_STR, ( 32 * SECTOR_SIZE ),
		   0, ET_EMPTY, AT_TAIL );
	}


/***********************************************************************
***
***  AllocDirList
***
***********************************************************************/

PRIVATE MLIST *AllocDirList(
	void
	)
	{
	MLIST *n;

	n = AllocNode( &DirListPool );
	NewList( (LIST *) n );
	return( n );
	}


/***********************************************************************
***
***  AllocNewPath
***
***********************************************************************/

PRIVATE CSTR AllocNewPath(
	void
	)
	{
	return( AllocMem( PATHNAME_SIZE, MEMF_CLEAR ) );
	}


/***********************************************************************
***
***  AllocSort
***
***********************************************************************/

PRIVATE DIRNODE **AllocSort(
	LONG Length
	)
	{
	if ( Length > TEMP_SORT )
		return( (DIRNODE **) AllocSysMem( ( Length * sizeof( DIRNODE * ) ) ) );
	else
		return( TempSort );
	}


/***********************************************************************
***
***  ArrangeDir
***
***	Remember to change the SortReverse offset if the jump table
***	changes
***
***********************************************************************/

PRIVATE void ArrangeDir(
	MLIST *DList
	)
	{
	PRIVATE int (*SortFunc[])( DIRNODE **, DIRNODE ** ) =
		{
		SortCmpAlpha,
		SortCmpSize,
		SortCmpDate,
		SortCmpNone,

		SortCmpAlphaRev,
		SortCmpSizeRev,
		SortCmpDateRev,
		SortCmpNoneRev,
		};
	PRIVATE int (*GroupFunc[])( DIRNODE **, DIRNODE ** ) =
		{
		SortCmpInfo,
		SortCmpDirs,
		SortCmpExtension,
		SortCmpFNone,

		SortCmpInfoRev,
		SortCmpDirsRev,
		SortCmpExtensionRev,
		SortCmpFNoneRev,
		};

	SortFuncPtr = SortFunc[ ( SortOrder + ( ( SortReverse ) ? 4 : 0 ) ) ];

	SortScanDir( DList, GroupFunc[ SortGroup + ( ( GroupReverse ) ? 4 : 0 ) ] );
	}


/***********************************************************************
***
***  BuildDirList
***
***	A) Recursively parse directories, building lists of DIRNODES, first setting
***	   up root dir DIRNODE:
***
***	B) Build Path Table, sorting directories alphabetically, and connecting
***	   NextInDir's:
***
***	C) Following Path Table, flatten out and re-arrange according to rules,
***	   creating a main list:
***
***	D) Free pool of dir MLIST's.
***
***********************************************************************/

STATUS BuildDirList(
	MLIST *MasterList,
	CSTR Path
	)
	{
	MLIST *RootList;
	CSTR Err = NULL;

	// Setup
	RemovePathTable();
	RemoveDNodePool();
	CreateDNodePool();
	CreateDListPool();

	// Requester for Abort
	if ( ! ( AbortWin = InitAbortWin( &AbortBox, Interface->IFontReq, "ISOCD", "Scanning directories", NULL ) ) )
		ERR( Err, "Cannot obtain requester" );

	// Box for ShowPath text
	InitShowPath();

	// Now ready for fresh start
	Dirs  = 1;
	Files = 0;
	Deep  = 0;

	// RootList is special case
	if ( ! ( RootList = ScanDir( Path, &RootDate ) ) )
		goto Failed;

	// Check for empty
	if ( ! HAS_NODE_LIST( RootList ) )
		ERR( Err, "Directory is empty" );

	// Load all directories into MLIST's of DIRNODES
	if ( ExpandDir( Path, RootList ) )
		goto Failed;

	ShowPath( " " );

	if ( VerboseFlag )
		Print( "\t%ld Files - %ld Directories\n", Files, Dirs );

	// Build Path Table (sorting dirs and connecting NextInDir's)
	if ( BuildPathTable( RootList ) )
		goto Failed;

	// Now flatten out multiple MLIST's of DIRNODES into one MLIST, according
	// to the rules
	FlattenAndArrange( MasterList );

	// Node for PathTable, Headers, etc.
	AddSpecials( MasterList, Path );

	// This will provide us with all of the sizes
	if ( DeterminePositions( MasterList ) )
		{
		UnSetRootName();
		goto Failed;
		}

	UnSetRootName();

	Finish();

	return( OK );

Failed:
	Finish();

	if ( Err )
		TellUser( Interface, Err );

	return( ERROR );
	}


/***********************************************************************
***
***  BuildPathTable
***
***	1) Alloc Path Table, Dirs entries large.
***	2) Add in root dir as first entry, not sure what its Parent # is.
***	3) Loop through Path Table entries, using FirstEntry pointer in table
***	   to get to list:
***		a) Sort dir list alphabetically.
***		b) Scan list:
***			i)   connect NextInDir's.
***			ii)  if DIR, add ptr to 1st node to Path Table,
***			     placing loop # in Path Table Parent # (+1).
***			     Extent must now hold offset into Path Table (+1 to 
***			     reflect ISO9660 numbering).
***			iii) if FILE, store loop # in Extent (+1 - see above).
***		c) the list will terminate normally since the last entry
***		   will not have its NextInDir adjusted.
***		d) If there are no entries in list, FirstEntry = NULL
***
***********************************************************************/

PRIVATE STATUS BuildPathTable(
	MLIST *RootList
	)
	{
	REG PATHTABLE *PTGet;
	REG PATHTABLE *PTPut;
	REG MLIST *DList;
	REG DIRNODE *DNode;
	REG DIRNODE *PrevDNode;
	LONG i;
	LONG Depth;

	// get memory for table
	CreatePathTable();

	PTGet = PTPut = PathTable;

	// Add in root dir
	// DirEntry will be added later (FlattenAndArrange)
	PTPut->FirstEntry = (DIRNODE *) RootList->mlh_Head;
	PTPut->Parent	  = 1;
	PTPut++;

	// Loop through PathTable as it is being created, each scan adds the
	// successive dirs in order!
	SPAN( i, Dirs )
		{
		if ( CheckAbortWin( AbortWin ) )
			return( ERROR );

		Depth = ( i + 1 );
		DList = (MLIST *) PTGet->FirstEntry->Next.ln_Pred;

		if ( HAS_NODE_LIST( DList ) )
			{
			SortScanDir( DList, SortCmpAlpha );

			// First entry has probably changed
			PTGet->FirstEntry = (DIRNODE *) DList->mlh_Head;

			PrevDNode = NULL;
			FOR_EACH_NODE( DList, DNode )
				{
				DNode->NextInDir = NULL;

				if ( PrevDNode )
					PrevDNode->NextInDir = DNode;

				if ( IS_DIR( DNode ) )
					{
					PTPut->FirstEntry = (DIRNODE *) DNode->Extent;
					PTPut->DirEntry	  = DNode;
					PTPut->Parent	  = Depth;
					DNode->Extent	  = ( PTPut - PathTable + 1 );
					PTPut++;
					}
				else
					DNode->Extent	  = Depth;

				PrevDNode = DNode;
				}
			}
		else
			PTGet->FirstEntry = NULL;

		PTGet++;
		}

	return( OK );
	}


/***********************************************************************
***
***  CheckList
***
***	After loading from file, we now must sort for ISO and obtain
***	Size/Date from files.  Will also remove missing files from list
***	and add additional files.
***
***********************************************************************/

PRIVATE STATUS CheckList(
	MLIST *MasterList,
	CSTR RootPath
	)
	{
	REG PATHTABLE *PTGet = PathTable;
	REG LONG i;
	CSTR NewPath;
	CSTR OldRootStr	     = NULL;
	STATUS Err	     = OK;
	FLAG NeedSort	     = FALSE;

	MissingFiles = 0;
	ExtraFiles   = 0;

	if ( ! ( NewPath = AllocNewPath() ) )
		return( ERROR );

	SetRootPath( RootPath );

	// Correct RootNode for scanning ("<Root Dir>" is not kosher)
	OldRootStr = PTGet->DirEntry->Next.ln_Name;
	PathTable[ 0 ].DirEntry->Next.ln_Name = "";

	SPAN( i, Dirs )
		{
		if ( CheckAbortWin( AbortWin ) )
			{
			Err = ERROR;
			break;
			}

		if ( PTGet->FirstEntry )
			{
			PTGet->FirstEntry = SortLoadDir( PTGet->FirstEntry, SortCmpAlpha );

			MakePathName( NewPath, PTGet->DirEntry );

			if ( LoadDir( MasterList, &PTGet->FirstEntry, NewPath, &NeedSort, ( i + 1 ) ) )
				{
				Err = ERROR;
				break;
				}

			if ( NeedSort )
				{
				PTGet->FirstEntry = SortLoadDir( PTGet->FirstEntry, SortCmpAlpha );
				NeedSort = FALSE;
				}
			}

		PTGet++;
		}

	FreeNewPath( &NewPath );
	PathTable[ 0 ].DirEntry->Next.ln_Name = OldRootStr;
	ShowPath( " " );
	return( Err );
	}


/***********************************************************************
***
***  CreateDListPool
***
***********************************************************************/

PRIVATE void CreateDListPool(
	void
	)
	{
	ExtendPool( &DirListPool );
	}


/***********************************************************************
***
***  CreateDNodePool
***
***********************************************************************/

PRIVATE void CreateDNodePool(
	void
	)
	{
	ExtendPool( &DirNodePool );
	}


/***********************************************************************
***
***  CreateLockList
***
***********************************************************************/

PRIVATE void CreateLockList(
	void
	)
	{
	LockStack = (BPTR *) AllocSysMem( sizeof( BPTR ) * MAX_LOCKS );
	LockMeter = 0;
	}


/***********************************************************************
***
***  CreatePathTable
***
***********************************************************************/

PRIVATE void CreatePathTable(
	void
	)
	{
	PathTableSize = ( Dirs * sizeof( PATHTABLE ) );
	PathTable     = AllocSysMem( PathTableSize );
	}


/***********************************************************************
***
***  ExpandDir
***
***	Read the subdirectories of a directory list.
***	1) Scan directory entries within dir:
***		a) Create MLIST for this dir, add to MLIST of dirs.
***		b) Scan entries:
***			i)  add DIRNODE for entry.
***			ii) if DIR, -> (1)
***			    this returns the MLIST for the dir, store the first node
***			    in the Extent.  Also, Dirs++ else Files++.
***		c) returns the MLIST for the dir
***
***********************************************************************/

PRIVATE STATUS ExpandDir(
	CSTR path,
	MLIST *DList
	)
	{
	CSTR Format	= "%s/%s";
	CSTR NewPath	= NULL;
	REG MLIST *NextDir;
	REG DIRNODE *dn;

	// Check for ISO compatibility
	if ( Deep > ISO_DEEP )
		TooDeepForISO = TRUE;

	Deep++;

	// correct for path that designates root (like "dh0:")
	if ( path[ strlen( path ) - 1 ] == ':' )
		Format = "%s%s";

	FOR_EACH_NODE( DList, dn )
		{
		if ( CheckAbortWin( AbortWin ) )
			goto Failed;

		if ( IS_DIR( dn ) )
			{
			if ( ! ( NewPath = AllocNewPath() ) )
				goto Failed;

			Dirs++;
			sprintf( NewPath, Format, path, dn->Next.ln_Name );

			if ( ! ( NextDir = ScanDir( NewPath, NULL ) ) )
				goto Failed;

			dn->Extent = (LONG) NextDir->mlh_Head;

			if ( ExpandDir( NewPath, NextDir ) )
				goto Failed;

			FreeNewPath( &NewPath );
			}
		else
			Files++;
		}

	Deep--;
	return( OK );

Failed:
	Deep--;
	FreeNewPath( &NewPath );
	return( ERROR );
	}


/***********************************************************************
***
***  FindFile
***
***********************************************************************/

DIRNODE *FindFile(
	MLIST *DList,
	CSTR Name
	)
	{
	DIRNODE *dn;

	if ( DList )
		FOR_EACH_NODE( DList, dn )
			if ( stricmp( Name, dn->Next.ln_Name ) == 0 )
				return( dn );

	return( NULL );
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
	// Pool of MLIST's not needed
	RemoveDListPool();

	QuitAbortWin( AbortWin ), AbortWin = NULL;
	}


/***********************************************************************
***
***  FlattenAndArrange
***
***	1) using FirstEntry pointer in table to get to list, re-arrange list
***	   according to rules.
***	2) merge with main list.
***
***********************************************************************/

PRIVATE void FlattenAndArrange(
	MLIST *MasterList
	)
	{
	REG PATHTABLE *PTPtr;
	REG MLIST *DList;
	REG DIRNODE *NewEnd;
	REG LONG i;

	NewList( (LIST *) MasterList );

	// Add artificial Root Dir
	RootNode = MakeDNode( MasterList, ROOT_STR, 0, RootDate, ET_DIR, AT_TAIL );
	RootNode->Extent = 1;

	PathTable[ 0 ].DirEntry = NewEnd = RootNode;

	// Scan all Dir lists
	PTPtr = PathTable;

	SPAN( i, Dirs )
		{
		if ( PTPtr->FirstEntry )
			{
			DList = (MLIST *) PTPtr->FirstEntry->Next.ln_Pred;

			if ( HAS_NODE_LIST( DList ) )
				{
				// Arrange according to rules
				ArrangeDir( DList );

				// Flatten (connect NewEnd to first node in list, update NewEnd)
				NewEnd->Next.ln_Succ	  = (NODE *)	DList->mlh_Head;
				DList->mlh_Head->mln_Pred = (MNODE *)	NewEnd;
				NewEnd			  = (DIRNODE *)	DList->mlh_TailPred;
				}
			}

		PTPtr++;
		}

	// Connect NewEnd back into list
	MasterList->mlh_TailPred = (MNODE *) NewEnd;
	NewEnd->Next.ln_Succ	 = (NODE *) &( MasterList->mlh_Tail );
	}


/***********************************************************************
***
***  FreeNewPath
***
***********************************************************************/

PRIVATE void FreeNewPath(
	CSTR *NewPathPtr
	)
	{
	if ( NewPathPtr && *NewPathPtr )
		FreeMem( *NewPathPtr, PATHNAME_SIZE ), *NewPathPtr = NULL;
	}


/***********************************************************************
***
***  FreeSort
***
***********************************************************************/

PRIVATE void FreeSort(
	DIRNODE **SortBase,
	LONG Length
	)
	{
	if ( Length > TEMP_SORT )
		FreeSysMem( SortBase );
	}


/***********************************************************************
***
***  InitDir
***
***********************************************************************/

void InitDir(
	void
	)
	{
	// Stack of locks (in case of CTRL-C)
	CreateLockList();

	// Setup FIB
	Fib = (FIB *) ( ( (LONG) FIBBuffer + 3 ) & ~3 );
	}


/***********************************************************************
***
***  InitShowPath
***
***********************************************************************/

PRIVATE void InitShowPath(
	void
	)
	{
	DrawNewBox( AbortWin, 8, 27, 466, 14 );
	AbortRP = *( AbortWin->IWindow->RPort );
	SetAPen( &AbortRP, 1 );
	SetFont( &AbortRP, AbortWin->IFont );
	AbortY = ( 36 + AbortWin->TopBorder );
	}


/***********************************************************************
***
***  InitTM
***
***	Try to find TM_NAME in current dir, then in program launch dir
***
***********************************************************************/

PRIVATE STATUS InitTM(
	MLIST *MasterList,
	CSTR RootPath
	)
	{
	CSTR Program;

	// Search current dir first
	strcpy( TMFileName, TM_NAME );
	if ( SizeOfFile( TMFileName ) )
		return( OK );

	// Next, check in the directory the program was launched
	if ( Program = ProgramName() )
		{
		strcpy( TMFileName, Program );
		strcpy( FilePart( TMFileName ), TM_NAME );
		if ( SizeOfFile( TMFileName ) )
			return( OK );
		}

	// Well, check in the scanned list...
	if ( TMFileNode = FindFile( MasterList, TM_NAME ) )
		{
		SetRootPath( RootPath );
		MakePathName( TMFileName, TMFileNode );
		if ( SizeOfFile( TMFileName ) )
			return( OK );
		}

	// OK, give up.
	TMFileName[ 0 ] = NIL;
	return( ERROR );
	}


/***********************************************************************
***
***  LoadDir
***
***	Read a directory and fill in Size/Date on each matching DIRNODE
***	in single link list (NextInDir).
***
***********************************************************************/

PRIVATE STATUS LoadDir(
	MLIST *MasterList,
	DIRNODE **TopNode,
	CSTR Path,
	FLAG *NeedSort,
	LONG Extent
	)
	{
	REG DIRNODE *dn;
	REG BPTR lock;
	REG DIRNODE *PrevNode;

	ShowPath( Path );

	if ( lock = Lock( Path, ACCESS_READ ) )
		{
		PushLock( lock );

		Examine( lock, Fib );

		// This SHOULD be a directory
		if ( Fib->fib_DirEntryType > 0 )
			{
			// Add it's entries at this time
			while ( ExNext( lock, Fib ) )
				{
				if ( CheckAbortWin( AbortWin ) )
					goto Failed;

				// Find and change
				PrevNode = NULL;
				for ( dn = *TopNode; dn; dn = dn->NextInDir )
					{
					if ( ! ( dn->Size || dn->Date ) )
						if ( stricmp( dn->Next.ln_Name, Fib->fib_FileName ) == 0 )
							{
							dn->Size = Fib->fib_Size;
							dn->Date = JUL_SEC( Fib->fib_Date );
							break;
							}

					PrevNode = dn;
					}

				// If not found, add to list
				if ( ! dn )
					{
					// If Dir, entire list is changed!
					// Logic to correct all of the extent's, etc.
					// is too complicated for now.
					if ( Fib->fib_DirEntryType > 0 )
						{
						StatusError( "Incompatible dir heirarchy" );
						goto Failed;
						}

					// Add to flat list (just before <Buffer>)
					dn = MakeDNode( MasterList,
							Fib->fib_FileName,
							Fib->fib_Size,
							JUL_SEC( Fib->fib_Date ),
							ET_FILE, AT_BEFORE_TAIL );
					dn->Extent = Extent;

					// May become only node
					if ( PrevNode )
						{
						dn->NextInDir = PrevNode->NextInDir;
						PrevNode->NextInDir = dn;
						}
					else
						{
						dn->NextInDir = NULL;
						*TopNode = dn;
						}

					ExtraFiles++;
					Files++;
					}
				}

			// Check for any missing, remove
			PrevNode = NULL;
			for ( dn = *TopNode; dn; dn = dn->NextInDir )
				{
				if ( ! ( dn->Size || dn->Date ) )
					{
					// If Dir, entire list is changed!
					// Logic to correct all of the extent's, etc.
					// is too complicated for now.
					if ( IS_DIR( dn ) )
						{
						StatusError( "Incompatible dir heirarchy" );
						goto Failed;
						}

					// Remove from flat list
					// Remember: This does waste some space
					Remove( (NODE *) dn );

					// Remove from path table dir list
					// (may change top pointer)
					if ( PrevNode )
						PrevNode->NextInDir = dn->NextInDir;
					else
						*TopNode = dn->NextInDir;

					MissingFiles++;
					Files--;
					}

				PrevNode = dn;
				}
			}
		else
			goto Failed;

		// Also unlocks
		PopLock();
		}

	return( OK );

Failed:
	PopLock();
	return( ERROR );
	}


/***********************************************************************
***
***  LoadFile
***
***	Assumes MasterList is empty.
***	A) Free old Path Table and old main list (if applicable).
***	B) Load option data
***	C) Load Path Table Names and Parent #'s, until a blank line.
***	D) Load entries into a new main list:
***		1) Alloc Node, add to list, load Type #.
***		2) If File, AllocVector space for name, load Extent.
***		3) If Dir, load extent, find name pointer from Path Table
***		   (with extent).
***		4) Using Extent into Path Table, move FirstEntry pointer into
***		   NextInDir, and the Node address into FirstEntry.
***		   This will produce a backwards single linked list representing
***		   the original directory structure.
***	E) Following Path Table, alphabetically sort each string of files.
***
***********************************************************************/

STATUS LoadFile(
	CSTR FileName,
	MLIST *MasterList,
	CSTR SourceName
	)
	{
	REG DIRNODE *DNode;
	REG PATHTABLE *PTPut;
	FILE *File;
	CSTR Err	= NULL;
	CHAR Buffer[ 256 ];
	LONG i;

	// Setup
	RemovePathTable();
	RemoveDNodePool();
	CreateDNodePool();

	Deep = 0;

	// load them
	if ( ! ( File = fopen( FileName, "rb" ) ) )
		goto Failed;

	// Requester for Abort
	if ( ! ( AbortWin = InitAbortWin( &AbortBox, Interface->IFontReq, "ISOCD", "Loading Text File", NULL ) ) )
		ERR( Err, "Cannot obtain requester" );

	// Box for ShowPath text
	InitShowPath();

	// Load Options
	if ( fscanf( File,
		     "%ld %ld %ld %ld\n"
		     "%ld %ld\n"
		     "%ld %ld %ld %ld %ld\n"
		     "%ld %ld %ld %ld\n",
		     &SortOrder, &SortReverse, &SortGroup, &GroupReverse,
		     &NumPVD, &BaseSector,
		     &CacheData, &CacheDir, &PoolLock, &PoolFile, &Retries,
		     &DirectReadOn, &FastSearch, &TradeMark, &Speed ) != 15 )
		goto Failed;

	READ_BUF( File, Buffer, IDVol );
	READ_BUF( File, Buffer, IDSet );
	READ_BUF( File, Buffer, IDPub );
	READ_BUF( File, Buffer, IDPrep );
	READ_BUF( File, Buffer, IDApp );
	READ_BUF( File, Buffer, SplitFileName );
	READ_BUF( File, Buffer, ImageName );

	// Load Path Table
	// # of Dirs and SourceName
	if ( fgets( Buffer, 256, File ) == 0 )
		goto Failed;

	// Text File header (# of dirs and SourceName)
	Buffer[ ( strlen( Buffer ) - 1 ) ] = NIL;
	sscanf( Buffer, "%d", &Dirs );
	strcpy( SourceName, ( Buffer + 5 ) );

	CreatePathTable();
	PTPut = PathTable;

	// Load in dirs to PathTable
	SPAN( i, Dirs )
		{
		if ( CheckAbortWin( AbortWin ) )
			goto Failed;

		if ( fgets( Buffer, 256, File ) == 0 )
			goto Failed;

		// Check for premature division
		if ( Buffer[ 0 ] == '\n' )
			goto Failed;

		// Set Parent (position in table denotes all else)
		// DirEntry/FirstEntry is set when loading files
		if ( sscanf( Buffer, " %d", &PTPut->Parent ) != 1 )
			goto Failed;

		PTPut++;
		}

	// Division
	if ( ( fgets( Buffer, 256, File ) == 0 ) || ( Buffer[ 0 ] != '\n' ) )
		goto Failed;

	// Load Entries
	Files = 0;
	while ( fgets( Buffer, 256, File ) )
		{
		if ( CheckAbortWin( AbortWin ) )
			goto Failed;

		// Get Name and Extent (remove '\n' first)
		Buffer[ ( strlen( Buffer ) - 1 ) ] = NIL;
		DNode = MakeDNode( MasterList, ( Buffer + 6 ), 0, 0, ET_FILE, AT_TAIL );

		if ( sscanf( ( Buffer + 1 ), "%d", &DNode->Extent ) != 1 )
			goto Failed;

		// Proof Extent
		if ( ( DNode->Extent < 0 ) || ( DNode->Extent > Dirs ) )
			ERR( Err, "Corrupted file" );

		PTPut = &PathTable[ ( DNode->Extent - 1 ) ];

		switch ( Buffer[ 0 ] )
			{
			case ( 'F' ):
				AS( DNode, ET_FILE );
				DNode->NextInDir = PTPut->FirstEntry;
				PTPut->FirstEntry = DNode;
				Files++;
				break;

			case ( 'D' ):
				AS( DNode, ET_DIR );
				PTPut->DirEntry = DNode;

				// Dir belongs to it's parent
				// (except for Root Dir, of course!)
				if ( DNode->Extent > 1 )
					{
					PTPut = &PathTable[ ( PTPut->Parent - 1 ) ];
					DNode->NextInDir = PTPut->FirstEntry;
					PTPut->FirstEntry = DNode;
					}
				break;

			case ( 'P' ):
				AS( DNode, ET_PATHTABLE );
				break;

			case ( 'C' ):
				AS( DNode, ET_CDTV_TM );
				DNode->Size = SizeOfFile( TMFileName );
				TMFileNode  = DNode;
				break;

			case ( 'E' ):
				AS( DNode, ET_EMPTY );
				if ( sscanf( ( Buffer + 6 ), "%d", &DNode->Size ) != 1 )
					goto Failed;

				// Wastes a little space, but forces a legible name
				AllocVector( ( strlen( END_STR ) + 1 ), (void **) &DNode->Next.ln_Name );
				strcpy( DNode->Next.ln_Name, END_STR );
				break;

			case ( 'H' ):
				AS( DNode, ET_HEADERS );
				DNode->Size = ( ( 16 + NumPVD + 1 ) * SECTOR_SIZE );
				break;

			default:
				goto Failed;
			}
		}

	fclose( File ), File = 0;

	// Check for TradeMark
	if ( TradeMark && TMFileNode )
		{
		if ( ! TMFileName[ 0 ] )
			InitTM( MasterList, SourceName );

		TMFileNode->Size = SizeOfFile( TMFileName );
		}

	// Now check all entries
	if ( CheckList( MasterList, SourceName ) )
		goto Failed;

	if ( DeterminePositions( MasterList ) )
		{
		UnSetRootName();
		goto Failed;
		}

	UnSetRootName();

	if ( ( ! Batch ) && ( MissingFiles || ExtraFiles ) )
		{
		sprintf( Buffer, "There were %ld missing file(s) and %ld extra files", MissingFiles, ExtraFiles );
		TellUser( Interface, Buffer );
		}

	Finish();
	return( OK );

Failed:
	if ( File )
		fclose( File );

	Finish();
	TellUser( Interface, "Did not finish loading file" );

	// Clear out resources
	SourceName[ 0 ] = NIL;
	NewList( (LIST *) MasterList );

	return( ERROR );
	}


/***********************************************************************
***
***  MakeDNode
***
***********************************************************************/

DIRNODE *MakeDNode(
	MLIST *DList,
	CSTR Name,
	LONG Size,
	LONG Date,
	enum EntryType EType,
	enum Position Pos
	)
	{
	DIRNODE *n = AllocNode( &DirNodePool );

	AllocVector( ( strlen( Name ) + 1 ), (void **) &n->Next.ln_Name );
	strcpy( n->Next.ln_Name, Name );

	n->Extent = 0;
	n->Size   = Size;
	n->Date   = Date;
	AS( n, EType );

	// Where does it go?
	switch ( Pos )
		{
		case ( AT_HEAD ):
			AddHead( (LIST *) DList, (NODE *) n );
			break;

		case ( AT_TAIL ):
			AddTail( (LIST *) DList, (NODE *) n );
			break;

		case ( AT_BEFORE_TAIL ):
			Insert( (LIST *) DList, (NODE *) n,
				(NODE *) DList->mlh_TailPred->mln_Pred );
			break;

		case ( AT_AFTER_HEAD ):
			Insert( (LIST *) DList, (NODE *) n,
				(NODE *) DList->mlh_Head );
			break;

		default:
		case ( AT_NOWHERE ):
			break;
		}

	return( n );
	}


/***********************************************************************
***
***  PopLock
***
***	returns TRUE if removed a lock
***
***********************************************************************/

PRIVATE FLAG PopLock(
	void
	)
	{
	BPTR lock;

	if ( ( LockMeter - 1 ) < 0 )
		return( FALSE );

	if ( lock = LockStack[ --LockMeter ] )
		UnLock( lock );

	return( TRUE );
	}


/***********************************************************************
***
***  PushLock
***
***********************************************************************/

PRIVATE void PushLock(
	BPTR lock
	)
	{
	ASSERT( ( LockMeter < MAX_LOCKS ), "Too many directories deep" );

	LockStack[ LockMeter++ ] = lock;
	}


/***********************************************************************
***
***  QuitDir
***
***********************************************************************/

void QuitDir(
	void
	)
	{
	QuitAbortWin( AbortWin ), AbortWin = NULL;

	RemovePathTable();
	RemoveDListPool();
	RemoveDNodePool();
	RemoveLockList();
	}


/***********************************************************************
***
***  RemoveDListPool
***
***********************************************************************/

PRIVATE void RemoveDListPool(
	void
	)
	{
	ShrinkPool( &DirListPool );
	}


/***********************************************************************
***
***  RemoveDNodePool
***
***********************************************************************/

PRIVATE void RemoveDNodePool(
	void
	)
	{
	ShrinkPool( &DirNodePool );
	}


/***********************************************************************
***
***  RemoveLockList
***
***********************************************************************/

PRIVATE void RemoveLockList(
	void
	)
	{
	while ( PopLock() )
		;
	}


/***********************************************************************
***
***  RemovePathTable
***
***********************************************************************/

PRIVATE void RemovePathTable(
	void
	)
	{
	if ( PathTable )
		{
		FreeSysMem( PathTable );
		PathTable = NULL;
		}
	}


/***********************************************************************
***
***  SaveFile
***
***	A) Save Option data
***	B) Save Dirs and SourceName (for ease of rebuilding PathTable)
***	C) Save Path Table Name and Parent # for all Path Table entries.
***	D) Save blank line as separator.
***	E) Loop through main list, saving Extent #, Type #, and Name.
***
***	This can be optimized later.
***
***********************************************************************/

void SaveFile(
	CSTR FileName,
	MLIST *MasterList,
	CSTR SourceName
	)
	{
	FILE *File;
	CSTR Err = NULL;
	LONG i;

	// save them
	if ( SizeOfFile( FileName ) )
		if ( ! AskUser( Interface, "%s exists, replace?", "Replace|Cancel", FileName ) )
			return;

	if ( ! ( File = fopen( FileName, "wb" ) ) )
		ERR( Err, "Cannot open file" );

	// Requester for Abort
	if ( ! ( AbortWin = InitAbortWin( NULL, Interface->IFontReq, "ISOCD", "Saving Text File", NULL ) ) )
		ERR( Err, "Cannot obtain requester" );

	// Save Options
	if ( fprintf( File, 
		      "%ld %ld %ld %ld\n"
		      "%ld %ld\n"
		      "%ld %ld %ld %ld %ld\n"
		      "%ld %ld %ld %ld\n",
		      SortOrder, SortReverse, SortGroup, GroupReverse,
		      NumPVD, BaseSector,
		      CacheData, CacheDir, PoolLock, PoolFile, Retries,
		      SAVE_STATE( DirectReadOn ),
		      SAVE_STATE( FastSearch ),
		      SAVE_STATE( TradeMark ),
		      SAVE_STATE( Speed ) ) < 30 )
		goto Failed;

	if ( fprintf( File, "%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
			IDVol, IDSet, IDPub, IDPrep, IDApp, SplitFileName, ImageName ) < 14 )
		goto Failed;

	// Save PathTable
	if ( fprintf( File, "%04.4d\t%s\n", Dirs, SourceName ) < 7 )
		goto Failed;

	SPAN( i, Dirs )
		if ( fprintf( File, " %04.4d\t%s\n",
			     PathTable[ i ].Parent,
			     PathTable[ i ].DirEntry->Next.ln_Name ) < 8 )
			goto Failed;

	if ( fprintf( File, "\n" ) < 1 )
		goto Failed;

	// Save Files
	if ( WalkList( MasterList, SaveFileNode, (LONG) File ) )
		goto Failed;

	QuitAbortWin( AbortWin ), AbortWin = NULL;
	fclose( File );
	return;

Failed:
	if ( Err )
		TellUser( Interface, Err );

	QuitAbortWin( AbortWin ), AbortWin = NULL;

	if ( File )
		fclose( File );
	}


/***********************************************************************
***
***  SaveFileNode
***
***	REMEMBER: TypeNode needs to be updated with enum EntryType.
***
***********************************************************************/

PRIVATE STATUS SaveFileNode(
	MNODE *Node,
	LONG File
	)
	{
	PRIVATE TypeNode[ 16 ] = { 'F', 'D', 'P', 'C', 'E', 'H', '0', '1',
				   '2', '3', '4', '5', '6', '7', '8', '9' };
	CHAR Buffer[ 30 ];
	REG DIRNODE *DNode = (DIRNODE *) Node;
	REG LONG Num;

	if ( CheckAbortWin( AbortWin ) )
		return( ERROR );

	Num = fprintf( (FILE *) File, "%c%04.4d\t",
			TypeNode[ IS_MASK( DNode ) ],
			DNode->Extent );

	if ( Num < 6 )
		return( ERROR );

	if ( IS_EMPTY( DNode ) )
		{
		sprintf( Buffer, "%-9d", DNode->Size );
		Num = fprintf( (FILE *) File, "%s\n", Buffer );
		}
	else
		Num = fprintf( (FILE *) File, "%s\n", DNode->Next.ln_Name );

	return( ( ( Num < 2 ) ? ERROR : OK ) );
	}


/***********************************************************************
***
***  ScanDir
***
***	Read a directory and transfer each element into a dir list.
***
***********************************************************************/

PRIVATE MLIST *ScanDir(
	CSTR Path,
	LONG *DirDate
	)
	{
	REG MLIST *DList = NULL;
	REG BPTR lock;

	ShowPath( Path );

	if ( lock = Lock( Path, ACCESS_READ ) )
		{
		PushLock( lock );

		Examine( lock, Fib );

		if ( DirDate )
			*DirDate = JUL_SEC( Fib->fib_Date );

		// This SHOULD be a directory
		if ( Fib->fib_DirEntryType > 0 )
			{
			DList = AllocDirList();

			// Add it's entries at this time
			while ( ExNext( lock, Fib ) )
				{
				if ( CheckAbortWin( AbortWin ) )
					{
					PopLock();
					return( NULL );
					}

				MakeDNode( DList, Fib->fib_FileName, Fib->fib_Size,
					   JUL_SEC( Fib->fib_Date ),
					   ( ( Fib->fib_DirEntryType > 0 ) ? ET_DIR : ET_FILE ),
					   AT_TAIL );
				}
			}

		// Also unlocks
		PopLock();
		}

	return( DList );
	}


/***********************************************************************
***
***  ShowPath
***
***********************************************************************/

PRIVATE void ShowPath(
	CSTR Path
	)
	{
	PRIVATE CHAR Buffer[ 60 ];

	if ( AbortWin )
		{
		sprintf( Buffer, "%-57.57s", Path );
		Move( &AbortRP, 12, AbortY );
		Text( &AbortRP, Buffer, 57 );
		}
	}


/***********************************************************************
***
***  SortCmpAlpha
***
***********************************************************************/

PRIVATE int SortCmpAlpha(
	DIRNODE **a,
	DIRNODE **b
	)
	{
	return( stricmp( (*a)->Next.ln_Name, (*b)->Next.ln_Name ) );
	}


/***********************************************************************
***
***  SortCmpAlphaRev
***
***********************************************************************/

PRIVATE int SortCmpAlphaRev(
	DIRNODE **a,
	DIRNODE **b
	)
	{
	return( SortCmpAlpha( b, a ) );
	}


/***********************************************************************
***
***  SortCmpDate
***
***********************************************************************/

PRIVATE int SortCmpDate(
	DIRNODE **a,
	DIRNODE **b
	)
	{
	return( (int) ( (*a)->Date - (*b)->Date ) );
	}


/***********************************************************************
***
***  SortCmpDateRev
***
***********************************************************************/

PRIVATE int SortCmpDateRev(
	DIRNODE **a,
	DIRNODE **b
	)
	{
	return( SortCmpDate( b, a ) );
	}


/***********************************************************************
***
***  SortCmpDirs
***
***********************************************************************/

PRIVATE int SortCmpDirs(
	DIRNODE **a,
	DIRNODE **b
	)
	{
	REG STATE ADir = IS_DIR( *a );
	REG STATE BDir = IS_DIR( *b );

	if ( ADir && ( ! BDir ) )
		return( -1 );
	else
		if ( BDir && ( ! ADir ) )
			return( 1 );
		else
			return( (*SortFuncPtr)( a, b ) );
	}


/***********************************************************************
***
***  SortCmpDirsRev
***
***	Note switching of -1 and 1, remember that SortFuncPtr cannot
***	be reversed (that's handled by itself)
***
***********************************************************************/

PRIVATE int SortCmpDirsRev(
	DIRNODE **a,
	DIRNODE **b
	)
	{
	REG STATE ADir = IS_DIR( *a );
	REG STATE BDir = IS_DIR( *b );

	if ( ADir && ( ! BDir ) )
		return( 1 );
	else
		if ( BDir && ( ! ADir ) )
			return( -1 );
		else
			return( (*SortFuncPtr)( a, b ) );
	}


/***********************************************************************
***
***  SortCmpExtension
***
***********************************************************************/

PRIVATE int SortCmpExtension(
	DIRNODE **a,
	DIRNODE **b
	)
	{
	REG CSTR ExtA = FindExt( (*a)->Next.ln_Name );
	REG CSTR ExtB = FindExt( (*b)->Next.ln_Name );
	REG Diff;

	if ( ExtA && ( ! ExtB ) )
		return( -1 );

	if ( ( ! ExtA ) && ExtB )
		return( 1 );

	if ( ExtA && ExtB )
		if ( SortFuncPtr == SortCmpAlpha )
			{
			Diff = stricmp( ExtA, ExtB );
			if ( Diff == 0 )
				Diff = stricmp( (*a)->Next.ln_Name, (*b)->Next.ln_Name );

			return( Diff );
			}
		else
			if ( SortFuncPtr == SortCmpAlphaRev )
				{
				Diff = stricmp( ExtB, ExtA );
				if ( Diff == 0 )
					Diff = stricmp( (*b)->Next.ln_Name, (*a)->Next.ln_Name );

				return( Diff );
				}

	return( (*SortFuncPtr)( a, b ) );
	}


/***********************************************************************
***
***  SortCmpExtensionRev
***
***	Note switching of -1 and 1, remember that SortFuncPtr cannot
***	be reversed (that's handled by itself)
***
***********************************************************************/

PRIVATE int SortCmpExtensionRev(
	DIRNODE **a,
	DIRNODE **b
	)
	{
	REG CSTR ExtA = FindExt( (*a)->Next.ln_Name );
	REG CSTR ExtB = FindExt( (*b)->Next.ln_Name );
	REG Diff;

	if ( ExtA && ( ! ExtB ) )
		return( 1 );

	if ( ( ! ExtA ) && ExtB )
		return( -1 );

	if ( ExtA && ExtB )
		if ( SortFuncPtr == SortCmpAlpha )
			{
			Diff = stricmp( ExtA, ExtB );
			if ( Diff == 0 )
				Diff = stricmp( (*a)->Next.ln_Name, (*b)->Next.ln_Name );

			return( Diff );
			}
		else
			if ( SortFuncPtr == SortCmpAlphaRev )
				{
				Diff = stricmp( ExtB, ExtA );
				if ( Diff == 0 )
					Diff = stricmp( (*b)->Next.ln_Name, (*a)->Next.ln_Name );

				return( Diff );
				}

	return( (*SortFuncPtr)( a, b ) );
	}


/***********************************************************************
***
***  SortCmpFNone
***
***********************************************************************/

PRIVATE int SortCmpFNone(
	DIRNODE **a,
	DIRNODE **b
	)
	{
	return( (*SortFuncPtr)( a, b ) );
	}


/***********************************************************************
***
***  SortCmpFNoneRev
***
***	Remember that SortFuncPtr cannot be reversed (that's handled by
***	itself)
***
***********************************************************************/

PRIVATE int SortCmpFNoneRev(
	DIRNODE **a,
	DIRNODE **b
	)
	{
	return( (*SortFuncPtr)( a, b ) );
	}


/***********************************************************************
***
***  SortCmpInfo
***
***	".info"
***
***********************************************************************/

PRIVATE int SortCmpInfo(
	DIRNODE **a,
	DIRNODE **b
	)
	{
	REG CSTR Ptr;
	REG FLAG A_Info = FALSE;
	REG FLAG B_Info = FALSE;

	Ptr = ( *a )->Next.ln_Name;
	if ( Ptr && ( strlen( Ptr ) >= INFO_LEN ) )
		A_Info = ( stricmp( ( Ptr + strlen( Ptr ) - INFO_LEN ), INFO_STR ) == 0 );

	Ptr = ( *b )->Next.ln_Name;
	if ( Ptr && ( strlen( Ptr ) >= INFO_LEN ) )
		B_Info = ( stricmp( ( Ptr + strlen( Ptr ) - INFO_LEN ), INFO_STR ) == 0 );

	if ( A_Info && ( ! B_Info ) )
		return( -1 );
	else
		if ( B_Info && ( ! A_Info ) )
			return( 1 );
		else
			return( (*SortFuncPtr)( a, b ) );
	}


/***********************************************************************
***
***  SortCmpInfoRev
***
***	".info"
***
***	Note switching of -1 and 1, remember that SortFuncPtr cannot
***	be reversed (that's handled by itself)
***
***********************************************************************/

PRIVATE int SortCmpInfoRev(
	DIRNODE **a,
	DIRNODE **b
	)
	{
	REG CSTR Ptr;
	REG FLAG A_Info = FALSE;
	REG FLAG B_Info = FALSE;

	Ptr = ( *a )->Next.ln_Name;
	if ( Ptr && ( strlen( Ptr ) >= INFO_LEN ) )
		A_Info = ( stricmp( ( Ptr + strlen( Ptr ) - INFO_LEN ), INFO_STR ) == 0 );

	Ptr = ( *b )->Next.ln_Name;
	if ( Ptr && ( strlen( Ptr ) >= INFO_LEN ) )
		B_Info = ( stricmp( ( Ptr + strlen( Ptr ) - INFO_LEN ), INFO_STR ) == 0 );

	if ( A_Info && ( ! B_Info ) )
		return( 1 );
	else
		if ( B_Info && ( ! A_Info ) )
			return( -1 );
		else
			return( (*SortFuncPtr)( a, b ) );
	}


/***********************************************************************
***
***  SortCmpNone
***
***********************************************************************/

PRIVATE int SortCmpNone(
	DIRNODE **a,
	DIRNODE **b
	)
	{
	return( 0 );
	}


/***********************************************************************
***
***  SortCmpNoneRev
***
***********************************************************************/

PRIVATE int SortCmpNoneRev(
	DIRNODE **a,
	DIRNODE **b
	)
	{
	return( 0 );
	}


/***********************************************************************
***
***  SortCmpSize
***
***********************************************************************/

PRIVATE int SortCmpSize(
	DIRNODE **a,
	DIRNODE **b
	)
	{
	return( (int) ( (*a)->Size - (*b)->Size ) );
	}


/***********************************************************************
***
***  SortCmpSizeRev
***
***********************************************************************/

PRIVATE int SortCmpSizeRev(
	DIRNODE **a,
	DIRNODE **b
	)
	{
	return( SortCmpSize( b, a ) );
	}


/***********************************************************************
***
***  SortLoadDir
***
***	Sort a single linked list (NextInDir) into case insensitve
***	alphabetic order (ISO).  Used after LoadFile (Flat list is
***	already irrevocably in place).
***
***********************************************************************/

PRIVATE DIRNODE *SortLoadDir(
	DIRNODE *TopNode,
	int (*SortFunc)( DIRNODE **, DIRNODE ** )
	)
	{
	DIRNODE	*dn;
	DIRNODE	**sort;
	DIRNODE	**sortbase;
	LONG	Length;
	LONG	i;

	// Count
	for ( Length = 0, dn = TopNode; dn; dn = dn->NextInDir )
		Length++;

	if ( Length < 2 )
		return( TopNode );

	// Sorting memory
	sortbase = AllocSort( Length );

	// Setup array of pointers to nodes:
	dn = TopNode;
	sort = sortbase;
	SPAN( i, Length )
		{
		*sort++ = dn;
		dn = dn->NextInDir;
		}

	// Sort nodes:
	qsort( (CSTR) sortbase, Length, sizeof( DIRNODE * ), SortFunc );

	// Rebuild the list:
	sort = sortbase;
	TopNode = dn = (*sort++);

	SPAN( i, ( Length - 1 ) )
		{
		dn->NextInDir = (*sort++);
		dn = dn->NextInDir;
		}

	dn->NextInDir = NULL;

	FreeSort( sortbase, Length );

	return( TopNode );
	}


/***********************************************************************
***
***  SortScanDir
***
***	Sort a directory linked list into case insensitve alphabetic
***	order (ISO).  Used after ScanDir, etc.
***
***********************************************************************/

PRIVATE void SortScanDir(
	MLIST *DList,
	int (*SortFunc)( DIRNODE **, DIRNODE ** )
	)
	{
	DIRNODE	*dn;
	DIRNODE	**sort;
	DIRNODE	**sortbase;
	LONG	Length;
	LONG	i;

	Length = CountNodes( DList );

	// Sorting memory
	sortbase = AllocSort( Length );

	// Setup array of pointers to nodes:
	dn = (DIRNODE *) DList->mlh_Head;
	sort = sortbase;
	SPAN( i, Length )
		{
		*sort++ = dn;
		dn = (DIRNODE *) ( dn->Next.ln_Succ );
		}

	// Sort nodes:
	qsort( (CSTR) sortbase, Length, sizeof( DIRNODE * ), SortFunc );

	// Rebuild the list:
	NewList( (LIST *) DList );
	sort = sortbase;
	SPAN( i, Length )
		AddTail( (LIST *) DList, (NODE *) (*sort++) );

	FreeSort( sortbase, Length );
	}
