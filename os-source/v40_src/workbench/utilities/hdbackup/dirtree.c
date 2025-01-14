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
#include "dirtree.h"
#include "eventloop.h"
#include "mainwin.h"
#include "scansort.h"
#include "dbug.h"
#include "reqs.h"
#include "libfuncs.h"



#define FD_WIDTH	70



/*
 *	Exported variables.
 */

char root_name[MAX_CUR_PATH] = ROOTNAME;  /* probably could be smaller... */
struct TreeNode *root_node = NULL;
char current_path[MAX_CUR_PATH];
struct TreeNode *current_node = NULL;
char (*entry_string)[1][FD_WIDTH] = NULL;
UBYTE *entry_color = NULL;

/*
 *	Local functions.
 */

static ERRORCODE read_directory PROTO((BPTR));
static ERRORCODE read_dir PROTO((BPTR));
static struct TreeNode *process_dir PROTO((BPTR, struct TreeNode *));
static struct ScratchEntry *find_scratch_entry PROTO((int));
static ERRORCODE expand_buffer PROTO((void));
static void scratch_to_node PROTO((struct TreeNode *, int));
static void free_node PROTO((struct TreeNode *));
static struct TreeNode *create_node PROTO((struct TreeNode *));
static BOOL _walk_files PROTO((struct TreeNode *));
static BOOL _walk_nodes PROTO((struct TreeNode *));
static BOOL _walk_entrys PROTO((struct TreeNode *));
static BOOL _find_filename PROTO((struct TreeEntry *));

/*
 * number of entrys actually read into the scratch
 * buffer by read_directory()
 */

static int scratch_entrys;
static int max_scratch_entrys = SCRATCH_START;

/*
 * Pointer to the scratch buffer.  This is an area of memory
 * which will hold entrys as they are read from the disk directory.
 * This area can expand if neccessary in a dynamic fashion.
 */

static struct ScratchEntry *scratchbuf = NULL;
static ERRORCODE tree_error;
static int expansions = 0;
static int rfc;
static char ff_buf[MAX_CUR_PATH];
static char ff_buf2[MAX_CUR_PATH];
static char *ff_name;
static struct TreeEntry *ff_te;

/*
 * The pointer to the function to be called-back by walk_files()
 * is stored here.
 */

static BOOL (*wf_func) PROTO((struct TreeEntry *));

/*
 * The pointer to the function to be called-back by walk_entrys()
 * is stored here.
 */

static BOOL (*we_func) PROTO((struct TreeEntry *));
static char *we_buf;

/*
 * The pointer to the function to be called-back by walk_nodes()
 * is stored here.
 */

static BOOL (*wn_func) PROTO((struct TreeNode *));
static char *wn_buf;

/*..............................................................*/
/*								*/
/*		Movement in our directory tree			*/
/*..............................................................*/

/*
 * When passed the address of a tree entry which is a directory, this
 * will return a pointer to the treenode containing the contents of
 * the passed directory.  If an entry which is not a dir is passed,
 * or one which is empty, no action is taken.
 * Updates the contents of the current_path string.
 */

void descend (te)
struct TreeEntry *te;
{
    DBUG_ENTER ("descend");
    if (FlagIsSet (te -> Flags, ENTRY_IS_DIR) && (te -> Size != NULL)) {
	/* This is a non-empty directory entry */
	current_node = (struct TreeNode *) (te -> Size);
	add_path (current_path, te -> Name);
    }
    DBUG_VOID_RETURN;
}

/*
 * Return a pointer to the parent treenode of the given treenode.
 * Updates the contents of the current_path string.
 */

void ascend ()
{
    struct TreeNode *tn;

    DBUG_ENTER ("ascend");
    tn = current_node -> Parent;
    if (tn == NULL) {
	/* We are in the root, cannot ascend */
	DBUG_VOID_RETURN;
    }
    /* Change to the parent */
    current_node = tn;
    subtract_path (current_path);
    DBUG_VOID_RETURN;
}

/*
 * Changes the current node to the root of the tree.
 * Updates the contents of the current_path string.
 */

void to_root ()
{
    DBUG_ENTER ("to_root");
    current_node = root_node;
    strcpy (current_path, root_name);
    DBUG_VOID_RETURN;
}

/*..............................................................*/
/*								*/
/*		Creation of our directory tree			*/
/*..............................................................*/


/*
 * This will build an entire directory tree in memory, returning a pointer
 * to the root node.  A null pointer is returned upon failure.  Errors
 * which happen during the execution of this routine will be reported
 * directly to the user via mention_error().
 * Also allocates the buffer areas for storing the formated entry strings.
 */

