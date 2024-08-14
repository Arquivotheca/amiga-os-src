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
***	ISOCD.c
***
***  PURPOSE:
***
***	Control panel for ISOCD
***
***  QUOTE:
***	"They told me my Commodore would stand up to anything!"
***		- Doctor Love, Earth Girls Are Easy
***		  (when finding his A2000 destroyed)
***
***  HISTORY:
***
***	0.01 2316 Ken Yeast	Created from SimCDPanel.c
***	0.02 2329 Ken Yeast	Interface.c is mutating again...
***	0.03 2420 Ken Yeast	I'm back...
***				InitDirList
***	0.04 2426 Ken Yeast	Oh wow, ListView is ALIVE
***				Hard-Codes for testing
***	0.05 2504 Ken Yeast	ListView is reasonable, back to ISOCD
***	0.06 2505 Ken Yeast	No need for gadtools LISTVIEW
***				options moved here
***				MakeCD moved here
***	0.07 2506 Ken Yeast	File.c added
***				Flatten.c added
***	0.08 2507 Ken Yeast	Changing list methodology
***	0.09 2508 Ken Yeast	New changes to Dir.c working.
***	0.10 2510 Ken Yeast	New changes to File.c
***	0.11 2513 Ken Yeast	Now that creating works, adding remaining
***				stuff
***				MakeCD removed
***				ExamineList
***				InitListView
***				Build
***	0.12 2515 Ken Yeast	Gadgetry connected (somewhat)
***				Adding capability to repeat (different memory
***				management)
***				InitListDir -> InitListView
***				InitListView -> AllowEdit
***	0.13 2519 Ken Yeast	OptFile is here, ugh.
***	0.14 2520 Ken Yeast	PrepareLine
***				Interface totally re-arranged
***				ShowPath
***				LayoutSave/LoadGFunc
***	0.15 2521 Ken Yeast	CheckBreak removed
***				Req abort on scan and build
***				Adjusted to fit 640x200
***	0.16 2522 Ken Yeast	Tweaks, LoadFile changes
***				Source from args (returned)
***				Options
***				Entire window for Options (Interface handles OK!)
***	0.17 2526 Ken Yeast	Flesh out Option window
***				DoneMFunc
***				SetOptions
***				ISO/CDFS Options
***				Various command line options removed
***	0.18 2527 Ken Yeast	Tweaks and twiddles
***				CDFS stuff
***				GAD_O_ID_VOL
***				Option screen rearranged
***				BoxGad
***	0.19 2528 Ken Yeast	More changes
***				CheckForDeviceGFunc
***				Inform user of TotalSize/Dirs/Files
***				Option added to adjust file buffer size
***	0.20 2529 Ken Yeast	ShowPath moved (and gadget)
***				Progress bar stuff removed
***	0.21 2601 Ken Yeast	InformDirLoaded
***				default for image removed, speed off
***				minor interface changes
***	0.22 2602 Ken Yeast	Minor interface internal changes
***				SetOptions no longer needed
***				Sent this to CBM (Ben Phister)
***	0.23 2603 Ken Yeast	Fixing stuff that Ben found
***				CancelGFunc
***				Set/ResetOptions
***				SetCDFSDefaults/GFunc
***				Group (from Sort)
***				GroupReverse
***	0.24 2604 Ken Yeast	Interface internal changes
***				SetSize/DateMFunc
***	     2605 Ken Yeast	Some user "proofing"
***				StatusError
***				GAD_T_STATERR
***				Usage of GetGadgetReturn
***				CheckNewSourceGFunc
***	0.25 2608 Ken Yeast	AdjustForWindow
***				(will try for large window)
***				Verify
***				Show date if file/dir
***	     2609 Ken Yeast	CurrentDirName for source
***				AddCurrentDir
***				Ability to change qualifier for "assemble" in list
***				Ability to choose minimum size window
***				Date print changed to match list
***	     2610 Ken Yeast	Verify temporarily removed, FindNode replaced it
***	0.26 2617 Ken Yeast	Build w/Source but no examine bug fixed
***				Check that stack is at least 10K
***				Dirs deep check for ISO
***	0.27 2618 Ken Yeast	Interface fixes
***				FindGFunc returns
***	     2619 Ken Yeast	FindNodeGFunc
***				all moved to Find.c
***				Find Next/Prev all working fine (ADos pattern matching)
***	0.28 2622 Ken Yeast	SpaceAdd/DelGFunc
***	     2625 Ken Yeast	Fix to ISO for zero length files
***	0.29 2626 Ken Yeast	Fix to LayoutSaveGFunc - check for Examine properly
***				Option to adjust for CDFS dates
***	0.30 2629 Ken Yeast	Sent to CBM (Ben)
***	0.31 2701 Ken Yeast	ReflectInAll to correct intuition bug with
***				failure to activate gadget w/o return or tab
***	     2708 Ken Yeast	Correct UsageText
***	     2722 Ken Yeast	Default to topaz8 font, -f chooses system font
***				-b is now buffer flag
***	0.32 2727 Ken Yeast	Added Alt numbers (reverse order) to PVD/etc.
***				(This should allow Meridian and other systems to
***				 write these images - since they use the intel fields)
***				AddAltPathTable for reverse byte systems
***				Version #'s (";1") for file names
***	0.33 2728 Ken Yeast	Changes to use Interface font everywhere
***	     2730 Ken Yeast	InformDirLoaded needed ReflectAll (thanks Henry!)
***				Changed look to accomodate OptCD changes
***				FindGFunc -> FindMFunc
***				SpaceAdd/DelGFunc -> SpaceAdd/DelMFunc
***	0.34 2731 Ken Yeast	Optimize button added
***				Menus for Add/Del/Find, and changed around
***				Build -> BuildGFunc
***				ExamineList -> ExamineListGFunc
***				Options -> OptionsGFunc
***				Done/QuitMFunc removed
***				MFunc's changed
***	0.35 2804 Ken Yeast	Optimize button ghosted for now
***				Additional path table is -p (was -a)
***				Sent to CBM (Ben)
***	0.36 2810 Ken Yeast	Added busywait tags to some gadgets
***	     2814 Ken Yeast	Custom is changing (Interface internal)
***				G_C_LISTVIEW vs. G_GENERIC
***	     2819 Ken Yeast	ListView became a custom gadget, changes made
***				InitList
***	0.37 2821 Ken Yeast	InitInterface changed (Tags)
***	     2824 Ken Yeast	More minor changes
***	0.38 2917 Ken Yeast	Will not write to volume controlled by cdtv.device
***				(read only)
***				Source defaults to current dir
***				Fixed bug w/enforcer and listview 1st refresh
***				Fixed bug in ISO/CreateImage (if exit while create)
***				AddAltPathTable and AddVersion default to ON
***				Cosmetic touchups required for interface changes
***				ListView gadget was not getting IDCMP_INTUITICKS msgs
***				BoxGad unneeded
***	0.39 2918 Ken Yeast	Load layout file (-l<file>)
***				Auto Build (dangerous!) -b
***				AutoLoadLayout
***				DiskChange after successful build
***	0.40 2a02 Ken Yeast	ISO.c:CopyStrUp was not upper casing international
***				characters properly
***				(Sent to Chris Henry 2a05)
***	0.41 2a13 Ken Yeast	Interface changes to keep consistent with SimCD
***				CheckForLayoutFile
***				Sent to Commodore 2a14
***	0.42 2a28 Ken Yeast	ISOCD was ignoring command line spec of dir to examine
***				FastSearch now defaults to on, since we ALWAYS create
***				a case insensitive dir sort.
***	     2a29 Ken Yeast	Save state as 1 or 0 for CDFS on/off options in layout
***				Option to display sizes rounded in menu
***				SetSizeRMFunc
***	0.43 2b02 Ken Yeast	Entering a volume that does not exist for image
***				was not being properly trapped.
***
************************************************************************/


#define	VER 0
#define	REV 43


