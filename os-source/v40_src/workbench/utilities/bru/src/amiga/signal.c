/************************************************************************
 *									*
 *	Copyright (c) 1988 Enhanced Software Technologies, Inc.		*
 *			    All Rights Reserved				*
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
 *	signal.c    emulation of unix signal library function
 *
 *  SCCS
 *
 *	@(#)signal.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	This is a partial emulation of the unix signal() library
 *	function for Manx versions of bru, since Manx doesn't
 *	currently support this.  We define our own _abort() function
 *	which does the magic when an interrupt signal is received.
 *
 *  AUTHOR
 *
 *	Fred Fish
 *	Tempe, Arizona
 *
 */


#include "globals.h"

#ifndef MANX

static int dummy;	/* Under ANSI C, must have SOMETHING to compile */

#else

static int (*sigintfunc)() = SIG_DFL;
static int (*sigquitfunc)() = SIG_DFL;


/*
 *  FUNCTION
 *
 *	signal    partial emulation of unix signal() function for Manx
 *
 *  SYNOPSIS
 *
 *	int (*signal (sig, func)()
 *	int sig;
 *	int (*func)();
 *
 *  DESCRIPTION
 *
 *	This is a partial emulation of the unix signal() function for
 *	versions of bru compiled with Manx C.
 *
 */

int (*signal (sig, func))()
int sig;
int (*func)();
{
    int (*rtnval)();

    DBUG_ENTER ("signal");
    DBUG_PRINT ("signals", ("set signal (%d,%lx)", sig, func));
    switch (sig) {
	case SIGINT:
	    rtnval = sigintfunc;
	    sigintfunc = func;
	    break;
	case SIGQUIT:
	    rtnval = sigquitfunc;
	    sigquitfunc = func;
	    break;
	default:
	    DBUG_PRINT ("sig", ("unknown signal %d", sig));
	    break;
    }
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	_abort    replace default _abort() function with our own
 *
 *  SYNOPSIS
 *
 *	VOID _abort();
 *
 */

VOID _abort()
{
    DBUG_ENTER ("_abort");
    if (sigintfunc == SIG_DFL) {
	DBUG_PRINT ("sig", ("no signal catching function, aborting..."));
	exit (1);
    } else {
	DBUG_PRINT ("sig", ("calling signal catching function"));
	(*sigintfunc)(SIGINT);
    }
    DBUG_VOID_RETURN;
}

#endif	/* !MANX */
