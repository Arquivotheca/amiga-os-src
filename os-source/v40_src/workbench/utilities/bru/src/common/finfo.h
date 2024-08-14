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
 *	finfo.h    file information structure definition
 *
 *  SCCS
 *
 *	@(#)finfo.h	12.8	11 Feb 1991
 *
 *  SYNOPSIS
 *
 *	#include <sys/types.h>
 *	#include <sys/stat.h>
 *	#include "typedefs.h"
 *	#include "finfo.h"
 *
 *  DESCRIPTION
 *
 *	Certain information about the current file being processed
 *	is important to the error reporting strategies, archive
 *	corruption recovery strategies, maintenance of archive
 *	block header information, etc.  Rather than make all these
 *	globals, they are collected together in a single structure
 *	called appropriately enough, "finfo".
 *
 */

/*
 *	The internal stat structure used by bru.  Individual members
 *	are large enough to hold the values of the system's stat
 *	structure members, which range from short to long.
 */

struct bstat {
    long bst_mode;		/* File permissions, types, etc */
    int bst_ino;		/* Inode number (UNIX versions) */
    int bst_dev;		/* Major device number (UNIX versions) */
    int bst_rdev;		/* Minor device number (UNIX versions) */
    int bst_nlink;		/* Number of links */
    int bst_uid;		/* File owner's user ID number */
    int bst_gid;		/* File owner's group ID number */
    unsigned long bst_size;	/* File size in bytes */
    long bst_atime;		/* Timestamp of last access to file contents */
    long bst_mtime;		/* Timestamp of last mod of file contents */
    long bst_ctime;		/* Timestamp of last change to inode */
};

/*
 *	Defines and macros for manipulating the bst_mode member.  These
 *	are similar to the typical UNIX definitions.
 */

#define BS_IAMB		0000777		/* Permission bits mask */
#define BS_IEXEC	0000100		/* Owner Execute/search permission */
#define BS_IWRITE	0000200		/* Owner Write permission */
#define BS_IREAD	0000400		/* Owner Read permission */

#define BS_ISMB		0007000		/* Special bits mask */
#define BS_ISVTX	0001000		/* Save swapped text after use */
#define BS_ISGID	0002000		/* Set group id on execution */
#define BS_ISUID	0004000		/* Set user id on execution */

#define BS_IFMT		0170000		/* Mask for file type */
#define BS_IFIFO	0010000		/* A fifo */
#define BS_IFCHR	0020000		/* A character special file */
#define BS_IFDIR	0040000		/* A directory file */
#define BS_IFNAM	0050000		/* A special named file (XENIX) */
#define BS_IFBLK	0060000		/* A block special file */
#define BS_IFREG	0100000		/* A a regular file */
#define BS_IFCTG	0110000		/* A contiguous regular file */
#define BS_IFLNK	0120000		/* A symbolic link (BSD) */
#define BS_IFSOCK	0140000		/* A socket */

#define IS_FIFO(mode) (((mode) & BS_IFMT) == BS_IFIFO)	/* Is fifo */
#define IS_DIR(mode) (((mode) & BS_IFMT) == BS_IFDIR)	/* Is directory */
#define IS_CSPEC(mode) (((mode) & BS_IFMT) == BS_IFCHR)	/* Is char special */
#define IS_BSPEC(mode) (((mode) & BS_IFMT) == BS_IFBLK)	/* Is block special */
#define IS_SPEC(mode) (IS_CSPEC(mode)||IS_BSPEC(mode))	/* CSPEC or BSPEC */
#define IS_REG(mode) (((mode) & BS_IFMT) == BS_IFREG)	/* Is normal file */
#define IS_FLNK(mode) (((mode) & BS_IFMT) == BS_IFLNK)	/* Is symbolic link */
#define IS_CTG(mode) (((mode) & BS_IFMT) == BS_IFCTG)	/* Is contiguous */


/*
 *	Defined bits in the fi_flags word.  The flags word is initialized
 *	to zero for Unix hosts and to FI_AMIGA for AmigaDOS.
 *
 *	The state of Zflag is saved in each file header, so that it
 *	can be automatically recovered and set for operations which
 *	read the archive.
 */

#define FI_CHKSUM	(1<<0)	/* Checksum error seen on this file */
#define FI_BSEQ		(1<<1)	/* Block sequence error seen on this file */
#define FI_AMIGA	(1<<2)	/* Enable special AmigaDOS features */
#define FI_LZW		(1<<3)	/* File compressed with modified Lempel-Ziv */
#define FI_ZFLAG	(1<<4)	/* Zflag was active, even if not compressed */
#define FI_XFNAME	(1<<5)	/* Filename is in extension area */
#define FI_XLNAME	(1<<6)	/* Linked name in extension area */

#if amigados
#    define FI_FLAGS_INIT	FI_AMIGA
#else
#    define FI_FLAGS_INIT	0
#endif

/*
 *	The file information structure is used internally to keep track
 *	of useful information about files that are being processed.
 *	Some of the information is also recorded in the bru archive,
 *	and some is only transient and used internally.
 *
 *	When a file is being stored in compressed form, the 
 *	IS_COMPRESSED() macro returns nonzero.  In this case,
 *	the zsize field is used to hold the size of the file
 *	after compression.
 */

struct finfo {
    struct bstat *bstatp;	/* Pointer to internal form of stat info */
    char *fname;		/* Full name of file */
    char *zfname;		/* Full name of compressed temporary file */
    char *lname;		/* Full name of file linked to, if known */
    char *bname1;		/* Image of h_name field in block header */
    char *bname2;		/* Image of f_lname field in file header */
    char *bname3;		/* Image of f_xname field in file header */
    int fildes;			/* File descriptor */
    int zfildes;		/* Compress file descriptor */
    LBA flba;			/* Archive blk relative to start of file */
    LBA chkerrs;		/* Accumulated checksum error count */
    long fi_flags;		/* Flag word */
    int type;			/* Type of pathname if archived file */
    long fib_Protection;	/* AmigaDOS fib_Protection word */
    long fib_OrigProt;		/* Original AmigaDOS fib_Protection word */
    char fib_Comment[116];	/* AmigaDOS fib_Comment string */
    unsigned long zsize;	/* Size of file after compression */
    long fseq;			/* Sequence number of this file */
};

/*
 *	Useful macros from manipulating file information.
 */
 
#define IS_STEM(fip) ((fip) -> type == STEM)
#define IS_LEAF(fip) ((fip) -> type == LEAF)
#define IS_EXTENSION(fip) ((fip) -> type == EXTENSION)
#define IS_NOMATCH(fip) ((fip) -> type == NOMATCH)
#define IS_FINISHED(fip) ((fip) -> type == FINISHED)
#define IS_COMPRESSED(fip) ((fip) -> fi_flags & FI_LZW)
