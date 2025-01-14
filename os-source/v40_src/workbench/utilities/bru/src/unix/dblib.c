/************************************************************************
 *									*
 *	Copyright (c) 1988 Enhanced Software Technologies, Inc.		*
 *			   All Rights Reserved				*
 *									*
 *	This software and/or documentation is protected by U.S.		*
 *	Copyright Law (Title 17 United States Code).  Unauthorized	*
 *	reproduction and/or sales may result in imprisonment of up	*
 *	to 1 year and fines of up to $10,000 (17 USC 506).		*
 *	Copyright infringers may also be subject to civil liability.	*
 *									*
 *	(NOTE: See attribution to original author below)		*
 *									*
 ************************************************************************
 */


/*
 *  FILE
 *
 *	dblib.c    support for double buffering under unix, using System V IPC
 *
 *  SCCS
 *
 *	@(#)dblib.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	Overriding philosophy is that the parent (reader or writer) sees a
 *	very long input stream that has a definite end, with no media changes
 *	being necessary.  The child handles all media switching and block
 *	handling.
 *
 *	Since little documentation exists on the "correct" way to deal with a
 *	tape device, I'll expound on the philosophy used here.  On writing, it
 *	is assumed that writes either write the whole amount requested, or
 *	fail (a partial write causes a call to the write recovery routine
 *	however, for safety).  On reading, if a partial read occurs, it is
 *	assumed to be the result of an earlier partial write, and the read
 *	recovery routine is invoked, thus discarding the partial read.  This
 *	ensures that the stream out contains a consistent view of the data.
 *	If you want to use every last inch of the tape, then a tape blocksize
 *	matching the device blocksize (512 bytes for streaming tape usually)
 *	must be used.  (J.M. Barton)
 *
 *  AUTHOR
 *
 *	Original code by J. M. Barton at Silicon Graphics Inc.  Incorporated
 *	into bru source code under license agreement between Silicon Graphics
 *	Inc and Engineering Software Tools.
 *
 */

#include "globals.h"

#if HAVE_SHM

/*
 *	C purists hold your noses, the error return from shmat()
 *	is not a NULL pointer, it is (-1) cast to a pointer.  Ick, choke,
 *	PeeeeYoooouuuu!
 *
 */

#if (xenix && !M_I386)			/* xenix and !i386 (should be i286?) */
#define SHMATNULL ((far char *) -1)
#else
#if defined(__STDC__) && !defined(_STYPES)	/* prototypes and SVR4 */
#define SHMATNULL ((char *) -1)
#else
#define SHMATNULL ((char *) -1)
#endif
#endif

/*
 *	A return status is passed from child to parent via a shared
 *	memory location.  These constants define the possible return
 *	values.
 */

#define RV_INIT		0		/* Initial value (no status) */
#define RV_EOF		1		/* end of file on input */
#define RV_WFAIL	2		/* write recovery failed */
#define RV_RFAIL	3		/* read recovery failed */
#define RV_OK		4		/* writer acknowledges end of data */
#define RV_SYSERR	5		/* system interaction error */
#define RV_SIG		6		/* signal caught */
#define RV_NOMEM	7		/* out of memory */

/*
 *	Define the amount by which we wish to try to temporarily
 *	move our brk value (ROOM), and the minimum amount of workspace
 *	we want to try to leave available for future malloc's
 *	(WORKSPACE).
 */

#define ROOM		(segsize)	/* A shared mem segments worth */
#define WORKSPACE	(1024 * 1024)	/* Leave about a megabyte */


/*
 *	Synchronization of parent and child is done via message
 *	passing.  This defines the structure of those messages and
 *	some of the constants used in messages.
 */

struct brumsg {
    long m_type;
    int m_action;
    int m_buf;
    VOID (*m_proc) ();
    int m_nb;
    int m_err;
    long m_arg;
};

#define MSGSZ		(sizeof (struct brumsg) - sizeof (long))

#define DOBUF		1
#define FREEBUF		2	/* buffer is now free */
#define CALLDONE	3
#define CALLPROC	4
#define CHEND		5	/* child end */

/*
 *	The following structure is used to keep track of
 *	shared memory segments that have been attached.
 *	Many of the members of this structure used to be
 *	fixed length arrays, with the length set by a 
 *	MAXNBUF parameter, which set a compile time limit
 *	on the maximum number of segments that could be
 *	by a given executable.
 *
 *	Now, we allocate these structures (actually an
 *	array of these structures) dynamically, so there is
 *	no upper bound on their numbers, other than that
 *	set by memory, and that set by the number of
 *	segments that can actually be used.  This second
 *	limit is generally the limiting factor.
 *
 */

struct segment {		/* One per shared mem segment */
    char *shmaddr;		/* buffer locations */
    long *len;			/* length of a buffer */
};

static struct segment *segs;	/* Point to array of segs */

static struct segment misc = {	/* Miscellaneous shared variables */
    SHMATNULL,			/* THIS is why we must initialize it! */
    NULL			/* This is not same as SHMATNULL, ick! */
};

/*
 *	Variables used locally.
 */

static SIGTYPE savesig;		/* place to save a signal */
static int *bufcache;		/* hysterisis cache */
static UINT tpblk;		/* tape block size */
static int ncache;		/* number in cache */
static int curcache;		/* next cache entry */
static struct brumsg srpc;		/* saved RPC call */
static int issrpc;		/* if a saved one present */
static long *rval;		/* child return value */
static long dumvol = 0;		/* dummy volume number */
static long *volumep = &dumvol;	/* current volume */
static int *errnop;		/* child errno */
static int status;		/* exit status */
static int chendseen;		/* child end message */
static int msg;			/* message queue */
static int dodb;		/* if double buffering */
static int (*outerr) ();	/* output error */
static int (*inerr) ();		/* input error */
static int (*uswrite) ();	/* user write routine */
static int (*usread) ();	/* user read routine */
static jmp_buf errjump;		/* for bugging out */
static int cpid = -1;		/* child pid (0 = we are child) */
static int writing;		/* direction (0 = read; 1 = write) */
static int nomore;		/* used by parent for no data */
static int deathok;		/* OK for child to die */
static char *copybuf = NULL;	/* local buffer if needed */

/*
 *	Write state that needs to be shared with the flush routine.
 */

static long lastwsize = 0;
static int lastwbuf;

/*
 *	On most systems with System V shared memory, there is a
 *	system enforced limit on the size of any particular shared
 *	memory segment, and on the number of segments.  For tuning
 *	purposes, we also can set a limit on the size of any
 *	particular shared memory segment and a limit on the number
 *	of segments, via an entry in the brutab file.
 *
 *	Note that the two limits (size and number) are not independent.
 *	Generally, for any given size, the product of the size and
 *	the number of segments is roughly a constant.  I.E., halving
 *	the size of each segment will double the number of segments
 *	that can be allocated.  However, there are hard limits at
 *	both ends, one on the size of any segment regardless of 
 *	the number of them, and another on the number of segments,
 *	regardless of their sizes.
 *
 *	Generally, we will operate with some reasonable default
 *	values for the number of segments and their sizes.  However,
 *	by setting shmseg, shmmax, and shmall in the brutab file
 *	to unreasonably high values, we can operate in "greedy mode",
 *	whereby we automatically allocate the largest segments possible
 *	and the maximum number of them that we can.  We can also
 *	use the brutab entries to impose artificial limits lower
 *	than the default values, if we desire.
 *
 */

static UINT maxsegs;			/* Max segments enforced by system */
static UINT limshmseg = B_SHMSEG;	/* Max segments limit requested */
static UINT nsegs;			/* Actual number to attempt to use */
static UINT asegs;			/* Actual number attached */

static int maxsegsize;			/* Max seg size enforced by system */
static long limshmmax = B_SHMMAX;	/* Max segment size limit requested */
static int segsize;			/* Actual size we will use */

static long limshmall = B_SHMALL;	/* Max total shm size requested */

/*
 *	Local functions, forward declared.
 */

