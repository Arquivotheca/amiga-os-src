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
 *	execute.c    routines for running a child process
 *
 *  SCCS
 *
 *	@(#)execute.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	Contains routines responsible for forking another program,
 *	waiting for a return, and interpreting the conditions under
 *	which the child process returned.
 *
 */

#include "globals.h"

static VOID wait_failed PROTO((int child_rtn));
static VOID wait_success PROTO((int child_status, char *pathname));
static BOOLEAN secure PROTO((void));

#if amigados
struct ProcID child;
#endif


/*
 *  FUNCTION
 *
 *	execute   start a process with given argument list
 *
 *  SYNOPSIS
 *
 *	BOOLEAN execute (dir, file, vector)
 *	char *dir;
 *	char *file;
 *	char *vector[];
 *
 *  DESCRIPTION
 *
 *	Given executable file <file> in directory <dir>
 *	runs the file as a child process with arguments <vector>.
 *	The function argument <dir> is optional, if present
 *	it is concatenated with <file>, separated with a '/'
 *	to build the full pathname of the file to execute.
 *
 *	Returns TRUE on success, with no errors.  If <file>
 *	cannot be executed for some reason, an error message is
 *	printed and FALSE is returned.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin execute
 *	    If no directory specified then
 *		Use just the file name
 *	    Else
 *		First component is the directory
 *		Second component is the separator
 *		Third component is the file name
 *	    End if
 *	    Fork a child process
 *	    If we are the child process then
 *		If security checks pass then
 *		    Overlay with new executable file
 *		    Print error message if failed
 *		End if
 *		Exit the child process
 *	    Else if fork failed then
 *		Print message
 *		Return will be FALSE
 *	    Else
 *		Do
 *		    Wait for child
 *		    If not the child we were waiting for then
 *			Process failure
 *		    Else
 *			Process success for our child
 *		    Endif
 *		While our child has not yet returned
 *		Return will be TRUE
 *	    End if
 *	    Return flag
 *	End execute
 *
 */


BOOLEAN execute (dir, file, vector)
char *dir;
char *file;
char *vector[];
{
    char **vp;
    int child_rtn;
    BOOLEAN rtnval;
    char pathname[128];
#if unix || xenix
    int child_pid;
    int child_status;
#endif

    DBUG_ENTER ("execute");
    if (dir == NULL) {
	(VOID) s_strcpy (pathname, file);
    } else {
	(VOID) s_strcpy (pathname, dir);
	(VOID) s_strcat (pathname, "/");
	(VOID) s_strcat (pathname, file);
    }
    DBUG_PRINT ("exec", ("exec file \"%s\"", pathname));
    for (vp = vector; *vp != NULL; vp++) {
	DBUG_PRINT ("vector", ("vector[%d] \"%s\"", vp - vector, *vp));
    }
#if amigados
    DBUG_PRINT ("fork", ("calling forkv for '%s'", pathname));
    if (forkv (pathname, vector, (struct FORKENV *) NULL, &child) == -1) {
	DBUG_PRINT ("fork", ("fork unsuccessful"));
	bru_message (MSG_EXEC, pathname);
	rtnval = FALSE;
    } else {
	DBUG_PRINT ("fork", ("fork successful, now waiting for child"));
	child_rtn = wait (&child);
	/* wait_success (child_rtn, pathname); */
	rtnval = TRUE;
    }
#else
    child_pid = s_fork ();
    if (child_pid == 0) {
 	if (secure ()) {
	    (VOID) s_execv (pathname, vector);
	    bru_message (MSG_EXEC, pathname);
	}
	s_exit (1);
    } else if (child_pid == -1) {
	bru_message (MSG_FORK);
	rtnval = FALSE;
    } else {
	do {
	    DBUG_PRINT ("wait", ("waiting for child %d", child_pid));
	    child_rtn = s_wait (&child_status);
	    if (child_rtn != child_pid) {
		wait_failed (child_rtn);
	    } else {
		wait_success (child_status, pathname);
	    }
	} while (child_pid != child_rtn);
	DBUG_PRINT ("child", ("child %d returned", child_pid));
	rtnval = TRUE;
    }
#endif
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	wait_failed    process bad return from child
 *
 *  SYNOPSIS
 *
 *	static VOID wait_failed (child_rtn)
 *	int child_rtn;
 *
 *  DESCRIPTION
 *
 *	When the return value from the "wait" system call
 *	is not the process ID of the child, there is some
 *	sort of a problem.  This function is called when
 *	this condition occurs.  Basically, either the wait
 *	call returned the process ID of another process, or
 *	was interrupted.
 *
 */
 
static VOID wait_failed (child_rtn)
int child_rtn;
{
    DBUG_ENTER ("wait_failed");
    if (child_rtn != -1) {
	bru_message (MSG_BADWAIT, child_rtn);
    } else {
	bru_message (MSG_EINTR);
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	wait_success    process successful return from child
 *
 *  SYNOPSIS
 *
 *	static VOID wait_success (child_status, pathname)
 *	int child_status;
 *	char *pathname;
 *
 *  DESCRIPTION
 *
 *	When the system call "wait" returns the process ID of
 *	the child process, we must check the child's return
 *	status to see if there were any problems.  This
 *	function interprets the returned status code and issues
 *	any appropriate messages when needed.
 *
 *  NOTES
 *
 *	The ordering of the "if" statements is significant
 *	in determining/eliminating states.
 *
 */
 
static VOID wait_success (child_status, pathname)
int child_status;
char *pathname;
{
    int low_byte;
    int high_byte;

    DBUG_ENTER ("wait_success");
    low_byte = child_status & 0377;
    high_byte = child_status >> 8;
    if (low_byte == 0177) {
	if (high_byte != SIGINT) {
	    bru_message (MSG_CSTOP, pathname, high_byte);
	}
    } else if (low_byte == 0) {
	LINTCOOKIE;
	DBUG_PRINT ("child_status", ("child status %d", high_byte));
    } else if (high_byte == 0) {
	bru_message (MSG_CTERM, pathname, low_byte & 0177);
	if (low_byte & 0200) {
	    bru_message (MSG_CORE, pathname);
	}
    } else {
	bru_message (MSG_WSTATUS, child_status);
    }
    DBUG_VOID_RETURN;
}


static BOOLEAN secure ()
{
    int rtnval;
    
    DBUG_ENTER ("secure");
    rtnval = TRUE;
    if (s_setuid (info.bru_uid) == SYS_ERROR) {
	bru_message (MSG_SETUID, info.bru_uid);
	if (info.bru_uid != 0) {
	    rtnval = FALSE;
	}
    }
    if (s_setgid (info.bru_gid) == SYS_ERROR) {
	bru_message (MSG_SETGID, info.bru_gid);
	if (info.bru_uid != 0) {
	    rtnval = FALSE;
	}
    }
    DBUG_RETURN (rtnval);
}
