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
 *	libaccess.c    emulation of Unix access library routine
 *
 *  SCCS
 *
 *	@(#)libaccess.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	Unix compatible emulation of the access() library routine.
 *
 *	Neither Manx nor Lattice seem to get this right, so we
 *	roll our own.
 *
 *	Note that the original amiga versions of bru fully implemented
 *	access protections on files, I.E. it honored the read and write
 *	protection bits and would not attempt to read a read-protected
 *	file or write a write-protected file.  However, by CBM's request,
 *	it was modified to subvert the file protections.
 *
 */

#include "globals.h"

int access (path, mode)
char *path;
int mode;
{
    long lck;
    struct FileInfoBlock *fp;
    int rtnval = 0;
    long prot;

    DBUG_ENTER ("access");
    if ((path == 0) || ((lck = Lock (path, (long) ACCESS_READ)) == 0)) {
	DBUG_PRINT ("acc", ("path == 0 or can't get lock"));
	rtnval = -1;
	errno = ENOENT;
    } else {
	DBUG_PRINT ("lck", ("got lock for '%s'", path));
	fp = (struct FileInfoBlock *)
		AllocMem ((long) sizeof (struct FileInfoBlock),
		(long) (MEMF_CLEAR | MEMF_CHIP));
	if (Examine (lck, fp) == 0) {
	    DBUG_PRINT ("xmn", ("can't examine lock"));
	    rtnval = -1;
	    errno = EACCES;
	} else {
	    prot = fp -> fib_Protection;
	    DBUG_PRINT ("prot", ("fp -> fib_Protection = %x, mode = %x", prot, mode));
	    if (  ((mode & 1) && (prot & FIBF_EXECUTE))
#if 0
	       || ((mode & 2) && (prot & FIBF_WRITE))
	       || ((mode & 4) && (prot & FIBF_READ))
#endif
	       ) {
		    rtnval = -1;
		    errno = EACCES;
	    }
	}
	(VOID) FreeMem (fp, (long) sizeof (struct FileInfoBlock));
	(VOID) UnLock (lck);
    }
    DBUG_PRINT ("access", ("access returns %d", rtnval));
    DBUG_RETURN (rtnval);
}
