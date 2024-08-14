/************************************************************************
 *									*
 *	Copyright (c) 1989 Enhanced Software Technologies, Inc.		*
 *			   All Rights Reserved				*
 *									*
 *	This software and/or documentation is protected by U.S.		*
 *	Copyright Law (Title 17 United States Code).  Unauthorized	*
 *	reproduction and/or sales may result in imprisonment of up	*
 *	to 1 year and fines of up to $10,000 (17 USC 506).		*
 *	Copyright infringers may also be subject to civil liability.	*
 *									*
 ************************************************************************
 */

#include "standard.h"
#include "scansort.h"
#include "bailout.h"
#include "brushell.h"
#include "dirtree.h"
#include "eventloop.h"
#include "logfile.h"
#include "mainwin.h"
#include "menus.h"
#include "dbug.h"
#include "libfuncs.h"
#include "reqs.h"


#define BRU_FILE_SIZE(x)	(   (x) > 1792 ? \
( ( (x-1) / 1792 ) + 1 ) * 2048 : 2048  )
/* #define BRU_FILE_SIZE(x)			(((x)/1792) * 2048) */
#define BRU_PER_FILE_OVERHEAD		2048
#define BRU_PER_ARCHIVE_OVERHEAD	4096


static int type_compare PROTO((struct TreeEntry *, struct TreeEntry *));
static BOOL _do_include PROTO((struct TreeEntry *));
static BOOL _do_exclude PROTO((struct TreeEntry *));
static BOOL do_the_sort PROTO((struct TreeNode *));
static BOOL totalize_files PROTO((struct TreeEntry *));
static BOOL totalize_nodes PROTO((struct TreeNode *));
static BOOL by_size PROTO((struct TreeEntry *));
static BOOL by_date PROTO((struct TreeEntry *));
static BOOL by_pattern PROTO((struct TreeEntry *));
static BOOL by_arc PROTO((struct TreeEntry *));
static BOOL _shadow PROTO((struct TreeEntry *));
static BOOL _un_shadow PROTO((struct TreeEntry *));
static BOOL _clear_backed PROTO((struct TreeEntry *));
static BOOL _select_all PROTO((struct TreeEntry *));
static BOOL _select_none PROTO((struct TreeEntry *));
static int calc_vols_needed( ULONG size );
static ULONG get_vol_size( char *device );


/*
 * This function pointer points at the function to actually do the
 * comparison between two entrys during a sort.
 */

int (*sort_func) () = alpha_compare;

/*
 * If this flag is greater than zero, directorys will be sorted to the
 * top of the list.
 */

int dflag = 1;

/* These are filled in by file_status() */

int file_count;
ULONG file_size;

int file_selected_count;
ULONG file_selected_size;

ULONG final_archive_size;
int final_archive_vols;


/* Variables used by the include/exclude criteria functions */

static ULONG date;
static ULONG size;
static int date_mode;
static int size_mode;
static int pattern_mode;
static char pattern[MAX_NAME];
static int arc_mode;

/* Filled in by the treenode statistics functions */

int node_count;
ULONG node_memsize;
int node_entrys;
int most_entrys;

/*..............................................................*/
/*								*/
/*		Sort by Entry Name				*/
/*..............................................................*/

static int  type_compare (te1, te2)
struct TreeEntry *te1;
struct TreeEntry *te2;
{
    DBUG_ENTER ("type_compare");
    if (FlagIsSet (te1 -> Flags, ENTRY_IS_DIR) &&
	    FlagIsClear (te2 -> Flags, ENTRY_IS_DIR)) {
	/* te1 only is a directory, move it to front of list always */
	DBUG_RETURN (-1);
    }
    if (FlagIsSet (te1 -> Flags, ENTRY_IS_DIR) &&
	    FlagIsClear (te2 -> Flags, ENTRY_IS_DIR)) {
	/* te2 only is a directory, move it to front of list always */
	DBUG_RETURN (1);
    }
    /* Either both are files or both are dirs.  Equal as far as we care */
    DBUG_RETURN (0);
}

/*
 * This can be called-back by the QuickSort to render judgment
 * based upon file name (case insensitive).
 */

int alpha_compare (te1, te2)
struct TreeEntry *te1;
struct TreeEntry *te2;
{
    int rtnval;

