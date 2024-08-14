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

#include "bailout.h"
#include "brushell.h"
#include "dirtree.h"
#include "backup.h"
#include "eventloop.h"
#include "help.h"
#include "info_req.h"
#include "mainwin.h"
#include "menus.h"
#include "reqs.h"
#include "rexxcom.h"
#include "scansort.h"
#include "slist.h"
#include "dbug.h"
#include "libfuncs.h"
#include "logfile.h"
#include "hdbackup_rev.h"



/*
 *	Local functions.
 */

static void start PROTO((void));
static void check_mainwin_port PROTO((void));
static void process_gadget PROTO((USHORT, LONG, USHORT));
static void process_options PROTO((int,int));
static void process_sort PROTO((int));
static void process_test PROTO((int));
static void process_menu PROTO((USHORT));
static void process_project PROTO((int));
static void perform_inclusion PROTO((struct TreeNode *));
static void perform_exclusion PROTO((struct TreeNode *));
static void process_slist_sel PROTO((int, USHORT));
static void toggle_range PROTO((int, int));
static void toggle_file PROTO((int));
static void toggle_dir PROTO((int));
static void cancel_range PROTO((void));
static void debug_about ( void );


/*
 * When set true, this will cause us to leave the eventloop
 * and exit the program.
 */

static BOOL finished = FALSE;

static SHORT vector_data[2][10] = {
    { 0, 0, 29, 0, 29, 39, 0, 39, 0, 0 },
    { 0, 0, 27, 0, 27, 37, 0, 37, 0, 0 }
};

static struct Border box[2] = {
    { 0, 0, 2, 0, JAM1, 5, vector_data[0], NULL },
    { 1, 1, 3, 0, JAM1, 5, vector_data[1], NULL }
};

int mode = NO_MODE;

static LONG event_secs;
static LONG event_micros;
static LONG prev_secs;
static LONG prev_micros;
static int range_anchor_entry = -1;

char arch_level_buf[SBUF_SIZE] = "100k";
LONG arch_level = (1024 * 100);


static char title_buf[81];


void eventloop ()
{
    ULONG waitflags;
    ULONG waitmask;


    DBUG_ENTER ("eventloop");
    /* Do some auto startup options */
    if (auto_backup == TRUE) {
	init_backup ();
    }
    if (auto_restore == TRUE) {
	init_restore ();
    }
    if (auto_start == TRUE) {
	start ();
    }
    while (!finished) {		/* loop until user say done */
	waitmask = mainwin_sig_bit | ipc_sig_bit;
	waitflags = Wait (waitmask);
	/* First check if it was from the mainwindow Intuition port */
	if (waitflags & mainwin_sig_bit) {
	    check_mainwin_port ();
	}
	/* Any activity at the IPC port? */
	if (waitflags & ipc_sig_bit) {
		check_ipc_port();
	}
    }
    DBUG_VOID_RETURN;
}

static void check_mainwin_port ()
{
    /* define some variables to store Intuition Message data in */
    struct IntuiMessage *message;
    ULONG class;
    USHORT code;
    USHORT qualifier;
    void *address;	/* void; this can point to Screens, Gadgets, etc. */

    DBUG_ENTER ("check_mainwin_port");
    while (message = (struct IntuiMessage *) GetMsg (mainwin -> UserPort)) {
	class = message -> Class;
	code = message -> Code;
	qualifier = message -> Qualifier;
	address = message -> IAddress;
	/* Save time values for double-click testing */
	prev_secs = event_secs;
	prev_micros = event_micros;
	event_secs = message -> Seconds;
	event_micros = message -> Micros;
	ReplyMsg ((struct Message *) message);
	switch (class) {
	    case MENUPICK: 
		process_menu (code);
		break;
	    case GADGETDOWN: 
	    case GADGETUP: 
		process_gadget ((USHORT) (((struct Gadget  *)
				 address) -> GadgetID), class, qualifier);
		break;
	}
    }
    DBUG_VOID_RETURN;
}

