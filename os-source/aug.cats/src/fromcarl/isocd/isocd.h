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
***	ISOCD.h
***
***  PURPOSE:
***
***	Header for ISOCD modules
***
***  QUOTE:
***	"Thwaap!!!"
***		- "Several species of small, furry animals in a cave
***		   grooving with a pict" -- Ummagumma, Pink Floyd
***
***  HISTORY:
***
***	0.01 0308 C. Sassenrath	Created.
***	0.02 1630 C. Sassenrath	Updated.
***	0.03 2210 C. Sassenrath	Revisited.
***	0.04 2504 Ken Yeast	Changed to match current standards
***				(headers and other meaningless stuff)
***				Crammed in my coding style and ansi stuff
***	0.05 2505 Ken Yeast	DIRNODE changing (NODE)
***				AS/IS_DIR changed to use ln_Type
***				DIR_FLAG
***	0.06 2507 Ken Yeast	IS_DIR/FILE/PT/EMPTY and related
***
************************************************************************/


/***********************************************************************
***
***  Definitions
***
***********************************************************************/

#define	PREP_TITLE		" - ISOCD 0.00 by Pantaray, Inc. USA -"

#define	SECTOR_SIZE		2048

#define	SECTORS_USED( s )	( ( (s) + SECTOR_SIZE - 1 ) / SECTOR_SIZE )

#define	FILENAME_LEN		128

#define	MAX_DEEP		32

#define	ISO_DEEP		8

#define	FOR_EACH_NODE( d, n )	for ( (n) = (DIRNODE *) (d)->mlh_Head;		\
				      (n)->Next.ln_Succ;			\
				      (n) = (DIRNODE *) (n)->Next.ln_Succ )

#define	LVN_HIGH		( 0x80 )
#define	LVN_TEMP		( 0x40 )
#define	LVN_USETEMP		( 0x20 )

#define	IS_HIGH( t )		( ( (NODE *) (t) )->ln_Type & LVN_HIGH )
#define	HIGH_OFF( t )		( (NODE *) (t) )->ln_Type &= ~LVN_HIGH
#define	HIGH_ON( t )		( (t)->ln_Type |= LVN_HIGH )

// For line printing in custom listview
#define	LVN_NOT_FILE( t )	( (t)->ln_Type & 0x0f )
#define	LVN_IS_HIGH( t )	( (t)->ln_Type & ( ( (t)->ln_Type & LVN_USETEMP ) ? LVN_TEMP : LVN_HIGH ) )

#define	ENTRY_TYPE_MASK		( 0x0f )

enum EntryType
	{
	ET_FILE,
	ET_DIR,
	ET_PATHTABLE,
	ET_CDTV_TM,		// There is only one, we may opt to ignore user placement
	ET_EMPTY,
	ET_HEADERS
	};

#define	AS( dn, t )	(dn)->Next.ln_Type = ( ( (dn)->Next.ln_Type & ~ENTRY_TYPE_MASK ) + (t) )
#define	AS_DIR( dn )	AS( dn, ET_DIR )

#define	IS_MASK( dn )		( (dn)->Next.ln_Type & ENTRY_TYPE_MASK )
#define	IS_DIR( dn )		( IS_MASK( dn ) == ET_DIR	)
#define	IS_FILE( dn )		( IS_MASK( dn ) == ET_FILE	)
#define	IS_PT( dn )		( IS_MASK( dn ) == ET_PATHTABLE	)
#define	IS_CDTV_TM( dn )	( IS_MASK( dn ) == ET_CDTV_TM	)
#define	IS_EMPTY( dn )		( IS_MASK( dn ) == ET_EMPTY	)

#define	TM_NAME			"cdtv.tm"

#define	HEAD_STR		"<ISO Header>"
#define	PATH_STR		"<Path Table>"
#define	TM_STR			"<Trade Mark>"
#define	ROOT_STR		"<Root Dir>"
#define	END_STR			"<Empty Space>"

#define	END_PAD			32

// Julian stuff
#define	SECS_DAY	( 86400 )
#define	SECS_HOUR	( 3600 )

#define	SECS_YEAR	( 31536000 )		// ( SECS_DAY * 365 )

#define	JUL_SEC( d )	( ( (d).ds_Days * SECS_DAY ) +			\
			  ( (d).ds_Minute * 60 ) +			\
			  ( (d).ds_Tick / TICKS_PER_SECOND ) )

enum SortOrder
	{
	S_ALPHA,
	S_SIZE,
	S_DATE,
	S_NONE,
	};

enum Filter
	{
	F_INFO,
	F_DIRS,
	F_EXTENSION,
	F_NONE,
	};

enum Position
	{
	AT_HEAD,
	AT_TAIL,
	AT_BEFORE_TAIL,
	AT_AFTER_HEAD,
	AT_NOWHERE,
	};

#define	INFO_STR	".info"
#define	INFO_LEN	5

