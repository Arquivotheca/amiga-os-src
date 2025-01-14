/* -----------------------------------------------------------------------
 * functions.c
 *
 * $Locker:  $
 *
 * $Id: functions.c,v 1.3 92/07/21 15:59:19 bj Exp $
 *
 * $Revision: 1.3 $
 *
 * $Log:	functions.c,v $
 * Revision 1.3  92/07/21  15:59:19  bj
 * Added code to handle reserved filehandle numbers for the
 * s_socket() and s_accept() calls. This is bookkeeping for stdio
 * users.
 * 
 * V4.0 and greater.  AS225 R2
 * 
 * Revision 1.2  91/08/07  13:31:16  bj
 * rcs header added.
 * 
 *
 * $Header: AS225:src/lib/ss/RCS/functions.c,v 1.3 92/07/21 15:59:19 bj Exp $
 *
 *------------------------------------------------------------------------
 */

/* accept, bind, connect, listen, shutdown, setsockopt, getsockopt */
/* getpeername getsockname */

/****** socket.library/accept ******************************************
*
*   NAME
*	accept -- Accept new connection from socket.
*
*   SYNOPSIS
*	ns = accept( s, name, len )
*	D0           D0 A0    A1
*
*	int accept(int, struct sockaddr *, int *);
*
*   FUNCTION
*	Servers using TCP (or another connection-oriented protocol) accept()
*	connections initiated by a client program.  The connections are
*	generally accept()ed on a socket which on which bind() and listen()
*	have been executed.  Unless there is an error, accept() will return
*	a new socket which is connected to the client.  The server can then
*	use the new socket ('ns') to communicate with the client (via send()
*	and recv()) and still accept() new connections on the old socket
*	('s').
*
*	'Len' should be initialized to the amount of space pointed
*	to by 'name.'  The actual size of 'name' will be returned in
*	'namelen' and 'name' will contain the name of the socket initiating
*	the connect().  This saves the server from needing to do a
*	getpeername() for new connections.
*
*	Accept() generally blocks until a client attempts a connect().
*	You may use select() to determine whether a connection is pending,
*	or use a non-blocking socket (see setsockopts()).
*
*   INPUTS
*	s		- socket descriptor.
*	name		- name of the peer socket.
*	len		- pointer to the length of the sockaddr struct.
*			  The value returned will be the actual size of the
*			  sockaddr struct that was returned.
*
*   RESULT
*	ns		- new socket descriptor or -1 (errno will be set to
*			  reflect exact error condition).
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*	socket(), bind(), listen()
*
******************************************************************************
*
*/


/*
** accept(s, name, lenp) - accept new connection from socket
*/

#include "sslib.h"

int __saveds __asm accept(register __d0 int s,
	register __a0 struct sockaddr *name,
	register __a1 int *lenp)
{
	struct acceptargs aa;
	int ns=0;
	extern u_long reserved;
	extern int rsize;

	GETSOCK(s, aa.fp);

	/* find an empty space in the socket array */
	while(socks[ns] || (reserved && *(int *)(reserved+rsize*ns)) ) {
		ns++;
		if(ns>=MAXSOCKS) {
			*ss_errno = EMFILE;
			return (-1);
		}
	}

	aa.errno = (short)0;
	aa.rval = (void *)0;
	aa.namelen = (short)(lenp ? *lenp : 0) ;
	aa.name = (caddr_t)name;
	AcceptAsm(&aa);
	if(aa.errno){
		*ss_errno = aa.errno;
		s_close(ns);
		return -1;
	}
	*ss_errno = 0;

	socks[ns] = (long)aa.rval;
	if(lenp){
		*lenp = aa.namelen;
	}

	return ns;
}


