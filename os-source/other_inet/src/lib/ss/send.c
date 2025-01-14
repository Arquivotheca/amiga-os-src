/* -----------------------------------------------------------------------
 * send.c
 *
 * $Locker:  $
 *
 * $Id: send.c,v 1.2 91/08/07 13:35:56 bj Exp $
 *
 * $Revision: 1.2 $
 *
 * $Log:	send.c,v $
 * Revision 1.2  91/08/07  13:35:56  bj
 * rcs header added.
 * 
 *
 * $Header: AS225:src/lib/ss/RCS/send.c,v 1.2 91/08/07 13:35:56 bj Exp $
 *
 *------------------------------------------------------------------------
 */
/****** socket.library/send ******************************************
*
*   NAME
*	send, sendto, sendmsg -- Send data from a socket.
*
*   SYNOPSIS
*	#include <sys/types.h>
*	#include <sys/socket.h>
*
*	cc = send( s, buf, len, flags )
*	D0         D0 A0   D1   A1
*
*	cc = sendto ( s, buf, len, flags, to, to_len )
*	D0            D0 A0   D1   D2     A1  D3
*
*	cc = sendmsg( s, msg, flags )
*	D0            D0 A0   D1
*
*	int send (int, char *, int, int );
*	int sendto (int, char *, int, int, struct sockaddr *, int );
*	int sendmsg (int, struct msghdr *, int );
*
*   FUNCTION
*	send(), sendto(), and sendmsg() transmit data from a socket.
*	send() must be used with a connect()ed socket.  sendto() can
*	only be used with non-connect()ed DGRAM-type sockets.  sendmsg()
*	is an advanced function.
*
*   INPUTS
*	s       	- socket descriptor.
*	buf     	- pointer to message buffer.
*	len     	- length of message to transmit.
*	flags   	- 0 or MSG_OOB to send out-of-band data.
*	to      	- pointer to a sockaddr containing the destination.
*	to_len  	- sizeof(struct sockaddr).
*	msg     	- pointer to a struct msghdr, defined in
*			  "sys/socket.h."
*
*   RESULT
*	cc		- the number of bytes sent. This does not imply that
*			  the bytes were recieved.  -1 is returned on a local
*			  error (errno will be set to the specific error
*			  code).  Possible errors are:
*
*		EBADF		an invalid descriptor was used
*		ENOTSOCK	's' is not a socket
*		EMSGSIZE	the socket requires that the message be
*				sent atomically and the size of the message
*				prevents that.
*		EWOULDBLOCK	requested operation would block
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*	recv(), socket()
*
******************************************************************************
*
*/

#include "sslib.h"

int __saveds __asm send(register __d0 int s,
	register __a0 char *buf,
	register __d1 int len,
	register __a1 int flags)
{
	struct sendargs sa;

	GETSOCK(s, sa.fp);
	sa.buf = (caddr_t)buf;
	sa.len = (long)len;
	sa.flags = (short)flags;
	sa.errno = 0 ;
	SendAsm(&sa);

	*ss_errno = sa.errno;
	if (sa.errno) {
		return (-1);
	}
	return((int)sa.rval) ;

}


int __saveds __asm sendto(register __d0 int s,
	register __a0 char * buf,
	register __d1 int len,
	register __d2 int flags,
	register __a1 struct sockaddr *to,
	register __d3 int tolen)
{
	struct sendtoargs sa;

	GETSOCK(s, sa.fp);
	sa.buf = (caddr_t)buf;
	sa.len = (long)len;
	sa.flags = (short)flags;
	sa.tolen = (short)tolen;
	sa.to = (caddr_t)to;
	sa.errno = (short)0;

	SendToAsm(&sa);
	*ss_errno = sa.errno;
	if (sa.errno) {
		return (-1);
	}
	return ((int)sa.rval);
}

/* -------------------------------------------- */

int __saveds __asm sendmsg(register __d0 s,
	register __a0 struct msghdr *msg,
	register __d1 int flags)
{
	struct sendmsgargs sa;

	GETSOCK(s, sa.fp);
	sa.msg = (caddr_t)msg;
	sa.flags = (short)flags;
	sa.errno = (short)0;

	SendMsgAsm(&sa);
	*ss_errno = sa.errno;
	if (sa.errno) {
		return (-1);
	}
	return((int)sa.rval) ;
}
