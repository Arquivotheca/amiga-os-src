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

#include <workbench/startup.h>

#include <rexx/storage.h>
#include <rexx/rxslib.h>

#include "bailout.h"
#include "brushell.h"
#include "dirtree.h"
#include "eventloop.h"
#include "mainwin.h"
#include "menus.h"
#include "reqs.h"
#include "rexxcom.h"
#include "slist.h"
#include "backup.h"
#include "filereq.h"
#include "dbug.h"
#include "scansort.h"
#include "logfile.h"
#include "libfuncs.h"

/* #define DON_TEST */

#define ARG_BUFSIZE			300


/*
 *	Prototypes for local functions, not exported.
 */
static BOOL backit ( struct TreeNode *tn );
static BOOL interface( void );
static void trim_files ( struct TreeNode *tn );
static BOOL _trim_files ( struct TreeNode *tn );
static ERRORCODE start_bru ( char *argstring );
static void cleanup_bru( void );
static char *xfindbru ( char *path, char *name );
static char *build_backup_args( void );
static BOOL restore_it ( struct TreeNode *tn );
static char *build_restore_args( void );

static BOOL diff_it ( struct TreeNode *tn );
static char *build_diff_args( void );

static BOOL inspect_it ( struct TreeNode *tn );
static char *build_inspect_args( void );

BOOL filename_sent;

/* This flag is set after BRU sends a "finished" message */
BOOL finished_flag;

/* This flag is set when all names in the tree have been passed to BRU */
BOOL tree_end_flag;

/* A place to keep the name of the next file to be backed up */
char current_file[MAX_CUR_PATH];

BOOL user_cancel_flag = FALSE;

BOOL need_to_embed_logfile = FALSE;



static void prepare( char *text )
{
	int ec;

	
	/* Clear the file display */
	ec = entry_count;
	entry_count = 0;
	update_slist( 1 );
	entry_count = ec;
	
    processed_count = 0;
    finished_flag = FALSE;
    tree_end_flag = FALSE;
	user_cancel_flag = FALSE;
	need_to_embed_logfile = FALSE;

    strcpy (current_path, root_name);

    post_cancel_req (mainwin, text);
}



static void clean_it( void )
{
    tree_end_flag = TRUE;

	cleanup_bru();

    clear_cancel_req ();

    to_root();

    if( current_node )
    {
		format_entrys (current_node);
		update_slist (1);
		/* stats(); */
	}
}



/*
 * Called by the menu selection "Backup".  This will build the
 * directory tree.
 */

void init_backup ( void )
{
	static struct IntuiText itext = {
		2, 0, JAM2, 310, 80, NULL, (UBYTE *) "Files Scanned:", NULL };


    DBUG_ENTER ("init_backup");

    if( auto_backup == FALSE )
	{
		/* If there is an existing tree, do we really want to do this? */
		if ((root_node != NULL) &&
			(ask_question ("Replace existing file list?") == FALSE))
		{
	    	/* Was probably an oops! by the user. */
		    DBUG_VOID_RETURN;
		}

		if (do_vol_req ("Enter Volume or Directory to Back Up:",
			    root_name) == FALSE)
		{
	    	/* user changed their mind? */
		    DBUG_VOID_RETURN;
		}

	    /* Clear the file display */
    	entry_count = 0;
	    update_slist (1);

		if (root_node != NULL)
		{
		    free_tree (root_node);
	    	root_node = NULL;
		}
    }
	else
	{
		/*
		 * Don't ask user for a starting directory, use what they defined
		 * on the command line or config file.
		 */
		auto_backup = FALSE;	/* but only the first time... */
    }

    stats ();
    PrintIText (mainwin -> RPort, &itext, 0L, 0L);

    SetWaitPointer (mainwin);

    root_node = create_tree (root_name);
    DBUG_PRINT ("rootnode", ("Rootnode is 0x%08lx", root_node));
    if (root_node != NULL)
	{
		setmode (BACKUP_MODE);
		to_root ();
		sort (root_node);
		format_entrys (current_node);
		update_slist (1);
		stats ();
    }
	else
	{
		/* Could not build the tree for some reason */
		setmode (NO_MODE);
    }

    ClearWaitPointer (mainwin);

    DBUG_VOID_RETURN;
}



/*
 * When the mode is set to backup, and a tree has been built, this
 * will be called by the "Start" gadget being clicked to actually
 * create a backup archive.
 */

