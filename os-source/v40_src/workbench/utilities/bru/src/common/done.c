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
 *	done.c    routines for cleaning up and exiting
 *
 *  SCCS
 *
 *	@(#)done.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	Contains routines for graceful exit by cleaning up and
 *	then calling exit with appropriate status.
 *
 *	Also is responsible for printing an execution summary
 *	if requested.
 *
 */

#include "globals.h"

/*
 *	Local functions.
 */

static VOID summary PROTO((void));


/*
 *  FUNCTION
 *
 *	done    clean up and exit with status
 *
 *  SYNOPSIS
 *
 *	VOID done ()
 *
 *  DESCRIPTION
 *
 *	Unlinks the temporary file and exits with meaningful status.
 *
 */

VOID done ()
{
    int exitstatus;

    DBUG_ENTER ("done");
    if (flags.vflag > 3) {
	summary ();
    }
    if (einfo.errors > 0) {
	exitstatus = 2;
    } else if (einfo.warnings > 0) {
	exitstatus = 1;
    } else {
	exitstatus = 0;
    }
#if amigados
    if (afile.fildes >= 0) {
	(VOID) s_close (afile.fildes);
	afile.fildes = -1;
    }
    if (flags.Afflag) {
	ipc_cleanup ();
    }
#endif
#if HAVE_SHM
    dbl_done ();
#endif
    s_exit (exitstatus);
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	summary    print execution summary info
 *
 *  SYNOPSIS
 *
 *	static VOID summary ()
 *
 *  DESCRIPTION
 *
 *	Prints a rather verbose execution summary containing such
 *	information as number of archive blocks read or written,
 *	total number of errors or warnings, soft or hard read or
 *	write errors, total blocks with checksum errors, etc.
 *
 */

static VOID summary ()
{
    long zavg;

    DBUG_ENTER ("summary");
    if (einfo.filebytes == 0) {
	zavg = 0;
    } else {
	if (einfo.zfilebytes > (1 << 24)) {
	    DBUG_PRINT ("zstat", ("rescale large values to avoid overflow"));
	    einfo.zfilebytes >>= 8;
	    einfo.filebytes >>= 8;
	}
	DBUG_PRINT ("zstat", ("%ld original bytes", einfo.filebytes));
	DBUG_PRINT ("zstat", ("%ld compressed bytes", einfo.zfilebytes));
	zavg = einfo.zfilebytes * 100;
	zavg /= einfo.filebytes;
	zavg = 100 - zavg;
    }
    vflush ();
    voutput ("**** %s: execution summary ****", info.bru_name);
    vflush ();
    vflush ();
    voutput ("Messages:\t\t%d warnings,  %d errors", einfo.warnings,
	     einfo.errors);
    vflush ();
    voutput ("Archive I/O:\t\t%ld blocks (%ldKb) written", 
	     einfo.bwrites, KBYTES (einfo.bwrites));
    vflush ();
    voutput ("Archive I/O:\t\t%ld blocks (%ldKb) read", 
	     einfo.breads, KBYTES (einfo.breads));
    vflush ();
    voutput ("Read errors:\t\t%d soft,  %d hard", einfo.rsoft, einfo.rhard);
    vflush ();
    voutput ("Write errors:\t\t%d soft,  %d hard", einfo.wsoft, einfo.whard);
    vflush ();
    voutput ("Checksum errors:\t%d", einfo.chkerr);
    vflush ();
    voutput ("Min Compression:\t%d%%", einfo.zmin);
    vflush ();
    voutput ("Avg Compression:\t%ld%%", zavg);
    vflush ();
    voutput ("Max Compression:\t%d%%", einfo.zmax);
    vflush ();
    DBUG_VOID_RETURN;
}
