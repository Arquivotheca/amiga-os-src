/* -----------------------------------------------------------------------
 * socket.c
 *
 * $Locker:  $
 *
 * $Id: socket.c,v 1.5 92/07/21 16:23:33 bj Exp $
 *
 * $Revision: 1.5 $
 *
 * $Log:	socket.c,v $
 * Revision 1.5  92/07/21  16:23:33  bj
 * Added s_inherit(), s_release() and s_dev_list() calls.
 * Socket.library 4.0 and greater.  AS225  R2.
 * 
 * Revision 1.4  91/10/01  16:58:15  bj
 * fixed the fixes.
 * 
 * Revision 1.3  91/10/01  15:27:31  bj
 * Fixed bugs in setup_sockets() function.
 * 1. Fixed improper AllocSignal/FreeSignal.
 * 2. Fixed bad exit handling.
 * 
 * Revision 1.2  91/08/07  13:13:53  bj
 * rcs header added.
 * autodoc addition
 * 
 *
 * $Header: AS225:src/lib/ss/RCS/socket.c,v 1.5 92/07/21 16:23:33 bj Exp $
 *
 *------------------------------------------------------------------------
 */
#include "sslib.h"
#include <exec/memory.h>
#include <devices/timer.h>
#include <signal.h>

int *ss_errno = NULL;
long *socks = NULL;

/* device table stuff */
u_long reserved=0;
int rsize;

BYTE Socket_sigurg =0;
BYTE Socket_sigio =0;
UWORD MAXSOCKS = 0;

UWORD Timeropen =0;
BYTE Select_timerbit =0;
struct timerequest Timereq = { 0 };
BPTR debugfh = NULL;

/****** socket.library/setup_sockets ******************************************
*
*   NAME
*	setup_sockets -- Initialize global data for sockets.
*
*   SYNOPSIS
*	retval = setup_sockets( max_sockets, errnop );
*	D0                      D1           A0
*
*	ULONG setup_sockets( UWORD, int * );
*
*   FUNCTION
*	This function initializes global variables, sets the library errno
*	to point to the application's errno, and allocates signals.
*	This should always be called immediately after an OpenLibrary().
*	The only reason this initialization is not done automatically
*	is because a pointer to errno must be passed in.
*
*	'max_sockets' must be less than FD_SETSIZE.
*
*   INPUTS
*	max_sockets	- maximum number of sockets that can be open at once.
*			  (4 bytes are allocated for each socket.)
*	errno		- pointer to the global int 'errno.'
*
*   RESULT
*	retval		- TRUE on success, FALSE on failure.  If 'max_sockets'
*			  is greater than FD_SETSIZE, setup_sockets() will
*			  fail.  FD_SETSIZE is currently 128 (see
*			  <sys/types.h>)
*
*   NOTES
*	If you are using a language other than C, you must pass in a pointer
*	to a variable that will hold the error numbers.
*
*	In SAS C, errno is a global in lc.lib.  If you don't link with lc.lib
*	you will have to declare errno locally.
*
*   BUGS
*
*   SEE ALSO
*	cleanup_sockets()
*
******************************************************************************
*
*/


ULONG __saveds __asm setup_sockets(              
	register __d1 UWORD maxsocks,
	register __a0 int *apps_errno )
{
	if ( apps_errno==NULL || InetBase==NULL )
		goto ex2;

	ss_errno = apps_errno ;

	/* can't have more than FD_SETSIZE sockets */
	if(maxsocks>FD_SETSIZE)
		goto ex2;

	socks = AllocMem(maxsocks*4,MEMF_CLEAR|MEMF_PUBLIC);
	if (!socks)
		goto ex2;
	MAXSOCKS = maxsocks;

	Socket_sigurg = AllocSignal(-1L) ;
	if (Socket_sigurg == -1)
		goto ex1;

	Socket_sigio  = AllocSignal(-1L) ;
	if (Socket_sigio != -1)
		return (TRUE);

	FreeSignal(Socket_sigurg);
ex1:
	FreeMem(socks,maxsocks*4);
ex2:
	return (FALSE);

}

/****** socket.library/cleanup_sockets ******************************************
*
*   NAME
*	cleanup_sockets -- Free global data for sockets.
*
*   SYNOPSIS
*	cleanup_sockets( );
*
*	void cleanup_sockets( void );
*
*   FUNCTION
*	This function frees all signals and allocated memory.
*	It closes all open sockets.  This function should be
*	called after all socket functions are completed and before
*	CloseLibrary().
*
*   INPUTS
*	None.
*
*   RESULT
*	None.
*
*   BUGS
*
*   SEE ALSO
*	setup_sockets()
*
******************************************************************************
*
*/