/****** socket.library/bind ******************************************
*
*   NAME
*	bind -- Bind a name to a socket.
*
*   SYNOPSIS
*	return = bind( s, name, namelen )
*	D0             D0 A1    D1
*
*	int bind(int, struct sockaddr *, int);
*
*   FUNCTION
*	Assigns a name to an unnamed socket.
*
*	Connection-oriented servers generally bind() a socket to a well-
*	known address and listen() at that socket, accept()ing connections
*	from clients who know the address.
*
*	Connectionless servers generally bind() a socket to a well-known
*	address and recvfrom() that socket.
*
*	Most servers should build 'name' from a well-known port obtained
*	from getservbyname().  Hard-coded ports are often used in prototype
*	servers, but should never be used in production code.  For more
*	information on port numbering, see your favorite TCP/IP refrence.
*
*   INPUTS
*	s		- socket descriptor.
*	name		- address to bind to socket 's.'
*	namelen		- length (in bytes) of 'name.'
*
*   RESULT
*	return		- 0 if successful else -1 (errno will contain the
*			  specific error).
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*	socket(), listen(), accept()
*
******************************************************************************
*
*/



int __saveds __asm bind(register __d0 int s,
	register __a1 struct sockaddr *name,
	register __d1 int len)
{
	struct bindargs ba;

	ba.errno = (short)0;
	GETSOCK(s, ba.fp);
	ba.name = (caddr_t)name;
	ba.namelen = (short)len;
	BindAsm(&ba);
	if(ba.errno) {
		*ss_errno = ba.errno;
		return (-1);
	}
	*ss_errno = 0;
	return 0;
}


/****** socket.library/connect ******************************************
*
*   NAME
*	connect -- Connect to socket.
*
*   SYNOPSIS
*	return = connect( s, name, namelen )
*	D0                D0  A1     D1
*
*	int connect(int, struct sockaddr *, int);
*
*   FUNCTION
*	To communicate with a server using a connection-oriented protocol
*	(i.e. TCP), the client must connect() a socket obtained by the
*	client (from socket()) to the server's well-known address.  (The
*	server will receive (from accept()) a new socket which is connected
*	to the socket which the client is connect()ing to the server.)
*
*	Clients of a connectionless server (i.e. one using UDP) can use
*	connect() and send() rather than always using sendto().  In this
*	case, no actual connection is created, the address to send to is
*	just stored with the socket.
*
*	Most clients should build 'name' from a well-known port obtained from
*	getservbyname().  Hard-coded ports are often used in prototype
*	clients, but should never be used in production code.  For more
*	information on port numbering, see your favorite TCP/IP refrence.
*
*   INPUTS
*	s		- socket descriptor.
*	name		- address of socket to connect 's' to.
*	namelen		- length of 'name.'
*
*   RESULT
*	return		- 0 if successful else -1 (errno will contain the
*			  specific error).
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*	socket(), bind(), listen(), accept()
*
******************************************************************************
*
*/


int __saveds __asm connect(register __d0 int s,
	register __a1 struct sockaddr *name,
	register __d1 int len)
{
	struct connectargs ca;

	ca.errno = (short)0 ;
	GETSOCK(s, ca.fp);
	ca.name = (caddr_t)name;
	ca.namelen = (short)len;
	ConnectAsm(&ca);
	if (ca.errno) {
		*ss_errno = ca.errno;
		return (-1);
	}
	*ss_errno = 0;
	return 0;
}

/****** socket.library/listen ******************************************
*
*   NAME
*	listen -- Indicate willingness to accept() connections.
*
*   SYNOPSIS
*	return = listen( s, backlog )
*	D0		 D0 D1
*
*	int listen(int, int);
*
*   FUNCTION
*	This function is used for a connection-oriented server (i.e. one
*	using the TCP protocol)	to indicate that it is waiting to receive
*	connections.  It is usually executed after socket() and bind(), and
*	before accept().
*
*   INPUTS
*	s		- socket descriptor.
*	backlog		- max number of connection requests to queue
*			  (usually 5).
*
*   RESULT
*	return		- 0 is returned if successful, else  -1. On error,
*			  errno will be set to one of the following:
*
*	EBADF		- s is not a valid descriptor
*	ENOTSOCK	- s is not a socket
*	ECONNREFUSED	- connection refused, normally because the queue
*			  is full
*	EOPNOTSUPP	- s is a socket type which does not support this
*			  operation.  s must be type SOCK_STREAM.
*
*   BUGS
*	'backlog' is currently limited to 5.
*
*   SEE ALSO
*	accept(), bind(), connect(), socket()
*
******************************************************************************
*
*/


