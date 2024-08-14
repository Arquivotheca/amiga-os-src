/************************************************************************
 *									*
 *	Copyright (c) 1988 Enhanced Software Technologies, Inc.		*
 *			     All Rights Reserved			*
 *									*
 *	This software and/or documentation is protected by U.S.		*
 *	Copyright Law (Title 17 United States Code).  Unauthorized	*
 *	reproduction and/or sales may result in imprisonment of up	*
 *	to 1 year and fines of up to $10,000 (17 USC 506).		*
 *	Copyright infringers may also be subject to civil liability.	*
 *									*
 ************************************************************************
 */


/*
 *  FILE
 *
 *	estimate.c    functions for estimating media usage
 *
 *  SCCS
 *
 *	@(#)estimate.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	Contains functions for estimating media usage for same
 *	command with "create mode".  This is only an estimate and
 *	may not be accurate if some files are "statable" but
 *	unreadable.  However, when wrong the estimate will almost always
 *	be an over-estimate.
 *
 *	Reports the estimated number of volumes required, the number
 *	of files to be archive, the total number of archive blocks,
 *	and the total size of the archive in kilobytes.
 *
 */

#include "globals.h"

/*
 *	Local variables.
 */

static long files = 0L;		/* Number of files found */


/*
 *	Local function declarations.
 */

static VOID efile PROTO((struct finfo *fip));
static VOID do_estimate PROTO((struct finfo *fip));
static VOID totals PROTO((void));


/*
 *  NAME
 *
 *	estimate    estimate the media usage requirements
 *
 *  SYNOPSIS
 *
 *	VOID estimate ()
 *
 *  DESCRIPTION
 *
 *	This is the main entry point for estimate media usage when
 *	creating archives.
 *	It is called once all the command line options have been
 *	processed and common initialization has been performed.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin estimate
 *	    Set current mode to estimate
 *	    Reset count of files
 *	    Accumulate space for archive header block
 *	    Walk directory tree, estimate requirements for files
 *	    Accumulate space for archive trailer block
 *	    Print estimate totals
 *	    Discard any linkage information collected
 *	End create
 *
 */

VOID estimate ()
{
    
    DBUG_ENTER ("estimate");
    mode = 'e';
    files = 0L;
    ar_estimate (1L);
    if (flags.Zflag & (Z_ON | Z_AUTO)) {
	bru_message (MSG_NOEZ);
    }
    if (flags.PFflag) {
	filter (efile);
    } else {
	tree_walk (efile);
    }
    ar_estimate (1L);
    totals ();
    free_list ();
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	efile    estimate usage for a file
 *
 *  SYNOPSIS
 *
 *	static VOID efile (fip)
 *	struct finfo *fip;
 *
 *  DESCRIPTION
 *
 *	Given pointer to a file information structure, estimates
 *	the media requirements for archiving this particular file.
 *	Each regular file requires one block for the file header
 *	and zero or more data blocks for storing the file's contents.
 *
 *	Note that directories, block special files, character special
 *	files, and other special files only have a header block written.
 *	This is so those nodes can be restored with the proper attributes.
 *	This is a particular failing of the Unix "tar" utility.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin efile
 *	    If there is a file to estimate and is not "." or ".." then
 *		If this file is selected then
 *		    If estimating confirmed then
 *			Committed, so estimate it's requirements
 *		    End if
 *		End if
 *	    End if
 *	End efile
 *
 */

static VOID efile (fip)
struct finfo *fip;
{
    DBUG_ENTER ("efile");
    DBUG_PRINT ("path", ("estimate usage for \"%s\"", fip -> fname));
    if (*fip -> fname != EOS && !DOTFILE (fip -> fname)) {
	if (selected (fip)) {
	    if (confirmed ("e", fip)) {
		do_estimate (fip);
	    }
	}
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	do_estimate    committed to doing estimate so do it
 *
 *  SYNOPSIS
 *
 *	static VOID do_estimate (fip)
 *	struct finfo *fip;
 *
 *  DESCRIPTION
 *
 *	Once it has been decided to estimate a file's requirements
 *	after applying all tests, this function is called to do
 *	the actual estimation.  Note how this parallels the archive
 *	creation logic.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin do_estimate
 *	    If not a symbolic link then
 *		Set linked file name to NULL.
 *	    End if
 *	    If not directory and more than one link then
 *		Add link to list of linked files
 *	    End if
 *	    If the filenames fit in the blocks then
 *		Increment count of files to archive
 *		Do verbosity processing
 *		If the file is linked to another file then
 *		    Remember that file is linked
 *		Else
 *		    Remember file is not linked
 *		End if
 *		Accumulate space for file header block
 *		If not linked and file is a regular file then
 *		    Accumulate space for file contents
 *		End if
 *	    End if
 *	End do_estimate
 *
 */

static VOID do_estimate (fip)
struct finfo *fip;
{
    BOOLEAN linked;
    
    DBUG_ENTER ("do_estimate");
    if (!IS_FLNK (fip -> bstatp -> bst_mode)) {
	fip -> lname = NULL;
    }
    if (!IS_DIR (fip -> bstatp -> bst_mode) && fip -> bstatp -> bst_nlink > 1) {
	fip -> lname = add_link (fip);
    }
    if (namesfit (fip)) {
	files++;
	verbosity (fip);
	if (fip -> lname != NULL && *fip -> lname != EOS) {
	    linked = TRUE;
	} else {
	    linked = FALSE;
	}
	ar_estimate (1L);
	if (!linked && IS_REG (fip -> bstatp -> bst_mode)) {
	    ar_estimate ((LBA) ZARBLKS (fip));
	}
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	totals   print results of estimate
 *
 *  SYNOPSIS
 *
 *	static VOID totals ()
 *
 *  DESCRIPTION
 *
 *	Prints results of the estimate.  Reports the number of
 *	volumes, the number of archive blocks, and the total
 *	number of kilobytes of archive.
 *
 */
 
static VOID totals ()
{
    int volume;
    LBA arblocks;
    
    DBUG_ENTER ("totals");
    volume = ar_vol () + 1;
    arblocks = ar_tell ();
    voutput ("%s: %d volume(s), %ld files,", info.bru_name, volume, files);
    voutput (" %ld archive blocks,", arblocks);
    voutput (" %ld Kbytes", (long) KBYTES (arblocks));
    vflush ();
    DBUG_VOID_RETURN;
}

