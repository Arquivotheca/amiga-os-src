/************************************************************************
 *									*
 *	Copyright (c) 1988 Enhanced Software Technologies, Inc.		*
 *			     All Rights Reserved			*
 *		(Major additions by Arnold Robbins at Georgia Tech)	*
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
 *	sys.c    runtime support library calls
 *
 *  SCCS
 *
 *	@(#)sys.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	Calls to the runtime support library are localized here
 *	to aid portability and modification and/or inclusion
 *	of custom versions.
 *
 *	This form of interface adds an extra level of function
 *	call overhead, but the benefits for modification or
 *	debugging purposes outweigh the negligible decrease
 *	in efficiency.  (Use the profiler to discover just
 *	how little time is actually spent in these routines!)
 *
 *	There is some question about the best way to deal with
 *	situations in which functions like "printf" return
 *	error results.  This generally indicates a severe
 *	problem with the environment or corrupted runtime
 *	data space.  In any case, it is probably undesirable
 *	to continue execution under those circumstances, so
 *	we abort with an IOT signal and core dump.
 *
 *	An additional advantage of having these functions
 *	centralized is that lint will only have one version
 *	of the library call to complain about.  This
 *	considerably reduces the typical size and complexity
 *	of the lint output log.
 *
 */

#include "globals.h"


/*
 *  FUNCTION
 *
 *	s_vfprintf    call the vfprintf function
 *
 *  SYNOPSIS
 *
 *	int s_vfprintf (stream, format, ap)
 *	FILE *stream;
 *	char *format;
 *	va_list ap;
 *
 *  DESCRIPTION
 *
 *	Invokes the library "vfprintf" function.  If varargs support
 *	is supplied by the system, then we just pass our args on to
 *	vfprintf.  If not, then we need to rebuild an appropriate
 *	argument list for the fprintf call.  This should work on
 *	most reasonable machines that don't have varargs (because if
 *	it didn't work they would HAVE varargs support).
 *
 *  NOTE
 *
 *	DO NOT use the DBUG macros here, since the DBUG package
 *	also calls this function to obtain varargs support.
 *
 */

