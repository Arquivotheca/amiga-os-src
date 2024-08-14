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
 *	utils.c    general utility routines
 *
 *  SCCS
 *
 *	@(#)utils.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	Contains routines which are general in nature and didn't
 *	seem to fit in any other file.
 *
 */

#include "globals.h"

#if (unix || xenix)
#define TEMPLATE "%s/%s"
#else
#define TEMPLATE "%s%s"
#endif

#if CRAMPED
#  define VBUFSIZE (256)
#else
#  define VBUFSIZE (512)
#endif

static char vbuf[VBUFSIZE];	/* Local verbosity buffer */
static char *vbufp = vbuf;	/* Current pointer into verbosity buffer */


/*
 *  FUNCTION
 *
 *	voutput    add to verbosity output
 *
 *  SYNOPSIS
 *
 *	VOID voutput (format, va_alist)
 *	char *format;
 *	va_dcl
 *
 *  DESCRIPTION
 *
 *	Add given output to the verbosity output buffer.
 *
 *  BUGS
 *
 *	The static buffer is a fixed size.  We should really have
 *	some way of checking for overflow.  However, since we
 *	sprintf into it, and we have no way of controlling how
 *	much of it the sprintf will use up, there is not much
 *	we could do anyway except detect overflow and report it
 *	somehow.
 *
 */

/*VARARGS1*/
VOID voutput (VA_ARG(char *,format), VA_ALIST)
VA_OARGS(char *format;)
VA_DCL
{
    va_list args;

    VA_START (args, format);
    (VOID) s_vsprintf (vbufp, format, args);
    vbufp += s_strlen (vbufp);
    va_end (args);
}


/*
 *  FUNCTION
 *
 *	vflush    flush verbosity info
 *
 *  SYNOPSIS
 *
 *	VOID vflush ()
 *
 *  DESCRIPTION
 *
 *	Flushes the current content of the verbosity buffer. to
 */

VOID vflush ()
{
    DBUG_ENTER ("vflush");
    bru_message (MSG_VERBOSITY, vbuf);
    vbufp = vbuf;
    *vbufp = EOS;
    DBUG_VOID_RETURN;
}


/*
 *  NAME
 *
 *	eoablk    test block for end of archive marker block
 *
 *  SYNOPSIS
 *
 *	BOOLEAN eoablk (blkp)
 *	union blk *blkp;
 *
 *  DESCRIPTION
 *
 *	Test block to see if it is the end of archive terminator block.
 *
 *	Note that there is some question about a logical return
 *	value if the pointer is NULL.  It was arbitrarily
 *	decided to return TRUE in this case, as if the end
 *	of the archive was found.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin eoablk
 *	    Set default return to TRUE
 *	    If block pointer is valid then
 *		Test for end of block
 *	    End if
 *	    Return result 
 *	End eoablk
 *
 */

BOOLEAN eoablk (blkp)
union blk *blkp;
{
    BOOLEAN result;		/* Result of test */

    DBUG_ENTER ("eoablk");
    result = TRUE;
    if (blkp == NULL) {
	bru_message (MSG_BUG, "eoablk");
    } else {
	result = (FROMHEX (blkp -> HD.h_magic) == T_MAGIC);
    }
    DBUG_RETURN (result);
}


/*
 *  FUNCTION
 *
 *	allnulls    check to see if a block contains all null bytes
 *
 *  SYNOPSIS
 *
 *	BOOLEAN allnulls (data, nbytes)
 *	char *data;
 *	int nbytes;
 *
 *  DESCRIPTION
 *
 *	Given a pointer to a block of memory of given length in bytes,
 *	check to see if it is all null bytes.  If so, returns TRUE
 *	else returns FALSE.
 *
 *  NOTES
 *
 *	We check as many bytes as possible using a pointer to a long,
 *	and then check the remaining bytes using a character pointer.
 *	This greatly increases the speed on most machines, however
 *	it may fail on some machines if the data is not aligned
 *	properly.  For our situation, the data should (will) always
 *	be aligned properly.
 *
 */

BOOLEAN allnulls (data, nbytes)
char *data;
int nbytes;
{
    long *lp;
    BOOLEAN result;
    char *cp;

    DBUG_ENTER ("allnulls");
    DBUG_PRINT ("fdsize", ("%d bytes to check", nbytes));
    lp = (long *) data;
    result = TRUE;
    while ((nbytes -= sizeof (long)) >= sizeof (long)) {
	if (*lp++ != 0) {
	    result = FALSE;
	    break;
	}
    }
    if (result && (nbytes > 0)) {
	cp = (char *) lp;
	while (nbytes-- > 0) {
	    if (*cp++ != '\000') {
		result = FALSE;
		break;
	    }
	}
    }
    DBUG_PRINT ("allnulls", ("allnulls returns %d", result));
    DBUG_RETURN (result);
}


/*
 *  FUNCTION
 *
 *	copyname    copy a pathname from one place to another
 *
 *  SYNOPSIS
 *
 *	VOID copyname (out, in)
 *	char *out;
 *	char *in;
 *
 *  DESCRIPTION
 *
 *	Copyname copies a pathname from one location to another,
 *	enforcing the current size limit on pathnames set by
 *	the manifest constant "BRUPATHMAX" in the configuration file.
 *
 *	If however, the input name is too long, then the output
 *	will be truncated and null terminated such that the
 *	output still fits in an array of size "BRUPATHMAX". 
 *
 */

VOID copyname (out, in)
char *out;
char *in;
{
    DBUG_ENTER ("copyname");
    if (!copy (out, in, BRUPATHMAX)) {
	bru_message (MSG_BIGPATH, in, BRUPATHMAX - 1);
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	mklink    make a link to a file
 *
 *  SYNOPSIS
 *
 *	BOOLEAN mklink (exists, new)
 *	char *exists;
 *	char *new;
 *
 *  DESCRIPTION
 *
 *	Given pointer to the name of an existing file, and
 *	pointer to the name of a file to link to it, attempts
 *	to make the link, returning TRUE if successful, FALSE
 *	otherwise.
 *
 */

BOOLEAN mklink (exists, new)
char *exists;
char *new;
{
    int rtnval;

    if (s_link (exists, new) == SYS_ERROR) {
	bru_message (MSG_MKLINK, new, exists);
	rtnval = FALSE;
    } else {
	rtnval = TRUE;
    }
    return (rtnval);
}


/*
 *  FUNCTION
 *
 *	mksymlink    make a symbolic link to a file
 *
 *  SYNOPSIS
 *
 *	BOOLEAN mksymlink (exists, new)
 *	char *exists;
 *	char *new;
 *
 *  DESCRIPTION
 *
 *	Given pointer to the name of an existing file, and
 *	pointer to the name of a file to link to it, attempts
 *	to make the symbolic link, returning TRUE if successful, FALSE
 *	otherwise.
 *
 *	This is where we handle Pyramid style conditional symbolic
 *	links, and do hard links on a non-4.2BSD system.  On a non-4.2
 *	system, be sure to do a hard link only if the file to be linked
 *	to exists, and is not a directory.
 *
 */


/*
 *   PSEUDO CODE
 *
 *	Begin mksymlink
 *	    Initialize return value to TRUE
 *	    If on a pyramid then
 *		set linking function to be csymlink
 *	    Else if on a 4.2 system then
 *		get currect pathname
 *		set linking function to be symlink
 *	    Else
 *		get currect pathname
 *		set linking function to be link
 *		If file to be linked to exists then
 *		    If it is directory then
 *			return value = FALSE
 *		    End if
 *		Else
 *		    return value = FALSE
 *		End if
 *	    End if
 *	    If return value still TRUE and linking succeeds then
 *		return value = TRUE
 *	    Else
 *		return value = FALSE
 *	    End if
 *	End mksymlink
 */


BOOLEAN mksymlink (exists, new)
char *exists;
char *new;
{
    int rtnval = TRUE;
    int (*linkfile)PROTO((char *exists, char *new));
#if !HAVE_SYMLINKS && !pyr
    struct bstat bsbuf;
#endif

    DBUG_ENTER ("mksymlink");
    DBUG_PRINT ("mksymlink", ("exists %s  new %s", exists, new));

#if pyr
    DBUG_PRINT ("symlink", ("on a pyramid"));
    linkfile = s_csymlink;
#else
    exists = univlink (exists);
#if HAVE_SYMLINKS
    DBUG_PRINT ("symlink", ("under 4.2"));
    linkfile = s_symlink;
#else
    DBUG_PRINT ("symlink", ("under System V"));
    linkfile = s_link;
    if (file_access (exists, A_EXISTS, FALSE)) {
	if (bstat (exists, &bsbuf, TRUE)) {
	    if (IS_DIR (bsbuf.bst_mode)) {
		bru_message (MSG_SYMTODIR, new, exists, exists);
		rtnval = FALSE;
	    }
	}
    } else {
	bru_message (MSG_HARDLINK, new, exists, exists);
	rtnval = FALSE;
    }
    DBUG_PRINT ("symlink", ("will try hard link of %s to %s", exists, new));
#endif
#endif
    if (rtnval && (*linkfile)(exists, new) != SYS_ERROR) {
	rtnval = TRUE;
    } else {
	rtnval = FALSE;
    }

    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	verbosity    issue a verbosity message
 *
 *  SYNOPSIS
 *
 *	VOID verbosity (fip)
 *	struct finfo *fip;
 *
 *  DESCRIPTION
 *
 *	Given pointer to a file information structure, issues
 *	a verbosity message concerning current mode and file
 *	name, if verbosity enabled.
 *
 */

VOID verbosity (fip)
struct finfo *fip;
{
    LBA blocks = 1;
    LBA total;
    S32BIT kbytes;
    BOOLEAN linked;
    S32BIT zratio = 0;

    DBUG_ENTER ("verbosity");
    if (fip == NULL || fip -> fname == NULL || fip -> bstatp == NULL) {
	bru_message (MSG_BUG, "verbosity");
    } else if (flags.vflag > 0) {
	if (fip -> lname != NULL && *fip -> lname != EOS) {
	    linked = TRUE;
	} else {
	    linked = FALSE;
	}
	voutput ("%c", mode);
	if (!linked && IS_REG (fip -> bstatp -> bst_mode)) {
	    blocks += ZARBLKS (fip);
	    if (IS_COMPRESSED (fip)) {
		if (fip -> bstatp -> bst_size == 0) {
		    zratio = 0;				/* zero length file */
		} else {
		    zratio = fip -> zsize * 100;	/* overflow possible */
		    zratio /= fip -> bstatp -> bst_size;
		    zratio = 100 - zratio;
		}
		if (einfo.zmin > 0) {
		    einfo.zmin = min (einfo.zmin, zratio);
		} else {
		    einfo.zmin = zratio;
		}
		einfo.zmax = max (einfo.zmax, zratio);
		einfo.filebytes += fip -> bstatp -> bst_size;
		einfo.zfilebytes += fip -> zsize;
	    }
	}
	if (flags.vflag > 0 && (fip -> fi_flags & FI_ZFLAG)) {
	    voutput (" (%2ld%%)", zratio);
	}
	kbytes = KBYTES (blocks);
	voutput (" %3ldk", kbytes);
	total = ar_tell () + blocks;
	if (mode == 'c') {
	    total++;
	}
	kbytes = KBYTES (total);
	voutput (" of %3ldk", kbytes);
	voutput (" [%d]", ar_vol () + 1);
	voutput (" %s", fip -> fname);
	/* must test for symlinks first */
	if (flags.vflag > 1 && IS_FLNK (fip -> bstatp -> bst_mode)) {
	    voutput (" symbolic link to %s", namelink (fip -> lname));
	} else if (flags.vflag > 1 && linked) {
	    voutput (" linked to %s", fip -> lname);
	}
	vflush ();
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	out_of_date    check for existing file is out of date
 *
 *  SYNOPSIS
 *
 *	BOOLEAN out_of_date (fip)
 *	struct finfo *fip;
 *
 *  DESCRIPTION
 *
 *	Checks to see if there is an existing file of the same name
 *	with an earlier modification date.  The modification date is
 *	defined to be the most recent of either the mtime or ctime
 *	fields (for UNIX).  This means that any change of the file
 *	contents or the file attributes updates the modification 
 *	date as defined by bru.
 *
 *	If there is no existing file of the same name, then the
 *	result is TRUE.
 *
 *	If there is an existing file but the date cannot be determined
 *	for any reason, then the result is FALSE.
 *
 *	If there is an existing file and it has the same or a more recent
 *	modification date then the result is FALSE.
 *
 */

BOOLEAN out_of_date (fip)
struct finfo *fip;
{
    BOOLEAN result;
    struct bstat bsbuf;

    DBUG_ENTER ("out_of_date");
    result = FALSE;
    if (fip == NULL || fip -> fname == NULL) {
	bru_message (MSG_BUG, "out_of_date");
    } else {
	if (!file_access (fip -> fname, A_EXISTS, FALSE)) {
	    result = TRUE;
	} else {
	    if (bstat (fip -> fname, &bsbuf, FALSE)) {
		if (bsbuf.bst_mtime < fip -> bstatp -> bst_mtime) {
		    result = TRUE;
		}
		if (bsbuf.bst_ctime < fip -> bstatp -> bst_ctime) {
		    result = TRUE;
		}
	    }
	}
    }
    DBUG_RETURN (result);
}


/*
 *  FUNCTION
 *
 *	selected    apply selection criteria
 *
 *  SYNOPSIS
 *
 *	BOOLEAN selected (fip)
 *	struct finfo *fip;
 *
 *  DESCRIPTION
 *
 *	Given pointer to a file information structure,
 *	applies the selection criteria specified on the command line,
 *	such as date of modification, wildcard filename match, etc.
 *
 *	Note that selection is true by default, but if any user specified
 *	test fails, then the selection becomes false.
 *
 */

BOOLEAN selected (fip)
struct finfo *fip;
{
    BOOLEAN select;

    DBUG_ENTER ("selected");
    select = TRUE;
    if (flags.nflag) {
	select &= (fip -> bstatp -> bst_mtime > ntime || fip -> bstatp -> bst_ctime > ntime);
    }
    if (flags.oflag) {
	select &= fip -> bstatp -> bst_uid == uid;
    }
    if ((fip -> fi_flags & FI_AMIGA) && flags.Arflag) {
	select &= !abit_set (fip);
    }
    DBUG_RETURN (select);
}


/*
 *  FUNCTION
 *
 *	getsize    convert an ascii size string to internal long
 *
 *  SYNOPSIS
 *
 *	S32BIT getsize (cp)
 *	char *cp;
 *
 *  DESCRIPTION
 *
 *	Given pointer to a string which is presumably an ascii
 *	numeric string with an optional scale factor, converts
 *	the string to an internal long and returns the value.
 *
 *	Current scale factors are:
 *
 *		'M' or 'm'	2**20  (1 Million)
 *		'K' or 'k'	1024
 *		'B' or 'b'	512    (1 block)
 */

S32BIT getsize (cp)
char *cp;
{
    S32BIT size;
    char *scale;

    DBUG_ENTER ("getsize");
    scale = NULL;
    size = s_strtol (cp, &scale, 0);
    DBUG_PRINT ("getsize", ("size from strtol is %d", size));
    if (scale == NULL) {
	bru_message (MSG_BUG, "getsize");
    } else {
	switch (*scale) {
	    case 'k':
	    case 'K':
	        size *= 1024;
		break;
	    case 'b':
	    case 'B':
	        size *= 512;
		break;
	    case 'm':
	    case 'M':
		size *= (1024 * 1024);
		break;
	}
    }
    DBUG_RETURN (size);
}

    

/*
 *	The library function "strncpy" is almost what we need but not
 *	quite.
 *
 *	Returns TRUE if copy was successful with no truncation, FALSE
 *	otherwise.  Output string is always null terminated.
 *
 */

BOOLEAN copy (out, in, outsize)
char *out;
char *in;
int outsize;
{
    DBUG_ENTER ("copy");
    DBUG_PRINT ("copy", ("copy %s", in));
    while (--outsize > 0) {
	if (*in == EOS) {
	    break;
	} else {
	    *out++ = *in++;
	}
    }
    *out = EOS;
    DBUG_RETURN (*in == EOS);
}


/*
 *  FUNCTION
 *
 *	unconditional    test for unconditional selection
 *
 *  SYNOPSIS
 *
 *	BOOLEAN unconditional (fip)
 *	struct finfo *fip;
 *
 *  DESCRIPTION
 *
 *	Given pointer to a file information structure, uses
 *	the information in the stat buffer for the file, along
 *	with flags set on the command line, to indicate
 *	if the file is to be unconditionally selected regardless
 *	of its modification date.
 *
 *	Returns TRUE if the file is to be selected, FALSE otherwise.
 *
 */

BOOLEAN unconditional (fip)
struct finfo *fip;
{
    BOOLEAN rtnval;
    ushort stmode;

    DBUG_ENTER ("unconditional");
    rtnval = FALSE;
    if (flags.uflag) {
	if (flags.uaflag) {
	    rtnval = TRUE;
	} else {
	    stmode = fip -> bstatp -> bst_mode;
	    if (IS_BSPEC (stmode)) {
		rtnval = flags.ubflag;
	    } else if (IS_CSPEC (stmode)) {
		rtnval = flags.ucflag;
	    } else if (IS_DIR (stmode)) {
		rtnval = flags.udflag;
	    } else if (IS_FLNK (stmode)) {
		rtnval = flags.ulflag;
	    } else if (IS_FIFO (stmode)) {
		rtnval = flags.upflag;
	    } else if (IS_REG (stmode)) {
		rtnval = flags.urflag;
	    }
	}
    }
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	finfo_init    initialize a file information structure
 *
 *  SYNOPSIS
 *
 *	VOID finfo_init (fip, name, bstatp)
 *	struct finfo *fip;
 *	char *name;
 *	struct bstat *bstatp;
 *
 *  DESCRIPTION
 *
 *	Most file information structures are allocated on the stack.
 *	Such structures are not guaranteed to be in any particular state,
 *	thus this routine is available to initialize specific fields
 *	and zero the rest.
 *
 *	The fildes field is set to -1 to indicate an invalid file
 *	descriptor.  Valid file descriptors are positive integers.
 *
 */

VOID finfo_init (fip, name, bstatp)
struct finfo *fip;
char *name;
struct bstat *bstatp;
{
    DBUG_ENTER ("finfo_init");
    (VOID) s_memset ((char *) fip, 0, sizeof (struct finfo));
    fip -> bstatp = bstatp;
    fip -> fname = name;
    fip -> fildes = -1;
    fip -> fi_flags = FI_FLAGS_INIT;
    if (flags.Zflag != Z_OFF) {
	fip -> fi_flags |= FI_ZFLAG;
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	swapbytes    swap bytes in an archive block
 *
 *  SYNOPSIS
 *
 *	VOID swapbytes (blkp)
 *	union blk *blkp;
 *
 *  DESCRIPTION
 *
 *	Swaps each pair of bytes in an archive block.
 */

VOID swapbytes (blkp)
union blk *blkp;
{
    char *cs1;
    char *cs2;
    char tmp;

    DBUG_ENTER ("swapbytes");
    cs1 = &blkp -> bytes[0];
    cs2 = &blkp -> bytes[1];
    while (cs1 < OVERRUN (blkp -> bytes)) {
	tmp = *cs1;
	*cs1++ = *cs2++;
	*cs1++ = tmp;
	cs2++;
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	swapshorts    swap shorts in an archive block
 *
 *  SYNOPSIS
 *
 *	VOID swapshorts (blkp)
 *	union blk *blkp;
 *
 *  DESCRIPTION
 *
 *	Swaps each pair of shorts in an archive block.
 */

VOID swapshorts (blkp)
union blk *blkp;
{
    short *ss1;
    short *ss2;
    short stmp;

    DBUG_ENTER ("swapshorts");
    ss1 = &blkp -> shorts[0];
    ss2 = &blkp -> shorts[1];
    while (ss1 < OVERRUN (blkp -> shorts)) {
	stmp = *ss1;
	*ss1++ = *ss2++;
	*ss1++ = stmp;
	ss2++;
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	get_memory    ask system for more memory
 *
 *  SYNOPSIS
 *
 *	VOID *get_memory (size, quit)
 *	UINT size;
 *	BOOLEAN quit;
 *
 *  DESCRIPTION
 *
 *	Given number of bytes to allocate from system (size),
 *	issues the appropriate system call to get the memory.
 *
 *	Returns pointer to the allocated memory if successful,
 *	NULL otherwise.  Note that any memory allocated is guaranteed
 *	to be properly aligned for any use (see malloc(3C) in UPM).
 *	Get_memory is declared to return type "pointer to void" so
 *	that casting to any other type of object will not give
 *	lint messages about "possible pointer alignment problem".
 *	So instead of numerous bogus warnings, we will get exactly
 *	one lint warning here, for the conversion from (char *) to
 *	(void *).
 *
 *	If the quit flag is set, and no memory can be allocated,
 *	the program will call the routine to clean up and exit.
 *
 *	The memory allocated may be freed with the "free" system
 *	call.
 *
 *	Note that the memory is NOT guaranteed to be zeroed.
 *
 */
 
VOID *get_memory (size, quit)
UINT size;
BOOLEAN quit;
{
    char *new;
    
    new = s_malloc (size);
    if (new == NULL && quit) {
	bru_message (MSG_MALLOC, size);
	done ();
    }
    return ((VOID *) new);
}


/*
 *	In cases of extreme bugs, we can define DUMPBLK, which enables
 *	dumping of the contents of each archive block using the DBUG
 *	macro mechanism.  We don't normally want this, as it results
 *	in overwhelming output...
 */

#ifdef DUMPBLK

do_dump (blkp)
union blk *blkp;
{
    char *cp;
    int count;

    DBUG_ENTER ("do_dump");
    cp = blkp -> bytes;
    count = 0;
    while (cp < &blkp -> bytes[BLKSIZE]) {
	if (*cp < '\040') {
	    (VOID) s_fprintf (DBUG_FILE, "   ^%c", *cp | 0100);
	} else if (*cp < '\177') {
	    (VOID) s_fprintf (DBUG_FILE, "    %c", *cp);
	} else {
	    (VOID) s_fprintf (DBUG_FILE, " \\%3.3o", *cp);
	}
	cp++;
	count++;
	if ((count % 16) == 0) {
	    (VOID) s_fprintf (DBUG_FILE, "\n");
	}
    }
    s_fflush (DBUG_FILE);
    DBUG_VOID_RETURN;
}

VOID dump_blk (blkp, lba)
union blk *blkp;
LBA lba;
{
    DBUG_ENTER ("dump_blk");
    DBUG_EXECUTE ("dump", {(VOID) s_fprintf (DEBUGFILE, "\nDump of block %ld\n\n", lba);});
    DBUG_EXECUTE ("dump", {(VOID) do_dump (blkp);});
    DBUG_VOID_RETURN;
}

#endif	/* DUMPBLK */


/*
 *  FUNCTION
 *
 *	file_chown    change ownership of a file
 *
 *  SYNOPSIS
 *
 *	VOID file_chown (path, owner, group)
 *	char *path;
 *	int owner, group;
 *
 *  DESCRIPTION
 *
 *	Changes owner and group of a file pointed to by path.
 *	Returns no value but causes error message to be printed
 *	if any problems occur.
 *
 *	Under 4.2 BSD the chown system call only works if called
 *	with effective uid of root.  This is supposedly for accounting.
 *
 *	The bru default is to always restore as much as possible,
 *	including the file ownership to the archived uid.  This has
 *	obvious problems for normal users reading tapes from other sites
 *	where the numeric uid's most likely are completely different.
 *	For this case, we provide a special flag (-C) that chown's all
 *	the files to the uid of the user running bru.  The reasoning
 *	behind which case to make the default and which case to make
 *	a special flag is given in the "ownership" file in the notes
 *	directory.  Basically, the default is easier to correct, and
 *	potentially less damaging to the system, than the results of
 *	applying the -C flag.
 *
 */

VOID file_chown (path, owner, group)
char *path;
int owner, group;
{
    DBUG_ENTER ("file_chown");

    if (flags.Cflag) {
	owner = info.bru_uid;
	group = info.bru_gid;
    }

    DBUG_PRINT ("chown", ("file %s, owner %d, group %d", path, owner, group));

    if (s_chown (path, owner, group) == SYS_ERROR) {
	bru_message (MSG_CHOWN, path);
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	reload    reload first volume of multivolume archive
 *
 *  SYNOPSIS
 *
 *	VOID reload (reason)
 *	char *reason;
 *
 *  DESCRIPTION
 *
 *	Issues prompt for user to reload first volume of a multivolume
 *	archive for the reason (pass) specified.  For example, a differences
 *	pass of a newly created archive.
 *
 */
 
VOID reload (reason)
char *reason;
{
    DBUG_ENTER ("reload");
    if (ejectable () || (last_vol () > 0)) {
	bru_message (MSG_NEWPASS, reason);
	(VOID) new_arfile (0);
    }
    DBUG_VOID_RETURN;
}


VOID fork_shell ()
{
    char *dir;
    char *file;
    char *shell;
#if !amigados
    char *vector[2];
#endif
    
    DBUG_ENTER ("fork_shell");
    shell = s_getenv ("SHELL");
    if (shell == NULL) {
#if amigados
	dir = NULL;		/* Turn these into defines */
	file = "c:newcli";
#else
	dir = "/bin";
	file = "sh";
#endif
    } else {
	dir = "";
	file = shell;
    }
#if amigados
    (VOID) Execute (file, NULL, NULL);
#else
    vector[0] = file;
    vector[1] = NULL;
    (VOID) execute (dir, file, vector);
#endif
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	newdir    make a directory node
 *
 *  SYNOPSIS
 *
 *	BOOLEAN newdir (fip)
 *	struct finfo *fip;
 *
 *  DESCRIPTION
 *
 *	Makes a directory with given stat and name.  If an error occurs,
 *	print error message.
 *
 */


BOOLEAN newdir (fip)
struct finfo *fip;
{
    BOOLEAN filemade;

    DBUG_ENTER ("newdir");
    if (s_mkdir (fip -> fname, (int) fip -> bstatp -> bst_mode) == SYS_ERROR) {
	bru_message (MSG_MKDIR, fip -> fname);
    } else {
	filemade = TRUE;
    }
    DBUG_RETURN (filemade);
}


/*
 *  FUNCTION
 *
 *	magic_ok    test a block to see if the magic value is recognized
 *
 *  SYNOPSIS
 *
 *	BOOLEAN magic_ok (blkp)
 *	union blk *blkp;
 *
 *  DESCRIPTION
 *
 *	Test the block pointed to and return TRUE if the block has a
 *	recognized magic number, FALSE otherwise.
 *
 */

BOOLEAN magic_ok (blkp)
union blk *blkp;
{
    BOOLEAN rtnval;
    int mag;
	
    DBUG_ENTER ("magic_ok");
    rtnval = FALSE;
    if (blkp == NULL) {
	bru_message (MSG_BUG, "magic_ok");
    } else {
	mag = (int) FROMHEX (blkp -> HD.h_magic);
	if (mag==A_MAGIC || mag==H_MAGIC || mag==D_MAGIC || mag==T_MAGIC) {
	    rtnval = TRUE;
	}
    }
    DBUG_RETURN (rtnval);
 }
 

/*
 *	Note that because the name buffer is static and reused on each call,
 *	we can only have one finfo structure, with an attached compressed
 *	file, active at any given time.
 *
 *	The size of tname is probably overkill, by a large margin.
 */

BOOLEAN openzfile (fip)
struct finfo *fip;
{
    int status = FALSE;
    static char tname[BRUPATHMAX + 256];
    static char *tfilep;

    DBUG_ENTER ("openzfile");
    if (tfilep == NULL) {
	tfilep = mktemp ("brutmpXXXXXX");
    }
    (VOID) s_sprintf (tname, TEMPLATE, info.bru_tmpdir, tfilep);
    DBUG_PRINT ("brutmpdir", ("use temp file '%s'", tname));
    fip -> zfname = tname;
    if ((fip -> zfildes = s_creat (fip -> zfname, 0600)) == SYS_ERROR) {
	bru_message (MSG_OPEN, fip -> zfname);
    } else {
	status = TRUE;
    }
    DBUG_PRINT ("openz", ("openzfile returns %d", status));
    DBUG_RETURN (status);
}


/*
 *  FUNCTION
 *
 *	compressfip    compress a file
 *
 *  DESCRIPTION
 *
 *	Given file info pointer for an open uncompressed file, attempt
 *	to create a compressed version of the file in the directory
 *	set by the BRUTMPDIR environment variable.  Returns TRUE if
 *	successful, FALSE if the compression fails for some reason.
 *
 *  NOTES
 *
 *	We play it safe before proceeding and unconditionally mark 
 *	the fip as not having a compressed version of the file.
 *
 *	Since we aren't bothering to check the return values of
 *	s_unlink and s_close, just do them at the end if we fail
 *	for some reason, and don't worry about whether or not
 *	the file descriptor is open or the file exists.  This might
 *	be a potential problem on nonunix systems (closing nonopen
 *	descriptors or unlinking nonexistant files).
 *
 */

BOOLEAN compressfip (fip)
struct finfo *fip;
{
    int status = FALSE;
    struct bstat bsbuf;

    DBUG_ENTER ("compressfip");
    fip -> fi_flags &= ~FI_LZW;
    fip -> zsize = 0;
    if (openzfile (fip)) {
	if (compress (fip)) {
	    (VOID) s_lseek (fip -> fildes, 0L, 0);
	    (VOID) s_close (fip -> zfildes);
	    fip -> zfildes = s_open (fip -> zfname, O_RDONLY, 0);
	    if (fip -> zfildes == SYS_ERROR) {
		bru_message (MSG_OPEN, fip -> zfname);
	    } else {
		if (bstat (fip -> zfname, &bsbuf, TRUE)) {
		    status = TRUE;
		    fip -> fi_flags |= FI_LZW;
		    fip -> zsize = bsbuf.bst_size;
		    DBUG_PRINT ("zsize", ("zsize %d", fip -> zsize));
		}
	    }
	}
	if (!status) {
	    (VOID) s_close (fip -> zfildes);
	    (VOID) s_unlink (fip -> zfname);
	}
    }
    DBUG_RETURN (status);
}


BOOLEAN decompfip (fip)
struct finfo *fip;
{
    int status = FALSE;
    long fullsize;

    DBUG_ENTER ("decompfip");
    fip -> zfildes = s_open (fip -> zfname, O_RDONLY, 0);
    if (fip -> zfildes == SYS_ERROR) {
	bru_message (MSG_OPEN, fip -> zfname);
	bru_message (MSG_ZXFAIL, fip -> fname);
    } else {
	if (IS_CTG (fip -> bstatp -> bst_mode)) {
	    fullsize = fip -> bstatp -> bst_size;
	    fip -> fildes = s_ccreat (fip -> fname, 0600, fullsize);
	} else {
	    fip -> fildes = s_creat (fip -> fname, 0600);
	}
	if (fip -> fildes == SYS_ERROR) {
	    bru_message (MSG_OPEN, fip -> fname);
	    bru_message (MSG_ZXFAIL, fip -> fname);
	} else {
	    if (!decompress (fip)) {
		bru_message (MSG_ZXFAIL, fip -> fname);
	    } else {
		status = TRUE;
	    }
	    (VOID) s_close (fip -> fildes);
	}
	(VOID) s_close (fip -> zfildes);
    }
    (VOID) s_unlink (fip -> zfname);
    DBUG_RETURN (status);
}

/*
 *	Add the ".Z" extension to a name.  Note that we only call addz
 *	on buffers which have previously been filled via copyname, which
 *	ensures that there is sufficient space for the ".Z" extension
 *	if the -Z command line option is given.  Thus we don't need to
 *	check for a buffer overflow.
 */

VOID addz (fname)
char *fname;
{
    char *endp;
    
    DBUG_ENTER ("addz");
    DBUG_PRINT ("addz", ("add a .Z extension to '%s'", fname));
    endp = fname + s_strlen (fname);
    *endp++ = '.';
    *endp++ = 'Z';
    *endp = EOS;
    DBUG_PRINT ("addz", ("final name '%s'", fname));
    DBUG_VOID_RETURN;
}

/*
 *	Strip the ".Z" extension from a name.
 */

VOID stripz (fname)
char *fname;
{
    char *endp;
    
    DBUG_ENTER ("stripz");
    DBUG_PRINT ("stripz", ("strip a .Z extension from '%s'", fname));
    endp = fname + s_strlen (fname);
    if ((*--endp == 'Z') && (*--endp == '.')) {
	*endp = EOS;
    }
    DBUG_PRINT ("stripz", ("final name '%s'", fname));
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	discard_zfile    discard any compressed version of file
 *
 *  SYNOPSIS
 *
 *	static VOID discard_zfile (fip)
 *	struct finfo *fip;
 *
 *  DESCRIPTION
 *
 *	Given a pointer to a file information structure, if there
 *	is a compressed version of the file associated with it,
 *	discard the compressed version and fix up the file info
 *	structure appropriately.
 *
 */

VOID discard_zfile (fip)
struct finfo *fip;
{
    DBUG_ENTER ("discard_zfile");
    if (IS_COMPRESSED (fip)) {
	fip -> fi_flags &= ~FI_LZW;
	fip -> zsize = 0;
	if (s_close (fip -> zfildes) == SYS_ERROR) {
	    bru_message (MSG_CLOSE, fip -> zfname);
	}
	if (s_unlink (fip -> zfname) == SYS_ERROR) {
	    bru_message (MSG_UNLINK, fip -> zfname);
	}
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	pcreat    protected version of creat system call
 *
 *  SYNOPSIS
 *
 *	int pcreat (path, fmode)
 *	char *path;
 *	int fmode;
 *
 *  DESCRIPTION
 *
 *	This is a protected version of the creat system call, that
 *	works the same as creat would if bru was not running suid.
 *	We have to be careful not to allow the user to create files
 *	or write to files which would ordinarily be denied due to
 *	permissions.
 *
 *	If successful, returns a pointer to the newly opened or
 *	created file.  If the file did not previously exist, it
 *	is chown'd and chgrp'd to the real user id and group.
 *
 */

int pcreat (path, fmode)
char *path;
int fmode;
{
    int fildes;
    BOOLEAN accessible;
    BOOLEAN exists;

    DBUG_ENTER ("do_open");
    fildes = -1;
    accessible = FALSE;
    exists = file_access (path, A_EXISTS, FALSE);
    if (exists) {
	if (file_access (path, A_WRITE, FALSE)) {
	    accessible = TRUE;
	}
    } else {
	if (dir_access (path, A_WRITE, FALSE)) {
	    accessible = TRUE;
	}
    }
    if (accessible) {
	fildes = s_open (path, O_WRONLY | O_CREAT | O_TRUNC, fmode);
	if (fildes >= 0) {
	    if (!exists) {
		file_chown (path, s_getuid (), s_getgid ());
	    }
	}
	DBUG_PRINT ("pcreat", ("%s open on %o", path, fildes));
    }
    DBUG_RETURN (fildes);
}


/*
 *  FUNCTION
 *
 *	namesane    check device name for sanity
 *
 *  SYNOPSIS
 *
 *	BOOLEAN namesane (devname)
 *	char *devname;
 *
 *  DESCRIPTION
 *
 *	Check the name given for the intended archive device to see if
 *	it passes some simple sanity checks.  If so, then let it pass
 *	unmolested.  Otherwise, confirm it's use with the operator.
 *	This IS NOT a security checkpoint, it is to prevent accidentally
 *	using unintended names, such as names with control characters
 *	in them.
 *
 *	The size of the name buffers should be more than adequate for
 *	any names except the most pathological ones (BRUPATHMAX worth
 *	of ^A bytes for example).  To be really safe, we could use
 *	4 * BRUPATHMAX for each, then they could never overflow.
 */

BOOLEAN namesane (devname)
char *devname;
{
    BOOLEAN result = TRUE;
    char newname[BRUPATHMAX + 128];
    char prtbuf[BRUPATHMAX + 128];
    char *scan;
    char *new;
    char key;

    DBUG_ENTER ("namesane");
    scan = devname;
    new = newname;
    while (*scan != EOS) {
	if (isprint (*scan) && !isspace (*scan)) {
	    *new++ = *scan++;
	} else {
	    (VOID) s_sprintf (new, "\\%3.3o", *scan++);
	    while (*new != EOS) {new++;}
	}
    }
    *new = EOS;
    if (!STRSAME (devname, newname)) {
	(VOID) s_sprintf (prtbuf,
			  "\"%s\" is an unusual name, use it anyway? [y/n]",
			  newname);
	key = response (prtbuf, 'n');
	if (key == 'y' || key == 'Y') {
	    result = TRUE;
	} else {
	    result = FALSE;
	}
    }
    DBUG_RETURN (result);
}

/*
 *	If devname specifies a block or character special device file,
 *	confirm that this name is really ok to use.  Returns TRUE if
 *	so, false otherwise.  Non-special file names are automatically
 *	confirmed.
 *
 *	The size of the name buffers should be more than adequate for
 *	any names except the most pathological ones (BRUPATHMAX worth
 *	of ^A bytes for example).  To be really safe, we could use
 *	4 * BRUPATHMAX for each, then they could never overflow.
 */

BOOLEAN nameconfirm (dname)
char *dname;
{
    BOOLEAN result = TRUE;
    char newname[BRUPATHMAX + 128];
    char prtbuf[BRUPATHMAX + 128];
    char *scan;
    char *new;
    char key;
    struct bstat bsbuf;

    DBUG_ENTER ("nameconfirm");
    scan = dname;
    new = newname;
    while (*scan != EOS) {
	if (isprint (*scan) && !isspace (*scan)) {
	    *new++ = *scan++;
	} else {
	    (VOID) s_sprintf (new, "\\%3.3o", *scan++);
	    while (*new != EOS) {new++;}
	}
    }
    *new = EOS;
    if (bstat (dname, &bsbuf, FALSE) && (IS_SPEC (bsbuf.bst_mode))) {
	(VOID) s_sprintf (prtbuf, "please confirm use of device \"%s\" [y/n]",
			  newname);
	key = response (prtbuf, 'n');
	if (key == 'y' || key == 'Y') {
	    result = TRUE;
	} else {
	    result = FALSE;
	}
    }
    DBUG_RETURN (result);
}
