/****** unix/exit ******************************************
*
*   NAME
*		exit -- cleanup and exit program
*
*   SYNOPSIS
*		exit( status )
*
*		void exit (int); 
*
*   FUNCTION
*		This function flushes all buffers and closes all files.
*		The BSD return codes are then mapped to AmigaDOS return
*		codes.
*
*	INPUTS
*		status		integer -  mapped as follows
*
*		BSD			AmigaDOS
*		1			RETURN_FAIL (20)
*		0			RETURN_OK	(0)
*		-1  		RETURN_WARN	(5)
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

#ifdef LATTICE

#include <dos.h>
#include <stdio.h>
#include <ios1.h>
#include <fcntl.h>

void exit(int);
void _exit(int);
/**
*
* name         exit -- close buffered output files and exit
*
* synopsis     exit(errcode [,message]);
*              int errcode;    exit error code
*              char *message;  exit error message (optional)
*
* description  This function closes all files and calls _exit.
*              See that function for a description of the
*              parameters.
*
**/
void exit(errcode)
int errcode;
{
register int size;
register FILE *fp;

clean_sock();

for (fp = stdin; fp != NULL; fp = fp->_next)
   {
   if (fp->_flag & _IONBF) continue;
   if ((fp->_flag & _IOWRT) == 0) continue;
   if (size = fp->_ptr - fp->_base)
      write((int)(fp->_file),fp->_base,size);
   }
	switch(errcode){
	case 1:
		errcode = RETURN_ERROR;
		break;

	case 0:
		errcode = RETURN_OK;
		break;

	case -1:
		errcode = RETURN_WARN;
		break;
	}

_exit(errcode);
}

#else

/* Copyright 1989 Manx Software Systems, Inc. All rights reserved */

/*
 *	Synopsis
 *
 *	void exit(int status);
 *
 *
 *	Description
 *
 *		The exit function causes normal program termination to occur. If more
 *	than one call to the exit functions is executed by a program, the behavior
 *	is undefined.
 *
 *		First, all functions registered by the atexit function are called, in
 *	the reverse order of their registration.
 *
 *		Next, all open output streams are flushed, all open streams are closed,
 *	and all files created by the tmpfile function are removed.
 *
 *		Finally, control is returned to the host environment. If the value of
 *	status is zero or EXIT_SUCCESS, an implementation-defined form of the
 *	status "successful termination" is returned. If the value of the status is
 *	EXIT_FAILURE, an implementation-defined form of the status "unsuccessful
 *	termination" is returned. Otherwise the status returned is implementation-
 *	defined.
 *
 *
 *	Returns
 *
 *		The exit function cannot return to its caller.
 */

#include <libraries/dos.h>

typedef void (*PFV)(void);

struct _exfunc {
	struct _exfunc *next;
	PFV func;
};

struct _exfunc *_exit_funcs;

PFV _close_stdio;
extern void _exit(int status);


void
exit(int status)
{
	while (_exit_funcs) {
		(*_exit_funcs->func)();
		_exit_funcs = _exit_funcs->next;
	}

	if (_close_stdio)
		(*_close_stdio)();

	switch(status){
	case 1:
		status = RETURN_ERROR;
		break;

	case 0:
		status = RETURN_OK;
		break;

	case -1:
		status = RETURN_WARN;
		break;
	}

	_exit(status);
}

#endif /* LATTICE */

