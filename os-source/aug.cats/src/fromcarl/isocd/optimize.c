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
***	Optimize.c
***
***  PURPOSE:
***
***	Optimize Layout list according to a statistics file gathered by
***	OptCD/CDFS.  This action can only be done once.
***
***  QUOTE:
***
***  HISTORY:
***
***	0.01 2731 Ken Yeast	Created.
***	0.02 2b04 Ken Yeast	OK, I'm back
***				implementing simple "float most hit to top
***				of list", then will add "order" later
***				FindBlock
***				OptMSortFunc
***				Hey! It works!!!!
***				RefreshBackWindow
***				Filter
***				Still works, nice.
***	0.03 2b05 Ken Yeast	Adding Grouping order method
***				Optimize -> OptimizeMost
***				OptimizeGroup
***				OptimizeDirs
***				OptGSortFunc
***				OptGPSortFunc
***
************************************************************************/


/***********************************************************************
***
***  Functions
***
***	Filter
***	FindBlock
***	OptimizeDirs
***	OptimizeGroup
***	OptimizeMost
***	OptimizePanel
***	OptGSortFunc
***	OptGPSortFunc
***	OptMSortFunc
***	RefreshBackWindow
***
************************************************************************/


#include <work/Standard.h>
#include <work/System.h>

#include <exec/memory.h>
#include <exec/lists.h>
#include <dos/dos.h>

#include <string.h>
#include <limits.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>

#include <work/Support.h>
#include <work/Interface.h>
#include <work/List.h>
#include <work/FSE.h>

#include "ISOCD.h"


/***********************************************************************
***
***  Definitions
***
***********************************************************************/

// Main window
enum OptGadgets
	{
	GAD_O_FILE,
	GAD_O_USE_DIR_NC,
	GAD_O_USE_DIR_C,
	GAD_O_USE_BLOCK_NC,
	GAD_O_USE_BLOCK_C,
	GAD_O_USE_FILE,
	GAD_O_USE_LOW_READ,
	GAD_O_TYPE,
	GAD_O_OPT,
	GAD_O_CANCEL,
	MAX_OPT_GAD
	};

enum OType
	{
	OT_MOST,
	OT_GROUP,
	OT_DIRS,
	};

#define		MAX_PAIR	10


/***********************************************************************
***
***  Structures
***
***********************************************************************/

// Used by OT_MOST algorithm
typedef struct
	{
	DIRNODE	*DNode;
	ULONG	Counter;
	} OPTM_NODE;

#define		OPTM_NODE_SIZE	sizeof( OPTM_NODE )

// Used by OT_GROUP algorithm
typedef struct
	{
	DIRNODE	*DNode;
	struct 
		{
		UWORD	Offset;
		UBYTE	Counter;		; watch rollover UCHAR_MAX
		} Pair[ MAX_PAIR ];
	} OPTG_MODE;

#define		OPTG_NODE_SIZE	sizeof( OPTG_NODE )

typedef struct
	{
	DIRNODE	*EntryA;
	DIRNODE	*EntryB;
	UWORD	Counter;
	} OPTG_PAIR;

#define		OPTG_PAIR_SIZE	sizeof( OPTG_PAIR )


/***********************************************************************
***
***  Function Prototypes
***
***********************************************************************/

PRIVATE STATUS	Filter( UWORD );
PRIVATE OPTM_NODE * _ASM FindBlock( _A0 OPTM_NODE *, _D0 ULONG );
PRIVATE STATUS	OptimizeDirs( INTERFACE * );
PRIVATE STATUS	OptimizeGroup( INTERFACE * );
PRIVATE STATUS	OptimizeMost( INTERFACE * );
PRIVATE int	OptGSortFunc( OPTG_MODE *, OPTG_MODE * );
PRIVATE int	OptGPSortFunc( OPTG_PAIR *, OPTG_PAIR * );
PRIVATE int	OptMSortFunc( OPTM_NODE *, OPTM_NODE * );
PRIVATE void	RefreshBackWindow( void );


/***********************************************************************
***
***  External Dependencies
***
***********************************************************************/

IMPORT INTERFACE *Interface;

IMPORT MLIST	MasterDirList;


/***********************************************************************
***
***  Module Variables
***
***********************************************************************/

PRIVATE STATE	DoAction	= FALSE;
PRIVATE CHAR	StatName[ FILENAME_LEN ];

