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
 *	stubs.c    stubs for functions that have no Unix equivalents
 *
 *  SCCS
 *
 *	@(#)stubs.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	This module contains stubs for functions that have no Unix
 *	equivalent, and for which no equivalent is necessary.  This
 *	helps to prevent proliferation of lots of #ifdef'd code,
 *	which the author finds to be ugly and detrimental to
 *	readability.  This file will probably grow as #ifdefs are
 *	removed from existing routines.
 *
 */

#include "globals.h"



/*VARARGS*/
VOID clear_archived (fip)
struct finfo *fip;
{
}

/*VARARGS*/
VOID mark_archived (fip)
struct finfo *fip;
{
}

/*VARARGS*/
int setinfo (fip)
struct finfo *fip;
{
    return (0);
}

#if 0

/*
 *	Set the raw AmigaDOS file protection bits.
 */

int setprot (path, prot)
char *path;
long prot;
{
	return (0);
}

/*
 *	Get the raw AmigaDOS file protection bits.
 */

long getprot (path)
char *path;
{
	return (0);
}

#endif

/*
 *	This routine checks to see if the archive bit is set in the file
 *	attributes flags word (currently Amiga only).
 */

/*VARARGS*/
int abit_set (fip)
struct finfo *fip;
{
    return (0);
}

/*
 *	This routine gets the next pathname during filter mode using
 *      a ipc type interface (currently Amiga only).  Returns 1 on
 *	success, 0 on failure.
 */

/*VARARGS*/
int ipc_getfilename (namebuf)
char *namebuf;
{
    return (0);
}

/*
 *	This routine sends a query string to a ipc type interface
 *	(currently Amiga only).  Returns 1 on success, 0 on failure.
 *
 */

/* VARARGS*/
int ipc_query (message)
char *message;
{
    return (0);
}

/*
 *	This routine reads a single character from a ipc type interface
 *	(currently Amiga only).  Returns EOF on failure.  Note that the
 *	return type is int, so it can return EOF as an out-of-band
 *	indicator that no character is available.
 *
 */

/* VARARGS*/
int ipc_getc ()
{
    return (EOF);
}

/*
 *	This routine sends an error string out a ipc type interface
 *	(currently Amiga only).
 *
 */

/* VARARGS*/
int ipc_error (errmsg)
char *errmsg;
{
    return (0);
}

int ipc_init ()
{
    return (0);
}

/* This is a NOP on most reasonable (I.E. Unix) machines. */

VOID stackcheck ()
{
}
