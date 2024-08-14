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
***	Find.c
***
***  PURPOSE:
***
***	Find a node in our dir list
***
***  QUOTE:
***	"All right.  We waste him.  No offense..."
***		- Corporal Hicks, Aliens
***
***  HISTORY:
***
***	0.01 2619 Ken Yeast	Created.
***				Next/Prev, works nice!
***	0.02 2730 Ken Yeast	Find changed to menu
***	0.03 2821 Ken Yeast	InitInterface changed (Tags)
***	0.04 2917 Ken Yeast	Interface changes
***				BoxGad unneeded
***
************************************************************************/


/***********************************************************************
***
***  Functions
***
***	FindMFunc
***	FindNextNode
***	FindPrevNode
***	HelpMFunc
***	NextMFunc
***	PrevMFunc
***	SetPattern
***
************************************************************************/


#include <work/Standard.h>

#include <exec/lists.h>
#include <dos/dos.h>

#include <string.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>

#include <work/Support.h>
#include <work/Interface.h>
#include <work/List.h>

#include "ISOCD.h"


/***********************************************************************
***
***  Definitions
***
************************************************************************/

enum FindGadgets
	{
	GAD_F_FIND,
	GAD_F_NEXT,
	GAD_F_PREV,
	GAD_F_DONE,
	MAX_FIND_GAD
	};


/***********************************************************************
***
***  Function Prototypes
***
***********************************************************************/

PRIVATE void	FindNextNode( void );
PRIVATE void	FindPrevNode( void );
PRIVATE STATUS	HelpMFunc( INTERFACE *, UWORD );
PRIVATE STATUS	NextMFunc( INTERFACE *, UWORD );
PRIVATE STATUS	PrevMFunc( INTERFACE *, UWORD );
PRIVATE void	SetPattern( void );


/***********************************************************************
***
***  External Dependencies
***
***********************************************************************/

IMPORT MLIST	MasterDirList;

IMPORT INTERFACE	*Interface;


/***********************************************************************
***
***  Module Variables
***
***********************************************************************/

PRIVATE CHAR	SearchName[ FILENAME_LEN ];
PRIVATE CHAR	MatchBuffer[ 260 ];
PRIVATE STATE	PatternAvailable		= FALSE;
PRIVATE NODE	*CurrentNode			= NULL;


/***********************************************************************
***
***  FindMFunc
***
***********************************************************************/

STATUS FindMFunc(
	INTERFACE *Interface,
	UWORD Code
	)
	{
	// Find windows
	PRIVATE TAG	TagW_Find[]		=
		{
		{ WA_Left,	0			},
		{ WA_Top,	0			},
		{ WA_Width,	270			},
		{ WA_Height,	55			},
		{ WA_IDCMP,	( NORMAL_IDCMP | GADGET_IDCMP | MENU_IDCMP )	},
		{ WA_Flags,	NORMAL_WFLAGS		},
		{ WA_Title,	(ULONG) "Find"	},
		TAG_DONE
		};
	// Find Menu
	PRIVATE struct NewMenu FindMenu[] =
		{
		{ NM_TITLE, "Project",		 0 , 0, 0, 0		},
		{  NM_ITEM, "Next",		"N", 0, 0, NextMFunc	},
		{  NM_ITEM, "Prev",		"P", 0, 0, PrevMFunc	},
		{  NM_ITEM, "Help",		"H", 0, 0, HelpMFunc	},
		{  NM_ITEM, NM_BARLABEL,	 0 , 0, 0, 0		},
		{  NM_ITEM, "Done",		"Q", 0, 0, DoneMFunc	},
	
		{ NM_END, 0,			 0 , 0, 0, 0		}
		};
	// Find Action tags...
	PRIVATE TAG	TagA_F_Find[]		=
		{
		{ GAD_TA_Update,	(ULONG) SearchName	},
		{ GAD_TA_Stub,		(ULONG) SetPattern	},
		{ GAD_TA_Stub,		(ULONG) FindNextNode	},
		TAG_DONE
		};
	PRIVATE TAG	TagA_F_Next[]		=
		{
		{ GAD_TA_Stub,		(ULONG) FindNextNode	},
		TAG_DONE
		};
	PRIVATE TAG	TagA_F_Prev[]		=
		{
		{ GAD_TA_Stub,		(ULONG) FindPrevNode	},
		TAG_DONE
		};
	PRIVATE TAG	TagA_F_Done[]		=
		{
		{ GAD_TA_Exit,		(ULONG) TRUE		},
		TAG_DONE
		};
	// Find Gadgets!!!!
	// Desired Gadgets (non-interactive first)
	//	ID, Kind, X, Y, SizeX, SizeY, Flags, TitleText, ReqTags, ActionTags
	PRIVATE GAD_REQ FindGadTable[]		=
		{
	{ GAD_F_FIND,	G_STRING,  65,  4, 190, 6, 0, "Find:",	NULL, TagA_F_Find	},
	{ GAD_F_NEXT,	G_BUTTON,  10, 22,  80, 6, 0, "Next",	NULL, TagA_F_Next	},
	{ GAD_F_PREV,	G_BUTTON,  95, 22,  80, 6, 0, "Prev",	NULL, TagA_F_Prev	},
	{ GAD_F_DONE,	G_BUTTON, 180, 22,  80, 6, 0, "Done",	NULL, TagA_F_Done	},
		};

	// User Proofing
	StatusError( NULL );
	if ( ! HAS_NODE_LIST( &MasterDirList ) )
		{
		StatusError( "Need to Examine first" );
		return( OK );
		}

	// Setup
	TagW_Find[ 0 ].ti_Data = WINDOW_LEFT( Interface, 10 );
	TagW_Find[ 1 ].ti_Data = WINDOW_TOP( Interface, 10 );

	PatternAvailable = FALSE;
	MatchBuffer[ 0 ] = SearchName[ 0 ] = NIL;

	if ( DoSimpleInterface( NULL, TagW_Find, FindMenu, FindGadTable,
				MAX_FIND_GAD, NULL, NULL ) )
		{
		TellUser( Interface, "Find window not available" );
		return( OK );
		}

	return( OK );
	}


