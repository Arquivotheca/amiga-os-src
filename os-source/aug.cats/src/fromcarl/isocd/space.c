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
***	Space.c
***
***  PURPOSE:
***
***	Add/Ðel node for empty space
***
***  QUOTE:
***	"... the final frontier..."
***
***  HISTORY:
***
***	0.01 2622 Ken Yeast	Created.
***	0.02 2626 Ken Yeast	Changed interface logic around
***				SpaceAdd/Del
***	0.03 2730 Ken Yeast	SpaceAdd/DelGFunc -> SpaceAdd/DelMFunc
***				AskSpaceGFunc removed
***	0.04 2821 Ken Yeast	InitInterface changed (Tags)
***
************************************************************************/


/***********************************************************************
***
***  Functions
***
***	CountEmpties
***	SpaceAdd
***	SpaceAddMFunc
***	SpaceDel
***	SpaceDelMFunc
***
************************************************************************/


#include <work/Standard.h>

#include <exec/lists.h>

#include <string.h>

#include <proto/exec.h>

#include <work/Support.h>
#include <work/Interface.h>
#include <work/List.h>
#include <work/Custom.h>

#include "ISOCD.h"


/***********************************************************************
***
***  Definitions
***
************************************************************************/

enum SpaceGadgets
	{
	GAD_S_SPACE,
	GAD_S_OK,
	GAD_S_CANCEL,
	MAX_SPACE_GAD
	};


/***********************************************************************
***
***  Function Prototypes
***
***********************************************************************/

PRIVATE LONG	CountEmpties( MLIST * );
PRIVATE STATUS	SpaceAdd( MNODE *, LONG );
PRIVATE STATUS	SpaceDel( MNODE *, LONG );


/***********************************************************************
***
***  External Dependencies
***
***********************************************************************/

IMPORT MLIST	MasterDirList;


/***********************************************************************
***
***  Module Variables
***
***********************************************************************/

PRIVATE ULONG	Space		= 16;
PRIVATE STATE	DoAction	= FALSE;
PRIVATE STATE	AnyChanged	= FALSE;
PRIVATE DIRNODE	*FirstEmpty	= NULL;
PRIVATE MNODE	*LastNode	= NULL;
PRIVATE LONG	Count		= 0;


/***********************************************************************
***
***  CountEmpties
***
***********************************************************************/

PRIVATE LONG CountEmpties(
	MLIST *DirList
	)
	{
	REG LONG Count = 0;
	REG DIRNODE *DNode;

	for ( DNode = (DIRNODE *) DirList->mlh_Head;
	      DNode->Next.ln_Succ;
	      DNode = (DIRNODE *) DNode->Next.ln_Succ )
		if ( IS_EMPTY( DNode ) )
			{
			if ( ! FirstEmpty )
				FirstEmpty = DNode;

			Count++;
			}

	return( Count );
	}


/***********************************************************************
***
***  SpaceAdd
***
***	OK to modify list by adding after node, WalkList can handle this.
***
***********************************************************************/

PRIVATE STATUS SpaceAdd(
	MNODE *Node,
	LONG Arg
	)
	{
	REG DIRNODE *DNode;

	if ( IS_HIGH( Node ) )
		{
		HIGH_OFF( Node );

		// Cannot add after last node
		if ( Node == LastNode )
			return( ERROR );

		AnyChanged = TRUE;
		DNode = MakeDNode( &MasterDirList, END_STR, ( Space * SECTOR_SIZE ),
				   0, ET_EMPTY, AT_NOWHERE );

		Insert( (LIST *) &MasterDirList, (NODE *) DNode, (NODE *) Node );
		}

	return( OK );
	}


/***********************************************************************
***
***  SpaceAddMFunc
***
***	Add: Add after highlighted spots, otherwise after first entry
***
***********************************************************************/