    DBUG_ENTER ("alpha_compare");
    rtnval = stricmp (te1 -> Name, te2 -> Name);
    DBUG_RETURN (rtnval);
}

/*
 * This can be called-back by the QuickSort to render judgment
 * based upon file date.
 */

int date_compare (te1, te2)
struct TreeEntry *te1;
struct TreeEntry *te2;
{
    DBUG_ENTER ("date_compare");
    if (te1 -> Date > te2 -> Date) {
	DBUG_RETURN (1);
    }
    if (te1 -> Date < te2 -> Date) {
	DBUG_RETURN (-1);
    }
    DBUG_RETURN (0);
}

/*
 * This can be called-back by the QuickSort to render judgment
 * based upon file size.
 */

int size_compare (te1, te2)
struct TreeEntry *te1;
struct TreeEntry *te2;
{
    DBUG_ENTER ("size compare");
    if (te1 -> Size > te2 -> Size) {
	DBUG_RETURN (1);
    }
    if (te1 -> Size < te2 -> Size) {
	DBUG_RETURN (-1);
    }
    DBUG_RETURN (0);
}

/*
 * This can be called-back by the QuickSort to render judgment
 * based upon file name (case insensitive).
 */

int archive_compare (te1, te2)
struct TreeEntry *te1;
struct TreeEntry *te2;
{
    DBUG_ENTER ("archive_compare");
    if (FlagIsSet (te1 -> Status, FIBF_ARCHIVE) &&
	    FlagIsClear (te2 -> Status, FIBF_ARCHIVE)) {
	DBUG_RETURN (1);
    }
    if (FlagIsSet (te2 -> Status, FIBF_ARCHIVE) &&
	    FlagIsClear (te1 -> Status, FIBF_ARCHIVE)) {
	DBUG_RETURN (-1);
    }
    DBUG_RETURN (0);
}

/*
 * This can be passed to walk_nodes() to perform the sort defined by
 * the pointer-to-function  sort_func
 * upon each treenode.
 */

static BOOL do_the_sort (tn)
struct TreeNode *tn;
{
    int i = 0;
    struct TreeEntry *te;

    DBUG_ENTER ("do_the_sort");
    /* See if the user would like to cancel the sort */
    if (check_cancel () == TRUE) {
	DBUG_RETURN (FALSE);
    }
    /* First, we may need to sort directorys from files */
    if (dflag > 0) {
	/* Sort dirs to the top */
	qsort ((char *) &(tn -> FirstEntry), (int) (tn -> Entries),
		(int) sizeof (struct TreeEntry), type_compare);
	/* Now, find out where the dirs end and the files begin. */
	for (i = 0; i < tn -> Entries; i++) {
	    te = find_entry (tn, i);
	    if (FlagIsClear (te -> Flags, ENTRY_IS_DIR)) {
		break;
	    }
	}
	/* Sort the directory entrys (if there are at least two!) */
	if (i >= 2) {
	    /* At this time, the only sort which makes sense for
	     * directorys is the alphabetic name sort.
	     */
	    qsort ((char *) &(tn -> FirstEntry), i,
		    (int) sizeof (struct TreeEntry), alpha_compare);
	}
    }
    /* Sort the file entrys (if there are at least two!) */
    if (i <= (tn -> Entries - 2)) {
	qsort ((char *) find_entry (tn, i), (tn -> Entries - i),
		(int) sizeof (struct TreeEntry), sort_func);
    }
    DBUG_RETURN (TRUE);
}

void sort (tn)
struct TreeNode *tn;
{
    DBUG_ENTER ("sort");
    post_cancel_req (mainwin, "Sorting");
    walk_nodes (tn, do_the_sort, NULL);
    clear_cancel_req ();
    DBUG_VOID_RETURN;
}

/*..............................................................*/
/*								*/
/*		TreeNode statistics				*/
/*..............................................................*/

static BOOL totalize_nodes (tn)
struct TreeNode *tn;
{
    DBUG_ENTER ("totalize_nodes");
    node_count++;
    node_memsize += tn -> Size;
    node_entrys += tn -> Entries;
    if (tn -> Entries > most_entrys) {
	most_entrys = tn -> Entries;
    }
    DBUG_RETURN (TRUE);
}