/***********************************************************************
***
***  FindNextNode
***
***********************************************************************/

PRIVATE void FindNextNode(
	void
	)
	{
	if ( ! PatternAvailable )
		return;

	while ( ! MatchPatternNoCase( MatchBuffer, CurrentNode->ln_Name ) )
		{
		if ( CurrentNode == (NODE *) MasterDirList.mlh_TailPred )
			{
			CurrentNode = (NODE *) MasterDirList.mlh_Head;
			break;
			}

		CurrentNode = CurrentNode->ln_Succ;
		}

	GadgetCustomMethod( Interface, GAD_BOX, GAD_TC_L_Jump, (ULONG) CurrentNode, TAG_DONE );

	if ( CurrentNode == (NODE *) MasterDirList.mlh_TailPred )
		CurrentNode = (NODE *) MasterDirList.mlh_Head;
	else
		CurrentNode = CurrentNode->ln_Succ;
	}


/***********************************************************************
***
***  FindPrevNode
***
***********************************************************************/

PRIVATE void FindPrevNode(
	void
	)
	{
	if ( ! PatternAvailable )
		return;

	while ( ! MatchPatternNoCase( MatchBuffer, CurrentNode->ln_Name ) )
		{
		if ( CurrentNode == (NODE *) MasterDirList.mlh_Head )
			break;

		CurrentNode = CurrentNode->ln_Pred;
		}

	GadgetCustomMethod( Interface, GAD_BOX, GAD_TC_L_Jump, (ULONG) CurrentNode, TAG_DONE );

	if ( CurrentNode == (NODE *) MasterDirList.mlh_Head )
		CurrentNode = (NODE *) MasterDirList.mlh_TailPred;
	else
		CurrentNode = CurrentNode->ln_Pred;
	}


/***********************************************************************
***
***  HelpMFunc
***
***********************************************************************/

PRIVATE STATUS HelpMFunc(
	INTERFACE *Interface,
	UWORD Code
	)
	{
	TellUser( Interface,
		"Will find file in list, while Next/Prev continues search.\n"
		"You can use any AmigaDos pattern matching, such as\n"
		"\"#?.info\", etc.  The search is case insensitive." );
	return( OK );
	}


/***********************************************************************
***
***  NextMFunc
***
***********************************************************************/

PRIVATE STATUS NextMFunc(
	INTERFACE *Interface,
	UWORD Code
	)
	{
	FindNextNode();
	return( OK );
	}


/***********************************************************************
***
***  PrevMFunc
***
***********************************************************************/

PRIVATE STATUS PrevMFunc(
	INTERFACE *Interface,
	UWORD Code
	)
	{
	FindPrevNode();
	return( OK );
	}


/***********************************************************************
***
***  SetPattern
***
***********************************************************************/

PRIVATE void SetPattern(
	void
	)
	{
	ParsePatternNoCase( SearchName, MatchBuffer, 256 );
	PatternAvailable = TRUE;
	CurrentNode	 = (NODE *) MasterDirList.mlh_Head;
	}
