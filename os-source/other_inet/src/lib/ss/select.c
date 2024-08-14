/* -----------------------------------------------------------------------
 * select.c
 *
 * $Locker:  $
 *
 * $Id: select.c,v 1.5 92/07/21 17:04:17 bj Exp $
 *
 * $Revision: 1.5 $
 *
 * $Log:	select.c,v $
 * Revision 1.5  92/07/21  17:04:17  bj
 * socket.library 4.0
 * 
 * CreatePort() -> CreateMsgPort()
 * DeletePort() -> DeleteMsgPort()
 * 
 * Revision 1.4  91/10/01  16:56:13  bj
 * Fixed the fixes.
 * 
 * Revision 1.3  91/10/01  15:24:25  bj
 * Fixed bugs in select() function.  
 * 
 * Fixed bugs in selectwait() function, that is.
 * 
 * 1. selectbit signal not beeing FreeSignal()'d properly.
 * 2. Timer message port was not being Deleted at exit of function.
 * 
 * Revision 1.2  91/08/07  13:07:24  bj
 * Removed debug() calls.
 * RCS header.
 * 
 *
 * $Header: AS225:src/lib/ss/RCS/select.c,v 1.5 92/07/21 17:04:17 bj Exp $
 *
 *------------------------------------------------------------------------
 */

#include "sslib.h"
#include <sys/time.h>
#include <devices/timer.h>
#include <exec/exec.h>
#include <dos/dos.h>


/* #define DEBUG 1  */

#ifdef DEBUG
int kprintf( char *, ...) ;
#define DB(x)  kprintf(x)
#else
#define DB(x) ;
#endif

/****** socket.library/select  *************************************************
*
*   NAME
*	select -- Examines specified sockets' read, write or exception status.
*
*   SYNOPSIS
*	num = select( numfds, readfds, writefds, exceptfds, timeout )
*	D0            D0      A0       A1        A2         D1
*
*	int select( int, fd_set *, fd_set *, fd_set *, struct timeval *);
*
*   FUNCTION
*	select() examines the socket masks specified 'readfds,' 'writefds,'
*	and 'execptfds' (see FD_SET) to see if they are ready for reading,
*	for writing, or have an exceptional condition pending.

*	When select() returns, the masks will have been modified so that
*	only the "bits" for those sockets on which an event has occured are
*	set.  The total number of ready sockets is returned in 'num.'
*
*	If 'timeout' is a non-NULL pointer, it specifies a maximum
*	interval to wait for the selection to complete. If timeout is
*	a NULL pointer, the select blocks indefinitely. To affect a
*	poll, the timeout argument should be nonzero, pointing to a
*	zero valued timeval structure.  As you know, busy-loop polling
*	is a no-no on the Amiga.
*
*	Any of readfds, writefds, and execptfds may be given as NULL if
*	any of those categories are not of interest.
*
*	select() should not be used with a single socket (use s_getsignal()
*	and Wait() instead).
*
*   INPUTS
*	numfds    - Maximum number of bits in the masks that
*	            represent valid file descriptors.
*	readfds   - 32 bit mask representing read file descriptors
*	writefds  - 32 bit mask representing write file descriptors
*	exceptfds - 32 bit mask representing except file descriptors
*	timeout   - Pointer to a timeval structure which holds the
*	            maximum amount of time to wait for the selection
*	            to complete.
*
*   RESULT
*	num       - The number of ready sockets, zero if a timeout occurred,
*	            -1 on error.
*	readfds   - A mask of the ready socket descriptors
*	writefds  -      "      "     "     "      "
*	exceptfds -      "      "     "     "      "
*
*   NOTES
*	If a process is blocked on a select() waiting for input from a
*	socket and the sending process closes the socket, the select
*	notes this as an exception rather than as data.  Hence, if
*	the select is not currently looking for exceptions, it will
*	wait forever.
*
*	The descriptor masks are always modified on return, even if
*	the call returns as the result of the timeout.
*
*	The current version of this function calls selectwait()
*	with a control-C option in the umask field.
*
*	A common error is to use the socket number in which you are
*	interested as the first argument. Use socket+1.
*
*   BUGS
*
*   SEE ALSO
*	FD_SET(), selectwait(), s_getsignal()
*
********************************************************************
*
* select/selectwait - selects are in general very expensive operations in
* both Unix and AmigaDOS.  The user should really use FIOASYNC rather
* than relying on select.
*/


