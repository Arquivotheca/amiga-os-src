/*
 *	This file contains intersystem compatibility routines derived from
 *	various public domain sources.  This file is not subject to any of
 *	the licensing or distribution restrictions applicable to the rest
 *	of the bru source code.  It is provided to improved transportability
 *	of bru source code between various flavors of unix.
 *
 *	NOTE:  The remote tape drive access routines are derived from a
 *	public domain version of the remote tape library originally
 *	produced at Georgia Tech.  They are provided for convenience
 *	in using the bru source code on systems where the library has
 *	not been installed.
 *
 *	They have been reformatted to fit in with the style in the
 *	rest of the bru source code but other than that are essentially
 *	unchanged.
 *
 *		Fred Fish  28-Oct-85
 *
 */

#define RMT_RUNTIME 1	/* Don't do function remapping for runtime support */

#include "globals.h"

#if RMT

static int _rmt_close PROTO((int fildes));
static _rmt_ioctl PROTO((int fildes, int op, IOCTL3 arg));
static long _rmt_lseek PROTO((int fildes, long offset, int whence));
static int _rmt_open PROTO((char *path, int oflag, int mode));
static int _rmt_read PROTO((int fildes, char *buf, unsigned int nbyte));
static int _rmt_write PROTO((int fildes, char *buf, unsigned int nbyte));

#ifndef EOPNOTSUPP
#define EOPNOTSUPP EINVAL
#endif

#if BSD
#define strchr index
typedef int NBYTETYPE;		/* Funny things, system calls... */
typedef char *IOCTL3;
#else
# ifdef sgi
typedef char *IOCTL3;
# else
typedef int IOCTL3;
# endif
typedef unsigned int NBYTETYPE;
#endif

/*
 *	Note that local vs remote file descriptors are distinguished
 *	by adding a bias to the remote descriptors.  This is a quick
 *	and dirty trick that may not be portable to some systems.
 */

#define REM_BIAS 128


/*
 *	BUFMAGIC --- Magic buffer size
 *	MAXUNIT --- Maximum number of remote tape file units
 */

#define BUFMAGIC	64
#define MAXUNIT		4

/*
 *	Useful macros.
 *
 *	READ --- Return the number of the read side file descriptor
 *	WRITE --- Return the number of the write side file descriptor
 */

#define READ(fd)	(_rmt_Ctp[fd][0])
#define WRITE(fd)	(_rmt_Ptc[fd][1])

static int _rmt_Ctp[MAXUNIT][2] = {
    -1, -1, -1, -1, -1, -1, -1, -1
};

static int _rmt_Ptc[MAXUNIT][2] = {
    -1, -1, -1, -1, -1, -1, -1, -1
};

/*
 *	Isrmt. Let a programmer know he has a remote device.
 */

int isrmt (fd)
int fd;
{
    int rtnval;

    DBUG_ENTER ("isrmt");
    rtnval = fd >= REM_BIAS;
    DBUG_RETURN (rtnval);
}

/*
 *	abort --- close off a remote tape connection
 */

VOID _rmt_abort (fildes)
int fildes;
{
    DBUG_ENTER ("_rmt_abort");
    (VOID) close (READ (fildes));
    (VOID) close (WRITE (fildes));
    READ (fildes) = -1;
    WRITE (fildes) = -1;
    DBUG_VOID_RETURN;
}

/*
 *	Test pathname for specified access.  Looks just like access(2)
 *	to caller.
 */

int rmtaccess (path, amode)
char *path;
int amode;
{
    int rtnval;

    DBUG_ENTER ("rmtaccess");
    if (_rmt_dev (path)) {
	rtnval = 0;		/* Let /etc/rmt find out */
    } else {
	rtnval = access (path, amode);
    }
    DBUG_RETURN (rtnval);
}

/*
 *	Close a file.  Looks just like close(2) to caller.
 */

int rmtclose (fildes)
int fildes;
{
    int rtnval;

    DBUG_ENTER ("rmtclose");
    if (isrmt (fildes)) {
	rtnval = _rmt_close (fildes - REM_BIAS);
    } else {
	rtnval = close (fildes);
    }
    DBUG_RETURN (rtnval);
}

/*
 *	_rmt_close --- close a remote magtape unit and shut down
 */

static int _rmt_close (fildes)
int fildes;
{
    int rtnval;

    DBUG_ENTER ("_rmt_close");
    if (_rmt_command (fildes, "C\n") != -1) {
	rtnval = _rmt_status (fildes);
	_rmt_abort (fildes);
    } else {
	rtnval = -1;
    }
    DBUG_RETURN (rtnval);
}

