/****** socket/bind ******************************************
*
*   NAME
*		bind -- bind a name to a socket
*
*   SYNOPSIS
*		return = bind (s, name, namelen)
*
*		int bind (int, struct sockaddr *, int); 
*
*   FUNCTION
*		bind() assigns a name to an unnamed socket
*
*	INPUTS
*		s			
*		name
*		namelen
*
*   RESULT
*		0 is returned if successful.  -1 is returned on error.
*		errno will contain the specific error.
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


#include "fd_to_fh.h"
#include <socket.h>

bind(s, name, len)
	register int s;
	caddr_t name;
	int	len;
{
	struct bindargs ba;

	ba.errno = 0L;
	GETSOCK(s, ba.fp);
	ba.name = name;
	ba.namelen = len;
	BindAsm(&ba);
	errno = ba.errno;

	return errno? -1:0;
}

