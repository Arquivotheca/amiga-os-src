/*
 * socket(af, type, proto) 	- get a socket
 * close(s)			- close a socket
 * read(s, buf, len)		- read from socket/file
 * write(s, buf, len)		- write to socket/file
 * select(s, rs, ws, es, tvp)	- check sockets for buffer activity
 * selectwait()			- check sockets for buf activity or user mask
 */

#define SOCKET 1

#include <sys/types.h>
#include <sys/time.h>
#include <exec/types.h>
#include <devices/timer.h>

#include "//lib/net/fd_to_fh.h"
#include <socket.h>

#include <libraries/dos.h>

/*
#ifdef LATTICE
#include <proto/exec.h>
#endif
*/


struct InetBase *InetBase;
static char inet_lib[] = "inet:libs/inet.library";
static int select_timerbit, setup;
static struct timerequest timereq;
/* static short Timer_Used = 0; */
int socket_sigio = -1, socket_sigurg = -1, num_socket = 0;
void * _socks[FD_SETSIZE];
fd_set _is_socket;

extern struct Library *OpenLibrary();
extern struct MsgPort *CreatePort();
extern struct Task *FindTask();

static void
cleanup_sockets()
{
	if(!setup){
		return;
	}
	setup = 0;

	/*
	 * Aborting timers is risky business.  CheckIO() falsely returns "busy"
	 * if we've never launched an I/O operation, so we AbortIO() and
	 * conditionally WaitIO() if the operation actually aborted.
	 * I'm not satisified with the empirical dinking around which resulted
	 * in this sequence.  Better doc needed.
	 */
	if(!CheckIO((struct IORequest *)&timereq)){
		if(!AbortIO((struct IORequest *)&timereq)){
			WaitIO((struct IOrequest *)&timereq);
		}
	}
/*
	if(Timer_Used) {
		AbortIO((struct IORequest *)&timereq);
		WaitIO((struct IORequest *)&timereq);
	}
*/
	CloseDevice((struct IORequest *)&timereq);
	DeletePort(timereq.tr_node.io_Message.mn_ReplyPort);

	bzero(&timereq, sizeof(timereq));

	CloseLibrary(InetBase);
	InetBase = 0;

	FreeSignal(socket_sigurg);
	FreeSignal(socket_sigio);
	socket_sigurg = -1;
	socket_sigio = -1;
	select_timerbit = -1;

	FD_ZERO(&_is_socket);
	bzero(_socks, sizeof(_socks));
}

static 
setup_sockets()
{
	if(setup){
		return 0;
	}

	FD_ZERO(&_is_socket);
	bzero(_socks, sizeof(_socks));

	InetBase = (struct InetBase *)OpenLibrary(inet_lib, 0L);
	if(!InetBase){
		errno = ECONFIGPROBLEM;
		return -1;
	}

	bzero(&timereq, sizeof(timereq));
	timereq.tr_node.io_Message.mn_ReplyPort = CreatePort(0L, 0L);
	if(OpenDevice(TIMERNAME, UNIT_VBLANK, (struct IORequest *)&timereq, 0L)){
		CloseLibrary(InetBase);	
		InetBase = 0;
		DeletePort(timereq.tr_node.io_Message.mn_ReplyPort);
		return -1;
	}

	socket_sigurg = AllocSignal(-1L);
	socket_sigio = AllocSignal(-1L);
	select_timerbit = timereq.tr_node.io_Message.mn_ReplyPort->mp_SigBit;

	num_socket = 0;
	setup++;

	return 0;
}

/****** socket/socket *************************************************
*
*	NAME
*		socket - create an endpoint for communication.
*
*	SYNOPSIS
*		s = socket( family, type, protocol )
*
*		int socket( int, int, int )
*
*	FUNCTION
*		socket() returns a small integer representing a socket
*		descriptor. 
*
*
*	INPUTS
*		family   - This specifies an address format with which
*		           addresses specified in later operations using
*		           socket should be interpreted.
*		type     - Specifies the semantice of communication.
*		protocol - Specifies a particular protocol to be used with the
*		           socket.
*
*	RESULT
*		s        - Returns a (-1) upon failure ; a socket descriptor
*		           upon success. 
*
*   
********************************************************************
*
*
*/

