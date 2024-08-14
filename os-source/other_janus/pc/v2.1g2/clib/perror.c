/****** ljanus.lib/PError ******************************************
*
*   NAME   
*		PError -- Print out an ASCII string for the error code given.
*
*   SYNOPSIS
*		PError(Error);
*
*		VOID PError(UBYTE);
*
*   FUNCTION
*		Prints the ASCII error message associated with the return code.
*
*   INPUTS
*		Error - Error code returned by one of the Service functions.
*
*   RESULT
*		An ASCII error message is printed to stdout.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*		PrintSD()
*
*****************************************************************************
*
*/
#define LINT_ARGS
#include	<janus/janus.h>

VOID PError(error)
UBYTE error;
{

	switch(error)
	{
		case 0:	printf("No error\n"); break;
		case 3:	printf("Not enough Janus memory\n"); break;
		case 4:	printf("Not enough Amiga memory\n"); break;
		case 5:	printf("Not enough PC memory\n"); break;
		case 6:	printf("Service does not exist\n"); break;
		case 7:	printf("Service already exists\n"); break;
		case 8:	printf("Illegal function\n"); break;
		case 9:	printf("No exclusive access\n"); break;
		case 10:	printf("Cannot autoload\n"); break;
		default: printf("Unknown error\n"); break;
	}
}
