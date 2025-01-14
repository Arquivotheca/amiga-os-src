/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */


#include <sys/types.h>
#include <netinet/in.h>

/****** socket/inet_netof ******************************************
*
*   NAME
*		inet_netof -- give the network number
*
*   SYNOPSIS
*		#include <sys/types.h>
*		#include <sys/socket.h>
*		#include <netinet/in.h>
*		#include <arpa/inet.h>
*
*		net = inet_netof( in )
*
*		u_long inet_netof ( struct in_addr ); 
*
*   FUNCTION
*		Returns the network number
*
*	INPUTS
*		in	
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*		inet_addr(), inet_makeaddr(), inet_lnaof()
*
******************************************************************************
*
*/

/*
 * Return the network number from an internet
 * address; handles class a/b/c network #'s.
 */
u_long
inet_netof(in)
	struct in_addr in;
{
	register u_long i = ntohl(in.s_addr);

	if (IN_CLASSA(i))
		return (((i)&IN_CLASSA_NET) >> IN_CLASSA_NSHIFT);
	else if (IN_CLASSB(i))
		return (((i)&IN_CLASSB_NET) >> IN_CLASSB_NSHIFT);
	else
		return (((i)&IN_CLASSC_NET) >> IN_CLASSC_NSHIFT);
}