/*
 *	_rmt_command --- attempt to perform a remote tape command
 */

int _rmt_command (fildes, buf)
int fildes;
char *buf;
{
    int blen;
    SIGTYPE pstat;

    DBUG_ENTER ("_rmt_command");
    DBUG_PRINT ("rmtcommand", ("issue command '%s'", buf));

    /*
     *	save current pipe status and try to make the request
     */

    blen = strlen (buf);
    pstat = s_signal (SIGPIPE, SIG_IGN);
    if (write (WRITE (fildes), buf, (NBYTETYPE) blen) == blen) {
	(VOID) s_signal (SIGPIPE, pstat);
	DBUG_PRINT ("rmtcommand", ("return 0 for success"));
	DBUG_RETURN (0);
    }

    /*
     *	something went wrong. close down and go home
     */

    (VOID) s_signal (SIGPIPE, pstat);
    _rmt_abort (fildes);

    errno = EIO;
    DBUG_PRINT ("rmtcommand", ("return -1 for failure"));
    DBUG_RETURN (-1);
}

/*
 *	Create a file from scratch.  Looks just like creat(2) to the caller.
 */

int rmtcreat (path, mode)
char *path;
int mode;
{
    int rtnval;

    DBUG_ENTER ("rmtcreat");
    if (_rmt_dev (path)) {
	rtnval = rmtopen (path, 1 | O_CREAT, mode);
    } else {
	rtnval = creat (path, mode);
    }
    DBUG_RETURN (rtnval);
}

/*
 *	Test pathname to see if it is local or remote.  A remote device
 *	is any string that contains ":/dev/".  Returns 1 if remote,
 *	0 otherwise.
 */

int _rmt_dev (path)
char *path;
{
    DBUG_ENTER ("_rmt_dev");
    if ((path = strchr (path, ':')) != (char *) 0) {
	if (strncmp (path + 1, "/dev/", 5) == 0) {
	    DBUG_RETURN (1);
	}
    }
    DBUG_RETURN (0);
}

/*
 *	Duplicate an open file descriptor.  Looks just like dup(2)
 *	to caller.
 */

int rmtdup (fildes)
int fildes;
{
    int rtnval;

    DBUG_ENTER ("rmtdup");
    if (isrmt (fildes)) {
	errno = EOPNOTSUPP;
	rtnval = -1;		/* For now (fnf) */
    } else {
	rtnval = dup (fildes);
    }
    DBUG_RETURN (rtnval);
}

/*
 *	Rmtfcntl. Do a remote fcntl operation.
 */

#ifdef DEADCODE

int rmtfcntl (fd, cmd, arg)
int fd;
int cmd;
int arg;
{
    int rtnval;

    DBUG_ENTER ("rmtfcntl");
    if (isrmt (fd)) {
	errno = EOPNOTSUPP;
	rtnval = -1;
    } else {
	rtnval = fcntl (fd, cmd, arg);
    }
    DBUG_RETURN (rtnval);
}

#endif  /* DEADCODE */

/*
 *	Do ioctl on file.  Looks just like ioctl(2) to caller.
 */

int rmtioctl (fildes, request, arg)
int fildes;
int request;
IOCTL3 arg;
{
    int rtnval;

    DBUG_ENTER ("rmtioctl");
    if (isrmt (fildes)) {
#ifdef RMTIOCTL
	rtnval = _rmt_ioctl (fildes, request, arg);
#else
	errno = EOPNOTSUPP;
	rtnval = -1;		/* For now  (fnf) */
#endif
    } else {
	rtnval =  ioctl (fildes, request, arg);
    }
    DBUG_RETURN (rtnval);
}

/*
 *	_rmt_ioctl --- perform raw tape operations remotely
 */

#ifdef RMTIOCTL