/***********************************************************************
***
***  Functions
***
***	main
***	AboutMFunc
***	AddCurrentDir
***	AdjustForWindow
***	AutoLoadLayout
***	BuildGFunc
***	CancelGFunc
***	CheckAssumptions
***	CheckForDevice
***	CheckForLayoutFile
***	CheckNewSourceGFunc
***	ExamineListGFunc
***	InformDirLoaded
***	Init
***	InitList
***	InitListView
***	LayoutLoadGFunc
***	LayoutSaveGFunc
***	Main
***	OptFile
***	OptionsGFunc
***	PrepareLine
***	Quit
***	ResetOptions
***	ScanArg
***	SetCDFSDefaults
***	SetCDFSDefaultsGFunc
***	SetDateMFunc
***	SetOptions
***	SetSizeMFunc
***	SetSizeRMFunc
***	StatusError
***
************************************************************************/


#include <work/Standard.h>

#include <exec/memory.h>
#include <exec/lists.h>
#include <exec/tasks.h>
#include <exec/execbase.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <graphics/gfxmacros.h>
#include <utility/tagitem.h>
#include <libraries/gadtools.h>

#include <string.h>

#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/gadtools.h>
#include <proto/dos.h>
#include <proto/utility.h>

#include <work/Core.h>
#include <work/Support.h>
#include <work/Dos.h>
#include <work/Interface.h>
#include <work/List.h>
#include <work/Memory.h>

#include "ISOCD.h"

#include <work/KillAbortCode.h>


/***********************************************************************
***
***  Definitions
***
************************************************************************/

#define		ASK_WIN_X		720
#define		ASK_WIN_Y		480

#define		MIN_WIN_X		640
#define		MIN_WIN_Y		200

#define		STACK_NEEDED		( 10 * KILO )

// Option window gadgets (main are in ISOCD.h)
enum OptionGadgets
	{
	GAD_O_SORT,
	GAD_O_REVERSE,
	GAD_O_FILTER,
	GAD_O_F_REVERSE,
	GAD_O_ID_VOL,
	GAD_O_ID_SET,
	GAD_O_ID_PUB,
	GAD_O_ID_PREP,
	GAD_O_ID_APP,
	GAD_O_PVD,
	GAD_O_BASE,
	GAD_O_SPLIT,
	GAD_O_CACHE_DATA,
	GAD_O_CACHE_DIR,
	GAD_O_POOL_LOCK,
	GAD_O_POOL_FILE,
	GAD_O_RETRY,
	GAD_O_DIRECT_READ,
	GAD_O_FAST_SEARCH,
	GAD_O_TRADE_MARK,
	GAD_O_SPEED,
	GAD_O_CDFS_DEF,
	GAD_O_OK,
	GAD_O_CANCEL,
	MAX_OPTION_GAD
	};


/***********************************************************************
***
***  Function Prototypes
***
***********************************************************************/

void	main( INT, CSTR * );
STATUS	AboutMFunc( INTERFACE *, UWORD );
void	AddCurrentDir( void );
void	AdjustForWindow( void );
void	AutoLoadLayout( void );
STATUS	BuildGFunc( INTERFACE *, GAD *, UWORD );
STATUS	CancelGFunc( INTERFACE *, GAD *, UWORD );
void	CheckAssumptions( void );
STATUS	CheckForDeviceGFunc( INTERFACE *, GAD *, UWORD );
STATUS	CheckForLayoutFile( INTERFACE * );
STATUS	CheckNewSourceGFunc( INTERFACE *, GAD *, UWORD );
STATUS	ExamineListGFunc( INTERFACE *, GAD *, UWORD );
void	Init( void );
void	InitList( void );
void	InitListView( void );
STATUS	LayoutLoadGFunc( INTERFACE *, GAD *, UWORD );
STATUS	LayoutSaveGFunc( INTERFACE *, GAD *, UWORD );
void	Main( void );
void	OptFile( CSTR, CSTR );
STATUS	OptimizeGFunc( INTERFACE *, GAD *, UWORD );
STATUS	OptionsGFunc( INTERFACE *, GAD *, UWORD );
void	PrepareLine( struct RastPort *, NODE *, CSTR, UWORD );
void	ResetOptions( void );
void	ScanArg( CSTR );
void	SetCDFSDefaults( void );
STATUS	SetCDFSDefaultsGFunc( INTERFACE *, GAD *, UWORD );
STATUS	SetDateMFunc( INTERFACE *, UWORD );
void	SetOptions( void );
STATUS	SetSizeMFunc( INTERFACE *, UWORD );
STATUS	SetSizeRMFunc( INTERFACE *, UWORD );


/***********************************************************************
***
***  Global Variables
***
***********************************************************************/

PUBLIC struct	GfxBase *GfxBase		= NULL;
PUBLIC struct	IntuitionBase *IntuitionBase	= NULL;

// Interface
PUBLIC INTERFACE	*Interface		= NULL;

PUBLIC STATE	Batch				= FALSE;

PUBLIC CSTR	PrepTitle			= PREP_TITLE;
PUBLIC FLAG	VerboseFlag			= FALSE;
PUBLIC CHAR	ImageName[ FILENAME_LEN ];
PUBLIC LONG	FileBufSize			= ( 128 * SECTOR_SIZE );

// ISO 9660 Compatibility Options
PUBLIC FLAG	AddAltPathTable			= TRUE;
PUBLIC FLAG	AddVersion			= TRUE;
PUBLIC FLAG	UpperCaseNames			= FALSE;

// Dir Info
PUBLIC LONG	MissingFiles			= 0;
PUBLIC LONG	ExtraFiles			= 0;
PUBLIC STATE	TooDeepForISO			= FALSE;

// Build info
PUBLIC FLAG	ImageIsDevice			= TRUE;
PUBLIC MLIST	MasterDirList;
PUBLIC PATHTABLE *PathTable			= NULL;
PUBLIC LONG	Dirs				= 0;
PUBLIC LONG	Files				= 0;
PUBLIC LONG	TotalBytes			= 0;
PUBLIC LONG	CDSize				= 0;
PUBLIC STATE	FixCDFSDateBug			= FALSE;

// Trade Mark
PUBLIC DIRNODE	*TMFileNode			= NULL;
PUBLIC CHAR	TMFileName[ FILENAME_LEN ];

// -------- Options --------
PUBLIC LONG	SortOrder			= S_ALPHA;
PUBLIC LONG	SortReverse			= FALSE;
PUBLIC LONG	SortGroup			= F_NONE;
PUBLIC LONG	GroupReverse			= FALSE;

// ISO Stuff
PUBLIC CHAR	IDVol[ 33 ]			= "CDTV_TEST";
PUBLIC CHAR	IDSet[ 129 ];
PUBLIC CHAR	IDPub[ 129 ];
PUBLIC CHAR	IDPrep[ 129 ];
PUBLIC CHAR	IDApp[ 129 ];
PUBLIC LONG	NumPVD				= 2;
PUBLIC LONG	BaseSector			= 0;
PUBLIC CHAR	SplitFileName[ FILENAME_LEN ];

// CDFS
PUBLIC LONG	CacheData;
PUBLIC LONG	CacheDir;
PUBLIC LONG	PoolLock;
PUBLIC LONG	PoolFile;
PUBLIC LONG	Retries;
PUBLIC LONG	DirectReadOn;
PUBLIC LONG	FastSearch;
PUBLIC LONG	TradeMark;
PUBLIC LONG	Speed;

// -------- Options Backup --------
PRIVATE LONG	BkSortOrder;
PRIVATE LONG	BkSortReverse;
PRIVATE LONG	BkSortGroup;
PRIVATE LONG	BkGroupReverse;

// ISO Stuff
PRIVATE CHAR	BkIDVol[ 33 ];
PRIVATE CHAR	BkIDSet[ 129 ];
PRIVATE CHAR	BkIDPub[ 129 ];
PRIVATE CHAR	BkIDPrep[ 129 ];
PRIVATE CHAR	BkIDApp[ 129 ];
PRIVATE LONG	BkNumPVD;
PRIVATE LONG	BkBaseSector;
PRIVATE CHAR	BkSplitFileName[ FILENAME_LEN ];