int __saveds __asm listen(register __d0 int s,
	register __d1 int backlog)
{
	struct listenargs la;

	la.errno = (short)0;
	GETSOCK(s, la.fp);
	la.backlog = (short)backlog;
	ListenAsm(&la);
	if (la.errno) {
		*ss_errno = la.errno;
		return (-1);
	}
	*ss_errno = 0;
	return 0;
}

/****** socket.library/shutdown ******************************************
*
*   NAME
*	shutdown -- Shut down part of a full-duplex connection.
*
*   SYNOPSIS
*	return = shutdown( s, how )
*	D0                 D0 D1
*
*	int shutdown(int, int);
*
*   FUNCTION
*	Sockets are normally terminated by using just s_close().  However,
*	s_close() will attempt to deliver any data that is still pending.
*	Further, shutdown() provides more control over how a connection is
*	terminated.  You should eventually use s_close() on all sockets you
*	own, regardless of what shutdown() is done on those sockets.
*
*   INPUTS
*	s		- socket descriptor.
*	how		- 'how' can be one of the following:
*			  0	disallow further receives
*			  1	disallow further sends
*			  2	disallow further sends and receives
*
*   RESULT
*	return		- 0 if successful else -1 (errno will contain the
*			  specific error).
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*   	s_close()
*
******************************************************************************
*
*/


int __saveds __asm shutdown(register __d0 int s,
	register __d1 int how)
{
	struct shutdownargs sa;

	GETSOCK(s, sa.fp);
	sa.how = (short)how;
	ShutdownAsm(&sa);
	if (sa.errno) {
		*ss_errno = sa.errno;
		return (-1);
	}
	*ss_errno = 0;
	return 0;
}


/****** socket.library/setsockopt ******************************************
*
*   NAME
*	setsockopt -- Set socket options.
*
*   SYNOPSIS
*	return = setsockopt( s, level, optname, optval, optlen )
*	D0                   D0 D1     D2       A0      D3
*
*	int setsockopt( int, int, int, char *, int );
*
*   FUNCTION
*	Sets the option specified by 'optname' for socket 's.'
*	This is an advanced function.  See the "sys/socket.h" header and
*	your favorite TCP/IP reference for more information on options.
*
*   INPUTS
*	s		- socket descriptor.
*	level		- protocol level.  Valid levels are:
*			   IPPROTO_IP	IP options
*			   IPPROTO_TCP	TCP options
*			   SOL_SOCKET	socket options
*	optname		- option name.
*	optval		- pointer to the buffer with the new value.
*	optlen		- size of 'optval' (in bytes).
*
*   RESULT
*	return		- 0 if successful else -1 (errno will contain the
*			  specific error).
*
*   EXAMPLE
*		int on = 1;
*		setsockopt( s, SOL_SOCKET, SO_DEBUG, &on, (int)sizeof(on));
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*	getsockopt()
*
******************************************************************************
*
*/



int __saveds __asm setsockopt(register __d0 int s,
	register __d1 int level,
	register __d2 int optname,
	register __a0 char *optval,
	register __d3 int optlen)
{
	struct setsockoptargs ssa;

	GETSOCK(s, ssa.fp);
	ssa.errno = (short)0;
	ssa.level = (short)level;
	ssa.name = (short)optname;
	ssa.valsize = (short)optlen;
	ssa.val = (caddr_t)optval;
	SetSockOptAsm(&ssa);
	if (ssa.errno) {
		*ss_errno = ssa.errno;
		return (-1);
	}
	*ss_errno = 0;
	return 0;
}


/****** socket.library/getsockopt ******************************************
*
*   NAME
*	getsockopt -- Get socket options.
*
*   SYNOPSIS
*	return = getsockopt( s, level, optname, optval, optlenp )
*	D0                   D0 D1     D2       A0      A1
*
*	int getsockopt( int, int, int, char *, int * );
*
*   FUNCTION
*	Gets the option specified by 'optname' for socket 's.'
*	This is an advanced function.  See the "sys/socket.h" header and
*	your favorite TCP/IP reference for more information on options.
*
*   INPUTS
*	s		- socket descriptor.
*	level		- protocol level.  Valid levels are:
*			  IPPROTO_IP               IP options
*			  IPPROTO_TCP              TCP options
*			  SOL_SOCKET               socket options
*	optname		- option name.
*	optval		- pointer to the buffer which will contain the
*			  answer.
*	optlen		- initially sizeof(optval). reset to new value
*			  on return
*
*   RESULT
*	return		- 0 if successful else -1 (errno will contain the
*			  specific error).
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*	setsockopt()
*
******************************************************************************
*
*/

