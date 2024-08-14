/* -----------------------------------------------------------------------
 * exit.c   for AS225's FTP
 *
 * $Locker:  $
 *
 * $Id: exit.c,v 2.3 92/12/09 16:02:57 bj Exp $
 *
 * $Revision: 2.3 $
 *
 * $Log:	exit.c,v $
 * Revision 2.3  92/12/09  16:02:57  bj
 * Binary 2.8
 * 
 * Backed out of the previous version. Global 'console_fh' not
 * required after all. All such relevant code removed.
 * 
 * Revision 2.2  92/12/08  17:35:47  bj
 * Binary 2.7
 * 
 * Adds the global 'console_fh' so that the exit() routinie
 * can handle the open 'CONSOLE:' (used in getcommand())
 * if necessary.  Code to handle same also added.
 * 
 * Revision 2.1  92/10/30  11:26:31  bj
 * Binary 2.3
 * 
 * No changes to code. Added RCS header.
 * 
 *
 * $Header: AS225:src/c/ftp/RCS/exit.c,v 2.3 92/12/09 16:02:57 bj Exp $
 *
 *------------------------------------------------------------------------
 */
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


#include <dos.h>

void exit(int);
void _exit(int);
void clean_sock(void);
void restore_dir(long);

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
	extern long StartDir;

	if (StartDir) 
		restore_dir(StartDir);

	clean_sock();

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

