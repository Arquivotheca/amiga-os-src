/* Unix socket compatibility functions for SAS 
** To use these, just call init_sock(), then use sockets
** just like file descriptors.  Call clean_sock()
** on exit.
*/

#define UNIX_COMPAT 1	/* to get correct socket() and accept() */
#include <ss/socket.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#ifdef LATTICE
#include <dos.h>
#include <error.h>
#include <fcntl.h> 
#include <ios1.h>  
#include <stdio.h>
extern int _nufbs;
extern struct UFB _ufbs[];
#define NUMFD _nufbs
#endif /* LATTICE */

extern struct ExecBase *SysBase;
extern struct Library *DOSBase;

struct Library *SockBase;
int socket_sigio = -1, socket_sigurg = -1;
static int setup;
fd_set _is_sock;

void clean_sock(void);
int init_sock(void);

void clean_sock()
{
	if(!setup){
		return;
	}
	setup = 0;

	cleanup_sockets();
	CloseLibrary(SockBase);
	SockBase = 0;

}

 
int init_sock()
{
	if(setup){
		return 0;
	}

	if((SockBase=OpenLibrary("inet:libs/socket.library",1L))==NULL) {
		errno = ECONFIGPROBLEM;
		return -1;
	}
	setup_sockets( NUMFD, &errno );
	s_dev_list((u_long)_ufbs,sizeof(struct UFB));
	socket_sigurg = s_getsignal(SIGURG);
	socket_sigio = s_getsignal(SIGIO);

	FD_ZERO(&_is_sock);

	setup++;

	return 0;
}


int socket( af, type, pf)
int af, type, pf;
{
	int ns;
	struct UFB *ufb;


	if((ns = s_socket(af, type, pf))<0)
		return(-1);
	/*
	 * We open a real filehandle here to accompany the socket in
	 * case the application manages to manuever around all the checks
	 * engineered to prevent launching a DOS packet.
	 */
#ifdef LATTICE
	ufb = &_ufbs[ns];

	if((ufb->ufbfh = Open("nil:", MODE_OLDFILE))==0) {
		s_close(ns);
		return(-1);
	}
	ufb->ufbflg = O_RAW | UFB_RA | UFB_WA ;
#endif
	FD_SET(ns, &_is_sock);
	return(ns);
}


int accept(int s, struct sockaddr *name, int *lenp)
{
	int ns;
	struct UFB *ufb;

	if((ns = s_accept(s, name, lenp))<0)
		return(-1);

	/*
	 * We open a real filehandle here to accompany the socket in
	 * case the application manages to manuever around all the checks
	 * engineered to prevent launching a DOS packet.
	 */
#ifdef LATTICE
	ufb = &_ufbs[ns];
	if((ufb->ufbfh = Open("nil:", MODE_OLDFILE))==0) {
		s_close(ns);
		return(-1);
	}
	ufb->ufbflg = O_RAW | UFB_RA | UFB_WA ;
#endif
	FD_SET(ns, &_is_sock);
	return(ns);
}


/* Daemons call this on startup.
** It will map stdout and stdin to the socket
** that was passed to the daemon.
*/

int inherit(char *sp)
{
	int ns;
	struct UFB *ufb;

	/* first, close std(in|out) */
	close(0);
	close(1);


	/* we just freed 0 and 1, so these will open a socket on 0 and 1 */
	(void)s_inherit((void *)atol(sp));
	ns = s_inherit((void *)atol(sp));
	if(ns != 1)
		return(-1); /* should never happen */

	/* open a dummy file and set up flags */
	for(ns=0;ns<2;ns++) {
#ifdef LATTICE
		ufb = &_ufbs[ns];
		if((ufb->ufbfh = Open("nil:", MODE_OLDFILE))==0) {
			s_close(ns);
			return(-1);
		}
		ufb->ufbflg = O_RAW | UFB_RA;
		if(ns>0)
			ufb->ufbflg |= UFB_WA;
#endif
		FD_SET(0, &_is_sock);
		FD_SET(1, &_is_sock);
	}
	return 0;
}


#ifdef LATTICE
/* Copyright Lattice, INC. 1989, ALL RIGHTS RESERVED */

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
	if(FD_ISSET(f,&_is_sock)){
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

	if((ufb = chkufb(f)) == 0) {
		return(-1);
	}
	if(ufb->ufbflg & O_APPEND) {
		lseek(f,0L,2);
	}
	if(FD_ISSET(f,&_is_sock)) {
		x = send(f, b, n, 0);
	} else {
		x = _dwrite(ufb->ufbfh,b,n);
	}
	if(_OSERR){
		return(-1);
	}
	return((int)x);
}

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

extern int _oserr;
extern struct UFB _ufbs[];

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

	if(FD_ISSET(f, &_is_sock)) {
		s_close(f);
		FD_CLR(f, &_is_sock);
	}
	_dclose(ufb->ufbfh);
	ufb->ufbflg = 0;
	if (_oserr)
   		return(-1);

	return(0);
}



#endif