/*-----------------------  Deal with Menus  ----------------------*/

static void process_menu (menucode)
USHORT menucode;
{
    int menu, item, subitem;
    struct MenuItem *itm;
    extern struct MenuItem *ItemAddress ();

    DBUG_ENTER ("process_menu");
	cancel_range();						/* End range op if need be */
    while (menucode != MENUNULL) {
	menu = MENUNUM (menucode);
	item = ITEMNUM (menucode);
	subitem = SUBNUM (menucode);
	switch (menu) {
	    case PROJECT_MENU: 
		process_project (item);
		break;
	    case SORT_MENU: 
		process_sort (item);
		break;
	    case OPTIONS_MENU: 
		process_options (item,subitem);
		break;
	    case TEST_MENU: 
		process_test (item);
		break;
	}
	/* get next selection (if any) */
	itm = ItemAddress (&project_menu, (LONG) menucode);
	menucode = itm -> NextSelect;
    }
    /* menu_status (); */	/* make any ghosting changes needed */
    DBUG_VOID_RETURN;
}

static void process_project (item)
int item;
{
    DBUG_ENTER ("process_project");
    switch (item) {
	case BACKUP_ITEM: 
	    init_backup ();
	    break;
	case RESTORE_ITEM: 
	    init_restore ();
	    break;
	case RECOVER_ITEM:
		recover();
		break;
	case DIFFERENCE_ITEM: 
	    init_diff ();
	    break;
	case INSPECT_ITEM: 
	    init_inspect ();
	    break;
	case HELP_ITEM:
		help( HELP_FILENAME );
	    break;
	case ABOUT_ITEM: 	/* About */
	    tell_about ();
	    break;
	case INFO_ITEM: 	/* Info */
	    tell_info ();
	    break;
	case QUIT_ITEM: 	/* Quit */
	    finished = TRUE;
	    break;
    }
    DBUG_VOID_RETURN;
}

static void process_sort (item)
int item;
{
    DBUG_ENTER ("process_sort");
    SetWaitPointer (mainwin);
    switch (item) {
	case SORT_DIRS_ITEM: 
	    break;
	case SORTBY_NAME_ITEM: 
	    sort_func = alpha_compare;
	    break;
	case SORTBY_DATE_ITEM: 
	    sort_func = date_compare;
	    break;
	case SORTBY_SIZE_ITEM: 
	    sort_func = size_compare;
	    break;
	case SORTBY_ARCHIVE_ITEM: 
	    sort_func = archive_compare;
	    break;
    }
    dflag = FlagIsSet (sort_item[SORT_DIRS_ITEM].Flags, CHECKED);
    /* 
     * We can only do a sort if we have built a tree and are in backup
     * mode.
     */
    if ((mode == BACKUP_MODE) || (mode == BACKUP_COMPLETE)) {
	sort (root_node);
	format_entrys (current_node);
	update_slist (0);
    }
    ClearWaitPointer (mainwin);
    DBUG_VOID_RETURN;
}