// CDFS
PRIVATE LONG	BkCacheData;
PRIVATE LONG	BkCacheDir;
PRIVATE LONG	BkPoolLock;
PRIVATE LONG	BkPoolFile;
PRIVATE LONG	BkRetries;
PRIVATE LONG	BkDirectReadOn;
PRIVATE LONG	BkFastSearch;
PRIVATE LONG	BkTradeMark;
PRIVATE LONG	BkSpeed;


/***********************************************************************
***
***  Program Usage Information
***
***********************************************************************/

PUBLIC CSTR UsageText[] =
	{
	"PURPOSE: Create an ISO-9660 CD-ROM image.\n\nUSAGE: %s [options] <directory>\n",
	"  -l<file> Layout file",
	"  -o<file> ISO-9660 output file or device",
	"  -s<file> Split ISO descriptor file",
	"  -b       Build image (requires layout file (-l), will exit when done)",
	"           NOTE: Use this option with the greatest of care...",
	" ",
	"  -d       Adjust dates to match CDFS 24.x date error",
	"  -x       Verbose debugging information",
	"  -f<n>    # of sectors in file buffer [128]",
	"  -m       Use mimimum size window",
	"  -q<n>    Change qualifier for insert in lists [0]",
	"           (0=CTRL, 1=SHIFT, 2=ALT)",
	"  -t       Do not use topaz font, use system font instead",
	" ",
	"  -p       Do not add Path Table for reverse byte systems",
	"  -v       Do not append version number \";1\" to file names",
	NULL
	};


/***********************************************************************
***
***  Module Variables
***
***********************************************************************/

// Program
PRIVATE	INT	Ver			= VER;
PRIVATE	INT	Rev			= REV;
PRIVATE	CSTR	ProgTitle		= "ISOCD %ld.%02ld by Pantaray - Copyright (C) 1992 Commodore International";
PRIVATE CHAR	WindowTitle[ 90 ];
PRIVATE CSTR	Credits			=
	"Produced by:    Carl Sassenrath\n"
	"Programmed by:  Carl Sassenrath and Kenneth Yeast\n"
	"Interface by:   Kenneth Yeast\n"
	"Special Thanks: Benjamin Phister and Chris Henry\n"
	"\n"
	"Pantaray, Inc. (c) 1992\n"
	"\"All things in motion...\"";

PRIVATE SHORT	FileCount		= 0;
PRIVATE STATE	UseMinimumWindow	= FALSE;
PRIVATE CHAR	CurrentDirName[ FILENAME_LEN ];

// Device communication
PRIVATE	UBYTE	*CardImage		= NULL;

// Files
PRIVATE	CHAR	SourceName[ FILENAME_LEN ];
PRIVATE	CHAR	LayoutName[ FILENAME_LEN ];

// Misc
PRIVATE STATE	Success			= FALSE;
PRIVATE STATE	ShowSize		= TRUE;
PRIVATE STATE	RoundSizes		= FALSE;
PRIVATE LONG	AssQual			= Q_CTRL;
PRIVATE CHAR	StatusStr[ 60 ];
PRIVATE CHAR	StatusErrStr[ 60 ];

// Window on public screen
PRIVATE TAG	TagW_Main[]		=
	{
	{ WA_Width,	ASK_WIN_X		},	// Position dependent!
	{ WA_Height,	ASK_WIN_Y		},	// Position dependent!
	{ WA_IDCMP,	( NORMAL_IDCMP | MENU_IDCMP | CUSTOM_IDCMP )	},
	{ WA_Flags,	NORMAL_WFLAGS		},
	{ WA_Title,	(ULONG) WindowTitle	},
	TAG_DONE
	};

// Menus...
PRIVATE struct NewMenu PanelMenu[] =
	{
	{ NM_TITLE, "Project",		 0 , 0, 0, 0		},
	{  NM_ITEM, "About...",		"?", 0, 0, AboutMFunc	},
	{  NM_ITEM, "Quit",		"Q", 0, 0, DoneMFunc	},
	{ NM_TITLE, "Entries",		 0 , 0, 0, 0		},
	{  NM_ITEM, "Display",		 0,  0, 0, 0		},
	{   NM_SUB, "Sizes",		"S", CHECKIT|CHECKED,	~0x0001, SetSizeMFunc },
	{   NM_SUB, "Sizes Rounded",	"E", CHECKIT,		~0x0002, SetSizeRMFunc },
	{   NM_SUB, "Dates",		"D", CHECKIT,		~0x0003, SetDateMFunc },
	{  NM_ITEM, "Find File...",	"F", 0, 0, FindMFunc	},
	{  NM_ITEM, "Add Spaces...",	"A", 0,	0, SpaceAddMFunc },
	{  NM_ITEM, "Remove Spaces",	"R", 0,	0, SpaceDelMFunc },

	{ NM_END, 0,			 0 , 0, 0, 0		}
	};

// Default to topaz (for scroller)
PRIVATE struct	TextAttr TopazReq	= { "topaz.font", 8, 0, 0 };
PRIVATE struct	TextAttr *UseFont	= &TopazReq;

// ActionTags...
PRIVATE TAG	TagA_Update[][ 2 ]	=
	{
	{ { GAD_TA_Update, (ULONG) StatusStr	}, TAG_DONE },
	{ { GAD_TA_Update, (ULONG) StatusErrStr }, TAG_DONE },
	};
PRIVATE TAG	TagA_Options[]		=
	{
	{ GAD_TA_BusyWait,	0				},
	{ GAD_TA_Function,	(ULONG) OptionsGFunc		},
	TAG_DONE
	};
PRIVATE TAG	TagA_Load[]		=
	{
	{ GAD_TA_BusyWait,	0				},
	{ GAD_TA_Function,	(ULONG) LayoutLoadGFunc		},
	TAG_DONE
	};
PRIVATE TAG	TagA_Save[]		=
	{
	{ GAD_TA_BusyWait,	0				},
	{ GAD_TA_Function,	(ULONG) LayoutSaveGFunc		},
	TAG_DONE
	};
PRIVATE TAG	TagA_Examine[]		=
	{
	{ GAD_TA_BusyWait,	0				},
	{ GAD_TA_Function,	(ULONG) ExamineListGFunc	},
	TAG_DONE
	};
PRIVATE TAG	TagA_Optimize[]		=
	{
	{ GAD_TA_BusyWait,	0				},
	{ GAD_TA_Function,	(ULONG) OptimizeGFunc		},
	TAG_DONE
	};
PRIVATE TAG	TagA_Build[]		=
	{
	{ GAD_TA_BusyWait,	0				},
	{ GAD_TA_Function,	(ULONG) BuildGFunc		},
	TAG_DONE
	};
PRIVATE TAG	TagA_Source[]		=
	{
	{ GAD_TA_BusyWait,	0				},
	{ GAD_TA_GF_Name,	(ULONG) "Source Directory"	},
	{ GAD_TA_GF_Type,	(ULONG) REQ_DIR_ONLY		},
	{ GAD_TA_ReturnCode,	(ULONG) 0			},
	{ GAD_TA_GetFile,	(ULONG) -1			},
	{ GAD_TA_Function,	(ULONG) CheckNewSourceGFunc	},
	{ GAD_TA_Stub,		(ULONG) AddCurrentDir		},
	{ GAD_TA_Update,	(ULONG) SourceName		},
	TAG_DONE
	};
PRIVATE TAG	TagA_Image[]		=
	{
	{ GAD_TA_BusyWait,	0				},
	{ GAD_TA_GF_Name,	(ULONG) "Image File or Device"	},
	{ GAD_TA_GetFile,	(ULONG) -1			},
	{ GAD_TA_Function,	(ULONG) CheckForDeviceGFunc	},
	{ GAD_TA_Update,	(ULONG) ImageName		},
	TAG_DONE
	};
