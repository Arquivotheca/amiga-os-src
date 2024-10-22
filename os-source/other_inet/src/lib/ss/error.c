/* -----------------------------------------------------------------------
 * error.c
 *
 * $Locker:  $
 *
 * $Id: error.c,v 1.3 92/07/21 16:57:56 bj Exp $
 *
 * $Revision: 1.3 $
 *
 * $Log:	error.c,v $
 * Revision 1.3  92/07/21  16:57:56  bj
 * Socket.library 4.0
 * changed #include
 * 
 * Revision 1.2  91/08/07  14:00:51  bj
 * rcs header added.
 * 
 *
 * $Header: AS225:src/lib/ss/RCS/error.c,v 1.3 92/07/21 16:57:56 bj Exp $
 *
 *------------------------------------------------------------------------
 */

/****** socket.library/strerror  *************************************************
*
*   NAME
*	strerror - Returns a pointer to an error message.
*
*   SYNOPSIS
*	#include <string.h>
*
*	error = strerror( error_number )
*	D0                D1
*
*	char *strerror( int );
*
*   FUNCTION
*	The strerror() function maps the error number to a language-
*	dependent Unix error message.
*
*   INPUTS
*	error_number 	- usually the value of errno.
*
*   RESULT
*	error		- pointer to the error message.
*
*   NOTES
*	This function should eventually be localized.
*
*   SEE ALSO
*	<errno.h>,  perror()
*
********************************************************************
*
*
*/

#include <exec/types.h>
#include <errno.h>

char * __saveds __asm strerror(register __d1 int num)
{
	if (num < 0 || num >= sys_nerr)
		return sys_errlist[EINVAL];

	return sys_errlist[num];
}


char *  sys_errlist[] = {
/*  0 */	"Error code not set",
/*  1 */	"Permission to access object is denied",
/*  2 */	"No such file, directory, or volume",
/*  3 */	"No such process",
/*  4 */	"System call was interrupted",
/*  5 */	"Input/output error",
/*  6 */	"No such device",
/*  7 */	"Argument was too big for operation",
/*  8 */	"Execution error when starting process",
/*  9 */	"Bad or incorrect file handle",
/* 10 */	"Child process error",
/* 11 */	"No more processes allowed",
/* 12 */	"Out of memory",
/* 13 */	"Access to object is denied",
/* 14 */	"Bad address",
/* 15 */	"Block or bulk device required",
/* 16 */	"File, directory, or volume is in use",
/* 17 */	"Object already exists",
/* 18 */	"Cross device link",
/* 19 */	"No such device",
/* 20 */	"Operation required directory, supplied argument was not",
/* 21 */	"Operation needs non-directory, supplied argument was directory",
/* 22 */	"Invalid argument supplied",
/* 23 */	"No more files allowed",
/* 24 */	"No more files allowed for this process",
/* 25 */	"Not a terminal",
/* 26 */	"Text file was in use",
/* 27 */	"File too large",
/* 28 */	"No space left on device",
/* 29 */	"Object does not support seek operation",
/* 30 */	"Filesystem is read only",
/* 31 */	"Too many links",
/* 32 */	"Broken connection",
/* 33 */	"Domain error",
/* 34 */	"Range error",
/* 35 */	"Operation would have blocked",
/* 36 */	"Operation now in progress",
/* 37 */	"Operation already in progress",
/* 38 */	"Argument not a socket",
/* 39 */	"Destination address is required",
/* 40 */	"Message was of inappropriate size",
/* 41 */	"Protocol wrong type for socket",
/* 42 */	"Protocol doesn't support option",
/* 43 */	"Protocol not supported",
/* 44 */	"Socket type not supported",
/* 45 */	"Operation not supported on socket",
/* 46 */	"Protocol family not supported",
/* 47 */	"Address family not supported",
/* 48 */	"Requested address already in use",
/* 49 */	"Requested address not available",
/* 50 */	"Network software is down or unconfigured",
/* 51 */	"Network is unreachable",
/* 52 */	"Network was reset",
/* 53 */	"Connection aborted",
/* 54 */	"Connection reset",
/* 55 */	"Network software out of memory buffers",
/* 56 */	"Already connected",
/* 57 */	"Not connected",
/* 58 */	"Socket was shutdown",
/* 59 */	"Too many references",
/* 60 */	"Connection timed out",
/* 61 */	"Connection request was refused",
/* 62 */	"Too many levels of symbolic links",
/* 63 */	"File name too long",
/* 64 */	"Host is down",
/* 65 */	"Host is unreachable",
/* 66 */	"Directory not empty",
/* 67 */	"Too many processes",
/* 68 */	"Too many users",
/* 69 */	"Disk quota exceeded",
/* 70 */	"Stale NFS file handle",
/* 71 */	"Too many levels of remote in path",
/* 72 */	"Device is not a stream",
/* 73 */	"Timer expired",
/* 74 */	"Out of stream resources",
/* 75 */	"No message of desired type",
/* 76 */	"Trying to read unreadable message",
/* 77 */	"Identifier removed",
/* 78 */	"Deadlock condition",
/* 79 */	"No record locks available",
/* 80 */	"Bad volume name",
/* 81 */	"Software configuration problem"
};

int sys_nerr =  sizeof(sys_errlist) / sizeof(sys_errlist[0]);
