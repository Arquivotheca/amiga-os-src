/* @(#)bindresvport.c	1.1 87/11/04 3.9 RPCSRC */
/*
 * Sun RPC is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify Sun RPC without charge, but are not authorized
 * to license or distribute it to anyone else except as part of a product or
 * program developed by the user.
 *
 * SUN RPC IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 *
 * Sun RPC is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 *
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY SUN RPC
 * OR ANY PART THEREOF.
 *
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 *
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */


/*
 * Copyright (c) 1987 by Sun Microsystems, Inc.
 */

/****** socket/bindresvport ******************************************
*
*   NAME
*		bindresvport -- Bind a socket to a privileged IP port
*
*   SYNOPSIS
*		return = bindresvport ( sd, sin )
*
*		int bindresvport ( int, struct sockaddr_in *);
*
*   FUNCTION
*		bindresvport is used to bind a socket descriptor to a
*		priveleged IP port (in the range 0-1023).
*
*
*	INPUTS
*		sd
*
*		sin
*
*   RESULT
*		0 is returned if successful.  -1 is returned on error.
*		errno will be set to the specific error code.
*
*   EXAMPLE
*
*   NOTES
*		rresvport() is equivalent to bindresvport() except that
*		rresvport() only works for TCP sockets.
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

#include <ss/socket.h>
#include <rpc/types.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>

bindresvport(sd, sin)
	int sd;
	struct sockaddr_in *sin;
{
	int res;
	static short port;
	struct sockaddr_in myaddr;
	extern int errno;
	int i;

#define STARTPORT 600
#define ENDPORT (IPPORT_RESERVED - 1)
#define NPORTS	(ENDPORT - STARTPORT + 1)

	if (sin == (struct sockaddr_in *)0) {
		sin = &myaddr;
		bzero(sin, sizeof (*sin));
		sin->sin_family = AF_INET;
	} else if (sin->sin_family != AF_INET) {
#ifdef AMIGA
		errno = EPROTOTYPE;
#else
		errno = EPFNOSUPPORT;
#endif
		return (-1);
	}
	if (port == 0) {
		port = (getpid() % NPORTS) + STARTPORT;
	}
	for (i = 0; i < NPORTS; i++) {
		sin->sin_port = htons(port++);
		if (port > ENDPORT) {
			port = STARTPORT;
		}
		res = bind(sd, sin, sizeof(struct sockaddr_in));
		if (res == 0) {
			return (0);
		}
		if ((res < 0) && (errno == EACCES)) {
			return (-1);
		}
	}
	return (-1);
}
