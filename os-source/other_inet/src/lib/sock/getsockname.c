/****** socket/getsockname ******************************************
*
*   NAME
*		getsockname -- get the name of a socket
*
*   SYNOPSIS
*		return = getsockname( s, name, namelen )
*
*		int getsockname ( int, struct sockaddr *, int * ); 
*
*   FUNCTION
*		Returns the name of the specified socket.  Namelen should be
*		initialized to the amount of space pointed to by name.  The
*		actual size of name will be returned.
*
*	INPUTS
*		s		socket descriptor
*		name	socket name
*		namelen	sizeof(name)
*
*   RESULT
*		0 is returned if successful.  -1 is returned on error.
*		errno will be set to the specific error code.
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
** getsockname(s, name, lenp)	- get socket name
*/

#include "fd_to_fh.h"
#include <socket.h>

getsockname(s, name, lenp)
	register int s;
	caddr_t name;
	int	*lenp;
{
	struct getsocknameargs gsa;

	gsa.errno = 0L;
	GETSOCK(s, gsa.fp);
	gsa.asa = name;
	gsa.len = lenp? *lenp:0;
	GetSockNameAsm(&gsa);
	errno = (int)gsa.errno;
	if(lenp){
		*lenp = gsa.len;
	}

	return errno? -1:0;
}
