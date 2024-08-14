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
 *	macros.h    useful preprocessor macros for bru utility
 *
 *  SCCS
 *
 *	@(#)macros.h	12.8	11 Feb 1991
 *
 *  SYNOPSIS
 *
 *	#include <sys/types.h>
 *	#include <sys/stat.h>
 *	#include "typedefs.h"
 *	#include "macros.h"
 *
 *  DESCRIPTION
 *
 *	Contains miscellaneous useful macros for the bru utility.
 *
 *	Note that all macros consist of upper case names only.  I
 *	consider it a serious error that the stdio library makes
 *	use of macros with lower case names, because then it is
 *	not obvious that the symbol represents a macro.  Even more
 *	serious is the fact that not all implementations use macros
 *	for the same symbol.  This causes porting problems when
 *	things like "extern int getc();" or "foo (getc,&foobar)"
 *	work in implementations where getc is a real function.
 *
 */


#define TOHEX(out,val) tohex((out),(S32BIT)(val),sizeof(out))
#define FROMHEX(in) fromhex((in),sizeof(in))
#define STRSAME(a,b) (s_strcmp ((a),(b)) == 0)		/* Strings equal */
#define DOTFILE(a) (STRSAME (".", (a)) || STRSAME ("..", (a)))
#define SPECIAL(a) (DOTFILE(a) || STRSAME ("/", (a)))
#define STREND(start,scan) for ((scan) = (start); *(scan) != EOS; (scan)++)
#define LINKED(blkp) (*((blkp) -> sb.data.fh.f_lname) != EOS)


/*
 *	The following macros deal with size conversions.
 *
 *	ARBLKS:		Convert number of data bytes to
 *			number of archive data blocks
 *			required to archive the data.
 *
 *	BLOCKS:		Convert number of bytes to number
 *			of archive blocks.
 *
 *	KBYTES:		Convert number of blocks to
 *			kilobytes of archive output.  Note
 *			that this IS NOT kilobytes of
 *			archived data, because it includes
 *			the header overhead.
 *
 *	ZSIZE		Given a file info pointer, return
 *			"real" size of the file if not
 *			compressed, or "compressed" size.
 *
 *	ZARBLKS		Same as ARBLKS, but takes a file
 *			info pointer instead, and returns
 *			number of archive blocks for compressed
 *			version if compression is used.
 *
 */
 
#define ARBLKS(size) (((size) + DATASIZE - 1) / DATASIZE)
#define BLOCKS(size) (((size) + BLKSIZE - 1) / BLKSIZE)
#define KBYTES(blks) ((blks * BLKSIZE) / 1024)
#define B2KB(bytes) ((long) bytes/1024)
#define ZSIZE(fip) (IS_COMPRESSED(fip)?((fip)->zsize):((fip)->bstatp->bst_size))
#define ZARBLKS(fip) (ARBLKS(ZSIZE(fip)))


/*
 *	The following macros provide the number of elements
 *	in an array and the address of the first array
 *	element which is past the end of the array.
 */

#define ELEMENTS(array) (sizeof(array)/sizeof(array[0]))
#define OVERRUN(array) (&(array[ELEMENTS(array)]))

/*
 *	Declare certain externals used in the macro expansions
 */

extern S32BIT fromhex ();
extern VOID tohex ();
extern int s_strcmp ();

/*
 *	Define the common max() and min() macros if not defined
 *	already.
 */

#ifndef min
#define min(a,b)	((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a,b)	((a) > (b) ? (a) : (b))
#endif

/*
 *	Keep lint happy when it wants to see some statement.
 */

#if lint
#define LINTCOOKIE {extern int lintcookie; lintcookie = 1;}
#else
#define LINTCOOKIE
#endif

#ifdef __STDC__
# define PROTO(s) s
#else
# define PROTO(s) ()
#endif

/*
 *	The following macros are nops on any machine except for the
 *	Amiga, where we have to subvert the file protections to do
 *	our job of being able to save or restore any file.
 */

#if amigados
#define PROTBITS	(FIBF_READ | FIBF_WRITE | FIBF_DELETE)
#define GETPROT(fip)	((fip) -> fib_OrigProt = getprot ((fip) -> fname))
#define SETOLDPROT(fip)	setprot ((fip) -> fname, (fip) -> fib_OrigProt)
#define FULLACCESS(fip)	setprot ((fip) -> fname, ~PROTBITS)
#else
#define GETPROT(fip)
#define SETOLDPROT(fip)
#define FULLACCESS(fip)
#endif