int __saveds __asm select( register __d0 int numfds,
	register __a0 fd_set *readfds,
	register __a1 fd_set *writefds,
	register __a2 fd_set *exceptfds,
	register __d1 struct timeval *tvp)
{
	long	mask = SIGBREAKF_CTRL_C;
	return selectwait(numfds, readfds, writefds, exceptfds, tvp, &mask);
}

/****** socket.library/selectwait *************************************************
*
*   NAME
*	selectwait -- select() with optional, task specific Wait() mask.
*
*   SYNOPSIS
*	num = selectwait( numfds, rfds, wfds, exfds, time, umask )
*	D0                D0      A0    A1    A2     D1    D2
*
*	int selectwait (int, int *, int *, int *, struct timeval *, long *);
*
*   FUNCTION
*	selectwait() is the same as select() except that it processes
*	one additional argument, 'umask'
*
*	The umask argument should contain either a NULL or a mask
*	of the desired task-specific signal bits that will be tested
*	along with the socket descriptors.  selectwait() does a standard
*	Exec Wait() call and adds the supplied mask value to Wait()
*	argument.
*
*   INPUTS
*	numfds    - The maximum number of bits in the masks that
*	            represent valid file descriptors.
*	readfds   - 32 bit mask representing read file descriptors
*	writefds  - 32 bit mask representing write file descriptors
*	exceptfds - 32 bit mask representing except file descriptors
*	timeout   - A pointer to a timeval structure which holds the
*	            maximum amount of time to wait for the selection
*	            to complete.
*	umask     - A mask of the task's signal bits that will be
*	            used (in addition to the standard select() call
*	            return options) to have the call return. This can
*	            be SIGBREAKF signals, Intuition userport signals,
*	            console port signals, etc. Any mask that you would
*	            pass to the the Exec Wait() call is ok here.
*
*   RESULT
*	num       - The number of ready file descriptors if
*	            successful.  Zero (0) if a timeout occurred.
*	            (-1) upon error.
*	readfds   - A mask of the ready file descriptors
*	writefds  -      "      "     "     "      "
*	exceptfds -      "      "     "     "      "
*	umask     - A mask of the bits in the originally passed 'umask'
*	            variable that have actually occured.
*
*   EXAMPLE
*	long umask = SIGBREAKF_CTRL_D | 1L << myport->mp_SigBit ;
*
*	numfds = selectwait(n, r, w, e, time, &umask) ;
*	if( event & SIGBREAKF_CTRL_D ) {
*		printf("user hit CTRL-D\n") ;
*	}
*	if( event & 1L << myport->mp_SigBit ) {
*		printf( "myport got a message\n" ) ;
*	}
*   NOTES
*	A common error is to use the socket number in which you are
*	interested as the first argument. Use socket+1.

*   BUGS
*
*   SEE ALSO
*	FD_SET(), select(), s_getsignal()
*
********************************************************************
*
* selectwait() - examine specified buffers for data.  Returns > 0 if some
* of the buffers have data/space available, == 0 if we timed out, and < 0
* if user event mask occurred.
*/

