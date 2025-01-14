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
 *	headers.c    routines to manipulate header blocks
 *
 *  SCCS
 *
 *	@(#)headers.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	Contains routines which are primarily used to manipulate
 *	archive header blocks.
 *
 */

#include "globals.h"


/*
 *  NAME
 *
 *	hcheck    perform sanity checks on header block
 *
 *  SYNOPSIS
 *
 *	BOOLEAN hcheck (blkp)
 *	union blk *blkp;
 *
 *  DESCRIPTION
 *
 *	Given pointer to a presumed header block, tests
 *	to see if the block passes various sanity checks.
 *
 *	Currently only checks for proper checksum and magic number.
 *	Should eventually perform pattern type checks also.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin hcheck
 *	    Default result is FALSE
 *	    If the checksum is ok then
 *		If the magic number is ok then
 *		    Result is TRUE
 *		End if
 *	    End if
 *	    Return result
 *	End hcheck
 *
 */

BOOLEAN hcheck (blkp)
union blk *blkp;
{
    BOOLEAN result;		/* Result of test */

    DBUG_ENTER ("hcheck");
    result = FALSE;
    if (chksum_ok (blkp)) {
	DBUG_PRINT ("sanity", ("header checksum ok"));
	if (FROMHEX (blkp -> HD.h_magic) == H_MAGIC) {
	    DBUG_PRINT ("sanity", ("magic number ok"));
	    result = TRUE;
	}
    }
    DBUG_RETURN (result);
}



/*
 *  NAME
 *
 *	hstat    decode header block into a stat buffer
 *
 *  SYNOPSIS
 *
 *	VOID hstat (blkp, fip)
 *	union blk *blkp;
 *	struct finfo *fip;
 *
 *  DESCRIPTION
 *
 *	Decodes the contents of a header block, pointed to by
 *	"blkp", into a "stat" buffer.
 *
 *	Also pulls out a couple of other finfo fields while we
 *	are at it.
 */

VOID hstat (blkp, fip)
union blk *blkp;
struct finfo *fip;
{
    int start = 0;

    DBUG_ENTER ("hstat");
    fip -> bstatp -> bst_mode = (ushort) FROMHEX (blkp -> FH.f_mode);
    fip -> bstatp -> bst_ino = (ino_t) FROMHEX (blkp -> FH.f_ino);
    fip -> bstatp -> bst_dev = (dev_t) FROMHEX (blkp -> FH.f_dev);
    fip -> bstatp -> bst_rdev = (dev_t) FROMHEX (blkp -> FH.f_rdev);
    fip -> bstatp -> bst_nlink = (short) FROMHEX (blkp -> FH.f_nlink);
    fip -> bstatp -> bst_uid = (ushort) FROMHEX (blkp -> FH.f_uid);
    fip -> bstatp -> bst_gid = (ushort) FROMHEX (blkp -> FH.f_gid);
    fip -> bstatp -> bst_atime = (time_t) FROMHEX (blkp -> FH.f_atime);
    fip -> bstatp -> bst_mtime = (time_t) FROMHEX (blkp -> FH.f_mtime);
    fip -> bstatp -> bst_ctime = (time_t) FROMHEX (blkp -> FH.f_ctime);
    fip -> fi_flags = (int) FROMHEX (blkp -> FH.f_flags);
    fip -> fib_Protection = (long) FROMHEX (blkp -> FH.f_fibprot);
    (VOID) s_strcpy (fip -> fib_Comment, blkp -> FH.f_fibcomm);
    if (IS_COMPRESSED (fip)) {
	DBUG_PRINT ("hstat", ("found a compressed file"));
	fip -> bstatp -> bst_size = (off_t) FROMHEX (blkp -> FH.f_xsize);
	fip -> zsize = (off_t) FROMHEX (blkp -> FH.f_size);
	nzbits = (UINT) FROMHEX (blkp -> FH.f_nzbits);
    } else {
	fip -> bstatp -> bst_size = (off_t) FROMHEX (blkp -> FH.f_size);
    }
    if (fip -> fi_flags & FI_XFNAME) {
	(VOID) s_strcpy (fip -> fname, blkp -> FH.f_xname);
	DBUG_PRINT ("hstat", ("extended pathname %s", fip -> fname));
    } else {
	(VOID) s_strcpy (fip -> fname, blkp -> HD.h_name);
	DBUG_PRINT ("hstat", ("regular pathname %s", fip -> fname));
    }
    if (fip -> fi_flags & FI_XLNAME) {
	if (fip -> fi_flags & FI_XFNAME) {
	    start = s_strlen (blkp -> FH.f_xname) + 1;
	}
	DBUG_PRINT ("hstat", ("linkname extension offset %d", start));
	(VOID) s_strcpy (fip -> lname, blkp -> FH.f_xname + start);
	DBUG_PRINT ("hstat", ("extended linkname %s", fip -> lname));
    } else {
	(VOID) s_strcpy (fip -> lname, blkp -> FH.f_lname);
	DBUG_PRINT ("hstat", ("regular linkname %s", fip -> lname));
    }
    if (IS_COMPRESSED (fip)) {
	stripz (fip -> fname);
	stripz (fip -> lname);
    }
    DBUG_VOID_RETURN;
}

/*
 *  FUNCTION
 *
 *	namesfit    allocate the filename space in file header block
 *
 *  DESCRIPTION
 *
 *	Given a file info pointer that points to the full pathname of the
 *	file to be archived (hereafter refered to as "fname"), and also
 *	points to the full pathname of any file it might be linked to
 *	(hereafter refered to as "lname"), decide if there is sufficient
 *	space in the file header block to store all the name information.
 *	If so, initialize the pointers to the name fragments that will go
 *	in each storage area.  Note that the actual storage for these
 *	fragments is statically allocated and overwritten on each call,
 *	but that's ok since we only do one file at a time anyway.
 *
 *	There are basically three areas of the block which require file
 *	name information:
 *
 *		h_name[NAMESIZE]	in the block header, used strictly
 *					for saving all or part of fname
 *
 *		f_lname[NAMESIZE]	in the file header block, used
 *					strictly for saving all or part of
 *					lname
 *
 *		f_xname[XNAMESIZE]	in the file header block, used to
 *					store full fname or lname when they
 *					do not fit in the primary storage
 *					areas
 *
 *	The goals of the name space allocation are:
 *
 *	(1)	Allocate as much of each name as will fit into the h_name
 *		and f_lname arrays as possible, for some amount of
 *		backwards compatibility.  Names that totally fit will
 *		be 100% backwards compatible.  Names that overflow will
 *		be somewhat backwards compatible as described below
 *		(though obviously portions of the path will be lost).
 *
 *	(2)	When a name overflows the primary region, store an
 *		abbreviated version of it in the primary region and the
 *		full version of it in the extension area.  The abbreviated
 *		version is formed by using as much of the tail end of
 *		the pathname as possible.  Try to split paths at '/' and if
 *		there is room, prepend a fixed name (such as "brutmp/") to
 *		the abbreviated version placed in the primary region, so
 *		that older versions will extract the files into a directory
 *		under a known location, when the leading portions of the
 *		pathname are lost.
 *
 *	(3)	When there is not room to store all of both fname and
 *		lname, discard lname, thus severing the link in the archive.
 *		It is better to have two copies in the archive, with the
 *		link severed, than to have a link with some pathname info
 *		lost.
 *		
 *
 *	The basic algorithm is something like this:
 *
 *	(1)	If fname fits in it's primary region, then initialize
 *		the copy of the primary region and set the pointers
 *		and flag bits appropriately.
 *
 *	(2)	If step (1) fails, then initialize the copy of the extension
 *		area (the name is guaranteed to fit due to earlier checks)
 *		and set the pointers and flag bits appropriately.
 *
 *	(3)	Initialize the copy of the primary region with an abbreviated
 *		form of the name, formed by the longest substring consisting
 *		of the tail of the name, split at a '/', with the prefix
 *		added.  If there is no such '/', then split the name at an
 *		arbitrary location such the that sum of the prefix and the
 *		tail exactly fills the primary region.
 *
 *	(4)	Repeat steps (1) - (3) for lname, except that (2) can now
 *		fail if part of the extension area is already allocated
 *		to fname.  In such cases discard lname, which breaks the
 *		link and causes the entire file to be archived.  Change
 *		the various bits and pieces in the file info block as
 *		appropriate.
 *
 */

BOOLEAN namesfit (fip)
struct finfo *fip;
{
    BOOLEAN result = TRUE;
    int fsz;					/* Total size of fname+null */
    int lsz;					/* Total size of lname+null */
    char *scan;
    static char hnamebuf[NAMESIZE];		/* Null terminated */
    static char lnamebuf[NAMESIZE];		/* Null terminated */
    static char xnamebuf[XNAMESIZE];		/* Null terminated */

    DBUG_ENTER ("namesfit");
    (VOID) s_memset (xnamebuf, 0, sizeof (xnamebuf));
    fsz = s_strlen (fip -> fname) + 1;
    DBUG_PRINT ("fsz", ("file name size (incl null) %d", fsz));
    if (IS_COMPRESSED (fip)) {
	fsz += 2;
	DBUG_PRINT ("fsz", ("compressed file name size (incl null) %d", fsz));
    }
    if (fsz <= NAMESIZE) {
	DBUG_PRINT ("fname", ("allocate name to primary storage"));
	copyname (hnamebuf, fip -> fname);
	fsz = 0;
    } else {
	DBUG_PRINT ("fname", ("allocate name to secondary storage"));
	copyname (xnamebuf, fip -> fname);
	if (IS_COMPRESSED (fip)) {
	    addz (xnamebuf);
	}
	fip -> bname3 = xnamebuf;
	DBUG_PRINT ("bname3", ("%s", fip -> bname3));
	fip -> fi_flags |= FI_XFNAME;
	scan = fip -> fname + fsz - NAMESIZE + PREFSIZE - 1;
	while (*scan != '/' && *scan != '\000') {
	    scan++;
	}
	if (*scan == '/') {
	    DBUG_PRINT ("fname", ("found room for prefix"));
	    (VOID) s_strcpy (hnamebuf, FNAMEPREF);
	    (VOID) s_strcpy (hnamebuf + PREFSIZE, ++scan);
	} else {
	    DBUG_PRINT ("fname", ("no room for prefix, use maximal string"));
	    (VOID) s_strcpy (hnamebuf, fip -> fname + fsz - NAMESIZE);
	}
    }
    if (IS_COMPRESSED (fip)) {
	addz (hnamebuf);
    }
    fip -> bname1 = hnamebuf;
    DBUG_PRINT ("bname1", ("%s" , fip -> bname1));
    if (fip -> lname) {
	lsz = s_strlen (fip -> lname) + 1;
	DBUG_PRINT ("lsz", ("linked name size (incl null) %d", lsz));
	if (IS_COMPRESSED (fip)) {
	    lsz += 2;
	    DBUG_PRINT ("lsz", ("compressed linked name size %d", lsz));
	}
	if (lsz <= NAMESIZE) {
	    DBUG_PRINT ("lname", ("lname allocated to primary storage"));
	    copyname (lnamebuf, fip -> lname);
	    if (IS_COMPRESSED (fip)) {
		addz (lnamebuf);
	    }
	    fip -> bname2 = lnamebuf;
	} else if ((lsz + fsz) > XNAMESIZE) {
	    DBUG_PRINT ("lname", ("lname overflows extension area"));
	    bru_message (MSG_LBROKEN, fip -> fname, fip -> lname);
	    fip -> lname = NULL;
	    if (IS_FLNK (fip -> bstatp -> bst_mode)) {
		DBUG_PRINT ("lname", ("was symbolic link, lost it"));
		result = FALSE;
	    }
	} else {
	    scan = xnamebuf;
	    if (fsz > 0) {
		scan += fsz;
	    }
	    (VOID) s_strcpy (scan, fip -> lname);
	    if (IS_COMPRESSED (fip)) {
		addz (scan);
	    }
	    DBUG_PRINT ("lname", ("%s", scan));
	    scan = fip -> lname + lsz - NAMESIZE + PREFSIZE - 1;
	    while (*scan != '/' && *scan != '\000') {
		scan++;
	    }
	    if (*scan == '/') {
		DBUG_PRINT ("lname", ("found room for prefix"));
		(VOID) s_strcpy (lnamebuf, FNAMEPREF);
		(VOID) s_strcpy (lnamebuf + PREFSIZE, ++scan);
	    } else {
		DBUG_PRINT ("lname", ("no room for prefix, use max string"));
		(VOID) s_strcpy (lnamebuf, fip -> lname + lsz - NAMESIZE);
	    }
	    if (IS_COMPRESSED (fip)) {
		addz (lnamebuf);
	    }
	    fip -> bname2 = lnamebuf;
	    fip -> bname3 = xnamebuf;
	    fip -> fi_flags |= FI_XLNAME;
	}
	DBUG_PRINT ("bname2", ("%s", fip -> bname2));
    }
    DBUG_RETURN (result);
}