void perform_backup ( void )
{
    BOOL rc;
    ERRORCODE ec;
	char *ap;


    DBUG_ENTER ("perform_backup");

	ap = build_backup_args();

    /* Start up BRU */
    ec = start_bru( ap );
    if( ec )
	{
		mention_error ("Backup aborted, unable to start BRU.", ec);
		DBUG_VOID_RETURN;
    }

    prepare( "Backup in progress" );
    
    setmode (BACKUP_INPROGRESS);

	if( embed_logfile == TRUE )
	{
		if(  save_embedded_logfile( root_node ) != 0  )
		{
			mention_error( "Warning: Unable to embed logfile in archive.",
				ERC_NONE | ERR52 );
		}
		else
		{
			need_to_embed_logfile = TRUE;
		}
	}

    rc = walk_nodes( root_node, backit, current_path );

	if( embed_logfile == TRUE )
	{
		DeleteFile( EMBED_LOGNAME );
	}

	clean_it();

    /* Now that bru has terminated, see how things went in general */
    if (rc == TRUE)
	{
		/* If true, then the backup was not aborted */

		if (FlagIsSet (options_item[AUTO_TRIM_ITEM].Flags, CHECKED))
		{
	    	/* Remove files which were not selected from the tree */
		    trim_files (root_node);
		}

		if( bru_logdir[0] != '\0' )
		{
			save_tree (root_node);
		}

		setmode (BACKUP_MODE);

		tell_user( "Backup complete" );
	}
	else
	{
		/* Backup was aborted */

		mention_error ("Backup aborted, log file NOT generated!",
			ERC_NONE | ERR23);
		setmode (BACKUP_MODE);
	}


	/* Reset all the flags which say a file is backed up */
	clear_backed (root_node);

    DBUG_VOID_RETURN;
}



/*
 * This function is called for every node in the tree to do a backup
 * of the contents as appropriate.
 *
 * Returns FALSE (which will halt the tree walk) if BRU sent a "finished"
 * message OR the user canceled.
 */

static BOOL backit ( struct TreeNode *tn )
{
    int i;
    struct TreeEntry *te;
    BOOL flag = TRUE;


    DBUG_ENTER ("backit");

    for (i = 0; i < tn -> Entries; i++)
	{
#if 0
		/* Check for user saying stop */
		if (check_cancel () == TRUE)
		{
		    DBUG_RETURN (FALSE);
		}
#endif

		te = find_entry (tn, i);

		if (FlagIsSet (te -> Flags, ENTRY_IS_DIR))
		{
		    /* A Directory */

		    if (FlagIsSet (te -> Flags, ENTRY_SELECTED) &&
			    FlagIsClear (te -> Flags, ENTRY_SHADOWED) &&
				( backup_the_dirs == TRUE )
				)
			{
				/* Back it up */
				add_path (current_path, te -> Name);
				strcpy (current_file, current_path + strlen (root_name));
				trim_leading (current_file, '/');
				SetFlag (te -> Flags, ENTRY_BACKEDUP);

				flag = interface();

				subtract_path (current_path);
		    }
		}
		else
		{
	    	/* A File */

		    if (FlagIsSet (te -> Flags, ENTRY_SELECTED) &&
			    FlagIsClear (te -> Flags, ENTRY_SHADOWED))
			{
				add_path (current_path, te -> Name);
				processed_count++;
				strcpy (current_file, current_path + strlen (root_name));
				trim_leading (current_file, '/');

				/* 
				 * Assume this anyway, unless BRU tells us
				 * differently later on.
				 */
				SetFlag( te->Flags, ENTRY_BACKEDUP );

				flag = interface();

				subtract_path( current_path );
		    }
		}

		if( flag == FALSE )
		{
			break;
		}
    }


    DBUG_RETURN( flag );
}



/*
 * This is called with a filename to be backed/restored.  This function
 * will retain control, checking the ipc port continously, until
 * the filename has been requested by BRU or a "finished" message
 * is recieved.  All other messages will be handled without this
 * function returning.
 *
 * Return is TRUE if the filename was passed on normally.
 *
 * Return is FALSE if we have received the "finished" message OR
 * if the user clicked the cancel gadget (and confirmed it).
 */

static BOOL interface( void )
{
	ULONG waitflags;


    DBUG_ENTER ("interface");

    filename_sent = FALSE;

    while( filename_sent == FALSE )
	{
		/* 
		 * We stay in this loop, waiting on the IPC port until a
		 * finished message is recieved or a filename is requested.
		 */

		waitflags = Wait( mainwin_sig_bit | ipc_sig_bit );

		/* First check if it was from the mainwindow Intuition port.
		 * We are using our knowledge of the fact that the cancel
		 * requester was posted in the main window.
		 */
		if( waitflags & mainwin_sig_bit )
		{
			if( check_cancel() == TRUE )
			{
				user_cancel_flag = TRUE;
			    DBUG_RETURN( FALSE );
			}
		}

		/* Any activity at the IPC port? */
		if( waitflags & ipc_sig_bit )
		{
			check_ipc_port();
		}

		if( filename_sent == TRUE )
		{
		    /* Display some info telling how things are progressing */
			disp_processed( current_path );
		}

		/* If BRU said it's finished, let's oblige */
    	if( finished_flag == TRUE )
		{
		    DBUG_RETURN( FALSE );
	    }
	}

    DBUG_RETURN( TRUE );
}