struct TreeNode *create_tree (dirname)
char *dirname;
{
    BPTR lock;
    struct TreeNode *tn;
	char buf[128];
	ULONG stackneeded;

	
    DBUG_ENTER ("create_tree");
    tree_error = 0;
    rfc = 0;
    if(  init_criteria () != 0  ) {
	mention_error ("Invalid date criteria", ERC_NONE | ERR40);
	DBUG_RETURN ((struct TreeNode *) NULL);
	}

    /* Set the path name buffer to the root name as given by user. */
    strcpy (current_path, dirname);

    /* First, we need to allocate the scratch buffer */
    scratchbuf = (struct ScratchEntry *)
	RemAlloc ((ULONG) (sizeof (struct ScratchEntry) * max_scratch_entrys),
                   MEMF_PUBLIC);
    if (scratchbuf == NULL)
	{
		mention_error ("Unable to Create Tree", ERC_NO_MEMORY | ERR05);
		DBUG_RETURN ((struct TreeNode *) NULL);
    }

    lock = Lock (dirname, ACCESS_READ);
    if( lock == NULL )
	{
		mention_error ("Unable to find that volume/directory",
			ERC_IO_FAILED | ERR06);
		RemFree( scratchbuf );
		UnLock (lock);
		DBUG_RETURN ((struct TreeNode *) NULL);
    }

	if(  is_a_dir( dirname )  != 0  )
	{
		mention_error( "Sorry, that appears to be a file!",
			ERC_IO_FAILED | ERR66 );
		RemFree (scratchbuf);
		UnLock (lock);
		DBUG_RETURN ((struct TreeNode *) NULL);
	}


    post_cancel_req (mainwin, "Reading Disk Directories");
    tn = process_dir (lock, 0L);
    clear_cancel_req ();
    
    if( tree_error != 0 )
	{
		mention_error ("Unable to build directory tree", tree_error);
		free_tree (tn);
		tn = NULL;
    }
	else
	{
		/* No error building tree */
		if( tn == NULL )
		{
	    	/* Tree is empty */
	    	mention_error ("Sorry, that directory appears empty!",
		    	ERC_NONE | ERR13);
		}
		else
		{
	    	/* A valid tree */
	    	node_status (tn);

	    	/* Is the stack adequate for the qsort() to run okay? */
	    	stackneeded = ( most_entrys * 75 ) + 4000;
#if 0
			sprintf( buf, "Stack is %d, needed is %d", stacksize(),
				stackneeded );
			tell_user( buf );
#endif
			if( stacksize() < stackneeded )
			{
				free_tree (tn);
				tn = NULL;
				sprintf( buf, "Sorry, stack of %d needed for directory \
with %d entries",
					( ( stackneeded / 4 ) + 1 ) * 4, most_entrys );
		    	mention_error( buf, ERC_NONE | ERR74 );
			}

	    	/* Allocate buffer area for entry display formating */
	    	if ((entry_string = (char (*)[1][FD_WIDTH])
				RemAlloc ((ULONG) (FD_WIDTH * most_entrys * sizeof (char)),
				MEMF_CLEAR)) == NULL)
			{
				free_tree (tn);
				tn = NULL;
				mention_error ("Not enough memory", ERC_NO_MEMORY | ERR19);
	    	}

		    if ((entry_color = (UBYTE *)
				RemAlloc ((ULONG) (most_entrys * sizeof (BOOL)),
				MEMF_CLEAR)) == NULL)
			{
				free_tree (tn);
				tn = NULL;
				mention_error ("Not enough memory", ERC_NO_MEMORY | ERR20);
	    	}
		}
    }
    
    RemFree( scratchbuf );

    UnLock (lock);

    DBUG_PRINT ("entries", ("most entries encountered %d", most_entrys));

    DBUG_RETURN (tn);
}

/*
 * This is the main recursive function which is the heart of this
 * attempt to build a directory tree in memory.  If an error occurs
 * in the this routine or a sub-routine an errorcode (in standard format)
 * will be left in the global variable "tree_error".
 *  Note that a null return is legal from this function, signifying
 * the directory to be processed was empty and that no node was created.
 */

static struct TreeNode *process_dir (lock, parent)
BPTR lock;
struct TreeNode *parent;
{
    int i;
    int rc;
    struct TreeEntry *te;
    struct TreeNode *tn;
    BPTR newlock;
    BPTR old_dir;