socket( af, type, pf)
int af, type, pf;
{
	struct sockargs sa;
	int ns;

	if(!inet_up() && !start_network()){
		errno = ENETDOWN;
		return -1;
	}

	if(setup_sockets() < 0){
		return -1;
	}

	/*
	 * We open a real filehandle here to accompany the socket in
	 * case the application manages to manuever around all the checks
	 * engineered to prevent launching a DOS packet.
	 */
	if((ns = open("nil:", 2)) < 0){
		goto bad;
	}

	sa.errno = 0;
	sa.domain = af;
	sa.type = type;
	sa.protocol = pf;
	sa.sigurg = socket_sigurg;
	sa.sigio = socket_sigio;

	SocketAsm(&sa);
	if(errno = sa.errno){
		int saveerr = errno;
		close(ns);
		errno = saveerr;
		goto bad;
	}

	PUTSOCK(ns, sa.rval);
	return ns;

bad:	if(num_socket == 0){
		cleanup_sockets();
	}
	return -1;
}

/*
 * select/selectwait - selects are in general very expensive operations in
 * both Unix and AmigaDOS.  The user should really use FIOASYNC rather
 * than relying on select.
*/


int
select(numfds, readfds, writefds, exceptfds, tvp)
	int numfds;
	fd_set	*readfds, *writefds, *exceptfds;
	struct timeval *tvp;
{
	long	mask = SIGBREAKF_CTRL_C;
	return selectwait(numfds, readfds, writefds, exceptfds, tvp, &mask);
}

/*
 * selectwait() - examine specified buffers for data.  Returns > 0 if some
 * of the buffers have data/space available, == 0 if we timed out, and < 0
 * if user event mask occurred.
*/
selectwait(numfds, readfds, writefds, exceptfds, tvp, umask)
	int numfds;
	register fd_set	*readfds, *writefds, *exceptfds;
	struct timeval *tvp;
	long *umask;
{
	fd_set rd_fd, wr_fd, ex_fd, save_rd, save_wr, save_ex;
	long timerevent, event, usermask, selectmask;
	int returnval, selectbit;
	struct selectargs sa;
	register int fd;

	selectbit = AllocSignal(-1L);
	if(selectbit == -1){
		selectbit = socket_sigio;
	}
	selectmask = 1L << selectbit;
	timerevent = 0L;
	usermask = umask? *umask : 0L;

	if(tvp){
		timerevent = (1L << select_timerbit);
		if(!CheckIO((struct IORequest *)&timereq)){
			if(!AbortIO((struct IORequest *)&timereq)){
				WaitIO((struct IORequest *)&timereq);
				SetSignal(0,timerevent);
			}
		}

		timereq.tr_node.io_Command = TR_ADDREQUEST;
		timereq.tr_time = *tvp;
		SendIO((struct IORequest *)&timereq);
/*		Timer_Used++; */
	}

	sa.numfds = min(numfds, NUMFD);
	sa.numfds = min(sa.numfds, FD_SETSIZE);
	sa.socks = _socks;
	sa.rd_set = readfds? &rd_fd : 0;
	sa.wr_set = writefds? &wr_fd : 0;
	sa.ex_set = exceptfds? &ex_fd : 0;
	sa.task = FindTask(0L);
	sa.sigbit = selectbit;

	returnval = 0; 

	for(fd = 0; fd < (sa.numfds+NFDBITS-1)>>NFDPOW; fd++){
		if(readfds){
			save_rd.fds_bits[fd] = readfds->fds_bits[fd];
		}
		if(writefds){
			save_wr.fds_bits[fd] = writefds->fds_bits[fd];
		}
		if(exceptfds){
			save_ex.fds_bits[fd] = exceptfds->fds_bits[fd];
		}
	}

	do {
		event = 0L;

		if(readfds){
			rd_fd = save_rd;
		}
		if(writefds){
			wr_fd = save_wr;
		}
		if(exceptfds){
			ex_fd = save_ex;
		}

		sa.rval = sa.errno = 0;
		if(sa.numfds){
			if(!InetBase){
				errno = EBADF;
				returnval = -1;
				break;
			}
			SelectAsm(&sa);
		}
		if(sa.errno){
			errno = sa.errno;
			returnval = -1;
			break;
		}
		if(returnval = sa.rval){
			event = usermask & SetSignal(0L, 0L);
			break;
		}

		event = Wait(usermask | selectmask | timerevent);
		if(event & (timerevent | usermask)){
			if(event & usermask){
				errno = EINTR;
				returnval = -1;
			}
			/* timed out or user event */
			break;
		}

		if(event & selectmask){
			continue;
		}

	} while(1);

	if(tvp){
		if(!CheckIO((struct IORequest *)&timereq)){
			if(!AbortIO((struct IORequest *)&timereq)){
				WaitIO((struct IORequest *)&timereq);
				SetSignal(0,timerevent);
			}
		}
	}

	for(fd = 0; fd < (sa.numfds+NFDBITS-1)>>NFDPOW; fd++){
		if(readfds){
			readfds->fds_bits[fd] = rd_fd.fds_bits[fd];
		}
		if(writefds){
			writefds->fds_bits[fd] = wr_fd.fds_bits[fd];
		}
		if(exceptfds){
			exceptfds->fds_bits[fd] = ex_fd.fds_bits[fd];
		}
	}

	if(umask){
		*umask = event;
	}

	if(selectbit != socket_sigio){
		FreeSignal(selectbit);
	}

	return returnval;
}