/*
 * Compact nodes.  If a file entry is not marked as backed up, remove
 * it from the node.
 */

static void trim_files ( struct TreeNode *tn )
{
    DBUG_ENTER ("trim_files");
    DBUG_PRINT ("trim", ("trimming files from tree"));
    walk_nodes (root_node, _trim_files, NULL);
    DBUG_PRINT ("trim", ("files trimmed"));
    DBUG_VOID_RETURN;
}

static BOOL _trim_files ( struct TreeNode *tn )
{
    int i;
    int j;
    struct TreeEntry *te;

    DBUG_ENTER ("_trim_files");
    for (i = 0; i < tn -> Entries; i++) {
	te = find_entry (tn, i);
	if (FlagIsSet (te -> Flags, ENTRY_IS_DIR)) {
	    /* A Directory */
	    /* Ignore it */
	} else {
	    /* A File */
	    if (FlagIsClear (te -> Flags, ENTRY_SELECTED) |
		    FlagIsSet (te -> Flags, ENTRY_SHADOWED)) {
		/* 
		 * It's either not selected or it's shadowed,
		 * so delete this entry
		 */
		if (i < tn -> Entries - 1) {
		    /* 
		     * Not last entry, so must move
		     * entrys down
		     */
		    for (j = i + 1; j < tn -> Entries; j++) {
			CopyMem ((char *) find_entry (tn, j),
				(char *) find_entry (tn, j - 1),
				(long) sizeof (struct TreeEntry));
		    }
		}
		tn -> Entries--;
		i--;
	    }
	}
    }
    DBUG_RETURN (TRUE);
}



void init_restore ( void )
{
    DBUG_ENTER ("init_restore");
    /* If there is an existing tree, do we really want to do this? */
    if ((root_node != NULL) &&
	    (ask_question ("Replace existing file list?") == FALSE)) {
	/* Was probably an oops! by the user. */
	DBUG_VOID_RETURN;
    }
    
    /* Clear the file display */
    entry_count = 0;
    update_slist (1);

    if (root_node) {
	free_tree (root_node);
	root_node = NULL;
    }
    
    stats ();
    load_tree ();
    DBUG_PRINT ("rootnode", ("load tree Rootnode is 0x%08lx", root_node));
    if (root_node != NULL) {
	setmode (RESTORE_MODE);
	to_root ();
/*	sort (root_node); */
	format_entrys (current_node);
	update_slist (1);
	stats ();
    } else {
	/* Could not load the tree log file for some reason */
	setmode (NO_MODE);
    }
    DBUG_VOID_RETURN;
}



void perform_restore ( void )
{
	char *ap;
    BOOL rc;
    ERRORCODE ec;
	char buf[256];
	

    DBUG_ENTER ("perform_restore");

    if (auto_restore == FALSE)
	{
		if (do_vol_req ("Volume or Directory to Restore to:",
		    root_name) == FALSE)
		{
	    	/* user changed their mind? */
		    DBUG_VOID_RETURN;
		}
    }

	if(  is_a_dir( root_name )  != 0  )
	{
		mention_error( "Sorry, that directory doesn't exist or is a file!",
			ERC_NONE | ERR65 );
		errno = 0;
	    DBUG_VOID_RETURN;
	}

    if (auto_restore == FALSE)
	{
		/* Warn about restore */
		sprintf( buf, "Warning, data on %s may be overwritten. Continue?",
			root_name );
		if(  ask_question( buf ) == 0  )
		{
			/* Quit */
			DBUG_VOID_RETURN;
		}
	}
	
	ap = build_restore_args();

    /* Start up BRU */
    ec = start_bru( ap );
    if( ec )
	{
		mention_error( "Restore aborted, unable to start BRU.", ec );
		DBUG_VOID_RETURN;
    }

    prepare( "Restore in progress" );
    
    setmode (RESTORE_INPROGRESS);

    rc = walk_nodes( root_node, restore_it, current_path );

#if 0
	if( embed_logfile == TRUE )
	{
		DeleteFile( EMBED_LOGNAME );
	}
#endif

	clean_it();

    /* Now that bru has terminated, see how things went in general */
    if (rc == TRUE)
	{
		/* If true, then the restore was not aborted */
		tell_user( "Restore complete" );
	}
	else
	{
		/* Restore was aborted */
		mention_error ("Restore aborted!", ERC_NONE | ERR56);
	}


	/* Reset all the flags which say a file is backed up */
	/* clear_backed (root_node); */

    setmode (RESTORE_MODE);

    DBUG_VOID_RETURN;
}



