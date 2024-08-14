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
 *	scan.c    routines to execute a function for each archived file
 *
 *  SCCS
 *
 *	@(#)scan.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	These routines provide a central facility for scanning an
 *	archive, passing control to a given function for each file
 *	in the archive.  The called function can process the file
 *	or ignore it as it chooses.
 *
 */

#include "globals.h"

/*
 *	Local functions.
 */

static VOID scan_errors PROTO((struct finfo *fip));
static VOID missing PROTO((char *name));


/*
 *  FUNCTION
 *
 *	scan    scan the archive executing function for each file
 *
 *  SYNOPSIS
 *
 *	VOID scan (funcp)
 *	VOID (*funcp)();
 *
 *  DESCRIPTION
 *
 *	Scans the archive, locating the header block of each
 *	archived file, decoding the header into a stat buffer,
 *	and calling the specified function with a block pointer
 *	pointing to the file header block, and a file information
 *	structure pointer.
 *
 *	Note that the pathname and state of the file linkage
 *	flag must be saved prior to executing the function.
 *	This is because if the file is a regular file,
 *	the function may request archive blocks itself,
 *	which may potentially overlay the block pointed to
 *	by blkp.
 *
 *	Also note that there is no guarantee that the routine
 *	scanning this file will leave the archive "current pointer"
 *	at any particular location.  If it is only interested in the
 *	header block, it will read no other blocks.  On the other
 *	hand, it might only read a few blocks of the current file,
 *	leaving some unread.  Or it might read all the blocks, leaving
 *	the header for the next file unread.  Or (and this is a "gotcha")
 *	it might read MORE than the current file due to error recovery
 *	(in the case of a hard read error for example) where the archive
 *	may even be left positioned at the start of the next volume.
 *	Thus we compute the address of the header block of the next
 *	file, and seek to there if we are not already there or past it.
 * 
 */


/*
 *  PSEUDO CODE
 *
 *	Begin scan
 *	    Initialize a file info structure for files found
 *	    Remember buffer for linked name
 *	    Reset flag for done with archive
 *	    While not done with archive
 *		Seek to next block of archive
 *		Reset file logical block address to zero
 *		Read the block from archive
 *		Done if at end of archive
 *		If not done then
 *		    If header check fails then
 *		        Tell user about error and resync attempt
 *		        Do
 *			    Seek to next block
 *			    Read block from archive
 *			    Done if at end of archive
 *		        While not done and bad header
 *		    Endif
 *		    If not done then
 *			Remember archive logical block of file header
 *			Decode header block
 *			Save copy of name from header block
 *			Save copy of linked name from header block
 *			Save state of file linkage from header block
 *			If this file is selected then
 *			    Reset scan flags
 *			    Initialize file logical block address
 *			    Reset file checksum error count
 *			    Get type of pathname
 *			    If no more matches possible then
 *				Set done with archive flag
 *			    Else
 *			        Invoke caller specified function
 *			        Check for errors during scan
 *			    End if
 *			End if
 *			If not done and not linked to another file then
 *			    Compute address of header block for next file
 *			    If not already there, or past it, then
 *				Skip over contents of file, if any
 *			    End if
 *			End if
 *		    End if
 *		End if
 *	    End while
 *	    Test tree for nodes not processed in archive scan
 *	    Reset all match information in tree for next scan
 *	End scan
 *
 */

#ifdef __STDC__
VOID scan (VOID (*funcp)(union blk *blkp, struct finfo *fip))
#else
VOID scan (funcp)
VOID (*funcp)();
#endif
{
    LBA hdrlba;			/* Lba of this file's header block */
    LBA nxthdr;			/* Lba of next file's header block */
    BOOLEAN done;		/* Finished scanning archive */
    union blk *blkp;		/* Pointer to block */
    BOOLEAN linked;		/* Linked to previous entry */
    struct finfo file;		/* A file info buffer */
    struct bstat bsbuf;		/* Decoded header stat info */
    char name[BRUPATHMAX];	/* File name buffer */
    char lname[BRUPATHMAX];	/* Linked name buffer */
    
    DBUG_ENTER ("scan");
    finfo_init (&file, name, &bsbuf);
    file.lname = lname;
    done = FALSE;
    while (!done) {
	DBUG_PRINT ("header", ("next header block"));
	blkp = ar_next ();
	file.flba = 0L;
	ar_read (&file);
	done = eoablk (blkp);
	if (!done) {
	    if (!hcheck (blkp)) {
	        bru_message (MSG_DSYNC);
	        do {
		    blkp = ar_next ();
		    file.flba = 0L;
		    ar_read (&file);
		    done = eoablk (blkp);
	        } while (!done && !hcheck (blkp));
	    }
	    if (!done) {
		hdrlba = ar_tell ();
		hstat (blkp, &file);
		linked = LINKED (blkp);
		if (selected (&file)) {
		    file.fi_flags &= ~(FI_CHKSUM | FI_BSEQ);
		    file.flba = 1L;
		    file.chkerrs = 0L;
		    file.type = path_type (file.fname);
		    if (file.type == FINISHED) {
			done = TRUE;
		    } else {
			(*funcp) (blkp, &file);
			scan_errors (&file);
		    }
		}
		if (!done && !linked) {
		    nxthdr = hdrlba + ZARBLKS (&file);
		    DBUG_PRINT ("nxthdr", ("next header at lba %ld", nxthdr));
		    if (nxthdr > ar_tell ()) {
			(VOID) ar_seek (nxthdr, 0);
		    }
		}
	    }
	}
    }
    nodes_ignored (missing);
    clear_tree ();
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	scan_errors    check for errors during file scan
 *
 *  SYNOPSIS
 *
 *	static VOID scan_errors (fip)
 *	struct finfo *fip;
 *
 *  DESCRIPTION
 *
 *	There are certain errors, such as a block checksum error,
 *	for which it is more appropriate to issue a message about
 *	on a file by file basis, rather than at each occurance.
 *	Thus information about these errors is collected for
 *	testing after the file has been processed.  This routine
 *	is responsible for testing for these special errors and
 *	issuing appropriate messages.
 *
 */

static VOID scan_errors (fip)
struct finfo *fip;
{
    DBUG_ENTER ("scan_errors");
    DBUG_PRINT ("fip", ("flags %#4x", fip -> fi_flags));
    if (fip -> fi_flags & FI_CHKSUM) {
	bru_message (MSG_SUM, fip -> fname, fip -> chkerrs);
    }
    if (fip -> fi_flags & FI_BSEQ) {
	bru_message (MSG_BSEQ, fip -> fname);
    }
    DBUG_VOID_RETURN;
}

static VOID missing (name)
char *name;
{
    DBUG_ENTER ("missing");
    bru_message (MSG_IGNORED, name);
    DBUG_VOID_RETURN;
}
