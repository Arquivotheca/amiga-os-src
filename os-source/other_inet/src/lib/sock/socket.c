/* -----------------------------------------------------------------------
 * socket.c    LATTICE 5.10   for  socket.library
 *
 * $Locker:  $
 *
 * $Id: socket.c,v 1.2 90/12/10 16:06:05 bj Exp $
 *
 * $Revision: 1.2 $
 *
 * $Header: HOG:Other/inet/src/lib/net2/RCS/socket.c,v 1.2 90/12/10 16:06:05 bj Exp $
 *
 * $Log:	socket.c,v $
 * Revision 1.2  90/12/10  16:06:05  bj
 * fixed relative path for header file 'fd_to_fh.h'
 * 
 *
 *------------------------------------------------------------------------
 */




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

#include "fd_to_fh.h"
#include <socket.h>

#include <libraries/dos.h>


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


/****** socket/select  *************************************************
*
*	NAME
*		select - Examines the specified file descriptors for read, 
*		         write or exception status.
*
*	SYNOPSIS
*		num = select( numfds, readfds, writefds, exceptfds, timeout)
*
*		int select( int, int *, int *, int *, struct timeval * )	
*
*	FUNCTION
*		The select call examines the I/O descriptors specified by 
*		the bit masks readfds, writefds, and execptfds to see if 
*		they are ready for reading, writing, or have an exceptional
*		condition pending, respectively. The I/O descriptors can be
*		pointers to arrays of integers if multiple fd's are required
*		to be selected. File descriptor f is represented by the bit 
*		"1<<f" in the mask. The nfds descriptors are checked, that is
*		the bits from 0 through nfds-1 in the masks are examined. The
*		select system call returns, in place, a mask of those 
*		descriptors which are ready. The total number of ready 
*		descriptors is returned in nfound.
*
*		If timeout is a nonzero pointer, it specifies a maximum 
*		interval to wait for the selection to complete. If timeout is
*		a zero pointer, the select blocks indefinitely. To affect a 
*		poll, the timeout argument should be nonzero, pointing to a 
*		zero valued timeval structure. ** SEE BUGS NOTE BELOW ! **
*
*		Any of readfds, writefds, and execptfds may be given as 0 if 
*		no descriptors are of interest.
*
*	RESULT
*		The select system call returns the number of descriptors 
*		which are contained in the bit masks, or -1 if an error 
*		occurred. If the time limit expires then select returns 0.
*
*	NOTES
*		If a process is blocked on a select waiting for input from a
*		socket and the sending process closes the socket, the select
*		notes this as an exception rather than as data.  Hence, if
*		the select is not currently looking for exceptions, it will
*		wait forever.
*
*		The descriptor masks are always modified on return, even if
*		the call returns as the result of the timeout.
*
* 		The current version of this function calls selectwait()
*		with a control-C option in the umask field.
*
*	INPUTS
*		numfds    - Maximum number of bits in the masks that
*		            represent valid file descriptors.
*		readfds   - 32 bit mask representing read file descriptors
*		writefds  - 32 bit mask representing write file descriptors 
*		exceptfds - 32 bit mask representing except file descriptors 
*		timeout   - Pointer to a timeval structure which holds the
*		            maximum amount of time to wait for the selection 
*		            to complete.
*		            
*		file descriptor 'f' is represented in the masks as bit "1 << f"
*		in the mask.
*
*	RESULT
*		num       - The number of ready file descriptors if 
*		            successful. Zero (0) is a timeout occurred.
*		            (-1) upon error. 
*		readfds   - A mask of the ready file descriptors
*		writefds  -      "      "     "     "      "
*		exceptfds -      "      "     "     "      "        
*
*	BUGS
*		The timer device in Amiga OS versions <= 1.3 has a bug that
*		can cause system problems if a request is made for a timer 
*		return in under 5 microseconds. So, to effect a polled call 
*		to select() the timeval structure should be:
*
*			struct timeval *tvp ;
*				tvp->tv_secs = 0L ;
*				tvp->tv_secs = 5L ;    <<<<<---- not ZERO !
*
*		This will avoid the problem. Failure to do this can cause 
*		system errors.
*
*	SEE ALSO
*		selectwait()
*   
********************************************************************
*
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

/****** socket/selectwait *************************************************
*
*	NAME
*		selectwait - Examines the specified file descriptors for read, 
*		             write or exception status with an optional, task-
*		             specific signal mask.
*
*	SYNOPSIS
*		num = selectwait(numfds, rfds, wfds, exfds, time, umask)
*
*		int selectwait
*			(int, int *, int *, int *, struct timeval *, long *) 
*
*	FUNCTION
*		The select call examines the I/O descriptors specified by 
*		the bit masks readfds, writefds, and execptfds to see if 
*		they are ready for reading, writing, or have an exceptional
*		condition pending, respectively. The I/O descriptors can be
*		pointers to arrays of integers if multiple fd's are required
*		to be selected. File descriptor f is represented by the bit 
*		"1<<f" in the mask. The nfds descriptors are checked, that is
*		the bits from 0 through nfds-1 in the masks are examined. The
*		select system call returns, in place, a mask of those 
*		descriptors which are ready. The total number of ready 
*		descriptors is returned in nfound.
*
*		If timeout is a nonzero pointer, it specifies a maximum 
*		interval to wait for the selection to complete. If timeout is
*		a zero pointer, the select blocks indefinitely. To affect a 
*		poll, the timeout argument should be nonzero, pointing to a 
*		zero valued timeval structure. ** SEE BUGS NOTE BELOW ! **
*
*		The umask argument should contain either a ZERO or a mask
*		of the desired task-specific signal bits that will be tested
*		along with the file descriptors. Selectwait() does a standard
*		Amiga Exec Wait() call and adds the supplied mask value to
*		Wait() argument.
*
*		Any of readfds, writefds, and execptfds may be given as 0 if 
*		no descriptors are of interest.
*
*	INPUTS
*		numfds    - The maximum number of bits in the masks that
*		            represent valid file descriptors.
*		readfds   - 32 bit mask representing read file descriptors
*		writefds  - 32 bit mask representing write file descriptors 
*		exceptfds - 32 bit mask representing except file descriptors 
*		timeout   - A pointer to a timeval structure which holds the
*		            maximum amount of time to wait for the selection
*		            to complete.
*		umask     - A mask of the task's signal bits that will be
*		            used (in addition to the standard select() call 
*		            return options) to have the call return. This can
*		            be SIGBREAKF signals, Intuition userport signals, 
*		            console port signals, etc. Any mask that you would
*		            pass to the the Exec Wait() call is ok here.
*
*	RESULT
*		num       - The number of ready file descriptors if 
*		            successful. Zero (0) is a timeout occurred.
*		            (-1) upon error. 
*		readfds   - A mask of the ready file descriptors
*		writefds  -      "      "     "     "      "
*		exceptfds -      "      "     "     "      "        
*
*		umask     - A mask of the bits in the originally passed 'umask'
*		            variable that have actually occured.  
*
*	EXAMPLE
*		long umask = SIGBREAKF_CTRL_D | 1L << myport->mp_SigBit ;
*
*		numfds = selectwait(n, r, w, e, time, &umask) ;
*		if( event & SIGBREAKF_CTRL_D ) {
*			printf("user hit CTRL-D\n") ;
*		}
*		if( event & 1L << myport->mp_SigBit ) {
*			printf( "myport got a message\n" ) ;
*		}
*
*	BUGS
*		The timer device in Amiga OS versions <= 1.3 has a bug that
*		can cause system problems if a request is made fof a timer 
*		return in under 5 microseconds. So, to effect a polled call 
*		to select() the timeval structure should be:
*
*			struct timeval *tvp ;
*				tvp->tv_secs = 0L ;
*				tvp->tv_secs = 5L ;    <<<<<---- not ZERO !
*
*		This will avoid the problem. Failure to do this can cause 
*		system errors.
*
*
*	SEE ALSO
*		select()
*   
********************************************************************
*
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
#include <fcntl.h> 
#include <ios1.h>  
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
*		int  = read ( int, char *, unsigned); 
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
read(int f,char *b,unsigned n)
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
*		int  = write ( int, char *, unsigned); 
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
write( int f, char *b, unsigned n )
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
	return((int)x);
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

int close( int fd )
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

int read( int fd, char *buf, int len)
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

int write( fd, buf, len)
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