    DBUG_ENTER ("process_dir");

    /* get directory entrys */
    rc = read_directory( lock );
    if (rc)
	{
		/* Something went wrong */
		tree_error = rc;
		DBUG_RETURN ((struct TreeNode *) NULL);
    }
    
    /* Test for empty directory */
    if( scratch_entrys == 0 )
	{
		/* Empty, do not create a node for it's contents.  An empty
		 * directory should be indicated by the Size = 0.
		 */
		DBUG_RETURN ((struct TreeNode *) NULL);
    }
    
    /* make a node containing them */
    tn = create_node (parent);
    if( tn == NULL )
	{
		tree_error = ERC_NO_MEMORY | ERR06;
		DBUG_RETURN ((struct TreeNode *) NULL);
    }
    
    /* process the node to create sub-nodes for any dirs */
    for( i = 0; i < tn -> Entries; i++ )
	{
		te = find_entry (tn, i);
		if (FlagIsSet (te -> Flags, ENTRY_IS_DIR))
		{
		    /* We have a directory!  Process it! */
		    old_dir = CurrentDir (lock);

	    	newlock = Lock (te -> Name, ACCESS_READ);
		    if( newlock == NULL )
			{
				/* could not lock the sub-dir */
				(void) CurrentDir (old_dir);
				tree_error = ERC_IO_FAILED | ERR07;
				DBUG_RETURN (tn);
	    	}

		    add_path (current_path, te -> Name);

		    /* Recurse!!! */
		    te -> Size = (ULONG) process_dir (newlock, tn);
	    	subtract_path (current_path);
	    	UnLock (newlock);
	    	CurrentDir (old_dir);

		    if( tree_error != 0 )
			{
				DBUG_RETURN (tn);
		    }
		}
    }
    
    DBUG_RETURN (tn);
}



/*
 * Create a treenode containing the entrys already read into
 * the buffer area.
 */

static struct TreeNode *create_node (parent)
struct TreeNode *parent;
{
    ULONG size;
    struct TreeNode *tn;
    int i;

    DBUG_ENTER ("create_node");

    size = sizeof (struct TreeNode) +
	   ((scratch_entrys - 1) * sizeof (struct TreeEntry));

    tn = (struct TreeNode *) AllocMem (size, MEMF_PUBLIC);
    if( tn == NULL )
	{
		DBUG_RETURN ((struct TreeNode *) NULL);
    }
    
    tn -> Parent = parent;
    tn -> Size = size;
    tn -> Entries = scratch_entrys;
    for( i = 0; i < scratch_entrys; i++ )
	{
		scratch_to_node( tn, i );
    }
    
    /* Track the maximum number of entrys (used to alloc the format
     * buffer area)
     */
    if( scratch_entrys > most_entrys)
	{
		most_entrys = scratch_entrys;
    }
    
    DBUG_RETURN (tn);
}



static void scratch_to_node (tn, ent)
struct TreeNode *tn;
int ent;
{
    struct ScratchEntry *se;
    struct TreeEntry *te;

    DBUG_ENTER ("scratch_to_node");
    se = find_scratch_entry (ent);
    te = find_entry (tn, ent);
    CopyMem ((char *) se, (char *) te, (long) sizeof (struct TreeEntry));
    DBUG_VOID_RETURN;
}

/*..............................................................*/
/*								*/
/*	Directory Reading & Scratchbuffer access		*/
/*..............................................................*/


static ERRORCODE read_directory (lock)
BPTR lock;
{
    ERRORCODE rc;
    BOOL finished = FALSE;
	int rfc_check;

	
    DBUG_ENTER ("read_directory");

    if (check_cancel () == TRUE) {
	/* User wants us to stop */
	DBUG_RETURN (ERC_USER);
    }

	rfc_check = rfc;
    while (finished == FALSE)
	{
		rfc = rfc_check;
		rc = read_dir (lock);
		if(  FlagIsSet( rc, ERF_PRIVATE )  )
		{
	    	/* We overflowed the scratchbuffer, see about making it bigger */
		    rc = expand_buffer ();
		    if( rc != 0 )
			{
				/* Expand did not work, this is fatal! */
				finished = TRUE;
		    }
		}
		else
		{
	    	/* Either the directory was read, or there was a fatal
		     * error of some kind.  In either case, we're done!
		     */
	    	finished = TRUE;
		}
    }
    
    DBUG_RETURN (rc);
}



/*
 * Fill the scratchbuffer (an area containing contiguous ScratchEntry
 * structures) with the contents of the locked directory.
 * A non-zero return from this function indicates an error.
 */