PRIVATE TAG	TagA_Layout[]		=
	{
	{ GAD_TA_BusyWait,	0				},
	{ GAD_TA_GF_Name,	(ULONG) "Layout File"		},
	{ GAD_TA_GetFile,	(ULONG) -1			},
	{ GAD_TA_Update,	(ULONG) LayoutName		},
	TAG_DONE
	};
PRIVATE TAG	TagA_Box[]		=
	{
	{ GAD_TA_AssociateGAD,	(ULONG) GAD_SLID		},
	TAG_DONE
	};
PRIVATE TAG	TagA_Slid[]		=
	{
	{ GAD_TA_AssociateGAD,	(ULONG) GAD_BOX			},
	TAG_DONE
	};

// Request tags...
PRIVATE TAG	TagR_Source[]		=
	{
	{ GAD_TC_Title,		(ULONG) "Source"		},
	TAG_DONE
	};
PRIVATE TAG	TagR_Image[]		=
	{
	{ GAD_TC_Title,		(ULONG) "Image"			},
	TAG_DONE
	};
PRIVATE TAG	TagR_Layout[]		=
	{
	{ GAD_TC_Title,		(ULONG) "Layout"		},
	TAG_DONE
	};
PRIVATE TAG	TagR_Slid[]		=
	{
	{ PGA_Freedom,		LORIENT_VERT	},
	{ GTSC_Arrows,		8		},
	{ GA_Immediate,		TRUE		},
	{ GA_RelVerify,		TRUE		},
	TAG_DONE
	};

// Gadgets!!!!
// Desired Gadgets (non-interactive first)
//	ID, Kind, X, Y, SizeX, SizeY, Flags, TitleText, ReqTags, ActionTags
PRIVATE GAD_REQ GadTable[]		=
	{
{ GAD_OPTIONS,	G_BUTTON,  30,  16, 120, 6, 0, "Options",	NULL, TagA_Options	},
{ GAD_LOAD,	G_BUTTON,  30,  35, 120, 6, 0, "Load Layout",	NULL, TagA_Load		},
{ GAD_SAVE,	G_BUTTON,  30,  54, 120, 6, 0, "Save Layout",	NULL, TagA_Save		},

{ GAD_EXAMINE,	G_BUTTON, 175,  16, 120, 6, 0, "Examine", NULL, TagA_Examine	},
{ GAD_OPTIMIZE, G_BUTTON, 175,  35, 120, 6, 0, "Optimize",NULL, TagA_Optimize	},
{ GAD_BUILD,	G_BUTTON, 175,  54, 120, 6, 0, "Build",   NULL, TagA_Build	},

{ GAD_SOURCE,	G_C_BUTTON, 74, 90, 241, 4, 0, NULL,	TagR_Source,	TagA_Source	},
{ GAD_IMAGE,	G_C_BUTTON, 74,108, 241, 4, 0, NULL,	TagR_Image,	TagA_Image	},
{ GAD_LAYOUT,	G_C_BUTTON, 74,126, 241, 4, 0, NULL,	TagR_Layout,	TagA_Layout	},

{ GAD_STAT,	G_TEXT,	   16, 161, 300, 0, 0, NULL,	NULL,		TagA_Update[ 0 ] },
{ GAD_STATERR,G_TEXT,	   16, 171, 300, 0, HIGH, NULL, NULL,		TagA_Update[ 1 ] },

{ GAD_BOX,	G_C_LISTVIEW,325,10,288, 166, 0, NULL,	  NULL, TagA_Box	},
{ GAD_SLID,	G_SCROLLER,618, 10,  16, 166, 0, NULL,	  TagR_Slid, TagA_Slid	},
	};

// Grouped Box positions
PRIVATE BOX	ActionBox		= {  10,  10, 310,  63 };
PRIVATE BOX	FilesBox		= {  10,  83, 310,  62 };
PRIVATE BOX	StatusBox		= {  10, 155, 310,  29 };

PRIVATE RTEXT	ActionText		= { 132,   0, "Actions"	};
PRIVATE RTEXT	FilesText		= { 140,  73, "Files"	};
PRIVATE RTEXT	StatusText		= { 135, 145, "Status"	};

PRIVATE RTEXT	BoxText			= { 330,   0, "Path File" };
PRIVATE RTEXT	DSText			= { 577,   0, "Size"	};

// Refresh tags...
PRIVATE TAG TagRefreshList[]		=
	{
	{ GAD_TR_Box,		(ULONG) &ActionBox	},
	{ GAD_TR_Box,		(ULONG) &FilesBox	},
	{ GAD_TR_Box,		(ULONG) &StatusBox	},
	{ GAD_TR_Label,		(ULONG) &ActionText	},
	{ GAD_TR_Label,		(ULONG) &FilesText	},
	{ GAD_TR_Label,		(ULONG) &StatusText	},
	{ GAD_TR_Label,		(ULONG) &BoxText	},
	{ GAD_TR_Label,		(ULONG) &DSText		},
	TAG_DONE
	};

// Option menu
PRIVATE struct NewMenu OptionMenu[] =
	{
	{ NM_TITLE, "Project",		 0 , 0, 0, 0		},
	{  NM_ITEM, "Done",		"Q", 0, 0, DoneMFunc	},

	{ NM_END, 0,			 0 , 0, 0, 0		}
	};

// Window on public screen
PRIVATE TAG	TagW_Options[]		=
	{
	{ WA_Width,	640			},
	{ WA_Height,	200			},
	{ WA_IDCMP,	( NORMAL_IDCMP | MENU_IDCMP | CUSTOM_IDCMP )	},
	{ WA_Flags,	NORMAL_WFLAGS		},
	{ WA_Title,	(ULONG) "Options"	},
	TAG_DONE
	};

// Option Action tags...
PRIVATE TAG	TagA_O_Update[][ 2 ]	=
	{
	{ { GAD_TA_Update, (ULONG) SplitFileName}, TAG_DONE },
	{ { GAD_TA_Update, (ULONG) &SortOrder	}, TAG_DONE },
	{ { GAD_TA_Update, (ULONG) &SortReverse	}, TAG_DONE },
	{ { GAD_TA_Update, (ULONG) &SortGroup	}, TAG_DONE },
	{ { GAD_TA_Update, (ULONG) &GroupReverse}, TAG_DONE },
	{ { GAD_TA_Update, (ULONG) IDVol	}, TAG_DONE },
	{ { GAD_TA_Update, (ULONG) IDSet	}, TAG_DONE },
	{ { GAD_TA_Update, (ULONG) IDPub	}, TAG_DONE },
	{ { GAD_TA_Update, (ULONG) IDPrep	}, TAG_DONE },
	{ { GAD_TA_Update, (ULONG) IDApp	}, TAG_DONE },
	{ { GAD_TA_Update, (ULONG) &NumPVD	}, TAG_DONE },
	{ { GAD_TA_Update, (ULONG) &BaseSector	}, TAG_DONE },
	{ { GAD_TA_Update, (ULONG) &CacheData	}, TAG_DONE },
	{ { GAD_TA_Update, (ULONG) &CacheDir	}, TAG_DONE },
	{ { GAD_TA_Update, (ULONG) &PoolLock	}, TAG_DONE },
	{ { GAD_TA_Update, (ULONG) &PoolFile	}, TAG_DONE },
	{ { GAD_TA_Update, (ULONG) &Retries	}, TAG_DONE },
	{ { GAD_TA_Update, (ULONG) &DirectReadOn}, TAG_DONE },
	{ { GAD_TA_Update, (ULONG) &FastSearch	}, TAG_DONE },
	{ { GAD_TA_Update, (ULONG) &TradeMark	}, TAG_DONE },
	{ { GAD_TA_Update, (ULONG) &Speed	}, TAG_DONE },
	};
