/*
 *  FILE
 *
 *	rmt.h    redefine the system calls for the remote mag tape library
 *
 *  SCCS
 *
 *	@(#)rmt.h	1.3	9/20/85
 *
 *  SYNOPSIS
 *
 *	#ifdef RMT
 *	#include <rmt.h>
 *	#endif
 *
 *  DESCRIPTION
 *
 *	This file makes the use of remote tape drives transparent
 *	to the program that includes it.  A remote tape drive has
 *	the form system[.user]:/dev/???.
 *
 *	Note that the standard system calls are simply remapped to
 *	the remote mag tape library support routines.
 *
 *	Also, note that if <sys/stat.h> is included, it must be AFTER
 *	this file to get the stat structure name remapped.
 *
 *	There are no declarations of the remapped functions (rmt<name>)
 *	because the original declarations elsewhere will be remapped also.
 *	("extern int access()" will become "extern int rmtaccess()")
 *
 *  NOTE
 *
 *	This file is derived from a public domain implementation
 *	of a remote mag tape support library.  It is not subject to
 *	any of the restrictions, with respect to copyright or licensing,
 *	of any of the rest of the bru source code.
 *
 */

#ifndef access			/* Avoid multiple redefinition */

#ifndef RMT_RUNTIME		/* Only do remapping in client routines */
#define access rmtaccess
#define close rmtclose
#define creat rmtcreat
#define dup rmtdup
#define fcntl rmtfcntl
#define ioctl rmtioctl
#define isatty rmtisatty
#define lseek rmtlseek
#define open rmtopen
#define read rmtread
#define write rmtwrite
#endif	/* RMT_RUNTIME */

extern int rmtaccess PROTO((char *path, int amode));
extern int rmtclose PROTO((int fildes));
extern int rmtcreat PROTO((char *path, int mode));
extern int rmtdup PROTO((int fildes));
extern int rmtfcntl PROTO((int fd, int cmd, int arg));
extern int rmtfstat PROTO((int fildes, struct stat *buf));
extern int rmtioctl PROTO((int fildes, int request, IOCTL3 arg));
extern int rmtisatty PROTO((int fd));
extern int rmtopen PROTO((char *path, int oflag, int mode));
extern int rmtread PROTO((int fildes, char *buf, unsigned int nbyte));
extern int rmtwrite PROTO((int fildes, char *buf, unsigned int nbyte));
extern int _rmt_command PROTO((int fildes, char *buf));
extern int _rmt_dev PROTO((char *path));
extern int _rmt_status PROTO((int fildes));
extern long rmtlseek PROTO((int fildes, long offset, int whence));
extern VOID _rmt_abort PROTO((int fildes));

#endif	/* access */