static ERRORCODE read_dir (lock)
BPTR lock;
{
    int i;
    struct FileInfoBlock *fib;
    struct ScratchEntry *se;

    DBUG_ENTER ("read_dir");

    /* Get a file info block we can use to read directory entries into. */
    fib = (struct FileInfoBlock *)
		RemAlloc ((ULONG) sizeof (struct FileInfoBlock), MEMF_PUBLIC);

    if( fib == NULL )
	{
		DBUG_RETURN (ERC_NO_MEMORY | ERR01);
    }
    
    if( Examine( lock, fib ) == 0  )
	{
		RemFree (fib);
		DBUG_RETURN (ERC_IO_FAILED | ERR02);
    }
    
    /* 
     * Loop through all the directory entrys, filling the scratchbuffer
     * as we go.
     */
    for( i = 0; i < max_scratch_entrys; i++ )
	{
		/* Get the first directory entry */
		if(  ExNext( lock, fib ) == 0  )
		{
	    	/* We failed, find out why */
		    if( IoErr () == ERROR_NO_MORE_ENTRIES )
			{
				/* Not really a failure, just finished. */
				RemFree (fib);
				scratch_entrys = i;
				DBUG_RETURN (0);
	    	}
			else
			{
				/* A real failure of some sort */
				RemFree (fib);
				DBUG_RETURN (ERC_IO_FAILED | ERR03);
	    	}
		}
	
		/* 
		 * Now we have an entry in the fib, copy stuff we want to
		 * a scratchbuffer entry.
		 */
		se = find_scratch_entry (i);
		se -> Date = amigados_to_secs (fib -> fib_Date.ds_Days,
		fib -> fib_Date.ds_Minute, fib -> fib_Date.ds_Tick);
		strlcpy (se -> Name, fib -> fib_FileName, MAX_NAME);
		se -> Status = (UBYTE) (fib -> fib_Protection & 0x000000FF);

		if(fib -> fib_EntryType < 0)
		{
		    /* A file */
	    	se -> Size = fib -> fib_Size;
		    se -> Flags = ENTRY_SELECTED;
		    /* Display the filecount and file name (and path?) */
		    disp_run_count (++rfc, se -> Name);
		    /* Test wether or not the user wants this file */
		    if( want_file ((struct TreeEntry *) se) == FALSE)
			{
				i--;
		    }
		}
		else
		{
		    /* A directory */
			se -> Size = NULL;
		    se -> Flags = ENTRY_SELECTED | ENTRY_IS_DIR;
		}
    }
    
    /* if we get here, the scratch buffer overflowed */
    RemFree (fib);

    DBUG_RETURN (ERF_PRIVATE | ERR04);
}

static struct ScratchEntry *find_scratch_entry (n)
int n;
{
    struct ScratchEntry *se;

    DBUG_ENTER ("find_scratch_entry");
    se = scratchbuf + n;
    DBUG_RETURN (se);
}

/*
 * Expands the size of the scratch buffer area.  Note that this will
 * cause the pointer to change.
 * At some point, this routine may want to limit how big the scratch
 * buffer will grow.  But not today.
 */

static ERRORCODE expand_buffer ()
{
    int new_count;

    DBUG_ENTER ("expand_buffer");
    /* Free the existing buffer */
    RemFree (scratchbuf);
    /* Increase by 50% */
    new_count = max_scratch_entrys + (max_scratch_entrys / 2);
    /* Allocate a new, improved scratch buffer */
    scratchbuf = (struct ScratchEntry *)
                 RemAlloc ((ULONG) (sizeof (struct ScratchEntry) * new_count),
                 MEMF_PUBLIC);
    if (scratchbuf == NULL) {
	max_scratch_entrys = 0;
	DBUG_RETURN (ERC_NO_MEMORY | ERR08);
    }
    max_scratch_entrys = new_count;
    expansions++;		/* How many times we have expanded. */
    DBUG_RETURN (0);
}

/*..............................................................*/
/*								*/
/*	Deletion of our directory tree				*/
/*..............................................................*/


/* Recursivly free the passed node and any nodes beneath it. */

void free_tree (tn)
struct TreeNode *tn;
{
    int i;
    struct TreeEntry *te;