PRIVATE TAG	TagA_O_UpdateCheck[][ 3 ]	=
	{
	{ { GAD_TA_IntMin, MIN_PVD	  },
	  { GAD_TA_IntMax, MAX_PVD	  },
	  { TAG_MORE, (ULONG) TagA_O_Update[ 10 ] } },

	{ { GAD_TA_IntMin, MIN_CACHE_DATA },
	  { GAD_TA_IntMax, MAX_CACHE_DATA },
	  { TAG_MORE, (ULONG) TagA_O_Update[ 12 ] } },

	{ { GAD_TA_IntMin, MIN_CACHE_DIR  },
	  { GAD_TA_IntMax, MAX_CACHE_DIR  },
	  { TAG_MORE, (ULONG) TagA_O_Update[ 13 ] } },

	{ { GAD_TA_IntMin, MIN_POOL_LOCK  },
	  { GAD_TA_IntMax, MAX_POOL_LOCK  },
	  { TAG_MORE, (ULONG) TagA_O_Update[ 14 ] } },

	{ { GAD_TA_IntMin, MIN_POOL_FILE  },
	  { GAD_TA_IntMax, MAX_POOL_FILE  },
	  { TAG_MORE, (ULONG) TagA_O_Update[ 15 ] } },

	{ { GAD_TA_IntMin, MIN_RETRIES	  },
	  { GAD_TA_IntMax, MAX_RETRIES    },
	  { TAG_MORE, (ULONG) TagA_O_Update[ 16 ] } },
	};
PRIVATE TAG	TagA_O_Split[]		=
	{
	{ GAD_TA_GF_Name,	(ULONG) "Split File"		},
	{ GAD_TA_GetFile,	(ULONG) -1			},
	{ TAG_MORE,		(ULONG) TagA_O_Update[ 0 ]	},
	};
PRIVATE TAG	TagA_O_CDFS_Def[]		=
	{
	{ GAD_TA_Function,	(ULONG) SetCDFSDefaultsGFunc	},
	TAG_DONE
	};
PRIVATE TAG	TagA_O_OK[]		=
	{
	{ GAD_TA_Exit,		(ULONG) TRUE			},
	TAG_DONE
	};
PRIVATE TAG	TagA_O_Cancel[]		=
	{
	{ GAD_TA_Exit,		(ULONG) TRUE			},
	{ GAD_TA_Function,	(ULONG) CancelGFunc		},
	TAG_DONE
	};

// Option Request tags...
// Sort Order
PRIVATE CSTR SortText[] =
	{
	"Alpha",
	"Size",
	"Date",
	"None",
	NULL
	};
PRIVATE TAG	TagR_O_Sort[]		=
	{
	{ GTCY_Labels,		(ULONG) SortText		},
	TAG_DONE
	};
PRIVATE CSTR	GroupText[]		=
	{
	".Info",
	"Dirs",
	"Extension",
	"None",
	NULL
	};
PRIVATE TAG	TagR_O_Group[]		=
	{
	{ GTCY_Labels,		(ULONG) GroupText		},
	TAG_DONE
	};
PRIVATE TAG	TagR_O_Vol[]		=
	{
	{ GTST_String,		(ULONG) IDVol			},
	{ GTST_MaxChars,	32				},
	TAG_DONE
	};
PRIVATE TAG	TagR_O_Set[]		=
	{
	{ GTST_String,		(ULONG) IDSet			},
	{ GTST_MaxChars,	128				},
	TAG_DONE
	};
PRIVATE TAG	TagR_O_Pub[]		=
	{
	{ GTST_String,		(ULONG) IDPub			},
	{ GTST_MaxChars,	128				},
	TAG_DONE
	};
PRIVATE TAG	TagR_O_Prep[]		=
	{
	{ GTST_String,		(ULONG) IDPrep			},
	{ GTST_MaxChars,	128				},	// Will reduce (note init)
	TAG_DONE
	};
PRIVATE TAG	TagR_O_App[]		=
	{
	{ GTST_String,		(ULONG) IDApp			},
	{ GTST_MaxChars,	128				},
	TAG_DONE
	};
PRIVATE TAG	TagR_O_PVD[]		=
	{
	{ GTIN_MaxChars,	4				},
	TAG_DONE
	};
PRIVATE TAG	TagR_O_Split[]		=
	{
	{ GAD_TC_Title,		(ULONG) "Split File"		},
	TAG_DONE
	};

// Option Gadgets!!!!
// Desired Gadgets (non-interactive first)
//	ID, Kind, X, Y, SizeX, SizeY, Flags, TitleText, ReqTags, ActionTags
PRIVATE GAD_REQ OptionGadTable[]		=
	{
{ GAD_O_SORT,	    G_CYCLE,    113,  17, 120, 6, 0, "Order",		TagR_O_Sort,	TagA_O_Update[  1 ] },
{ GAD_O_REVERSE,    G_CHECKBOX, 315,  17,  20, 0, 0, "Reverse",		NULL,		TagA_O_Update[  2 ] },
{ GAD_O_FILTER,	    G_CYCLE,    113,  34, 120, 6, 0, "Group",		TagR_O_Group,	TagA_O_Update[  3 ] },
{ GAD_O_F_REVERSE,  G_CHECKBOX, 315,  34,  20, 0, 0, "Reverse",		NULL,		TagA_O_Update[  4 ] },

{ GAD_O_ID_VOL,	    G_STRING,   113,  51, 244, 6, 0, "Volume ID",	TagR_O_Vol,	TagA_O_Update[  5 ] },
{ GAD_O_ID_SET,	    G_STRING,   113,  68, 244, 6, 0, "Volume Set",	TagR_O_Set,	TagA_O_Update[  6 ] },
{ GAD_O_ID_PUB,	    G_STRING,   113,  85, 244, 6, 0, "Publisher",	TagR_O_Pub,	TagA_O_Update[  7 ] },
{ GAD_O_ID_PREP,    G_STRING,   113, 102, 244, 6, 0, "Preparer",	TagR_O_Prep,	TagA_O_Update[  8 ] },
{ GAD_O_ID_APP,	    G_STRING,   113, 119, 244, 6, 0, "Application",	TagR_O_App,	TagA_O_Update[  9 ] },
{ GAD_O_PVD,	    G_INTEGER,  113, 136,  40, 6, 0, "PVDs",		TagR_O_PVD,	TagA_O_UpdateCheck[ 0 ] },

{ GAD_O_BASE,	    G_INTEGER,  113, 153,  80, 6, 0, "Base Sector",	NULL,		TagA_O_Update[ 11 ] },
{ GAD_O_SPLIT,	    G_C_BUTTON, 113, 170, 240, 4, 0, NULL,		TagR_O_Split,	TagA_O_Split	    },

{ GAD_O_CACHE_DATA, G_INTEGER,  495,  17,  60, 4, 0, "Data Cache",	NULL,		TagA_O_UpdateCheck[ 1 ] },
{ GAD_O_CACHE_DIR,  G_INTEGER,  495,  37,  60, 4, 0, "Dir Cache",	NULL,		TagA_O_UpdateCheck[ 2 ] },
{ GAD_O_POOL_LOCK,  G_INTEGER,  495,  57,  60, 4, 0, "File Lock",	NULL,		TagA_O_UpdateCheck[ 3 ] },
{ GAD_O_POOL_FILE,  G_INTEGER,  495,  77,  60, 4, 0, "File Handle",	NULL,		TagA_O_UpdateCheck[ 4 ] },
{ GAD_O_RETRY,	    G_INTEGER,  495,  97,  60, 4, 0, "Retries",		NULL,		TagA_O_UpdateCheck[ 5 ] },
{ GAD_O_DIRECT_READ,G_CHECKBOX, 475, 117,  20, 0, 0, "Direct Read",	NULL,		TagA_O_Update[ 17 ] },
{ GAD_O_FAST_SEARCH,G_CHECKBOX, 475, 137,  20, 0, 0, "Fast Search",	NULL,		TagA_O_Update[ 18 ] },
{ GAD_O_TRADE_MARK, G_CHECKBOX, 595, 117,  20, 0, 0, "Trade Mark",	NULL,		TagA_O_Update[ 19 ] },
{ GAD_O_SPEED,	    G_CHECKBOX, 595, 137,  20, 0, 0, "Speed Ind.",	NULL,		TagA_O_Update[ 20 ] },
{ GAD_O_CDFS_DEF,   G_BUTTON,   400, 153, 200, 4, 0, "Restore CDFS Defaults",NULL,	TagA_O_CDFS_Def	    },

{ GAD_O_OK,	    G_BUTTON,   369, 173, 128, 4, 0, "OK",		NULL,		TagA_O_OK	    },
{ GAD_O_CANCEL,	    G_BUTTON,   502, 173, 128, 4, 0, "Cancel",		NULL,		TagA_O_Cancel	    },
	};