/*
 * This function is called for every node in the tree to do a restore
 * of the contents as appropriate.
 *
 * Returns FALSE (which will halt the tree walk) if BRU sent a "finished"
 * message OR the user canceled.
 */

static BOOL restore_it ( struct TreeNode *tn )
{
    int i;
    struct TreeEntry *te;
    BOOL flag = TRUE;


    DBUG_ENTER ("restore_it");

    for (i = 0; i < tn -> Entries; i++)
	{
		te = find_entry (tn, i);

		if (FlagIsSet (te -> Flags, ENTRY_IS_DIR))
		{
		    /* A Directory */

		    if (FlagIsSet (te -> Flags, ENTRY_SELECTED) &&
			    FlagIsClear (te -> Flags, ENTRY_SHADOWED) &&
			    FlagIsSet (te -> Flags, ENTRY_BACKEDUP))
			{
				/* Restore it */
				add_path (current_path, te -> Name);
				strcpy (current_file, current_path + strlen (root_name));
				trim_leading (current_file, '/');
				SetFlag (te -> Flags, ENTRY_RESTORED);

				flag = interface();

				subtract_path (current_path);
		    }
		}
		else
		{
	    	/* A File */

		    if (FlagIsSet (te -> Flags, ENTRY_SELECTED) &&
			    FlagIsClear (te -> Flags, ENTRY_SHADOWED) &&
			    FlagIsSet (te -> Flags, ENTRY_BACKEDUP))
			{
				add_path (current_path, te -> Name);
				processed_count++;
				strcpy (current_file, current_path + strlen (root_name));
				trim_leading (current_file, '/');

				/* 
				 * Assume this anyway, unless BRU tells us
				 * differently later on.
				 */
				SetFlag( te->Flags, ENTRY_RESTORED );

				flag = interface();

				subtract_path( current_path );
		    }
		}

		if( flag == FALSE )
		{
			break;
		}
    }


    DBUG_RETURN( flag );
}



static char *build_backup_args( void )
{
	static char buf[ARG_BUFSIZE];
	char *p;
    int i;


	strcpy( buf, " -c" );		/* Backup mode */


	/* Device arguments */

	for (i = 0; i < NUM_DEVS_ITEMS; i++) {
	    if (FlagIsSet (devs_item[i].Flags, CHECKED)) {
		p = (char *) ((((struct IntuiText *)
				(devs_item[i].ItemFill)) -> IText));
		if (strlen (p) > 0) {
		    strcat (buf, " -f");
		    strcat (buf, " ");
		    strcat (buf, p);
		    strcat (buf, " ");
		}
	    }
	}


	/* Generic BRU arguments and options */

	strcat (buf, " ");
	strcat (buf, bru_args);
	strcat (buf, " ");
	strcat( buf, BRU_OPTS " - " );


	/* Place root name as a comment in the archive */

	strcat (buf, " ");
	strcat (buf, "-L ");
	strcat (buf, root_name);


	/* Archive bits */

    if (FlagIsSet (options_item[SET_ARC_BITS_ITEM].Flags, CHECKED)) {
		strcat (buf, " -As");
	}


	/* Compression options */

    if (FlagIsSet (compression_item[COMPRESS_ALL_ITEM].Flags, CHECKED)) {
		strcat (buf, " -Z");
	}

	p = &buf[strlen(buf)];
    if (FlagIsSet (compression_item[COMPRESS_LARGER_ITEM].Flags, CHECKED)) {
		sprintf( p, " -S%ld", arch_level );
	}

	return( buf );
}


static char *build_restore_args( void )
{
	static char buf[ARG_BUFSIZE];
	char *p;
    int i;


	strcpy( buf, " -x" );		/* Extract mode */

	/* Device arguments */

	for (i = 0; i < NUM_DEVS_ITEMS; i++) {
	    if (FlagIsSet (devs_item[i].Flags, CHECKED)) {
		p = (char *) ((((struct IntuiText *)
				(devs_item[i].ItemFill)) -> IText));
		if (strlen (p) > 0) {
		    strcat (buf, " -f");
		    strcat (buf, " ");
		    strcat (buf, p);
		    strcat (buf, " ");
		}
	    }
	}


	/* Generic BRU arguments and options */

	strcat (buf, " ");
	strcat (buf, bru_args);
	strcat (buf, " ");
	strcat (buf, BRU_OPTS " - " );


#if 0
	/* Archive bits */

    if (FlagIsSet (options_item[SET_ARC_BITS_ITEM].Flags, CHECKED)) {
		strcat (buf, " -As");
	}
#endif

	return( buf );
}



