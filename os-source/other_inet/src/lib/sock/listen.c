/****** socket/listen ******************************************
*
*   NAME
*		listen -- wait for connections
*
*   SYNOPSIS
*		return = listen( s, backlog)
*
*		int listen (int , int); 
*
*   FUNCTION
*		This function is used on a connection-oriented server (TCP)
*		to indicate it is waiting to receive connections.  It is usually
*		executed after socket() and bind(), and before accept().
*		
*	INPUTS
*		s		- socket descriptor
*		backlog	- max number of connection requests to que (usually 5)
*
*   RESULT
*		0 is returned if successful.  -1 is returned on error.
*		On error, errno will be set to one of the following:
*
*		EBADF		- s is not a valid descriptor
*		ENOTSOCK	- s is not a socket
*		ECONNREFUSED- connection refused, normally because the queue is full			
*		EOPNOTSUPP	- s is a socket type which does not support this 
*						operation.  s must be type SOCK_STREAM.
*
*   BUGS
*		backlog is currently limited to 5
*
*   SEE ALSO
*		accept(), bind(), connect(), socket()
******************************************************************************
*
*/

#include "fd_to_fh.h"
#include <socket.h>

int
listen(register int s, register int backlog)
{
	struct listenargs la;

	la.errno = 0L;
	GETSOCK(s, la.fp);
	la.backlog = backlog;
	ListenAsm(&la);
	errno = la.errno;

	return errno? -1:0;
}