// Refresh tags...
PRIVATE BOX	ISOBox			= {  10,   2, 351, 183 };
PRIVATE RTEXT	ISOText			= { 170,   2, "ISO" };
PRIVATE RTEXT	OGText			= {  35,  26, "Dir" };

PRIVATE BOX	CDFSBox			= { 369,   2, 261, 169 };
PRIVATE RTEXT	CDFSText		= { 485,   2, "CDFS" };

PRIVATE RTEXT	DataText		= { 562,  17, "Sectors" };
PRIVATE RTEXT	DirText			= { 562,  37, "Sectors" };
PRIVATE RTEXT	LockText		= { 562,  57, "Nodes" };
PRIVATE RTEXT	HandleText		= { 562,  77, "Nodes" };

PRIVATE TAG OptionTagRefreshList[]	=
	{
	{ GAD_TR_Box,		(ULONG) &ISOBox		},
	{ GAD_TR_Label,		(ULONG) &ISOText	},
	{ GAD_TR_Text,		(ULONG) &OGText		},
	{ GAD_TR_Box,		(ULONG) &CDFSBox	},
	{ GAD_TR_Label,		(ULONG) &CDFSText	},
	{ GAD_TR_Text,		(ULONG) &DataText	},
	{ GAD_TR_Text,		(ULONG) &DirText	},
	{ GAD_TR_Text,		(ULONG) &LockText	},
	{ GAD_TR_Text,		(ULONG) &HandleText	},
	TAG_DONE
	};


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
	sprintf( WindowTitle, ProgTitle, Ver, Rev );

	ScanProgArgs( argc, argv, ScanArg );

	CheckAssumptions();

	Init();

	Main();

	Done();
	}


/***********************************************************************
***
***  AboutMFunc
***
************************************************************************/

STATUS AboutMFunc(
	INTERFACE *Interface,
	UWORD Code
	)
	{
	TellUser( Interface, Credits );
	return( OK );
	}


/***********************************************************************
***
***  AddCurrentDir
***
***	If the user gives us a Source w/o a drive or volume, it must
***	be some offset from our current dir.  Therefore we add the
***	current dir, otherwise a layout file saved will not work anywhere
***	but in the original dir.
***
***********************************************************************/

void AddCurrentDir(
	void
	)
	{
	CHAR NewName[ FILENAME_LEN ];

	// Is ":" good enough?
	if ( SourceName[ 0 ] )
		if ( strchr( SourceName, ':' ) == NULL )
			{
			strcpy( NewName, SourceName );
			strcpy( SourceName, CurrentDirName );
			strcat( SourceName, NewName );
			ReflectGadget( Interface, GAD_SOURCE );
			}
	}


/***********************************************************************
***
***  AdjustForWindow
***
***********************************************************************/

void AdjustForWindow(
	void
	)
	{
	WORD IncX = ( Interface->IWindow->Width  - MIN_WIN_X );
	WORD IncY = ( Interface->IWindow->Height - MIN_WIN_Y );

	if ( IncX > 0 )
		{
		GadTable[ GAD_BOX ].SizeX += IncX;
		GadTable[ GAD_SLID ].X += IncX;
		DSText.X += IncX;
		}

	if ( IncY > 0 )
		{
		GadTable[ GAD_BOX ].SizeY += IncY;
		GadTable[ GAD_SLID ].SizeY += IncY;
		StatusBox.SizeY += IncY;
		}
	}


/***********************************************************************
***
***  AutoLoadLayout
***
***********************************************************************/

void AutoLoadLayout(
	void
	)
	{
	GAD *Entry	 = &Interface->GadgetActions[ GAD_LOAD ];
	TAG *TagPtr;
	LONG *ReturnCode = NULL;

	// Make like we hit "OK" on file req
	if ( TagPtr = FindTagItem( GAD_TA_ReturnCode, Entry->ActionTags ) )
		ReturnCode = (LONG *) &( TagPtr->ti_Data );

	if ( ReturnCode )
		*ReturnCode = TRUE;

	LayoutLoadGFunc( Interface, Entry, 0 );

	if ( ! HAS_NODE_LIST( &MasterDirList ) )
		Batch = FALSE;
	}


/***********************************************************************
***
***  BuildGFunc
***
***********************************************************************/

STATUS BuildGFunc(
	INTERFACE *Interface,
	GAD *Entry,
	UWORD Code
	)
	{
	CHAR DCStr[ 80 ];

	// User Proofing
	StatusError( NULL );
	if ( ! HAS_NODE_LIST( &MasterDirList ) )
		{
		StatusError( "Need to Examine first" );
		return( OK );
		}

	if ( ! ImageName[ 0 ] )
		{
		ActionGadget( Interface, GAD_IMAGE, 0 );
		if ( ! ImageName[ 0 ] )
			return( OK );
		}

	// It's important that everything look good, since this may
	// take a while, also they should see the STAT (sizes, etc.)
	Refresh( Interface );
	ReflectGadget( Interface, GAD_STAT );

	Success = CreateImage( &MasterDirList, SourceName );

	// Perform a diskchange if needed
	// Later change to ACTION_DISK_CHANGE
	if ( ( Success == OK ) && ImageIsDevice )
		{
		sprintf( DCStr, "DiskChange >NIL: %s", ImageName );
		(void) Execute( DCStr, NULL, NULL );
		}

	return( OK );
	}


/***********************************************************************
***
***  CancelGFunc
***
***********************************************************************/

STATUS CancelGFunc(
	INTERFACE *Interface,
	GAD *Entry,
	UWORD Code
	)
	{
	ResetOptions();
	ReflectAll( Interface );
	return( OK );
	}


/***********************************************************************
***
***  CheckAssumptions
***
************************************************************************/

void CheckAssumptions(
	void
	)
	{
	if ( Batch && ( ! LayoutName[ 0 ] ) )
		Error( "Batch option requires a layout file" );
	}


/***********************************************************************
***
***  CheckForDeviceGFunc
***
***********************************************************************/

STATUS CheckForDeviceGFunc(
	INTERFACE *Interface,
	GAD *Entry,
	UWORD Code
	)
	{
	if ( ImageName[ strlen( ImageName ) - 1 ] == ':' )
		if ( ValidDOSVolume( ImageName ) )
			{
			DeviceFromVolume( ImageName, ImageName );
			ReflectGadget( Interface, GAD_IMAGE );
			}

	return( OK );
	}


/***********************************************************************
***
***  CheckForLayoutFile
***
***********************************************************************/

STATUS CheckForLayoutFile(
	INTERFACE *Interface
	)
	{
	// User Proofing
	StatusError( NULL );

	if ( ! LayoutName[ 0 ] )
		{
		ActionGadget( Interface, GAD_LAYOUT, 0 );
		if ( ! LayoutName[ 0 ] )
			return( ERROR );
		}

	return( OK );
	}


/***********************************************************************
***
***  CheckNewSourceGFunc
***
***********************************************************************/

STATUS CheckNewSourceGFunc(
	INTERFACE *Interface,
	GAD *Entry,
	UWORD Code
	)
	{
	LONG DoAction;

	// User Proofing
	StatusError( NULL );

	GetGadgetReturn( Interface, Entry->ID, &DoAction );

	if ( DoAction )
		{
		NewList( (LIST *) &MasterDirList );

		InitList();
		}

	return( OK );
	}