void node_status (tn)
struct TreeNode *tn;
{
    DBUG_ENTER ("node_status");
    node_count = 0;
    node_memsize = 0;
    node_entrys = 0;
    most_entrys = 0;
    walk_nodes (tn, totalize_nodes, NULL);
    DBUG_PRINT ("nodes", ("%d nodes", node_count));
    DBUG_PRINT ("entries", ("%d entries", node_entrys));
    DBUG_PRINT ("memory", ("%ld memory used", node_memsize));
    DBUG_PRINT ("entries", ("%d max entries in a node", most_entrys));
    DBUG_VOID_RETURN;
}

/*..............................................................*/
/*								*/
/*		TreeEntry Statistics				*/
/*..............................................................*/

static BOOL totalize_files (te)
struct TreeEntry *te;
{
    DBUG_ENTER ("totalize_files");

    if(  FlagIsClear( te -> Flags, ENTRY_IS_DIR )  )
	{
	    file_count++;
	    file_size += te -> Size;
	}

    if (FlagIsSet (te -> Flags, ENTRY_SELECTED)
	    && FlagIsClear (te -> Flags, ENTRY_SHADOWED))
	{
	    if(  FlagIsClear( te -> Flags, ENTRY_IS_DIR )  )
		{
			file_selected_count++;
			file_selected_size += te -> Size;
			final_archive_size += BRU_FILE_SIZE(te->Size);
		}
		else
		{
			final_archive_size += BRU_PER_FILE_OVERHEAD;
		}
    }

    DBUG_RETURN (TRUE);
}

void file_status (tn)
struct TreeNode *tn;
{
	char dummy_buf[256];


    DBUG_ENTER ("file_status");
    file_count = 0;
    file_size = 0;
    file_selected_count = 0;
    file_selected_size = 0;
	final_archive_size = BRU_PER_ARCHIVE_OVERHEAD;
    if (tn == NULL) {
	DBUG_VOID_RETURN;
    }

	if( backup_the_dirs == TRUE )
	{
	    walk_entrys (tn, totalize_files, dummy_buf);
	}
	else
	{
	    walk_files (tn, totalize_files);
	}

	/* Finish the estimated archive size calculations.
	 * This is done in "two pieces" to speed up the tree scanning
	 * time.
	 */
	final_archive_size += BRU_PER_FILE_OVERHEAD * file_selected_count;

	if( embed_logfile == TRUE ) {
	final_archive_size += BRU_FILE_SIZE(logfile_size( FALSE )) +
		BRU_PER_FILE_OVERHEAD;
	}

    if( FlagIsSet( compression_item[COMPRESS_ALL_ITEM].Flags, CHECKED ) ||
    	FlagIsSet( compression_item[COMPRESS_LARGER_ITEM].Flags, CHECKED))
	{
		/* Compression on.  Cannot calculate volumes. */
		final_archive_vols = -1;
	}
	else
	{
		final_archive_vols = calc_vols_needed( final_archive_size );
	}

    DBUG_VOID_RETURN;
}



static int calc_vols_needed( ULONG size )
{
	return( (int)( (size / get_vol_size("DF0:")) * 10 ) + 1 );
		/* Note: this must be divided by ten to get volumes.
		 * Allows display of fractional volumes w/o floating point
		 */
}


/*
 * Note!  This will NEVER return a number less than 10!
 */

static ULONG get_vol_size( char *device )
{
	if(  stricmp( device, "DF0:" ) == 0  )
		return( 900096 );

	if(  stricmp( device, "DF1:" ) == 0  )
		return( 900096 );

	if(  stricmp( device, "DF2:" ) == 0  )
		return( 900096 );

	if(  stricmp( device, "DF3:" ) == 0  )
		return( 900096 );

	return( 100000 );
}



/*..............................................................*/
/*								*/
/*			Mass Selections				*/
/*..............................................................*/

/*-------------------  All   ---------------------*/

static BOOL _select_all (te)
struct TreeEntry *te;
{
    DBUG_ENTER ("_select_all");
    SetFlag (te -> Flags, ENTRY_SELECTED);
    if (mode == RESTORE_MODE) {
	if (FlagIsClear (te -> Flags, ENTRY_BACKEDUP)) {
	    ClearFlag (te -> Flags, ENTRY_SELECTED);
	}
    }
    DBUG_RETURN (TRUE);
}

void select_all (tn)
struct TreeNode *tn;
{
    DBUG_ENTER ("select_all");
    walk_files (tn, _select_all);
    DBUG_VOID_RETURN;
}