static _rmt_ioctl (fildes, op, arg)
int fildes;
int op;
IOCTL3 arg;
{
    char c;
    int rc;
    int cnt;
    char buffer[BUFMAGIC];
    int rtnval;

    DBUG_ENTER ("_rmt_ioctl");

    /*
     *	MTIOCOP is the easy one. nothing is transfered in binary
     */

    if (op == MTIOCTOP) {
	(VOID) sprintf (buffer, "I%d\n%d\n", ((struct mtop *) arg) -> mt_op,
		 ((struct mtop  *) arg) -> mt_count);
	if (_rmt_command (fildes, buffer) == -1) {
	    DBUG_RETURN (-1);
	}
	rtnval = _rmt_status (fildes);
	DBUG_RETURN (rtnval);
    }

    /*
     *	we can only handle 2 ops, if not the other one, punt
     */

    if (op != MTIOCGET) {
	errno = EINVAL;
	DBUG_RETURN (-1);
    }

    /*
     *	grab the status and read it directly into the structure
     *	this assumes that the status buffer is (hopefully) not
     *	padded and that 2 shorts fit in a long without any word
     *	alignment problems, ie - the whole struct is contiguous
     *	NOTE - this is probably NOT a good assumption.
     */

    if (_rmt_command (fildes, "S\n") == -1 || (rc = _rmt_status (fildes)) == -1)
	DBUG_RETURN (-1);

    for (; rc > 0; rc -= cnt, arg += cnt) {
	cnt = read (READ (fildes), arg, rc);
	if (cnt <= 0) {
	    _rmt_abort (fildes);
	    errno = EIO;
	    DBUG_RETURN (-1);
	}
    }

    /*
     *	now we check for byte position. mt_type is a small integer field
     *	(normally) so we will check its magnitude. if it is larger than
     *	256, we will assume that the bytes are swapped and go through
     *	and reverse all the bytes
     */

    if (((struct mtget *) arg) -> mt_type < 256) {
	DBUG_RETURN (0);
    }
    for (cnt = 0; cnt < rc; cnt += 2) {
	c = arg[cnt];
	arg[cnt] = arg[cnt + 1];
	arg[cnt + 1] = c;
    }
    DBUG_RETURN (0);
}

#endif				/* RMTIOCTL */

/*
 *	Rmtisaatty.  Do the isatty function.
 */

#ifdef DEADCODE

int rmtisatty (fd)
int fd;
{
    int rtnval;

    DBUG_ENTER ("rmtisatty");
    if (isrmt (fd)) {
	rtnval = 0;
    } else {
	rtnval = isatty (fd);
    }
    DBUG_RETURN (rtnval);
}

#endif  /* DEADCODE */

/*
 *	_rmt_lseek --- perform an imitation lseek operation remotely
 */

static long _rmt_lseek (fildes, offset, whence)
int fildes;
long offset;
int whence;
{
    char buffer[BUFMAGIC];
    int rtnval;

    DBUG_ENTER ("_rmt_lseek");
    (VOID) sprintf (buffer, "L%d\n%d\n", offset, whence);
    if (_rmt_command (fildes, buffer) == -1) {
	rtnval = -1;
    } else {
	rtnval = _rmt_status (fildes);
    }
    DBUG_RETURN (rtnval);
}

/*
 *	Perform lseek on file.  Looks just like lseek(2) to caller.
 */

long rmtlseek (fildes, offset, whence)
int fildes;
long offset;
int whence;
{
    long rtnval;

    DBUG_ENTER ("rmtlseek");
    if (isrmt (fildes)) {
	rtnval = _rmt_lseek (fildes - REM_BIAS, offset, whence);
    } else {
	rtnval = lseek (fildes, offset, whence);
    }
    DBUG_RETURN (rtnval);
}

/*
 *	Open a local or remote file.  Looks just like open(2) to
 *	caller.
 */

int rmtopen (path, oflag, mode)
char *path;
int oflag;
int mode;
{
    int rtnval;

    DBUG_ENTER ("rmtopen");
    if (_rmt_dev (path)) {
	rtnval = _rmt_open (path, oflag, mode);
	if (rtnval != -1) {
	    rtnval += REM_BIAS;
	}
    } else {
	rtnval = open (path, oflag, mode);
    }
    DBUG_RETURN (rtnval);
}

/*
 *	_rmt_open --- open a magtape device on system specified, as given user
 *
 *	file name has the form system[.user]:/dev/????
 */

#define MAXHOSTLEN	257	/* BSD allows very long host names... */

/*ARGSUSED*/	/* Turn off lint checking for unused args in next func */