/*
 * Starts the execution of "bru" as a child process.
 * Still not certain of the exact correct way to do this, so there are
 * are two different pieces of code here...
 *
 * We first look for bru in the current directory.  If found, we use
 * that one.  Otherwise, we look for bru in the path specified by the
 * configuration.  If not found, we check the c: directory as the
 * default.
 */

static ERRORCODE start_bru( char *argstring )
{
    char buf[ARG_BUFSIZE];	/* This should be enough for most anything */
    char *bname;
    LONG launchcode = 0;
    ERRORCODE rtnval = 0;
	BPTR fh;
/*	int i; */


    DBUG_ENTER ("start_bru");

    if(  ( bname = findbru() ) == NULL  )
	{
		DBUG_PRINT ("bru", ("can't find bru, give up"));
		DBUG_RETURN( ERC_NONE | ERR37 );
    }

    DBUG_PRINT ("bru", ("running bru from '%s'", bname));

    fh = Open ("NIL:", MODE_NEWFILE);
    if (fh != NULL) {
	sprintf (buf, "stack %d\n", bru_stack > 0 ? bru_stack : stacksize ());
	strcat (buf, "c:run >NIL: ");
	strcat (buf, bname);
	strcat (buf, " ");

	strcat (buf, argstring );
 
#ifdef DON_TEST
	printf( "BRU start string ->%s<-\n", buf );
#endif

	DBUG_PRINT ("bruargs", ("run '%s'", buf));
	launchcode = Execute (buf, NULL, fh);
	Delay (50L);
	Close (fh);
	DBUG_PRINT ("bruargs", ("Launch code = %ld", launchcode));
    }
    if (launchcode == 0) {
	rtnval = (ERC_NONE | ERR30);
    }


	if( rtnval == 0 )
	{
#if 0
		/* Wait 20 seconds to see if the bru task starts up */
		for( i=0; i<20; i++ )
		{
			if( FindTask( bru_name ) == NULL )
			{
				rtnval = ERC_NONE | ERR64;
			}
			else
			{
				/* It did */
				rtnval = 0;
				break;
			}

			Delay( 50 );
		}
#endif
	}


    DBUG_RETURN (rtnval);
}



/* DTM_NEW */

static void cleanup_bru( void )
{
	if( user_cancel_flag == TRUE )
	{
		if( bru_tcb )
		{
			/* Send BRU the control-C kill signal */
			Signal( bru_tcb, SIGBREAKF_CTRL_C );
			bru_tcb = NULL;		/* Make sure we never resuse stale
								 * pointer.
								 */
		}
		
		post_canceling( mainwin );
	}

    /* 
     * Now we wait for BRU to say it's finished.  Which it _has_ to.
     * If it does not, then it died and we probably want to hang also.
     */
    while( finished_flag == FALSE )
	{
		DBUG_PRINT ("fini", ("Waiting for finished msg from bru"));
		interface();
    }
	
    DBUG_PRINT ("fini", ("got finished msg from bru"));

	if( user_cancel_flag == TRUE )
	{
		clear_canceling();
	}
}


char *findbru( void )
{
	char *bname;


	/* Current directory */

    if ((bname = xfindbru ("", bru_name)) != NULL)
	{
		DBUG_RETURN( bname );
	}

	/* Then the given path */

	if ((bname = xfindbru (bru_path, bru_name)) != NULL)
	{
		DBUG_RETURN( bname );
	}

	/* Then in sys:tools */

    if ((bname = xfindbru ("sys:tools", bru_name)) != NULL)
	{
		DBUG_RETURN( bname );
	}

	/* Then in C: */

    if ((bname = xfindbru ("c:", bru_name)) != NULL)
	{
		DBUG_RETURN( bname );
	}

	/* Give up */

	DBUG_RETURN( NULL );
}



/*
 *	Given pointers to a path and the name of the bru executable,
 *	build a fully qualified pathname and check to see if bru
 *	can be found there.  If so, return a pointer to the fully
 *	qualified pathname.  Otherwise, return NULL.
 *
 *	Note that the pathname is stored in a statically allocated
 *	buffer which is overwritten on each call.
 *
 */

static char *xfindbru( char *path, char *name )
{
    register char *lastchar;
    static char fullname[256];


    DBUG_ENTER ("xfindbru");

	strcpy( fullname, path );

    if( *fullname != '\0' )
	{
		lastchar = &fullname[ strlen(fullname)-1 ];

		if( *lastchar != ':' && *lastchar != '/' )
		{
	    	*++lastchar = '/';
		    *++lastchar = '\000';
		}

		DBUG_PRINT ("path", ("look in directory '%s'", fullname));
    }

    strcat( fullname, name );

    DBUG_PRINT ("bru", ("look for bru in '%s'", fullname));
#ifdef DON_TEST
    printf( "look for bru in '%s'\n", fullname );
#endif

    if(  access( fullname, 4 ) == 0  )
	{
		DBUG_PRINT ("bru", ("found it"));
#ifdef DON_TEST
		printf( "found it\n" );
#endif
		name = fullname;
    }
	else
	{
		DBUG_PRINT ("bru", ("didn't find it"));
#ifdef DON_TEST
		printf( "didn't find it\n" );
#endif
		name = NULL;
    }

    DBUG_RETURN( name );
}