    DBUG_ENTER ("free_tree");
    /* Not the trusting sort... */
    if (tn == NULL) {
	DBUG_VOID_RETURN;
    }
    for (i = 0; i < tn -> Entries; i++) {
	te = find_entry (tn, i);
	if (FlagIsSet (te -> Flags, ENTRY_IS_DIR)) {
	    /* 
	     * This is a directory so free the node it points to.
	     * Note that if this is an entry for an empty dir, no
	     * harm will come of trying to free a null pointer.
	     */
	    free_tree ((struct TreeNode *) (te -> Size));
	}
    }
    free_node (tn);
    /* Free the buffers for entry list formatting */
    if (entry_string != NULL) {
	RemFree (entry_string);
	entry_string = NULL;
    }
    if (entry_color != NULL) {
	RemFree (entry_color);
	entry_color = NULL;
    }
    DBUG_VOID_RETURN;
}

/*
 * This will free a TreeNode allocated by create_node.  Any sub-nodes
 * should have been freed prior to this call!
 */

static void free_node (tn)
struct TreeNode *tn;
{
    DBUG_ENTER ("free_node");
    FreeMem (tn, tn -> Size);
    DBUG_VOID_RETURN;
}

/*..............................................................*/
/*								*/
/*	Access to our directory tree				*/
/*..............................................................*/

/*
 * This will walk through the passed node and all sub-nodes, calling
 * function of the users choice for each _file_ entry.
 * The users function must return a boolean true or false.  If a false
 * is returned, the walk will abort, returning to the function which
 * originally called walk_files().  The declaration for the function
 * which will be called-back should be:
 *		BOOL myfunc (tn)
 *		struct TreeNode *tn;
 *		{...
 *
 * The order of the files will not form any usefull pattern.
 * Note that this will not call-back for a directory entry.
 * Note also that this is NOT reentrant!
 *
 * This could be written to use walk_nodes(), in which case the call-backs
 * would be in order, at least within a directory.  I felt that doing
 * it this way was a little faster.
 */

BOOL walk_files (tn, func)
struct TreeNode *tn;
BOOL (*func) ();
{
    BOOL rtnval;

    DBUG_ENTER ("walk_files");
    wf_func = func;
    if (tn == NULL) {
	DBUG_RETURN (FALSE);
    }
    rtnval = _walk_files (tn);
    DBUG_RETURN (rtnval);
}

/* This is the recursing part of walk_files() that does all the work */

static BOOL _walk_files (tn)
struct TreeNode *tn;
{
    struct TreeEntry *te;
    int i;

    DBUG_ENTER ("_walk_files");
    for (i = 0; i < tn -> Entries; i++) {
	te = find_entry (tn, i);
	if (FlagIsSet (te -> Flags, ENTRY_IS_DIR)) {
	    /* A directory entry.  Perhaps we will recurse. */
	    if ((struct TreeNode *) (te -> Size) != NULL) {
		/* There is a sub-node, let us descend. */
		if (_walk_files ((struct TreeNode *) te -> Size) == FALSE) {
		    /* Somewhere down there, the user said quit. */
		    DBUG_RETURN (FALSE);
		}
	    }
	} else {
	    /* A file!  Time to call-back the users function. */
	    if ((*wf_func) (te) == FALSE) {
		/* Hey, the call-back func wants us to quit. */
	        DBUG_RETURN (FALSE);
	    }
	}
    }
    DBUG_RETURN (TRUE);
}

/*
 * This will walk through the passed node and all sub-nodes, calling
 * function of the users choice for each entry, be it file or dir.
 * The users function must return a boolean true or false.  If a false
 * is returned, the walk will abort, returning to the function which
 * originally called walk_entrys().  The declaration for the function
 * which will be called-back should be:
 *		BOOL myfunc (tn)
 *		struct TreeNode *tn;
 *		{...
 *
 * The order of the files will not form any usefull pattern.
 * Note that this is NOT reentrant!
 *
 * This could be written to use walk_nodes(), in which case the call-backs
 * would be in order, at least within a directory.  I felt that doing
 * it this way was a little faster.
 */

BOOL walk_entrys (tn, func, buffer)
struct TreeNode *tn;
BOOL (*func) ();
char *buffer;
{
    BOOL rtnval;

    DBUG_ENTER ("walk_entrys");
    we_func = func;
    we_buf = buffer;
    if (tn == NULL) {
	rtnval = FALSE;
    } else {
	rtnval = _walk_entrys (tn);
    }
    DBUG_RETURN (rtnval);
}


/* This is the recursing part of walk_entrys() that does all the work */

static BOOL _walk_entrys (tn)
struct TreeNode *tn;
{
    struct TreeEntry *te;
    int i;

