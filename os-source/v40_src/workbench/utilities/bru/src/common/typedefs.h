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
 *  FILES
 *
 *	typedefs.h    user defined type definitions
 *
 *  SCCS
 *
 *	@(#)typedefs.h	12.8	11 Feb 1991
 *
 *  SYNOPSIS
 *
 *	#include "typedefs.h"
 *
 *  DESCRIPTION
 *
 *	Miscellaneous typedefs to make things clearer.
 *
 *	Also, since not all C compilers support all mixes of
 *	types (such as unsigned char for example), the
 *	more exotic types are typedef'd also.
 *
 */


typedef unsigned int UINT;	/* Unsigned integer */
typedef unsigned long ULONG;	/* Unsigned long integer */

#ifndef EXEC_TYPES_H		/* Found in AmigaDOS's types.h */
#undef USHORT			/* CTIX has USHORT(x) macro, yuck! */
typedef unsigned short USHORT;	/* Unsigned short */
#endif

typedef unsigned char UCHAR;	/* Unsigned char */

#ifndef VOID
#if BROKEN_VOID
#define VOID char
#else
#define VOID void		/* Void type (typedef bug?) */
#endif
#endif

typedef int MODE;
typedef int BOOLEAN;		/* Has value of TRUE or FALSE */
typedef long S32BIT;		/* Signed 32 bit word */

typedef S32BIT LBA;		/* Logical block address */
typedef S32BIT CHKSUM;		/* Checksums are 32 bits */

/*
 *	Nobody can seem to agree on whether signal() returns a
 *	pointer to a function that returns int or a pointer to
 *	a function that returns void.  I know for a fact that
 *	various ports of BSD define these both ways, and the same
 *	for USG ports.  Some ports don't even define them compatibly
 *	between the actual library code, the <signal.h> file, and the
 *	lint libraries!  Grrrrr!!!!
 *
 */

#if !SIGTYPEINT && !SIGTYPEVOID
#  define SIGTYPEINT 1		/* Pick a default, any default... */
#endif

#if SIGTYPEINT
typedef int (*SIGTYPE)();	/* Pointer to function returning int */
#endif

#if SIGTYPEVOID
typedef VOID (*SIGTYPE)();	/* Pointer to function return void */
#endif