#ifdef LATTICE
/* Copyright Lattice, INC. 1989, ALL RIGHTS RESERVED */
#include <dos.h>
#include <error.h>
#include <fcntl.h> /******** Must use special fcntl.h *******/
#include <ios1.h>  /**** that nukes _read() and _write() refs ***/
#include <stdio.h>

extern int errno,_OSERR;
extern struct UFB *chkufb();

/****** socket/read ******************************************
*
*   NAME
*		read -- read a file
*
*   SYNOPSIS
*		status = read ( unit, buffer, length);
*
*		int  = read ( int, char *, int); 
*
*   FUNCTION
*		This function reads the next set of bytes from a file
*		that has been activated via a previous open call.
*
*	INPUTS
*		unit		unit number
*		buffer		input buffer
*		length		buffer length in bytes
*
*   RESULT
*		status		= number of bytes actually read
*					= 0 if end of file
*					= -1 if error occurred
*
*   EXAMPLE
*
*   NOTES
*		This replaces the normal compiler read() function.
*
*   BUGS
*
*   SEE ALSO
*		write(), close()
*
******************************************************************************
*
*/

int 
read(int f,char *b,int n)
{
	register struct UFB *ufb;
	register long x;

	if ((ufb = chkufb(f)) == 0){
   		return(-1);
	}
	if(FD_ISSET(f, &_is_socket)){
		x = recv(f, b, n, 0);
	} else {
		x = _dread(ufb->ufbfh,b,n);
	}
	if (_OSERR){
   		return(-1);
	}
	return((int)x);
}

/* Copyright Lattice, INC. 1989, ALL RIGHTS RESERVED */
extern int _OSERR;

/****** socket/write ******************************************
*
*   NAME
*		write -- write to a file
*
*   SYNOPSIS
*		status = write ( unit, buffer, length);
*
*		int  = write ( int, char *, int); 
*
*   FUNCTION
*		This function writes a set of bytes to the current file
*		position.
*
*	INPUTS
*		unit		unit number
*		buffer		output buffer
*		length		buffer length in bytes
*
*   RESULT
*		status		= number of bytes actually written
*					= -1 if error occurred
*
*   EXAMPLE
*
*   NOTES
*		This replaces the normal compiler write() function.
*
*   BUGS
*
*   SEE ALSO
*		read(), close()
*
******************************************************************************
*
*/

int
write( int f, char *b, int n )
{
	register long x;
	register struct UFB *ufb;

	if((ufb = chkufb(f)) == 0){
		return(-1);
	}
	if(ufb->ufbflg & O_APPEND){
		lseek(f,0L,2);
	}
	if(FD_ISSET(f, &_is_socket)){
		x = send(f, b, n, 0);
	} else {
		x = _dwrite(ufb->ufbfh,b,n);
	}
	if(_OSERR){
		return(-1);
	}
	return(x);
}

