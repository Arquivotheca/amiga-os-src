/* -----------------------------------------------------------------------
 * fd_set.c
 *
 * $Locker:  $
 *
 * $Id: fd_set.c,v 1.2 91/08/07 13:34:15 bj Exp $
 *
 * $Revision: 1.2 $
 *
 * $Log:	fd_set.c,v $
 * Revision 1.2  91/08/07  13:34:15  bj
 * rcs header added.
 * 
 *
 * $Header: AS225:src/lib/ss/RCS/fd_set.c,v 1.2 91/08/07 13:34:15 bj Exp $
 *
 *------------------------------------------------------------------------
 */
/****** socket.library/FD_SET ******************************************
*
*   NAME
*	FD_SET -- Macros to manipulate socket masks.
*
*   SYNOPSIS
*	#include <sys/types.h>
*
*	FD_SET(socket, mask)
*	FD_CLR(socket, mask)
*	result = FD_ISSET(socket, mask)
*	FD_ZERO(mask)
*
*   FUNCTION
*	Type fd_set contains masks for use with sockets and select() just
*	like longs can contain masks for use with signals and Wait().
*	Unlike signal masks, you can't manipulate socket masks with boolean
*	operators, instead you must use the FD_*() macros.
*
*	FD_SET() sets the "bit" for 'socket' in 'mask.'
*
*	FD_CLR() clears the "bit" for 'socket' in 'mask.'
*
*	FD_ISSET() returns non-zero if the "bit" for 'socket' in 'mask' is
*	set, else zero.
*
*	FD_ZERO() clears all "bits" in 'mask.'  FD_ZERO should be used
*	to initialize an fd_set variable before it is used.
*
*   EXAMPLE
*
*   SEE ALSO
*	select(), selectwait()
*
******************************************************************************
*
*/