STATUS SpaceAddMFunc(
	INTERFACE *Interface,
	UWORD Code
	)
	{
	// Space windows
	PRIVATE TAG	TagW_Space[]		=
		{
		{ WA_Left,	0			},
		{ WA_Top,	0			},
		{ WA_Width,	145			},
		{ WA_Height,	55			},
		{ WA_IDCMP,	( NORMAL_IDCMP | GADGET_IDCMP | MENU_IDCMP )	},
		{ WA_Flags,	NORMAL_WFLAGS		},
		{ WA_Title,	(ULONG) "Empty Space"	},
		TAG_DONE
		};
	// Space Menu
	PRIVATE struct NewMenu SpaceMenu[] =
		{
		{ NM_TITLE, "Project",		 0 , 0, 0, 0		},
		{  NM_ITEM, "Done",		"Q", 0, 0, DoneMFunc	},
		{ NM_END, 0,			 0 , 0, 0, 0		}
		};
	// Space Action tags...
	PRIVATE TAG	TagA_S_Space[]		=
		{
		{ GAD_TA_Update,	(ULONG) &Space		},
		TAG_DONE
		};
	PRIVATE TAG	TagA_S_OK[]		=
		{
		{ GAD_TA_SetTrue,	(ULONG) &DoAction	},
		{ GAD_TA_Exit,		(ULONG) TRUE		},
		TAG_DONE
		};
	PRIVATE TAG	TagA_S_Cancel[]		=
		{
		{ GAD_TA_SetFalse,	(ULONG) &DoAction	},
		{ GAD_TA_Exit,		(ULONG) TRUE		},
		TAG_DONE
		};

	// Space Gadgets!!!!
	// Desired Gadgets (non-interactive first)
	//	ID, Kind, X, Y, SizeX, SizeY, Flags, TitleText, ReqTags, ActionTags
	PRIVATE GAD_REQ SpaceGadTable[]		=
		{
	{ GAD_S_SPACE,	G_INTEGER, 80,  4,  60, 6, 0, "Sectors:",	NULL, TagA_S_Space	},
	{ GAD_S_OK,	G_BUTTON,  10, 22,  60, 6, 0, "OK",		NULL, TagA_S_OK		},
	{ GAD_S_CANCEL,	G_BUTTON,  75, 22,  60, 6, 0, "CANCEL",		NULL, TagA_S_Cancel	},
		};

	// User Proofing
	StatusError( NULL );
	if ( ! HAS_NODE_LIST( &MasterDirList ) )
		{
		StatusError( "Need to Examine first" );
		return( OK );
		}

	DoAction = FALSE;

	// Setup
	TagW_Space[ 0 ].ti_Data = WINDOW_LEFT( Interface, 10 );
	TagW_Space[ 1 ].ti_Data = WINDOW_TOP( Interface, 10 );

	if ( DoSimpleInterface( NULL, TagW_Space, SpaceMenu, SpaceGadTable,
				MAX_SPACE_GAD, NULL, NULL ) )
		TellUser( Interface, "Space window not available" );

	if ( DoAction )
		{
		AnyChanged = FALSE;
		LastNode   = MasterDirList.mlh_TailPred;

		WalkList( &MasterDirList, SpaceAdd, 0 );

		if ( ! AnyChanged )
			MakeDNode( &MasterDirList, END_STR, ( Space * SECTOR_SIZE ),
				   0, ET_EMPTY, AT_AFTER_HEAD );

		InformDirLoaded( TRUE );
		}

	return( OK );
	}


/***********************************************************************
***
***  SpaceDel
***
***	OK to modify list by remove(), WalkList can handle this.
***
***********************************************************************/

PRIVATE STATUS SpaceDel(
	MNODE *Node,
	LONG Arg
	)
	{
	// Check for HIGH, EMPTY, and Count is an optimization
	if ( IS_HIGH( Node ) )
		{
		HIGH_OFF( Node );

		// Cannot remove last node, <Empty> MUST be there!
		if ( Node == LastNode )
			return( ERROR );

		if ( IS_EMPTY( (DIRNODE *) Node ) && ( Count > 1 ) )
			{
			// This wastes some memory... (until next examine)
			AnyChanged = TRUE;
			Remove( (NODE *) Node );

			if ( --Count <= 1 )
				return( ERROR );
			}
		}

	return( OK );
	}


/***********************************************************************
***
***  SpaceDelMFunc
***
***	Del: Delete highlighted spaces, if none, delete first empty
***
***********************************************************************/

STATUS SpaceDelMFunc(
	INTERFACE *Interface,
	UWORD Code
	)
	{
	// User Proofing
	StatusError( NULL );
	if ( ! HAS_NODE_LIST( &MasterDirList ) )
		{
		StatusError( "Need to Examine first" );
		return( OK );
		}

	LastNode = MasterDirList.mlh_TailPred;

	FirstEmpty = NULL;
	if ( ( Count = CountEmpties( &MasterDirList ) ) <= 1 )
		{
		StatusError( "No more Empty Spaces to remove" );
		return( OK );
		}

	AnyChanged = FALSE;

	WalkList( &MasterDirList, SpaceDel, 0 );

	// This wastes some memory... (until next examine)
	if ( ! AnyChanged )
		Remove( (NODE *) FirstEmpty );

	InformDirLoaded( TRUE );

	return( OK );
	}