static void process_options (item, subitem)
int item;
int subitem;
{
	static char buf2[SBUF_SIZE];


    DBUG_ENTER ("process_options");

	switch( item )
	{
		case BACK_DIR_ITEM:
			if (FlagIsSet (options_item[BACK_DIR_ITEM].Flags, CHECKED))
			{
				backup_the_dirs = TRUE;
			}
			else
			{
				backup_the_dirs = FALSE;
			}
			stats();
			break;

		case COMPRESSION_ITEM:
			switch( subitem )
			{
				case COMPRESS_NONE_ITEM:
#if 0
					ClearMenuStrip( mainwin );
					SetFlag( compression_item[COMPRESS_NONE_ITEM].Flags,
						CHECKED );
					ClearFlag( compression_item[COMPRESS_ALL_ITEM].Flags,
						CHECKED );
					ClearFlag( compression_item[COMPRESS_LARGER_ITEM].Flags,
						CHECKED );
					SetMenuStrip( mainwin, &project_menu );
#endif
					break;

				case COMPRESS_ALL_ITEM:
#if 0
					ClearMenuStrip( mainwin );
					SetFlag( compression_item[COMPRESS_ALL_ITEM].Flags,
						CHECKED );
					ClearFlag( compression_item[COMPRESS_NONE_ITEM].Flags,
						CHECKED );
					ClearFlag( compression_item[COMPRESS_LARGER_ITEM].Flags,
						CHECKED );
					SetMenuStrip( mainwin, &project_menu );
#endif
					break;

				case COMPRESS_LARGER_ITEM:
					for(;;)
					{
						do_string_req( "Compress Files Larger than:",
							arch_level_buf );

						if( kmstring_to_long( &arch_level,
							arch_level_buf ) == TRUE )
						{
							/* A valid entry */
							break;
						}
						else
						{
							mention_error( "Invalid compression size.",
								ERC_NONE | ERR49 );
						}
					}
					ClearMenuStrip( mainwin );
#if 0
					SetFlag( compression_item[COMPRESS_LARGER_ITEM].Flags,
						CHECKED );
					ClearFlag( compression_item[COMPRESS_NONE_ITEM].Flags,
						CHECKED );
					ClearFlag( compression_item[COMPRESS_ALL_ITEM].Flags,
						CHECKED );
#endif
					sprintf( buf2, "Larger Than [%s]", arch_level_buf );
					((struct IntuiText *)
					(compression_item[COMPRESS_LARGER_ITEM].ItemFill))
					->IText = buf2;
					SetMenuStrip( mainwin, &project_menu );
					break;
			}
			stats();	/* Compression effects the volumes needed! */
			break;
	}

    DBUG_VOID_RETURN;
}



static void process_test (item)
int item;
{
	char *p;


    DBUG_ENTER ("process_test");

    switch (item) {
	case WRITELOG_ITEM: 
	    save_tree (root_node);
	    break;

	case T1_ITEM:
		do_string_req( "Logfile format string", logfile_pattern );
		p = when_pattern( logfile_pattern );
		mention_error( p, 0 );	/* Not an error, just a quick and easy
								 * way to show the result.
								 */
		break;

	case T2_ITEM:
		debug_about();
		break;
	}

    DBUG_VOID_RETURN;
}


static void start ()
{
    DBUG_ENTER ("start");
    switch (mode) {
	case BACKUP_MODE: 
	    perform_backup ();
	    break;
	case RESTORE_MODE: 
	    perform_restore ();
	    break;
	case DIFF_MODE: 
	    perform_diff ();
	    break;
	case INSPECT_MODE: 
	    perform_inspect ();
	    break;
	default: 
	    mention_error ("You must select a mode first", 0);
	    break;
    }
    DBUG_VOID_RETURN;
}

/*----------------------  Deal with Gadgets  --------------------*/