/*******************************************************************/
/************		Diff Functions							********/
/*******************************************************************/

void init_diff ( void )
{
    DBUG_ENTER ("init_diff");
    /* If there is an existing tree, do we really want to do this? */
    if ((root_node != NULL) &&
	    (ask_question ("Replace existing file list?") == FALSE)) {
	/* Was probably an oops! by the user. */
	DBUG_VOID_RETURN;
    }

    /* Clear the file display */
    entry_count = 0;
    update_slist (1);

    if (root_node) {
	free_tree (root_node);
	root_node = NULL;
    }
    stats ();
    load_tree ();
    DBUG_PRINT ("rootnode", ("load tree Rootnode is 0x%08lx", root_node));
    if (root_node != NULL) {
	setmode (DIFF_MODE);
	to_root ();
	format_entrys (current_node);
	update_slist (1);
	stats ();
    } else {
	/* Could not load the tree log file for some reason */
	setmode (NO_MODE);
    }
    DBUG_VOID_RETURN;
}



void perform_diff ( void )
{
	char *ap;
    BOOL rc;
    ERRORCODE ec;


    DBUG_ENTER ("perform_diff");

	if (do_vol_req ("Volume or Directory to Diff:",
	    root_name) == FALSE)
	{
    	/* user changed their mind? */
	    DBUG_VOID_RETURN;
	}

	ap = build_diff_args();

    /* Start up BRU */
    ec = start_bru( ap );
    if( ec )
	{
		mention_error( "Diference aborted, unable to start BRU.", ec );
		DBUG_VOID_RETURN;
    }

    prepare( "Difference in progress" );
    
    setmode (DIFF_INPROGRESS);

    rc = walk_nodes( root_node, diff_it, current_path );

	clean_it();

    /* Now that bru has terminated, see how things went in general */
    if (rc == TRUE)
	{
		/* If true, then the diff was not aborted */
		tell_user( "Difference complete" );
	}
	else
	{
		/* Difference was aborted */
		mention_error ("Difference aborted!", ERC_NONE | ERR56);
	}

    setmode (DIFF_MODE);

    DBUG_VOID_RETURN;
}



/*
 * This function is called for every node in the tree to do a diff
 * of the contents as appropriate.
 *
 * Returns FALSE (which will halt the tree walk) if BRU sent a "finished"
 * message OR the user canceled.
 */

static BOOL diff_it ( struct TreeNode *tn )
{
    int i;
    struct TreeEntry *te;
    BOOL flag = TRUE;


    DBUG_ENTER ("diff_it");

    for (i = 0; i < tn -> Entries; i++)
	{
		te = find_entry (tn, i);

		if (FlagIsSet (te -> Flags, ENTRY_IS_DIR))
		{
		    /* A Directory */

		    if (FlagIsSet (te -> Flags, ENTRY_SELECTED) &&
			    FlagIsClear (te -> Flags, ENTRY_SHADOWED) &&
			    FlagIsSet (te -> Flags, ENTRY_BACKEDUP))
			{
				/* Diff it */
				add_path (current_path, te -> Name);
				strcpy (current_file, current_path + strlen (root_name));
				trim_leading (current_file, '/');
				/* SetFlag (te -> Flags, ENTRY_RESTORED); */

				flag = interface();

				subtract_path (current_path);
		    }
		}
		else
		{
	    	/* A File */

		    if (FlagIsSet (te -> Flags, ENTRY_SELECTED) &&
			    FlagIsClear (te -> Flags, ENTRY_SHADOWED) &&
			    FlagIsSet (te -> Flags, ENTRY_BACKEDUP))
			{
				add_path (current_path, te -> Name);
				processed_count++;
				strcpy (current_file, current_path + strlen (root_name));
				trim_leading (current_file, '/');

				/* 
				 * Assume this anyway, unless BRU tells us
				 * differently later on.
				 */

				/* SetFlag( te->Flags, ENTRY_RESTORED ); */

				flag = interface();

				subtract_path( current_path );
		    }
		}

		if( flag == FALSE )
		{
			break;
		}
    }


    DBUG_RETURN( flag );
}



