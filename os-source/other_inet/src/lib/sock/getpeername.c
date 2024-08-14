/****** socket/getpeername ******************************************
*
*   NAME
*		getpeername -- get name of connected peer
*
*   SYNOPSIS
*		return = getpeername( s, name, namelen )
*
*		int getpeername ( int, struct sockaddr *, int * ); 
*
*   FUNCTION
*		Returns the name of the peer connected to a socket.
*
*	INPUTS
*		s		socket descriptor
*		name	pointer to a struct sockaddr
*		namelen	initialized to sizeof(name). On return this contains
*				the actual sizeof(name)
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
*		bind(), socket(), getsockname()
*
******************************************************************************
*
*/


#include "fd_to_fh.h"
#include <socket.h>

int
getpeername(s, name, lenp)
	register int s;
	struct sockaddr *name;
	int	*lenp;
{
	struct getpeernameargs gpa;

	gpa.errno = 0L;
	gpa.asa = name;
	gpa.len = lenp? *lenp:0;
	GETSOCK(s, gpa.fp);
	GetPeerNameAsm(&gpa);
	errno = gpa.errno;
	if(lenp){
		*lenp = gpa.len;
	}

	return errno? -1:0;
}
