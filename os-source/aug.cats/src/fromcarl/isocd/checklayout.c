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
***	CheckLayout.c
***
***  PURPOSE:
***
***	Load/Save layout files for use by OptCD
***
***  QUOTE:
***
***  HISTORY:
***
***	0.01 2716 Ken Yeast	Created from ISOCD/Dir.c.
***	0.02 2720 Ken Yeast	Minor changes
***	0.03 2728 Ken Yeast	InitAbortWin using FontReq
***
************************************************************************/


/***********************************************************************
***
***  Functions
***
***	CreateDNodePool
***	InitLayout
***	LoadFile
***	MakeDNode
***	QuitLayout
***	RemoveDNodePool
***	SaveFile
***	SaveFileNode
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

#include <work/Support.h>
#include <work/Core.h>
#include <work/Dos.h>
#include <work/Interface.h>
#include <work/List.h>
#include <work/Memory.h>

#include "OptCD.h"


/***********************************************************************
***
***  Definitions
***
***********************************************************************/

#define		READ_BUF( f, b, s )	if ( fgets( b, 256, f ) == 0 )		\
						goto Failed;			\
					(b)[ ( strlen( b ) - 1 ) ] = NIL;	\
					strcpy( s, b );



/***********************************************************************
***
***  Function Prototypes
***
***********************************************************************/

PRIVATE void	CreateDNodePool( void );
PRIVATE void	RemoveDNodePool( void );
PRIVATE STATUS	SaveFileNode( MNODE *, LONG );


/***********************************************************************
***
***  Module Variables
***
***********************************************************************/

// Abort
PRIVATE INTERFACE *AbortWin		= NULL;


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
***  InitLayout
***
***********************************************************************/

void InitLayout(
	void
	)
	{
	}


/***********************************************************************
***
***  LoadFile
***
***	Assumes MasterList is empty.
***	A) Free old Path Table and old main list (if applicable).
***	B) Read past option data and path table.  When we save, we'll just
***	   re-read this information.
***	C) Load entries into MLIST
***
***********************************************************************/

STATUS LoadFile(
	INTERFACE *Interface,
	CSTR FileName,
	MLIST *MasterList
	)
	{
	REG DIRNODE *DNode;
	FILE *File;
	CSTR Err	= NULL;
	CHAR Buffer[ 256 ];
	LONG Dirs;
	LONG i;

	// Setup
	RemoveDNodePool();
	CreateDNodePool();

	// load them
	if ( ! ( File = fopen( FileName, "rb" ) ) )
		goto Failed;

	// Requester for Abort
	if ( ! ( AbortWin = InitAbortWin( NULL, Interface->IFontReq, "OptCD", "Loading Text File", NULL ) ) )
		ERR( Err, "Cannot obtain requester" );

	// Read past Options (11 lines)
	SPAN( i, 11 )
		{
		if ( CheckAbortWin( AbortWin ) )
			goto Failed;

		if ( fgets( Buffer, 256, File ) == 0 )
			goto Failed;
		}

	// Read past Path Table
	// (Find # of Dirs to skip)
	if ( fgets( Buffer, 256, File ) == 0 )
		goto Failed;

	// Text File header (# of dirs and SourceName)
	Buffer[ ( strlen( Buffer ) - 1 ) ] = NIL;
	sscanf( Buffer, "%d", &Dirs );

	// Let them know the source
	// NOTE: size dependent (ISOCD uses 4 digits for now)
	InformDirLoaded( ( Buffer + 5 ) );

	// Load in dirs to PathTable
	SPAN( i, Dirs )
		{
		if ( CheckAbortWin( AbortWin ) )
			goto Failed;

		if ( fgets( Buffer, 256, File ) == 0 )
			goto Failed;
		}

	// Division
	if ( ( fgets( Buffer, 256, File ) == 0 ) || ( Buffer[ 0 ] != '\n' ) )
		goto Failed;

	// NOW Load Entries...
	while ( fgets( Buffer, 256, File ) )
		{
		if ( CheckAbortWin( AbortWin ) )
			goto Failed;

		// Get Name and Extent (remove '\n' first)
		Buffer[ ( strlen( Buffer ) - 1 ) ] = NIL;
		DNode = MakeDNode( MasterList, ( Buffer + 6 ), AT_TAIL );

		if ( sscanf( ( Buffer + 1 ), "%d", &DNode->Extent ) != 1 )
			goto Failed;

		// Proof Extent
		if ( ( DNode->Extent < 0 ) || ( DNode->Extent > Dirs ) )
			ERR( Err, "Corrupted file" );

		// Repair this
		switch ( Buffer[ 0 ] )
			{
			case ( 'F' ):
				AS( DNode, ET_FILE );
				break;

			case ( 'D' ):
				AS( DNode, ET_DIR );
				break;

			case ( 'P' ):
				AS( DNode, ET_PATHTABLE );
				break;

			case ( 'C' ):
				AS( DNode, ET_CDTV_TM );
				break;

			case ( 'E' ):
				AS( DNode, ET_EMPTY );
				break;

			case ( 'H' ):
				AS( DNode, ET_HEADERS );
				break;

			default:
				goto Failed;
			}
		}

	fclose( File );
	QuitAbortWin( AbortWin ), AbortWin = NULL;
	return( OK );

Failed:
	if ( File )
		fclose( File );

	QuitAbortWin( AbortWin ), AbortWin = NULL;

	TellUser( Interface, "Did not finish loading file" );

	// Clear out resources
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
	enum Position Pos
	)
	{
	REG DIRNODE *n = AllocNode( &DirNodePool );

	AllocVector( ( strlen( Name ) + 1 ), (void **) &n->Next.ln_Name );
	strcpy( n->Next.ln_Name, Name );

	n->Weight = 0;

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
***  QuitLayout
***
***********************************************************************/

void QuitLayout(
	void
	)
	{
	RemoveDNodePool();
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
***  SaveFile
***
***	1) If the "save" name is the same as the "load" name:
***		A) Rename <file> to <file>.bak
***		B) Transfer Options and PathTable from <file>.bak to <file>
***		C) Write out new order
***		D) Remove <file>.bak
***	2) Otherwise:
***		A) Transfer Options and PathTable from old to new.
***		B) Write out new order
***
***********************************************************************/