/***********************************************************************
***
***  ExamineListGFunc
***
***********************************************************************/

STATUS ExamineListGFunc(
	INTERFACE *Interface,
	GAD *Entry,
	UWORD Code
	)
	{
	// User Proofing
	StatusError( NULL );

	if ( ! SourceName[ 0 ] )
		{
		ActionGadget( Interface, GAD_SOURCE, 0 );
		if ( ! SourceName[ 0 ] )
			return( OK );
		}

	TooDeepForISO = FALSE;
	NewList( (LIST *) &MasterDirList );

	InitList();

	if ( BuildDirList( &MasterDirList, SourceName ) )
		return( OK );

	if ( TooDeepForISO )
		StatusError( "Too many dirs deep for ISO" );

	if ( ShowSize && ( SortOrder == S_DATE ) )
		ShowSize = TRUE;

	TurnGadget( Interface, GAD_OPTIMIZE, ON );

	InformDirLoaded( TRUE );
	return( OK );
	}


/***********************************************************************
***
***  InformDirLoaded
***
***********************************************************************/

void InformDirLoaded(
	STATE InformList
	)
	{
	if ( InformList )
		InitList();

	sprintf( StatusStr, "Bytes %-9d Dirs %-4d Files %-5d",
		 ( CDSize * SECTOR_SIZE ), Dirs, Files );

	ReflectAll( Interface );
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
	CHAR Buffer[ 60 ];

	if ( ProgramStack() < STACK_NEEDED )
		{
		sprintf( Buffer, "Requires a stack of at least %ld bytes", STACK_NEEDED );
		Error( Buffer );
		}

	if ( ! ( GfxBase = (struct GfxBase *) OpenLibrary( "graphics.library", 0 ) ) )
		Error( "Not an Amiga system" );

	if ( ! ( IntuitionBase = (struct IntuitionBase *) OpenLibrary( "intuition.library", 37 ) ) )
		Error( "Intuition (V2.0) not available" );

	// Get Current Dir
	if ( ! GetCurrentDirName( CurrentDirName, FILENAME_LEN ) )
		Error( "Could not get current directory" );

	// If not specified, use current dir
	if ( ! SourceName[ 0 ] )
		strcpy( SourceName, CurrentDirName );

	if ( CurrentDirName[ strlen( CurrentDirName ) - 1 ] != ':' )
		strcat( CurrentDirName, "/" );

	// Version our PrepTitle and change the space available for user
	sprintf( ( PrepTitle + 9 ), "%1d.%02d", VER, REV );
	PrepTitle[ 13 ] = ' ';

	TagR_O_Prep[ 1 ].ti_Data -= ( strlen( PrepTitle ) + 1 );

	InitMemory();
	InitDir();
	InitOut();
	InitISO();

	// Adjust window if necessary
	if ( UseMinimumWindow )
		{
		TagW_Main[ 0 ].ti_Data = MIN_WIN_X;
		TagW_Main[ 1 ].ti_Data = MIN_WIN_Y;
		}

	if ( ! ( Interface = InitInterface( NULL, TagW_Main, PanelMenu,
					    NULL, 0, UseFont, TagRefreshList ) ) )
		Error( "Interface not available" );

	AdjustForWindow();

	if ( CreateGadgets( Interface, GadTable, MAX_GAD ) )
		Error( "Could not create gadgets" );

	SetCDFSDefaults();

	InitListView();

	ReflectAll( Interface );

	if ( LayoutName[ 0 ] )
		AutoLoadLayout();
	}


/***********************************************************************
***
***  InitList
***
***********************************************************************/

void InitList(
	void
	)
	{
	GadgetCustomMethod( Interface, GAD_BOX, GAD_TC_L_InitList, (ULONG) &MasterDirList, TAG_DONE );
	}


/***********************************************************************
***
***  InitListView
***
************************************************************************/

void InitListView(
	void
	)
	{
	NewList( (LIST *) &MasterDirList );

	GadgetCustomMethod( Interface, GAD_BOX,
			    GAD_TC_L_InitGad,  (ULONG) Interface->GadgetActions[ GAD_BOX ].ActionTags,
			    GAD_TC_L_InitList, (ULONG) &MasterDirList,
			    GAD_TC_L_InitLine, (ULONG) PrepareLine,
			    GAD_TC_L_InitQual, (ULONG) AssQual,
			    TAG_DONE );
	}


/***********************************************************************
***
***  LayoutLoadGFunc
***
***********************************************************************/

STATUS LayoutLoadGFunc(
	INTERFACE *Interface,
	GAD *Entry,
	UWORD Code
	)
	{
	STATUS Err;

	if ( CheckForLayoutFile( Interface ) )
		return( OK );

	if ( SizeOfFile( LayoutName ) == 0 )
		{
		TellUser( Interface, "File is empty" );
		return( OK );
		}

	TooDeepForISO = FALSE;
	NewList( (LIST *) &MasterDirList );

	InitList();

	Err = LoadFile( LayoutName, &MasterDirList, SourceName );

	ReflectAll( Interface );

	if ( Err )
		return( OK );

	InformDirLoaded( TRUE );

	TurnGadget( Interface, GAD_OPTIMIZE, ON );

	if ( TooDeepForISO )
		StatusError( "Too many dirs deep for ISO" );

	return( OK );
	}


/***********************************************************************
***
***  LayoutSaveGFunc
***
***********************************************************************/