static void process_gadget (gadgetid, class, qualifier)
USHORT gadgetid;
LONG class;
USHORT qualifier;
{
    int rc;
    ULONG pos;
    ULONG x;

    DBUG_ENTER ("process_gadget");
    /* This is the point where we prevent operations from being performed
     * which are not appropriate.
     */
    if (mode == NO_MODE) {
	/* Only want to deal with changing the text in criteria gadgets
	 * if there is no mode selected yet.
	 */
	if ((gadgetid != DATE_GADGET_ID) &&
		(gadgetid != SIZE_GADGET_ID) &&
		(gadgetid != ARC_GADGET_ID) &&
		(gadgetid != PATTERN_GADGET_ID)) {
	    /* Gadget hit was not one we want to deal with now */
	    DBUG_VOID_RETURN;
	}
    }
    switch (gadgetid) {
	case ROOT_GADGET_ID: 
		cancel_range();						/* End range op if need be */
	    to_root ();
	    SetWaitPointer (mainwin);
	    format_entrys (current_node);
	    update_slist (1);
	    ClearWaitPointer (mainwin);
	    break;
	case PARENT_GADGET_ID: 
		cancel_range();						/* End range op if need be */
	    ascend ();
	    SetWaitPointer (mainwin);
	    format_entrys (current_node);
	    update_slist (1);
	    ClearWaitPointer (mainwin);
	    break;
	case INCLUDE_ID: 
	    perform_inclusion (current_node);
	    break;
	case EXCLUDE_ID: 
	    perform_exclusion (current_node);
	    break;
	case DATE_GADGET_ID: 
	    /* Cycle the gadget text to the next selection */
	    pos = RemoveGadget (mainwin, &date_gadget[1]);
	    x = (ULONG) (date_gadget[1].UserData);
	    if (++x > 3) {
		x = 1;
	    }
	    date_gadget[1].GadgetText = (struct IntuiText *) &date_text[x];
	    date_gadget[1].UserData = (APTR) x;
	    AddGadget (mainwin, &date_gadget[1], pos);
	    RefreshGList (&date_gadget[1], mainwin, NULL, 1L);
	    break;
	case SIZE_GADGET_ID: 
	    /* Cycle the gadget text to the next selection */
	    pos = RemoveGadget (mainwin, &size_gadget[1]);
	    x = (ULONG) (size_gadget[1].UserData);
	    if (++x > 3) {
		x = 1;
	    }
	    size_gadget[1].GadgetText = (struct IntuiText *) &size_text[x];
	    size_gadget[1].UserData = (APTR) x;
	    AddGadget (mainwin, &size_gadget[1], pos);
	    RefreshGList (&size_gadget[1], mainwin, NULL, 1L);
	    break;
	case PATTERN_GADGET_ID: 
	    /* Cycle the gadget text to the next selection */
	    pos = RemoveGadget (mainwin, &pattern_gadget[1]);
	    x = (ULONG) (pattern_gadget[1].UserData);
	    if (++x > 2) {
		x = 1;
	    }
	    pattern_gadget[1].GadgetText =
		(struct IntuiText *) &pattern_text[x];
	    pattern_gadget[1].UserData = (APTR) x;
	    AddGadget (mainwin, &pattern_gadget[1], pos);
	    RefreshGList (&pattern_gadget[1], mainwin, NULL, 1L);
	    break;
	case ARC_GADGET_ID: 
	    /* Cycle the gadget text to the next selection */
	    pos = RemoveGadget (mainwin, &arc_gadget[1]);
	    x = (ULONG) (arc_gadget[1].UserData);
	    if (++x > 2) {
		x = 1;
	    }
	    arc_gadget[1].GadgetText = (struct IntuiText *) &arc_text[x];
	    arc_gadget[1].UserData = (APTR) x;
	    AddGadget (mainwin, &arc_gadget[1], pos);
	    RefreshGList (&arc_gadget[1], mainwin, NULL, 1L);
	    break;
	case START_ID:
		cancel_range();						/* End range op if need be */
		start ();
		break;
	default: 
	    if (rc = handle_slist ((LONG) gadgetid, class)) {
		/* Hey, the user selected something */
		process_slist_sel (rc - 1, qualifier);
	    }
	    break;
    }
    DBUG_VOID_RETURN;
}

/*
 * This is called when a gadget in the list is clicked.  This will
 * take any action neccessary, which breaks down into two things.
 * Either toggle the state of a file entry, or descend into a directory.
 */