int s_vfprintf (stream, format, ap)
FILE *stream;
char *format;
va_list ap;
{
    int rtnval;
#if HAVE_VARARGS
    rtnval = vfprintf (stream, format, ap);
#else
    ARGS_DCL;

    ARG0 =  va_arg (ap, ARGS_TYPE);
    ARG1 =  va_arg (ap, ARGS_TYPE);
    ARG2 =  va_arg (ap, ARGS_TYPE);
    ARG3 =  va_arg (ap, ARGS_TYPE);
    ARG4 =  va_arg (ap, ARGS_TYPE);
    ARG5 =  va_arg (ap, ARGS_TYPE);
    ARG6 =  va_arg (ap, ARGS_TYPE);
    ARG7 =  va_arg (ap, ARGS_TYPE);
    ARG8 =  va_arg (ap, ARGS_TYPE);
    ARG9 =  va_arg (ap, ARGS_TYPE);
    rtnval = fprintf (stream, format, ARGS_LIST);
#endif
    return (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_fprintf    call the fprintf function
 *
 *  SYNOPSIS
 *
 *	int s_fprintf (stream, format, va_alist)
 *	FILE *stream;
 *	char *format;
 *	va_dcl
 *
 *  DESCRIPTION
 *
 *	Invokes the library "fprintf" function.  We call the
 *	internal version of vprintf and let it deal with the
 *	variable arguments problem.
 */

/*VARARGS2*/
int s_fprintf (VA_ARG(FILE *,stream), VA_ARG(char *,format), VA_ALIST)
VA_OARGS(FILE *stream;)
VA_OARGS(char *format;)
VA_DCL
{
    int rtnval;
    va_list args;

    VA_START (args, format);
    rtnval = s_vfprintf (stream, format, args);
    va_end (args);
    return (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_vsprintf    call the vsprintf function
 *
 *  SYNOPSIS
 *
 *	int s_vsprintf (s, format, ap)
 *	char *s;
 *	char *format;
 *	va_list ap;
 *
 *  DESCRIPTION
 *
 *	Invokes the library "vsprintf" function.  We call the internal
 *	form of vsprintf and let it deal with the variable arguments
 *	problem.
 *
 *	Big problem here.  System V defines sprintf to return an
 *	int, and -1 for error.  4.2BSD follows V7 and has it returning
 *	a char *, the string it is printing into, and therefore
 *	there really is no way to tell if it returned an error.
 *	If we're lucky, on a bizarre conversion, the string will
 *	have an EOS as the first character, so we test against that.
 */

/*VARARGS2*/
int s_vsprintf (s, format, ap)
char *s;
char *format;
va_list ap;
{
    int rtnval;

#if HAVE_VARARGS
    rtnval = vsprintf (s, format, ap);
#else
    ARGS_DCL;
    ARG0 =  va_arg (ap, ARGS_TYPE);
    ARG1 =  va_arg (ap, ARGS_TYPE);
    ARG2 =  va_arg (ap, ARGS_TYPE);
    ARG3 =  va_arg (ap, ARGS_TYPE);
    ARG4 =  va_arg (ap, ARGS_TYPE);
    ARG5 =  va_arg (ap, ARGS_TYPE);
    ARG6 =  va_arg (ap, ARGS_TYPE);
    ARG7 =  va_arg (ap, ARGS_TYPE);
    ARG8 =  va_arg (ap, ARGS_TYPE);
    ARG9 =  va_arg (ap, ARGS_TYPE);
#if BSD4_2
    /* kludge */
    if (sprintf (s, format, ARGS_LIST) != s || *s == EOS) {
	rtnval = 0;
    }
#else
    /* do it right */
    rtnval = sprintf (s, format, ARGS_LIST);
#endif
#endif
    return (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_sprintf    call the sprintf function
 *
 *  SYNOPSIS
 *
 *	int s_sprintf (s, format, va_alist)
 *	char *s;
 *	char *format;
 *	va_dcl
 *
 *  DESCRIPTION
 *
 *	Invokes the library "sprintf" function.  We call the internal
 *	form of vsprintf and let it deal with the variable arguments
 *	problem.
 *
 */

/*VARARGS2*/
int s_sprintf (VA_ARG(char *,s), VA_ARG(char *,format), VA_ALIST)
VA_OARGS(char *s;)
VA_OARGS(char *format;)
VA_DCL
{
    int rtnval;
    va_list args;

    VA_START (args, format);
    rtnval = s_vsprintf (s, format, args);
    va_end (args);
    return (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_fflush    invoke the library fflush function
 *
 *  SYNOPSIS
 *
 *	VOID s_fflush (stream)
 *	FILE *stream;
 *
 *  DESCRIPTION
 *
 *	Invokes the library fflush function.  Any error is
 *	immediately fatal.
 *
 */

VOID s_fflush (stream)
FILE *stream;
{
    DBUG_ENTER ("s_flush");
    if (fflush (stream) == EOF) {
	(VOID) abort ();
    }
    DBUG_VOID_RETURN;
}
 

/*
 *  FUNCTION
 *
 *	s_open    invoke file open library function
 *
 *  SYNOPSIS
 *
 *	int s_open (path, oflag [ , mode ])
 *	char *path;
 *	int oflag, mode;
 *
 *  DESCRIPTION
 *
 *	Invoke the library function to open a file.  Path points
 *	to a path name, oflag is for the file status flags, and
 *	mode is the file mode if the file is created.
 *
 */

int s_open (path, oflag, mode)
char *path;
int oflag, mode;
{
    int rtnval;

    DBUG_ENTER ("s_open");
    DBUG_PRINT ("open", ("file \"%s\", flag %o, mode %o", path, oflag, mode));
#if amigados
    if (AccessRawFloppy (path, 0) == 0) {
	DBUG_PRINT ("xopen", ("open '%s' as a raw device file", path));
	rtnval = OpenRawFloppy (path, oflag, mode);
    } else if (AccessRawTape (path, 0) == 0) {
	DBUG_PRINT ("xopen", ("open '%s' as a raw tape device", path));
	rtnval = OpenRawTape (path, oflag, mode);
    } else {
	DBUG_PRINT ("xopen", ("open '%s' as a normal file", path));
	rtnval = open (path, oflag, mode);
    }
#else
    rtnval = open (path, oflag, mode);
#endif
    DBUG_PRINT ("open", ("open on descriptor %d", rtnval));
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_ccreat    create a contiguous file
 *
 *  SYNOPSIS
 *
 *	int s_ccreat (path, mode, size)
 *	char *path;
 *	int mode;
 *	int size;
 *
 *  DESCRIPTION
 *
 *	Invoke the library function to creat a Masscomp style contiguous
 *	file.  Path points to a path name, mode is the file mode, and size
 *	is the initial size of the file.
 *
 *	The open will fail with errno ENOSPC if insufficient contiguous
 *	disk space is available.  In this case, we issue a warning
 *	that the file will not be contiguous, and open it as a normal
 *	noncontiguous file.
 *
 */

/*ARGSUSED*/	/* Turn off lint checking for unused args in s_ccreat */

int s_ccreat (path, mode, size)
char *path;
int mode;
long size;
{
    int rtnval;

    DBUG_ENTER ("s_ccreat");
    DBUG_PRINT ("ccreat", ("create contiguous file \"%s\"", path));
    DBUG_PRINT ("ccreat", ("mode %o, size %ld", mode, size));
#if HAVE_CONTIGUOUS
    rtnval = open (path, O_RDWR | O_CREAT | O_CTG | O_TRUNC, mode, size);
    if ((rtnval == SYS_ERROR) && (errno == ENOSPC)) {
	bru_errnor (MSG_COPEN, path);
	rtnval = open (path, O_RDWR | O_CREAT | O_TRUNC, mode, size);
    }
#else
    bru_message (MSG_CNOSUP, path);
    rtnval = open (path, O_RDWR | O_CREAT | O_TRUNC, mode);
#endif
    DBUG_PRINT ("ccreat", ("open on stream %d", rtnval));
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_close    invoke the low level file close facility
 *
 *  SYNOPSIS
 *
 *	int s_close (fildes)
 *	int fildes;
 *
 *  DESCRIPTION
 *
 *	Close the file for the given file descriptor.
 *
 */

int s_close (fildes)
int fildes;
{
    int rtnval;

    DBUG_ENTER ("s_close");
    DBUG_PRINT ("close", ("close descriptor %d", fildes));
#if amigados
    if (IsRawTapeFd (fildes)) {
	rtnval = CloseRawTape (fildes);
    } else if (IsRawFloppyFd (fildes)) {
	rtnval = CloseRawFloppy (fildes);
    } else {
	rtnval =  close (fildes);
    }
#else
    rtnval =  close (fildes);
#endif
    DBUG_RETURN (rtnval);
}
 


/*
 *  FUNCTION
 *
 *	s_access    invoke low level access library routine
 *
 *  SYNOPSIS
 *
 *	int s_access (path, amode)
 *	char *path;
 *	int amode;
 *
 *  DESCRIPTION
 *
 *	Invoke low level library routine "access" to test files
 *	for accessibility.
 *
 */

int s_access (path, amode)
char *path;
int amode;
{
    int rtnval;
    
    DBUG_ENTER ("s_access");
    DBUG_PRINT ("access", ("test '%s' for access %d", path, amode));
#if amigados
    if ((rtnval = AccessRawFloppy (path, amode)) != 0) {
	if ((rtnval = AccessRawTape (path, amode)) != 0) {
	    rtnval = access (path, amode);
	}
    }
#else
    rtnval = access (path, amode);
#endif
    DBUG_PRINT ("access", ("result = %d", rtnval));
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_read    invoke library function to read from file
 *
 *  SYNOPSIS
 *
 *	int s_read (fildes, buf, nbyte)
 *	int fildes;
 *	char *buf;
 *	UINT nbyte;
 *
 *  DESCRIPTION
 *
 *	Invoke library function to read from file.
 *
 */

int s_read (fildes, buf, nbyte)
int fildes;
char *buf;
UINT nbyte;
{
    int rtnval;
    
    DBUG_ENTER ("s_read");
    DBUG_PRINT ("read", ("read %u bytes from descriptor %d", nbyte, fildes));
#if amigados
    if (IsRawTapeFd (fildes)) {
	rtnval = ReadRawTape (fildes, buf, nbyte);
    } else if (IsRawFloppyFd (fildes)) {
	rtnval = ReadRawFloppy (fildes, buf, nbyte);
    } else {
	rtnval = read (fildes, buf, nbyte);
    }
#else
    rtnval = read (fildes, buf, nbyte);
#endif
    DBUG_PRINT ("read", ("read returns %d", rtnval));
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_chown    invoke library function to change file owner
 *
 *  SYNOPSIS
 *
 *	int s_chown (path, owner, group)
 *	char *path;
 *	int owner, group;
 *
 *  DESCRIPTION
 *
 *	Invoke low level library routine to change ownership of file.
 *
 */

int s_chown (path, owner, group)
char *path;
int owner, group;
{
    int rtnval;

    DBUG_ENTER ("s_chown");
#if unix || xenix
    rtnval = chown (path, owner, group);
#else
    rtnval = 0;
#endif
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_sleep    invoke library function to sleep for N seconds
 *
 *  SYNOPSIS
 *
 *	VOID s_sleep (seconds)
 *	UINT seconds;
 *
 *  DESCRIPTION
 *
 *	Invoke library function to sleep for specified number of
 *	seconds.
 *
 */

VOID s_sleep (seconds)
UINT seconds;
{
    DBUG_ENTER ("s_sleep");
#if unix || xenix
    (VOID) sleep (seconds);
#endif
#if amigados
    (VOID) Delay ((long) (50 * seconds));
#endif
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	s_uname    invoke library function to get system info
 *
 *  SYNOPSIS
 *
 *	int s_uname (name)
 *	struct utsname *name;
 *
 *  DESCRIPTION
 *
 *	Invoke library function to get information about host system.
 *
 */

int s_uname (name)
struct utsname *name;
{
    int rtnval;
    
    DBUG_ENTER ("s_uname");
    rtnval = uname (name);
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_umask    invoke library function to set/get file mask
 *
 *  SYNOPSIS
 *
 *	int s_umask (cmask)
 *	int cmask;
 *
 *  DESCRIPTION
 *
 *	Invoke library function to set and get the file creation mask.
 *
 */

int s_umask (cmask)
int cmask;
{
    int rtnval;

    DBUG_ENTER ("s_umask");
#if unix || xenix
    rtnval = umask (cmask);
#else
    rtnval = 0;
#endif
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_utime    set file access and modification times
 *
 *  SYNOPSIS
 *
 *	int s_utime (path, times)
 *	char *path;
 *	struct utimbuf *times;
 *
 *  DESCRIPTION
 *
 *	Invoke the library function utime to set file access and
 *	modification times.
 *
 *  NOTES
 *
 *	For non-unix systems, this is currently a NOP, though
 *	in theory it is possible to change the date for systems
 *	like the Amiga by accessing the raw file system (yetch!).
 *
 */

/*VARARGS1*/
int s_utime (path, times)
char *path;
struct utimbuf *times;
{
    int rtnval;

    DBUG_ENTER ("s_utime");
    rtnval = utime (path, times);
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_chmod    change mode of file
 *
 *  SYNOPSIS
 *
 *	int s_chmod (path, mode)
 *	char *path;
 *	int mode;
 *
 *  DESCRIPTION
 *
 *	Invoke library function to change the access mode of a file.
 *
 *  NOTES
 *
 *	On some systems there is a real problem with lint concerning the
 *	type of the second argument (sometimes declared as unsigned short
 *	in the lint library).  The declaration used here matches the actual
 *	code in the system V library and the System V Interface Definition
 *	published by AT&T.  If your lint complains then either fix your
 *	library or complain to your vender.  This has been fixed in the
 *	UniPlus 5.2 lint library.
 *
 *	For the Amiga, the owner modes are used.  Delete permission
 *	is always set.
 *
 */

int s_chmod (path, mode)
char *path;
int mode;
{
    int rtnval;

    DBUG_ENTER ("s_chmod");
    DBUG_PRINT ("chmod", ("chmod %s to %#x", path, mode));
#if amigados
    rtnval = 0;		/* build Lattice mode here */
    if (mode & BS_IREAD) {
	rtnval |= S_IREAD;
    }
    if (mode & BS_IWRITE) {
	rtnval |= (S_IWRITE | S_IDELETE);
    }
    if (mode & BS_IEXEC) {
	rtnval |= S_IEXECUTE;
    }
    mode = rtnval;
    DBUG_PRINT ("chmod", ("lattice mode is %#x", mode));
#endif
    rtnval = chmod (path, mode);
    DBUG_PRINT ("chmod", ("chmod result is %d", rtnval));
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_write    write data to a file
 *
 *  SYNOPSIS
 *
 *	int s_write (fildes, buf, nbyte)
 *	int fildes;
 *	char *buf;
 *	UINT nbyte;
 *
 *  DESCRIPTION
 *
 *	Invoke the low level I/O routine to write to a file.
 *
 */

int s_write (fildes, buf, nbyte)
int fildes;
char *buf;
UINT nbyte;
{
    int rtnval;

    DBUG_ENTER ("s_write");
    DBUG_PRINT ("write", ("write %u bytes to fildes %d", nbyte, fildes));
#if amigados
    if (IsRawTapeFd (fildes)) {
	rtnval = WriteRawTape (fildes, buf, nbyte);
    } else if (IsRawFloppyFd (fildes)) {
	rtnval = WriteRawFloppy (fildes, buf, nbyte);
    } else {
	rtnval = write (fildes, buf, nbyte);
    }
#else
    rtnval = write (fildes, buf, nbyte);
    DBUG_PRINT ("write", ("write returns %d, errno %d", rtnval, errno));
#endif
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_unlink    remove a directory entry
 *
 *  SYNOPSIS
 *
 *	int s_unlink (path)
 *	char *path;
 *
 *  DESCRIPTION
 *
 *	Invoke low level routine to remove a directory entry.
 *
 */

int s_unlink (path)
char *path;
{
    int rtnval;

    DBUG_ENTER ("s_unlink");
    rtnval = unlink (path);
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_time    get current system time
 *
 *  SYNOPSIS
 *
 *	long s_time (tloc)
 *	long *tloc;
 *
 *  DESCRIPTION
 *
 *	Invoke low level routine to get system time.
 *
 *	Note that BSD4_2 lint library defines both the argument and result
 *	as typedef time_t, which is an int, and the BSD4_2 online manual
 *	defines the types correctly (grrrrrrrr!!!!!).
 *
 */

long s_time (tloc)
long *tloc;
{
    long rtnval;

    DBUG_ENTER ("s_time");
#if BSD4_2 && lint
    rtnval = (long) time ((time_t *) tloc);
#else
    rtnval = time (tloc);
#endif
    DBUG_PRINT ("time", ("time is %ld", rtnval));
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_ulimit    get and set user limits
 *
 *  SYNOPSIS
 *
 *	long s_ulimit (cmd, newlimit)
 *	int cmd;
 *	long newlimit;
 *
 *  DESCRIPTION
 *
 *	Provides control over process limits.  Currently bru only
 *	uses it to get or set the maximum file size limit.
 *
 *	If we do not have a ulimit, then we assume there are no
 *	limits to a file's size.  Getting the limit returns the
 *	maximum value of an unsigned long.  Setting the limit is
 *	essentially a nop and returns the requested size.
 *
 *  BUGS
 *
 *	Determining the maximum value of an unsigned long is currently
 *	dependent on two's complement arithmetic.
 *
 */

long s_ulimit (cmd, newlimit)
int cmd;
long newlimit;
{
    long result;
#if !HAVE_ULIMIT
    unsigned long temp;
#endif

    DBUG_ENTER ("s_ulimit");
    DBUG_PRINT ("ulimit", ("cmd = %d; newlimit = %ld", cmd, newlimit));
#if HAVE_ULIMIT
    result = ulimit (cmd, newlimit);
#else
    switch (cmd) {
	case 1:
	    temp = ~0;
	    temp >>= 1;
	    result = (long) temp;
	    break;
	case 2:
	    result = newlimit;
	    break;
    }
#endif
    DBUG_PRINT ("ulimit", ("returns %ld", result));
    DBUG_RETURN (result);
}


/*
 *  FUNCTION
 *
 *	s_getuid    get real user ID of process
 *
 *  SYNOPSIS
 *
 *	int s_getuid ()
 *
 *  DESCRIPTION
 *
 *	Invoke low level routine to get the real user ID of process.
 *
 */

int s_getuid ()
{
    int rtnval;

    DBUG_ENTER ("s_getuid");
#if unix || xenix
    rtnval = getuid ();
#else
    rtnval = 0;
#endif
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_getgid    get real group ID of process
 *
 *  SYNOPSIS
 *
 *	int s_getgid ()
 *
 *  DESCRIPTION
 *
 *	Invoke low level routine to get the real group ID of process.
 *
 */

int s_getgid ()
{
    int rtnval;

    DBUG_ENTER ("s_getgid");
#if unix || xenix
    rtnval = getgid ();
#else
    rtnval = 0;
#endif
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_ctime    convert internal time to string
 *
 *  SYNOPSIS
 *
 *	char *s_ctime (clock)
 *	long *clock;
 *
 *  DESCRIPTION
 *
 *	Call standard library routine to convert from internal time
 *	format to string form.
 *
 *	Note that BSD4_2 lint library defines the argument as typedef
 *	time_t, which is an int, and the BSD4_2 online manual defines
 *	the type correctly (grrrrrrrr!!!!!).
 */

char *s_ctime (clock)
long *clock;
{
    char *rtnval;

    DBUG_ENTER ("s_ctime");
#if BSD4_2 && lint
    rtnval = ctime ((time_t *) clock);
#else
    rtnval = ctime (clock);
#endif
    DBUG_PRINT ("ctime", ("converted to '%s'", rtnval));
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_localtime    convert internal time to time structure
 *
 *  SYNOPSIS
 *
 *	struct tm *s_localtime (clock)
 *	long *clock;
 *
 *  DESCRIPTION
 *
 *	Call standard library routine to convert from internal time
 *	format to time structure form.
 *
 *	Note that BSD4_2 lint library defines the argument as "time_t *"
 *	where time_t is typedef as an int, and the BSD4_2 online manual defines
 *	the type correctly (grrrrrrrr!!!!!).
 */

struct tm *s_localtime (clock)
long *clock;
{
    struct tm *rtnval;

    DBUG_ENTER ("s_localtime");
    DBUG_PRINT ("localtime", ("convert time %ld", *clock));
#if BSD4_2 && lint
    rtnval = localtime ((time_t *) clock);
#else
    rtnval = localtime (clock);
#endif
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_asctime    convert time structure to ascii string
 *
 *  SYNOPSIS
 *
 *	char *s_asctime (tm)
 *	struct tm *tm;
 *
 *  DESCRIPTION
 *
 *	Call standard library routine to convert from time structure form
 *	to an ascii string.
 *
 */

char *s_asctime (tm)
struct tm *tm;
{
    char *rtnval;

    DBUG_ENTER ("s_asctime");
    rtnval = asctime (tm);
    DBUG_PRINT ("asctime", ("asctime returns '%s'", rtnval));
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_fopen    open a stream
 *
 *  SYNOPSIS
 *
 *	FILE *s_fopen (file_name, type)
 *	char *file_name;
 *	char *type;
 *
 *  DESCRIPTION
 *
 *	Invokes the standard library routine to open a stream.
 *
 */

FILE *s_fopen (file_name, type)
char *file_name;
char *type;
{
    FILE *rtnval;
    
    DBUG_ENTER ("s_fopen");
    rtnval = fopen (file_name, type);
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_mknod    make a directory, or a special or ordinary file
 *
 *  SYNOPSIS
 *
 *	int s_mknod (path, mode, dev)
 *	char *path;
 *	int mode, dev;
 *
 *  DESCRIPTION
 *
 *	Invokes the low level function mknod to make a directory entry.
 *
 *	If called on a symbolic link, report an error, since that code
 *	should go through s_symlink and/or s_csymlink.
 *
 *	!!! BEWARE !!!  On BSD4.1 systems, the dev parameter must be
 *	zero for directories or regular files or nasty filesystem
 *	things happen.
 *
 *	I don't know if this applies to 4.2 as well, but I'm going to
 *	assume that it does.  The manual says that 'dev' is ignored, but
 *	Berkeley doc isn't always right.  ADR.
 */

int s_mknod (path, mode, dev)
char *path;
int mode, dev;
{
    int rtnval;

    DBUG_ENTER ("s_mknod");
#if BSD4_1 || BSD4_2
    if (IS_DIR (mode) || IS_REG (mode)) {
	dev = 0;
    }
#endif
    DBUG_PRINT ("mknod", ("make node \"%s\" mode %o device %x", path, mode, dev));
    if (IS_FLNK (mode)) {
	bru_message (MSG_BUG, "s_mknod (symlink)");
    }
#if unix || xenix
    rtnval = mknod (path, mode, dev);
#else
    (VOID) s_fprintf (errfp, "warning -- mknod() not implemented!\n");
    rtnval = SYS_ERROR;
#endif
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_strtol   convert string to a long integer
 *
 *  SYNOPSIS
 *
 *	long s_strtol (str, ptr, base)
 *	char *str;
 *	char **ptr;
 *	int base;
 *
 *  DESCRIPTION
 *
 *	Invoke standard library routine to convert string to a long
 *	integer.
 *
 */

S32BIT s_strtol (str, ptr, base)
char *str;
char **ptr;
int base;
{
    long rtnval;

    DBUG_ENTER ("s_strtol");
    rtnval = strtol (str, ptr, base);
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_malloc    allocate memory from system
 *
 *  SYNOPSIS
 *
 *	char *s_malloc (size)
 *	UINT size;
 *
 *  DESCRIPTION
 *
 *	Call standard library routine to allocate memory.
 *
 */

char *s_malloc (size)
UINT size;
{
    char *rtnval;

    errno = 0;					/* 286 bug workaround */
    rtnval = malloc (size);
    return (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_exit    terminate process
 *
 *  SYNOPSIS
 *
 *	VOID s_exit (status)
 *	int status;
 *
 *  DESCRIPTION
 *
 *	Call the low level routine to terminate process.
 *
 */

VOID s_exit (status)
int status;
{
    DBUG_ENTER ("s_exit");
    DBUG_PRINT ("exit", ("exiting with status %d", status));
#if BSD4_2
    (VOID) exit (status);
#else
    exit (status);
#endif
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	s_getopt    parse command line options
 *
 *  SYNOPSIS
 *
 *	int s_getopt (argc, argv, optstring)
 *	int argc;
 *	char **argv;
 *	char *optstring;
 *
 */

int s_getopt (argc, argv, optstring)
int argc;
char **argv;
char *optstring;
{
    int rtnval;

    DBUG_ENTER ("s_getopt");
    rtnval = getopt (argc, argv, optstring);
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_free    free memory received via malloc
 *
 *  SYNOPSIS
 *
 *	VOID s_free (ptr)
 *	char *ptr;
 *
 *  DESCRIPTION
 *
 *	Used to free memory allocated via the standard library
 *	memory allocator (malloc).
 *
 */

VOID s_free (ptr)
char *ptr;
{
    DBUG_ENTER ("s_free");
#if BSD4_2
    (VOID) free (ptr);
#else
    free (ptr);
#endif
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	s_getpwent    get password file entry
 *
 *  SYNOPSIS
 *
 *	struct passwd *s_getpwent ()
 *
 *  DESCRIPTION
 *
 *	Invoke standard library routine to get a password file entry.
 *
 */

#if !unix && !xenix
static BOOLEAN pwentopen = FALSE;
static struct passwd pwentfake = {
    "root",
    "NONE",
    0,
    0,
    "NONE",
    "NONE",
    "NONE",
    "/",
    "/bin/sh"
};
#endif

struct passwd *s_getpwent ()
{
    struct passwd *rtnval;
#if unix || xenix
    rtnval = getpwent ();
#else
    if (pwentopen) {
	rtnval = (struct passwd *)NULL;
    } else {
	pwentopen = TRUE;
	rtnval = &pwentfake;
    }
#endif
    return (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_endpwent    close password file when done with it
 *
 *  SYNOPSIS
 *
 *	VOID s_endpwent ()
 *
 *  DESCRIPTION
 *
 *	Calls standard library function endpwent to close password
 *	file when done with it.
 *
 */

VOID s_endpwent ()
{
#if unix || xenix
#if BSD4_2
    (VOID) endpwent ();
#else
    endpwent ();
#endif
#else
    pwentopen = FALSE;
#endif
}


/*
 *  FUNCTION
 *
 *	s_getgrent    get group file entry
 *
 *  SYNOPSIS
 *
 *	struct group *s_getgrent ()
 *
 *  DESCRIPTION
 *
 *	Invoke standard library routine to get a group file entry.
 *
 */

#if !unix && !xenix
static BOOLEAN grentopen = FALSE;
static struct group grentfake = {
    "root",
    "NONE",
    0,
    NULL
};
#endif

struct group *s_getgrent ()
{
    struct group *rtnval;

#if unix || xenix
    rtnval = getgrent ();
#else
    if (grentopen) {
	rtnval = (struct group *)NULL;
    } else {
	grentopen = TRUE;
	rtnval = &grentfake;
    }
#endif
    return (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_endgrent    close group file when done with it
 *
 *  SYNOPSIS
 *
 *	VOID s_endgrent ()
 *
 *  DESCRIPTION
 *
 *	Calls standard library function endpwent to close group
 *	file when done with it.
 *
 */

VOID s_endgrent ()
{
#if unix || xenix
#if BSD4_2
    (VOID) endgrent ();
#else
    endgrent ();
#endif
#else
    grentopen = FALSE;
#endif
}


/*
 *  FUNCTION
 *
 *	s_signal    set up signal handling
 *
 *  SYNOPSIS
 *
 *	SIGTYPE s_signal (sig, func)
 *	int sig;
 *	SIGTYPE func;
 *
 *  DESCRIPTION
 *
 *	Set signal handling by calling low level routine "signal".
 *
 *  NOTES
 *
 *	There is considerable disagreement, between different implementations
 *	and proposed standards, for the declared type of signal() and it's
 *	second argument.  For example:
 *
 *		(1) System V Interface Definition from AT&T
 *
 *			int (*signal (sig, func)()) { int sig; int (*func)();
 *
 *		(2) Proposed ANSI C standard
 *
 *			void (*signal (sig, func)()) { int sig; void (*func)();
 *
 *		(3) 68000 System V generic release (Motorola Microport)
 *
 *			int (*signal (sig, func)()) { int sig; int (*func)();
 *			(in library)
 *
 *			int (*signal (sig, func)()) { int sig; void (*func)();
 *			(in lint and documentation for signal(2))
 *
 *	The inconsistency in (3) has been corrected in UniSoft's UniPlus
 *	based on the 68000 generic release, by fixing the lint library to
 *	be in agreement with the code and header files (change void to int).
 *
 *	If your lint complains about this then complain to your vendor!
 *
 */

SIGTYPE s_signal (sig, func)
int sig;
SIGTYPE func;
{
    SIGTYPE rtnval;

    DBUG_ENTER ("s_signal");
    DBUG_PRINT ("signals", ("set signal (%d,%lx)", sig, func));
#if unix || xenix || amigados
    rtnval = signal (sig, func);
#else
    DBUG_PRINT ("signals", ("signals not implemented in this system"));
    rtnval = (SIGTYPE) SYS_ERROR;
#endif
    DBUG_PRINT ("signals", ("signal returns %d", rtnval));
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_ioctl    issue low level function to control device
 *
 *  SYNOPSIS
 *
 *	int s_ioctl (fildes, request, arg)
 *	int fildes;
 *	int request;
 *	int arg;
 *
 *  DESCRIPTION
 *
 *	Issue the low level request to control device.
 *
 */

int s_ioctl (fildes, request, arg)
int fildes, request, arg;
{
    int rtnval;

    DBUG_ENTER ("s_ioctl");
    DBUG_PRINT ("ioctl", ("ioctl on fildes %d", fildes));
    DBUG_PRINT ("ioctl", ("request %d, arg %d", request, arg));
#if unix || xenix
#if BSD4_2
    rtnval = ioctl (fildes, request, (char *) arg);
#else
    rtnval = ioctl (fildes, request, arg);
#endif
#else
    (VOID) s_fprintf (errfp, "warning -- ioctl() not implemented!\n");
    rtnval = SYS_ERROR;
#endif
    DBUG_PRINT ("ioctl", ("ioctl returns %d", rtnval));
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_strcat    concatenate two strings
 *
 *  SYNOPSIS
 *
 *	char *s_strcat (s1, s2)
 *	char *s1, *s2;
 *
 *  DESCRIPTION
 *
 *	Append copy of string s2 to end of string s1.
 *
 */

char *s_strcat (s1, s2)
char *s1, *s2;
{
    char *rtnval;
    
    rtnval = strcat (s1, s2);
    return (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_strcmp    compare two strings lexicographically
 *
 *  SYNOPSIS
 *
 *	int s_strcmp (s1, s2)
 *	char *s1, *s2;
 *
 *  DESCRIPTION
 *
 *	Compare string s1 with string s2 and return an integer less
 *	than, equal to, or greater than zero according as s1 is
 *	lexicographically less than, equal to, or greater than s2.
 *
 */

int s_strcmp (s1, s2)
char *s1, *s2;
{
    int rtnval;

    rtnval = strcmp (s1, s2);
    return (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_strcpy    copy strings from one location to another
 *
 *  SYNOPSIS
 *
 *	char *s_strcpy (s1, s2)
 *	char *s1, *s2;
 *
 *  DESCRIPTION
 *
 *	Copy string s2 to s1, stopping after the null character has been
 *	copied.
 *
 */

char *s_strcpy (s1, s2)
char *s1, *s2;
{
    char *rtnval;
    
    rtnval = strcpy (s1, s2);
    return (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_strncpy    copy strings from one location to another
 *
 *  SYNOPSIS
 *
 *	char *s_strncpy (s1, s2, n)
 *	char *s1, *s2;
 *	int n;
 *
 *  DESCRIPTION
 *
 *	Copy n characters from string s2 to s1, truncating s2 or
 *	padding s1 with nulls as required.
 *
 */

char *s_strncpy (s1, s2, n)
char *s1, *s2;
int n;
{
    char *rtnval;
    
    rtnval = strncpy (s1, s2, n);
    return (rtnval);
}



/*
 *  FUNCTION
 *
 *	s_strlen    find length of string
 *
 *  SYNOPSIS
 *
 *	int s_strlen (s)
 *	char *s;
 *
 *  DESCRIPTION
 *
 *	Count characters in string s, not including terminating null.
 *
 */

int s_strlen (s)
char *s;
{
    int rtnval;

    rtnval = strlen (s);
    return (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_strchr    find first occurance of character in string
 *
 *  SYNOPSIS
 *
 *	char *s_strchr (s, c)
 *	char *s;
 *	int c;
 *
 *  DESCRIPTION
 *
 *	Search forward in string s until first occurance of
 *	string character c, returning pointer.  If strchr is available,
 *	just use it.  If not, and index is available, substitute it
 *	instead.  Otherwise just do brute force.
 *
 */

char *s_strchr (s, c)
char *s;
int c;
{
    char *rtnval;

#if HAVE_STRCHR
    rtnval = strchr (s, c);
#else
#if HAVE_INDEX
    rtnval = index (s, c);
#else
    rtnval = NULL;
    if (s != NULL) {
	while (*s != EOS && *s != c) {s++;}
	if (*s == c) {
	    rtnval = s;
	}
    }
#endif	/* HAVE_INDEX */
#endif	/* HAVE_STRCHR */
    return (rtnval);
}



/*
 *  FUNCTION
 *
 *	s_strrchr    find last given character in string
 *
 *  SYNOPSIS
 *
 *	char *s_strrchr (s, c)
 *	char *s;
 *	char c;
 *
 *  DESCRIPTION
 *
 *	Find last occurance of the given character c in the string s.
 *	Use strrchr if available.  If not, and rindex is available,
 *	use that.  Otherwise just do brute force.
 *
 */

char *s_strrchr (s, c)
char *s;
int c;
{
    char *rtnval;
#if HAVE_STRRCHR
    rtnval = strrchr (s, c);
#else
#if HAVE_RINDEX
    rtnval = rindex (s, c);
#else
    if (s == NULL) {
	rtnval = NULL;
    } else {
	rtnval = s;
	while (*s != EOS) {s++;}
	while (s > rtnval && *s != c) {s--;}
	if (*s == c) {
	    rtnval = s;
	} else {
	    rtnval = NULL;
	}
    }
#endif	/* HAVE_RINDEX */
#endif	/* HAVE_STRRCHR */
    return (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_mkdir    make a directory node
 *
 *  SYNOPSIS
 *
 *	int s_mkdir (path, mode)
 *	char *path;
 *	int mode;
 *
 *  DESCRIPTION
 *
 *	Contains code to make a directory node and its entries
 *	"." and ".."
 *
 *	This is a system call under 4.2, so we just use it.
 *	Also just a system call under UniPlus with NFS.
 *
 *	Note that the effective user id must be root for this
 *	to work.  Thus this is only usable by root or if the
 *	process is owned by root and has the SUID bit set.
 *
 *	As an alternative, the system "mkdir" command could
 *	be forked but this is much faster.  This is probably
 *	fertile ground for anyone looking for system security
 *	loopholes.
 *
 *	Note that the parent directory must be writable by the
 *	real user.
 *
 *	Under 4.2, 'mkdir' is a system call, so this routine will just
 *	use the system call.  Using the system call should help
 *	security issues too.  Similarly UniPlus with NFS...
 *
 *	Note, this routine may not be completely bulletproof.  How
 *	will it behave if the name ends with a '/', as in
 *	"/usr/bin/"?  How will BSD4.2 mkdir() behave in this case?
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin mkdir
 *	    Default return value is SYS_ERROR
 *	    If path will not fit in working buffer then
 *		Zap errno to prevent misleading messages
 *	    Else
 *		Copy file name to local work buffer
 *		Find stem and leaf separator
 *		If there is a leaf string then
 *		    Erase leaf, leaving stem and '/'
 *		    Build name by adding "." to stem
 *		Else
 *		    Set the name to "."
 *		End if
 *		If the parent directory can be written then
 *		    If the directory node is made successfully then
 *			Copy name to entries buffer
 *			Build name for first entry
 *			If the entry cannot be linked then
 *			    Unlink the directory file
 *			Else
 *			    Build name for second entry
 *			    If the entry cannot be linked then
 *				Build name for previous entry
 *				Unlink previous entry
 *				Unlink the directory
 *			    Else
 *				Directory was made successfully
 *			    End if
 *			End if
 *		    End if
 *		End if
 *	    End if
 *	    Return result
 *	End mkdir
 *
 */



#if !amigados

int s_mkdir (path, mode)
char *path;
int mode;
{
    int rtnval;

#if HAVE_MKDIR

    DBUG_ENTER ("s_mkdir");
    rtnval = mkdir (path, mode);
    DBUG_RETURN (rtnval);

#else

    char *leaf;
    char parent[BRUPATHMAX];
    char entries[BRUPATHMAX+3];		/* Leave room for "/.." */

    DBUG_ENTER ("s_mkdir");
    rtnval = SYS_ERROR;
    if (s_strlen (path) >= BRUPATHMAX) {
	errno = ENAMETOOLONG;
	DBUG_PRINT ("mkdir", ("pathname too long!"));
    } else {
	(VOID) s_strcpy (parent, path);
	leaf = s_strrchr (parent, '/');
	if (leaf != NULL) {
	    *++leaf = EOS;
	    (VOID) s_strcat (parent, ".");
	} else {
	    (VOID) s_strcpy (parent, ".");
	}
	DBUG_PRINT ("mkdir", ("test %s for access %d", parent, A_WRITE));
	if (s_access (parent, A_WRITE) == 0) {
	    DBUG_PRINT ("mkdir", ("parent writable, make the %s node", path));
	    if (s_mknod (path, mode, 0) != SYS_ERROR) {
		DBUG_PRINT ("mkdir", ("node made, make the . & .. entries"));
		(VOID) s_strcpy (entries, path);
		(VOID) s_strcat (entries, "/.");
		if (s_link (path, entries) == SYS_ERROR) {
		    (VOID) s_unlink (path);
		} else {
		    (VOID) s_strcat (entries, ".");
		    if (s_link (parent, entries) == SYS_ERROR) {
			entries[s_strlen(entries)-1] = EOS;
			(VOID) s_unlink (entries);
			(VOID) s_unlink (path);
		    } else {
			DBUG_PRINT ("mkdir", ("directory made"));
			rtnval = 0;
		    }
		}
	    }
	}
    }
    DBUG_RETURN (rtnval);
#endif
}

#endif	/* !amigados */


/*
 *  FUNCTION
 *
 *	s_lseek    seek to location in file
 *
 *  SYNOPSIS
 *
 *	S32BIT s_lseek (fildes, offset, whence)
 *	int fildes;
 *	S32BIT offset;
 *	int whence;
 *
 *  DESCRIPTION
 *
 *	Invoke the standard library function to seek to file
 *	location.
 *
 */

S32BIT s_lseek (fildes, offset, whence)
int fildes;
S32BIT offset;
int whence;
{
    S32BIT rtnval;
    
    DBUG_ENTER ("s_lseek");
    DBUG_PRINT ("lseek", ("lseek on fildes %d", fildes));
    DBUG_PRINT ("lseek", ("offset %ld, whence %d", offset, whence));
#if amigados
    if (IsRawTapeFd (fildes)) {
	rtnval = LseekRawTape (fildes, offset, whence);
    } else if (IsRawFloppyFd (fildes)) {
	rtnval = LseekRawFloppy (fildes, offset, whence);
    } else {
	rtnval = lseek (fildes, offset, whence);
    }
#else
    rtnval = lseek (fildes, offset, whence);
#endif
    DBUG_PRINT ("lseek", ("lseek returns %ld", rtnval));
    DBUG_RETURN (rtnval);
}


int s_tolower (c)
int c;
{
#if BSD4_2
    /* BSD tolower blindly converts, while S5 checks, so we check for it */
    if (!isupper (c)) {
	return (c);
    } else {
	return (tolower (c));
    }
#else
    return (tolower (c));
#endif
}


int s_creat (path, mode)
char *path;
int mode;
{
    int rtnval;
    
    DBUG_ENTER ("s_creat");
    rtnval = creat (path, mode);
    DBUG_RETURN (rtnval);
}


int s_isdigit (c)
int c;
{
    return (isdigit (c));
}


int s_atoi (str)
char *str;
{
    int rtnval;
    
    DBUG_ENTER ("s_atoi");
    rtnval = atoi (str);
    DBUG_RETURN (rtnval);
}


int s_fileno (stream)
FILE *stream;
{
    return (fileno (stream));
}


/*
 *	If we have memcpy() then just use it.  Otherwise, we can
 *	fake it with bcopy() if available.  If neither, then just
 *	do it brute force.
 */

char *s_memcpy (s1, s2, n)
char *s1;
char *s2;
int n;
{
#if HAVE_MEMCPY
    return (memcpy (s1, s2, n));
#else
#if HAVE_BCOPY
    (VOID) bcopy (s2, s1, n);
    return (s1);
#else
    char *saved_s1 = s1;
    while (n-- > 0) {
	*s1++ = *s2++;
    }
    return (saved_s1);
#endif	/* HAVE_BCOPY */
#endif	/* HAVE_MEMCPY */
}

/*
 *	If we have memset() then just use it.  Otherwise, just do
 *	it brute force.
 *
 *	We could optimize here and do block memory sets one longword
 *	at a time.  This may be a significant improvement if memset is
 *	not available.
 *
 */

char *s_memset (s, c, n)
char *s;
int c;
int n;
{
#if HAVE_MEMSET
    return (memset (s, c, n));
#else
    char *saved_s = s;
    while (n-- > 0) {
	*s++ = c;
    }
    return (saved_s);
#endif	/* HAVE_MEMSET */
}


int s_link (path1, path2)
char *path1, *path2;
{
    int rtnval;
    
    DBUG_ENTER ("s_link");
    DBUG_PRINT ("link", ("link \"%s\" to existing \"%s\"", path2, path1));
#if unix || xenix
    rtnval = link (path1, path2);
#else
    rtnval = SYS_ERROR;		/* No obvious way to fake it */
#endif
    DBUG_RETURN (rtnval);
}


/*
 *	For the amigados version, we only need to fake a dup for
 *	stdin, stdout, or stderr by simply returning the given fildes.
 *	This should work ok for what we want to do with it...
 */

int s_dup (fildes)
int fildes;
{
    int rtnval;
    
    DBUG_ENTER ("s_dup");
#if unix || xenix
    rtnval = dup (fildes);
#else
#if amigados
    if (fildes == 0 || fildes == 1 || fildes == 2) {
	rtnval = fildes;
    } else {
	errno = EBADF;
	rtnval = SYS_ERROR;
    }
#else
    (VOID) s_fprintf (errfp, "warning -- dup() not implemented!\n");
    rtnval = SYS_ERROR;
#endif
#endif
    DBUG_RETURN (rtnval);
}


char *s_getenv (name)
char *name;
{
    char *rtnval;
    
    DBUG_ENTER ("s_getenv");
    rtnval = getenv (name);
    DBUG_RETURN (rtnval);
}


int s_fork ()
{
    int rtnval;
    
    DBUG_ENTER ("s_fork");
#if unix || xenix
    rtnval = fork ();
#else
    rtnval = SYS_ERROR;
#endif
    DBUG_PRINT ("fork", ("fork returns %d", rtnval));
    DBUG_RETURN (rtnval);
}



/*
 *	Under SVR4, wait() returns type pid_t, which is a long.
 *	So we make our internal version return long in all cases.
 */

#if !amigados

long s_wait (stat_loc)
int *stat_loc;
{
    long rtnval;
    
    DBUG_ENTER ("s_wait");
    DBUG_PRINT ("wait", ("wait with &status = %lx", stat_loc));
#if HAVE_UNIONWAIT
    rtnval = wait ((union wait *) stat_loc);
    if (stat_loc != NULL) {
	DBUG_PRINT ("wait", ("status = %x", stat_loc -> w_status));
    }
#else
    rtnval = wait (stat_loc);
    if (stat_loc != NULL) {
	LINTCOOKIE;
	DBUG_PRINT ("wait", ("status = %x", *stat_loc));
    }
#endif
    DBUG_PRINT ("wait", ("wait returns %ld", rtnval));
    DBUG_RETURN (rtnval);
}

#endif	/* !amigados */


/*
 *	The BSD4_2 lint library has a totally screwed up entry for execv.
 *	It has no explicitly declared return type, so defaults to int, but
 *	then does not explicitly return anything so lint complains about
 *	any code that tries to use the return value.
 */

int s_execv (path, argv)
char *path;
char *argv[];
{
    int rtnval;
    
    DBUG_ENTER ("s_execv");
#if BSD4_2 && lint
    (VOID) execv (path, argv);			/* Lint library lies! */
    rtnval = -1;
#else
#if amigados
    (VOID) s_fprintf(errfp, "Sorry, execv not yet implemented!\n");
    rtnval = -1;
#else
    rtnval = execv (path, argv);
#endif
#endif
    DBUG_RETURN (rtnval);
}


int s_setuid (uid)
int uid;
{
    int rtnval;
    
    DBUG_ENTER ("s_setuid");
#if unix || xenix
    rtnval = setuid (uid);
#else
    rtnval = 0;
#endif
    DBUG_RETURN (rtnval);
}


int s_setgid (gid)
int gid;
{
    int rtnval;
    
    DBUG_ENTER ("s_setgid");
#if unix || xenix
    rtnval = setgid (gid);
#else
    rtnval = 0;
#endif
    DBUG_RETURN (rtnval);
}


int s_fclose (stream)
FILE *stream;
{
    int rtnval;
    
    DBUG_ENTER ("s_fclose");
    rtnval = fclose (stream);
    DBUG_RETURN (rtnval);
}


char *s_fgets (s, n, stream)
char *s;
int n;
FILE *stream;
{
    char *rtnval;
    
    rtnval = fgets (s, n, stream);
    return (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_readlink    read a symbolic link
 *
 *  SYNOPSIS
 *
 *	int s_readlink (name, buf, size);
 *	char *name;
 *	char *buf;
 *	int size;
 *
 *  DESCRIPTION
 *
 *	Read a symbolic link on systems that have symbolic links.
 *	Is a NOP on other systems and just returns SYS_ERROR.
 *
 *	Under SUN's 3.2 OS, readlink() may return garbage if the first
 *	argument is one of the special files in /dev.  So, we must
 *	avoid calling readlink for anything except real symbolic links.
 *	Readlink is documented to leave ENXIO in errno if the name is
 *	not a symbolic link, so that's what we do also...
 *
 */

/*ARGSUSED*/	/* Turn off lint checking for unused args in s_readlink */

int s_readlink (name, buf, size)
char *name, *buf;
int size;
{
    int rtnval;
#if HAVE_SYMLINKS
    struct bstat bsbuf;			/* Used to avoid SUN 3.2 bug */
#endif

    DBUG_ENTER ("s_readlink");
#if HAVE_SYMLINKS
    if (bstat (name, &bsbuf, FALSE) == NULL) {
	rtnval = SYS_ERROR;
    } else {
	if (IS_FLNK (bsbuf.bst_mode)) {
	    rtnval = readlink (name, buf, size);
	} else {
	    errno = ENXIO;
	    rtnval = SYS_ERROR;
	}
    }
#else
    rtnval = SYS_ERROR;
#endif
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	s_timezone    get local time zone information
 *
 *	S5 defines an extern long timezone == difference in seconds between
 *	GMT and local standard time. BSD does not have this extern, but it
 *	will tell us what our timezone is in minutes west of GMT. So we
 *	just compute it on the fly.
 *
 */

#define SEC_PER_MIN (60)

#if HAVE_TZSET

long s_timezone ()
{
    tzset ();
    return (timezone);
}

#else

#if HAVE_GETTIMEOFDAY

long s_timezone ()
{
    long timezone;
    struct timeval tv;
    struct timezone tz;

    (VOID) gettimeofday (&tv, &tz);
    timezone = tz.tz_minuteswest * SEC_PER_MIN;
    return (timezone);
}

#endif	/* HAVE_GETTIMEOFDAY */
#endif	/* HAVE_TZSET */


#if HAVE_SYMLINKS

/*
 * Supply these routines on our own for 4.2.  Make them static (if
 * appropriate) to keep this stuff all in one file.
 */

/* s_symlink --- make a symbolic link */

int s_symlink (name1, name2)
char *name1, *name2;
{
    int rtnval;

    DBUG_ENTER ("s_symlink");
    rtnval = symlink (name1, name2);
    DBUG_RETURN (rtnval);
}

#if pyr

/* s_csymlink --- make a conditional symbolic link */

int s_csymlink (name1, name2)
char *name1, *name2;
{
    int rtnval;

    DBUG_ENTER ("s_symlink");
    rtnval = csymlink (name1, name2);
    DBUG_RETURN (rtnval);
}

#endif	/* pyr */
#endif	/* HAVE_SYMLINKS */

/*ARGSUSED*/	/* Turn off lint checking for unused args in s_eject */

int s_eject (fildes)
int fildes;
{
    int rtnval = 0;

    DBUG_ENTER ("s_eject");
    DBUG_PRINT ("eject", ("eject media on fildes %d", fildes));
#if AUX
    rtnval = s_ioctl (fildes, AL_EJECT, 0);
#endif
    DBUG_RETURN (rtnval);
}


/*ARGSUSED*/	/* Turn off lint checking for unused args in s_format */

s_format (fildes)
int fildes;
{
    int status = 0;
#if AUX
    struct diskformat fmt;

    fmt.d_secsize = 512;
    fmt.d_dens = 2;
    fmt.d_fsec = 0;
    fmt.d_lsec = 1599;
    fmt.d_fhead = 0;
    fmt.d_lhead = 1;
    fmt.d_fcyl = 0;
    fmt.d_lcyl = 79;
    fmt.d_ileave = 1;
    status = !(ioctl (fildes, UIOCFORMAT, &fmt) == -1);
#endif
    return (status);
}


/*
 *  FUNCTION
 *
 *	s_strdup   make a duplicate of a string in new memory
 *
 *  SYNOPSIS
 *
 *	char *s_strdup (string)
 *	char *string;
 *
 *  DESCRIPTION
 *
 *	Given pointer to a string, allocates sufficient memory to make
 *	a duplicate copy, and copies the string to the newly allocated
 *	memory.  Returns pointer to the new string on success, NULL
 *	on failure.
 *
 */


char *s_strdup (string)
char *string;
{
    char *new;

    if ((new = s_malloc ((UINT) (strlen (string) + 1))) != NULL) {
	(VOID) s_strcpy (new, string);
    }
    return (new);
}

#if HAVE_SHM

int s_kill (pid, sig)
int pid;
int sig;
{
    int status;

    DBUG_ENTER ("s_kill");
    DBUG_PRINT ("kill", ("kill pid %d with sig %d", pid, sig));
    status = kill (pid, sig);
    DBUG_PRINT ("kill", ("kill returns %d", status));
    DBUG_RETURN (status);
}

#endif