// CDFS Stuff
#define	DEFAULT_CACHE_DATA	8
#define	DEFAULT_CACHE_DIR	16
#define	DEFAULT_POOL_LOCK	40
#define	DEFAULT_POOL_FILE	16
#define	DEFAULT_RETRIES		32

#define	MIN_PVD			1
#define	MIN_CACHE_DATA		1
#define	MIN_CACHE_DIR		2
#define	MIN_POOL_LOCK		1
#define	MIN_POOL_FILE		1
#define	MIN_RETRIES		0

#define	MAX_PVD			254
#define	MAX_CACHE_DATA		127
#define	MAX_CACHE_DIR		127
#define	MAX_POOL_LOCK		9999
#define	MAX_POOL_FILE		9999
#define	MAX_RETRIES		9999

// Main window
enum Gadgets
	{
	GAD_OPTIONS,
	GAD_LOAD,
	GAD_SAVE,
	GAD_EXAMINE,
	GAD_OPTIMIZE,
	GAD_BUILD,
	GAD_SOURCE,
	GAD_IMAGE,
	GAD_LAYOUT,
	GAD_STAT,
	GAD_STATERR,
	GAD_BOX,
	GAD_SLID,
	MAX_GAD
	};


/***********************************************************************
***
***  Structures
***
***********************************************************************/

/***********************************************************************
***
***  DirNode Structure (34 bytes)
***
***	This structure is used for each directory entry.  Each node
***	contains relevant information about a file or directory.
***	Use IS_DIR/FILE/etc. to determine node type.  When a dir, the
***	Extent points to the first file in that directory.
***	Otherwise the Extent is for the file.  NextInDir for a directory
***	points to the first file in the directory and for each file points
***	to the next.  This preserves the directory tree layout.
***	The Next node allows re-arrangement of the actual storage locations
***	of all entries (files/dirs/empty spots/pathtables/etc.).
***	Next.ln_Name points to the name in the VectorPool.
***
***	Next.ln_Type flags:
***		7	<Interface Private: Actual User selection>
***		6	<Interface Private: Temporary User Selection>
***		5	<Interface Private: Use Temporary vs. Actual>
***		4	Unused
***		3	| 0-15: Entry Types (see enum EntryType)
***		2	|
***		1	|
***		0	|
***
***********************************************************************/

typedef struct DIRNODEStruct
	{
	NODE	Next;		// File list linkage (base too)
				// ln_Type has user choice/flags
				// ln_Pri not yet used
				// ln_Name is name w/o path OR path
	struct	DIRNODEStruct *NextInDir;
				// Single link list for each directory
				// This keeps the dir tree relationship
				// separate
	ULONG	Extent;		// File: LBN, Dir: Ptr to 1st entry
	ULONG	Size;		// Size in bytes
	ULONG	Date;		// Creation date
	ULONG	Position;	// Used in actual creation of image
	} DIRNODE;

/***********************************************************************
***
***  PathTable Structure (12 bytes)
***
***	Order of these entries in the array is their own numbers.
***	FirstEntry points to the first file entry, which in turn points
***	to the next file, etc.
***	DirEntry points to the entry describing this directory.
***	Parent is the parent dir number.
***
***********************************************************************/

typedef struct
	{
	DIRNODE	*FirstEntry;
	DIRNODE *DirEntry;
	ULONG	Parent;
	} PATHTABLE;


/***********************************************************************
***
***  Functions
***
***	This will be cleaned up LATER!
***
************************************************************************/

// Dir.c
STATUS	BuildDirList( MLIST *, CSTR );
DIRNODE	*FindFile( MLIST *, CSTR );
void	InitDir( void );
STATUS	LoadFile( CSTR, MLIST *, CSTR );
DIRNODE	*MakeDNode( MLIST *, CSTR, LONG, LONG, enum EntryType, enum Position );
void	QuitDir( void );
void	SaveFile( CSTR, MLIST *, CSTR );

// Find.c
STATUS	FindMFunc( INTERFACE *, UWORD );

// ISO.c
FLAG	CheckPVD( UBYTE * );
STATUS	CreateImage( MLIST *, CSTR );
STATUS	DeterminePositions( MLIST * );
void	InitISO( void );
void	MakePathName( CSTR, DIRNODE * );
void	QuitISO( void );
void	SetISODate( LONG, UBYTE * );
void	SetRootName( void );
void	SetRootPath( CSTR );
void	UnSetRootName( void );
STATUS	VerifyImage( MLIST *, CSTR );

// ISOCD.c
void	InformDirLoaded( STATE );
void	StatusError( CSTR );

// Optimize.c
STATUS	OptimizePanel( INTERFACE * );

// Out.c
void	CloseImage( void );
void	CloseSplit( void );
void	InitOut( void );
STATUS	OpenImage( void );
void	QuitOut( void );
STATUS	WriteOut( CSTR, LONG );

// Space.c
STATUS	SpaceAddMFunc( INTERFACE *, UWORD );
STATUS	SpaceDelMFunc( INTERFACE *, UWORD );