static int copyout ();
static int copyin ();
static int getfullbuf ();
static int getfreebuf ();
static VOID fullbuf ();
static VOID freebuf ();
static VOID dorpc ();
static VOID chend ();
static int dbli_localread ();
static int dbli_localwrite ();
static int ShmSizeOk ();
static VOID trashmsg ();
static int dbli_setupbuf ();
static BOOLEAN setupcopy ();
static int copyoutbuf ();


/*
 *	Find the maximum size of a single shared memory segment, to
 *	the nearest 4 bytes.  First find the upper and lower bounds
 *	to the nearest power of two, then use a binary search to narrow
 *	the difference between the lower and upper bounds to less
 *	than 4.  This is the straightforward, quick and dirty
 *	solution.  There is probably an elegant 5 line recursive
 *	solution waiting to be written...
 *
 */

int maxshmsize ()
{
    int lowval;
    int testval;
    int highval;
	
    DBUG_ENTER ("maxshmsize");
    if (maxsegsize > 0) {
	lowval = maxsegsize;
    } else {
	for (testval = 1; testval > 0; testval *= 2) {
	    highval = testval;
	    lowval = testval / 2;
	    if (!ShmSizeOk (testval)) {
		break;
	    }
	}
	if (testval < 0) {
	    lowval = highval;
	    highval = testval - 1;
	}
	while ((highval - lowval) > 4) {
	    DBUG_PRINT ("shmsize", ("lower bound %d", lowval));
	    DBUG_PRINT ("shmsize", ("upper bound %d", highval));
	    testval = (highval / 2) + (lowval / 2);
	    if (ShmSizeOk (testval)) {
		lowval = testval;
	    } else {
		highval = testval;
	    }
	}
	maxsegsize = lowval;
    }
    DBUG_PRINT ("maxsegsize", ("max shared mem seg size = %d", lowval));
    DBUG_RETURN (lowval);
}


static int ShmSizeOk (size)
int size;
{
    int shmid;
    int result = 0;
    
    DBUG_ENTER ("ShmSizeOk");
    DBUG_PRINT ("shmsize", ("test size %ld", size));
    if ((shmid = shmget (IPC_PRIVATE, size, IPC_CREAT | 0600)) != -1) {
	result = 1;
	(VOID) shmctl (shmid, IPC_RMID, (struct shmid_ds *) NULL);
    }
    DBUG_RETURN (result);
}


/*
 *	Determine the maximum number of shared memory segments we could
 *	allocate if we reduced the size of each segment to the smallest
 *	chunk of data (one block, BLKSIZE bytes) read or written by bru
 *	in a single read or write to the archive device.  Don't bother
 *	testing further if we go over 1024 segments.
 */

static int maxshmsegs ()
{
    int segcount;
    int shmid;
    char *shmaddrs[256];

    DBUG_ENTER ("maxshmsegs");
    if (maxsegs == 0) {
	segcount = 0;
	do {
	    shmaddrs[segcount] = SHMATNULL;
	    shmid = shmget (IPC_PRIVATE, (int) BLKSIZE, IPC_CREAT | 0600);
	    if (shmid != -1) {
		shmaddrs[segcount] = shmat (shmid, 0, 0);
		DBUG_PRINT ("shmat", ("attached shmseg at %x", shmaddrs[segcount]));
		(VOID) shmctl (shmid, IPC_RMID, (struct shmid_ds *) NULL);
	    }
	} while ((shmaddrs[segcount] != SHMATNULL) && (++segcount < 256));
	maxsegs = segcount;
	while (--segcount >= 0) {
	    DBUG_PRINT ("shmdt", ("detach segment at %#x", shmaddrs[segcount]));
	    (VOID) shmdt (shmaddrs[segcount]);
	}
    }
    DBUG_PRINT ("maxsegs", ("max number of segments = %d", maxsegs));
    DBUG_RETURN (maxsegs);
}


/*
 *	dbl_errp - set the error parameters.
 *	Meaningful at any time.
 */

VOID dbl_errp (inerror, outerror)
int (*inerror) ();
int (*outerror) ();
{
    DBUG_ENTER ("dbl_errp");
    inerr = inerror;
    outerr = outerror;
    DBUG_VOID_RETURN;
}

/*
 *	dbl_iop - set the I/O parameters.
 *	Only meaningful before a call to dbl_setup.
 */

VOID dbl_iop (readf, writef)
int (*readf) ();
int (*writef) ();
{
    DBUG_ENTER ("dbl_iop");
    if (readf != 0) {
	usread = readf;
    }
    if (writef != 0) {
	uswrite = writef;
    }
    DBUG_VOID_RETURN;
}

/*
 *	Set the buffering parameters.  Only meaningful before a call
 * 	to dbl_setup.
 *
 *	The shmsize parameter, if nonzero, sets a maximum limit on the
 *	size of a shared memory segment that will be allocated in a
 *	single call to shmget ().
 *
 *	The shmsegs parameter, if nonzero, sets a maximum limit on the
 *	number of shared memory segments that will be allocated.
 *
 *
 *	The tapeblk parameter, if nonzero, sets the size of each record
 *	on the archive device.
 *
 */

VOID dbl_parms (shmseg, shmmax, shmall, tapeblk)
UINT shmseg;
long shmmax;
long shmall;
long tapeblk;
{
    DBUG_ENTER ("dbl_parms");
    if (shmseg != 0) {
	DBUG_PRINT ("limshmseg", ("limit to %u segments", shmseg));
	limshmseg = shmseg;
    }
    if (shmmax != 0) {
	DBUG_PRINT ("limshmmax", ("limit segment size to %ld", shmmax));
	limshmmax = shmmax;
    }
    if (shmall != 0) {
	limshmall = shmall;
	DBUG_PRINT ("limshmall", ("use requested shmall of %ld", limshmall));
    } else if (shmseg != 0 || shmmax != 0) {
	limshmall = limshmseg * limshmmax;
	DBUG_PRINT ("limshmall", ("use computed shmall of %ld", limshmall));
    }
    if (tapeblk != 0) {
	DBUG_PRINT ("tpblk", ("block size is %ld", tapeblk));
	tpblk = tapeblk;
    }
    DBUG_VOID_RETURN;
}

/*
 *	dbl_done - call when done with reading or writing.
 */

VOID dbl_done ()
{
    int i;

    DBUG_ENTER ("dbl_done");
    if (dodb) {
	(VOID) s_signal (SIGCLD, SIG_IGN);
	DBUG_PRINT ("sigcld", ("SIGCLD now set to SIG_IGN"));
	if (cpid > 0) {
	    (VOID) s_kill (cpid, SIGTERM);
	}
	cpid = -1;
	(VOID) s_signal (SIGCLD, savesig);
	DBUG_PRINT ("sigcld", ("SIGCLD now set to savesig (%d)", savesig));
	if (segs != NULL) {
	    for (i = 0; i < asegs; i++) {
		if (segs[i].shmaddr != SHMATNULL) {
		    DBUG_PRINT ("shmdt", ("detach segment at %#x", segs[i].shmaddr));
		    (VOID) shmdt (segs[i].shmaddr);
		    segs[i].shmaddr = SHMATNULL;
		}
	    }
	    s_free ((char *) segs);
	    segs = NULL;
	}
	if (misc.shmaddr != SHMATNULL) {
	    DBUG_PRINT ("shmdt", ("detach segment at %#x", misc.shmaddr));
	    (VOID) shmdt (misc.shmaddr);
	    misc.shmaddr = SHMATNULL;
	}
	if (bufcache != NULL) {
	    s_free ((char *) bufcache);
	    bufcache = NULL;
	}
	if (msg != 0) {
	    (VOID) msgctl (msg, IPC_RMID, (struct msqid_ds *) NULL);
	    msg = 0;
	}
	if (copybuf != NULL) {
	    s_free (copybuf);
	    copybuf = NULL;
	}
    }
    DBUG_VOID_RETURN;
}