static BOOL _select_none (te)
struct TreeEntry *te;
{
    DBUG_ENTER ("_select_none");
    ClearFlag (te -> Flags, ENTRY_SELECTED);
    DBUG_RETURN (TRUE);
}

void select_none (tn)
struct TreeNode *tn;
{
    DBUG_ENTER ("select_none");
    walk_files (tn, _select_none);
    DBUG_VOID_RETURN;
}

void do_include (tn)
struct TreeNode *tn;
{
    DBUG_ENTER ("do_include");

    if(  init_criteria () != 0  ) {
	mention_error ("Invalid date criteria", ERC_NONE | ERR41);
    DBUG_VOID_RETURN;
	}

    walk_files (tn, _do_include);
    DBUG_VOID_RETURN;
}

static BOOL _do_include (te)
struct TreeEntry *te;
{
    DBUG_ENTER ("_do_include");
    if (want_file (te) == TRUE) {
	SetFlag (te -> Flags, ENTRY_SELECTED);
    }
    if (mode == RESTORE_MODE) {
	if (FlagIsClear (te -> Flags, ENTRY_BACKEDUP)) {
	    ClearFlag (te -> Flags, ENTRY_SELECTED);
	}
    }
    DBUG_RETURN (TRUE);
}

void do_exclude (tn)
struct TreeNode *tn;
{
    DBUG_ENTER ("do_exclude");
    if(  init_criteria () != 0  ) {
	mention_error ("Invalid date criteria", ERC_NONE | ERR42);
    DBUG_VOID_RETURN;
	}
    walk_files (tn, _do_exclude);
    DBUG_VOID_RETURN;
}

static BOOL _do_exclude (te)
struct TreeEntry *te;
{
    DBUG_ENTER ("_do_exclude");
    if (want_file (te) == TRUE) {
	ClearFlag (te -> Flags, ENTRY_SELECTED);
    }
    DBUG_RETURN (TRUE);
}


/* This will return a 0 if the criteria were legal.
 * A -1 will be returned if the date is ill-formed.
 */

int init_criteria ()
{
    DBUG_ENTER ("init_criteria");
    date_mode = 0;
    size_mode = 0;
    pattern_mode = 0;
    arc_mode = 0;
    /* Set size criteria */
    if (FlagIsSet (size_gadget[0].Flags, SELECTED)) {
	size = ((struct StringInfo *)
                (size_gadget[2].SpecialInfo)) -> LongInt;
	size_mode = (int) ((long) (size_gadget[1].UserData));
    }
    /* Set date criteria */
    if (FlagIsSet (date_gadget[0].Flags, SELECTED)) {
	date_mode = (int) ((long) (date_gadget[1].UserData));
	date = ascwhen_to_secs (
	    ((struct StringInfo *) (date_gadget[2].SpecialInfo)) -> Buffer);

	if( date == 0 ) {
	DBUG_RETURN (-1);
	}

	switch( date_mode ) {
		case 1:
			break;
		case 2:
			date += 86399;		/* Set to last second of day */
			break;
		case 3:
			date /= 86400;		/* Must ignore time for "on" case... */
			break;
		}
    }
    /* by Pattern */
    if (FlagIsSet (pattern_gadget[0].Flags, SELECTED)) {
	strcpy (pattern, ((struct StringInfo *)
	                  (pattern_gadget[2].SpecialInfo)) -> Buffer);
	pattern_mode = (int) ((long) (pattern_gadget[1].UserData));
    }
    /* Set arc criteria */
    if (FlagIsSet (arc_gadget[0].Flags, SELECTED)) {
	arc_mode = (int) ((long) (arc_gadget[1].UserData));
    }
    DBUG_PRINT ("date", ("date of %ld", date));
    DBUG_PRINT ("size", ("size of %ld", size));
    DBUG_PRINT ("pattern", ("pattern of %s", pattern));
    DBUG_PRINT ("arcbit", ("arc bit of %d", arc_mode));
    DBUG_RETURN (0);
}

/*..................................................................*/
/*								*/
/*	Tests of TreeEntry params against include/exclude criteria. */
/*..................................................................*/

/*
 * Test a file entry against all of the currently specified
 * inclusion/exclusion criteria, returning TRUE if the file meets same.
 * Criteria should have been preset via a call to init_criteria().
 */

