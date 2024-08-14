/* -----------------------------------------------------------------------
 * bind.c     ss.library (shared)  Manx 5.0E1
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

/****** socket/bind ******************************************
*
*   NAME
*		bind -- bind a name to a socket
*
*   SYNOPSIS
*		return = bind(s, name, namelen, ss_err)
*		D0            D0 A0    D1       A1
*
*		int bind (int, struct sockaddr *, int, int *); 
*
*   FUNCTION
*		bind() assigns a name to an unnamed socket
*
*   INPUTS
*		s	a socket descriptor
*		name
*		namelen
*
*   RESULT
*		0 is returned if successful.  -1 is returned on error.
*		errno will contain the specific error.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/*
#pragma(bind(D0,A0,D1,A1))
*/

#include "fd_to_fh.h"
#include <socket.h>

bind( int s, caddr_t name, int len, int *ss_err )  /*** <<--  ***/
{                                                  /* args !!  **/
	struct bindargs ba ;

	ba.errno   = 0L ;
	GETSOCK(s, ba.fp) ;
	ba.name    = name ;
	ba.namelen = len ;
	BindAsm(&ba) ;
	*ss_err    = ba.errno ;

	return( *ss_err ? -1 : 0 ) ;
}