PRIVATE enum	OType OptType	= OT_MOST;

PRIVATE ULONG	Nodes		= 0;

PRIVATE FLAG	UseDirNoCache	= TRUE;
PRIVATE FLAG	UseDirCache	= FALSE;
PRIVATE FLAG	UseBlockNoCache	= TRUE;
PRIVATE FLAG	UseBlockCache	= FALSE;
PRIVATE FLAG	UseFile		= TRUE;
PRIVATE FLAG	UseLowRead	= FALSE;


/***********************************************************************
***
***  Filter
***
***********************************************************************/

PRIVATE STATUS Filter(
	UWORD Code
	)
	{
	switch ( Code )
		{
		case ( FSEB_PACKETS ):
			// Ignored...
			break;

		case ( FSEB_DIR_NOCACHE ):
			if ( UseDirNoCache )
				return( OK );
			break;

		case ( FSEB_DIR_CACHE ):
			if ( UseDirCache )
				return( OK );
			break;

		case ( FSEB_BLOCK_NOCACHE ):
			if ( UseBlockNoCache )
				return( OK );
			break;

		case ( FSEB_BLOCK_CACHE ):
			if ( UseBlockCache )
				return( OK );
			break;

		case ( FSEB_FILE ):
			if ( UseFile )
				return( OK );
			break;

		case ( FSEB_LOW_READ ):
			if ( UseLowRead )
				return( OK );
			break;

		default:
			break;
		}

	return( ERROR );
	}


/***********************************************************************
***
***  FindBlock
***
***	This algorithm depends on the last node never being accessed,
***	that's ok since it is the empty buffer.
***
***********************************************************************/

PRIVATE OPTM_NODE * _ASM FindBlock(
	_A0 OPTM_NODE *OptBuffer,
	_D0 ULONG Block
	)
	{
	REG ULONG i;

	SPAN( i, Nodes )
		{
		if ( OptBuffer->DNode->Position > Block )
			return( OptBuffer - 1 );

		OptBuffer++;
		}

	return( NULL );
	}


/***********************************************************************
***
***  OptimizeDirs
***
***********************************************************************/

PRIVATE STATUS OptimizeDirs(
	INTERFACE *Interface
	)
	{
	REG DIRNODE *Dn;
	REG DIRNODE *LastDn;
	REG DIRNODE *Next;
	REG DIRNODE *RootDir;
	CSTR Err		= NULL;

	if ( ! ( RootDir = (DIRNODE *) FindName( (LIST *) &MasterDirList, ROOT_STR ) ) )
		ERR( Err, "List is corrupt" );

	// Naturally skip <Root Dir>
	LastDn = RootDir;

	for ( Dn = (DIRNODE *) MasterDirList.mlh_Head;
	      Dn->Next.ln_Succ;
	      Dn = Next )
		{
		Next = (DIRNODE *) Dn->Next.ln_Succ;

		if ( ( Dn != RootDir ) && IS_DIR( Dn ) )
			{
			Remove( (NODE *) Dn );
			Insert( (LIST *) &MasterDirList, (NODE *) Dn, (NODE *) LastDn );
			LastDn = Dn;
			}
		}

	return( OK );

Failed:
	if ( Err )
		TellUser( Interface, Err );

	return( ERROR );
	}


/***********************************************************************
***
***  OptimizeGroup
***
***********************************************************************/

