/************************************************************************
 *									*
 *	Copyright (c) 1989 Enhanced Software Technologies, Inc.		*
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
 *	%M%    message code definitions
 *
 *  SCCS
 *
 *	%W%	%G%
 *
 *  SYNOPSIS
 *
 *	#include "%M%"
 *
 *  DESCRIPTION
 *
 *	Define message numbers.  Each message is assigned a name of
 *	the form "MSG_XXX..", where XXX... is some mnemonically
 *	meaningful string for the specific message.
 *
 *	Each time a condition requiring a message is encountered,
 *	the message handling routine "bru_message" is called to
 *	issue an appropriate message.  The first argument to
 *	bru_message is the message number.
 *
 */


#define MSG_MODE	1	/* Confusion over mode */
#define MSG_AROPEN	2	/* Error opening archive */
#define MSG_ARCLOSE	3	/* Error closing archive */
#define MSG_ARREAD	4	/* Error reading archive */
#define MSG_ARWRITE	5	/* Error writing archive */
#define MSG_ARSEEK	6	/* Error seeking on archive */
#define MSG_BUFSZ	7	/* Media smaller than I/O buffer */
#define MSG_MAXBUFSZ	8	/* Buffer size exceeds max for device */
#define MSG_BALLOC	9	/* Can't allocate block buffers */
#define MSG_BSEQ	10	/* Block sequence error */
#define MSG_DSYNC	11	/* File header error; resync */
#define MSG_EACCESS	12	/* File does not exist */
#define MSG_STAT	13	/* Can't stat file */
#define MSG_BIGPATH	14	/* Pathname too big */
#define MSG_BIGFAC	15	/* Blocking factor too big */
#define MSG_OPEN	16	/* Can't open file */
#define MSG_CLOSE	17	/* Error closing file */
#define MSG_READ	18	/* Error reading from file */
#define MSG_FTRUNC	19	/* File was truncated */
#define MSG_FGREW	20	/* File grew while archiving */
#define MSG_SUID	21	/* Not owner, can't set user id */
#define MSG_SGID	22	/* Not in group, can't set group id */
#define MSG_EXEC	23	/* Can't exec a file */
#define MSG_FORK	24	/* Can't fork */
#define MSG_BADWAIT	25	/* Unrecognized wait return */
#define MSG_EINTR	26	/* Interrupted system call */
#define MSG_CSTOP	27	/* Child process stopped */
#define MSG_CTERM	28	/* Child process terminated */
#define MSG_CORE	29	/* Child process dumped core */
#define MSG_WSTATUS	30	/* Inconsistent wait status */
#define MSG_SETUID	31	/* Error setting real & effective uid */
#define MSG_SETGID	32	/* Error setting real & effective gid */
#define MSG_SUM		33	/* Archive checksum error */
#define MSG_BUG		34	/* Internal bug detected */
#define MSG_MALLOC	35	/* Error allocating space */
#define MSG_WALK	36	/* Internal consistency error in tree walk */
#define MSG_DEPTH	37	/* Pathname too big in recursive save */
#define MSG_SEEK	38	/* Error seeking on file */
#define MSG_ISUM	39	/* Checksum error in info block */
#define MSG_WRITE	40	/* Error writing to file */
#define MSG_SMODE	41	/* Error setting file mode */
#define MSG_CHOWN	42	/* Error setting uid/gid */
#define MSG_STIMES	43	/* Error setting times */
#define MSG_MKNOD	44	/* Error making a non-regular file */
#define MSG_MKLINK	45	/* Error making link */
#define MSG_ARPASS	46	/* Inconsistent phys block addresses */
#define MSG_IMAGIC	47	/* Bad info block magic number */
#define MSG_LALLOC	48	/* Lost linkage for file */
#define MSG_URLINKS	49	/* Unresolved links */
#define MSG_TTYOPEN	50	/* Can't open tty */
#define MSG_NTIME	51	/* Error converting time */
#define MSG_UNAME	52	/* Error getting unix name */
#define MSG_LABEL	53	/* Archive label string too big */
#define MSG_GUID	54	/* Error converting uid for -o option */
#define MSG_CCLASS	55	/* Botched character class pattern */
#define MSG_OVRWRT	56	/* Can't overwrite file */
#define MSG_WACCESS	57	/* Can't access file for write */
#define MSG_RACCESS	58	/* Can't access file for read */
#define MSG_COPEN	59	/* File will not be contiguous */
#define MSG_CNOSUP	60	/* No support for contiguous files */
#define MSG_STDIN	61	/* Illegal use of standard input */
#define MSG_PEOV	62	/* Premature end of volume */
#define MSG_UNFMT	63	/* Media appears to be unformatted */
#define MSG_FIRST	64	/* General first read/write error */
#define MSG_BRUTAB	65	/* Can't find device table file */
#define MSG_SUPERSEDE	66	/* File not superseded */
#define MSG_WRTPROT	67	/* Media appears to be write protected */
#define MSG_IGNORED	68	/* File not in archive or ignored */
#define MSG_FASTMODE	69	/* May need to use -F option on read */
#define MSG_BACKGND	70	/* Abort if any interaction required */
#define MSG_MKDIR	71	/* Mkdir system call failed */
#define MSG_RDLINK	72	/* Readlink system call failed */
#define MSG_NOSYMLINKS	73	/* System does not support symbolic links */
#define MSG_MKSYMLINK	74	/* Could not make the symbolic link */
#define MSG_MKFIFO	75	/* Could not make a fifo */
#define MSG_SYMTODIR	76	/* Hard link to a directory if no symlinks */
#define MSG_HARDLINK	77	/* Target for a hard link does not exist */
#define MSG_FIFOTOREG	78	/* Extracted a fifo as a regular file */
#define MSG_ALINKS	79	/* Additional links added while running */
#define MSG_OBTF	80	/* Obsolete brutab entry */
#define MSG_DEFDEV	81	/* No default device in bru device table */
#define MSG_NOBTF	82	/* No support for obsolete brutab format */
#define MSG_BSIZE	83	/* Attempt to change buffer size on nth vol */
#define MSG_DBLIO	84	/* General double buffering I/O error */
#define	MSG_DBLSUP	85	/* Error setting up double buffering */
#define MSG_EJECT	86	/* Error attempting to eject media */
#define MSG_NOSHRINK	87	/* Compressed version was larger, saved
				   uncompressed */