int __saveds __asm getsockopt(register __d0 int s,
	register __d1 int level,
	register __d2 int optname,
	register __a0 char *optval,
	register __a1 int *optlenp)

{
	struct getsockoptargs gsa;

	GETSOCK(s, gsa.fp);
	gsa.errno = 0;
	gsa.level = (short)level;
	gsa.name = (short)optname;
	gsa.val = (caddr_t)optval;
	gsa.valsize = (short)(optlenp? *optlenp:0);
	GetSockOptAsm(&gsa);
	if (gsa.errno) {
		*ss_errno = gsa.errno;
		return (-1);
	}
	*optlenp = gsa.valsize;
	*ss_errno = 0;
	return 0;
}


/****** socket.library/getpeername ******************************************
*
*   NAME
*	getpeername -- Get name of connected peer.
*
*   SYNOPSIS
*	return = getpeername( s, name, namelen )
*	D0                    D0 A0    A1
*
*	int getpeername( int, struct sockaddr *, int * );
*
*   FUNCTION
*	For every connected socket, there is a socket at the other end
*	of the connection.  To determine the address of the socket at the
*	other end of a connection, pass getpeername() the socket on your
*	end.  'Namelen' should be initialized to the amount of space pointed
*	to by 'name.'  The actual size of 'name' will be returned in
*	'namelen.'
*
*   INPUTS
*	s		- socket descriptor.
*	name		- pointer to a struct sockaddr.
*	namelen		- initialized to size of 'name.' On return this
*			  contains the actual sizeof(name)
*
*   RESULT
*	return		- 0 if successful else -1 (errno will contain the
*			  specific error).
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*	bind(), accept(), getsockname()
*
******************************************************************************
*
*/



int __saveds __asm getpeername( register __d0 int s,
	register __a0 struct sockaddr *name,
	register __a1 int *lenp)
{
	struct getpeernameargs gpa;

	gpa.errno = (short)0;
	gpa.asa = (caddr_t)name;
	gpa.len = (short)(lenp ? (short)*lenp : 0) ;
	GETSOCK(s, gpa.fp);
	GetPeerNameAsm(&gpa);
	if (gpa.errno) {
		*ss_errno = (int)gpa.errno;
		return (-1);
	}
	*ss_errno = 0;
	return 0;
}


/****** socket.library/getsockname ******************************************
*
*   NAME
*	getsockname -- Get the name of a socket.
*
*   SYNOPSIS
*	return = getsockname( s, name, namelen )
*	D0                    D0 A0    A1
*
*	int getsockname( int, struct sockaddr *, int * );
*
*   FUNCTION
*	Returns the name (address) of the specified socket.  'Namelen'
*	should be initialized to the amount of space pointed to by 'name.'
*	The actual size of 'name' will be returned in 'namelen.'
*
*   INPUTS
*	s		- socket descriptor.
*	name		- socket name.
*	namelen		- size of 'name' (in bytes).
*
*   RESULT
*	return		- 0 if successful else -1 (errno will be set to
*			  one of the following error codes:
*	 		  EBADF		invalid socket
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*	bind(), getpeername()
*
******************************************************************************
*
*/


int __saveds __asm getsockname( register __d0 int s,
	register __a0 struct sockaddr *name,
	register __a1 int *lenp)
{
	struct getsocknameargs gsa;

	gsa.errno = 0;
	GETSOCK(s, gsa.fp);
	gsa.asa = (caddr_t)name;
	gsa.len = (short)(lenp ? *lenp : 0);
	GetSockNameAsm(&gsa);
	if(lenp){
		*lenp = gsa.len;
	}
	if (gsa.errno) {
		*ss_errno = (int)gsa.errno;
		return (-1);
	}
	*ss_errno = 0;
	return 0;
}
