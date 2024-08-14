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
 *	filestat.c    routines to manipulate file "stat" information
 *
 *  SCCS
 *
 *	@(#)filestat.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	Contains routines to map between the internal and external
 *	representations of file "stat" information, such as protection
 *	modes and file type bits.  The internal representation is
 *	a superset of the classical UNIX definition of the st_mode
 *	field of the file's stat structure, and is also the format
 *	of the bits stored in the archive.  This mapping is not
 *	guaranteed to remain identical if UNIX changes in the future
 *	and particularly, when a port is done to non-UNIX machines,
 *	like the Amiga.
 *
 */

#include "globals.h"

static int s_stat PROTO((char *path, struct stat *buf));
static unsigned long modetrans PROTO((unsigned int lmode));

/*
 *	For compatibility between 4.2BSD and System V machines,
 *	we define certain file types that one has but the other
 *	doesn't.  This is so that a 4.2 system can handle a tape
 *	written under System V, and/or vice versa.  Luckily, there
 *	are only two cases, and the numbers don't conflict.  This
 *	does sort of defeat the whole purpose of standard header files
 *	to define magic constants, but what are we supposed to do?
 *
 *	This also works on the Pyramid, which defines both, and also
 *	happens to use the right magic numbers.
 *
 *	In addition, Masscomp has a special type of regular file,
 *	for which disk blocks are allocated contiguously.  This
 *	is yet another magic constant.
 *
 *	For now, these are not used here anyway, since there are
 *	equivalent constants defined in the finfo file for bru's
 *	private stat mechanism.
 *
 */

#ifndef S_IFIFO		/* System V FIFO, i.e. a pipe, named or otherwise */
#define S_IFIFO		0010000
#endif

#ifndef S_IFCTG		/* Masscomp style contiguous file */
#define S_IFCTG		0110000
#endif

#ifndef S_IFLNK		/* 4.2BSD Symbolic link */
#define S_IFLNK		0120000
#endif

#ifndef S_IFSOCK	/* SVR4 sockets */
#define S_IFSOCK	0140000
#endif


/*
 *  FUNCTION
 *
 *	fipstat    get stat info for file info struct
 *
 *  SYNOPSIS
 *
 *	BOOLEAN fipstat (fip, report)
 *	struct finfo *fip;
 *	int report;
 *
 *  DESCRIPTION
 *
 *	Given a pointer to a file info structure, attempts to stat the
 *	associated file to initialize the internal bru stat info and
 *	any machine dependent fields in the file information structure.
 *
 *	Returns TRUE if successful, FALSE otherwise.
 *
 */

BOOLEAN fipstat (fip, report)
struct finfo *fip;
int report;
{
    int rtnval;
#if HAVE_SYMLINKS
    int nmlen;
    static char linkbuf[BRUPATHMAX];
#endif

    DBUG_ENTER ("fipstat");
    DBUG_PRINT ("stat", ("stat %s", fip -> fname));
    if (bstat (fip -> fname, fip -> bstatp, report) == NULL) {
	rtnval = FALSE;
    } else {
	rtnval = TRUE;
#if HAVE_SYMLINKS
	if (IS_FLNK (fip -> bstatp -> bst_mode)) {
	    nmlen = s_readlink (fip -> fname, linkbuf, sizeof (linkbuf));
	    if (nmlen == SYS_ERROR) {
		bru_message (MSG_RDLINK, fip -> fname);
		rtnval = FALSE;
	    } else {
		linkbuf[nmlen] = EOS;
		fip -> lname = linkbuf;
		DBUG_PRINT ("symlink", ("symlink is %s", fip -> lname));
	    }
	}
#endif
	(void) getinfo (fip);
    }
    DBUG_RETURN (rtnval);
}

/*
 *	Stat a file given it's name, optionally reporting an error if
 *	the stat fails for any reason.  If a pointer to a stat structure
 *	is provided, then fill that one in.  Otherwise, fill in a local
 *	static one and return a pointer to it, or return NULL on error.
 */

struct bstat *bstat (fname, bstatp, report)
char *fname;
struct bstat *bstatp;
int report;
{
    struct stat sbuf;
    struct bstat bsbuf;

    DBUG_ENTER ("bstat");
    DBUG_PRINT ("stat", ("stat %s", fname));
#if RMT
    if (_rmt_dev (fname)) {
	errno = EOPNOTSUPP;
	if (report) {
	    bru_message (MSG_STATFAIL, fname);
	}
	DBUG_RETURN (NULL);
    }
#endif
    if (bstatp == NULL) {
	bstatp = &bsbuf;
    }
    if (s_stat (fname, &sbuf) == SYS_ERROR) {
	if (report) {
	    bru_message (MSG_STATFAIL, fname);
	}
	bstatp = NULL;
    } else {
#if LATTICE_STAT
	bstatp -> bst_mode = modetrans (sbuf.st_mode);
#else
	bstatp -> bst_mode = sbuf.st_mode;
#endif
	bstatp -> bst_ino = (unsigned short) sbuf.st_ino;
	bstatp -> bst_dev = (short) sbuf.st_dev;
	bstatp -> bst_rdev = (short) sbuf.st_rdev;
	bstatp -> bst_nlink = (short) sbuf.st_nlink;
	bstatp -> bst_uid = (unsigned short) sbuf.st_uid;
	bstatp -> bst_gid = (unsigned short) sbuf.st_gid;
	bstatp -> bst_size = (unsigned long) sbuf.st_size;
#if LATTICE_STAT
	bstatp -> bst_mtime = (unsigned long) amiga2unix (sbuf.st_mtime);
#else
	bstatp -> bst_mtime = (unsigned long) sbuf.st_mtime;
#endif
	bstatp -> bst_atime = bstatp -> bst_mtime;
	bstatp -> bst_ctime = bstatp -> bst_mtime;
    }
    DBUG_RETURN (bstatp);
}


/*
 *  FUNCTION
 *
 *	s_stat    get file status
 *
 *  SYNOPSIS
 *
 *	int s_stat (path, buf)
 *	char *path;
 *	struct stat *buf;
 *
 *  DESCRIPTION
 *
 *	Invoke low level routine to get file status.
 *
 *	If we have symbolic links we use the lstat routine instead,
 *	which will tell us whether or not we've found a symbolic link.
 *	An lstat on a normal file acts just like regular stat.
 *
 */

#if LATTICE_STAT	/* Lattice stat appears to give screwy mode bits */

/*VARARGS1*/	/* Funny stuff with 2nd arg due to rmt.h define of stat */
static int s_stat (path, buf)
char *path;
struct stat *buf;
{
    int rtnval;

    DBUG_ENTER ("s_stat");
#if HAVE_SYMLINKS
    rtnval = lstat (path, buf);
#else
    rtnval = stat (path, buf);
#endif
    DBUG_RETURN (rtnval);
}

/*
 *	Translate the Lattice mode bits to the bru internal form.
 *	Files that are writable or deletable take on internal "write".
 *	Files that are not directories become "regular files".
 */

static unsigned long modetrans (unsigned int lmode)
{
    unsigned long newmode = 0;

    DBUG_ENTER ("modetrans");
    DBUG_PRINT ("mode", ("lattice mode is %#x", lmode));
    if (lmode & S_IREAD) {
	newmode |= (BS_IREAD | BS_IREAD >> 3 | BS_IREAD >> 6);
    }
    if ((lmode & S_IWRITE) || (lmode & S_IDELETE)) {
	newmode |= (BS_IWRITE | BS_IWRITE >> 3 | BS_IWRITE >> 6);
    }
    if (lmode & S_IEXECUTE) {
	newmode |= (BS_IEXEC | BS_IEXEC >> 3 | BS_IEXEC >> 6);
    }
    if (lmode & S_IFDIR) {
	newmode |= BS_IFDIR;
    } else if ((lmode & S_IFREG) || (lmode & S_ISCRIPT)) {
	newmode |= BS_IFREG;
    } else {
	DBUG_PRINT ("mode", ("WARNING!! - unknown mode becomes reg file"));
	newmode |= BS_IFREG;
    }
    DBUG_PRINT ("mode", ("translated mode is %#x", newmode));
    DBUG_RETURN (newmode);
}

#else

static int s_stat (path, buf)
char *path;
struct stat *buf;
{
    long lck;
    struct FileInfoBlock *fp;
    long prot;
    long ftime;

    DBUG_ENTER ("s_stat");
    DBUG_PRINT ("stat", ("stat file '%s'", path));
    if ((lck = Lock (path, ACCESS_READ)) == 0) {
	DBUG_RETURN (SYS_ERROR);
    }
    fp = (struct FileInfoBlock *)
	AllocMem ((long) sizeof (struct FileInfoBlock), 
	(long) (MEMF_CLEAR | MEMF_CHIP));
    Examine (lck, fp);
    DBUG_PRINT ("fib", ("fib_DirEntryType is %d", fp -> fib_DirEntryType));
    if (fp -> fib_DirEntryType > 0) {
	buf -> st_mode = BS_IFDIR;
    } else {
	buf -> st_mode = BS_IFREG;
    }
    DBUG_PRINT ("prot", ("raw protection bits = %#lx", fp -> fib_Protection));
    prot = ~(fp -> fib_Protection) & 0xF;
    if (prot & 0x1) {
	DBUG_PRINT ("prot", ("delete bit set, set write bit also"));
	prot |= 0x4;
    }
    prot >>= 1;
    DBUG_PRINT ("prot", ("translated bits = %#lx", prot));
    buf -> st_mode |= (prot << 6 | prot << 3 | prot);
    DBUG_PRINT ("prot", ("final mode = %#lx", buf -> st_mode));
    buf -> st_nlink = 1;
    DBUG_PRINT ("links", ("file has %d links", buf -> st_nlink));
    buf -> st_size = fp -> fib_Size;
    DBUG_PRINT ("size", ("file size = %ld", buf -> st_size));
    ftime = dos2unix (&(fp -> fib_Date));
    buf -> st_atime = ftime;
    buf -> st_mtime = ftime;
    buf -> st_ctime = ftime;
    buf -> st_ino = 0;
    buf -> st_dev = 0;
    buf -> st_rdev = 0;
    buf -> st_uid = 0;
    buf -> st_gid = 0;
    (VOID) FreeMem (fp, (long) sizeof (struct FileInfoBlock));
    (VOID) UnLock (lck);
    DBUG_RETURN (0);
}

#endif
