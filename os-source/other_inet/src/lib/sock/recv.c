
/****** socket/recv  *************************************************
*
*	NAME
*		recv, recvfrom, recvmsg - receive a message from a socket
*
*	SYNOPSIS
*		#include <sys/types.h>
*		#include <sys/socket.h>
*
*		numbytes = recv( s, buf, len, flags )
*		numbytes = recvfrom(s, buf, len, flags, from, fromlen )
*		numbytes = recvmsg(s, msg, flags)
*
*		int recv(int, caddr_t, int, int)
*       int recvfrom( int, caddr_t, int, int, struct sockaddr *, int* )
*		int recvmsg(int, struct msghdr *, int)
*
*	FUNCTION
*		This function is used to receive messages on an already connected
*		socket. 
*
*		Recvfrom receives data from a socket whether it is in a connected
*		state or not.
*
*	INPUTS
*		s 		- a socket descriptor
*		buf     - the buffer into which the incoming data will be placed
*		len     - the size of the buffer
*		flags   - select options (MSG_OOB, MSG_PEEK)
*		from    - a pointer to a sockaddr structure
*		fromlen - Length of the 'from' buffer.
*		msg		- pointer to a struct msghdr, defined in sys/socket.h
*
*	RESULT
*		numbytes - number of bytes read if successful or (-1) upon failure.
*
*	NOTES
*		fromlen is passed with the length of the 'from' buffer.  If 'from' 
*		is non-zero, the structure will be filled with the source address
*		and fromlen will be filled in to represent the size of the actual
*		address stored in 'from'.
*
*   
********************************************************************
*
*
* recv - receive data from socket
*/

#include "fd_to_fh.h"
#include <socket.h>

recv(int s, caddr_t buf, int len, int flags)
{
	struct recvargs ra ;

	GETSOCK(s, ra.fp) ;
	ra.buf = buf ;
	ra.len = len ;
	ra.flags = flags ;
	ra.errno = 0L ;
	
	RecvAsm(&ra) ;
	errno = ra.errno ;

	return errno? -1 : (int)ra.rval ;
}


/*
** recvfrom - receive data from socket
*/

recvfrom(int s, caddr_t buf, int len, int flags, caddr_t from, int *fromlen)
{
	struct recvfromargs ra ;

	GETSOCK(s, ra.fp) ;
	ra.buf = buf ;
	ra.len = len ;
	ra.flags = flags ;
	ra.from = from ;
	ra.fromlen = fromlen? *fromlen:0 ;
	ra.errno = 0L ;
	
	RecvFromAsm(&ra) ;
	errno = ra.errno ;
	if(fromlen){
		*fromlen = ra.fromlen ;
	}

	return errno ? -1 : (int)ra.rval ;
}

/*
** recvmsg - receive data from socket
*/

recvmsg(s, msg, flags)
	int	s;
	caddr_t	msg;
	int	flags;
{
	struct recvmsgargs ra;

	GETSOCK(s, ra.fp);
	ra.msg = (struct msghdr *)msg;
	ra.flags = flags;
	ra.errno = 0L;
	
	RecvMsgAsm(&ra);
	errno = ra.errno;

	return errno? -1:(int)ra.rval;
}
