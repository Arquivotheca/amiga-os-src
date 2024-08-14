#ifndef FCNTL_H
#define FCNTL_H 1
/**
*
* The following symbols are used for the "open" and "creat" functions.
* They are generally UNIX-compatible, except for O_APPEND under MSDOS,
* which has been moved in order to accomodate the file sharing flags
* defined in MSDOS Version 3.
*
* Also, O_TEMP, O_UNIQ, and O_RAW are Lattice extensions.
*
**/
#define O_RDONLY 0	/* Read-only value (right byte of mode word) */
#define O_WRONLY 1	/* Write-only value */
#define O_RDWR 2	/* Read-write value */
#define O_NDELAY 0	/* Non-blocking I/O flag (N/A) */
#define O_APPEND 8	/* Append mode flag */

#define O_CREAT 0x0100	/* File creation flag */
#define O_TRUNC 0x0200	/* File truncation flag */
#define O_EXCL 0x0400	/* Exclusive access flag */

#define O_RAW 0x8000	/* Raw I/O flag (Lattice feature) */

/**
*
* The following flags are used to establish the protection mode.
*
*/
#define S_IFMT     (S_IFDIR|S_IFREG)
#define S_IFDIR    2048
#define S_IFREG    1024
#define S_ISCRIPT    64
#define S_IPURE      32
#define S_IARCHIVE   16
#define S_IREAD       8
#define S_IWRITE      4
#define S_IEXECUTE    2
#define S_IDELETE     1
/**
*
* The following symbols are used for the "fcntl" function.
*
*/
#define F_DUPFD 0	/* Duplicate file descriptor */
#define F_GETFD 1	/* Get file descriptor flags */
#define F_SETFD 2	/* Set file descriptor flags */
#define F_GETFL 3	/* Get file flags */
#define F_SETFL 4	/* Set file flags */

/**
*
* External definitions
*
**/
#ifndef __ARGS
#ifdef NARGS
#define __ARGS(a) ()
#else
#define __ARGS(a) a
#endif
#endif

extern int open __ARGS((char *, int, ));
extern int creat __ARGS((char *, int));
extern int unlink __ARGS((char *));
extern int remove __ARGS((char *));
extern int rename __ARGS((char *, char *));
extern int read __ARGS((int, void *, unsigned));
extern int write __ARGS((int, void *, unsigned));
extern long lseek __ARGS((int, long, int));
extern long tell __ARGS((int));
extern int close __ARGS((int));
extern int iomode __ARGS((int, int));
extern int isatty __ARGS((int));

/**
*
* Define NULL if it's not already defined
*
*/
#ifndef NULL
#define NULL 0L
#endif

#endif /*FCNTL_H*/