PRIVATE STATUS OptimizeGroup(
	INTERFACE *Interface
	)
	{
	CHAR Buffer[ 128 ];
	REG DIRNODE *Dn;
	REG OPTG_NODE *On;
	REG OPTG_PAIR *Pn;
	ULONG Block		= 0;
	ULONG LastBlock		= 0;
	ULONG LBlock		= 0;
	ULONG GBlock		= 0;
	ULONG Offset;
	ULONG Pairs;
	SHORT i;
	SHORT ii;
	CSTR Arg		= ( Buffer + 2 );
	OPTG_NODE *OptBuffer	= NULL;
	OPTG_PAIR *PairBuffer	= NULL;
	FILE *StatFile		= NULL;
	CSTR Err		= NULL;
	FLAG SomethingFound	= FALSE;

	// Check for file
	if ( ! StatName[ 0 ] )
		ERR( Err, "Statistics file needed" );

	// Allocate/Set re-arranger memory/nodes
	Nodes = CountNodes( &MasterDirList );

	if ( ! ( OptBuffer = AllocMem( ( Nodes * OPTG_NODE_SIZE ), MEMF_CLEAR ) ) )
		ERR( Err, "Not enough memory for optimization" );

	On = OptBuffer;
	FOR_EACH_NODE( &MasterDirList, Dn )
		( On++ )->DNode = Dn;

	// Load in text file, changing nodes info
	if ( ! ( StatFile = fopen( StatName, "rb" ) ) )
		ERR( Err, "Cannot open statistics file" );

	setvbuf( StatFile, NULL, _IOFBF, ( 20 * KILO ) );

	for ( EVER )
		{
		if ( fgets( Buffer, 128, StatFile ) == 0 )
			break;

		if ( Filter( ( Buffer[ 0 ] - '0' ) ) )
			continue;

		// [Code] [Block]
		// (Ignore FILE/LOW_READ's [LengthBlocks]
		if ( sscanf( Arg, "%ld", &Block ) != 1 )
			ERR( Err, "Line error" );

		// First read is not a pair
		if ( LastBlock == 0 )
			{
			LastBlock = Block;
			continue;
			}

		// Ignore same accesses
		if ( LastBlock == Block )
			continue;

		// LastBlock/Block access is a pair
		// Find lesser block, find this in list, find other in list
		// In lesser entry, find if offset is available,
		//	inc counter, otherwise use next (if avail), inc
		//	counter
		On	= FindBlock( OptBuffer, min( Block, LastBlock ) );
		Offset	= ( FindBlock( OptBuffer, max( Block, LastBlock ) ) - OptBuffer );

		SPAN( i, MAX_PAIR )
			{
			// Place in list if reached open entry
			if ( On->Pair[ i ].Offset == 0 )
				{
				On->Pair[ i ].Offset = Offset;
				On->Pair[ i ].Counter++;
				break;
				}

			// Inc counter if found (previous pairing)
			if ( On->Pair[ i ].Offset == Offset )
				{
				On->Pair[ i ].Counter++;
				break;
				}
			}

		LastBlock = Block;
		SomethingFound = TRUE;
		}

	fclose( StatFile ), StatFile = NULL;

	if ( ! SomethingFound )
		ERR( Err, "Nothing in statistics file was useful" );

	// sort pairing hits to top
	qsort( (CSTR) OptBuffer, Nodes, OPTG_NODE_SIZE, OptGSortFunc );

	// Count all pairs
	On	= OptBuffer;
	Pairs	= 0;
	SPAN( i, Nodes )
		{
		if ( On->Pair[ 0 ].Counter == 0 )
			break;

		SPAN( ii, MAX_PAIR )
			if ( On->Pair[ ii ].Counter )
				Pairs++;
			else
				break;

		On++;
		}

	if ( Pairs == 0 )
		ERR( Err, "Nothing in statistics file was useful" );

	// Allocate Pairs and transfer them...
	if ( ! ( PairBuffer = AllocMem( ( Nodes * OPTG_PAIR_SIZE ), MEMF_CLEAR ) ) )
		ERR( Err, "Not enough memory for optimization" );

	On = OptBuffer;
	Pn = PairBuffer;
	ii = 0;
	SPAN( i, Pairs )
		{
		if ( ( ii > MAX_PAIR ) || ( On->Pair[ ii ].Counter == 0 ) )
			{
			On++;
			ii = 0;
			}

		Pn->EntryA  = On->DNode;
		Pn->EntryB  = OptBuffer[ On->Pair[ ii ].Offset ].DNode;
		Pn->Counter = On->Pair[ ii ].Counter;

		Pn++;
		}

	// OptBuffer is no longer needed
	FreeMem( OptBuffer, ( Nodes * OPTG_NODE_SIZE ) ), OptBuffer = NULL;

	// Sort Pair Block in order of counters
	qsort( (CSTR) PairBuffer, Nodes, OPTG_PAIR_SIZE, OptGPSortFunc );

	// NOW rearrange the list.  We have a list of Pairs, in order of most
	// accessed.
	// Scan Pair Block
	//	If EntryA is not marked:
	//		if EntryB is not marked:
	//			Move EntryB after EntryA, Mark EntryA/EntryB
	//		else
	//			Move EntryB and all following marked
	//			entries after EntryA, Mark EntryA
	//	else
	//		do nothing
	Pn = PairBuffer;
	SPAN( i, Pairs )
		{
		if ( ! IS_HIGH( Pn->EntryA ) )
			if ( ! IS_HIGH( Pn->EntryB ) )
				{
				Remove( (NODE *) Pn->EntryB );
				Insert( (LIST *) &MasterDirList, (NODE *) Pn->EntryB, (NODE *) Pn->EntryA );
				HIGH_ON( Pn->EntryA );
				HIGH_ON( Pn->EntryB );
				}
			else
				{
				}
		else
			;

		Pn++;
		}

	// UnMark all of our markings
	Pn = PairBuffer;
	SPAN( i, Pairs )
		{
		if ( IS_HIGH( Pn->EntryA ) )
			HIGH_OFF( Pn->EntryA );

		if ( IS_HIGH( Pn->EntryB ) )
			HIGH_OFF( Pn->EntryB );
		}

	// Cleanup
	FreeMem( PairBuffer, ( Pairs * OPTG_PAIR_SIZE ) );

	return( OK );

Failed:
	if ( Err )
		TellUser( Interface, Err );

	if ( StatFile )
		fclose( StatFile );

	if ( OptBuffer )
		FreeMem( OptBuffer, ( Nodes * OPTG_NODE_SIZE ) );

	if ( PairBuffer )
		FreeMem( PairBuffer, ( Pairs * OPTG_PAIR_SIZE ) );

	return( ERROR );
	}