void SaveFile(
	INTERFACE *Interface,
	CSTR OldName,
	CSTR NewName,
	MLIST *MasterList
	)
	{
	CHAR Buffer[ 256 ];
	FILE *OldFile;
	FILE *NewFile;
	CSTR Err = NULL;
	LONG Dirs;
	LONG i;
	STATE IdenticalNames;

	// Check if names are identical
	IdenticalNames = ( stricmp( OldName, NewName ) == 0 );

	// Rename old to old.bak if identical
	if ( IdenticalNames )
		{
		strcat( OldName, ".bak" );

		if ( ! Rename( NewName, OldName ) )
			ERR( Err, "Cannot rename file to <file>.bak" );
		}

	if ( ! ( OldFile = fopen( OldName, "rb" ) ) )
		ERR( Err, "Cannot open original file" );

	// check for new
	if ( SizeOfFile( NewName ) )
		if ( ! AskUser( Interface, "%s exists, replace?", "Replace|Cancel", NewName ) )
			return;

	if ( ! ( NewFile = fopen( NewName, "wb" ) ) )
		ERR( Err, "Cannot open new file" );

	// Requester for Abort
	if ( ! ( AbortWin = InitAbortWin( NULL, Interface->IFontReq, "OptCD", "Saving Text File", NULL ) ) )
		ERR( Err, "Cannot obtain requester" );

	// Transfer Options (11 lines)
	SPAN( i, 11 )
		{
		if ( CheckAbortWin( AbortWin ) )
			goto Failed;

		if ( fgets( Buffer, 256, OldFile ) == 0 )
			goto Failed;

		if ( fputs( Buffer, NewFile ) )
			goto Failed;
		}

	// Tranfser Path Table
	// (Find # of Dirs to transfer)
	if ( fgets( Buffer, 256, OldFile ) == 0 )
		goto Failed;

	if ( fputs( Buffer, NewFile ) )
		goto Failed;

	// Text File header (# of dirs and SourceName)
	Buffer[ ( strlen( Buffer ) - 1 ) ] = NIL;
	sscanf( Buffer, "%d", &Dirs );

	// Transfer PathTable dirs
	SPAN( i, Dirs )
		{
		if ( CheckAbortWin( AbortWin ) )
			goto Failed;

		if ( fgets( Buffer, 256, OldFile ) == 0 )
			goto Failed;

		if ( fputs( Buffer, NewFile ) )
			goto Failed;
		}

	// Division
	if ( ( fgets( Buffer, 256, OldFile ) == 0 ) || ( Buffer[ 0 ] != '\n' ) )
		goto Failed;

	// OldFile not needed anymore
	fclose( OldFile ), OldFile = 0;

	if ( fputs( Buffer, NewFile ) )
		goto Failed;

	// Save Entries
	if ( WalkList( MasterList, SaveFileNode, (LONG) NewFile ) )
		goto Failed;

	QuitAbortWin( AbortWin ), AbortWin = NULL;
	fclose( NewFile );

	// Remove old file if identical
	if ( IdenticalNames )
		DeleteFile( OldName );

	// In any case, update name
	strcpy( OldName, NewName );
	return;

Failed:
	if ( Err )
		TellUser( Interface, Err );

	QuitAbortWin( AbortWin ), AbortWin = NULL;

	if ( OldFile )
		fclose( OldFile );

	if ( NewFile )
		fclose( NewFile );
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
	REG DIRNODE *DNode = (DIRNODE *) Node;
	REG LONG Num;

	if ( CheckAbortWin( AbortWin ) )
		return( ERROR );

	Num = fprintf( (FILE *) File, "%c%04.4d\t",
			TypeNode[ IS_MASK( DNode ) ],
			DNode->Extent );

	if ( Num < 6 )
		return( ERROR );

	Num = fprintf( (FILE *) File, "%s\n", DNode->Next.ln_Name );

	return( ( ( Num < 2 ) ? ERROR : OK ) );
	}
