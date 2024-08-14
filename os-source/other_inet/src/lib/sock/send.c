/****** socket/send ******************************************
*
*   NAME
*		send, sendto, sendmsg -- send data from a socket
*
*   SYNOPSIS
*		#include <sys/types.h>
*		#include <sys/socket.h>
*
*		cc = send   ( s, buf, len, flags)
*		cc = sendto ( s, buf, len, flags, to, to_len)
*		cc = sendmsg( s, msg, flags)
*
*		int send (int, char *, int, int);
*		int sendto (int, char *, int, int, struct sockaddr *, int);
*		int sendmsg (int, struct msghdr *, int); 
*
*   FUNCTION
*		send(), sendto(), and sendmsg() transmit data from a socket.
*		send() must be used with a connected socket because it has no
*		destination parameter.  
*
*	INPUTS
*		s		- socket descriptor
*		buf		- pointer to message buffer
*		len		- length of message to transmit
*		flags	- 0 or MSG_OOB to send out-of-band data
*		to		- pointer to a sockaddr containing the destination
*		to_len	- sizeof(struct sockaddr)
*		msg		- pointer to a struct msghdr, defined in sys/socket.h				
*
*   RESULT
*		Returns the number of bytes sent. This does not imply the bytes
*		were recieved.  -1 is returned on a local error.
*		errno will be set to the specific error code. Possible errors are
*
*		EBADF		  an invalid descriptor was used
*
*		ENOTSOCK	  's' is not a socket
*
*		EMSGSIZE	  the socket requires that the message be sent
*		atomically and the size of the message prevents that.
*
*		EWOULDBLOCK   requested operation would block	
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*		recv(), socket()
*
******************************************************************************
*
*/

#include "fd_to_fh.h"
#include <socket.h>

int
send(s, buf, len, flags)
	int	s;
	char *buf;
	int	len;
	int flags;
{
	struct sendargs sa;

	GETSOCK(s, sa.fp);
	sa.buf = buf;
	sa.len = len;
	sa.flags = flags;
	sa.errno = 0l;
	SendAsm(&sa);
	errno = sa.errno;

	return errno? -1:(int)sa.rval;
}