BOOL want_file (te)
struct TreeEntry *te;
{
    BOOL rtnval;

    DBUG_ENTER ("want_file");
    rtnval = Bool (by_size (te) && by_date (te) && by_pattern (te) &&
		   by_arc (te));
    DBUG_RETURN (rtnval);
}

/*
 * For all of the following functions, if the specific mode is set to
 * zero, they default to returning TRUE.
 */

static BOOL by_size (te)
struct TreeEntry *te;
{
    DBUG_ENTER ("by_size");
    switch (size_mode) {
	case 1: 
	    if (te -> Size < size) {
		DBUG_RETURN (TRUE);
	    }
	    break;
	case 2: 
	    if (te -> Size > size) {
		DBUG_RETURN (TRUE);
	    }
	    break;
	case 3: 
	    if (te -> Size == size) {
		DBUG_RETURN (TRUE);
	    }
	    break;
	default: 
	    DBUG_RETURN (TRUE);
    }
    DBUG_RETURN (FALSE);
}

static BOOL by_date (te)
struct TreeEntry *te;
{
    DBUG_ENTER ("by_date");
    switch (date_mode) {
	case 1: 
	    if (te -> Date < date) {
		DBUG_RETURN (TRUE);
	    }
	    break;
	case 2: 
	    if (te -> Date > date) {
		DBUG_RETURN (TRUE);
	    }
	    break;
	case 3: 
	    /* Exact only uses days, not hours etc. */
	    if ((te -> Date / 86400) == date) {
		DBUG_RETURN (TRUE);
	    }
	    break;
	default: 
	    DBUG_RETURN (TRUE);
    }
    DBUG_RETURN (FALSE);
}

static BOOL by_pattern (te)
struct TreeEntry *te;
{
    DBUG_ENTER ("by_pattern");
    switch (pattern_mode) {
	case 1: 
	    if (astcsma (te -> Name, pattern) != 0) {
		DBUG_RETURN (TRUE);
	    }
	    break;
	case 2: 
	    if (astcsma (te -> Name, pattern) == 0) {
		DBUG_RETURN (TRUE);
	    }
	    break;
	default: 
	    DBUG_RETURN (TRUE);
    }
    DBUG_RETURN (FALSE);
}

static BOOL by_arc (te)
struct TreeEntry *te;
{
    DBUG_ENTER ("by_arc");
    switch (arc_mode) {
	case 1: 
	    if (FlagIsSet (te -> Status, FIBF_ARCHIVE)) {
		DBUG_RETURN (TRUE);
	    }
	    break;
	case 2: 
	    if (FlagIsClear (te -> Status, FIBF_ARCHIVE)) {
		DBUG_RETURN (TRUE);
	    }
	    break;
	default: 
	    DBUG_RETURN (TRUE);
    }
    DBUG_RETURN (FALSE);
}

/*..............................................................*/
/*								*/
/*			File shadowing / un-shadowing		*/
/*..............................................................*/

static BOOL _shadow (te)
struct TreeEntry *te;
{
    DBUG_ENTER ("_shadow");
    SetFlag (te -> Flags, ENTRY_SHADOWED);
    DBUG_RETURN (TRUE);
}

void shadow (tn)
struct TreeNode *tn;
{
    DBUG_ENTER ("shadow");
    walk_entrys (tn, _shadow, NULL);
    DBUG_VOID_RETURN;
}

static BOOL _un_shadow (te)
struct TreeEntry *te;
{
    DBUG_ENTER ("_un_shadow");
    ClearFlag (te -> Flags, ENTRY_SHADOWED);
    DBUG_RETURN (TRUE);
}

void un_shadow (tn)
struct TreeNode *tn;
{
    DBUG_ENTER ("un_shadow");
    walk_entrys (tn, _un_shadow, NULL);
    DBUG_VOID_RETURN;
}

static BOOL _clear_backed (te)
struct TreeEntry *te;
{
    DBUG_ENTER ("_clear_backed");
    ClearFlag (te -> Flags, ENTRY_BACKEDUP);
    DBUG_RETURN (TRUE);
}

void clear_backed (tn)
struct TreeNode *tn;
{
    DBUG_ENTER ("clear_backed");
    walk_entrys (tn, _clear_backed, NULL);
    DBUG_VOID_RETURN;
}
