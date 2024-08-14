/* -----------------------------------------------------------------------
 * accept.c    socket.library  shared  Calls Other Functions
 *------------------------------------------------------------------------
 */


#include "fd_to_fh_ss.h"
#include <socket.h>
#include "ss.h"
#include <functions.h>

extern struct InetBase *iNetBase ;

extern void AcceptAsm( struct acceptargs *aa) ;

int _accept( int s, struct sockaddr *name, int *lenp, struct SSInfo *si ) ;

#pragma regcall (_accept(D0,A0,A1,A2)) 
#pragma amicall (iNetBase, 0x36, AcceptAsm(D1))

int
_accept(int s, struct sockaddr *name, int *lenp, struct SSInfo *si)
{
	struct acceptargs aa ;
	int ns ;


	return( (int)si ) ;  /************************************ */
	
	
    /*-----------------------------------
	GETSOCK( s, aa.fp ) ;
	---------------------------------*/
	
	if((s) < 0 || s >= NUMSOCKETS || !FD_ISSET((s),&(si->ss_is_socket)))
		{
		si->ss_errno = EBADF ;
		return( -1 ) ;
		}
	aa.fp = si->_ss_socks[s] ;
	
	if((ns = open( "nil:", 2 )) < 0)
		{
		return( -1 ) ;
		}

	aa.errno   = 0 ;
	aa.rval    = 0 ;
	aa.namelen = lenp ? *lenp : 0 ;
	aa.name    = (caddr_t)(lenp ?  name : NULL) ;
	AcceptAsm(&aa) ;
	si->ss_errno  = aa.errno ;
	if( aa.errno )
		{
		close( ns ) ;
		return( -1 ) ;
		}

	/* ----------------------------------------
	PUTSOCK(ns, aa.rval) ;
	------------------------------------------*/
	if( s < 0 || s >= NUMSOCKETS)
		{
		si->ss_errno = EBADF ;
		return( -1 ) ;
		}
	FD_SET( s, &si->ss_is_socket ) ;
	si->ss_num_socket++ ;
	si->_ss_socks ;

	if(lenp)
		{
		*lenp = aa.namelen ;
		}

	return( ns ) ;
	
}