int __saveds __asm selectwait( register __d0 int numfds,
	register __a0 fd_set *readfds,
	register __a1 fd_set *writefds,
	register __a2 fd_set *exceptfds,
	register __d1 struct timeval *tvp,
	register __d2 long *umask)
{
	fd_set rd_fd, wr_fd, ex_fd, save_rd, save_wr, save_ex;
	long event, usermask, selectmask;
	long timerevent;
	int returnval, selectbit;
	struct selectargs sa;
	register int fd;
	short do_timer;

	selectbit = AllocSignal(-1L);
	if(selectbit == -1)
	{
		selectbit = Socket_sigio;
	}
	selectmask = 1L << selectbit;
	timerevent = 0L;
	usermask = umask ? *umask : 0L;

	do_timer = (tvp && !(tvp->tv_secs==0 && tvp->tv_micro<=5));

	if(do_timer) 
	{
		DB("selectwait: in do_timer\n") ;
		if(Timeropen==0) 
		{
			
			DB("selectwait: into CreatePort()\n") ;
			Timereq.tr_node.io_Message.mn_ReplyPort = CreateMsgPort();
			if(OpenDevice(TIMERNAME, UNIT_VBLANK, (struct IORequest *)&Timereq, 0L))
			{
				DB("selectwait: opendevice(timer) failed\n") ;
				DeleteMsgPort(Timereq.tr_node.io_Message.mn_ReplyPort);
				if( selectbit != Socket_sigio )
				{
					FreeSignal( (long)selectbit) ;
				}
				return -1;
			}
			Timeropen = 1;
			DB("timer is open\n") ;
			Select_timerbit = Timereq.tr_node.io_Message.mn_ReplyPort->mp_SigBit;
		} 
		else 	/* kill previous timer */
		{
			DB("selectwait: timer already open\n") ;
			AbortIO((struct IORequest *)&Timereq);
			WaitIO((struct IORequest *)&Timereq);
			SetSignal(0,timerevent);
		}
		Timereq.tr_node.io_Command = TR_ADDREQUEST;
		Timereq.tr_time = *tvp;
		SendIO((struct IORequest *)&Timereq);
		DB("selectwait: SendIO(timereq)\n") ;
		timerevent = (1L << Select_timerbit);
	}

	sa.numfds = min(numfds, MAXSOCKS);
	sa.socks = (void *)socks;
	sa.rd_set = readfds? &rd_fd : 0;
	sa.wr_set = writefds? &wr_fd : 0;
	sa.ex_set = exceptfds? &ex_fd : 0;
	sa.task = FindTask(0L);
	sa.sigbit = selectbit;

	for(fd = 0; fd < (sa.numfds+NFDBITS-1)>>NFDPOW; fd++)
	{
		if(readfds)
		{
			save_rd.fds_bits[fd] = readfds->fds_bits[fd];
		}
		if(writefds)
		{
			save_wr.fds_bits[fd] = writefds->fds_bits[fd];
		}
		if(exceptfds)
		{
			save_ex.fds_bits[fd] = exceptfds->fds_bits[fd];
		}
	}

	do {
		event = 0L;

		if(readfds)
		{
			rd_fd = save_rd;
		}
		if(writefds)
		{
			wr_fd = save_wr;
		}
		if(exceptfds)
		{
			ex_fd = save_ex;
		}
		sa.rval = sa.errno = 0;
		if(sa.numfds)
		{
			SelectAsm(&sa);
		}

		*ss_errno = sa.errno;
		if (sa.errno) 
		{
			returnval = -1;
			break;
		}

		if(returnval = sa.rval)
		{
			event = usermask & SetSignal(0L, 0L);
			break;
		}

		/* if a poll then break */
		if (tvp && do_timer==0)
		{
			break;
		}

		DB("selectwait: into Wait()") ;
		event = Wait(usermask | selectmask | timerevent);
		if(event & (timerevent | usermask))
		{
			if(event & usermask)
			{
				*ss_errno = EINTR;
				returnval = -1;
			}
			/* timed out or user event */
			break;
		}

		if(event & selectmask)
		{
			continue;
		}

	} while(1);

	if(do_timer)
	{
		DB("selectwait: killing timer (#1)\n") ;
		AbortIO((struct IORequest *)&Timereq);
		WaitIO ((struct IORequest *)&Timereq);
		SetSignal(0,timerevent);
	}


	for(fd = 0; fd < (sa.numfds+NFDBITS-1)>>NFDPOW; fd++)
	{
		if(readfds)
		{
			readfds->fds_bits[fd] = rd_fd.fds_bits[fd];
		}
		if(writefds)
		{
			writefds->fds_bits[fd] = wr_fd.fds_bits[fd];
		}
		if(exceptfds)
		{
			exceptfds->fds_bits[fd] = ex_fd.fds_bits[fd];
		}
	}

	if(umask)
	{
		*umask = event;
	}


	if(selectbit != Socket_sigio)
	{
		DB("selectwait: FreeSignal() selectbit (2)\n"); 
		FreeSignal((long)selectbit);
	}

	DB("selectwait: returning - end of function\n") ;
	return returnval;
}

