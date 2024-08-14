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
 *	table.c    functions for doing table of contents
 *
 *  SCCS
 *
 *	@(#)table.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	Contains those functions which are solely for doing the
 *	table of contents of a bru archive.
 *
 */

#include "globals.h"

/*
 *	Local functions.
 */

static VOID tfile PROTO((union blk *blkp, struct finfo *fip));
static VOID pentry PROTO((union blk *blkp, struct finfo *fip));


/*
 *  NAME
 *
 *	table    main entry point for table of contents
 *
 *  SYNOPSIS
 *
 *	VOID table ()
 *
 *  DESCRIPTION
 *
 *	Performs table of contents for the selected archive.
 *	This routine is called after all initialization has been
 *	performed.  It sets the current mode to 't', for table,
 *	opens the archive, scans the archive printing table
 *	of contents, closes the archive, and returns to
 *	the startup code.
 *
 */

VOID table ()
{
    DBUG_ENTER ("table");
    mode = 't';
    reload ("table of contents");
    ar_open ();
    scan (tfile);
    ar_close ();
    DBUG_VOID_RETURN;
}


/*
 *  NAME
 *
 *	tfile    print contents of stat buffer
 *
 *  SYNOPSIS
 *
 *	static VOID tfile (blkp, fip)
 *	union blk *blkp;
 *	struct finfo *fip;
 *
 *  DESCRIPTION
 *
 *	Prints a header entry using contents of stat buffer
 *	and header block.
 *
 *	Note that the confirmation flag usage doesn't really provide
 *	any useful protection here.  It is used simply for completeness,
 *	since every other mode which accesses an archive also uses it.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin tfile
 *	    If explicitly or implicitly named then
 *		If action confirmed by user then
 *		    Print this entry
 *		End if
 *	    End if
 *	End tfile
 *
 */

static VOID tfile (blkp, fip)
union blk *blkp;
struct finfo *fip;
{
    DBUG_ENTER ("tfile");
    if (IS_LEAF (fip) || IS_EXTENSION (fip)) {
	if (confirmed ("t", fip)) {
	    pentry (blkp, fip);
	}
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	pentry    print archive entry information
 *
 *  SYNOPSIS
 *
 *	static VOID pentry (blkp, fip)
 *	union blk *blkp;
 *	struct finfo *fip;
 *
 *  DESCRIPTION
 *
 *	Once the decision has been made to print entry information
 *	for the current archive file, this routine is called to
 *	do the actual printing.
 *
 *	If the file was archived in compressed form, and the
 *	command line flag for compression is given, the file
 *	is listed in it's compressed form (name and size) rather
 *	than the original uncompressed form.
 *
 *  NOTES
 *
 *	This routine once used a line of the form:
 *
 *		fprintf (logfp, fip -> fname);
 *
 *	to print the file name, resulting in an obscure bug when the
 *	file name contained printf control characters (consider
 *	"/tmp/funny%sname").  The moral is, if the string being printed
 *	comes from external input, never pass it directly to print for
 *	printing.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin pentry
 *	    If verbose form of table of contents then
 *		Get the mode of the file
 *		Tranlate mode from internal format to string
 *		Get invocation time structure
 *		Remember the year of current bru invocation
 *		Get file modification time structure
 *		Tranlate modification time to string
 *		Print the mode string
 *		Print number of links to the file
 *		Translate the uid and print result
 *		Translate the gid and print result
 *		If file is block or character special file then
 *		    Get major/minor device numbers
 *		    Print major/minor device numbers
 *		Else
 *		    Print the file size
 *		End if
 *		Print month and day of modification
 *		If modified this year then
 *		    Print time of modification
 *		Else
 *		    Print year of modification
 *		End if
 *	    End if
 *	    Print the file name
 *	    If verbose listing and linked to another file then
 *		Print name of file linked to
 *	    End if
 *	    If verbose listing and archived on an Amiga then
 *		If file had a filenote attached then
 *		    Print the filenote in parenthesis
 *		Endif
 *	    Endif
 *	    Terminate the line
 *	End pentry
 *
 */


static VOID pentry (blkp, fip)
union blk *blkp;
struct finfo *fip;
{
    char modestr[MODESZ+1];		/* Pointer to mode string */
    char *mtime;			/* Pointer to time string */
    int year_now;			/* Current year */
    int fst_mode;			/* Mode of file */
    dev_t rdev;				/* Major/minor device no */
    struct tm *tmp;			/* Pointer to tm struct */
    char *name;				/* Temporary pointer */

    DBUG_ENTER ("pentry");
    if (flags.vflag > 0) {
	fst_mode = fip -> bstatp -> bst_mode;
	filemode (modestr, fst_mode);
	tmp = s_localtime (&info.bru_time);
	year_now = tmp -> tm_year;
	tmp = s_localtime ((long *) &fip -> bstatp -> bst_mtime);
	mtime = s_asctime (tmp);
	voutput (modestr);
	voutput ("%4d", fip -> bstatp -> bst_nlink);
	name = ur_gname ((USHORT)(fip -> bstatp -> bst_uid));
	voutput (" %-9.8s", name);
	name = gp_gname ((USHORT)(fip -> bstatp -> bst_gid));
	voutput ("%-9.8s", name);
	if (IS_BSPEC (fst_mode) || IS_CSPEC (fst_mode)) {
	    rdev = fip -> bstatp -> bst_rdev;
	    voutput ("%3d,%3d", (rdev & 0xFF00) >> 8, rdev & 0xFF);
	} else {
	    if ((flags.Zflag & (Z_ON | Z_AUTO)) && IS_COMPRESSED (fip)) {
		voutput ("%7ld", fip -> zsize);
	    } else {
		voutput ("%7ld", fip -> bstatp -> bst_size);
	    }
	}
	voutput (" %-6.6s ", &mtime[4]);
	DBUG_PRINT ("year", ("current year = %d", year_now));
	DBUG_PRINT ("year", ("file year = %d", tmp -> tm_year));
	if (tmp -> tm_year == year_now) {
	    voutput ("%5.5s ", &mtime[11]);
	} else {
	    voutput (" %4.4s ", &mtime[20]);
	}
    }
    if ((flags.Zflag & (Z_ON | Z_AUTO)) && IS_COMPRESSED (fip)) {
	voutput ("%s.Z", fip -> fname);
    } else {
	voutput ("%s", fip -> fname);
    }
    if (flags.vflag > 1 && IS_FLNK (fip -> bstatp -> bst_mode)) {
	voutput (" symbolic link to %s", namelink (fip -> lname));
    } else if (flags.vflag > 1 && LINKED (blkp)) {
	voutput (" linked to %s", fip -> lname);
    }
    if ((flags.vflag > 1) && (fip -> fi_flags & FI_AMIGA)) {
	if (fip -> fib_Comment[0] != EOS) {
	    voutput (" (%s)", fip -> fib_Comment);
	}
    }
    vflush ();
    DBUG_VOID_RETURN;
}