static int dbli_defouterr (nb, err)
int nb;
int err;
{
    DBUG_ENTER ("dbli_defouterr");
    bru_message (MSG_DBLIO, nb, err);
    DBUG_RETURN (0);
}

static int dbli_definerr (nb, err)
int nb;
int err;
{
    DBUG_ENTER ("dbli_definerr");
    bru_message (MSG_DBLIO, nb, err);
    DBUG_RETURN (0);
}

static VOID dbli_catcher ()
{
    DBUG_ENTER ("dbli_catcher");
    DBUG_LONGJMP (errjump, RV_SIG);
    DBUG_VOID_RETURN;
}

static VOID dbli_intclean ()
{
    DBUG_ENTER ("dbli_intclean");
    if (writing) {
        (VOID) s_signal (SIGINT, (SIGTYPE) dbli_intclean);
    } else {
        DBUG_LONGJMP (errjump, RV_SIG);
    }
    DBUG_VOID_RETURN;
}

static VOID deadone ()
{
    int wstatus;
    int pid;

    DBUG_ENTER ("deadone");
    DBUG_PRINT ("sigcld", ("received a SIGCLD signal"));
    if ((pid = s_wait (&wstatus)) == -1) {
	bru_message (MSG_WAITFAIL);
	DBUG_PRINT ("wait", ("wait failed, errno = %d", errno));
    } else if (pid != cpid) {
	bru_message (MSG_CLDUNK, pid, cpid, wstatus);
	(VOID) s_signal (SIGCLD, (SIGTYPE) deadone);
	DBUG_PRINT ("sigcld", ("SIGCLD now set to &deadone()"));
    } else {
	if (*rval == 0) {
	    if (!deathok) {
		if (wstatus != 0) {
		    bru_message (MSG_SLVDIED, wstatus);
		}
	    }
	} else if (*rval == RV_SYSERR || *rval == RV_NOMEM) {
	    bru_message (MSG_SLVERR, *rval);
	}
	(VOID) s_signal (SIGCLD, savesig);
	DBUG_PRINT ("sigcld", ("SIGCLD now set to savesig (%d)", savesig));
    }
    DBUG_VOID_RETURN;
}

/*
 *	dbl_setup - initiate the double buffering.
 *	If called with '0' for 'dodbl', then no true double buffering is
 *	done (i.e., no other process is created).  If called with a '1' 
 *	for the parameter, true double buffering is initiated.  This is
 *	useful for maintaining a single interface while retaining seekable
 *	aspects of random-access files.
 *
 *	The one second sleep in the child before exiting is to help
 *	ensure that the parent wakes up and continues long enough to
 *	stash away the child pid value returned from fork before the
 *	child actually dies and the parent gets a SIGCLD.  In some cases
 *	where the child completed it's work before the parent ever got
 *	another chance to get scheduled to run, the parent was receiving
 *	what it thought was a "spurious" SIGCLD signal, since the fork
 *	had completed but the return value of the fork had not yet been
 *	saved to the child pid variable.  Yuk!
 *
 */

int dbl_setup (dodbl, direction)
int dodbl;
int direction;
{
    int i;
    UINT mallocbytes;

    DBUG_ENTER ("dbl_setup");

    /* 
     * Reset back in case this is a multiple call.
     */

    dbl_done ();
    deathok = 0;
    cpid = -1;

    /* 
     * If in transparent mode, indicate such.
     */

    if (!dodbl) {
	dodb = 0;
	dumvol = 0;
	volumep = &dumvol;
	errnop = &errno;
	DBUG_RETURN (0);
    }
    dodb = 1;

    /*
     * Find the limits of of shared memory implementation.
     */

    (VOID) maxshmsize ();
    (VOID) maxshmsegs ();

    /* 
     * Initialize all the parameters from the caller supplied
     * info.
     */

    if (tpblk == 0) {
	tpblk = BUFSIZE;
    }
    DBUG_PRINT ("recsize", ("tape record size %d", tpblk));

    /*
     *  If user requested limit on each segment size is smaller
     *  than the requested tape record size, then silently increase
     *  the user limit.  The user limit is either the default limit
     *  or a limit read from brutab, so this action follows the 
     *  general principle that command line values (-b <buffersize>
     *  in this case) override brutab values.  We could optionally
     *  give a warning, perhaps only with level 4 verbosity, about
     *  raising the limit, since the limit in brutab might be
     *  intentionally set to strictly enforce shared memory usage
     *  limits on any given user.
     */

    if (tpblk > limshmmax) {
	limshmmax = tpblk;
	DBUG_PRINT ("limits", ("seg size limit increased to %ld", limshmmax));
    }
   
    /*
     *	Initialize the size to use for each shared memory segment.
     *  We attempt to use the largest segment possible, up to
     *  either the hardware imposed limit or the user imposed
     *	limit.  Then we round down to an even number of archive
     *  records, since we would just waste the memory at the 
     *  end of the segment anyway.
     */

    if ((limshmmax > 0) && (maxsegsize > 0)) {
	segsize = min (limshmmax, maxsegsize);
	DBUG_PRINT ("limits", ("segsize limited to %d", segsize));
    } else {
	segsize = maxsegsize;
	DBUG_PRINT ("limits", ("try for max segsize of %d", segsize));
    }
    segsize = (segsize / tpblk) * tpblk;
    DBUG_PRINT ("segsize", ("using segments of %d bytes", segsize));

    /*
     *	Set the upper bound on the number of segments we will try
     *  to allocate.  Note that we will almost certainly not be able
     *	to allocate this many, since we are being greedy and also
     *  trying to allocate the largest segments possible.  Also,
     *  we burn up one shared memory region for miscellaneous
     *  variables which are shared.
     */

    DBUG_PRINT ("nsegs", ("first apply any limits requested"));
    if (limshmseg != 0) {
	DBUG_PRINT ("nsegs", ("use min (%d,%d)", limshmseg, maxsegs));
	nsegs = min (limshmseg, maxsegs);
    } else {
	DBUG_PRINT ("nsegs", ("just use maxsegs of %d", maxsegs));
	nsegs = maxsegs;
    }
    DBUG_PRINT ("limshmall", ("limshmall is %ld", limshmall));
    DBUG_PRINT ("nsegs", ("limit to segs that fit in limshmall"));
    nsegs = min (limshmall / segsize, nsegs);
    if (nsegs < 2) {
	bru_message (MSG_NSEGS, nsegs);
	DBUG_RETURN (1);
    }
    nsegs--;
    DBUG_PRINT ("nsegs", ("try to use %d segments for buffers", nsegs));
    mallocbytes = nsegs * sizeof (struct segment);
    if ((segs = (struct segment *) get_memory (mallocbytes, FALSE)) == NULL) {
	bru_message (MSG_MALLOC, mallocbytes);
	DBUG_RETURN (1);
    } else {
	for (i = 0; i < nsegs; i++) {
	    segs[i].shmaddr = SHMATNULL;
	}
    }
    if ((bufcache = (int *) get_memory (nsegs * sizeof (int), FALSE)) == NULL) {
	bru_message (MSG_MALLOC, nsegs * sizeof (int));
	DBUG_RETURN (1);
    }
    if (usread == NULL) {
	usread = s_read;
    }
    if (uswrite == NULL) {
	uswrite = s_write;
    }

    /* 
     * Set up the buffers and message queue.
     */

    if (dbli_setupbuf ()) {
	dbl_done ();
	segsize = 0;
	DBUG_PRINT ("segsize", ("segsize = %d", segsize));
	DBUG_RETURN (1);
    }
    *rval = RV_INIT;
    writing = direction;
    nomore = 0;
    ncache = 0;
    issrpc = 0;
    chendseen = 0;
    if (inerr == NULL) {
	inerr = dbli_definerr;
    }
    if (outerr == NULL) {
	outerr = dbli_defouterr;
    }
    /* 
     * Start up the child, who will actually deal with the
     * device/file.
     */
    savesig = s_signal (SIGCLD, (SIGTYPE) deadone);
    DBUG_PRINT ("sigcld", ("SIGCLD now set to &deadone()"));
    DBUG_PRINT ("sigcld", ("SIGCLD was set to %d (now in savesig)", savesig));
    if ((cpid = s_fork ()) == 0) {
	errno = 0;
	(VOID) s_signal (SIGTERM, (SIGTYPE) dbli_catcher);
	(VOID) s_signal (SIGQUIT, (SIGTYPE) dbli_catcher);
	(VOID) s_signal (SIGINT, (SIGTYPE) dbli_intclean);
	if ((*rval = DBUG_SETJMP (errjump)) == 0) {
	    if (needshmcopy () && !setupcopy ()) {
		*rval = RV_NOMEM;
		*errnop = errno;
	    } else {
		if (writing) {
		    *rval = copyout ();
		} else {
		    *rval = copyin ();
		    trashmsg ();
		}
	    }
	}
	if (*rval == RV_SYSERR && errno == EINTR) {
	    *rval = RV_SIG;
	}
	if (writing) {
	    af_close ();
	}
	DBUG_PRINT ("rval", ("child returns status %d to parent", *rval));
	DBUG_PRINT ("errno", ("errno = %d", *errnop));
	s_sleep (1);
	s_exit (0);
    } else if (cpid == -1) {
	bru_message (MSG_FORK);
	(VOID) msgctl (msg, IPC_RMID, (struct msqid_ds *) NULL);
	DBUG_RETURN (1);
    }
    if (!isrmt (afile.fildes)) {
	af_close ();
    }
    DBUG_RETURN (0);
}