/***********************************************************************
***
***  OptimizeMost
***
***********************************************************************/

PRIVATE STATUS OptimizeMost(
	INTERFACE *Interface
	)
	{
	CHAR Buffer[ 128 ];
	REG DIRNODE *Dn;
	REG OPTM_NODE *On;
	REG OPTM_NODE *OnEnd;
	REG DIRNODE *RootDir;
	ULONG Block;
	ULONG Length;
	ULONG i;
	UWORD Code;
	CSTR Arg		= ( Buffer + 2 );
	OPTM_NODE *OptBuffer	= NULL;
	FILE *StatFile		= NULL;
	CSTR Err		= NULL;
	FLAG SomethingFound	= FALSE;

	// Check for file
	if ( ! StatName[ 0 ] )
		ERR( Err, "Statistics file needed" );

	// Allocate/Set re-arranger memory/nodes
	Nodes = CountNodes( &MasterDirList );

	if ( ! ( OptBuffer = AllocMem( ( Nodes * OPTM_NODE_SIZE ), MEMF_CLEAR ) ) )
		ERR( Err, "Not enough memory for optimization" );

	On = OptBuffer;
	FOR_EACH_NODE( &MasterDirList, Dn )
		( On++ )->DNode = Dn;

	// Load in text file, changing nodes info
	if ( ! ( StatFile = fopen( StatName, "rb" ) ) )
		ERR( Err, "Cannot open statistics file" );

	setvbuf( StatFile, NULL, _IOFBF, ( 20 * KILO ) );

	for ( EVER )
		{
		if ( fgets( Buffer, 128, StatFile ) == 0 )
			break;

		Code = ( Buffer[ 0 ] - '0' );

		if ( Filter( Code ) )
			continue;

		switch ( Code )
			{
			case ( FSEB_DIR_NOCACHE ):
			case ( FSEB_DIR_CACHE ):
			case ( FSEB_BLOCK_NOCACHE ):
			case ( FSEB_BLOCK_CACHE ):
				// [Code] [Block]
				if ( sscanf( Arg, "%ld", &Block ) != 1 )
					ERR( Err, "Line error" );

				On = FindBlock( OptBuffer, Block );
				On->Counter++;
				break;

			case ( FSEB_FILE ):
			case ( FSEB_LOW_READ ):
				// [Code] [Block] [LengthBlocks]
				if ( sscanf( Arg, "%ld %ld", &Block, &Length ) != 2 )
					ERR( Err, "Line error" );

				// Mark all covered - usually will be just one entry
				// In fact, this condition may never occur...
				On	= FindBlock( OptBuffer, Block );
				OnEnd	= FindBlock( OptBuffer, ( Block + Length ) );

				for ( ; On <= OnEnd; On++ )
					On->Counter++;
				break;
			}

		SomethingFound = TRUE;
		}

	fclose( StatFile ), StatFile = NULL;

	if ( ! SomethingFound )
		ERR( Err, "Nothing in statistics file was useful" );

	// rearrange nodes
	// sort hits to top
	qsort( (CSTR) OptBuffer, Nodes, OPTM_NODE_SIZE, OptMSortFunc );

	// walk hits back into list in order, after <Root Dir>
	if ( ! ( RootDir = (DIRNODE *) FindName( (LIST *) &MasterDirList, ROOT_STR ) ) )
		ERR( Err, "List is corrupt" );

	Dn = RootDir;
	On = OptBuffer;
	SPAN( i, Nodes )
		{
		if ( On->Counter == 0 )
			break;

		// Naturally skip <Root Dir>
		if ( On->DNode != RootDir )
			{
			Remove( (NODE *) On->DNode );
			Insert( (LIST *) &MasterDirList, (NODE *) On->DNode, (NODE *) Dn );
			Dn = On->DNode;
			}

		On++;
		}

	// Cleanup
	FreeMem( OptBuffer, ( Nodes * OPTM_NODE_SIZE ) );

	return( OK );

Failed:
	if ( Err )
		TellUser( Interface, Err );

	if ( StatFile )
		fclose( StatFile );

	if ( OptBuffer )
		FreeMem( OptBuffer, ( Nodes * OPTM_NODE_SIZE ) );

	return( ERROR );
	}