static void process_slist_sel (entry, qualifier)
int entry;
USHORT qualifier;
{
    struct TreeEntry *te;
    static int p_entry=-1;
    char buf[200];

    DBUG_ENTER ("process_slist_sel");
    te = find_entry (current_node, entry);

    if (qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
    {
	/* A shift-click */
	if( range_anchor_entry == -1 )
	{
	    /* First one, just save it and watch for the next one */
	    range_anchor_entry = entry;
		SetRangePointer( mainwin );
	}
	else
	{
	    /* Second one, let's do something */
		ClearRangePointer( mainwin );
	    toggle_range( entry, range_anchor_entry );
	    range_anchor_entry = -1;
	}

	/* DTM_NEW */
	DBUG_VOID_RETURN;
    }


	/* If we started an range operation, and didn't finish it, this
	 * will cancel it if we need to.
	 */
	cancel_range();

    /* Was a file or a directory clicked? */
    if (FlagIsSet (te -> Flags, ENTRY_IS_DIR)) {
	/* A directory */
	/* Check for double click */
	if ((entry == p_entry) && (DoubleClick (prev_secs, prev_micros,
			event_secs, event_micros) == TRUE)) {
	    /* A Double-Click */

	    /* Force selected state */
	    SetFlag (te -> Flags, ENTRY_SELECTED);
	    un_shadow ((struct TreeNode *) (te -> Size));

	    /* Change into the sub directory if possible */
	    if (FlagIsSet (te -> Flags, ENTRY_SELECTED)) {
		/* Descend only if the directory is selected */
		descend (te);
		SetWaitPointer (mainwin);
		format_entrys (current_node);
		update_slist (1);
		stats ();
		ClearWaitPointer (mainwin);
	    }
	} else {
	    /* Normal Click */
		toggle_dir (entry);
		stats ();
	}
    } else {
	/* a file */
	/* Check for double click */
	if ((entry == p_entry) && (DoubleClick (prev_secs, prev_micros,
		event_secs, event_micros) == TRUE) && (mode == BACKUP_MODE)) {
	    /* A Double-Click */
	    strcpy (buf, current_path);
	    add_path (buf, te -> Name);
	    do_fdata_req (buf);
	}
	/* A single click */
	toggle_file (entry);
	stats ();
    }
    /* Save the entry data for double-click purposes */
    p_entry = entry;
    DBUG_VOID_RETURN;
}




/* DTM_NEW */

/* This will cancel a range operation which has been initiated by
 * a starting shift-click and interupted by some action other than
 * a subsequent shift-click.
 */

static void cancel_range()
{
	if( range_anchor_entry != -1 )
	{
		/* A shift-click was done to start marking a range, but the next
		 * click wasn't a shift-click.  Cancel the range operation.
		 */
	    range_anchor_entry = -1;
		ClearRangePointer( mainwin );
	}
}



/* DTM_NEW */

static void toggle_range (entry1, entry2)
int entry1, entry2;
{
	int first, last, i;
	struct TreeEntry *te;


	SetWaitPointer (mainwin);

    first = min ( entry1, entry2 );
    last = max ( entry1, entry2 );

    for ( i=first; i<=last; i++ )
    {
	te = find_entry (current_node, i);

	if (FlagIsSet (te -> Flags, ENTRY_IS_DIR))
	{
		toggle_dir (i);
	}
	else
	{
		toggle_file (i);
	}
    }

    stats ();

	ClearWaitPointer (mainwin);
}



/* DTM_NEW */

static void toggle_file (entry)
int entry;
{
	struct TreeEntry *te;


	te = find_entry (current_node, entry);

	if ((mode == BACKUP_MODE) ||
		FlagIsSet (te -> Flags, ENTRY_BACKEDUP)) {
	    /* We are either in backup mode, or, if in restore mode
	     * the file is actually available for restoration.
	     */
	    /* Toggle it's selected state */
	    ToggleFlag (te -> Flags, ENTRY_SELECTED);
	    entry_color[entry] = FlagIsSet (te -> Flags, ENTRY_SELECTED)
		? sel_file_color : desel_file_color;
	    update_slist (0);
	}
}


/* DTM_NEW */

static void toggle_dir (entry)
int entry;
{
	struct TreeEntry *te;


	te = find_entry (current_node, entry);

	/* Toggle directory selection state */
	ToggleFlag (te -> Flags, ENTRY_SELECTED);
	entry_color[entry] = FlagIsSet (te -> Flags, ENTRY_SELECTED)
		? sel_file_color : desel_file_color;
	update_slist (0);
	if (FlagIsSet (te -> Flags, ENTRY_SELECTED)) {
		un_shadow ((struct TreeNode *) (te -> Size));
	} else {
		shadow ((struct TreeNode *) (te -> Size));
	}
}


void format_entrys (tn)
struct TreeNode *tn;
{
    int i;
    struct TreeEntry *te;
    char astat;
    char buf[10];
    LONG day;
    LONG sec;
    LONG tick;

    DBUG_ENTER ("format_entrys");
    buf[9] = '\0';
    DBUG_PRINT ("path", ("TreeNode %lx Parent %lx Entrys %d MemSize %ld",
		tn, tn -> Parent, tn -> Entries, tn -> Size));
    /* Display the current path */
    disp_path (current_path);
    for (i = 0; i < tn -> Entries; i++) {
	te = find_entry (tn, i);
	if (FlagIsSet (te -> Flags, ENTRY_IS_DIR)) {
	    if (te -> Size == 0) {
		/* An empty directory */
		sprintf ((char *) & entry_string[i],
			"%-32.32s  (empty directory) ", te -> Name);
	    } else {
		/* Directory has some files in it */
		sprintf ((char *) & entry_string[i],
			"%-32.32s    (directory)     ", te -> Name);
	    }
	    entry_color[i] = FlagIsSet (te -> Flags, ENTRY_SELECTED)
		? sel_file_color : desel_file_color;
	} else {
	    astat = FlagIsSet (te -> Status, FIBF_ARCHIVE) ? 'A' : ' ';
	    secs_to_amigados (te -> Date, &day, &sec, &tick);
	    days_to_string (day, buf);
#if 0
	    DBUG_PRINT ("x", ("%s %ld %s", &te -> Name, te -> Date, buf));
#endif
	    sprintf ((char *) & entry_string[i], "%-32.32s %6ld  %6s %c",
		te -> Name, te -> Size, buf, astat);
	    if ((mode == RESTORE_MODE) &&
		    FlagIsClear (te -> Flags, ENTRY_BACKEDUP)) {
		entry_color[i] = unbacked_file_color;
	    } else {
		entry_color[i] = FlagIsSet (te -> Flags, ENTRY_SELECTED)
		    ? sel_file_color : desel_file_color;
	    }
	}
    }
    entry_count = tn -> Entries;
    DBUG_VOID_RETURN;
}

static void perform_inclusion (tn)
struct TreeNode *tn;
{
    DBUG_ENTER ("perform_inclusion");
    SetWaitPointer (mainwin);
    if (FlagIsClear (size_gadget[0].Flags, SELECTED) &&
	    FlagIsClear (date_gadget[0].Flags, SELECTED) &&
	    FlagIsClear (pattern_gadget[0].Flags, SELECTED) &&
	    FlagIsClear (arc_gadget[0].Flags, SELECTED) &&
	    FlagIsClear (size_gadget[0].Flags, SELECTED)) {
	/* No qualifiers, which implies ALL */
	select_all (tn);
    } else {
	do_include (tn);
    }
    format_entrys (tn);
    update_slist (1);
    stats ();
    ClearWaitPointer (mainwin);
    DBUG_VOID_RETURN;
}

void perform_exclusion (tn)
struct TreeNode *tn;
{
    DBUG_ENTER ("perform_exclusion");
    SetWaitPointer (mainwin);
    if (FlagIsClear (size_gadget[0].Flags, SELECTED) &&
	    FlagIsClear (date_gadget[0].Flags, SELECTED) &&
	    FlagIsClear (arc_gadget[0].Flags, SELECTED) &&
	    FlagIsClear (pattern_gadget[0].Flags, SELECTED) &&
	    FlagIsClear (size_gadget[0].Flags, SELECTED)) {
	/* No qualifiers, which implies ALL */
	select_none (tn);
    } else {
	do_exclude (tn);
    }
    format_entrys (tn);
    update_slist (1);
    stats ();
    ClearWaitPointer (mainwin);
}

void setmode (n)
int n;
{
    DBUG_ENTER ("setmode");
    switch (n) {
	case NO_MODE: 
	case BACKUP_COMPLETE: 
	    mode = NO_MODE;
	    strcpy (title_buf, VERS);
	    strcat (title_buf, "   Please select an operation from the Project Menu");
	    SetWindowTitles (mainwin, (screen ? 0L : title_buf), title_buf);
	    OnMenu (mainwin,
			(LONG) (SHIFTMENU (SORT_MENU) | SHIFTITEM (NOITEM)));
	    OnMenu (mainwin,
			(LONG) (SHIFTMENU (OPTIONS_MENU) | SHIFTITEM (NOITEM)));
	    break;

	case BACKUP_MODE: 
	    mode = BACKUP_MODE;
	    strcpy (title_buf, VERS);
	    strcat (title_buf, "      Backup mode");
	    SetWindowTitles (mainwin, (screen ? 0L : title_buf), title_buf);
	    OnMenu (mainwin,
			(LONG) (SHIFTMENU (SORT_MENU) | SHIFTITEM (NOITEM)));
	    OnMenu (mainwin,
			(LONG) (SHIFTMENU (OPTIONS_MENU) | SHIFTITEM (NOITEM)));
	    break;
	case RESTORE_MODE: 
	case RESTORE_COMPLETE: 
	    mode = RESTORE_MODE;
	    strcpy (title_buf, VERS);
	    strcat (title_buf, "      Restore mode");
	    SetWindowTitles (mainwin, (screen ? 0L : title_buf), title_buf);
	    OffMenu (mainwin,
			(LONG) (SHIFTMENU (SORT_MENU) | SHIFTITEM (NOITEM)));
	    OffMenu (mainwin,
			(LONG) (SHIFTMENU (OPTIONS_MENU) | SHIFTITEM (NOITEM)));
	    break;
	case BACKUP_INPROGRESS: 
	    mode = BACKUP_INPROGRESS;
	    strcpy (title_buf, VERS);
	    strcat (title_buf, "      Backup in Progress");
	    SetWindowTitles (mainwin, (screen ? 0L : title_buf), title_buf);
	    break;
	case RESTORE_INPROGRESS: 
	    mode = RESTORE_INPROGRESS;
	    strcpy (title_buf, VERS);
	    strcat (title_buf, "      Restore in Progress");
	    SetWindowTitles (mainwin, (screen ? 0L : title_buf), title_buf);
	    break;

	case DIFF_MODE:
	    mode = DIFF_MODE;
	    strcpy (title_buf, VERS);
	    strcat (title_buf, "      Difference mode");
	    SetWindowTitles (mainwin, (screen ? 0L : title_buf), title_buf);
	    OffMenu (mainwin,
			(LONG) (SHIFTMENU (SORT_MENU) | SHIFTITEM (NOITEM)));
	    OffMenu (mainwin,
			(LONG) (SHIFTMENU (OPTIONS_MENU) | SHIFTITEM (NOITEM)));
	    break;

	case DIFF_INPROGRESS: 
	    mode = DIFF_INPROGRESS;
	    strcpy (title_buf, VERS);
	    strcat (title_buf, "      Difference in Progress");
	    SetWindowTitles (mainwin, (screen ? 0L : title_buf), title_buf);
	    break;

	case INSPECT_MODE:
	    mode = INSPECT_MODE;
	    strcpy (title_buf, VERS);
	    strcat (title_buf, "      Inspect mode");
	    SetWindowTitles (mainwin, (screen ? 0L : title_buf), title_buf);
	    OffMenu (mainwin,
			(LONG) (SHIFTMENU (SORT_MENU) | SHIFTITEM (NOITEM)));
	    OffMenu (mainwin,
			(LONG) (SHIFTMENU (OPTIONS_MENU) | SHIFTITEM (NOITEM)));
	    break;

	case INSPECT_INPROGRESS: 
	    mode = INSPECT_INPROGRESS;
	    strcpy (title_buf, VERS);
	    strcat (title_buf, "      Inspect in Progress");
	    SetWindowTitles (mainwin, (screen ? 0L : title_buf), title_buf);
	    break;

    }
    DBUG_VOID_RETURN;
}



/* The "Debug About" Requester */


static char da_buf[10][80];

static struct IntuiText da_itext[] = {
    { 6, 3, JAM2, 30, 5, NULL,
	(UBYTE *) "Internal parameters:",
	&da_itext[1] },
    { 6, 1, JAM2, 30, 17, NULL,				/* 1 */
	(UBYTE *) &da_buf[0][0],
	&da_itext[2] },
    { 6, 1, JAM2, 30, 29, NULL,
	(UBYTE *) &da_buf[1][0],
	&da_itext[3] },
    { 6, 1, JAM2, 30, 45, NULL,
	(UBYTE *) &da_buf[2][0],
	&da_itext[4] },
    { 6, 1, JAM2, 30, 60, NULL,
	(UBYTE *) &da_buf[3][0],
	&da_itext[5] },
    { 6, 1, JAM2, 30, 70, NULL,
	(UBYTE *) &da_buf[4][0],
	&da_itext[6] },
    { 6, 1, JAM2, 30, 80, NULL,
	(UBYTE *) &da_buf[5][0],
	&da_itext[7] },
    { 6, 1, JAM2, 30, 90, NULL,
	(UBYTE *) &da_buf[6][0],
	&da_itext[8] },
    { 6, 1, JAM2, 30, 100, NULL,
	(UBYTE *) &da_buf[7][0],
	&da_itext[9] },
    { 6, 1, JAM2, 30, 115, NULL,
	(UBYTE *) &da_buf[8][0],
	&da_itext[10] },
    { 6, 1, JAM2, 30, 125, NULL,	/* 10 */
	(UBYTE *) &da_buf[9][0],
	NULL },
    { 6, 1, JAM2, 5, 3, NULL,
	(UBYTE *) "OK", NULL }
};

static void debug_about ( void )
{
	char path_buf[255];
	char *bfa;


    DBUG_ENTER ("debug_about");

	bfa = findbru();
	if( bfa == NULL )
	{
		bfa = "Not Found";
	}

	sprintf( &da_buf[0][0], "Bru Path is: \"%s\"", bru_path );
	sprintf( &da_buf[1][0], "Bru Name is: \"%s\"", bru_name );
	sprintf( &da_buf[2][0], "IPC Port is: \"%s\"", ipcport_name );
	sprintf( &da_buf[3][0], "Bru Args is: \"%s\"", bru_args );
	sprintf( &da_buf[4][0], "Bru Logdir is: \"%s\"", bru_logdir );

	getcd( 0, path_buf );
	sprintf( &da_buf[5][0], "Current Dir is: \"%s\"", path_buf );

	sprintf( &da_buf[6][0], "Bru Found as: \"%s\"", bfa );

#if 0
	sprintf( &da_buf[7][0], " is: \"%s\"", );
	sprintf( &da_buf[8][0], " is: \"%s\"", );
#endif

	sprintf( &da_buf[9][0], "Embed logfile: %d   Make Icons: %d",
		(int)embed_logfile, (int)makeicon_flag );

    AutoRequest (mainwin, &da_itext[0], &da_itext[11], NULL,
	    NULL, NULL, 500L, 170L);
    DBUG_VOID_RETURN;
}