/*
 *  FUNCTION
 *
 *	dbl_read    read the given amount of data from the child process
 *
 *  SYNOPSIS
 *
 *	int dbl_read (buf, size)
 *	char *buf;
 *	UINT size;
 *
 *  DESCRIPTION
 *
 *	This routine takes the place of the normal read system call,
 *	using the same arguments.  If double buffering is not turned
 *	on, then it operates in transparent mode and simply calls the
 *	low level read routine to read the data directly.  If double
 *	buffering is turned on, then it waits for the child to fill
 *	up as many buffers as necessary to fulfill the read request.
 *
 *  NOTES
 *
 *	This routine is called only in the parent process.
 *
 */

int dbl_read (buf, size)
char *buf;
UINT size;
{
    UINT bytesin;
    UINT bytesleft;
    int rtnval;
    static char *lastrpos;
    static long lastrsize = 0;
    static int lastrbuf;

    DBUG_ENTER ("dbl_read");
    if (!dodb) {
	DBUG_PRINT ("dblr", ("transparent mode, read data directly"));
	rtnval = dbli_localread (buf, size);
    } else {
	DBUG_PRINT ("dblr", ("read using shared memory buffers"));
	if (nomore) {
	    if (nomore == 1) {
		nomore++;
	    } else {
		errno = EIO;
	    }
	    rtnval = 0;
	} else if (DBUG_SETJMP (errjump)) {
	    DBUG_PRINT ("dblr", ("took longjmp back into dbl_read"));
	    rtnval = -1;
	} else {
	    bytesin = 0;
	    rtnval = size;
	    DBUG_PRINT ("dblr", ("%ld bytes from last time", lastrsize));
	    while (bytesin < size) {
		if (lastrsize == 0) {
		    DBUG_PRINT ("dblr", ("get next shared buffer"));
		    lastrbuf = getfullbuf ();
		    DBUG_PRINT ("dblr", ("got buffer %d", lastrbuf));
		    if (lastrbuf < 0 || !(lastrsize = *(segs[lastrbuf].len))) {
			DBUG_PRINT ("dblr", ("found end of archive"));
			nomore++;
			errno = *errnop;
			rtnval = bytesin;
			break;
		    }
		    lastrpos = segs[lastrbuf].shmaddr;
		}
		DBUG_PRINT ("dblr", ("%ld bytes left in buffer", lastrsize));
		bytesleft = min (lastrsize, size - bytesin);
		DBUG_PRINT ("dblr", ("xfer %u bytes", bytesleft));
		(VOID) s_memcpy (buf + bytesin, lastrpos, (int) bytesleft);
		bytesin += bytesleft;
		lastrsize -= bytesleft;
		lastrpos += bytesleft;
		if (lastrsize == 0) {
		    DBUG_PRINT ("dblr", ("done with buffer %d", lastrbuf));
		    freebuf (lastrbuf);
		}
	    }
	}
    }
    DBUG_PRINT ("errno", ("errno = %d", errno));
    DBUG_PRINT ("dblr", ("%d bytes read", rtnval));
    DBUG_RETURN (rtnval);
}

/*
 *	dbli_wflush - automatic flush generated by sending an RPC message
 *	to the child while writing.
 */