void __saveds cleanup_sockets( void )
{
	int i;

	if (debugfh)
		Close(debugfh);
	debugfh = NULL;


	for (i=0;i<MAXSOCKS;i++)
		if (socks[i]) s_close(i);

	FreeMem(socks,MAXSOCKS*4);

	if(Timeropen)
	{
		DeleteMsgPort(Timereq.tr_node.io_Message.mn_ReplyPort);
		CloseDevice((struct IORequest *)&Timereq) ;
	}

	FreeSignal(Socket_sigurg) ;
	FreeSignal(Socket_sigio) ;
}


/****** socket.library/socket ************************************
*
*   NAME
*	socket -- Create an endpoint for communication.
*
*   SYNOPSIS
*	s = socket( family, type, protocol )
*	D0          D0      D1    D2
*
*	int socket( int, int, int );
*
*   FUNCTION
*	socket() returns a socket descriptor for a socket with .
*
*   INPUTS
*	family   - This specifies an address format with which
*	           addresses specified in later operations using
*	           socket should be interpreted.
*	type     - Specifies the semantice of communication.
*	protocol - Specifies a particular protocol to be used with the
*	           socket.
*
*   RESULT
*	s        - Returns a (-1) upon failure  ; a socket descriptor
*	           upon success.
*
*   NOTES
*	Unlike the linkable socket library, this function assumes that
*	you have already made a succesful call to 'setup_sockets()'.
*
*   SEE ALSO
*
********************************************************************
*
*
*/




int __saveds __asm socket( register __d0 int af,
	register __d1 int type,
	register __d2 int pf)
{
	struct sockargs sa ;
	int ns = 0 ;

	sa.errno    = 0 ;
	sa.domain   = af ;
	sa.type     = type ;
	sa.protocol = pf ;
	sa.sigurg   = Socket_sigurg ;
	sa.sigio    = Socket_sigio ;

	/* find an empty space in the socket array */
	while(socks[ns] || (reserved && *(int *)(reserved+rsize*ns)) ) {
		ns++;
		if(ns>=MAXSOCKS) {
			*ss_errno = EMFILE;
			return (-1);
		}
	}


	SocketAsm(&sa) ;

	if( sa.errno == 0 ) {
		*ss_errno = 0;
		socks[ns] = (long)sa.rval ;
		return( ns ) ;
	} else {
		*ss_errno = sa.errno ;
		return (-1);
	}

}

/****** socket.library/s_close ******************************************
*
*   NAME
*	s_close -- Close a socket.
*
*   SYNOPSIS
*	status = s_close( socket ) ;
*	D0                D0
*
*	int s_close( int ) ;
*
*   FUNCTION
*       This function closes a socket.
*
*   INPUTS
*       unit		- socket number.
*
*   RESULT
*	status          - 0 if successful, else -1.
*
*   EXAMPLE
*
*   NOTES
*	s_close() must always be used to close a socket.  This shared
*	library does not know about filehandles or file descriptors.
*
*   BUGS
*
*   SEE ALSO
*	socket()
*
****************************************************************************
*
*/

int __saveds __asm s_close( register __d0 int fd)
{
        struct closeargs ca ;

        GETSOCK(fd, ca.fp) ;
        ca.errno = 0 ;
        NetcloseAsm(&ca) ;
        socks[fd] = 0L ;
        if(ca.errno) {
            *ss_errno = ca.errno ;
            return( -1 ) ;
        }
        *ss_errno = 0;
        return( 0 ) ;
}


/****** socket.library/s_getsignal ******************************************
*
*   NAME
*	s_getsignal -- Get a network signal bit.
*
*   SYNOPSIS
*	signal = s_getsignal( type );
*	D0                    D1
*
*	BYTE s_getsignal( UWORD );
*
*   FUNCTION
*	This function returns a socket signal.  The socket signal can be
*	used to Wait() on an event for the shared socket library.  The
*	following signal types are supported:
*
*	SIGIO   This signal indicates a socket is ready for
*		asynchronous I/O.  This signal will be sent only if
*		the socket has been set to async by calling ioctl()
*		with a command of FIOASYNC.
*
*	SIGURG  This signal indicates the presence of urgent or
*		out-of-band data on a TCP socket.
*   INPUTS
*	type		- SIGIO or SIGURG.
*
*   RESULT
*	signal		- signal bit (0..31) or -1 if 'type' was invalid.
*
*   EXAMPLE
*
*   NOTES
*	The SIGIO signal will only be set for sockets on which FIOASYNC has
*	been set (with s_ioctl or s_setsockopts.)
*
*   BUGS
*
*   SEE ALSO
*	s_ioctl(), select(), selectwait()
*
******************************************************************************
*
*/


BYTE __saveds __asm s_getsignal( register __d1 UWORD type)
{
	switch (type) {
		case SIGIO:
			return(Socket_sigio);
		case SIGURG:
			return(Socket_sigurg);
		default:
			return(-1);
	}
}

