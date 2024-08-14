/****** socket/accept ******************************************
*
*   NAME
*		accept -- accept new connection from socket
*
*   SYNOPSIS
*		ns = accept( s, name, len)
*
*		int accept (int, struct sockaddr *, int *); 
*
*   FUNCTION
*		After a server executes the listen() call, a connection from 
*		some client process is waited for by having the server execute
*		the accept() call.  
*
*	INPUTS
*		s		socket descriptor
*
*		name	pointer to the sockaddr struct that is returned
*
*		len		pointer to the length of the sockaddr struct
*				the value returned will be the actual size of the
*				sockaddr struct that was returned
*
*
*   RESULT
*		ns		new socket descriptor or -1 on error
*
*   EXAMPLE
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


/*
** accept(s, name, lenp) - accept new connection from socket
*/

#include "fd_to_fh.h"
#include <socket.h>

accept(s, name, lenp)
	register int s;
	struct sockaddr	*name;
	int	*lenp;
{
	struct acceptargs aa;
	int ns;

	GETSOCK(s, aa.fp);
	if((ns = open("nil:", 2)) < 0){
		return -1;
	}

	aa.errno = 0;
	aa.rval = 0;
	aa.namelen = lenp? *lenp:0;
	aa.name = name;
	AcceptAsm(&aa);
	errno = aa.errno;
	if(aa.errno){
		close(ns);
		return -1;
	}

	PUTSOCK(ns, aa.rval);
	if(lenp){
		*lenp = aa.namelen;
	}

	return ns;
}
