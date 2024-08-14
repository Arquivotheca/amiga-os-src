
/****** socket/perror  *************************************************
*
*	NAME
*		perror - print to stderr the string associated with the current
*		value of the global int 'errno'.
*
*	SYNOPSIS
*		perror( error_message )
*
*		void perror( const char * )		(Manx version)
*		int perror ( char * )			(SAS version) 
*
*	FUNCTION
*
*		The perror subroutine produces a short error message on the
*		standard error file describing the last error encountered
*		during a call to the system. First the argument string s is
*		printed, then a colon, then the message and a new line. 
*
*	INPUTS
*		
*		error_message - a string normally used to identify the source of
*		the error. Often the file name is used.
*
*	RESULT
*
*		The SAS version always return 0.
*   	
********************************************************************
*
*
*/

#include <stdio.h>
#include <errno.h>

#ifdef AZTEC_C
void perror( const char *s )
#else
int perror(char *s)
#endif
{
	if (s)
		fprintf (stderr, "%s: ", s);

	if (errno < 0 || errno > sys_nerr)
		fprintf (stderr, "unknown error\n");
	else
		fprintf (stderr, "%s\n", sys_errlist[errno]);
#ifdef LATTICE
	return 0;
#endif
}


/* =========================================================
 * internal function 
 * =========================================================
 */
 
char *
strerror(num)
int num;
{
	static char buf[32];

	if (errno < 0 || errno > sys_nerr){
		sprintf(buf, "Unknown error %d", num);
		return buf;
	}

	return sys_errlist[num];
}