static int _rmt_open (path, oflag, mode)
char *path;
int oflag;
int mode;
{
    int i;
    int rc;
    char buffer[BUFMAGIC];
    char sysname[MAXHOSTLEN];
    char device[BUFMAGIC];
    char login[BUFMAGIC];
    char *sys;
    char *dev;
    char *user;

    DBUG_ENTER ("_rmt_open");
    sys = sysname;
    dev = device;
    user = login;

    /*
     *	first, find an open pair of file descriptors
     */

    for (i = 0; i < MAXUNIT; i++) {
	if (READ (i) == -1 && WRITE (i) == -1) {
	    break;
	}
    }
    if (i == MAXUNIT) {
	errno = EMFILE;
	DBUG_RETURN (-1);
    }

    /*
     *	pull apart system and device, and optional user
     *	don't munge original string
     */

    while (*path != '.' && *path != ':') {
	*sys++ = *path++;
    }
    *sys = '\0';
    path++;

    if (*(path - 1) == '.') {
	while (*path != ':') {
	    *user++ = *path++;
	}
	*user = '\0';
	path++;
    } else {
	*user = '\0';
    }

    while (*path) {
	*dev++ = *path++;
    }
    *dev = '\0';

    /*
     *	setup the pipes for the 'rsh' command and fork
     */

    if (pipe (_rmt_Ptc[i]) == -1 || pipe (_rmt_Ctp[i]) == -1) {
	DBUG_RETURN (-1);
    }

    if ((rc = fork ()) == -1) {
	DBUG_RETURN (-1);
    }

    if (rc == 0) {
	(VOID) close (0);
	(VOID) dup (_rmt_Ptc[i][0]);
	(VOID) close (_rmt_Ptc[i][0]);
	(VOID) close (_rmt_Ptc[i][1]);
	(VOID) close (1);
	(VOID) dup (_rmt_Ctp[i][1]);
	(VOID) close (_rmt_Ctp[i][0]);
	(VOID) close (_rmt_Ctp[i][1]);
	(VOID) setuid (getuid ());
	(VOID) setgid (getgid ());
	if (*login) {
	    (VOID) execl ("/usr/ucb/rsh", "rsh", sysname, "-l", login,
		    "/etc/rmt", (char *) 0);
	    (VOID) execl ("/usr/bsd/rsh", "rsh", sysname, "-l", login,
		    "/etc/rmt", (char *) 0);
	    (VOID) execl ("/usr/bin/remsh", "remsh", sysname, "-l", login,
		    "/etc/rmt", (char *) 0);
	    (VOID) execl ("/bin/remsh", "remsh", sysname, "-l", login,
		    "/etc/rmt", (char *) 0);
	    (VOID) execl ("/bin/rsh", "rsh", sysname, "-l", login,
		    "/etc/rmt", (char *) 0);
	} else {
	    (VOID) execl ("/usr/ucb/rsh", "rsh", sysname, "/etc/rmt", 
			  (char *) 0);
	    (VOID) execl ("/usr/bsd/rsh", "rsh", sysname, "/etc/rmt", 
			  (char *) 0);
	    (VOID) execl ("/usr/bin/remsh", "remsh", sysname, "/etc/rmt",
			  (char *) 0);
	    (VOID) execl ("/bin/remsh", "remsh", sysname, "/etc/rmt",
			  (char *) 0);
	    (VOID) execl ("/bin/rsh", "rsh", sysname, "/etc/rmt",
			  (char *) 0);
	}

	/*
	 *	bad problems if we get here
	 */

	(VOID) perror ("can't find remote shell program");
	(VOID) _exit (1);
    }

    (VOID) close (_rmt_Ptc[i][0]);
    (VOID) close (_rmt_Ctp[i][1]);

    /*
     *	now attempt to open the tape device
     */

    (VOID) sprintf (buffer, "O%s\n%d\n", device, oflag);
    if ((_rmt_command (i, buffer) == -1) || (_rmt_status (i) == -1)) {
	DBUG_PRINT ("rmtopen", ("return -1 for failure"));
	DBUG_RETURN (-1);
    }
    DBUG_PRINT ("rmtopen", ("return %d", i));
    DBUG_RETURN (i);
}

/*
 *	Read from stream.  Looks just like read(2) to caller.
 */

int rmtread (fildes, buf, nbyte)
int fildes;
char *buf;
unsigned int nbyte;
{
    int rtnval;

    DBUG_ENTER ("rmtread");
    if (isrmt (fildes)) {
	rtnval = _rmt_read (fildes - REM_BIAS, buf, nbyte);
    } else {
	rtnval = read (fildes, buf, (NBYTETYPE) nbyte);
    }
    DBUG_RETURN (rtnval);
}

/*
 *	_rmt_read --- read a buffer from a remote tape
 */

