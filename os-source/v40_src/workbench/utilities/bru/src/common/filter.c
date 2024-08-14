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
 *	filter.c    functions for supporting filter mode
 *
 *  SCCS
 *
 *	@(#)filter.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	When filter mode is enabled, bru acts more like a normal
 *	Unix style filter program.  Rather than first collecting
 *	all the pathnames and building a possibly huge internal
 *	directory tree, bru acts on each pathname before proceeding
 *	on to the next.
 *
 *	Filter mode has several advantages.  First of all, it avoids
 *	building the internal file tree, thus saving lots of memory
 *	and possibly even allowing it to run when it otherwise would
 *	run out of available memory.  In addition, if the pathnames 
 *	are being supplied by a program like "find", it can take a
 *	LONG time to acquire all the pathnames if a tree is built
 *	first.  Avoiding this allows bru to start processing as
 *	soon as the first pathname is available.  An additional
 *	advantage to filter mode is that it makes it easier to use
 *	bru under the control of another program, like a window
 *	system interface.
 *
 *	Filter mode also has several disadvantages.  First of all,
 *	the pathnames must be supplied in exactly the order desired.
 *	This is not critical during archive creation; but during any
 *	mode where an archive is being read, the order must match
 *	the order in the existing archive for correct results.
 *	Another disadvantage is that filter mode effectively turns
 *	off automatic archiving of parent directories and automatic
 *	expansion of directories, but for most uses, this is the
 *	desired behavior anyway.
 *
 *
 */

#include "globals.h"


/*
 *  FUNCTION
 *
 *	filter    call a specified function for each input pathname
 *
 *  SYNOPSIS
 *
 *	VOID filter (funcp)
 *	VOID (*funcp)();
 *
 *  DESCRIPTION
 *
 *	Given a pointer to a function to execute, acquire each pathname
 *	from the input list, build a file information structure for it,
 *	and then call the function with a pointer to the the file
 *	information structure.
 *
 */

VOID filter (funcp)
VOID (*funcp) PROTO((struct finfo *fip));
{
    auto struct finfo file;
    auto struct bstat bsbuf;
    auto char namebuf[BRUPATHMAX];

    DBUG_ENTER ("filter");
    while (nextname (namebuf, sizeof (namebuf))) {
	finfo_init (&file, namebuf, &bsbuf);
	if (file_access (file.fname, A_READ, TRUE)) {
	    if (fipstat (&file, TRUE)) {
		(*funcp) (&file);
	    }
	}
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	nextname    get the next pathname from input list
 *
 *  SYNOPSIS
 *
 *	static BOOLEAN nextname (bufp, bufsize)
 *	char *bufp;
 *	unsigned int bufsize;
 *
 *  DESCRIPTION
 *
 *	Get the next pathname from the input list.  This list may
 *	be coming from the standard input stream, or may be coming
 *	from some sort of ipc channel, such as an ipc port on
 *	the Amiga version.
 *
 *	Enforce the limit of bufsize characters (including a
 *	null byte termination).  Ignore lines that consist of nothing
 *	but a newline character.  In order to get a valid name, the
 *	name must fit in the buffer and must be terminated in the
 *	input stream with either a newline character or EOF.  I.E.
 *	the last line is allowed to not be newline terminated and 
 *	things should still work, provided getchar continues to return
 *	EOF once it hits the end of the input stream.
 *
 */

BOOLEAN nextname (bufp, bufsize)
char *bufp;
unsigned int bufsize;
{
    BOOLEAN result = FALSE;
    int ch;
    char *bp;

    DBUG_ENTER ("nextname");
    if (flags.Afflag) {
	result = ipc_getfilename (bufp);
    } else {
	do {
	    DBUG_PRINT ("paths", ("collect next line for possible name"));
	    bp = bufp;
	    while (bp < bufp + bufsize) {
		ch = getchar ();
		if (ch == '\n' || ch == EOF) {
		    *bp = EOS;
		    result = (bp > bufp);
		    break;
		} else {
		    *bp++ = (char) ch;
		}
	    }
	    if (bp == bufp + bufsize) {
		*(bp - 1) = EOS;
		bru_message (MSG_BRUPATHMAX, bufp, bufsize - 1);
		DBUG_PRINT ("paths", ("eat everything to newline or EOF"));
		do {
		    ch = getchar ();
		} while (ch != '\n' && ch != EOF);
	    }
	} while (ch != EOF && result == FALSE);
    }
    DBUG_RETURN (result);
}
