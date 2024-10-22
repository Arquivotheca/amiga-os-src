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
 *  FILE
 *
 *	usermsg.c    support routines for user messages
 *
 *  SCCS
 *
 *	@(#)usermsg.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	This module contains the first layer of interface routines
 *	to send informational messages to the user.
 *
 *	The following classes of messages, directed towards the user,
 *	are defined:
 *
 *	(1)	Error messages.   These messages are informational
 *		one way messages to the user, and require no direct
 *		response.  They are used to report errors, which
 *		may or may not be fatal.
 *
 *	(2)	Warning messages.   These messages are informational
 *		one way messages to the user, and require no direct
 *		response.  They are used to report information which
 *		is important for the user to know, but which is not
 *		serious enough to classify as an error.
 *
 *	(3)	Verbosity messages.  These messages are informational
 *		one way messages to the user, and require no direct
 *		response.  They are used to report information about
 *		the archive, current processing status, etc.
 *
 *	(4)	Query messages.  These messages are bidirectional
 *		messages that send information to the user and require
 *		a response of some sort.
 *
 *	(5)	Alert messages.  These messages are informational one way
 *		messages to the user, of about the same importance as a
 *		query message, but no reply is required.
 *
 */

#include "globals.h"

/*
 *	Define structure for organizing information about each
 *	possible user message.  Each message has a unique message
 *	number.
 */

struct usermsg {
    int msg_no;			/* The internal message number */
    int msg_flags;		/* Some control flags */
    int msg_type;		/* Type of message (error, warning, etc) */
    char *msg_shortfmt;		/* Short form args format */
    char *msg_longfmt;		/* Format string for message */
};

#define TYPE_ERROR	1	/* An error message */
#define TYPE_WARNING	2	/* A warning message */
#define TYPE_QUERY	3	/* A query message (requires reply) */
#define TYPE_ALERT	4	/* A alert message (like query, no reply) */
#define TYPE_VERBOSITY	5	/* A verbosity message (status, info, etc) */

#define MSG_ERRNO	0x01	/* A system error message available */
#define MSG_ID		0x02	/* Include id string */

static struct usermsg messages[] = {
    { MSG_MODE, MSG_ID, TYPE_ERROR, "",
	  "specify mode (-cdeghitx)" },
    { MSG_AROPEN, MSG_ID | MSG_ERRNO, TYPE_ERROR, "%s",
	  "\"%s\": can't open archive" },
    { MSG_ARCLOSE, MSG_ID | MSG_ERRNO, TYPE_WARNING, "%s", 
	  "\"%s\": warning - close error on archive" },
    { MSG_ARREAD, MSG_ID | MSG_ERRNO, TYPE_WARNING, "%ld",
	  "warning - archive read error at block %ld" },
    { MSG_ARWRITE, MSG_ID | MSG_ERRNO, TYPE_WARNING, "%ld",
	  "warning - archive write error at block %ld" },
    { MSG_ARSEEK, MSG_ID | MSG_ERRNO, TYPE_ERROR, "", 
	  "seek error on archive" },
    { MSG_BUFSZ, MSG_ID, TYPE_ERROR, "", 
	  "media size smaller than I/O buffer size!" },
    { MSG_MAXBUFSZ, MSG_ID, TYPE_WARNING, "%dKb %dKb", 
	  "warning - buffer size (%dKb) exceeds maximum (%dKb) allowed for device" },
    { MSG_BALLOC, MSG_ID | MSG_ERRNO, TYPE_ERROR, "%dKb", 
	  "can't allocate %dKb archive buffer" },
    { MSG_BSEQ, MSG_ID, TYPE_WARNING, "%s", 
	  "\"%s\": warning - block sequence error" },
    { MSG_DSYNC, MSG_ID, TYPE_WARNING, "", 
	  "warning - file synchronization error; attempting recovery ..." },
    { MSG_EACCESS, MSG_ID | MSG_ERRNO, TYPE_ERROR, "%s", 
	  "\"%s\": no file" },
    { MSG_STATFAIL, MSG_ID | MSG_ERRNO, TYPE_ERROR, "%s", 
	  "\"%s\": can't stat" },
    { MSG_BIGPATH, MSG_ID, TYPE_ERROR, "%s %d", 
	  "pathname \"%s\" too big (%d max)" },
    { MSG_BIGFAC, MSG_ID, TYPE_ERROR, "%d", 
	  "blocking factor %d too big" },
    { MSG_OPEN, MSG_ID | MSG_ERRNO, TYPE_ERROR, "%s", 
	  "\"%s\": can't open" },
    { MSG_CLOSE, MSG_ID | MSG_ERRNO, TYPE_WARNING, "%s", 
	  "\"%s\": warning - file close error" },
    { MSG_READ, MSG_ID | MSG_ERRNO, TYPE_ERROR, "%s", 
	  "\"%s\": read error" },
    { MSG_FTRUNC, MSG_ID, TYPE_WARNING, "%s", 
	  "\"%s\": warning - file was truncated" },
    { MSG_FGREW, MSG_ID, TYPE_WARNING, "%s", 
	  "\"%s\": warning - file grew while archiving" },
    { MSG_SUID, MSG_ID, TYPE_WARNING, "%s", 
	  "\"%s\": warning - can't set user id: Not owner" },
    { MSG_SGID, MSG_ID, TYPE_WARNING, "%s", 
	  "\"%s\": warning - can't set group id: Permission denied" },
    { MSG_EXEC, MSG_ID | MSG_ERRNO, TYPE_ERROR, "%s", 
	  "\"%s\": can't exec" },
    { MSG_FORK, MSG_ID | MSG_ERRNO, TYPE_ERROR, "", 
	  "can't fork, try again" },
    { MSG_BADWAIT, MSG_ID, TYPE_ERROR, "%d", 
	  "unrecognized wait return %d" },
    { MSG_EINTR, MSG_ID | MSG_ERRNO, TYPE_ERROR, "", 
	  "child interrupted" },
    { MSG_CSTOP, MSG_ID, TYPE_ERROR, "%s %d", 
	  "\"%s\": fatal error; stopped by signal %d" },
    { MSG_CTERM, MSG_ID, TYPE_ERROR, "%s %d", 
	  "\"%s\": fatal error; terminated by signal %d" },
    { MSG_CORE, MSG_ID, TYPE_WARNING, "%s", 
	  "\"%s\" core dumped" },
    { MSG_WSTATUS, MSG_ID, TYPE_ERROR, "%o", 
	  "inconsistent wait status %o" },
    { MSG_SETUID, MSG_ID | MSG_ERRNO, TYPE_ERROR, "%d", 
	  "can't set uid to %d" },
    { MSG_SETGID, MSG_ID | MSG_ERRNO, TYPE_ERROR, "%d", 
	  "can't set gid to %d" },
    { MSG_SUM, MSG_ID, TYPE_WARNING, "%s %ld", 
	  "\"%s\": warning - %ld block checksum errors" },
    { MSG_BUG, MSG_ID, TYPE_ERROR, "%s", 
	  "internal bug in routine \"%s\"" },
    { MSG_MALLOC, MSG_ID | MSG_ERRNO, TYPE_ERROR, "%u", 
	  "can't allocate %u more bytes" },
    { MSG_WALK, MSG_ID, TYPE_ERROR, "", 
	  "internal error in tree; pathname overflow" },
    { MSG_DEPTH, MSG_ID, TYPE_ERROR, "%s/%s", 
	  "pathname too big, lost \"%s%s\"" },
    { MSG_SEEK, MSG_ID | MSG_ERRNO, TYPE_ERROR, "%s", 
	  "\"%s\": seek error" },
    { MSG_ISUM, MSG_ID, TYPE_WARNING, "", 
	  "warning - info block checksum error" },
    { MSG_WRITE, MSG_ID | MSG_ERRNO, TYPE_ERROR, "%s", 
	  "\"%s\": write error" },
    { MSG_SMODE, MSG_ID | MSG_ERRNO, TYPE_WARNING, "%s", 
	  "\"%s\": warning - error setting mode" },
    { MSG_CHOWN, MSG_ID | MSG_ERRNO, TYPE_WARNING, "%s", 
	  "\"%s\": warning - error setting owner/group" },
    { MSG_STIMES, MSG_ID | MSG_ERRNO, TYPE_WARNING, "%s", 
	  "\"%s\": warning - error setting times" },
    { MSG_MKNOD, MSG_ID | MSG_ERRNO, TYPE_ERROR, "%s", 
	  "\"%s\": error making node" },
    { MSG_MKLINK, MSG_ID | MSG_ERRNO, TYPE_ERROR, "%s %s", 
	  "\"%s\": can't link to \"%s\"" },
    { MSG_ARPASS, MSG_ID, TYPE_ERROR, "", 
	  "internal error; inconsistent phys blk addrs" },
    { MSG_IMAGIC, MSG_ID, TYPE_WARNING, "%d", 
	  "warning - missing archive header block; starting at volume %d" },
    { MSG_LALLOC, MSG_ID | MSG_ERRNO, TYPE_WARNING, "%s", 
	  "\"%s\": warning - lost linkage" },
    { MSG_URLINKS, MSG_ID, TYPE_WARNING, "%s %d", 
	  "\"%s\": warning - %d unresolved link(s)" },
    { MSG_TTYOPEN, MSG_ID | MSG_ERRNO, TYPE_WARNING, "%s", 
	  "\"%s\": warning - can't open for interaction" },
    { MSG_NTIME, MSG_ID, TYPE_ERROR, "%s", 
	  "date conversion error: \"%s\"" },
    { MSG_UNAME, MSG_ID | MSG_ERRNO, TYPE_WARNING, "", 
	  "warning - uname failed" },
    { MSG_LABEL, MSG_ID, TYPE_WARNING, "%s", 
	  "warning - label \"%s\" too big" },
    { MSG_GUID, MSG_ID, TYPE_ERROR, "%s", 
	  "uid conversion error: \"%s\"" },
    { MSG_CCLASS, MSG_ID, TYPE_ERROR, "", 
	  "unterminated character class" },
    { MSG_OVRWRT, MSG_ID | MSG_ERRNO, TYPE_ERROR, "%s", 
	  "\"%s\": can't overwrite" },
    { MSG_WACCESS, MSG_ID | MSG_ERRNO, TYPE_ERROR, "%s", 
	  "\"%s\": can't access for write" },
    { MSG_RACCESS, MSG_ID | MSG_ERRNO, TYPE_ERROR, "%s", 
	  "\"%s\": can't access for read" },
    { MSG_STDIN, MSG_ID, TYPE_ERROR, "", 
	  "can't read both file list and archive from stdin!" },
    { MSG_PEOV, MSG_ID, TYPE_ALERT, "%d",
	  "warning - premature end of volume %d" },
    { MSG_UNFMT, MSG_ID, TYPE_ALERT, "",
	  "warning - media appears to be unformatted" },
    { MSG_COPEN, MSG_ID | MSG_ERRNO, TYPE_WARNING, "%s", 
	  "\"%s\": warning - will not be contiguous" },
    { MSG_CNOSUP, MSG_ID, TYPE_WARNING, "%s", 
	  "\"%s\": warning - contiguous files not supported, extracted as a regular file" },
    { MSG_BRUTAB, MSG_ID, TYPE_WARNING, "", 
	  "warning - using internal default device table" },
    { MSG_SUPERSEDE, MSG_ID, TYPE_WARNING, "%s", 
	  "\"%s\": warning - not superseded" },
    { MSG_WRTPROT, MSG_ID, TYPE_ALERT, "",
	  "warning - media appears to be write protected or unmounted" },
    { MSG_IGNORED, MSG_ID, TYPE_WARNING, "%s", 
	  "\"%s\": warning - not found or not selected" },
    { MSG_FASTMODE, MSG_ID, TYPE_WARNING, "", 
	  "warning - may have to use -F option to read archive" },
    { MSG_BACKGND, MSG_ID, TYPE_ERROR, "", 
	  "interaction needed, aborted by -B option" },
    { MSG_MKDIR, MSG_ID | MSG_ERRNO, TYPE_ERROR, "%s", 
	  "\"%s\": error making directory" },
    { MSG_RDLINK, MSG_ID | MSG_ERRNO, TYPE_ERROR, "%s", 
	  "\"%s\": error reading symbolic link" },
    { MSG_NOSYMLINKS, MSG_ID, TYPE_ERROR, "%s", 
	  "\"%s\": symbolic links not supported" },
    { MSG_MKSYMLINK, MSG_ID | MSG_ERRNO, TYPE_ERROR, "%s", 
	  "\"%s\": could not make symbolic link" },
    { MSG_MKFIFO, MSG_ID, TYPE_ERROR, "%s", 
	  "\"%s\": could not make fifo" },
    { MSG_SYMTODIR, MSG_ID, TYPE_WARNING, "%s %s %s", 
	  "warning - link of \"%s\" to \"%s\", \"%s\" is a directory, no link made" },
    { MSG_HARDLINK, MSG_ID, TYPE_WARNING, "%s %s %s", 
	  "warning - link of \"%s\" to \"%s\", \"%s\" does not exist" },
    { MSG_FIFOTOREG, MSG_ID, TYPE_WARNING, "%s", 
	  "warning - extracted fifo \"%s\" as a regular file" },
    { MSG_ALINKS, MSG_ID, TYPE_WARNING, "%s %d", 
	  "\"%s\": warning - %d additional link(s) added while archiving" },
    { MSG_OBTF, MSG_ID, TYPE_WARNING, "%s %d", 
	  "\"%s\": warning - line %d, obsolete brutab format" },
    { MSG_DEFDEV, MSG_ID, TYPE_ERROR, "", 
	  "no default device in brutab file, use -f option" },
    { MSG_NOBTF, MSG_ID, TYPE_ERROR, "", 
	  "support for obsolete brutab format not compiled in" },
    { MSG_BSZCHG, MSG_ID, TYPE_WARNING, "%u %u", 
	  "warning - attempt to change buffer size from %u to %u ignored (incompatible brutab entries)" },
    { MSG_DBLIO, MSG_ID | MSG_ERRNO, TYPE_ERROR, "%d %d", 
	  "double buffering I/O error, %d bytes read/written, errno %d" },
    { MSG_DBLSUP, MSG_ID, TYPE_ERROR, "", 
	  "problem setting up double buffering, using normal buffering" },
    { MSG_EJECT, MSG_ID | MSG_ERRNO, TYPE_ERROR, "%s", 
	  "\"%s\": media ejection failed" },
    { MSG_NOSHRINK, MSG_ID, TYPE_WARNING, "%s", 
	  "\"%s\": warning - compressed version was larger, stored uncompressed" },
    { MSG_ZXFAIL, MSG_ID, TYPE_WARNING, "%s", 
	  "\"%s\": warning - decompression failed, not extracted" },
    { MSG_NOEZ, MSG_ID, TYPE_WARNING, "", 
	  "warning - estimate mode ignores compression" },
    { MSG_UNLINK, MSG_ID | MSG_ERRNO, TYPE_WARNING, "%s", 
	  "\"%s\": warning - not deleted" },
    { MSG_ZFAILED, MSG_ID, TYPE_WARNING, "%s", 
	  "\"%s\": warning - compression failed, stored uncompressed" },
    { MSG_BADBITS, MSG_ID, TYPE_ERROR, "%d %d", 
	  "bad number of bits (%d) for compression (max %d)" },
    { MSG_SHMSIZE, MSG_ID, TYPE_WARNING, "%d %d", 
	  "warning - buffer size (%dKb) exceeds system imposed limit (%dKb) with double buffering" },
    { MSG_BUFADJ, MSG_ID, TYPE_WARNING, "%d", 
	  "warning - buffer size automatically adjusted to %dKb" },
    { MSG_SHMGET, MSG_ID | MSG_ERRNO, TYPE_ERROR, "%d", 
	  "could not get shared memory segment (%dKb)" },
    { MSG_SHMAT, MSG_ID | MSG_ERRNO, TYPE_ERROR, "", 
	  "could not attach shared memory segment" },
    { MSG_MSGGET, MSG_ID | MSG_ERRNO, TYPE_ERROR, "", 
	  "could not allocate message queue" },
    { MSG_IOPT, MSG_ID, TYPE_WARNING, "%s", 
	  "warning - don't understand -I option '%s'" },
    { MSG_NSEGS, MSG_ID, TYPE_WARNING, "%d", 
	  "warning - need more than %d shared memory segments" },
    { MSG_SBRK, MSG_ID | MSG_ERRNO, TYPE_WARNING, "%d", 
	  "warning - failed to move break value by %d bytes" },
    { MSG_NOZ, MSG_ID, TYPE_WARNING, "", 
	  "warning - compression initialization failed, -Z suppressed" },
    { MSG_CLDUNK, MSG_ID, TYPE_WARNING, "%d %d %x", 
	  "warning - unknown child died, pid %d (expected %d), status %#x" },
    { MSG_SLVDIED, MSG_ID, TYPE_ERROR, "%x", 
	  "double buffer child died, status %#x" },
    { MSG_SLVERR, MSG_ID, TYPE_WARNING, "%d", 
	  "warning - double buffer child error %d" },
    { MSG_REAP, MSG_ID | MSG_ERRNO, TYPE_WARNING, "", 
	  "warning - no double buffer child to reap" },
    { MSG_SHMCOPY, MSG_ID, TYPE_WARNING, "", 
	  "warning - archive device may need \"shmcopy\" flag set in brutab entry" },
    { MSG_UWERR, MSG_ID, TYPE_WARNING, "%s", 
	  "\"%s\": warning - unrecoverable archive write error, some data lost" },
    { MSG_UFORWP, MSG_ID, TYPE_ALERT, "",
	  "warning - media appears to be unformatted or write protected" },
    { MSG_AEOV, MSG_ID, TYPE_ALERT, "%d",
	  "warning - assuming end of volume %d (unknown size)" },
    { MSG_WRONGVOL, MSG_ID, TYPE_ALERT, "%d %d",
	  "warning - found volume %d, expecting %d" },
    { MSG_WRONGTIME, MSG_ID, TYPE_ALERT, "%s",
	  "warning - volume not part of archive created %s" },
    { MSG_DATAOVW, MSG_ID, TYPE_ALERT, "%s",
	  "warning - all data currently on \"%s\" will be destroyed" },
    { MSG_NEWPASS, MSG_ID, TYPE_ALERT, "%s",
	  "ready for %s pass" },
    { MSG_TTYNL, 0, TYPE_ALERT, "",
	  "" },
    { MSG_VERBOSITY, 0, TYPE_VERBOSITY, "%s",
	  "%s" },
    { MSG_NOREWIND, MSG_ID, TYPE_ALERT, "",
	  "don't know how to rewind archive device" },
    { MSG_RERUN, MSG_ID, TYPE_ALERT, "%u",
	  "rerun with \"-b %uk\" argument" },
    { MSG_CONFIRM, 0, TYPE_QUERY, "%s %s",
	  "%s %s: please confirm [y/n/g] " },
    { MSG_ACTION, MSG_ID, TYPE_QUERY, "%s %c",
	  "%s [default: %c] >> " },
    { MSG_LOADVOL, MSG_ID, TYPE_QUERY, "%d %s %c",
	  "load volume %d and enter device [default: %s] >> %c" },
    { MSG_TOOLARGE, MSG_ID, TYPE_WARNING, "%s",
	  "\"%s\": warning - too large under current ulimit, not extracted" },
    { MSG_ULIMITSET, MSG_ID | MSG_ERRNO, TYPE_ERROR, "%ld",
	  "ulimit call failed to set maximum file size limit to %ld blocks" },
    { MSG_NODBLBUF, MSG_ID, TYPE_WARNING, "%s",
	  "warning - no double buffering support included in this version" },
    { MSG_DBLBUFOFF, MSG_ID, TYPE_WARNING, "%s",
	  "warning - shared memory does not appear to be working in your kernel" },
    { MSG_MSGSND, MSG_ID | MSG_ERRNO, TYPE_ERROR, "",
	  "problem sending message to other process" },
    { MSG_MSGRCV, MSG_ID | MSG_ERRNO, TYPE_ERROR, "",
	  "problem receiving message from other process" },
    { MSG_FDATACHG, MSG_ID, TYPE_WARNING, "%s", 
	  "\"%s\": warning - file contents changed while archiving" },
    { MSG_STACK, MSG_ID, TYPE_WARNING, "%ld",
	  "warning - stack size of %ld is less than recommended minimum of %ld" },
    { MSG_FBIOERR, MSG_ID | MSG_ERRNO, TYPE_ALERT, "",
	  "warning - unrecoverable error on first block" },
    { MSG_IGNCLOSE, MSG_ID, TYPE_WARNING, "", 
	  "warning - archive device may need \"ignoreclose\" flag set in brutab entry" },
    { MSG_ADJMSIZE, MSG_ID, TYPE_WARNING, "%ld", 
	  "warning - media size automatically adjusted to %luKb" },
    { MSG_NOENTRY, MSG_ID, TYPE_WARNING, "%s %s",
	  "warning - no entry for device \"%s\" in \"%s\"" },
    { MSG_SELFCHK, MSG_ID, TYPE_ERROR, "",
	  "internal error - failed self consistency and portability checks" },
    { MSG_BRUPATHMAX, MSG_ID, TYPE_ERROR, "%s %d",
	  "path beginning with \"%s\" too large (%d characters max)" },
    { MSG_LBROKEN, MSG_ID, TYPE_WARNING, "%s %s", 
	  "\"%s\": warning - link to \"%s\" broken, saved as duplicate" },
    { MSG_WAITFAIL, MSG_ID | MSG_ERRNO, TYPE_WARNING, "",
	  "warning - wait failed" },
    { 0, 0, 0, NULL,
	  NULL }
};


/*
 *  FUNCTION
 *
 *	bru_message    print appropriate message for given error
 *
 *  SYNOPSIS
 *
 *	VOID bru_message (msgno, va_alist)
 *	int msgno;
 *	va_dcl
 *
 *  DESCRIPTION
 *
 *	Handles a user message uniquely specified by "msgno".
 *	Note that values for "msgno" are given in "usermsg.h".
 *
 *	Note that sys_errlist[] and sys_nerr are supplied by the
 *	runtime environment in most standard Unix systems.  Other
 *	implementations may require this module to be customized.
 *
 */

/*VARARGS1*/
VOID bru_message (VA_ARG(int,msgno), VA_ALIST)
VA_OARGS (int msgno;)
VA_DCL
{
    struct usermsg *msgp;
    char *sp;
    int msgtype;
    va_list args;
    char prtbuf[3 * BRUPATHMAX];
    
    DBUG_ENTER ("bru_message");
    DBUG_PRINT ("msgno", ("send user message %d", msgno));
    VA_START (args, msgno);
    sp = prtbuf;
    for (msgp = messages; msgp -> msg_no != 0 && msgp -> msg_no != msgno; msgp++) {;}
    msgtype = msgp -> msg_type;
    if ((msgp -> msg_no == 0) || (msgp -> msg_flags & MSG_ID)) {
	if (!flags.Afflag) {
	    (VOID) s_sprintf (sp, "%s: ", info.bru_name);
	    sp += s_strlen (sp);
	}
    }
    if (msgp -> msg_no != msgno) {
	(VOID) s_sprintf (sp, "warning - unknown internal error %d!", msgno);
	sp += s_strlen (sp);
	einfo.errors++;
    } else {
	if (msgtype == TYPE_ERROR) {
	    einfo.errors++;
	} else if (msgtype == TYPE_WARNING) {
	    einfo.warnings++;
	}
	if (flags.Afflag) {
	    (VOID) s_sprintf (sp, "message %d: ", msgp -> msg_no);
	    sp += s_strlen (sp);
	    (VOID) s_vsprintf (sp, msgp -> msg_shortfmt, args);
	} else {
	    (VOID) s_vsprintf (sp, msgp -> msg_longfmt, args);
	}
	sp += s_strlen (sp);
	if ((errno > 0) && (msgp -> msg_flags & MSG_ERRNO)) {
	    if (errno > sys_nerr) {
		(VOID) s_sprintf (sp, ": !! bad errno %d !!", errno);
	    } else {
		(VOID) s_sprintf (sp, ": %s", sys_errlist[errno]);
	    }
	    sp += s_strlen (sp);
	}
    }
    if (flags.Afflag) {
	switch (msgtype) {
	    case TYPE_QUERY:
	        (VOID) ipc_query (prtbuf);
		break;
	    case TYPE_ERROR:
	    case TYPE_WARNING:
		(VOID) ipc_error (prtbuf);
		break;
	    case TYPE_VERBOSITY:
		break;
	    case TYPE_ALERT:
		(VOID) ipc_error (prtbuf);
		break;
	}
    } else if (msgtype == TYPE_ALERT) {
	tty_printf ("%s\n", prtbuf);
	tty_flush ();
    } else if (msgtype == TYPE_QUERY) {
	tty_printf ("%s", prtbuf);
	tty_flush ();
    } else if (msgtype == TYPE_VERBOSITY) {
	(VOID) s_fprintf (logfp, "%s\n", prtbuf);
	s_fflush (logfp);
    } else {
	(VOID) s_fprintf (errfp, "%s\n", prtbuf);
	s_fflush (errfp);
    }
    va_end (args);
    DBUG_VOID_RETURN;
}



