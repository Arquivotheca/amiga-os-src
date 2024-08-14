/****** socket/connect ******************************************
*
*   NAME
*		connect -- connect to socket
*
*   SYNOPSIS
*		return = connect( s, name, namelen)
*
*		int connect (int, struct sockaddr *, int); 
*
*   FUNCTION
*		*
*	INPUTS
*		s		socket
*
*		name	
*
*		namelen	
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
** connect(s, name, len)	- connect name to given socket.
*/

#include "fd_to_fh.h"
#include <socket.h>

connect(s, name, len)
	register int s;
	struct sockaddr *name;
	int	len;
{
	struct connectargs ca;

	ca.errno = 0L;
	GETSOCK(s, ca.fp);
	ca.name = name;
	ca.namelen = len;
	ConnectAsm(&ca);
	errno = ca.errno;

	return errno? -1:0;
}
