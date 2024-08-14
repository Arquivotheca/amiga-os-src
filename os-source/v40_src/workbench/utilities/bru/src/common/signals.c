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
 *	signals.c    routines for manipulating signals
 *
 *  SCCS
 *
 *	@(#)signals.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	Bru catches both interrupt and quit when it is either writing
 *	to an archive or extracting files from an archive.  This helps
 *	to prevent inadvertent corruption of archives or the files being
 *	extracted.
 *
 *	Generally, when interrupt is caught, bru finishes processing on the
 *	current file then cleans up and exits.  When quit is caught, an
 *	immediate exit is taken.
 *
 */

#include "globals.h"

/*
 *	Bru functions in this file.
 */

static int sig_got PROTO((int signo));
static int got_done PROTO((void));


/*
 *  FUNCTION
 *
 *	sig_push    save the current state of interrupt and quit
 *
 *  SYNOPSIS
 *
 *	VOID sig_push (prevINTp, prevQUITp)
 *	SIGTYPE *prevINTp;
 *	SIGTYPE *prevQUITp;
 *
 *  DESCRIPTION
 *
 *	Sig_push is passed pointers to locations to store the values of
 *	the previous states of interrupt and quit.  It leaves the signal
 *	handling state in the same state as it was when called.  It
 *	returns no value.
 *
 *	If the system does not support SIGINT or SIGQUIT, the operation
 *	is a nop.
 */

VOID sig_push (prevINTp, prevQUITp)
SIGTYPE *prevINTp;
SIGTYPE *prevQUITp;
{
    DBUG_ENTER ("sig_push");
#ifdef SIGINT
    *prevINTp = s_signal (SIGINT, SIG_IGN);
    (VOID) s_signal (SIGINT, *prevINTp);
    DBUG_PRINT ("sig", ("previous SIGINT %#x", *prevINTp));
#endif
#ifdef SIGQUIT
    *prevQUITp = s_signal (SIGQUIT, SIG_IGN);
    (VOID) s_signal (SIGQUIT, *prevQUITp);
    DBUG_PRINT ("sig", ("previous SIGQUIT %#x", *prevQUITp));
#endif
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	sig_pop    restore the previous states of interrupt and quit
 *
 *  SYNOPSIS
 *
 *	VOID sig_pop (prevINTp, prevQUITp)
 *	SIGTYPE *prevINTp;
 *	SIGTYPE *prevQUITp;
 *
 *  DESCRIPTION
 *
 *	Generally, sig_push and sig_pop are used together to save
 *	the signal processing state, set it to some new state, then
 *	reset the state back when done.  The typical usage is of the
 *	form:
 *
 *		foo (arg1, arg2)
 *		{
 *			SIGTYPE prevINT;
 *			SIGTYPE prevQUIT;
 *
 *			sig_push (&prevINT, &prevQUIT);
 *			sig_catch ();
 *				.
 *				.
 *				.
 *			sig_pop (&prevINT, &prevQUIT);
 *		}
 *
 *	Sig_pop simply sets the signal handling state and returns
 *	no value.
 *
 */

VOID sig_pop (prevINTp, prevQUITp)
SIGTYPE *prevINTp;
SIGTYPE *prevQUITp;
{
    DBUG_ENTER ("sig_pop");
#ifdef SIGINT
    DBUG_PRINT ("sig", ("new SIGINT %#x", *prevINTp));
    (VOID) s_signal (SIGINT, *prevINTp);
#endif
#ifdef SIGQUIT
    DBUG_PRINT ("sig", ("new SIGQUIT %#x", *prevQUITp));
    (VOID) s_signal (SIGQUIT, *prevQUITp);
#endif
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	sig_catch    set up to catch interrupt and quit
 *
 *  SYNOPSIS
 *
 *	VOID sig_catch ()
 *
 *  DESCRIPTION
 *
 *	Sets up signal handling to catch both interrupt and quit, it
 *	they were not previously being ignored.  This call should
 *	generally be preceeded by a call to sig_push to save the state
 *	for later restoration by sig_pop.
 *
 *  AMIGADOS NOTE
 *
 *	Since the usual C environment does not provide an easy way
 *	to minic the async nature of Unix signals, and distinguish
 *	between SIGINT and SIGQUIT, we don't really have the option
 *	of ignoring SIGINT but still quitting on SIGQUIT.  So we
 *	never ignore signals.
 */

VOID sig_catch ()
{
#if !amigados
    SIGTYPE prevINT;
    SIGTYPE prevQUIT;
#endif

    DBUG_ENTER ("sig_catch");
#if defined(SIGINT) && !amigados
    prevINT = s_signal (SIGINT, SIG_IGN);
    if (prevINT != SIG_IGN) {
	(VOID) s_signal (SIGINT, (SIGTYPE) sig_got);
	DBUG_PRINT ("sig", ("SIGINT now being caught"));
    }
#endif
#if defined(SIGINT) && !amigados
    prevQUIT = s_signal (SIGQUIT, SIG_IGN);
    if (prevQUIT != SIG_IGN) {
	(VOID) s_signal (SIGQUIT, (SIGTYPE) sig_got);
	DBUG_PRINT ("sig", ("SIGQUIT now being caught"));
    }
#endif
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	sig_got    internal interrupt and quit handler
 *
 *  SYNOPSIS
 *
 *	static int sig_got (signo)
 *	int signo;
 *
 *  DESCRIPTION
 *
 *	Internal signal handler used by sig_catch.
 *
 *	When interrupt is caught, simply sets the external variable
 *	"interrupt" to TRUE and increments the warning count.
 *	Processing then continues normally.  It is the responsibility
 *	of other modules which use sig_catch to test the
 *	value of "interrupt" at appropriate times to determine if
 *	an interrupt has been received.
 *
 *	When quit is caught, increments the error count and calls
 *	the cleanup and exit routine.
 *
 *	If an unknown signal is caught, prints a bug warning and
 *	processing continues.
 *
 *  NOTE
 *
 *	This routine always returns a meaningful value (zero) because
 *	the kernel could (potential) make use of the value returned by
 *	signal handlers.
 */

static int sig_got (signo)
int signo;
{
    DBUG_ENTER ("sig_got");
    DBUG_PRINT ("sig", ("caught signal %d", signo));
    sig_catch ();
    switch (signo) {
#ifdef SIGINT
	case SIGINT:
	    if (flags.vflag > 3) {
		bru_message (MSG_VERBOSITY, "\ninterrupted ...");
	    }
	    einfo.warnings++;
	    interrupt = TRUE;
	    break;
#endif
#ifdef SIGQUIT
	case SIGQUIT:
	    if (flags.vflag > 3) {
		bru_message (MSG_VERBOSITY, "\naborted ...");
	    }
	    einfo.errors++;
	    done ();
	    break;
#endif
	default:
	    bru_message (MSG_BUG, "got_sig");
	    break;
    }
    DBUG_RETURN (0);
}


/*
 *  FUNCTION
 *
 *	sig_done    set signal handling to cause exit
 *
 *  SYNOPSIS
 *
 *	VOID sig_done ()
 *
 *  DESCRIPTION
 *
 *	Unconditionally sets the signal handling for interrupt and quit
 *	to cause cleanup and exit.  This routine is generally called
 *	immediately before reading interactive input from the user's
 *	terminal, thus it does not test the current state of signal
 *	handling.  Thus even a version of bru running in background,
 *	ignoring interrupt and quit, can be terminated if it ever
 *	issues a prompt to the terminal.
 *
 */

VOID sig_done ()
{
    DBUG_ENTER ("sig_done");
#ifdef SIGINT
    (VOID) s_signal (SIGINT, (SIGTYPE) got_done);
    DBUG_PRINT ("sig", ("SIGINT will now cause exit"));
#endif
#ifdef SIGQUIT
    (VOID) s_signal (SIGQUIT, (SIGTYPE) got_done);
    DBUG_PRINT ("sig", ("SIGQUIT will now cause exit"));
#endif
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	got_done    local cleanup and exit preprocessor code
 *
 *  SYNOPSIS
 *
 *	static int got_done ()
 *
 *  DESCRIPTION
 *
 *	The routine sig_done sets signal handling for interrupt and
 *	quit to call this routine when interrupt or quit is caught.
 *	This routine returns the cursor to the left margin (we
 *	presumably were waiting for user input in response to a prompt)
 *	then calls the global cleanup and exit routine.
 *
 *  NOTE
 *
 *	This routine always returns a meaningful value (zero) because
 *	the kernel could (potential) make use of the value returned by
 *	signal handlers.
 */

static int got_done ()
{
    DBUG_ENTER ("got_done");
#if unix || xenix
    bru_message (MSG_TTYNL);
#endif
    done ();
    DBUG_RETURN (0);
}


VOID sig_pipe (ignore)
BOOLEAN ignore;
{
    DBUG_ENTER ("sig_pipe");
#ifdef SIGPIPE
    if (ignore) {
	(VOID) s_signal (SIGPIPE, SIG_IGN);
    } else {
	(VOID) s_signal (SIGPIPE, SIG_DFL);
    }
#endif
    DBUG_VOID_RETURN;
}
