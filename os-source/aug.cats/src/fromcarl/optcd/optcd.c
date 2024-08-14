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
***	OptCD.c
***
***  PURPOSE:
***
***	Deal with FSE messages from CDFS.
***
***  QUOTE:
***	"You Nexus, huh?  I design you eyes."
***
***	"Chu, if only you could see what I have seen with your eyes..."
***		Bladerunner
***
***  HISTORY:
***
***	0.01 2129 Leo L. Scwhab	Created.
***	0.02 2708 Ken Yeast	Recreated Leo's changes onto new CDFS
***	0.03 2709 Ken Yeast	Got it all to work, (pool with Allocate
***				was not aligned)
***				Converted to our coding.
***	0.04 2713 Ken Yeast	Changed interface around
***				(Memory is on our side)
***				Changed to different set of data to monitor
***				distinguish between cache/no cache
***				output to a file
***				a lot of changes
***	0.05 2714 Ken Yeast	Cleaned up, everything works.
***	0.06 2715 Ken Yeast	Interface on top... (it's so cute)
***				Lots of changes...
***	0.07 2716 Ken Yeast	Load/Save working (real friendly, safe saves)
***				OptimizeGFunc
***	0.08 2717 Ken Yeast	Moved FSE record stuff out
***	0.09 2720 Ken Yeast	changed spec
***				removed LayoutLoad/SaveGFunc
***				User proofing all over
***	0.10 2721 Ken Yeast	Changed data being monitored (and methods)
***				Interface rearranged to reflect this
***	     2731 Ken Yeast	Done/QuitMFunc removed
***				other small changes
***	0.11 2802 Ken Yeast	Layout/Optimize removed
***				Changed to command line recorder - the files
***				produced are then used by ISOCD.
***				Many changes, uses empty input handler to
***				ID itself (like FakIR)
***				InputFake
***	0.12 2804 Ken Yeast	Tweaks
***	0.13 2a09 Ken Yeast	Notice/etc.
***	0.14 2a20 Ken Yeast	Notice stuff removed (could not get
***				resource stuff to work)
***	0.15 2a27 Ken Yeast	Notice returned (using MsgPort), works fine
***
************************************************************************/


#define	VER 0
#define	REV 15


/***********************************************************************
***
***  Functions
***
***	main
***	CheckAssumptions
***	Init
***	Main
***	OptFile
***	Quit
***	ScanArg
***
************************************************************************/


#include <work/Standard.h>
#include <work/System.h>

#include <string.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include <work/Core.h>
#include <work/Notice.h>

#include "OptCD.h"
#include <work/FSE.h>


/***********************************************************************
***
***  Definitions
***
***********************************************************************/

#define	FILENAME_LEN		64

#define	SetMon( m, v, f )	(m) |= ( ( v ) ? (f) : 0 )

#define	INPUT_NAME		"IH_OptCD.rendezvous"


/***********************************************************************
***
***  Function Prototypes
***
***********************************************************************/

void	main( INT, CSTR * );
void	CheckAssumptions( void );
void	Init( void );
void	Main( void );
void	OptFile( CSTR, CSTR );
void	ScanArg( CSTR );


/***********************************************************************
***
***  Program Usage Information
***
***********************************************************************/

PUBLIC CSTR UsageText[] =
	{
	"PURPOSE: Monitor FSE data from CDFS.\n\nUSAGE: %s [options] <Statistics file> [ON/OFF]\n",
	"  -b<n>     buffer size in K (default 8)",
	"  -m<#...>  events to monitor (default 123 - if not specified)",
	"             0: All DOS Packets",
	"             1: File Read",
	"             2: Dir Read (Not Cache)",
	"             3: Dir Read (Cache)",
	"             4: Block Read (Not Cache)",
	"             5: Block Read (Cache)",
	"             6: Lowest level read",
	"  [ON/OFF]  Force OptCD on or off",
	NULL
	};


/***********************************************************************
***
***  Extern Variables
***
************************************************************************/

IMPORT BPTR	_Backstdout;	// cback.o


/***********************************************************************
***
***  Global Variables
***
***********************************************************************/

// cback.o (we need stdout)
PUBLIC LONG	_BackGroundIO		= TRUE;


/***********************************************************************
***
***  Module Variables
***
***********************************************************************/

// Program
PRIVATE	INT	Ver			= VER;
PRIVATE	INT	Rev			= REV;
PRIVATE	CSTR	ProgTitle		= "\nOptCD %ld.%02ld by Pantaray - Copyright (C) 1992 Commodore International\n";
PRIVATE CSTR	Credits			=
	"Produced by:    Carl Sassenrath\n"
	"Programmed by:  Kenneth Yeast and Leo Schwab\n"
	"Interface by:   Kenneth Yeast\n"
	"\n"
	"Pantaray, Inc. (c) 1992\n"
	"\"All things in motion...\"";

// On/Off/Signature stuff
PRIVATE void	*Notice			= NULL;
PRIVATE STATE	MustBeOn		= FALSE;
PRIVATE STATE	MustBeOff		= FALSE;

// Files
PRIVATE SHORT	FileCount		= 0;
PRIVATE CHAR	StatFileName[ FILENAME_LEN ];

// FSE
PRIVATE FLAG	PacketsBit		= FALSE;
PRIVATE FLAG	FileBit			= FALSE;
PRIVATE FLAG	DirBit			= FALSE;
PRIVATE FLAG	DirCacheBit		= FALSE;
PRIVATE FLAG	BlockBit		= FALSE;
PRIVATE FLAG	BlockCacheBit		= FALSE;
PRIVATE FLAG	LowReadBit		= FALSE;

PRIVATE ULONG	BufferLength		= 8;


/***********************************************************************
***
***  main
***
***	main entry point
***
***********************************************************************/

void main(
	INT argc,
	CSTR argv[]
	)
	{
	// Do NOT close _Backstdout later, close Output()
	SelectOutput( _Backstdout );

	ScanProgArgs( argc, argv, ScanArg );

	CheckAssumptions();

	Init();

	Main();

	Done();
	}


/***********************************************************************
***
***  CheckAssumptions
***
***********************************************************************/

void CheckAssumptions(
	void
	)
	{
	struct Task *OptCDTask;

	if ( OptCDTask = FindNotice( INPUT_NAME ) )
		{
		if ( ! MustBeOn )
			{
			Signal( OptCDTask, SIGBREAKF_CTRL_C );
			Print( "\nOptCD Recorder removed.\n" );
			}

		Done();
		}

	if ( MustBeOff )
		Done();

	// There must be a stat file
	if ( ! StatFileName[ 0 ] )
		Error( "Need a statistics file" );

	// If not set, use these defaults:
	if ( ! ( PacketsBit || FileBit || DirBit || DirCacheBit ||
		 BlockBit || BlockCacheBit || LowReadBit ) )
		FileBit = DirBit = DirCacheBit = TRUE;
	}


/***********************************************************************
***
***  Init
***
***********************************************************************/

void Init(
	void
	)
	{
	if ( ! ( Notice = AddNotice( INPUT_NAME ) ) )
		Error( "Cannot install notice" );

	InitRecord();
	}


/***********************************************************************
***
***  Main
***
***********************************************************************/

void Main(
	void
	)
	{
	ULONG FSEMonitor = 0;

	SetMon( FSEMonitor, PacketsBit,	   FSEF_PACKETS		);
	SetMon( FSEMonitor, FileBit,	   FSEF_FILE		);
	SetMon( FSEMonitor, DirBit,	   FSEF_DIR_NOCACHE	);
	SetMon( FSEMonitor, DirCacheBit,   FSEF_DIR_CACHE	);
	SetMon( FSEMonitor, BlockBit,	   FSEF_BLOCK_NOCACHE	);
	SetMon( FSEMonitor, BlockCacheBit, FSEF_BLOCK_CACHE	);
	SetMon( FSEMonitor, LowReadBit,	   FSEF_LOW_READ	);

	Record( StatFileName, FSEMonitor, BufferLength );
	}


/***********************************************************************
***
***  OptFile
***
***	Look for an optional file name.
***
************************************************************************/

void OptFile(
	CSTR arg,
	CSTR str
	)
	{
	if ( arg[ 0 ] )
		strncpy( str, arg, FILENAME_LEN );
	}


/***********************************************************************
***
***  Quit
***
***********************************************************************/

void Quit(
	INT Code
	)
	{
	static STATE Quitting = FALSE;
	BPTR StdOut;

	if ( Quitting )
		return;

	Quitting = TRUE;

	RemoveNotice( Notice );

	QuitRecord();

	if ( Code )
		Print( "\n%s quit! (code %ld)\n", ProgramName(), Code );

	// _Backstdout was moved to Output(), since we are using cback.o close it.
	if ( StdOut = Output() )
		Close( StdOut );

	// OK, since we detach - need a pause for outputting text...
	Delay( 30 );

	exit( Code );
	}


/***********************************************************************
***
***  ScanArg
***
***	Scan and process a single command line argument.
***
***********************************************************************/

void ScanArg(
	CSTR arg
	)
	{
	CSTR ArgPtr;
	LONG Num;

	switch ( arg[ 0 ] )
		{
		case '-':
			switch ( tolower( arg[ 1 ] ) )
				{
				case 'b':
					Num = atol( &arg[ 2 ] );
					BufferLength = ( ( Num < 8 ) ? 8 : Num );
					break;

				case 'm':
					ArgPtr = ( arg + 2 );

					while ( *ArgPtr )
						{
						switch ( *ArgPtr )
							{
							case '0':
								PacketsBit	= TRUE;
								break;

							case '1':
								FileBit		= TRUE;
								break;

							case '2':
								DirBit		= TRUE;
								break;

							case '3':
								DirCacheBit	= TRUE;
								break;

							case '4':
								BlockBit	= TRUE;
								break;

							case '5':
								BlockCacheBit	= TRUE;
								break;

							case '6':
								LowReadBit	= TRUE;
								break;
							}

						ArgPtr++;
						}
					break;

				default:
					Print( ProgTitle, Ver, Rev );
					Usage( "Unrecognized program option: %s", arg, UsageText );
					break;
				}
			break;

		default:
			if ( stricmp( arg, "ON" ) == 0 )
				MustBeOn = TRUE;
			else
				if ( stricmp( arg, "OFF" ) == 0 )
					MustBeOff = TRUE;
				else
					if ( FileCount == 0 )
						{
						OptFile( arg, StatFileName );
						FileCount++;
						}
			break;

		case '?':
			Print( ProgTitle, Ver, Rev );
			Usage( NULL, NULL, UsageText );
			break;
		}
	}
