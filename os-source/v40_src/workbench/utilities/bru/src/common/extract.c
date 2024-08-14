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
 *	extract.c    contains routines specific to extracting files
 *
 *  SCCS
 *
 *	@(#)extract.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	Contains routines specific to extracting files from bru archives.
 *
 */


#include "globals.h"

/*
 *	Local functions.
 */

static VOID xfile PROTO((union blk *blkp, struct finfo *fip));
static VOID makelink PROTO((struct finfo *fip));
static BOOLEAN checkulimit PROTO((struct finfo *fip));
static VOID makefile PROTO((union blk *blkp, struct finfo *fip));
static VOID xregfile PROTO((union blk *blkp, struct finfo *fip));
static VOID makespecial PROTO((struct finfo *fip));
static VOID attributes PROTO((struct finfo *fip));
static VOID do_extract PROTO((union blk *blkp, struct finfo *fip));
static BOOLEAN makeparent PROTO((struct bstat *bstatp, struct finfo *fip));
static BOOLEAN makedir PROTO((struct finfo *fip));
static VOID makestat PROTO((struct bstat *bstatp));


/*
 *  NAME
 *
 *	extract    main entry point for extraction of file from archive
 *
 *  SYNOPSIS
 *
 *	VOID extract ()
 *
 *  DESCRIPTION
 *
 *	This is the main entry point for extracting files from a bru
 *	archive.  It is called once all the command line options
 *	have been processed and common initialization has been
 *	performed.
 *
 */

VOID extract ()
{
    DBUG_ENTER ("extract");
    mode = 'x';
    reload ("extraction");
    ar_open ();
    scan (xfile);
    ar_close ();
    DBUG_VOID_RETURN;
}


/*
 *  NAME
 *
 *	xfile    control extraction of a single file
 *
 *  SYNOPSIS
 *
 *	static VOID xfile (blkp, fip)
 *	union blk *blkp;
 *	struct finfo *fip;
 *
 *  DESCRIPTION
 *
 *	Controls extraction of a single file from the archive.
 *	Blkp points to the file header block and fip points
 *	to a file information structure.
 *
 *	There was initially some question about how to report
 *	files which are not extracted because there is an existing
 *	file which is more recent.  Since files that ARE extracted
 *	are only reported if verbosity is enabled, by analogy, files
 *	that are NOT extracted are reported only if verbosity is
 *	enabled.  The basic philosophy is to silently bring the existing
 *	hierarchy up to date by replacing missing files and
 *	superceding out of date files, without overwriting more
 *	recent files.  If every file in a given class (regular,
 *	directory, block special, etc) is to be extracted regardless
 *	of modification date, bru must be explicitly told this via
 *	the -u (unconditional) flag.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin xfile
 *	    Determine whether or not file is a directory
 *	    If this file is to be extracted then
 *		If unconditional extraction or file out of date then
 *		    If extraction confirmed then
 *			Issue verbosity message
 *			Do the extraction
 *			End if
 *		    End if
 *		Else
 *		    If file is not a directory and verbosity enabled
 *			Warn user that it was not extracted
 *		    End if
 *		End if
 *	    End if
 *	End xfile
 *
 */

static VOID xfile (blkp, fip)
union blk *blkp;
struct finfo *fip;
{
    BOOLEAN is_directory;

    DBUG_ENTER ("xfile");
    is_directory = IS_DIR (fip -> bstatp -> bst_mode);
    if ((IS_STEM (fip) && is_directory) || IS_LEAF (fip) || IS_EXTENSION (fip)) {
	if (unconditional (fip) || out_of_date (fip)) {
	    if (confirmed ("x", fip)) {
		verbosity (fip);
		do_extract (blkp, fip);
	    }
	} else {
	    if (!is_directory && flags.vflag > 1) {
		bru_message (MSG_SUPERSEDE, fip -> fname);
	    }
	}
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	makelink    link file being extracted to an existing file
 *
 *  SYNOPSIS
 *
 *	static VOID makelink (fip)
 *	struct finfo *fip;
 *
 *  DESCRIPTION
 *
 *	Links the file being extracted (file1) to the existing file
 *	(file2) that it is supposed to be linked to.
 *
 *	Note that this will fail if file2 does not exist.
 *	In this case there is nothing else to be done since the file
 *	contents for file2 are not stored in the archived file1.
 *
 *	Also note that any existing file1 is unlinked.  If the
 *	new link cannot be made for some reason, the effective
 *	result is the deletion of any existing file1.  This
 *	could be fixed by first linking an existing file1 to some
 *	temporary and then either unlinking or relinking it as
 *	necessary once the link between the new file1 and file2
 *	is established or denied respectively.  Another way is
 *	to open the existing file1, unlink the existing file1,
 *	then either close file1 or copy it back to a new 
 *	instance of file1 as necessary.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin makelink
 *	    If file to be linked to does not exist then
 *		Tell user we can't make the link
 *	    Else
 *		Unlink any existing file to be linked
 *		If the link to the target file is successful then
 *		    Set the attributes of the link just made
 *		End if
 *	    End if
 *	End makelink
 *
 */

static VOID makelink (fip)
struct finfo *fip;
{
    DBUG_ENTER ("makelink");
    if (!file_access (fip -> lname, A_EXISTS, FALSE)) {
	bru_message (MSG_MKLINK, fip -> fname, fip -> lname);
    } else {
	(VOID) s_unlink (fip -> fname);
	if (mklink (fip -> lname, fip -> fname)) {
	    attributes (fip);
	}
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	checkulimit     check or set ulimit as necessary
 *
 *  SYNOPSIS
 *
 *	static BOOLEAN checkulimit (fip)
 *	struct finfo *fip;
 *
 *  DESCRIPTION
 *
 *	Check to see if the file to be extracted is larger than the
 *	maximum file size set by the current ulimit value.  If so
 *	then attempt to raise the ulimit if the current real user
 *	id is the superuser (we can't just try it and let it fail
 *	for normal users since we are running suid to root).  If the
 *	user is not root, then issue a warning and return FALSE.
 *	If the file is less than the limit, or the limit is
 *	successfully raised, return TRUE.
 *
 *	There are bugs on some systems (Apple A/UX 1.0 for example)
 *	where if the ulimit is not a multiple of the actual filesystem
 *	block size, you cannot write the last few blocks.  The
 *	ULIMITSLOP factor thus adds enough 512 byte blocks to work
 *	around this bug, up to block sizes of 8K.  This will, of course
 *	cause problems with huge files if the ULIMITSLOP factor causes
 *	the long to overflow...
 */

#define ULIMITSLOP 16

static BOOLEAN checkulimit (fip)
struct finfo *fip;
{
    int result = TRUE;
    long needlimit;

    DBUG_ENTER ("checkulimit");
    needlimit = (fip -> bstatp -> bst_size / 512) + 1 + ULIMITSLOP;
    DBUG_PRINT ("ulimit", ("need ulimit %ld", needlimit));
    if (needlimit > info.bru_ulimit) {
	if (info.bru_uid != 0) {
	    bru_message (MSG_TOOLARGE, fip -> fname);
	    result = FALSE;
	} else {
	    if (s_ulimit (2, needlimit) == needlimit) {
		info.bru_ulimit = needlimit;
	    } else {
		bru_message (MSG_ULIMITSET, needlimit);
		result = FALSE;
	    }
	}
    }
    DBUG_RETURN (result);
}


/*
 *  FUNCTION
 *
 *	makefile    extract a regular file
 *
 *  SYNOPSIS
 *
 *	static VOID makefile (blkp, fip)
 *	union blk *blkp;
 *	struct finfo *fip;
 *
 *  DESCRIPTION
 *
 *	Controls extract of a regular file by testing for
 *	for it's existance and/or writability.  The file
 *	will only be extracted if it does not currently exist
 *	or if the user has write permission to overwrite an
 *	existing version.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin makefile
 *	    If file does not exist then
 *		Go ahead and extract it from archive
 *	    Else
 *		If file is not writable then
 *		    Tell user he can't overwrite the file
 *		Else
 *		    Go ahead and extract it from archive
 *		End if
 *	    End if
 *	End makefile
 *
 */

static VOID makefile (blkp, fip)
union blk *blkp;
struct finfo *fip;
{
    DBUG_ENTER ("makefile");
    if (checkulimit (fip)) {
	if (!file_access (fip -> fname, A_EXISTS, FALSE)) {
	    xregfile (blkp, fip);
	} else {
	    if (!file_access (fip -> fname, A_WRITE, FALSE)) {
		bru_message (MSG_OVRWRT, fip -> fname);
	    } else {
		xregfile (blkp, fip);
	    }
	}
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	xregfile    extract a normal file from archive
 *
 *  SYNOPSIS
 *
 *	static VOID xregfile (blkp, fip)
 *	union blk *blkp;
 *	struct finfo *fip;
 *
 *  DESCRIPTION
 *
 *	This routine is responsible for extracting a normal file from
 *	an archive.  If the -S option has been enabled (try to
 *	intelligently handle sparse files), then any blocks that
 *	contain only nulls will cause bru to seek for that many
 *	bytes forward in the file, rather than actually writing
 *	null bytes.  Depending upon the circumstances, this
 *	can result in fewer actual disk blocks consumed, since 
 *	no block will be consumed if it contains only nulls.
 *	Note also that we suppress this seeking on the last block
 *	to ensure that the file actually gets some data written
 *	at the end, otherwise the size would be wrong (seeking
 *	alone does not extend the file).
 *
 *	Returns TRUE if file was created, FALSE otherwise.
 *	Note that return of TRUE does NOT indicate successful
 *	extraction, only that a file was created.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin xregfile
 *	    Initialize the file truncated flag to FALSE
 *	    Create the file being extracted
 *	    If couldn't be created then 
 *		Inform user
 *	    Else
 *		Get number of bytes to extract
 *		If any bytes to extract then
 *		    For each block to be extracted
 *			Seek to archive block
 *			Read the archive block
 *			If extracting a partial block then
 *			    Only use that many bytes
 *			End if
 *			If preserving holes and block is all nulls then
 *			    Attempt to seek to end of hole
 *			    If not successful then
 *				Report seek error
 *				Write null bytes instead of seeking
 *			    Else
 *				Consider the bytes as written
 *			    End if
 *			Else
 *			    Write the bytes to the new file
 *			End if
 *			If write did not write requested number then
 *			    Remember that the file was truncated
 *			End if
 *		    End for
 *		    If last write returned error then
 *			Notify user about write error
 *		    End if
 *		    If file was truncated then
 *			Warn user about truncation
 *		    End if
 *		End if
 *		If file close get an error then
 *		    Warn user about the close error
 *		End if
 *		Set the attributes of the extracted file
 *	    End if
 *	End xregfile
 *
 */

static VOID xregfile (blkp, fip)
union blk *blkp;
struct finfo *fip;
{
    off_t bytes;
    int iobytes;
    UINT wbytes;
    int fildes;
    BOOLEAN truncated = FALSE;
    BOOLEAN doextract = TRUE;
    char *fname;

    DBUG_ENTER ("xregfile");
    if (IS_COMPRESSED (fip)) {
	if (openzfile (fip)) {
	    fname = fip -> zfname;
	    fildes = fip -> zfildes;
	} else {
	    bru_message (MSG_ZXFAIL, fip -> fname);
	    doextract = FALSE;
	}
    } else {
	fname = fip -> fname;
	FULLACCESS (fip);
	if (IS_CTG (fip -> bstatp -> bst_mode)) {
	    fildes = s_ccreat (fip -> fname, 0600, (long) ZSIZE (fip));
	} else {
	    fildes = s_creat (fip -> fname, 0600);
	}
	if (fildes == SYS_ERROR) {
	    bru_message (MSG_OPEN, fip -> fname);
	    doextract = FALSE;
	}
    }
    if (doextract) {
	bytes = ZSIZE (fip);
	if (bytes > 0) {
	    for (wbytes = DATASIZE; bytes > 0; bytes -= wbytes) {
		blkp = ar_next ();
		ar_read (fip);
		if (bytes < DATASIZE) {
		    wbytes = bytes;
		}
		if (bytes > DATASIZE && flags.Sflag
			&& allnulls (blkp -> FD, (int) wbytes)) {
		    if (s_lseek (fildes, (S32BIT) wbytes, 1) == SYS_ERROR) {
			bru_message (MSG_SEEK, fname);
			iobytes = s_write (fildes, blkp -> FD, wbytes);
		    } else {
			iobytes = wbytes;
		    }
		} else {
		    iobytes = s_write (fildes, blkp -> FD, wbytes);
		}
		if (iobytes != wbytes) {
		    truncated = TRUE;
		}
	    }
	    if (iobytes == SYS_ERROR) {
		bru_message (MSG_WRITE, fname);
	    }
	    if (truncated) {
		bru_message (MSG_FTRUNC, fname);
	    }
	}
	if (s_close (fildes) == SYS_ERROR) {
	    bru_message (MSG_CLOSE, fname);
	}
	if (!IS_COMPRESSED (fip) || decompfip (fip)) {
	    attributes (fip);
	}
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	makespecial    make a special file node
 *
 *  SYNOPSIS
 *
 *	static VOID makespecial (fip)
 *	struct finfo *fip;
 *
 *  DESCRIPTION
 *
 *	Is reponsible for controlling creation of special files.
 *	If the special file being made already exists, it is
 *	unlinked first, then the new node is made.
 *
 *	Adding code for 4.2 has complicated this routine, and made it
 *	very ugly.  It is even worse than it ought to be, since it is
 *	currently impossible under 4.2 to chmod a symbolic link (although
 *	a symlink with mode 000 can still be read!), or to reset the access
 *	and modification times on a symbolic link. This is dealt with 
 *	in attributes(), below.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin makespecial
 *	    If the current file exists then
 *		Unlink the file
 *	    End if
 *	    If under 4.2, not a pyramid, and file is a fifo, then
 *		Just create a regular file
 *		Set the attributes of the new node
 *	    Else if the file is a symbolic link then
 *		If cannot make the symbolic link then
 *		    Inform the user
 *		Else
 *		    Set the attributes of the new node
 *		Endif
 *	    Else if the new node cannot be made then
 *		Inform the user
 *	    Else
 *		Set attributes of the new node
 *	    End if
 *	End makespecial
 *
 */

static VOID makespecial (fip)
struct finfo *fip;
{
    DBUG_ENTER ("makespecial");
    if (file_access (fip -> fname, A_EXISTS, FALSE)) {
	(VOID) s_unlink (fip -> fname);	
    }
#if !HAVE_FIFOS
    if (IS_FIFO (fip -> bstatp -> bst_mode)) {
	int fd;

	FULLACCESS (fip);
	fd = s_creat (fip -> fname, 0666);	/* fudge it */
	if (fd == SYS_ERROR) {
	    bru_message (MSG_MKFIFO, fip -> fname);
	} else {
	    attributes (fip);
	    bru_message (MSG_FIFOTOREG, fip -> fname);
	    if (s_close (fd) == SYS_ERROR) {
		bru_message (MSG_CLOSE, fip -> fname);
	    }
	}
    } else
#endif
    if (IS_FLNK (fip -> bstatp -> bst_mode)) {
	if (! mksymlink (fip -> lname, fip -> fname) && ! flags.lflag) {
	    bru_message (MSG_MKSYMLINK, fip -> fname);
	} else {
	    attributes (fip);
	}
    } else if (s_mknod (fip -> fname, (int) fip -> bstatp -> bst_mode, (int) fip -> bstatp -> bst_rdev) == SYS_ERROR) {
	bru_message (MSG_MKNOD, fip -> fname);
    } else {
	attributes (fip);
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	attributes    set attributes of newly created file
 *
 *  SYNOPSIS
 *
 *	static VOID attributes (fip)
 *	struct finfo *fip;
 *
 *  DESCRIPTION
 *
 *	Using stat buffer information recovered from archive
 *	header block, sets the attributes of the file.
 *
 *	Under 4.2 BSD, it is impossible to reset the access and
 *	modification times of a symbolic link, or to change its
 *	mode with chmod.  Both chmod and utimes go through the
 *	symbolic link to the real file, which is not what we want.
 *	Therefore, we add code for checking symbolic links.
 *
 *	We have to build a utimbuf since the args to utime are not
 *	always the same as just taking the address of the atime
 *	field in the stat structure, as lots of nonportable programs
 *	sometimes do.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin attributes
 *	    If the mode cannot be set then
 *		Inform user that mode cannot be changed
 *	    End if
 *	    Change owner and group of the file
 *	    If the access and modification times cannot be set then
 *		Inform the user about time error
 *	    End if
 *	End attributes
 *
 */

static VOID attributes (fip)
struct finfo *fip;
{
    char *path;
    int s_mode;
    int s_owner;
    int s_group;
    struct utimbuf t;

    DBUG_ENTER ("attributes");
    (VOID) s_memset ((char *) &t, 0, sizeof (t));
    path = fip -> fname;
    s_mode = fip -> bstatp -> bst_mode;
    s_owner = fip -> bstatp -> bst_uid;
    s_group = fip -> bstatp -> bst_gid;
    if (s_owner != info.bru_uid && (info.bru_uid != 0 || flags.Cflag)) {
	if (s_mode & BS_ISUID) {
	    s_mode &= ~BS_ISUID;
	    bru_message (MSG_SUID, path);
	}
    }
    if (s_group != info.bru_gid && (info.bru_uid != 0 || flags.Cflag)) {
	if (s_mode & BS_ISGID) {
	    s_mode &= ~BS_ISGID;
	    bru_message (MSG_SGID, path);
	}
    }

    DBUG_PRINT ("attributes",
	("file %s, owner %d, group %d",	fip -> fname, s_owner, s_group));
    DBUG_PRINT ("attributes", ("file %s, mode %#o", fip -> fname, s_mode));

    file_chown (path, s_owner, s_group);

    if ((! IS_FLNK (fip -> bstatp -> bst_mode)) && s_chmod (path, s_mode) == SYS_ERROR) {
	bru_message (MSG_SMODE, path);
    }

    t.atime = fip -> bstatp -> bst_atime;
    t.mtime = fip -> bstatp -> bst_mtime;
    if ((! IS_FLNK (fip -> bstatp -> bst_mode)) && s_utime (fip -> fname, &t) == SYS_ERROR) {
	bru_message (MSG_STIMES, fip -> fname);
    }
    if (fip -> fi_flags & FI_AMIGA) {
	(VOID) setinfo (fip);
	if (flags.Asflag) {
	    mark_archived (fip);
	} else if (flags.Acflag) {
	    clear_archived (fip);
	}
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	do_extract    do the extraction
 *
 *  SYNOPSIS
 *
 *
 *	static VOID do_extract (blkp, fip)
 *	union blk *blkp;
 *	struct finfo *fip;
 *
 *  DESCRIPTION
 *
 *	Once all tests have been met and it has been decided to extract
 *	the file, this routine is called to do the actual extraction.
 *	At this point, we are committed to doing the extraction and
 *	cannot be interrupted until done.
 *
 *	Any missing parent directories will be created and then the
 *	file itself will be extracted.
 *
 *	For directories, there is an explicit call to set the attributes
 *	since the makedir function cannot do this if the directory already
 *	exists.  Thus directories which do not exist will have there
 *	attributes set when they are made by makedir, and again here.
 *	It is the case of existing directories that we only catch here.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin do_extract
 *	    Save current signal processing stat
 *	    Set up to catch signals
 *	    Check for directory in which to extract file
 *	    If no parent directory then
 *		Make a default stat buffer
 *		Make the parent directory
 *	    End if
 *	    If parent directory and parent accessible then
 *		If file to extract is a directory then
 *		    Make the directory
 *		    Set the directory attributes
 *		Else if file linked to another then
 *		    Make the linkage
 *		Else if not a regular file then
 *		    Make a special file
 *		Else
 *		    Make the file
 *		End if
 *	    End if
 *	    Pop the saved signal processing state
 *	    If interrupt came in while doing extraction
 *		Cleanup and exit
 *	    End if
 *	End do_extract
 *
 */


static VOID do_extract (blkp, fip)
union blk *blkp;
struct finfo *fip;
{
    struct bstat bsbuf;
    BOOLEAN got_parent;
    int bst_mode;
    SIGTYPE prevINT;
    SIGTYPE prevQUIT;
    
    DBUG_ENTER ("do_extract");
    sig_push (&prevINT, &prevQUIT);
    sig_catch ();
    got_parent = dir_access (fip -> fname, A_EXISTS, FALSE);
    if (!got_parent) {
	makestat (&bsbuf);
	got_parent = makeparent (&bsbuf, fip);
    }
    if (got_parent && dir_access (fip -> fname, A_WRITE, TRUE)) {
	bst_mode = fip -> bstatp -> bst_mode;
	if (IS_DIR (bst_mode)) {
	    (VOID) makedir (fip);
	    attributes (fip);
	} else if (LINKED (blkp) && ! IS_FLNK (bst_mode)) {
	    makelink (fip);
	} else if (!IS_REG (bst_mode) && !IS_CTG (bst_mode)) {
	    /*
	     * blkp and fip are passed down from scan, which will already
	     * set up the link name in fip, so makespecial can just pass
	     * it on if a symbolic link has to be made. In other words,
	     * we don't need to do any special checking for that here.
	     */
	    makespecial (fip);
	} else {
	    makefile (blkp, fip);
	}
    }
    sig_pop (&prevINT, &prevQUIT);
    if (interrupt) {
	done ();
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	makeparent    make a parent directory that does not exist
 *
 *  SYNOPSIS
 *
 *	static BOOLEAN makeparent (bstatp, fip)
 *	struct bstat *bstatp;
 *	struct finfo *fip;
 *
 *  DESCRIPTION
 *
 *	Given a pointer to a file info structure, attempts to
 *	make the parent directory if one does not already exist.
 *
 *	Note that the installation is recursive.  That is,
 *	if the parent of the parent does not exist, it is
 *	made also, with the same attributes.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin makeparent
 *	    If parent already exists then
 *		Result is TRUE
 *	    Else
 *		Find where parent and child names join.
 *		If no parent name
 *		    Tell user about bug
 *		    Result is FALSE
 *		Else
 *		    Split parent and child names
 *		    Attempt to make grandparent recursively
 *		    If attempt successful then
 *			Build a file info struct for parent
 *			Make the parent
 *		    End if
 *		    Rejoin parent and child names
 *		End if
 *	    End if
 *	    Return result
 *	End makeparent
 *
 */

static BOOLEAN makeparent (bstatp, fip)
struct bstat *bstatp;
struct finfo *fip;
{
    char *slash;
    BOOLEAN got_dir;
    struct finfo pfile;

    DBUG_ENTER ("makeparent");
    if (dir_access (fip -> fname, A_EXISTS, FALSE)) {
	got_dir = TRUE;
    } else {
	slash = s_strrchr (fip -> fname, '/');
	if (slash == NULL) {
	    bru_message (MSG_BUG, "makeparent");
	    got_dir = FALSE;
	} else {
	    *slash = EOS;
	    got_dir = makeparent (bstatp, fip);
	    if (got_dir) {
		finfo_init (&pfile, fip -> fname, bstatp);
		got_dir = makedir (&pfile);
	    }
	    *slash = '/';
	}
    }
    DBUG_RETURN (got_dir);
}


/*
 *  FUNCTION
 *
 *	makedir    make a directory
 *
 *  SYNOPSIS
 *
 *	static BOOLEAN makedir (fip)
 *	struct finfo *fip;
 *
 *  DESCRIPTION
 *
 *	Attempts to make a directory if one does not already exist.
 *
 *	Note that it is assumed that the parent has already
 *	been made by a call to makeparent.  If not, then
 *	makedir will fail if no parent exists.  Also, we need to
 *	verify that the user running bru has appropriate permission
 *	to write into the parent of the directory to be made, rather
 *	than depending upon the newdir/mkdir calls to enforce
 *	permissions.
 *
 *	Also note that the attributes are set ONLY if the directory
 *	is made.  This is because makedir may be called to ensure
 *	that parent directories exist in order to extract an archived
 *	directory.  If the attributes were unconditionally set,
 *	parents would take on the attributes of the child directory
 *	being extracted from the archive.  The actual directory
 *	being extracted has it attributes explicitly set elsewhere.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin makedir
 *	    If the directory already exists then
 *		Result is TRUE
 *	    Else if we can write in parent to make directory then
 *		Attempt to make the directory
 *		If directory node successfully made then
 *		    Set attributes of directory
 *		End if
 *	    Else
 *		Doesn't exist and can't make it, result is FALSE
 *	    End if
 *	    Return result
 *	End makedir
 *
 */

static BOOLEAN makedir (fip)
struct finfo *fip;
{
    BOOLEAN result;

    DBUG_ENTER ("makedir");
    if (file_access (fip -> fname, A_EXISTS, FALSE)) {
	result = TRUE;
    } else if (dir_access (fip -> fname, A_WRITE, TRUE)) {
	result = newdir (fip);
	if (result) {
	    attributes (fip);
	}
    } else {
	result = FALSE;
    }
    DBUG_RETURN (result);
}


/*
 *  FUNCTION
 *
 *	makestat    make a default stat buffer
 *
 *  SYNOPSIS
 *
 *	static VOID makestat (bstatp)
 *	struct bstat *bstatp;
 *
 *  DESCRIPTION
 *
 *	Initialize default stat info as appropriate for current user.
 *	This is typically used to set attributes of directories which
 *	must be made to install a file being extracted.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin makestat
 *	    Get current time
 *	    Get the mode word mask
 *	    Restore the mode word mask
 *	    Get the file mode and make it a directory
 *	    Initialize the mode
 *	    Initialize the user id
 *	    Initialize the group id
 *	    Initialize the access time
 *	    Initialize the modification time
 *	End makestat
 *
 */

static VOID makestat (bstatp)
struct bstat *bstatp;
{
    int mask;
    time_t now;

    DBUG_ENTER ("makestat");
    now = (time_t) s_time ((long *) 0);
    mask = s_umask (0);
    (VOID) s_umask (mask);
    bstatp -> bst_mode = (ushort) (BS_IFDIR | (DIR_MAGIC & ~mask));
    bstatp -> bst_uid = info.bru_uid;
    bstatp -> bst_gid = info.bru_gid;
    bstatp -> bst_atime = now;
    bstatp -> bst_mtime = now;
    DBUG_VOID_RETURN;
}