static VOID dbli_wflush ()
{
    DBUG_ENTER ("dbli_wflush");
    if (lastwsize != 0) {
	*(segs[lastwbuf].len) = segsize - lastwsize;
	fullbuf (lastwbuf);
	lastwsize = 0;
	DBUG_PRINT ("dblf", ("%ld bytes left in buffer", lastwsize));
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	dbl_flush    flush any still queued data out to the other side
 *
 *  SYNOPSIS
 *
 *	int dbl_flush ()
 *
 *  DESCRIPTION
 *
 *	Flush any still queued data out to the child process, and then
 *	wait for the child to finish and exit.
 *
 *	Under System V, if the child is still running when we do the
 *	wait, then the wait fails the first time because the child
 *	causes us to catch a SIGCLD signal when it exits.  After the
 *	SIGCLD handler returns, the wait then returns with errno set
 *	to EINTR.  We ignore this case and retry the wait, which again
 *	fails, and returns with errno set to ECHILD.  This is our clue
 *	that the child is done and has exited, so we don't do any more
 *	waits.
 *
 *	Under BSD style systems, if the child is still running when
 *	we do the wait, then the wait fails the first time because the
 *	child causes us to catch a SIGCLD signal when it exits.  After
 *	the SIGCLD handler returns, the wait then returns with errno
 *	set to ECHILD.  We never see a failed wait with EINTR.  At this
 *	point, we assume the child is done and don't do any more waits.
 *
 *	Thus, this code should work properly under both System V and
 *	BSD style signals.
 *
 */

int dbl_flush ()
{
    BOOLEAN trywait;

    DBUG_ENTER ("dbl_flush");
    if (dodb && cpid != 0 && writing) {
	if (msg == 0 || DBUG_SETJMP (errjump)) {
	    DBUG_RETURN (0);
	}
	dbli_wflush ();
	lastwbuf = getfreebuf ();
	*(segs[lastwbuf].len) = 0;
	fullbuf (lastwbuf);
	trywait = TRUE;
	while (trywait && s_wait (&status) == -1) {
	    DBUG_PRINT ("wait", ("wait failed, errno = %d", errno));
	    switch (errno) {
		case EINTR:
		    DBUG_PRINT ("wait", ("interrupted system call, try again"));
		    break;
		case ECHILD:
		    DBUG_PRINT ("wait", ("child has gone away"));
		    trywait = FALSE;
		    break;
		default:
		    DBUG_PRINT ("wait", ("bad wait!"));
		    bru_message (MSG_REAP);
		    trywait = FALSE;
		    break;
	    }
	}
	if (*(rval) != RV_OK && *(rval) != RV_EOF) {
	    bru_message (MSG_SLVERR, *rval);
	}
	(VOID) msgctl (msg, IPC_RMID, (struct msqid_ds *) NULL);
    }
    DBUG_RETURN (0);
}

/*
 *  FUNCTION
 *
 *	dbl_write    write the given amount of data out
 *
 *  SYNOPSIS
 *
 *	int dbl_write (buf, size)
 *	char *buf;
 *	UINT size;
 *
 *  DESCRIPTION
 *
 *	This routine takes the place of the normal write system call,
 *	using the same arguments.  If double buffering is not turned
 *	on, then it operates in transparent mode and simply calls the
 *	low level write routine to write the data directly.  If double
 *	buffering is turned on, then it copies the data to as many
 *	shared memory buffers as necessary, passing each one to the
 *	child process as it is filled.
 *
 *  NOTES
 *
 *	This routine is called only in the parent process.
 *
 */

int dbl_write (buf, size)
char *buf;
UINT size;
{
    UINT bytesout;
    UINT bytesleft;
    int rtnval;
    static char *lastwpos;

    DBUG_ENTER ("dbl_write");
    if (!dodb) {
	DBUG_PRINT ("dblw", ("transparent mode, just write the data"));
	rtnval = dbli_localwrite (buf, size);
    } else {
	DBUG_PRINT ("dblw", ("write using shared memory buffers"));
	if (DBUG_SETJMP (errjump)) {
	    DBUG_PRINT ("dblw", ("took longjmp back into dbl_write"));
	    rtnval = -1;
	} else {
	    bytesout = 0;
	    rtnval = size;
	    while (bytesout < size) {
		if (lastwsize == 0) {
		    DBUG_PRINT ("dblw", ("get next shared buffer"));
		    lastwbuf = getfreebuf ();
		    DBUG_PRINT ("dblw", ("got shared buffer %d", lastwbuf));
		    lastwpos = segs[lastwbuf].shmaddr;
		    lastwsize = segsize;
		}
		DBUG_PRINT ("dblw", ("%ld bytes left in buffer", lastwsize));
		bytesleft = min (lastwsize, size - bytesout);
		DBUG_PRINT ("dblw", ("xfer %u bytes", bytesleft));
		(VOID) s_memcpy (lastwpos, buf + bytesout, (int) bytesleft);
		lastwpos += bytesleft;
		bytesout += bytesleft;
		lastwsize -= bytesleft;
		if (lastwsize == 0) {
		    *(segs[lastwbuf].len) = segsize;
		    DBUG_PRINT ("dblw", ("pass buffer %d to child", lastwbuf));
		    fullbuf (lastwbuf);
		}
	    }
	}
    }
    DBUG_PRINT ("dblw", ("%d bytes written", rtnval));
    DBUG_RETURN (rtnval);
}

/*
 *	Write-through routines for direct files.
 */

static int dbli_localwrite (buf, size)
char *buf;
UINT size;
{
    int rtnval;

    DBUG_ENTER ("dbli_localwrite");
    rtnval = (*uswrite) (afile.fildes, buf, size);
    DBUG_RETURN (rtnval);
}

static int dbli_localread (buf, size)
char *buf;
UINT size;
{
    int rtnval;

    DBUG_ENTER ("dbli_localread");
    errno = 0;
    rtnval = (*usread) (afile.fildes, buf, size);
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	setupcopy    allocate a copy buffer
 *
 *  SYNOPSIS
 *
 *	static BOOLEAN setupcopy ();
 *
 *  DESCRIPTION
 *
 *	If we happen to be on a machine which can't access shared memory
 *	from the I/O subsystem (stupid Berkeley VM, anyway), then we
 *	have to do another copy.  This copy is kept in copybuf.
 *
 *	If there is no "shmcopy" boolean flag set in brutab for the
 *	archive device being used, then we always assume that we can
 *	do I/O directly to and from shared memory.  This is true of
 *	all but a very small number of systems.  Since the copy 
 *	operation adds overhead, we only penalize the systems that
 *	require copy mode.  Others run at full speed.
 *
 *	The first time this fails with the correct error condition
 *	(errno == EFAULT) indicating such I/O is not allowed,
 *	we automatically attempt to switch to a copy mode where
 *	the data is copied to and from shared memory as necessary.
 *	We also issue a warning for the user to put "shmcopy" in
 *	the appropriate brutab entry, because this dynamic workaround
 *	is not always successful, particularly on reads where the
 *	driver may read some data from the archive before it discovers
 *	that it can't do I/O to shared memory, and aborting the I/O.
 *
 *	This function is called only in the child process.
 *
 */

static BOOLEAN setupcopy ()
{
    BOOLEAN result = TRUE;

    DBUG_ENTER ("setupcopy");
    DBUG_PRINT ("shmcopy", ("allocate copy buffer of size %d", segsize));
    if ((copybuf = (char *) get_memory ((UINT) segsize, FALSE)) == NULL) {
	bru_message (MSG_MALLOC, (UINT) segsize);
	result = FALSE;
    }
    DBUG_RETURN (result);
}

/*
 *	Internal recovery routines for implementing the correct behaviour.
 *	Basically, on writes the child is responsible for switching volumes
 *	and keeping everything sane.  On reads, the double buffering goes 
 *	away on each switch and the parent is responsible for switching
 *	volumes and keeping everything sane.
 */

static VOID dbli_insync ()
{
    DBUG_ENTER ("dbli_insync");
    chend ();
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	copyin    copy from archive device to shared memory buffers
 *
 *  SYNOPSIS
 *
 *	static int copyin ();
 *
 *  DESCRIPTION
 *
 *	This function is called only by the child process, and is used during
 *	archive reads to copy data from the archive device to the shared
 *	memory buffers, for future reading by the parent.  It hangs in a loop
 *	copying data until a termination condition of some sort is found.
 *
 */

static int copyin ()
{
    long cnt;
    long nread;
    int i;
    char *indata;
    int rtnval;

    DBUG_ENTER ("copyin");
    rtnval = RV_INIT;
    while (rtnval == RV_INIT) {
	i = getfreebuf ();
	DBUG_PRINT ("rbuf", ("read in shared buffer %d", i));
	cnt = 0;
	indata = segs[i].shmaddr;
	if (copybuf != NULL) {
	    DBUG_PRINT ("shmcopy", ("setup for read into copy buffer"));
	    indata = copybuf;
	}
	while (cnt < segsize) {
	    errno = 0;
	    if ((nread = (*usread) (afile.fildes, indata, tpblk)) == tpblk) {
		indata += nread;
		cnt += nread;
	    } else if (errno == EINTR) {
		DBUG_PRINT ("ibuf", ("read was interrupted by signal"));
		continue;
	    } else if ((errno == EFAULT) && (copybuf == NULL)) {
		DBUG_PRINT ("shmcopy", ("no I/O to shared memory!"));
		bru_message (MSG_SHMCOPY);
		if (!setupcopy ()) {
		    rtnval = RV_RFAIL;
		    *errnop = errno;
		    DBUG_PRINT ("errno", ("errno = %d", errno));
		    break;
		} else {
		    indata = copybuf;
		}
	    } else {
#if 0		/* Currently recovery done by parent??? */
		DBUG_PRINT ("inb", ("see if recovery is possible"));
		if (!(*inerr) (nread, errno)) {
		    DBUG_PRINT ("inb", ("recovery failed, give up"));
		    rtnval = RV_RFAIL;
		    break;
		}
#else
		rtnval = RV_RFAIL;
		*errnop = errno;
		DBUG_PRINT ("errno", ("errno = %d", errno));
		break;
#endif
	    }
	}
	if (cnt > 0) {
	    if (copybuf != NULL) {
		DBUG_PRINT ("shmcopy", ("copy copybuf to shm buffer"));
		(VOID) s_memcpy (segs[i].shmaddr, copybuf, (int) cnt);
	    }
	    *(segs[i].len) = cnt;
	    fullbuf (i);
	}
    }
    dbli_insync ();
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	copyout    copy from shared memory buffers to archive device
 *
 *  SYNOPSIS
 *
 *	static int copyout ();
 *
 *  DESCRIPTION
 *
 *	This function is called only by the child process, and is used during
 *	archive writes to copy data, placed in the shared memory buffers by
 *	the parent, out to the archive device.  It hangs in a loop copying
 *	data until a termination condition of some sort is found.
 *
 *	The return value from this function is the status value returned
 *	to the parent process by the child process.
 *
 */

static int copyout ()
{
    int i;
    int rtnval;

    DBUG_ENTER ("copyout");
    rtnval = RV_INIT;
    while (rtnval == RV_INIT) {
	if ((i = getfullbuf ()) < 0) {
	    rtnval = RV_WFAIL;
	} else if (*(segs[i].len) == 0) {
	    DBUG_PRINT ("eod", ("writer found end of data, normal return"));
	    rtnval = RV_OK;
	    freebuf (i);
	} else {
	    rtnval = copyoutbuf (i);
	    freebuf (i);
	}
    }
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	copyoutbuf    copy out one buffer to archive device
 *
 *  SYNOPSIS
 *
 *	copyoutbuf (index)
 *	int index;
 *
 *  DESCRIPTION
 *
 *	This function is called in the child process to copy one buffer
 *	of data out to the archive device.  The shared memory buffer given
 *	by "index" is written to the archive device.
 *
 *	If the first write fails with errno EFAULT, then this generally
 *	indicates that we cannot do I/O directly from shared memory, a
 *	common failing on some systems.  In this case, we allocate a
 *	separate buffer, copy the shared data to this buffer, and then
 *	do the writes from this copy of the buffer.  Subsequent writes
 *	always do the copy.
 *
 *	If the write fails then we also attempt to do recovery by
 *	calling the user supplied (in this case "recover" in blocks.c)
 *	routine to fix things up.
 *
 *	As with writing without double buffering, we do not accept
 *	any partial writes.  Either an entire buffer is written (with
 *	or without error recovery), or the write fails.  Note that
 *	each shared memory segment can hold one or more buffers.
 *
 *	For each write, there are four possible conditions:
 *
 *	1.	The write is totally successful.
 *	2.	The write was interrupted by a system call (try again)
 *	3.	The write triggers copy mode (try again or quit)
 *	4.	The write triggers error recovery (try again or quit)
 *
 *  RETURNS
 *
 *	Returns RV_INIT if all goes well.  Any other return value
 *	signals an unrecoverable error of some sort.
 *
 *  NOTES
 *
 *	Just prior to the actual write, we patch the buffer with the
 *	current volume number and update the checksum.  The parent
 *	has already done this, but the child may have switched volumes
 *	in the meantime, so we get to do it again.
 *
 */

static int copyoutbuf (index)
int index;
{
    long bytesleft;
    char *outdata;
    int nwritten;
    int rtnval;

    DBUG_ENTER ("copyoutbuf");
    DBUG_PRINT ("obuf", ("copy out shared memory buffer %d", index));
    rtnval = RV_INIT;
    bytesleft = *(segs[index].len);
    outdata = segs[index].shmaddr;
    if (copybuf != NULL) {
	DBUG_PRINT ("shmcopy", ("copy shm buffer %d to copybuf", index));
	(VOID) s_memcpy (copybuf, outdata, (int) bytesleft);
	outdata = copybuf;
    }
    while (bytesleft > 0) {
	errno = 0;
	patch_buffer ((union blk *) outdata, BLOCKS (tpblk));
	if ((nwritten = (*uswrite) (afile.fildes, outdata, tpblk)) == tpblk) {
	    outdata += nwritten;
	    bytesleft -= nwritten;
	} else if (errno == EINTR) {
	    DBUG_PRINT ("obuf", ("write was interrupted by signal"));
	    continue;
	} else if ((errno == EFAULT) && (copybuf == NULL)) {
	    DBUG_PRINT ("shmcopy", ("no I/O from shared memory!"));
	    bru_message (MSG_SHMCOPY);
	    if (!setupcopy ()) {
		rtnval = RV_WFAIL;
		break;
	    } else {
		DBUG_PRINT ("shmcopy", ("copy shm buffer %d to copybuf", index));
		(VOID) s_memcpy (copybuf, outdata, (int) bytesleft);
		outdata = copybuf;
	    }
	} else {
	    DBUG_PRINT ("outb", ("see if recovery is possible"));
	    if (!(*outerr) (nwritten, errno)) {
		DBUG_PRINT ("outb", ("recovery failed, give up"));
		rtnval = RV_WFAIL;
		break;
	    }
	}
    }
    if (bytesleft < 0) {
	bru_message (MSG_BUG, "copyoutbuf");
	rtnval = RV_WFAIL;
    }
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	attachvars    create and attach shared variables segment
 *
 *  SYNOPSIS
 *
 *	static BOOLEAN attachvars ();
 *
 *  DESCRIPTION
 *
 *	Create and attach a shared memory segment to contain the
 *	miscellaneous shared variables.  Return 0 on error, 1
 *	on success.
 *
 *  NOTES
 *
 *	The size of the shared variables region must match up with
 *	the number of shared variables used.  At one time, a hard
 *	to find bug was introduced with the addition of another shared
 *	variable (from 2 to 3 plus lengths), when the size below was
 *	left at 2.  An attempt on SCO Xenix 386 to access the last
 *	length (which was outside the allocated shared segment) caused
 *	a memory fault.  To avoid such problems until this code
 *	is cleaned up somehow, we just allocate space for at least
 *	32 shared variables plus the lengths.
 *
 */

static BOOLEAN attachvars ()
{
    int nbytes;
    BOOLEAN result = FALSE;
    int shmid;

    DBUG_ENTER ("attachvars");
    nbytes = (32 * sizeof (long)) + (nsegs * sizeof (long));
    DBUG_PRINT ("shm", ("allocate vars segment of size %d", nbytes));
    shmid = shmget (IPC_PRIVATE, (int) nbytes, IPC_CREAT | 0600);
    if (shmid == -1) {
	bru_message (MSG_SHMGET, (int) (nbytes / 1024));
    } else if ((misc.shmaddr = shmat (shmid, 0, 0)) == SHMATNULL) {
	bru_message (MSG_SHMAT);
    } else {
	DBUG_PRINT ("shmat", ("attached shmseg at %x", misc.shmaddr));
	result = TRUE;
    }
    if (shmid != -1) {
	(VOID) shmctl (shmid, IPC_RMID, (struct shmid_ds *) NULL);
    }
    DBUG_RETURN (result);
}


/*
 *	Only print the double buffering message once per run.  This
 *	function may get called several times during a single run,
 *	if more than on major mode is specified.
 */

static int attachbuffers ()
{
    int i;
    int result = 1;
    int shmid;
    static BOOLEAN msgprinted = FALSE;

    DBUG_ENTER ("attachbuffers");
    for (i = 0; i < nsegs; i++) {
	DBUG_PRINT ("shm", ("allocate segment %d of size %d", i, segsize));
	shmid = shmget (IPC_PRIVATE, segsize, IPC_CREAT | 0600);
	if (shmid == -1) {
	    if (i == 0) {
		bru_message (MSG_SHMGET, segsize / 1024);
		DBUG_RETURN (0);
	    } else {
		DBUG_PRINT ("shmlim", ("hit number of segments limit %d", i));
		DBUG_PRINT ("shmlim", ("limited by errno = %d", errno));
		break;
	    }
	}
	segs[i].shmaddr = shmat (shmid, 0, 0);
	(VOID) shmctl (shmid, IPC_RMID, (struct shmid_ds *) NULL);
	if (segs[i].shmaddr == SHMATNULL) {
	    if (i == 0) {
		bru_message (MSG_SHMAT);
		DBUG_RETURN (0);
	    } else {
		DBUG_PRINT ("shmlim", ("hit number of segments limit %d", i));
		DBUG_PRINT ("shmlim", ("limited by errno = %d", errno));
		break;
	    }
	}
	DBUG_PRINT ("shmat", ("attached shmseg at %x", segs[i].shmaddr));
    }
    asegs = i;
    if (!msgprinted && flags.vflag > 3) {
	voutput ("double buffering to %s, ", afile.fname);
	voutput ("%d shared memory segments, each %dKb",
		 asegs, segsize / 1024);
	vflush ();
	msgprinted = TRUE;
    }
    DBUG_PRINT ("shm", ("successfully attached %d memory segments", asegs));
    DBUG_RETURN (result);
}


/*
 *  FUNCTION
 *
 *	movebrk    temporarily move brk value
 *
 *  SYNOPSIS
 *
 *	static VOID movebrk (room)
 *	int room;
 *
 *  DESCRIPTION
 *
 *	Some systems use a particularly bad selection algorithm for
 *	picking where to put shared segments. In particular, they
 *	just add some 'generic' amount to the last break value and
 *	attach there (assuming you won't need to allocate more than the
 *	space left).  To avoid this problem, we temporarily attempt to
 *	move our break value up by two I/O buffer sizes (one for the
 *	I/O buffer that will be allocated later, and one for the
 *	copy buffer, if needed), plus some slop for future malloc's.
 *
 *	Note that we don't report any errors, if we have run out
 *	of memory we will find out soon enough when we do the
 *	malloc's.  If we didn't need to do this in the first place,
 *	then trying it and not reporting any errors doesn't hurt
 *	anything.  We always give back whatever we might have
 *	allocated.
 *
 *	Also, because we might need to call malloc somewhere between
 *	moving the brk value up and then moving it back down, such
 *	as a hidden malloc in stdio to print an error message or
 *	something, we try to provide for some workspace by malloc'ing
 *	and then immediately freeing some reasonably large piece of
 *	memory.  We try the desired value first (WORKSPACE), and then
 *	successively try halving it until we either succeed or fall
 *	below the threshold to stop trying (128 bytes).
 *
 */

static VOID movebrk (room)
int room;
{
    static BOOLEAN gotbrk = FALSE;
    UINT nbytes;
    char *workspace;

    DBUG_ENTER ("movebrk");
    if (room > 0 && !gotbrk) {
	for (nbytes = WORKSPACE; nbytes > 128; nbytes /= 2) {
	    DBUG_PRINT ("wrkspc", ("try to get workspace of %u", nbytes));
	    if ((workspace = (char *) get_memory (WORKSPACE, FALSE)) != NULL) {
		s_free (workspace);
		break;
	    }
	}
	DBUG_PRINT ("sbrk", ("temporarily move brk up by %d", room));
	gotbrk = (sbrk (room) != (char *) -1);
    } else if (room < 0 && gotbrk) {
	DBUG_PRINT ("sbrk", ("give back %d bytes", room));
	(VOID) sbrk (room);
	gotbrk = FALSE;
    }
    DBUG_VOID_RETURN;
}

    
/*
 *	This routine goes through the rather gross setup needed to create the
 *	buffers and message queue.  Since we can get the OS to automatically
 *	dispose of the buffers for us on exit, we do so.  Too bad messages
 *	don't have the same property.
 */

static int dbli_setupbuf ()
{
    int i;
    long *shvar;
    
    DBUG_ENTER ("dbli_setupbuf");
    movebrk (ROOM);
    if (!attachvars ()) {
	DBUG_RETURN (1);
    }
    if (!attachbuffers ()) {
	DBUG_RETURN (1);
    }
    /*
     *  The shared variables section is structured as follows:
     *
     *	struct variables {
     *		long rval;		(return value)
     *		long volume;		(current volume)
     *		int errno;		(last errno)
     *		(padding)		(only if sizeof(int) < sizeof (long))
     *		long len[asegs];	(buffer lengths)
     *	};
     *
     */
    shvar = (long *) misc.shmaddr;
    rval = shvar++;
    volumep = shvar++;
    errnop = (int *) shvar++;
    for (i = 0; i < asegs; i++) {
	segs[i].len = shvar++;
    }
    
    if ((msg = msgget (IPC_PRIVATE, IPC_CREAT | 0660)) == -1) {
	bru_message (MSG_MSGGET);
	DBUG_RETURN (1);
    }
    for (i = 0; i < asegs; i++) {
	freebuf (i);
    }
    movebrk (-ROOM);
    DBUG_RETURN (0);
}

/*
 *	The following routines deal with the message queue, and implement the
 *	buffer management protocol.  At initialization, messages for the number
 *	of available buffers are placed on the queue as empty buffers.  The
 *	reader simply blocks waiting for one of these messages.  When one arrives,
 *	he fills the buffer and then sends a buffer full message.  The writer
 *	blocks on the queue waiting for buffer full messages.  As he gets each
 *	one, he writes the buffer and then enters it back on the queue as empty.
 *	Thus, once started, the management is self-sustaining.  A critical note:
 *	this code relies on the message queue staying ordered.
 */

static int sendmsg (mdes, pbuf, sz, mflags)
int mdes;
struct brumsg *pbuf;
int sz;
int mflags;
{
    DBUG_ENTER ("sendmsg");
    DBUG_PRINT ("msg", ("%s sends message", (cpid ? "parent" : "child")));
    DBUG_PRINT ("msg", ("message type %d", pbuf -> m_type));
    DBUG_PRINT ("msg", ("message action %d", pbuf -> m_action));
    while (msgsnd (mdes, (struct msgbuf *) pbuf, sz, mflags) == -1) {
	if (errno == EINTR) {
	    continue;
	} else {
	    DBUG_RETURN (-1);
	}
    }
    DBUG_RETURN (0);
}

static int recvmsg (mdes, pbuf, sz, type, mflags)
int mdes;
struct brumsg *pbuf;
int sz;
long type;
int mflags;
{
    DBUG_ENTER ("recvmsg");
    DBUG_PRINT ("msg", ("%s waiting on message", (cpid ? "parent" : "child")));
    DBUG_PRINT ("msg", ("waiting on message type %ld", type));
    while (msgrcv (mdes, (struct msgbuf *) pbuf, sz, type, mflags) == -1) {
	if (errno == EINTR) {
	    continue;
	} else {
	    DBUG_RETURN (-1);
	}
    }
    DBUG_PRINT ("msg", ("%s received msg", (cpid ? "parent" : "child")));
    DBUG_PRINT ("msg", ("message type %d", pbuf -> m_type));
    DBUG_PRINT ("msg", ("message action %d", pbuf -> m_action));
    DBUG_RETURN (0);
}

/*
 *	trashmsg - receive all messages on the queue intended for us
 *	and throw them away - if the parent queued an RPC call,
 *	then this ensures he wakes up.
 */

static VOID trashmsg ()
{
    struct brumsg pbuf;
    
    DBUG_ENTER ("trashmsg");
    while (msgrcv (msg, (struct msgbuf *) &pbuf, MSGSZ, (long) FREEBUF, IPC_NOWAIT) != -1) {
	switch (pbuf.m_action) {
	    /* do nothing */
	}
    }
    pbuf.m_type = CALLDONE;
    (VOID) sendmsg (msg, &pbuf, MSGSZ, 0);
    DBUG_VOID_RETURN;
}

/*
 *	External routine to allow the parent in a write situation to
 *	cause an action in the child (after all the data has been written,
 *	of course).
 */

#define GONE()	if (*(rval)!=0){(VOID)s_wait(&status);DBUG_RETURN(0);}

int dbl_rpcdown (func, arg)
VOID (*func) ();
long arg;
{
    struct brumsg pbuf;
    
    DBUG_ENTER ("dbl_rpcdown");
    GONE ();
    dbli_wflush ();
    if (writing) {
	pbuf.m_type = DOBUF;
    } else {
	pbuf.m_type = FREEBUF;
    }
    pbuf.m_action = CALLPROC;
    pbuf.m_proc = func;
    pbuf.m_arg = arg;
    GONE ();
    deathok = 1;
    if (sendmsg (msg, &pbuf, MSGSZ, 0) == -1) {
	bru_message (MSG_MSGSND);
	DBUG_RETURN (-1);
    }
    GONE ();
    while (recvmsg (msg, &pbuf, MSGSZ, (long) CALLDONE, IPC_NOWAIT) == -1) {
	if (errno == ENOMSG) {
	    GONE ();
	    s_sleep (1);
	} else {
	    bru_message (MSG_MSGRCV);
	    DBUG_RETURN (-1);
	}
    }
    deathok = 0;
    DBUG_RETURN (0);
}

/*
 *	chend - send a child end message to the other side.
 */

static VOID chend ()
{
    struct brumsg pbuf;
    
    DBUG_ENTER ("chend");
    pbuf.m_type = DOBUF;
    pbuf.m_action = CHEND;
    if (sendmsg (msg, &pbuf, MSGSZ, 0) == -1) {
	DBUG_LONGJMP (errjump, RV_SYSERR);
    }
    DBUG_VOID_RETURN;
}

/*
 *  FUNCTION
 *
 *	fullbuf    release a full buffer for the other side
 *
 *  SYNOPSIS
 *
 *	static VOID fullbuf (bufindex)
 *	int bufindex;
 *
 *  DESCRIPTION
 *
 *	Release a full buffer for the other side.
 *
 *  NOTES
 *
 *	This routine is called in the parent.
 *
 */

static VOID fullbuf (bufindex)
int bufindex;
{
    struct brumsg pbuf;
    
    DBUG_ENTER ("fullbuf");
    DBUG_PRINT ("fullbuf", ("send full buffer %d to other side", bufindex));
    pbuf.m_type = pbuf.m_action = DOBUF;
    pbuf.m_buf = bufindex;
    if (sendmsg (msg, &pbuf, MSGSZ, 0) == -1) {
	DBUG_PRINT ("fullbuf", ("message send failed, longjmp outta here"));
	DBUG_LONGJMP (errjump, RV_SYSERR);
    }
    DBUG_VOID_RETURN;
}

/*
 *  FUNCTION
 *
 *	freebuf    release a now empty buffer for the other side
 *
 *  SYNOPSIS
 *
 *	static VOID freebuf (bufindex)
 *	int bufindex;
 *
 *  DESCRIPTION
 *
 *	Notify the other process that a buffer is now free.
 *
 */

static VOID freebuf (bufindex)
int bufindex;
{
    struct brumsg pbuf;
    
    DBUG_ENTER ("freebuf");
    DBUG_PRINT ("freebuf", ("free shared memory buffer %d", bufindex));
    pbuf.m_type = pbuf.m_action = FREEBUF;
    pbuf.m_buf = bufindex;
    if (sendmsg (msg, &pbuf, MSGSZ, 0) == -1) {
	DBUG_PRINT ("freebuf", ("message send failed, longjmp outta here"));
	DBUG_LONGJMP (errjump, RV_SYSERR);
    }
    DBUG_VOID_RETURN;
}

/*
 *	getcache - get the next buffer from the cache, and execute any
 *	queued procedure calls.
 */

static int getcache ()
{
    int rtnval;

    DBUG_ENTER ("getcache");
    if (ncache > curcache) {
	rtnval = bufcache[curcache++];
	DBUG_PRINT ("gch", ("return buffer %d from cache", rtnval));
	DBUG_PRINT ("gch", ("buffer has %d bytes", *(segs[rtnval].len)));
    } else if (issrpc) {
	issrpc = 0;
	if (srpc.m_action == CHEND) {
	    errno = srpc.m_err;
	    rtnval = -2;
	} else {
	    dorpc (&srpc);
	    rtnval = -1;
	}
    } else {
	rtnval = -1;
    }
    DBUG_RETURN (rtnval);
}

/*
 *	dorpc - call the specified procedure in the child.
 */

static VOID dorpc (pbuf)
struct brumsg *pbuf;
{
    DBUG_ENTER ("dorpc");
    DBUG_PRINT ("rpc", ("rpc call"));
    (*pbuf -> m_proc) (pbuf -> m_arg);
    switch (pbuf -> m_action) {
	case CALLPROC: 
	    pbuf -> m_type = CALLDONE;
	    break;
    }
    if (sendmsg (msg, pbuf, MSGSZ, 0) == -1) {
	DBUG_LONGJMP (errjump, RV_SYSERR);
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	getfullbuf    get a buffer full of data
 *
 *  SYNOPSIS
 *
 *	static int getfullbuf ()
 *
 *  DESCRIPTION
 *
 *	Get the index of the next buffer that is ready to be processed.
 *
 *	This routine attempts to add some hysteresis to the output,
 *	to help ensure maximum overlap between input and output.
 *	This is done by not starting to write buffers until at least
 *	half are full, which helps if the reader is slower than the
 *	writer, making the output flow stop less often.  Otherwise,
 *	the writer could just chase the reader around the buffer
 *	list, having to wait on each new buffer.
 *
 */

static int getfullbuf ()
{
    struct brumsg pbuf;
    int half = asegs / 2;
    int rtnval;
    
    DBUG_ENTER ("getfullbuf");
    if ((rtnval = getcache ()) >= 0) {
	DBUG_RETURN (rtnval);
    } else if (rtnval == -2) {
	DBUG_RETURN (-1);
    }
    ncache = 0;
    curcache = 0;
    while (ncache < half) {
	if (chendseen) {
	    DBUG_RETURN (-1);
	}
	if (recvmsg (msg, &pbuf, MSGSZ, (long) DOBUF, 0) == -1) {
	    DBUG_LONGJMP (errjump, RV_SYSERR);
	}
	switch (pbuf.m_action) {
	    case CHEND: 
		if (ncache == 0) {
		    errno = pbuf.m_err;
		    DBUG_RETURN (-1);
		} else {
		    DBUG_PRINT ("gfb", ("cached CHEND"));
		    issrpc = 1;
		    srpc = pbuf;
		    goto out;
		}
		/*NOTREACHED*/
		break;
	    case CALLPROC: 
		if (ncache == 0) {
		    dorpc (&pbuf);
		    continue;
		} else {
		    DBUG_PRINT ("gfb", ("cached RPC"));
		    issrpc = 1;
		    srpc = pbuf;
		    goto out;
		}
		/*NOTREACHED*/
		break;
	    case DOBUF: 
		bufcache[ncache] = pbuf.m_buf;
		DBUG_PRINT ("gfb", ("cacheing buffer %d", pbuf.m_buf));
		DBUG_PRINT ("gfb", ("length %d", *(segs[pbuf.m_buf].len)));
		ncache++;
		if (*(segs[pbuf.m_buf].len) == 0) {
		    goto out;
		}
		break;
	}
    }
    out: 
    rtnval = getcache ();
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	getfreebuf    get the next empty buffer to be filled
 *
 *  SYNOPSIS
 *
 *	static int getfreebuf ()
 *
 *  DESCRIPTION
 *
 *	Get the index of the next available shared memory buffer
 *	than can be filled with data.
 *
 *  NOTES
 *
 *	Called in the parent process.
 *
 */

static int getfreebuf ()
{
    struct brumsg pbuf;
    int bufindex = -1;
    
    DBUG_ENTER ("getfreebuf");
    DBUG_PRINT ("gfb", ("get next empty buffer to fill"));
    while (bufindex == -1) {
	if (*(rval) != 0) {
	    (VOID) s_wait (&status);
	    DBUG_LONGJMP (errjump, (int) *(rval));
	}
	if (recvmsg (msg, &pbuf, MSGSZ, (long) FREEBUF, 0) == -1) {
	    DBUG_LONGJMP (errjump, RV_SYSERR);
	}
	DBUG_PRINT ("gfb", ("got message; action %d", pbuf.m_action));
	switch (pbuf.m_action) {
	    case FREEBUF: 
		bufindex = pbuf.m_buf;
		break;
	    case CALLPROC: 
		dorpc (&pbuf);
		break;
	    default:
		DBUG_LONGJMP (errjump, RV_SYSERR);
		break;
	}
    }
    DBUG_PRINT ("gfb", ("got free buffer %d", bufindex));
    DBUG_RETURN (bufindex);
}


VOID dbl_setvol (volume)
int volume;
{
    DBUG_ENTER ("dbl_setvol");
    DBUG_PRINT ("vol", ("set shared volume number to %d", volume));
    *volumep = (long) volume;
    DBUG_VOID_RETURN;
}


int dbl_getvol ()
{
    DBUG_ENTER ("get_vol");
    DBUG_RETURN ((int) *volumep);
}

#else

#if defined(__STDC__)
static int dummy;	/* avoid "empty translation unit" warnings */
#endif

#endif	/* HAVE_SHM */
