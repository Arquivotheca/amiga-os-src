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

void exit(errcode)
int errcode;
{
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