STATUS LayoutSaveGFunc(
	INTERFACE *Interface,
	GAD *Entry,
	UWORD Code
	)
	{
	if ( CheckForLayoutFile( Interface ) )
		return( OK );

	SaveFile( LayoutName, &MasterDirList, SourceName );
	InformDirLoaded( FALSE );

	return( OK );
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
	if ( Batch )
		{
		ActionGadget( Interface, GAD_BUILD, 0 );

		// Leave them in the program if something fails
		if ( Success == ERROR )
			Batch = FALSE;
		}

	if ( ! Batch )
		HandleWindowInterface( Interface );
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
***  OptimizeGFunc
***
***********************************************************************/

STATUS OptimizeGFunc(
	INTERFACE *Interface,
	GAD *Entry,
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

	// Can only optimize once
	if ( OptimizePanel( Interface ) == OK )
		{
		TurnGadget( Interface, GAD_OPTIMIZE, OFF );
		InformDirLoaded( TRUE );
		}

	return( OK );
	}


/***********************************************************************
***
***  OptionsGFunc
***
***********************************************************************/

STATUS OptionsGFunc(
	INTERFACE *Interface,
	GAD *Entry,
	UWORD Code
	)
	{
	StatusError( NULL );
	SetOptions();

	if ( DoSimpleInterface( NULL, TagW_Options, OptionMenu, OptionGadTable,
				MAX_OPTION_GAD, UseFont, OptionTagRefreshList ) )
		TellUser( Interface, "Option window not available" );

	return( OK );
	}


/***********************************************************************
***
***  PrepareLine
***
***********************************************************************/

void PrepareLine(
	struct RastPort *RP,
	NODE *NPtr,
	CSTR Buffer,
	UWORD Length
	)
	{
	PRIVATE CSTR Month[] =
		{
		"Jan",
		"Feb",
		"Mar",
		"Apr",
		"May",
		"Jun",
		"Jul",
		"Aug",
		"Sep",
		"Oct",
		"Nov",
		"Dec"
		};
	PRIVATE UBYTE RecTime[ 10 ];
	REG DIRNODE *DNode = (DIRNODE *) NPtr;
	REG Size;

	if ( ! DNode )
		{
		sprintf( Buffer, "%-*.*s", Length, Length, " " );
		return;
		}

	SetAPen( RP, ( LVN_NOT_FILE( NPtr ) ? 2 : 1 ) );
	SetBPen( RP, ( LVN_IS_HIGH(  NPtr ) ? 3 : 0 ) );

	if ( ShowSize )
		{
		Length -= 15;
		Size = DNode->Size;

		if ( RoundSizes )
			Size = ( ( Size + SECTOR_SIZE - 1 ) & ~( SECTOR_SIZE - 1 ) );

		sprintf( Buffer, "%4d %-*.*s %9d",
			 DNode->Extent,
			 Length, Length,
			 DNode->Next.ln_Name,
			 Size );
		}
	else
		{
		Length -= 24;
		if ( IS_DIR( DNode ) || IS_FILE( DNode ) )
			{
			SetISODate( DNode->Date, RecTime );
			sprintf( Buffer, "%4d %-*.*s %2d-%s-%02d %2d:%02d:%02d",
				 DNode->Extent,
				 Length, Length,
				 DNode->Next.ln_Name,
				 RecTime[ 2 ],
				 Month[ ( RecTime[ 1 ] - 1 ) ],
				 ( RecTime[ 0 ] % 100 ),
				 RecTime[ 3 ],
				 RecTime[ 4 ],
				 RecTime[ 5 ] );
			}
		else
			sprintf( Buffer, "%4d %-*.*s %*.*s",
				 DNode->Extent,
				 Length, Length,
				 DNode->Next.ln_Name,
				 18, 18, " " );
		}
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

	if ( Quitting )
		return;

	Quitting = TRUE;

	QuitISO();
	QuitOut();
	QuitDir();
	QuitMemory();

	QuitInterface( Interface );

	CloseLib( IntuitionBase );
	CloseLib( GfxBase );

	if ( Code )
		Print( "\n%s quit! (code %ld)\n", ProgramName(), Code );

	exit( Code );
	}


/***********************************************************************
***
***  ResetOptions
***
***********************************************************************/

void ResetOptions(
	void
	)
	{
	SortOrder	= BkSortOrder;
	SortReverse	= BkSortReverse;
	SortGroup	= BkSortGroup;
	GroupReverse	= BkGroupReverse;

	strcpy( IDVol,  BkIDVol  );
	strcpy( IDSet,  BkIDSet  );
	strcpy( IDPub,  BkIDPub  );
	strcpy( IDPrep, BkIDPrep );
	strcpy( IDApp,  BkIDApp  );

	NumPVD		= BkNumPVD;
	BaseSector	= BkBaseSector;

	strcpy( SplitFileName, BkSplitFileName );

	CacheData	= BkCacheData;
	CacheDir	= BkCacheDir;
	PoolLock	= BkPoolLock;
	PoolFile	= BkPoolFile;
	Retries		= BkRetries;
	DirectReadOn	= BkDirectReadOn;
	FastSearch	= BkFastSearch;
	TradeMark	= BkTradeMark;
	Speed		= BkSpeed;
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
	switch ( arg[ 0 ] )
		{
		case '-':
			switch ( tolower( arg[ 1 ] ) )
				{
				case 'l':	// -lfilename
					OptFile( &arg[ 2 ], LayoutName );
					break;

				case 'o':	// -ofilename
					OptFile( &arg[ 2 ], ImageName );
					break;

				case 's':	// -sfilename
					OptFile( &arg[ 2 ], SplitFileName );
					break;

				case 'b':
					Batch = TRUE;
					break;

				case 'x':
					VerboseFlag = TRUE;
					break;

				case 'd':
					FixCDFSDateBug = TRUE;
					break;

				case 'f':
					FileBufSize = ( atol( &arg[ 2 ] ) * SECTOR_SIZE );
					break;

				case 'm':
					UseMinimumWindow = TRUE;
					break;

				case 'q':
					switch ( atol( &arg[ 2 ] ) )
						{
						default:
						case ( 0 ):
							AssQual = Q_CTRL;
							break;

						case ( 1 ):
							AssQual = Q_SHIFT;
							break;

						case ( 2 ):
							AssQual = Q_ALT;
							break;
						}
					break;

				case 't':
					UseFont = NULL;
					break;

				case 'p':
					AddAltPathTable = FALSE;
					break;

				case 'v':
					AddVersion = FALSE;
					break;

				case 'u':
					UpperCaseNames = TRUE;
					break;

				default:
					PrintLine( WindowTitle );
					Usage( "Unrecognized program option: %s", arg, UsageText );
					break;
				}
			break;

		default:
			if ( FileCount < 1 )
				{
				OptFile( arg, SourceName );
				FileCount++;
				}
			break;

		case '?':
			PrintLine( WindowTitle );
			Usage( NULL, NULL, UsageText );
			break;
		}
	}


/***********************************************************************
***
***  SetCDFSDefaults
***
***********************************************************************/

void SetCDFSDefaults(
	void
	)
	{
	CacheData	= DEFAULT_CACHE_DATA;
	CacheDir	= DEFAULT_CACHE_DIR;
	PoolLock	= DEFAULT_POOL_LOCK;
	PoolFile	= DEFAULT_POOL_FILE;
	Retries		= DEFAULT_RETRIES;
	DirectReadOn	= FALSE;
	FastSearch	= TRUE;
	TradeMark	= TRUE;
	Speed		= FALSE;
	}


/***********************************************************************
***
***  SetCDFSDefaultsGFunc
***
***********************************************************************/

STATUS SetCDFSDefaultsGFunc(
	INTERFACE *Interface,
	GAD *Entry,
	UWORD Code
	)
	{
	SetCDFSDefaults();
	ReflectAll( Interface );
	return( OK );
	}


/***********************************************************************
***
***  SetDateMFunc
***
***********************************************************************/

STATUS SetDateMFunc(
	INTERFACE *Interface,
	UWORD Code
	)
	{
	if ( ShowSize )
		{
		ShowSize = FALSE;
		DSText.Name = "Date";
		Refresh( Interface );
		}

	return( OK );
	}


/***********************************************************************
***
***  SetOptions
***
***********************************************************************/

void SetOptions(
	void
	)
	{
	BkSortOrder	= SortOrder;
	BkSortReverse	= SortReverse;
	BkSortGroup	= SortGroup;
	BkGroupReverse	= GroupReverse;

	strcpy( BkIDVol,  IDVol  );
	strcpy( BkIDSet,  IDSet  );
	strcpy( BkIDPub,  IDPub  );
	strcpy( BkIDPrep, IDPrep );
	strcpy( BkIDApp,  IDApp  );

	BkNumPVD	= NumPVD;
	BkBaseSector	= BaseSector;

	strcpy( BkSplitFileName, SplitFileName );

	BkCacheData	= CacheData;
	BkCacheDir	= CacheDir;
	BkPoolLock	= PoolLock;
	BkPoolFile	= PoolFile;
	BkRetries	= Retries;
	BkDirectReadOn	= DirectReadOn;
	BkFastSearch	= FastSearch;
	BkTradeMark	= TradeMark;
	BkSpeed		= Speed;
	}


/***********************************************************************
***
***  SetSizeMFunc
***
***********************************************************************/

STATUS SetSizeMFunc(
	INTERFACE *Interface,
	UWORD Code
	)
	{
	RoundSizes = FALSE;

	if ( ! ShowSize )
		{
		DSText.Name = "Size";
		ShowSize = TRUE;
		}

	Refresh( Interface );

	return( OK );
	}


/***********************************************************************
***
***  SetSizeRMFunc
***
***********************************************************************/

STATUS SetSizeRMFunc(
	INTERFACE *Interface,
	UWORD Code
	)
	{
	RoundSizes = TRUE;

	if ( ! ShowSize )
		{
		DSText.Name = "Size";
		ShowSize = TRUE;
		}

	Refresh( Interface );

	return( OK );
	}


/***********************************************************************
***
***  StatusError
***
***********************************************************************/

void StatusError(
	CSTR Err
	)
	{
	if ( Err )
		sprintf( StatusErrStr, "Error: %-30.30s", Err );
	else
		sprintf( StatusErrStr, "%*.*s", 37, 37, " " );

	ReflectGadget( Interface, GAD_STATERR );
	}