#define MSG_ZXFAIL	88	/* Extraction of compressed file failed */
#define MSG_NOEZ	89	/* Estimate mode ignores compression */
#define MSG_UNLINK	90	/* Failed to unlink a file */
#define MSG_ZFAILED	91	/* Compression failed for some nonspecific
				   reason */
#define MSG_BADBITS	92	/* Bad number of compression bits */
#define MSG_SHMSIZE	93	/* Buffer size too large for double buffering */
#define MSG_BUFADJ	94	/* Buffer size automatically adjusted */
#define MSG_SHMGET	95	/* Call to shmget failed */
#define MSG_SHMAT	96	/* Call to shmat failed */
#define MSG_MSGGET	97	/* Could not get message queue */
#define MSG_IOPT	98	/* Garbled -I option */
#define MSG_MAXSEGS	99	/* Need at least two shared memory regions */
#define MSG_SBRK	100	/* Got error from sbrk call */
#define MSG_NOZ		101	/* Compression suppressed, can't setup */
#define MSG_CLDUNK	102	/* Unknown child died */
#define MSG_SLVDIED	103	/* Double buffer slave died */
#define MSG_SLVERR	104	/* Double buffer slave error */
#define MSG_REAP	105	/* No child to reap */
#define MSG_SHMCOPY	106	/* Warn that device may need shmcopy in 
				   brutab */
#define MSG_UWERR	107	/* Unrecoverable write error */
#define MSG_UFORWP	108	/* Unformatted or write protected media */
#define MSG_AEOV	109	/* Assuming end of volume */
#define MSG_WRONGVOL	110	/* Found wrong volume number */
#define MSG_WRONGTIME	112	/* Volume from different archive */
#define MSG_DATAOVW	113	/* All data on volume will be overwritten */
#define MSG_NEWPASS	114	/* Ready for a new pass over archive */
#define MSG_TTYNL	115	/* Terminate current tty output line */
#define MSG_VERBOSITY	116	/* A verbosity message */
#define MSG_NOREWIND	117	/* Can't rewind archive device */
#define MSG_RERUN	118	/* Rerun suggestion */
#define MSG_CONFIRM	119	/* A confirm query message */
#define MSG_ACTION	120	/* A query message to select action */
#define MSG_LOADVOL	121	/* Load a new volume */
#define MSG_TOOLARGE	122	/* File size exceeds ulimit maximum */
#define MSG_ULIMITSET	123	/* Can't set ulimit */
#define MSG_NODBLBUF	124	/* No double buffering support */
#define MSG_DBLBUFOFF	125	/* Problem with shared mem in kernel */
#define MSG_MSGSND	126	/* Problem sending message */
#define MSG_MSGRCV	127	/* Problem receiving message */
#define MSG_FDATACHG	128	/* File data changed (but size didn't) */
#define MSG_STACK	129	/* Stack size less than recommended min */
