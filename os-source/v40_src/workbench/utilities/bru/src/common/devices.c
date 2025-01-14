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
 *	devices.c    routines for manipulating information about known devices
 *
 *  SCCS
 *
 *	@(#)devices.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	When bru is configured for a given environment, one of the
 *	things that must be set up is a table of known devices, to
 *	help in error recovery.  Initial versions of bru had this table
 *	compiled in and required source code changes to change the
 *	known devices.  This scheme was quickly superceded by an
 *	external table in human readable form, generally in /etc/brutab.
 *	The first form of this table had records with a fixed number of
 *	fields.  This presented various problems, leading to the current
 *	implementation which is very much like termcap.
 *
 *	Routines in this module are responsible for maintaining and
 *	examining the internal format of one entry in the device
 *	table.  The external representation and manipulation is
 *	performed by routines in the brutab.c module.
 *
 */
 
#include "globals.h"

/*
 *	Cycling through multiple archive devices is supported by maintaining
 *	a linked list of device names as specified by the -f command line
 *	option.  Whenever a new media is requested, this list is tested
 *	to see if there is another entry after the current entry.  If so,
 *	then this next entry is used to automatically satisfy the request
 *	for a new volume.  If not, then the user is prompted, as before.
 *
 */

struct devnode {
    struct devnode *devnext;
    char *devname;
};

static struct devnode *devfirst = NULL;
static struct devnode *devcurrent = NULL;


/*
 *  FUNCTION
 *
 *	possible_end    test for likely end of device
 *
 *  SYNOPSIS
 *
 *	BOOLEAN possible_end (iobytes, ioerr, read)
 *	int iobytes;
 *	int ioerr;
 *	BOOLEAN read;
 *
 *  DESCRIPTION
 *
 *	Given number of bytes transfered in last I/O operation,
 *	system error number returned, and a flag indicating if
 *	the error occured on a read, determines if it was likely
 *	that the end of the device was found.  This test is
 *	not always conclusive.  Sometimes unix hides too much
 *	information!.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin possible_end
 *	    Default is FALSE
 *	    If the operation returned a system error then
 *		If reading and zero read io error matches then
 *		    Result is TRUE
 *		Else if not reading and zero write error matches then
 *		    Result is TRUE
 *		End if
 *	    Else 
 *		If reading and partial read error matches then
 *		    Result is TRUE
 *		Else if not reading and partial write error matches then
 *		    Result is TRUE
 *		End if
 *	    End if
 *	    Return result
 *	End possible_end
 *
 */

BOOLEAN possible_end (iobytes, ioerr, read)
int iobytes;
int ioerr;
BOOLEAN read;
{
    BOOLEAN end;

    DBUG_ENTER ("possible_end");
    DBUG_PRINT ("eod", ("ioerr %d", ioerr));
    end = FALSE;
    if (ardp != NULL) {
        if (iobytes == -1) {
	    if (read && ardp -> dv_zrerr == ioerr) {
		end = TRUE;
	    } else if (!read && ardp -> dv_zwerr == ioerr) {
		end = TRUE;
	    }
	} else {
	    if (read && ardp -> dv_prerr == ioerr) {
		end = TRUE;
	    } else if (!read && ardp -> dv_pwerr == ioerr) {
		end = TRUE;
	    }
	}
    }	
    DBUG_PRINT ("eod", ("end of device flag %d", end));
    DBUG_RETURN (end);
}


/*
 *  FUNCTION
 *
 *	known_end    test for known end of device
 *
 *  SYNOPSIS
 *
 *	BOOLEAN known_end (iobytes, ioerr)
 *	int iobytes;
 *	int ioerr;
 *
 *  DESCRIPTION
 *
 *	Given number of bytes transfered in last I/O operation and
 *	system error number returned, determines if the end of the
 *	device was reached.  This depends upon "ederr" in the brutab
 *	entry being reliable and distinct from all other possible
 *	error returns.
 *
 */

BOOLEAN known_end (iobytes, ioerr)
int iobytes;
int ioerr;
{
    BOOLEAN end = FALSE;

    DBUG_ENTER ("known_end");
    DBUG_PRINT ("eod", ("ioerr %d", ioerr));
    if (iobytes < 0 && ardp != NULL) {
	if (ardp -> dv_ederr != 0 && ardp -> dv_ederr == ioerr) {
	    end = TRUE;
	}
    }
    DBUG_PRINT ("eod", ("known end of device flag %d", end));
    DBUG_RETURN (end);
}


/*
 *  FUNCTION
 *
 *	unformatted    test for error due to media unformatted
 *
 *  SYNOPSIS
 *
 *	BOOLEAN unformatted (iobytes, ioerr, read)
 *	int iobytes;
 *	int ioerr;
 *	BOOLEAN read;
 *
 *  DESCRIPTION
 *
 *	Uses information about number of bytes transferred on
 *	last I/O operation, error returned by operation, and
 *	whether operation was read or write, to determine if
 *	the media was unformatted.  As with the end of
 *	device test, this test may not always be accurate.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin unformatted
 *	    Default is FALSE
 *	    If operation returned error then
 *		If reading then
 *		    Get unformatted read error expected
 *		Else
 *		    Get unformatted write error expected
 *		End if
 *		If error was the expected error then
 *		    Result is TRUE
 *		End if
 *	    End if
 *	    Return result
 *	End unformatted
 *
 */

BOOLEAN unformatted (iobytes, ioerr, read)
int iobytes;
int ioerr;
BOOLEAN read;
{
    BOOLEAN nofmt;
    int err;

    DBUG_ENTER ("unformatted");
    DBUG_PRINT ("fmt", ("ioerr %d", ioerr));
    nofmt = FALSE;
    if (iobytes <= 0 && ardp != NULL) {
	if (read) {
	    err = ardp -> dv_frerr;
	} else {
	    err = ardp -> dv_fwerr;
	}
	if (err == ioerr) {
	    nofmt = TRUE;
	}
    }	
    DBUG_PRINT ("fmt", ("unformatted flag %d", nofmt));
    DBUG_RETURN (nofmt);
}


/*
 *  FUNCTION
 *
 *	write_protect    test for error due to media write_protect
 *
 *  SYNOPSIS
 *
 *	BOOLEAN write_protect (iobytes, ioerr, read)
 *	int iobytes;
 *	int ioerr;
 *	BOOLEAN read;
 *
 *  DESCRIPTION
 *
 *	Uses information about number of bytes transferred on
 *	last I/O operation, error returned by operation, and
 *	whether operation was read or write, to determine if
 *	the media was write_protected.  As with the end of
 *	device test, this test may not always be accurate.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin write_protect
 *	    Default is FALSE
 *	    If operation returned error then
 *		If not reading then
 *		    Get write_protected error expected
 *		    If error was the expected error then
 *		        Result is TRUE
 *		    End if
 *		End if
 *	    End if
 *	    Return result
 *	End write_protect
 *
 */

BOOLEAN write_protect (iobytes, ioerr, read)
int iobytes;
int ioerr;
BOOLEAN read;
{
    BOOLEAN wprot;
    int err;

    DBUG_ENTER ("write_protect");
    DBUG_PRINT ("wprot", ("ioerr %d", ioerr));
    wprot = FALSE;
    if (iobytes <= 0 && ardp != NULL) {
	if (!read) {
	    err = ardp -> dv_wperr;
	    if (err == ioerr) {
	        wprot = TRUE;
	    }
	}
    }	
    DBUG_PRINT ("wprot", ("write_protected flag %d", wprot));
    DBUG_RETURN (wprot);
}


/*
 *  FUNCTION
 *
 *	seekable    test to see if file is seekable
 *
 *  SYNOPSIS
 *
 *	BOOLEAN seekable (fname, increment)
 *	char *fname;
 *	int increment;
 *
 *  DESCRIPTION
 *
 *	Tests to see if the specified file is seekable to at least
 *	the given increment.  If so, then seeks may be used instead
 *	of reads, to skip over blocks of the archive.  This speeds
 *	things up considerably.
 *
 *	If the device is unknown, then it is also checked to see if
 *	it is an existing normal (regular) disk file.  If so, then it is
 *	seekable by default.
 *
 *	Returns TRUE if seekable to the given increment.
 *
 */

BOOLEAN seekable (fname, increment)
char *fname;
int increment;
{
    struct bstat bsbuf;
    BOOLEAN seekok;

    DBUG_ENTER ("seekable");
    DBUG_PRINT ("seek", ("check file '%s' for seekability", fname));
    seekok = FALSE;
    if (ardp != NULL) {
	if (ardp -> dv_seek > 0 && ardp -> dv_seek <= increment) {
	    seekok = TRUE;
	}
    } else {
	seekok = bstat (fname, &bsbuf, FALSE) && IS_REG (bsbuf.bst_mode);
    }
    DBUG_PRINT ("seek", ("seekable returns logical value %d", seekok));
    DBUG_RETURN (seekok);
}


/*
 *  FUNCTION
 *
 *	raw_tape    test for raw tape as device
 *
 *  SYNOPSIS
 *
 *	BOOLEAN raw_tape ()
 *
 *  DESCRIPTION
 *
 *	Uses information from the archive device table to
 *	determine if the current archive device is a raw
 *	magnetic tape drive.
 *
 */

BOOLEAN raw_tape ()
{
    BOOLEAN raw;

    DBUG_ENTER ("raw_tape");
    if (ardp != NULL && (ardp -> dv_flags & D_RAWTAPE)) {
	raw = TRUE;
    } else {
	raw = FALSE;
    }	
    DBUG_PRINT ("raw", ("raw_tape flag %d", raw));
    DBUG_RETURN (raw);
}


/*
 *  FUNCTION
 *
 *	savedev    save a name as an archive device name
 *
 *  SYNOPSIS
 *
 *	VOID savedev (name)
 *	char *name;
 *
 *  DESCRIPTION
 *
 *	Adds the given name to the list of devices to cycle through when
 *	switching media.  Each new name is added to the end of the list.
 *
 */

VOID savedev (name)
char *name;
{
    struct devnode **scan;
    struct devnode *dvp;

    DBUG_ENTER ("savedev");
    DBUG_PRINT ("devname", ("add '%s' to the list of device names", name));
    for (scan = &devfirst; *scan != NULL; scan = &((*scan) -> devnext)) {;}
    dvp = (struct devnode *) get_memory (sizeof (struct devnode), TRUE);
    if ((dvp -> devname = s_strdup (name)) == NULL) {
	bru_message (MSG_MALLOC, s_strlen (name) + 1);
	s_free ((char *) dvp);
    } else {
	dvp -> devnext = NULL;
	*scan = dvp;
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	firstdev    return name of first device on list
 *
 *  SYNOPSIS
 *
 *	char *firstdev ()
 *
 *  DESCRIPTION
 *
 *	Make the first device on the list the current device, and
 *	return pointer to it's name.
 *
 */

char *firstdev ()
{
    char *namep = NULL;

    DBUG_ENTER ("firstdev");
    devcurrent = devfirst;
    if (devcurrent != NULL) {
	namep = devcurrent -> devname;
	DBUG_PRINT ("fdev", ("first device on list is '%s'", namep));
    }
    DBUG_RETURN (namep);
}


char *nextdev ()
{
    char *namep;

    DBUG_ENTER ("nextdev");
    if (devcurrent == NULL) {
	devcurrent = devfirst;
    } else {
	devcurrent = devcurrent -> devnext;
    }
    if (devcurrent == NULL) {
	namep = NULL;
	DBUG_PRINT ("ndev", ("no next device, recycle back to first"));
    } else {
	namep = devcurrent -> devname;
	DBUG_PRINT ("ndev", ("next device on list is '%s'", namep));
    }
    DBUG_RETURN (namep);
}


/*
 *	Test to see if the current archive device is flagged as using
 *	ejectable media (ala the MacIntosh).
 */

BOOLEAN ejectable ()
{
    BOOLEAN result = FALSE;

    DBUG_ENTER ("ejectable");
    if (ardp != NULL && (ardp -> dv_flags & D_EJECT)) {
	result = TRUE;
    }
    DBUG_PRINT ("eject", ("returns %d", result));
    DBUG_RETURN (result);
}


/*
 *	Test to see if the current archive device is flagged as
 *	not being able to do I/O directly from shared memory.
 *	This is a driver limitation that affects some systems.
 *	In this case, the data must be copied to/from a malloc'd
 *	space before/after doing I/O.
 *
 */

BOOLEAN needshmcopy ()
{
    BOOLEAN result = FALSE;

    DBUG_ENTER ("needshmcopy");
    if (ardp != NULL && (ardp -> dv_flags & D_SHMCOPY)) {
	result = TRUE;
    }
    DBUG_PRINT ("shmcopy", ("returns %d", result));
    DBUG_RETURN (result);
}
