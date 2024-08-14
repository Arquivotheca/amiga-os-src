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
 *	readinfo.c    routines to read and process archive info block
 *
 *  SCCS
 *
 *	@(#)readinfo.c	12.8	11 Feb 1991
 *
 */

 
/*
 *	Many of these routines used to be in the file blocks.c but have
 *	been moved to this file in an attempt to simplify blocks.c and
 *	decouple some of the routines.  This was only partially successful
 *	since they have gradually evolved into mutually referential files.
 *
 *	The entire "virtual archive" implementation has evolved from a
 *	relatively simple straightforward set of routines to the complicated
 *	convoluted code which now exists.  It is time to chuck the whole
 *	thing and start over, using what has been learned about the
 *	implementation requirements.  This is a prime example of why
 *	you should plan to throw away the first implementation of anything,
 *	because you WILL throw it away whether you plan to or not!!
 *
 */
 
#include "globals.h"

static BOOLEAN is_junk PROTO((union blk *blkp));
static VOID dump_info PROTO((union blk *blkp));
static VOID bad_magic PROTO((union blk *blkp));
static VOID size_hack PROTO((UINT size));


/*
 *	The info_done flag prevents the recursive buffer size
 *	adjustment from printing the info block more than once.
 *
 *	Note that we pass the fip of the archive file to ar_read
 *	just to satisfy it's need for one.  At this point, there is
 *	no "current file".  Also, we set the flba to 0 because it
 *	gets automatically incremented as blocks are read, and we
 *	can be called recursively, so it needs to be zero each time
 *	we attempt to read the first block.
 */
 
VOID read_info ()
{
    static BOOLEAN info_done = FALSE;
    union blk *blkp;
    int magic;

    DBUG_ENTER ("read_info");
    blkp = ar_seek (0L, 0);
    afile.flba = 0L;
    ar_read (&afile);
    if (!is_junk (blkp)) {
	artime = (time_t) FROMHEX (blkp -> HD.h_time);
	readsizes (blkp);
	magic = (int) FROMHEX (blkp -> HD.h_magic);
	DBUG_PRINT ("magic", ("archive header magic number %#x", magic));
	if (magic != A_MAGIC) {
	    bad_magic (blkp);
	} else {
	    if (!info_done) {
		if ((mode == 'i' && flags.vflag > 2) || (mode == 'g')) {
		    info_done = TRUE;
		    dump_info (blkp);
		}
	    }
	}
    }
    DBUG_VOID_RETURN;
}


/*
 *	Return non-zero if buffer can be forced to BLKSIZE for
 *	efficiency during special cases (like for reading only
 *	the file header blocks during -t mode).
 */

BOOLEAN forcebuffer ()
{
    int result;

    DBUG_ENTER ("forcebuffer");
    result = (mode == 't' && msize != 0 && !flags.bflag && !flags.pflag);
    result &= seekable (afile.fname, BLKSIZE);
    DBUG_PRINT ("force", ("returns logical value %d", result));
    DBUG_RETURN (result);
}


/*
 *  FUNCTION
 *
 *	sanity    check various options for sanity
 *
 *  SYNOPSIS
 *
 *	static VOID sanity ()
 *
 *  DESCRIPTION
 *
 *	After all command line options have been processed, this
 *	function performs various "sanity" checks.  These could
 *	be done in-line in options() but there are some advantages
 *	to delaying the operation:
 *
 *		(1)	Syntax errors in the command line show
 *			up before semantic errors.
 *
 *		(2)	The switch statement in options() is
 *			usually quite long already.
 *
 *	Note that this is also a logical place to emit various
 *	verbosity messages and do some "post-processing" of
 *	options.
 *
 *  NOTE
 *
 *	The been_here flag is a hack to prevent the recursive
 *	auto buffer size adjustment from printing the sizes more
 *	than once.  Note that the first instance of ar_open prints
 *	the new sizes, which the second instance of ar_open implemented.
 *	This is all very dependent on that fact than sanity() is called
 *	after read_info() in ar_open().
 *
 */

VOID sanity ()
{
    static BOOLEAN been_here = FALSE;
    
    DBUG_ENTER ("sanity");
    if (forcebuffer ()) {
	DBUG_PRINT ("bufsize", ("buffer size forced to BLKSIZE"));
	bufsize = BLKSIZE;
    }
    if (flags.vflag > 1 && !been_here) {
	been_here = TRUE;
	voutput ("buffer size = %uk bytes", bufsize/1024);
	vflush ();
	voutput ("media size = ");
	if (msize != 0) {
	    voutput ("%luk bytes", msize/1024);
	} else {
	    voutput ("<unknown>");
	}
	vflush ();
    }
    DBUG_VOID_RETURN;
}


static BOOLEAN is_junk (blkp)
union blk *blkp;
{
    BOOLEAN result;

    DBUG_ENTER ("is_junk");
    result = FALSE;
    if (!chksum_ok (blkp)) {
	if (!need_swap (blkp)) {
	    bru_message (MSG_ISUM);
	    result = TRUE;
	    if (chksum (blkp) != 0L) {
	        bru_message (MSG_FASTMODE);
	    }
	} else {
	    do_swap ();
	}
    }
    DBUG_RETURN (result);
}


static VOID dump_info (blkp)
union blk *blkp;
{
    USHORT ar_uid;
    USHORT ar_gid;
    char *dtime;

    DBUG_ENTER ("dump_info");
    ar_uid = (USHORT) FROMHEX (blkp -> AH.a_uid);
    ar_gid = (USHORT) FROMHEX (blkp -> AH.a_gid);
    if (mode == 'g') {
	voutput ("label:\t\t%s", blkp -> AH.a_label);
	vflush ();
	dtime = s_ctime((long *) &artime);
	dtime[s_strlen (dtime) - 1] = EOS;
	voutput ("created:\t%s", dtime);
	vflush ();
	voutput ("device:\t\t%s", blkp -> AH.a_name);
	vflush ();
	voutput ("user:\t\t%s", ur_gname (ar_uid));
	vflush ();
	voutput ("group:\t\t%s", gp_gname (ar_gid));
	vflush ();
	voutput ("system:\t\t%s %s %s %s %s",
		   blkp -> AH.a_sysname, blkp -> AH.a_nodename,
		   blkp -> AH.a_release, blkp -> AH.a_version,
		   blkp -> AH.a_machine);
	vflush ();
	voutput ("bru:\t\t\"%s\"", blkp -> AH.a_id);
	vflush ();
	voutput ("release:\t%ld.%ld",
		   FROMHEX (blkp -> HD.h_release),
		   FROMHEX (blkp -> HD.h_level));
	vflush ();
	voutput ("variant:\t%ld",
		   FROMHEX (blkp -> HD.h_variant));
	vflush ();
	voutput ("bufsize:\t%ld",
		   FROMHEX (blkp -> HD.h_bufsize));
	vflush ();
	voutput ("msize:\t\t%lu",
		   FROMHEX (blkp -> HD.h_msize));
	vflush ();
    } else {
	vflush ();
	voutput ("**** archive header info ****");
	vflush ();
	vflush ();
	dtime = s_ctime ((long *) &artime);
	dtime[s_strlen (dtime) - 1] = EOS;
	voutput ("archive created: %s", dtime);
	vflush ();
	voutput ("release: %ld.%ld  ",
		   FROMHEX (blkp -> HD.h_release),
		   FROMHEX (blkp -> HD.h_level));
	voutput ("variant: %ld", FROMHEX (blkp -> HD.h_variant));
	vflush ();
	voutput ("bru id: \"%s\"", blkp -> AH.a_id);
	vflush ();
	voutput ("archive label: \"%s\"", blkp -> AH.a_label);
	vflush ();
	voutput ("written on: \"%s\"", blkp -> AH.a_name);
	vflush ();
	voutput ("user id: %s  ", ur_gname (ar_uid));
	voutput ("group id: %s", gp_gname (ar_gid));
	vflush ();
	voutput ("system identification: %s %s %s %s %s",
		   blkp -> AH.a_sysname, blkp -> AH.a_nodename,
		   blkp -> AH.a_release, blkp -> AH.a_version,
		   blkp -> AH.a_machine);
	vflush ();
	vflush ();
	voutput ("**** end info ****");
	vflush ();
	vflush ();
    }
    DBUG_VOID_RETURN;
}


static VOID bad_magic (blkp)
union blk *blkp;
{
    int vol;
    char key;

    DBUG_ENTER ("bad_magic");
    vol = (int) FROMHEX (blkp -> HD.h_vol);
    bru_message (MSG_IMAGIC, vol + 1);
    key = response ("c => continue  q => quit  r => reload  s => fork shell",
    	'q');
    switch (key) {
	case 'q':
	case 'Q':
	    done ();
	    break;
	case 'r':
	case 'R':
	    switch_media (0);
	    phys_seek (0L);
	    read_info ();
	    break;
	case 's':
	case 'S':
	    fork_shell ();
	    bad_magic (blkp);
	    break;
    }
    DBUG_VOID_RETURN;
}


/*
 *	Attempt to adjust to new buffer size.  Temporarily
 *	suppress eject of media since we want the same
 *	media as before.
 */

static VOID size_hack (size)
UINT size;
{
    int ejectflag;

    DBUG_ENTER ("size_hack");
    if (ardp != NULL && ardp -> dv_flags & D_NOREWIND) {
	size /= 1024;
	bru_message (MSG_NOREWIND);
	bru_message (MSG_RERUN, size);
	done ();
    } else {
	DBUG_PRINT ("bufsize", ("adjust buffer size to %u", size));
	flags.bflag = TRUE;
	bufsize = size;
	shmcheck ();
	if (ardp != NULL && ardp -> dv_maxbsize != 0) {
	    if (bufsize > ardp -> dv_maxbsize) {
		bru_message (MSG_MAXBUFSZ, bufsize / 1024,
			   ardp -> dv_maxbsize / 1024);
		if (!flags.bflag) {
		    bufsize = BLOCKS (ardp -> dv_maxbsize) * BLKSIZE;
		    bru_message (MSG_BUFADJ, bufsize / 1024);		    
		}
	    }
	}
	if (ardp == NULL) {
	    ar_close ();
	} else {
	    ejectflag = ardp -> dv_flags & D_EJECT;
	    ardp -> dv_flags &= ~D_EJECT;
	    ar_close ();
	    ardp -> dv_flags |= ejectflag;
	}
	deallocate ();
	ar_open ();
    }
    DBUG_VOID_RETURN;
}


/*
 *	Note that in fast mode (-F flag), the information in the header
 *	block may be unreliable.  We ignore the sizes if they look
 *	unreasonable (like 0).  This can actually happen if there is
 *	a hard read error on the first block and -F is used.
 */

VOID readsizes (blkp)
union blk *blkp;
{
    UINT size;
    ULONG ar_msize;

    DBUG_ENTER ("readsizes");
    if (!flags.sflag) {
	ar_msize = (ULONG) FROMHEX (blkp -> HD.h_msize);
	if (ar_msize != 0) {
	    msize = ar_msize;
	}
    }
    if (!flags.bflag) {
	size = (UINT) FROMHEX (blkp -> HD.h_bufsize);
	DBUG_PRINT ("bufsize", ("recorded buffer size = %u", size));
	DBUG_PRINT ("bufsize", ("current buffer size = %u", bufsize));
	if (size != 0 && size != bufsize && !ar_ispipe()) {
	    size_hack (size);
	}
    }
    DBUG_VOID_RETURN;
}

