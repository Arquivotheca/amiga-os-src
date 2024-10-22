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
 *	diff.c    routines for finding archive differences
 *
 *  SCCS
 *
 *	@(#)diff.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	Contains routines for finding the differences between an
 *	archive and the current files on disk.
 *
 */



#include "globals.h"

#define D_MODE	000001		/* Mode is different */
#define D_INO	000002		/* Inodes different */
#define D_DEV	000004		/* Host device different */
#define D_RDEV	000010		/* Major/minor device different */
#define D_NLINK	000020		/* Number of links different */
#define D_UID	000040		/* User id different */
#define D_GID	000100		/* Group id different */
#define D_SIZE	000200		/* Size different */
#define D_ATIME	000400		/* Access time different */
#define D_MTIME	001000		/* Modification time different */
#define D_CONTS	002000		/* File contents different */
#define D_SYMLK 004000		/* Symbolic link to a different file */

struct diff {
    int mask;			/* Bit mask for difference found */
    char *short_name;		/* Name of the difference for user */
    char *long_name;		/* Verbose version of difference */
};

static struct diff diffs[] = {		/* Table of diffs (int size max) */
    D_MODE, "mode", "file access mode",
    D_INO, "inode", "inode number",
    D_DEV, "host-device", "host device (filesystem)",
    D_RDEV, "major/minor-device", "major/minor device number",
    D_NLINK, "nlinks", "number of links",
    D_UID, "uid", "user id",
    D_GID, "gid", "group id",
    D_SIZE, "size", "file size",
    D_ATIME, "atime", "time of last access",
    D_MTIME, "mtime", "time of last modification",
    D_CONTS, "contents", "file contents",
    D_SYMLK, "symlink", "contents of symbolic link",
    0, NULL, NULL
};
    
static int diffs_found;		/* Log of differences found */

/*
 *	Local functions.
 */

static VOID fverify PROTO((union blk *blkp, struct finfo *fip));
static VOID hverify PROTO((struct finfo *afip, struct finfo *cfip));
static VOID cverify PROTO((struct finfo *afip, struct finfo *cfip));
static BOOLEAN bdiff PROTO((char *cp1, char *cp2, int count));
static VOID diff_report PROTO((struct finfo *afip));
#if HAVE_SYMLINKS
static VOID sverify PROTO((struct finfo *afip, struct finfo *cfip));
#else
static VOID sverify PROTO((struct finfo *afip));
#endif

/*
 *	Macros to check stat items.
 */

#define HVERIFY(item,mask,level) \
	if (flags.dflag>=level && afip->bstatp->item != cfip->bstatp->item) { \
	    diffs_found |= mask; \
	}

#if HAVE_SYMLINKS
#define SVERIFY(afip,cfip) sverify(afip,cfip)
#else
#define SVERIFY(afip,cfip) sverify(afip)
#endif


/*
 *  FUNCTION
 *
 *	diff    mainline for the differences mode
 *
 *  SYNOPSIS
 *
 *	VOID diff ()
 *
 *  DESCRIPTION
 *
 *	This is the main entry point for the diff mode, called after
 *	all initialization is complete.  Since more than one mode
 *	may be specified on the command line, the "mode" variable
 *	is set to the mode currently being executed.  The archive
 *	is opened, scaned for differences, and then closed.
 *
 */

VOID diff ()
{
    DBUG_ENTER ("diff");
    mode ='d';
    reload ("differences");
    ar_open ();
    scan (fverify);
    ar_close ();
    DBUG_VOID_RETURN;
}


/*
 *  NAME
 *
 *	fverify    verify archived file matches existing file
 *
 *  SYNOPSIS
 *
 *	static VOID fverify (blkp, fip)
 *	union blk *blkp;
 *	struct finfo *fip;
 *
 *  DESCRIPTION
 *
 *	Test to verify that archived file is same as existing file.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin fverify
 *	    If file is explicitly or implicitly specified then
 *		If processing confirmed then
 *		    Issue verbosity message
 *		    If the file exists then
 *			Initialize a file information structure
 *			If the file can be stat'ed then
 *			    Clear differences log
 *			    Verify the file header block info
 *			    If file is regular and not linked to another
 *				If file can be read then
 *				    Verify the file contents
 *				End if
 *			    End if
 *			    Report differences
 *			End if
 *		    End if
 *		End if
 *	   End if
 *	End fverify
 *
 */


static VOID fverify (blkp, fip)
union blk *blkp;
struct finfo *fip;
{
    struct finfo cfip;
    struct bstat bsbuf;

    DBUG_ENTER ("fverify");
    if (IS_LEAF (fip) || IS_EXTENSION (fip)) {
	if (confirmed ("d", fip)) {
	    verbosity (fip);
	    if (file_access (fip -> fname, A_EXISTS, TRUE)) {
		finfo_init (&cfip, fip -> fname, &bsbuf);
		if (fipstat (&cfip, TRUE)) {
		    diffs_found = 0;
		    hverify (fip, &cfip);
		    if (IS_REG (fip -> bstatp -> bst_mode) && !LINKED (blkp)) {
			if (file_access (fip -> fname, A_READ, TRUE)) {
			    cverify (fip, &cfip);
			}
		    }
		    diff_report (fip);
		}
	    }
	}
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	hverify    verify contents of archived file header block
 *
 *  SYNOPSIS
 *
 *	static VOID hverify (afip, cfip)
 *	struct finfo *afip;
 *	struct finfo *cfip;
 *
 *  DESCRIPTION
 *
 *	Given pointers to an archive file info structure (afip) and an
 *	existant file info structure (cfip), compares the information
 *	in the stat section of both.
 *
 *	If they are both symbolic links, make sure that they are both
 *	linked to the same pathname.
 *
 *	For various reasons, we no longer verify either the ctime or
 *	atime attributes.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin hverify
 *	    Get mode of archived file
 *	    Verify mode
 *	    Verify host device
 *	    If character or block special then
 *		Verify major/minor device numbers
 *	    End if
 *	    If not directory or -ddd[d] given then
 *		If a symbolic link then
 *		    Verify contents of symbolic link
 *		Else
 *		    Verify number of links
 *		Endif
 *	    End if
 *	    Verify user id
 *	    Verify group id
 *	    If is regular file then
 *		Verify size
 *		Verify access time
 *		Verify modification time
 *	    Else
 *		If not a directory then
 *		    Verify size
 *		End if
 *		Verify access time
 *		Verify modification time
 *	    End if
 *	End hverify
 *
 */


static VOID hverify (afip, cfip)
struct finfo *afip;
struct finfo *cfip;
{
    ushort amode;

    DBUG_ENTER ("hverify");
    amode = afip -> bstatp -> bst_mode;
    HVERIFY (bst_mode, D_MODE, 2);
    HVERIFY (bst_ino, D_INO, 4);
    HVERIFY (bst_dev, D_DEV, 4);
    if (IS_CSPEC (amode) || IS_BSPEC (amode)) {
	HVERIFY (bst_rdev, D_RDEV, 3);
    }
    if (!IS_DIR (amode) || flags.dflag > 2) {
	if (IS_FLNK (amode)) {
	    SVERIFY (afip, cfip);
	} else {
	    HVERIFY (bst_nlink, D_NLINK, 2);
	}
    }
    HVERIFY (bst_uid, D_UID, 2);
    HVERIFY (bst_gid, D_GID, 2);
    if (IS_REG (amode)) {
	HVERIFY (bst_size, D_SIZE, 1);
	/*** HVERIFY (bst_atime, D_ATIME, 3); ***/
	HVERIFY (bst_mtime, D_MTIME, 2);
	/*** HVERIFY (bst_ctime, D_CTIME, 4);  ***/
    } else {
	if (!IS_DIR (amode)) {
	    HVERIFY (bst_size, D_SIZE, 4);
	}
	/*** HVERIFY (bst_atime, D_ATIME, 4); ***/
	HVERIFY (bst_mtime, D_MTIME, 4);
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	cverify    verify contents of an archived file
 *
 *  SYNOPSIS
 *
 *	static VOID cverify (afip, cfip)
 *	struct finfo *afip;
 *	struct finfo *cfip;
 *
 *  DESCRIPTION
 *
 *	Given pointer to a file info structure for an archived file,
 *	and pointer to a file info structure for the current file,
 *	verifies that the archived file contents are the same
 *	as the current file contents.  Only indicates
 *	whether they are different when compared as a byte stream,
 *	not what the actual differences are.
 *
 *	Note that we have a couple of options for compressed files.
 *	In order to get a handle on the amount of work involved for
 *	each, assume the following definitions:
 *
 *		'r'	cost of reading an uncompressed file
 *		'R'	cost of reading a compressed file
 *		'w'	cost of writing an uncompressed file
 *		'W'	cost of writing a compressed file
 *		'z'	cost of decompressing a file
 *		'Z'	cost of compressing a file
 *		'X'	cost of reading a compressed file from archive
 *		'c'	cost of comparing two files
 *		
 *	Then the cost of reading an archived file into a temporary
 *	file, decompressing into a second temporary file, then 
 *	reading and comparing the two uncompressed files can be given
 *	as:
 *
 *		X + W + R + z + w + r + r + c
 *
 *	If we assume a compression ratio of 50%, then r=2R and w=2W,
 *	which gives a cost of:
 *
 *		cost1 = X + 3W + 5R + z + c
 *
 *	If instead, we compress the disk file into a temporary file,
 *	then compare that directly with the archived, compressed
 *	file, then the cost is:
 *
 *		cost2 = r + Z + W + R + X + c
 *		cost2 = 3R + Z + W + X + c
 *
 *	Now if we assume that z = Z, then after rearranging:
 *
 *		cost1 = X + 3W + 5R + z + c
 *		cost2 = X +  W + 3R + z + c
 *
 *	And thus the second method is "cheaper"  (whew!).
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin cverify
 *	    If files are not the same size then
 *		Not the same contents, report different
 *	    Else
 *		Open the current file
 *		If file open failed then
 *		    Notify user can't open file
 *		Else
 *		    Get number of bytes in archived file
 *		    While more bytes to read and compare
 *			If file read failed then
 *			    Tell user
 *			    Break compare loop
 *			End if
 *			Seek to next block of archive
 *			Read archive block
 *			If block is a full block then
 *			    Compare all bytes in block
 *			Else
 *			    Compare only part of block
 *			End if
 *			Reduce bytes left by block data size
 *			If bytes are different then
 *			    Tell user contents different
 *			    Break compare loop
 *			End if
 *		    End while
 *		    If file close fails then
 *		        Warn user about close error
 *		    End if
 *		End if
 *	    End if
 *	End cverify
 *
 */


static VOID cverify (afip, cfip)
struct finfo *afip;
struct finfo *cfip;
{
    S32BIT bytes;
    union blk *blkp;
    int count;
    char *fname;
    int fildes;
    char fbuf[DATASIZE];

    DBUG_ENTER ("cverify");
    DBUG_PRINT ("verify", ("verify contents of %s", afip -> fname));
    if (afip -> bstatp -> bst_size != cfip -> bstatp -> bst_size) {
	diffs_found |= D_CONTS;
    } else {
	GETPROT (cfip);
	FULLACCESS (cfip);
	cfip -> fildes = s_open (cfip -> fname, O_RDONLY, 0);
	if (cfip -> fildes == SYS_ERROR) {
	    bru_message (MSG_OPEN, cfip -> fname);
	} else {
	    bytes = ZSIZE (afip);
	    fname = cfip -> fname;
	    fildes = cfip -> fildes;
	    if (IS_COMPRESSED (afip)) {
		if (compressfip (cfip)) {
		    fname = cfip -> zfname;
		    fildes = cfip -> zfildes;
		}
	    }
	    while (bytes > 0L) {
		if (s_read (fildes, fbuf, DATASIZE) == SYS_ERROR) {
		    bru_message (MSG_READ, fname);
		    break;
		}
		blkp = ar_next ();
		ar_read (afip);
		if (bytes > DATASIZE) {
		    count = DATASIZE;
		} else {
		    count = (int) bytes;
		}
		bytes -= DATASIZE;
		if (bdiff (fbuf, blkp -> FD, count)) {
		    diffs_found |= D_CONTS;
		    break;
		}
	    }
	    if (s_close (cfip -> fildes) == SYS_ERROR) {
		bru_message (MSG_CLOSE, cfip -> fname);
	    }
	    SETOLDPROT (cfip);
	    discard_zfile (cfip);
	}
    }
    DBUG_VOID_RETURN;
}



/*
 *  FUNCTION
 *
 *	bdiff    compare specified number of bytes
 *
 *  SYNOPSIS
 *
 *	static BOOLEAN bdiff (cp1, cp2, count)
 *	char *cp1;
 *	char *cp2;
 *	int count;
 *
 *  DESCRIPTION
 *
 *	Compares buffers pointed to by cp1 and cp2, up to
 *	specified number of bytes.  Returns TRUE if any byte
 *	is different, FALSE otherwise.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin bdiff
 *	    If either pointer is junk or bad count
 *		Tell user about bug
 *	    Else
 *		Scan until difference found or end
 *	    End if
 *	    Blocks different if not at end
 *	    Return result
 *	End bdiff
 *
 */

static BOOLEAN bdiff (cp1, cp2, count)
char *cp1;
char *cp2;
int count;
{
    BOOLEAN result;

    DBUG_ENTER ("bdiff");
    if (cp1 == NULL || cp2 == NULL || count < 0) {
	bru_message (MSG_BUG, "bdiff");
    } else {
	while (*cp1++ == *cp2++ && count--) {;}
    }
    result = (count > 0);
    DBUG_RETURN (result);
}


static VOID diff_report (afip)
struct finfo *afip;
{
    struct diff *diffp;
    
    DBUG_ENTER ("diff_report");
    if (diffs_found != 0) {
	if (flags.vflag < 1) {
	    voutput ("\"%s\":", afip -> fname);
	    for (diffp = diffs; diffp -> short_name != NULL; diffp++) {
		if (diffp -> mask & diffs_found) {
		    voutput (" %s", diffp -> short_name);
		}
	    }
	    vflush ();
	} else {
	    for (diffp = diffs; diffp -> long_name != NULL; diffp++) {
		if (diffp -> mask & diffs_found) {
		    voutput ("\"%s\": %s different",
			     afip -> fname, diffp -> long_name);
		    vflush ();
		}
	    }
	}
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	sverify    verify contents of a symbolic link
 *
 *  SYNOPSIS
 *
 *	static VOID sverify (afip, cfip)
 *	struct finfo *afip;
 *	struct finfo *cfip;
 *
 *  DESCRIPTION
 *
 *	Verify contents of a symbolic link.
 *
 */

#if HAVE_SYMLINKS

static VOID sverify (afip, cfip)
struct finfo *afip;
struct finfo *cfip;
{
    char linkbuf[BRUPATHMAX];
    int nm;

    DBUG_ENTER ("sverify");
    nm = s_readlink (cfip -> fname, linkbuf, sizeof (linkbuf));
    if (nm == SYS_ERROR) {
	bru_message (MSG_RDLINK, cfip -> fname);
    } else {
	linkbuf[nm] = EOS;
	if (!STRSAME (linkbuf, afip -> lname) && (flags.dflag >= 2)) {
	    diffs_found |= D_SYMLK;
	    /* the HVERIFY macro doesn't cut it here */
	}
    }
    DBUG_VOID_RETURN;
}

#else

static VOID sverify (afip)
struct finfo *afip;
{
    DBUG_ENTER ("sverify");
    if (flags.dflag >= 2) {
	bru_message (MSG_NOSYMLINKS, afip -> fname);
    }
    DBUG_VOID_RETURN;
}

#endif
