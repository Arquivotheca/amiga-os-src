/* -----------------------------------------------------------------------
 * recv.c
 *
 * $Locker:  $
 *
 * $Id: recv.c,v 1.2 91/08/07 13:22:15 bj Exp $
 *
 * $Revision: 1.2 $
 *
 * $Log:	recv.c,v $
 * Revision 1.2  91/08/07  13:22:15  bj
 * rcs header added.
 * 
 *
 * $Header: AS225:src/lib/ss/RCS/recv.c,v 1.2 91/08/07 13:22:15 bj Exp $
 *
 *------------------------------------------------------------------------
 */

/****** socket.library/recv  *************************************************
*
*   NAME
*	recv, recvfrom, recvmsg -- Receive a message from a socket.
*
*   SYNOPSIS
*	#include <sys/types.h>
*	#include <sys/socket.h>
*
*	numbytes = recv( s, buf, len, flags )
*	D0               D0 A0   D1   D2
*
*	numbytes = recvfrom( s, buf, len, flags, from, fromlen )
*	D0                   D0 A0    D1  D2     A1    A2
*
*	numbytes = recvmsg( s, msg, flags )
*	D0                  D0 A0   D1
*
*	int recv(int, char *, int, int )
*	int recvfrom( int, char *, int, int, struct sockaddr *, int *)
*	int recvmsg(int, struct msghdr *, int )
*
*   FUNCTION
*	recv() is used to receive messages on an already connect()ed
*	socket.
*
*	recvfrom() receives data from a socket whether it is in a connected
*	state or not.
*
*	recvmsg() is the most general of the recv calls and is for advanced
*	use.
*
*	If no data is available, these calls block unless the socket is
*	set to nonblocking in which case (-1) is returned with errno set
*	to EWOULDBLOCK.
*
*   INPUTS
*	s       	- a socket descriptor.
*	buf     	- the buffer into which the incoming data will be
*			  placed.
*	len     	- the size of the buffer.
*	flags   	- select options (MSG_OOB, MSG_PEEK).
*	from    	- a pointer to a sockaddr structure.
*	fromlen 	- Length of the 'from' buffer.
*	msg     	- pointer to a struct msghdr, defined in
*			  "sys/socket.h."
*
*   RESULT
*	numbytes 	- number of bytes read if successful else -1.
*
*   NOTES
*	'fromlen' is passed with the length of the 'from' buffer.  If 'from'
*	is non-zero, the structure will be filled with the source address
*	and fromlen will be filled in to represent the size of the actual
*	address stored in 'from'.
*
*   SEE ALSO
*	send(), socket(), connect(), bind(), listen(), accept()
*
********************************************************************
*
*
* recv - receive data from socket
*/

#include "sslib.h"

int __saveds __asm recv (register __d0 int s,
	register __a0 char *buf,
	register __d1 int len,
	register __d2 int flags)
{
	struct recvargs ra ;

	GETSOCK(s, ra.fp) ;
	ra.buf = (caddr_t)buf ;
	ra.len = (long)len ;
	ra.flags = (short)flags ;
	ra.errno = (short)0 ;

	RecvAsm(&ra) ;
	if (ra.errno) {
		*ss_errno = ra.errno;
		return (-1);
	}
	*ss_errno = 0;
	return ((int)ra.rval);
}


/*
** recvfrom - receive data from socket
*/

int __saveds __asm recvfrom (register __d0 int s,
	register __a0 char *buf,
	register __d1 int len,
	register __d2 int flags,
	register __a1 struct sockaddr *from,
	register __a2 int *fromlen)
{
	struct recvfromargs ra ;

	GETSOCK(s, ra.fp) ;
	ra.buf = (caddr_t)buf ;
	ra.len = (long)len ;
	ra.flags = (short)flags ;
	ra.from = (caddr_t)from ;
	ra.fromlen = (short)(fromlen ? *fromlen : 0 ) ;
	ra.errno = (short)0 ;

	RecvFromAsm(&ra) ;
	if (ra.errno) {
		*ss_errno = ra.errno;
		return (-1);
	}
	if(fromlen){
		*fromlen = ra.fromlen ;
	}
	*ss_errno = 0;
	return ((int)ra.rval) ;
}

/*
** recvmsg - receive data from socket
*/

int __saveds __asm recvmsg (register __d0 int s,
	register __a0 struct msghdr *msg,
	register __d1 int flags)
{
	struct recvmsgargs ra;

	GETSOCK(s, ra.fp);
	ra.msg = msg;
	ra.flags = (short)flags;
	ra.errno = (short)0 ;

	RecvMsgAsm(&ra);
	if (ra.errno) {
		*ss_errno = ra.errno;
		return (-1);
	}
	*ss_errno = 0;
	return ((int)ra.rval);
}