/****** socket.library/s_release ******************************************
*
*   NAME
*	s_release -- release a socket
*
*   SYNOPSIS
*	sockptr = s_release( s );
*	D0                   D1
*
*	void *s_release( int );
*
*   FUNCTION
*	This function along with s_inherit() provide a way to pass a
*	socket from one process to another.  s_release(s) will 
*	"detach" socket s from the current process.  Your process
*	will no longer know about the socket.  You can not call
*	s_close() on a socket you have released because it is no
*	longer owned until it is s_inherit()ed. Once socket 's'
*	is released, the socket descriptor 's' will be reused.
*
*   INPUTS
*	s		- socket descriptor
*
*   RESULT
*	sockptr		- really a pointer to an internal socket structure
*			      NULL on failure	
*
*   EXAMPLE
*	sockptr = s_release(s);
*	/ * start a new program and pass socket on command line * /
*	sprintf(cmdbuf, "run >nil: %s %ld", cmd_name,sockptr);
*	Execute(cmdbuf, 0L, 0L);
*
*   NOTES
*	Use with caution.  Don't release a socket unless another
*	process will s_inherit() it.
*
*   BUGS
*
*   SEE ALSO
*	s_inherit()
*
******************************************************************************
*
*/



void * __saveds __asm s_release( register __d1 int s )
{
	void *fp;

	if( s<0 || s >= MAXSOCKS || socks[s]==NULL) {
		*ss_errno = EBADF;
		return NULL;
	}
	fp = (void *)socks[s];
	socks[s] = 0L;
	*ss_errno = 0;
	return(fp);
}


/****** socket.library/s_inherit ******************************************
*
*   NAME
*	s_inherit -- inherit a socket
*
*   SYNOPSIS
*	s = s_inherit( sockptr );
*	D0                D1
*
*	int s_inherit( void * );
*
*   FUNCTION
*	This function along with s_release() provide a way to pass a
*	socket from one process to another.  s_inherit() will attach
*	a socket that has been released to your process. Use instead
*	of socket() when another process hands you a socket.
*
*   INPUTS
*	sockptr		- really a pointer to an internal socket structure
*
*   RESULT
*	s		- socket descriptor (-1 on failure)
*
*   EXAMPLE
*	void main(void)
*	{
*		int errno, s;
*		long opts[OPT_COUNT];
*		struct RDargs *rdargs;
*
*		memset((char *)opts, 0, sizeof(opts));
*		rdargs = ReadArgs(TEMPLATE, opts, NULL);
*
*		if ((SockBase = OpenLibrary( "inet:libs/socket.library", 1L ))==NULL) {
*			goto exit1;
*		}
*		setup_sockets(5,&errno);
*
*		/ * now get our socket * / 
*		s = s_inherit((void *)*(long *)opts[OPT_SOCKPTR]);
*
*
*   NOTES
*	Use with caution.  Calling s_inherit() with an invalid sockptr
*	will cause serious problems.
*	
*   BUGS
*	Not safe if an invalid pointer is passed.
*
*   SEE ALSO
*	s_release()
*
******************************************************************************
*
*/

int __saveds __asm s_inherit( register __d1 void *fp )
{
	struct inheritargs ia;
	register int ns = 0;

	ia.fp = fp;
	ia.sigio = Socket_sigio;
	ia.sigurg = Socket_sigurg;
	ia.errno = 0;
	InheritAsm(&ia);
	if(ia.errno){
		*ss_errno = ia.errno;
		return( -1 );
	}

	while(socks[ns] || (reserved && *(int *)(reserved+rsize*ns)) ) {
		ns++;
		if( ns >= MAXSOCKS ) {
			*ss_errno = EMFILE;
			return ( -1 );
		}
	}

	socks[ns] = (long)fp;
	return(ns);
}


/****** socket.library/s_dev_list ******************************************
*
*   NAME
*	s_dev_list -- set device list
*
*   SYNOPSIS
*	s_dev_list( res, size);
*	            D0    D1
*
*	void s_dev_list( u_long, int );
*
*   FUNCTION
*	This function is intended to assist in the building of
*	Unix compatibility libraries.  s_dev_list() pass a pointer
*	to an array that can be used to tell the socket library
*	what socket numbers should not be used.  The socket library
*	will not create a socket number 'X' if reserved[X] is set.
*	The socket library will never change the contents of the
*	reserved array.
*
*   INPUTS
*	res		- pointer to device table
*	size		- size of each entry in the device table
*
*   EXAMPLE
*
*   NOTES
*	This is a kludge to get around some of the problems caused because
*	a socket is not a file descriptor or a filehandle.
*	If you don't understand why this function is here, you probably
*	don't need it. 
*	
*
******************************************************************************
*
*/


void  __saveds __asm s_dev_list(
	register __d0 u_long res, 
	register __d1 int size )
{
	reserved = res;
	rsize = size;
}