    DBUG_ENTER ("_walk_entrys");
    for (i = 0; i < tn -> Entries; i++) {
	te = find_entry (tn, i);
	/* Call-back the users function. */
	if ((*we_func) (te) == FALSE) {
	    /* Hey, the call-back func wants us to quit. */
	    DBUG_RETURN (FALSE);
	}
	if (FlagIsSet (te -> Flags, ENTRY_IS_DIR)) {
	    /* A directory entry.  Perhaps we will recurse. */
	    if ((struct TreeNode *) (te -> Size) != NULL) {
		/* There is a sub-node, let us descend. */
		if (we_buf != NULL) {
		   add_path (we_buf, te -> Name);
		}
		if (_walk_entrys ((struct TreeNode *) te -> Size) == FALSE) {
		    /* Somewhere down there, the user said quit. */
		    DBUG_RETURN (FALSE);
		}
		if (we_buf != NULL) {
		    subtract_path (we_buf);
		}
	    }
	}
    }
    DBUG_RETURN (TRUE);
}

/*
 * This will walk through the passed node and all sub-nodes, calling
 * function of the users choice for each TreeNode.
 * The users function must return a boolean true or false.  If a false
 * is returned, the walk will abort, returning to the function which
 * originally called walk_nodes().  The declaration for the function
 * which will be called-back should be:
 *		BOOL myfunc (tn)
 *		struct TreeNode *tn;
 *		{...
 * Note that the desired function will be called for the passed node
 * in addition to it's sub-nodes.
 * Note also that this is NOT reentrant!
 *
 * If the variable buffer is non-NULL, this will
 * cause this string buffer to contain the
 * path to the files in the directory.
 */

BOOL walk_nodes (tn, func, buffer)
struct TreeNode *tn;
BOOL (*func) ();
char *buffer;
{
    BOOL rtnval;

    DBUG_ENTER ("walk_nodes");
    wn_func = func;
    wn_buf = buffer;
    rtnval = _walk_nodes (tn);
    DBUG_RETURN (rtnval);
}

/* This is the recursing part of walk_nodes() that does all the work */

static BOOL _walk_nodes (tn)
struct TreeNode *tn;
{
    struct TreeEntry *te;
    int i;

    DBUG_ENTER ("_walk_nodes");
    if (tn == NULL) {
	DBUG_RETURN (FALSE);
    }
    if ((*wn_func) (tn) == FALSE) {
	DBUG_RETURN (FALSE);
    }
    for (i = 0; i < tn -> Entries; i++) {
	te = find_entry (tn, i);
	if (FlagIsSet (te -> Flags, ENTRY_IS_DIR) && (te -> Size != NULL)) {
	    /* Descend */
	    if (wn_buf != NULL) {
		add_path (wn_buf, te -> Name);
		/* disp_path (current_path); */
	    }
	    if (_walk_nodes ((struct TreeNode *) te -> Size) == FALSE) {
		DBUG_RETURN (FALSE);
	    }
	    if (wn_buf != NULL) {
		subtract_path (wn_buf);
		/* disp_path (current_path); */
	    }
	}
    }
    DBUG_RETURN (TRUE);
}

/*
 * Return a pointer to the n'th entry.  Note that the first entry
 * would be entry 0.
 */

struct TreeEntry *find_entry (tn, n)
struct TreeNode *tn;
int n;
{
    struct TreeEntry *te;
    struct TreeEntry *fe;

    DBUG_ENTER ("find_entry");
    fe = &tn -> FirstEntry;
    te = fe + n;
    DBUG_RETURN (te);
}

/*
 * Search the tree for a file of the given name.  Name should be
 * relative to the root volume/path.
 * Return is NULL if filename not in the tree
 */

struct TreeEntry *find_filename (tn, name)
struct TreeNode *tn;
char *name;
{
    DBUG_ENTER ("find_filename");
    ff_name = ff_buf2;
    ff_te = NULL;
    strcpy (ff_buf, root_name);
    strcpy (ff_buf2, root_name);
    add_path (ff_buf2, name);
    walk_entrys (root_node, _find_filename, ff_buf);
    DBUG_RETURN (ff_te);
}

static BOOL _find_filename (te)
struct TreeEntry *te;
{
    DBUG_ENTER ("_find_filename");
    add_path (ff_buf, te -> Name);
    if (strequal (ff_buf, ff_name)) {
	ff_te = te;
	/* End the search early, since we have succeded */
	DBUG_RETURN (FALSE);
    }
    subtract_path (ff_buf);
    DBUG_RETURN (TRUE);
}