static int _rmt_read (fildes, buf, nbyte)
int fildes;
char *buf;
unsigned int nbyte;
{
    int rc;
    int i;
    int bytesread;
    char buffer[BUFMAGIC];

    DBUG_ENTER ("_rmt_read");
    (VOID) sprintf (buffer, "R%d\n", nbyte);
    if (_rmt_command (fildes, buffer) == -1 || (rc = _rmt_status (fildes)) == -1) {
	DBUG_RETURN (-1);
    }
    for (i = 0; i < rc; i += bytesread, buf += bytesread) {
	bytesread = read (READ (fildes), buf, (NBYTETYPE) rc);
	if (bytesread <= 0) {
	    _rmt_abort (fildes);
	    errno = EIO;
	    DBUG_RETURN (-1);
	}
    }

    DBUG_RETURN (rc);
}

/*
 *	_rmt_status --- retrieve the status from the pipe
 */

int _rmt_status (fildes)
int fildes;
{
    int i;
    char c;
    char *cp;
    char buffer[BUFMAGIC];

    DBUG_ENTER ("_rmt_status");

    /*
     *	read the reply command line
     */

    for (i = 0, cp = buffer; i < BUFMAGIC; i++, cp++) {
	if (read (READ (fildes), cp, 1) != 1) {
	    _rmt_abort (fildes);
	    errno = EIO;
	    DBUG_PRINT ("rmtstatus", ("return -1 for failure"));
	    DBUG_RETURN (-1);
	}
	if (*cp == '\n') {
	    *cp = 0;
	    break;
	}
    }
    DBUG_PRINT ("rmtstatus", ("got response '%s'", buffer));

    if (i == BUFMAGIC) {
	_rmt_abort (fildes);
	errno = EIO;
	DBUG_PRINT ("rmtstatus", ("return -1 for failure"));
	DBUG_RETURN (-1);
    }

    /*
     *	check the return status
     */

    for (cp = buffer; *cp; cp++) {
	if (*cp != ' ') {
	    break;
	}
    }

    if (*cp == 'E' || *cp == 'F') {
	errno = atoi (cp + 1);
	DBUG_PRINT ("rmtstatus", ("errno set to %d", errno));
	while (read (READ (fildes), &c, 1) == 1) {
	    if (c == '\n') {
		break;
	    }
	}

	if (*cp == 'F') {
	    _rmt_abort (fildes);
	}

	DBUG_PRINT ("rmtstatus", ("return -1 for failure"));
	DBUG_RETURN (-1);
    }

    /*
     *	check for mis-synced pipes
     */

    if (*cp != 'A') {
	_rmt_abort (fildes);
	errno = EIO;
	DBUG_PRINT ("rmtstatus", ("return -1 for failure"));
	DBUG_RETURN (-1);
    }

    i = atoi (cp + 1);
    DBUG_PRINT ("rmtstatus", ("return %d", i));
    DBUG_RETURN (i);
}

/*
 *	Write to stream.  Looks just like write(2) to caller.
 */

int rmtwrite (fildes, buf, nbyte)
int fildes;
char *buf;
unsigned int nbyte;
{
    int rtnval;

    DBUG_ENTER ("rmtwrite");
    if (isrmt (fildes)) {
	rtnval = _rmt_write (fildes - REM_BIAS, buf, nbyte);
    } else {
	rtnval = write (fildes, buf, (NBYTETYPE) nbyte);
    }
    DBUG_RETURN (rtnval);
}

/*
 *	_rmt_write --- write a buffer to the remote tape
 */

static int _rmt_write (fildes, buf, nbyte)
int fildes;
char *buf;
unsigned int nbyte;
{
    char buffer[BUFMAGIC];
    SIGTYPE pstat;
    int rtnval;

    DBUG_ENTER ("_rmt_write");
    (VOID) sprintf (buffer, "W%d\n", nbyte);
    if (_rmt_command (fildes, buffer) == -1) {
	DBUG_RETURN (-1);
    }

    pstat = s_signal (SIGPIPE, SIG_IGN);
    if (write (WRITE (fildes), buf, (NBYTETYPE) nbyte) == nbyte) {
	(VOID) s_signal (SIGPIPE, pstat);
	rtnval = _rmt_status (fildes);
	DBUG_RETURN (rtnval);
    }

    (VOID) s_signal (SIGPIPE, pstat);
    _rmt_abort (fildes);
    errno = EIO;
    DBUG_RETURN (-1);
}

#else

/*
 *	Stub out the isrmt to always return false.
 */

#if (unix || xenix)

/*ARGSUSED*/
int isrmt (fd)
int fd;
{
    int rtnval;

    DBUG_ENTER ("isrmt");
    rtnval = 0;
    DBUG_RETURN (rtnval);
}

#endif				/* unix || xenix */

#if defined(__STDC__)
static int dummy;	/* avoid "empty translation unit" warnings */
#endif

#endif				/* RMT */