static char *build_diff_args( void )
{
	static char buf[ARG_BUFSIZE];
	char *p;
    int i;


	strcpy( buf, " -d" );


	/* Device arguments */

	for (i = 0; i < NUM_DEVS_ITEMS; i++) {
	    if (FlagIsSet (devs_item[i].Flags, CHECKED)) {
		p = (char *) ((((struct IntuiText *)
				(devs_item[i].ItemFill)) -> IText));
		if (strlen (p) > 0) {
		    strcat (buf, " -f");
		    strcat (buf, p);
		}
	    }
	}


	/* Generic BRU arguments and options */

	strcat (buf, " ");
	strcat (buf, bru_args);
	strcat (buf, " ");
	strcat (buf, BRU_OPTS " - " );


#if 0
	/* Archive bits */

    if (FlagIsSet (options_item[SET_ARC_BITS_ITEM].Flags, CHECKED)) {
		strcat (buf, " -As");
	}
#endif

	return( buf );
}




/*******************************************************************/
/************		Inspect Functions						********/
/*******************************************************************/

void init_inspect ( void )
{
    DBUG_ENTER ("init_inspect");
    /* If there is an existing tree, do we really want to do this? */
    if ((root_node != NULL) &&
	    (ask_question ("Replace existing file list?") == FALSE)) {
	/* Was probably an oops! by the user. */
	DBUG_VOID_RETURN;
    }

    /* Clear the file display */
    entry_count = 0;
    update_slist (1);

    if (root_node) {
	free_tree (root_node);
	root_node = NULL;
    }
    stats ();
    load_tree ();
    DBUG_PRINT ("rootnode", ("load tree Rootnode is 0x%08lx", root_node));
    if (root_node != NULL) {
	setmode (INSPECT_MODE);
	to_root ();
	format_entrys (current_node);
	update_slist (1);
	stats ();
    } else {
	/* Could not load the tree log file for some reason */
	setmode (NO_MODE);
    }
    DBUG_VOID_RETURN;
}



void perform_inspect ( void )
{
	char *ap;
    BOOL rc;
    ERRORCODE ec;


    DBUG_ENTER ("perform_inspect");

	if (do_vol_req ("Volume or Directory to Inspect:",
	    root_name) == FALSE)
	{
    	/* user changed their mind? */
	    DBUG_VOID_RETURN;
	}

	ap = build_inspect_args();

    /* Start up BRU */
    ec = start_bru( ap );
    if( ec )
	{
		mention_error( "Inspect aborted, unable to start BRU.", ec );
		DBUG_VOID_RETURN;
    }

    prepare( "Inspect in progress" );
    setmode (INSPECT_INPROGRESS);

    rc = walk_nodes( root_node, inspect_it, current_path );

	clean_it();

    /* Now that bru has terminated, see how things went in general */
    if (rc == TRUE)
	{
		/* If true, then the inspect was not aborted */
		tell_user( "Inspect complete" );
	}
	else
	{
		/* Inspect was aborted */
		mention_error ("Inspect aborted!", ERC_NONE | ERR56);
	}

    setmode (INSPECT_MODE);

    DBUG_VOID_RETURN;
}


/*
 * This function is called for every node in the tree to do a inspect
 * of the contents as appropriate.
 *
 * Returns FALSE (which will halt the tree walk) if BRU sent a "finished"
 * message OR the user canceled.
 */

static BOOL inspect_it ( struct TreeNode *tn )
{
    int i;
    struct TreeEntry *te;
    BOOL flag = TRUE;


    DBUG_ENTER ("inspect_it");

    for (i = 0; i < tn -> Entries; i++)
	{
		te = find_entry (tn, i);

		if (FlagIsSet (te -> Flags, ENTRY_IS_DIR))
		{
		    /* A Directory */

		    if (FlagIsSet (te -> Flags, ENTRY_SELECTED) &&
			    FlagIsClear (te -> Flags, ENTRY_SHADOWED) &&
			    FlagIsSet (te -> Flags, ENTRY_BACKEDUP))
			{
				/* Inspect it */
				add_path (current_path, te -> Name);
				strcpy (current_file, current_path + strlen (root_name));
				trim_leading (current_file, '/');
				/* SetFlag (te -> Flags, ENTRY_RESTORED); */

				flag = interface();

				subtract_path (current_path);
		    }
		}
		else
		{
	    	/* A File */

		    if (FlagIsSet (te -> Flags, ENTRY_SELECTED) &&
			    FlagIsClear (te -> Flags, ENTRY_SHADOWED) &&
			    FlagIsSet (te -> Flags, ENTRY_BACKEDUP))
			{
				add_path (current_path, te -> Name);
				processed_count++;
				strcpy (current_file, current_path + strlen (root_name));
				trim_leading (current_file, '/');

				/* 
				 * Assume this anyway, unless BRU tells us
				 * differently later on.
				 */

				/* SetFlag( te->Flags, ENTRY_RESTORED ); */

				flag = interface();

				subtract_path( current_path );
		    }
		}

		if( flag == FALSE )
		{
			break;
		}
    }


    DBUG_RETURN( flag );
}