/***********************************************************************
***
***  OptimizePanel
***
***********************************************************************/

STATUS OptimizePanel(
	INTERFACE *Interface
	)
	{
	// Optimize windows
	// Note: Ignore Dir stats?
	PRIVATE TAG	TagW_Opt[]		=
		{
		{ WA_Left,	0			},
		{ WA_Top,	0			},
		{ WA_Width,	410			},
		{ WA_Height,	115			},
		{ WA_IDCMP,	( NORMAL_IDCMP | GADGET_IDCMP | MENU_IDCMP )	},
		{ WA_Flags,	NORMAL_WFLAGS		},
		{ WA_Title,	(ULONG) "Optimize"	},
		TAG_DONE
		};
	// Opt Menu
	PRIVATE struct NewMenu OptMenu[] =
		{
		{ NM_TITLE, "Project",		 0 , 0, 0, 0		},
		{  NM_ITEM, "Done",		"Q", 0, 0, DoneMFunc	},
		{ NM_END, 0,			 0 , 0, 0, 0		}
		};
	// Opt Action tags...
	PRIVATE TAG	TagA_O_OptFile[]	=
		{
		{ GAD_TA_BusyWait,	0				},
		{ GAD_TA_GF_Name,	(ULONG) "Statistics File"	},
		{ GAD_TA_GetFile,	(ULONG) -1			},
		{ GAD_TA_Update,	(ULONG) StatName		},
		{ GAD_TA_Stub,		(ULONG) RefreshBackWindow	},
		TAG_DONE
		};
	PRIVATE TAG	TagA_O_Update[][ 2 ]	=
		{
		{ { GAD_TA_Update_B, (ULONG) &UseDirNoCache	}, TAG_DONE },
		{ { GAD_TA_Update_B, (ULONG) &UseDirCache	}, TAG_DONE },
		{ { GAD_TA_Update_B, (ULONG) &UseBlockNoCache	}, TAG_DONE },
		{ { GAD_TA_Update_B, (ULONG) &UseBlockCache	}, TAG_DONE },
		{ { GAD_TA_Update_B, (ULONG) &UseFile		}, TAG_DONE },
		{ { GAD_TA_Update_B, (ULONG) &UseLowRead	}, TAG_DONE },
		{ { GAD_TA_Update,   (ULONG) &OptType		}, TAG_DONE },
		};
	PRIVATE TAG	TagA_O_Opt[]		=
		{
		{ GAD_TA_BusyWait,	0				},
		{ GAD_TA_SetTrue,	(ULONG) &DoAction		},
		{ GAD_TA_Exit,		(ULONG) TRUE			},
		TAG_DONE
		};
	PRIVATE TAG	TagA_O_Cancel[]		=
		{
		{ GAD_TA_SetFalse,	(ULONG) &DoAction		},
		{ GAD_TA_Exit,		(ULONG) TRUE			},
		TAG_DONE
		};
	// Opt Request tags...
	PRIVATE TAG	TagR_O_OptFile[]	=
		{
		{ GAD_TC_Title,		(ULONG) "Statistics"		},
		TAG_DONE
		};
	PRIVATE CSTR TypeText[]			=
		{
		"Float Most Used to Top",
		"Group Entries According to Access",
		"Just Float All Dirs to Top - No File",
		NULL
		};
	PRIVATE TAG	TagR_O_Type[]		=
		{
		{ GTCY_Labels,		(ULONG) TypeText		},
		TAG_DONE
		};
	// Opt Gadgets!!!!
	// Desired Gadgets (non-interactive first)
	//	ID, Kind, X, Y, SizeX, SizeY, Flags, TitleText, ReqTags, ActionTags
	PRIVATE GAD_REQ OptGadTable[]		=
		{
	{ GAD_O_FILE,		G_C_BUTTON,100,  5,296, 4, 0, NULL,		TagR_O_OptFile, TagA_O_OptFile	},

	{ GAD_O_USE_DIR_NC,	G_CHECKBOX, 35, 20, 60, 0, RIGHT, "Directory No Cache",NULL,TagA_O_Update[ 0 ] },
	{ GAD_O_USE_DIR_C,	G_CHECKBOX,225, 20, 60, 0, RIGHT, "Directory Cache",NULL,	TagA_O_Update[ 1 ] },
	{ GAD_O_USE_BLOCK_NC,	G_CHECKBOX, 35, 35, 60, 0, RIGHT, "Block No Cache",	NULL,	TagA_O_Update[ 2 ] },
	{ GAD_O_USE_BLOCK_C,	G_CHECKBOX,225, 35, 60, 0, RIGHT, "Block Cache",	NULL,	TagA_O_Update[ 3 ] },
	{ GAD_O_USE_FILE,	G_CHECKBOX, 35, 50, 60, 0, RIGHT, "File",		NULL,	TagA_O_Update[ 4 ] },
	{ GAD_O_USE_LOW_READ,	G_CHECKBOX,225, 50, 60, 0, RIGHT, "Low Level Read",	NULL,	TagA_O_Update[ 5 ] },

	{ GAD_O_TYPE,		G_CYCLE,    70, 65,330, 6, 0,	"Method",	TagR_O_Type, TagA_O_Update[ 6 ] },

	{ GAD_O_OPT,		G_BUTTON,   70, 85,100, 6, 0, "Optimize",	NULL, TagA_O_Opt	},
	{ GAD_O_CANCEL,		G_BUTTON,  240, 85,100, 6, 0, "CANCEL",		NULL, TagA_O_Cancel	},
		};

	// Ask for options
	DoAction = FALSE;

	// Setup
	TagW_Opt[ 0 ].ti_Data = WINDOW_LEFT( Interface, 10 );
	TagW_Opt[ 1 ].ti_Data = WINDOW_TOP( Interface, 10 );

	if ( DoSimpleInterface( NULL, TagW_Opt, OptMenu, OptGadTable,
				MAX_OPT_GAD, NULL, NULL ) )
		TellUser( Interface, "Optimize window not available" );

	if ( DoAction )
		switch ( OptType )
			{
			case ( OT_MOST ):
				return( OptimizeMost( Interface ) );

			case ( OT_GROUP ):
				return( OptimizeGroup( Interface ) );

			case ( OT_DIRS ):
				return( OptimizeDirs( Interface ) );
			}
	else
		return( ERROR );
	}


/***********************************************************************
***
***  OptGSortFunc
***
***********************************************************************/

PRIVATE int OptGSortFunc(
	OPTG_NODE *a,
	OPTG_NODE *b
	)
	{
	return( (int) ( (int) b->Pair[ 0 ].Counter - (int) a->Pair[ 0 ].Counter ) );
	}


/***********************************************************************
***
***  OptGPSortFunc
***
***********************************************************************/

PRIVATE int OptGPSortFunc(
	OPTG_PAIR *a,
	OPTG_PAIR *b
	)
	{
	return( (int) ( (int) b->Counter - (int) a->Counter ) );
	}


/***********************************************************************
***
***  OptMSortFunc
***
***********************************************************************/

PRIVATE int OptMSortFunc(
	OPTM_NODE *a,
	OPTM_NODE *b
	)
	{
	return( (int) ( (int) b->Counter - (int) a->Counter ) );
	}


/***********************************************************************
***
***  RefreshBackWindow
***
***********************************************************************/

PRIVATE void RefreshBackWindow(
	void
	)
	{
	Refresh( Interface );
	}
