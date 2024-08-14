/* -----------------------------------------------------------------------
 * accept.c    socket.library  shared  Calls Other Functions
 *
 * $Locker$
 *
 * $Id$
 *
 * $Revision$
 *
 * $Header$
 *
 * $Log$
 *
 *------------------------------------------------------------------------
 */

/****** socket/accept ******************************************
*
*   NAME
*	accept -- accept new connection from socket
*
*   SYNOPSIS
*	ns = accept( socket, name, len, ss_err)
*	D0           D0      A0     A1  A2
*
*	int accept (int, struct sockaddr *, int *, int *); 
*
*   FUNCTION
*	After a server executes the listen() call, a connection from 
*	some client process is waited for by having the server execute
*	the accept() call.  
*
*	INPUTS
*	s	socket descriptor.
*
*	name	Pointer to the sockaddr struct that is returned.
*
*	len	pointer to the length of the sockaddr struct
*		the value returned will be the actual size of the
*		sockaddr struct that was returned. If this value
*		is set to NULL then 'name' and 'len' will be
*		ignored.
*
*	ss_err	pointer to the calling process's global 'errno'
*		Errors that the caller would normally expect to
*		be found in "the global integer 'errno'" will be
*		found in this variable.
*
*   RESULT
*	ns	new socket descriptor or -1 on error.
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/


#include "fd_to_fh_ss.h"
#include <socket.h>
#include "ss.h"
#include <functions.h>

struct SSInfo *FindEntry( struct Process *pid ) ;
int accept( int s, struct sockaddr *name, int *lenp ) ;


#pragma regcall(accept(D0,A0,A1))
#pragma regcall(FindEntry(D0))


int
accept(register int s, struct sockaddr *name, int *lenp)
{
	struct acceptargs aa ;
	int ns ;
	struct SSInfo *ssinfo = NULL ;

	if((ssinfo = (struct SSInfo *)FindEntry((struct Process *)FindTask(0L))) == NULL)
		return( -1 ) ;
    else
    	return((int) ssinfo ) ;
    
    /*-----------------------------------
	GETSOCK( s, aa.fp ) ;
	
	if((ns = open( "nil:", 2 )) < 0)
		{
		return( -1 ) ;
		}

	aa.errno   = 0 ;
	aa.rval    = 0 ;
	aa.namelen = lenp ? *lenp : 0 ;
	aa.name    = lenp ?  name : NULL ;
	AcceptAsm(&aa) ;
	*ss_error  = aa.errno ;
	if( aa.errno )
		{
		close( ns ) ;
		return( -1 ) ;
		}

	PUTSOCK(ns, aa.rval) ;
	if(lenp)
		{
		*lenp = aa.namelen ;
		}

	return( ns ) ;
	-------------------------------------*/
}