static char *build_inspect_args( void )
{
	static char buf[ARG_BUFSIZE];
	char *p;
    int i;


	strcpy( buf, " -i" );


	/* Device arguments */

	for (i = 0; i < NUM_DEVS_ITEMS; i++) {
	    if (FlagIsSet (devs_item[i].Flags, CHECKED)) {
		p = (char *) ((((struct IntuiText *)
				(devs_item[i].ItemFill)) -> IText));
		if (strlen (p) > 0) {
		    strcat (buf, " -f");
		    strcat (buf, p);
		}
	    }
	}


	/* Generic BRU arguments and options */

	strcat (buf, " ");
	strcat (buf, bru_args);
	strcat (buf, " ");
	strcat (buf, BRU_OPTS " - " );


#if 0
	/* Archive bits */

    if (FlagIsSet (options_item[SET_ARC_BITS_ITEM].Flags, CHECKED)) {
		strcat (buf, " -As");
	}
#endif

	return( buf );
}



static char *build_recover_args( void )
{
	static char buf[ARG_BUFSIZE];
	char *p;
    int i;


	strcpy( buf, " -x" );		/* Extract mode */

	/* Device arguments */

	for (i = 0; i < NUM_DEVS_ITEMS; i++) {
	    if (FlagIsSet (devs_item[i].Flags, CHECKED)) {
		p = (char *) ((((struct IntuiText *)
				(devs_item[i].ItemFill)) -> IText));
		if (strlen (p) > 0) {
		    strcat (buf, " -f");
		    strcat (buf, p);
		}
	    }
	}


	/* Generic BRU arguments and options */

	strcat (buf, " ");
	strcat (buf, bru_args);
	strcat (buf, " ");
	strcat (buf, BRU_OPTS );

	printf( "Restore cmd to BRU: '%s'\n", buf );
	return( buf );
}



/* Things are getting rather kludge at this point.  This file should
 * probably be split into several seperate files.  Will do this
 * when diff and inspect are implemented?
 */
 
void recover( void )
{
	char *ap;
    BOOL rc;
    ERRORCODE ec;
	char buf[256];
	ULONG waitflags;
	

    DBUG_ENTER ("recover");

	if (do_vol_req ("Volume or Directory to Restore to:",
	    root_name) == FALSE)
	{
    	/* user changed their mind? */
	    DBUG_VOID_RETURN;
    }

	if(  is_a_dir( root_name )  != 0  )
	{
		mention_error( "Sorry, that directory doesn't exist or is a file!",
			ERC_NONE | ERR72 );
		errno = 0;
	    DBUG_VOID_RETURN;
	}

	/* Warn about restore */
	sprintf( buf, "Warning, data on %s may be overwritten. Continue?",
		root_name );
	if(  ask_question( buf ) == 0  )
	{
		/* Quit */
		DBUG_VOID_RETURN;
	}
	
	ap = build_recover_args();

    /* Start up BRU */
    ec = start_bru( ap );
    if( ec )
	{
		mention_error( "No-log Restore aborted, unable to start BRU.",
			ec );
		DBUG_VOID_RETURN;
    }

    prepare( "No-log Restore in progress" );
    
    setmode (RESTORE_INPROGRESS);

    for(;;)
	{
		/* 
		 * We stay in this loop, waiting on the IPC port until a
		 * finished message is recieved.
		 */

		waitflags = Wait( mainwin_sig_bit | ipc_sig_bit );

		/* First check if it was from the mainwindow Intuition port.
		 * We are using our knowledge of the fact that the cancel
		 * requester was posted in the main window.
		 */
		if( waitflags & mainwin_sig_bit )
		{
			if( check_cancel() == TRUE )
			{
				user_cancel_flag = TRUE;
			    break;
			}
		}

		/* Any activity at the IPC port? */
		if( waitflags & ipc_sig_bit )
		{
			check_ipc_port();
		}
#if 0
		if( filename_sent == TRUE )
		{
		    /* Display some info telling how things are progressing */
			disp_processed( current_path );
		}
#endif
		/* If BRU said it's finished, let's oblige */
    	if( finished_flag == TRUE )
		{
		    break;
	    }
	}

	clean_it();

    /* Now that bru has terminated, see how things went in general */
    if (user_cancel_flag == FALSE )
	{
		/* If false, then the restore was not aborted */
		tell_user( "No-log Restore complete" );
	}
	else
	{
		/* Restore was aborted */
		mention_error ("No-log Restore aborted!", ERC_NONE | ERR73);
	}

	/* Reset all the flags which say a file is backed up */
	/* clear_backed (root_node); */

    setmode (NO_MODE);

    DBUG_VOID_RETURN;
}


