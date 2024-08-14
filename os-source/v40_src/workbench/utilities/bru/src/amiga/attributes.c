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
 *	attributes.c    routines to get/set special machine dependent file attributes
 *
 *  SCCS
 *
 *	@(#)attributes.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	Some machines support additional file attributes that have no
 *	corresponding attribute in the Unix file model.  These routines
 *	support getting or setting these special attributes for the
 *	Commodore Amiga.
 *
 */

#include "globals.h"

/*
 *	This routine gets the 32-bit protection mask word for a
 *	named file.
 */

long getprot (path)
char *path;
{
    long lck;
    struct FileInfoBlock *fp;
    long prot;

    DBUG_ENTER ("getprot");
    DBUG_PRINT ("prot", ("get protection bits for '%s'", path));
    if ((lck = Lock (path, ACCESS_READ)) == 0) {
	prot = SYS_ERROR;
    } else {
	fp = (struct FileInfoBlock *)
	    AllocMem ((long) sizeof (struct FileInfoBlock), 
	    (long) (MEMF_CLEAR | MEMF_CHIP));
	Examine (lck, fp);
	prot = fp -> fib_Protection;
	DBUG_PRINT ("prot", ("protection bits = %#lx", prot));
	(VOID) FreeMem (fp, (long) sizeof (struct FileInfoBlock));
	(VOID) UnLock (lck);
    }
    DBUG_RETURN (prot);
}

/*
 *	Given file name and a 32-bit protection word, set the
 *	protection word.
 */

int setprot (path, prot)
char *path;
long prot;
{
    int rtnval = 0;

    DBUG_ENTER ("setprot");
    DBUG_PRINT ("prot", ("file '%s'; protection = %#lx", path, prot));
    if (SetProtection (path, prot) == 0) {
	rtnval = SYS_ERROR;
    }
    DBUG_RETURN (rtnval);
}

/*
 *	This routine gets the comment for the specified file and
 *	copies it to the specified buffer.
 */

int getcomment (path, buffer)
char *path;
char *buffer;
{
    long lck;
    struct FileInfoBlock *fp;
    long rtnval;

    DBUG_ENTER ("getcomment");
    if ((lck = Lock (path, ACCESS_READ)) == 0) {
	rtnval = SYS_ERROR;
    } else {
	fp = (struct FileInfoBlock *)
	    AllocMem ((long) sizeof (struct FileInfoBlock), 
	    (long) (MEMF_CLEAR | MEMF_CHIP));
	Examine (lck, fp);
	strcpy (buffer, fp -> fib_Comment);
	DBUG_PRINT ("comment", ("comment = '%s'", buffer));
	(VOID) FreeMem (fp, (long) sizeof (struct FileInfoBlock));
	(VOID) UnLock (lck);
	rtnval = 0;
    }
    DBUG_RETURN (rtnval);
}

int setcomment (path, buffer)
char *path;
char *buffer;
{
    int rtnval = 0;

    DBUG_ENTER ("setcomment");
    DBUG_PRINT ("comment", ("file '%s'; comment '%s'", path, buffer));
    if (SetComment (path, buffer) == 0) {
	rtnval = SYS_ERROR;
    }
    DBUG_RETURN (rtnval);
}

/*
 *	Fill in machine dependent portion of file information structure.
 */

int getinfo (fip)
struct finfo *fip;
{
    int rtnval = 0;

    DBUG_ENTER ("getinfo");
    if ((fip -> fib_Protection = getprot (fip -> fname)) == SYS_ERROR) {
	rtnval = SYS_ERROR;
    }
    if (getcomment (fip -> fname, fip -> fib_Comment) == SYS_ERROR) {
	rtnval = SYS_ERROR;
    }
    DBUG_RETURN (rtnval);
}

/*
 *	Set machine dependent attributes of file from a file information
 *	structure.
 */

int setinfo (fip)
struct finfo *fip;
{
    int rtnval = 0;

    DBUG_ENTER ("setinfo");
    if (setprot (fip -> fname, fip -> fib_Protection) == SYS_ERROR) {
	rtnval = SYS_ERROR;
    }
    if (setcomment (fip -> fname, fip -> fib_Comment) == SYS_ERROR) {
	rtnval = SYS_ERROR;
    }
    DBUG_RETURN (rtnval);
}

VOID mark_archived (fip)
struct finfo *fip;
{
    long prot;

    DBUG_ENTER ("mark_archived");
    prot = getprot (fip -> fname);
    prot |= FIBF_ARCHIVE;
    setprot (fip -> fname, prot);
    DBUG_VOID_RETURN;
}

VOID clear_archived (fip)
struct finfo *fip;
{
    long prot;

    DBUG_ENTER ("clear_archived");
    prot = getprot (fip -> fname);
    prot &= ~(FIBF_ARCHIVE);
    setprot (fip -> fname, prot);
    DBUG_VOID_RETURN;
}

int abit_set (fip)
struct finfo *fip;
{
    long prot;

    DBUG_ENTER ("abit_set");
    prot = getprot (fip -> fname);
    DBUG_RETURN (prot & FIBF_ARCHIVE);
}
