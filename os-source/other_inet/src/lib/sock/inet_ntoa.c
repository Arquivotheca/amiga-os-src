/****** socket/inet_ntoa ******************************************
*
*   NAME
*		inet_ntoa -- turn internet address into string
*
*   SYNOPSIS
*		#include <sys/types.h>
*		#include <sys/socket.h>
*		#include <netinet/in.h>
*		#include <arpa/inet.h>
*
*		string = inet_ntoa( in )
*
*		char *inet_addr ( struct in_addr ); 
*
*   FUNCTION
*		Converts an internet address to s string.
*
*	INPUTS
*
*   RESULT
*		pointer to a string
*
*   EXAMPLE
*
*   NOTES
*		The result points to a static buffer that is overwritten
*		with each call.
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/


/*
** inet_ntoa - convert internet address to dot (d.d.d.d) format.
*/

char *
inet_ntoa(l)
	register unsigned long	l;
{
	unsigned int a, b, c, d;
	static char buf[32];

	d = l & 0xff; l >>= 8;
	c = l & 0xff; l >>= 8;
	b = l & 0xff; l >>= 8;
	a = l & 0xff;

	sprintf(buf, "%d.%d.%d.%d", a, b, c, d);
				    
	return buf;
}

