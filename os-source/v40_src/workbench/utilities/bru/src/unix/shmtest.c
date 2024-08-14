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
 ************************************************************************
 */


/*
 *	This program can be used to dynamically determine some of the
 *	system enforced limits of your shared memory system.
 */

#include "autoconfig.h"

#include <stdio.h>

#if HAVE_SHM

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#if (xenix && !M_I386)			/* xenix and !i386 (should be i286?) */
extern far char *shmat ();
#define SHMATNULL ((far char *) -1)
#else
#if defined(__STDC__) && !defined(_STYPES)	/* prototypes and SVR4 */
extern void *shmat (int, void *, int);
#define SHMATNULL ((char *) -1)
#else
extern char *shmat ();
#define SHMATNULL ((char *) -1)
#endif
#endif

static int shmseg;	/* Derived max number of segments */
static int shmmax;	/* Derived max size of each segment */
static long shmall;	/* Derived max total shared memory */

/*
 *	Find the maximum size of a shared memory segment, to the
 *	nearest 4 bytes.  First find the upper and lower bounds to the
 *	nearest power of two, then use a binary search to narrow
 *	the difference between the lower and upper bounds to less
 *	than 4.  This is the straightforward, quick and dirty
 *	solution.  There is probably an elegant 5 line recursive
 *	solution waiting to be written...
 *
 *	Once we have found the real max value, we need to apply a
 *	kludge which subtracts 2Kb from the real value.  This ensures
 *	that the value used in the shmget() call above will always
 *	be in range.  Note that the value used in shmget() depends
 *	on the current buffer size plus about 4 or 8 bytes.
 */


int maxshmsize ()
{
    int lowval;
    int testval;
    int highval;
    static int shmsizeok ();

    for (testval = 1; testval > 0; testval *= 2) {
	highval = testval;
	lowval = testval / 2;
	if (!shmsizeok (testval)) {
	    break;
	}
    }
    if (testval < 0) {
	lowval = highval;
	highval = testval - 1;
    }
    while ((highval - lowval) > 4) {
	testval = (highval / 2) + (lowval / 2);
	if (shmsizeok (testval)) {
	    lowval = testval;
	} else {
	    highval = testval;
	}
    }
    return (lowval);
}

static int shmsizeok (size)
int size;
{
    int shmid;
    int result = 0;

    if ((shmid = shmget (IPC_PRIVATE, size, IPC_CREAT | 0600)) >= 0) {
	result = 1;
	shmctl (shmid, IPC_RMID, (struct shmid_ds  *) NULL);
    }
    return (result);
}

/*
 *	Try to determine the value of the kernel parameter SHMMNI,
 *	which is the total number of shared memory segments for
 *	the system.  Note that if any other process is currently
 *	using shared memory, this count will be too small by the
 *	number of shared memory segments in use.
 *
 *	This works despite the per user limit because we only
 *	call shmget(), and don't try to actually attach the
 *	segments.
 *
 *	Declare the shmid[] array as static rather than auto
 *	to avoid stack overflow on the #@$%&*%@ Intel 286. 
 *
 */

int findshmmni (size)
int size;
{
    int nsegs;
    int shmid;
    int maxsegs;
    static int shmids[4096];

    nsegs = 0;
    do {
	shmids[nsegs] = shmget (IPC_PRIVATE, size, IPC_CREAT | 0600);
    } while ((shmids[nsegs] != -1) && (++nsegs < 4096));
    maxsegs = nsegs;
    while (--nsegs >= 0) {
	shmctl (shmids[nsegs], IPC_RMID, (struct shmid_ds  *) NULL);
    }
    return (maxsegs);
}

int maxshmsegs (size)
int size;
{
    int nsegs;
    int shmid;
    int maxsegs;
    char *shmaddrs[1024];

    nsegs = 0;
    do {
	shmaddrs[nsegs] = SHMATNULL;
	shmid = shmget (IPC_PRIVATE, size, IPC_CREAT | 0600);
	if (shmid != -1) {
	    shmaddrs[nsegs] = shmat (shmid, 0, 0);
	    shmctl (shmid, IPC_RMID, (struct shmid_ds *) NULL);
	} else {
	}
    } while ((shmaddrs[nsegs] != SHMATNULL) && (++nsegs < 1024));
    maxsegs = nsegs;
    while (--nsegs >= 0) {
	shmdt (shmaddrs[nsegs]);
    }
    return (maxsegs);
}

/*
 *	Estimate the maximum aggregate amount of shared memory
 *	that can be allocated.  Locate the upper and lower bounds
 *	on the segment counts using a binary search on segment
 *	size, then locate the smallest segment size (to nearest 1K)
 *	that gives the upper bound on the segment counts.  Keep
 *	track of each amount of shared memory allocated and report
 *	the maximum found.
 */

long findshmall ()
{
    int lowersize;
    int testsize;
    int uppersize;
    int lowerbound;
    int testcount;
    int upperbound;
    long maxfound = 0;

    for (testsize = shmmax; testsize > 0; testsize /= 2) {
	testcount = findshmmni (testsize);
	if (((long) testcount * (long) testsize) > maxfound) {
	    maxfound = (long) testcount * (long) testsize;
	}
	if (testcount < shmseg) {
	    lowerbound = testcount;
	    lowersize = testsize;
	} else {
	    upperbound = testcount;
	    uppersize = testsize;
	    break;
	}		
    }
    for (testsize = lowersize; testsize > uppersize; testsize -= 1024) {
	testcount = findshmmni (testsize);
	if ((testcount * testsize) > maxfound) {
	    maxfound = testcount * testsize;
	}
	if (testcount >= shmseg) {
	    break;
	}
    }
    return (maxfound);
}

main ()
{
    int testsize;

    shmseg = findshmmni (1024);
    printf ("shmseg = %d\n", shmseg);
    shmmax = maxshmsize ();
    printf ("shmmax = %d (%dK)\n", shmmax, (shmmax / 1024));
    shmall = findshmall ();
    printf ("shmall = %ld (%dK)\n\n", shmall, (int) (shmall / 1024));
    printf ("The number of segments of various sizes\n");
    printf ("that can be actually allocated and\n");
    printf ("simultaneously attached are:\n\n");
    for (testsize = shmmax; testsize > 0; testsize /= 2) {
	if ((testsize < shmmax) && (testsize > 1024)) {
	    while ((testsize % 1024) != 0) {
		testsize++;
	    }
	}
	printf ("\t%3d segments of size %d", maxshmsegs (testsize), testsize);
	if (testsize > 1024) {
	    printf (" (%dK)", testsize / 1024);
	}
	printf ("\n");
    }
    printf ("\n");
}

#else		/* !HAVE_SHM */

main ()
{
    printf ("Sorry, you don't appear to have shared memory.\n");
}

#endif		/* HAVE_SHM */