/* Copyright Lattice, INC. 1989, ALL RIGHTS RESERVED */

extern int _oserr;
extern struct UFB _ufbs[];

/****** socket/close ******************************************
*
*   NAME
*		close -- close a file
*
*   SYNOPSIS
*		status = close ( unit );
*
*		int  = close ( int ); 
*
*   FUNCTION
*		This function closes a file.
*
*	INPUTS
*		unit		unit number
*
*   RESULT
*		status		= 0 if successful
*					= -1 if error occurred
*
*   EXAMPLE
*
*   NOTES
*		This replaces the normal compiler close() function.
*
*   BUGS
*
*   SEE ALSO
*		read(), write()
*
******************************************************************************
*
*/

int close( int f )
{
	register struct UFB *ufb;

	if ((ufb = chkufb(f)) == 0){
		return(-1);
	}
	if (ufb->ufbflg & UFB_NC){ /* only Amiga makes this possible */
   		ufb->ufbflg = 0;
   		return(0) ;
   	}

	if(FD_ISSET(f, &_is_socket)){
		struct closeargs ca;
	
		GETSOCK(f, ca.fp);
		ca.errno = 0;
		NetcloseAsm(&ca);
		FD_CLR(f, &_is_socket);
		_socks[f] = 0;
		if(--num_socket <= 0){
			cleanup_sockets();
		}
		if(ca.errno){
			errno = ca.errno;
			return -1;
		}
	}
	_dclose(ufb->ufbfh);
	ufb->ufbflg = 0;
	if (_oserr){
   		return(-1);
	}
	return(0);
}


#else
/*
 * When socket lib is in use, we replace normal Manx read/write/close such
 * that we can do these operations without sending DOS packets.
 */

/* Copyright (C) 1986,1987 by Manx Software Systems, Inc. */

close( fd )
register int fd;
{
	register struct _dev *refp;
	register int i, err;
	struct closeargs ca;

	refp = _devtab + fd;
	if (fd < 0 || fd >= _numdev || refp->fd == 0) {
		errno = EBADF;
		return(-1);
	}
	if(FD_ISSET(fd, &_is_socket)){
		GETSOCK(fd, ca.fp);
		ca.errno = 0;
		NetcloseAsm(&ca);
		FD_CLR(fd, &_is_socket);
		_socks[fd] = 0;
		Close(refp->fd);
		if(--num_socket <= 0){
			cleanup_sockets();
		}
		if(ca.errno){
			refp->fd = 0;
			errno = ca.errno;
			return -1;
		}
	} else {
		if ((refp->mode & O_STDIO) == 0){
			Close(refp->fd);
		}
	}

	refp->fd = 0;
	return 0;
}

read( fd, buf, len)
register int fd;
char *buf;
int len;
{
	register struct _dev *refp;
	register long err;

	Chk_Abort();
	refp = _devtab + fd;
	if (fd < 0 || fd >= _numdev || refp->fd == 0){
		errno = EBADF;
		return(-1);
	}
	if ((refp->mode & 3) == O_WRONLY) {
		errno = EINVAL;
		return(-1);
	}
	if(FD_ISSET(fd, &_is_socket)){
		err = recv(fd, buf, len, 0);
	} else {
		if ((err = Read(refp->fd, buf, (long)len)) == -1) {
			errno = IoErr();
			return(-1);
		}
	}
	return((int)err);
}

write( fd, buf, len)
register int fd;
char *buf;
int len;
{
	register struct _dev *refp;
	register long err;

	Chk_Abort();
	refp = _devtab + fd;
	if (fd < 0 || fd >= _numdev || refp->fd == 0){
		errno = EBADF;
		return(-1);
	}
	if ((refp->mode & 3) == O_RDONLY) {
		errno = EINVAL;
		return(-1);
	}
	if(FD_ISSET(fd, &_is_socket)){
		err = send(fd, buf, len, 0);
	} else {
		if(refp->fd == 0) {
			errno = EBADF;
			return(-1);
		}
		if ((err = Write(refp->fd, buf, (long)len)) == -1) {
			errno = IoErr();
			return(-1);
		}
	}
	return((int)err);
}

#endif

