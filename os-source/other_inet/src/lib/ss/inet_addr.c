/* -----------------------------------------------------------------------
 * inet_addr.c
 *
 * $Locker:  $
 *
 * $Id: inet_addr.c,v 1.5 92/07/21 17:12:15 bj Exp $
 *
 * $Revision: 1.5 $
 *
 * $Log:	inet_addr.c,v $
 * Revision 1.5  92/07/21  17:12:15  bj
 * socket.library 4.0
 * 
 * casts & comments.
 * 
 * Revision 1.2  91/08/07  13:17:06  bj
 * rcs header added.
 * 
 *
 * $Header: AS225:src/lib/ss/RCS/inet_addr.c,v 1.5 92/07/21 17:12:15 bj Exp $
 *
 *------------------------------------------------------------------------
 */
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

#include "sslib.h"
#include <fctype.h>
#include <clib/alib_stdio_protos.h>

/****** socket.library/inet_addr ******************************************
*
*   NAME
*	inet_addr -- make internet address from string
*
*   SYNOPSIS
*	#include <sys/types.h>
*	#include <sys/socket.h>
*	#include <netinet/in.h>
*
*	addr = inet_addr( string )
*	D0                A1
*
*	u_long inet_addr ( char * );
*
*   FUNCTION
*	Converts a string to an internet address.  All internet addresses
*	are in network order.
*
*   INPUTS
*	string		address string. "123.45.67.89" for example
*
*   RESULT
*	A long representing the network address in network byte order.
*
*	INADDR_NONE is returned if input is invalid.
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
 * Internet address interpretation routine.
 * All the network library routines call this
 * routine to interpret entries in the data bases
 * which are expected to be an address.
 * The value returned is in network order.
 */

u_long __saveds __asm inet_addr(register __a1 char *cp)
{
	register u_long val, base, n;
	register char c;
	u_long parts[4], *pp = parts;
//KPrintF("inet_addr: %s ",cp);
again:
	/*
	 * Collect number up to ``.''.
	 * Values are specified as for C:
	 * 0x=hex, 0=octal, other=decimal.
	 */
	val = 0; base = 10;
	if (*cp == '0') {
		if (*++cp == 'x' || *cp == 'X')
			base = 16, cp++;
		else
			base = 8;
	}
	while (c = *cp) {
		if (isdigit(c)) {
			val = (val * base) + (c - '0');
			cp++;
			continue;
		}
		if (base == 16 && isxdigit(c)) {
			val = (val << 4) + (c + 10 - (islower(c) ? 'a' : 'A'));
			cp++;
			continue;
		}
		break;
	}
	if (*cp == '.') {
		/*
		 * Internet format:
		 *	a.b.c.d
		 *	a.b.c	(with c treated as 16-bits)
		 *	a.b	(with b treated as 24 bits)
		 */
		if (pp >= parts + 4)
			return (INADDR_NONE);
		*pp++ = val, cp++;
		goto again;
	}
	/*
	 * Check for trailing characters.
	 */
	if (*cp && !isspace(*cp))
		return (INADDR_NONE);
	*pp++ = val;
	/*
	 * Concoct the address according to
	 * the number of parts specified.
	 */
	n = pp - parts;
	switch (n) {

	case 1:				/* a -- 32 bits */
		val = parts[0];
		break;

	case 2:				/* a.b -- 8.24 bits */
		val = (parts[0] << 24) | (parts[1] & 0xffffff);
		break;

	case 3:				/* a.b.c -- 8.8.16 bits */
		val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
			(parts[2] & 0xffff);
		break;

	case 4:				/* a.b.c.d -- 8.8.8.8 bits */
		val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
		      ((parts[2] & 0xff) << 8) | (parts[3] & 0xff);
		break;

	default:
		return (INADDR_NONE);
	}
	val = htonl(val);
//KPrintF(" %lx\n",val);
	return (val);
}


/****** socket.library/inet_makeaddr ******************************************
*
*   NAME
*	inet_makeaddr -- make internet address from network and host
*
*   SYNOPSIS
*	#include <sys/types.h>
*	#include <sys/socket.h>
*	#include <netinet/in.h>
*
*	addr = inet_makeaddr( net, lna )
*	D0                    D0   D1
*
*	struct in_addr inet_makeaddr( int, int );
*
*   FUNCTION
* 	Formulate an Internet address from network + host.  Used in
*	building addresses stored in the ifnet structure.
*
*	'in' is *NOT* a pointer to a struct in_addr, it IS a structure.
*	It is a 4-byte structure and is a passed as a structure (rather
*	than a long or pointer to a structure) for historical reasons.
*	See NOTES.
*

*   INPUTS
*	net		- network number in local integer format.
*	lna		- local node address in local integer format.
*
*   RESULT
*       in		- struct in_addr.
*
*	struct in_addr {
*		u_long s_addr; \* a long containing the internet address *\
*	};
*
*   EXAMPLE
*
*   NOTES
*	'in' is *NOT* a pointer to a struct in_addr, it IS a structure.
*	It is a 4-byte structure and is a passed as a structure (rather
*	than a long or pointer to a structure) for historical reasons.
*
*   BUGS
*
*   SEE ALSO
*	inet_addr(), inet_lnaof(), inet_netof()
*
******************************************************************************
*
*/

/*
 * Formulate an Internet address from network + host.  Used in
 * building addresses stored in the ifnet structure.
 */

static u_long addr = 0L;

struct in_addr __saveds __asm inet_makeaddr(
	register __d0 int net,
	register __d1 int host)
{

	if (net < 128)
		addr = (net << IN_CLASSA_NSHIFT) | (host & IN_CLASSA_HOST);
	else if (net < 65536)
		addr = (net << IN_CLASSB_NSHIFT) | (host & IN_CLASSB_HOST);
	else if (net < 16777216L)
		addr = (net << IN_CLASSC_NSHIFT) | (host & IN_CLASSC_HOST);
	else
		addr = net | host;
	addr = htonl(addr);
	return (*(struct in_addr *)&addr);
}


/****** socket.library/inet_lnaof ******************************************
*
*   NAME
*	inet_lnaof -- give the local network address
*
*   SYNOPSIS
*	#include <sys/types.h>
*	#include <sys/socket.h>
*	#include <netinet/in.h>
*
*	addr = inet_lnaof( in )
*	D0                 D1
*
*	int inet_lnaof ( struct in_addr );
*
*   FUNCTION
*	Returns the local network address.
*
*   INPUTS
*       in		- struct in_addr to find local network part of.
*
*	struct in_addr {
*		u_long s_addr; \* a long containing the internet address *\
*	};
*
*   RESULT
*	addr		-  the local network address portion of 'in.'
*
*   EXAMPLE
*
*   NOTES
*	'in' is *NOT* a pointer to a struct in_addr, it IS a structure.
*	It is a 4-byte structure and is a passed as a structure (rather
*	than a long or pointer to a structure) for historical reasons.
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
*	inet_addr(), inet_makeaddr(), inet_netof()
*
******************************************************************************
*
*/

/*
 * Return the local network address portion of an
 * internet address; handles class a/b/c network
 * number formats.
 */

int  __saveds __asm inet_lnaof( register __d1 struct in_addr in)
{
	register int i = ntohl(in.s_addr);

	if (IN_CLASSA(i))
		return ((i)&IN_CLASSA_HOST);
	else if (IN_CLASSB(i))
		return ((i)&IN_CLASSB_HOST);
	else
		return ((i)&IN_CLASSC_HOST);
}


/****** socket.library/inet_netof ******************************************
*
*   NAME
*	inet_netof -- give the network number of an address
*
*   SYNOPSIS
*	#include <sys/types.h>
*	#include <sys/socket.h>
*	#include <netinet/in.h>
*
*	net = inet_netof( in )
*	D0		  D1
*
*	int inet_netof( struct in_addr );
*
*   FUNCTION
*	Returns the network number.
*
*   INPUTS
*       in		- struct in_addr to find network part of.
*
*	struct in_addr {
*		u_long s_addr; \* a long containing the internet address *\
*	};
*
*   RESULT
*	net		- network part of 'in.'
*   EXAMPLE
*
*   NOTES
*	'in' is *NOT* a pointer to a struct in_addr, it IS a structure.
*	It is a 4-byte structure and is a passed as a structure (rather
*	than a long or pointer to a structure) for historical reasons.
*
*   BUGS
*
*   SEE ALSO
*	inet_addr(), inet_makeaddr(), inet_lnaof()
*
******************************************************************************
*
*/

int __saveds __asm inet_netof( register __d1 struct in_addr in)
{
	register u_long i = ntohl(in.s_addr);

	if (IN_CLASSA(i))
		return (int)(((i)&IN_CLASSA_NET) >> IN_CLASSA_NSHIFT);
	else if (IN_CLASSB(i))
		return (int)(((i)&IN_CLASSB_NET) >> IN_CLASSB_NSHIFT);
	else
		return (int)(((i)&IN_CLASSC_NET) >> IN_CLASSC_NSHIFT);
}


/****** socket.library/inet_network ******************************************
*
*   NAME
*	inet_network -- make network number from string
*
*   SYNOPSIS
*	#include <sys/types.h>
*	#include <sys/socket.h>
*	#include <netinet/in.h>
*
*	net = inet_network( string )
*	D0		    A1
*
*	int inet_network( char * );
*
*   FUNCTION
*	Converts a string to an network number if the string contains the
*	dotted-decimal representation of a network number. All network
*	numbers are in network order.
*
*   INPUTS
*	string		- network string. "123.45.67.89" for example.
*
*   RESULT
*	net		- INADDR_NONE is returned if input is invalid, else
*			  the network number corresponding to 'string.'
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

int __saveds __asm inet_network( register __a1 char *cp)
{
	register u_long val, base, n;
	register char c;
	u_long parts[4], *pp = parts;
	register int i;

again:
	val = 0; base = 10;
	if (*cp == '0')
		base = 8, cp++;
	if (*cp == 'x' || *cp == 'X')
		base = 16, cp++;
	while (c = *cp) {
		if (isdigit(c)) {
			val = (val * base) + (c - '0');
			cp++;
			continue;
		}
		if (base == 16 && isxdigit(c)) {
			val = (val << 4) + (c + 10 - (islower(c) ? 'a' : 'A'));
			cp++;
			continue;
		}
		break;
	}
	if (*cp == '.') {
		if (pp >= parts + 4)
			return (INADDR_NONE);
		*pp++ = val, cp++;
		goto again;
	}
	if (*cp && !isspace(*cp))
		return (INADDR_NONE);
	*pp++ = val;
	n = pp - parts;
	if (n > 4)
		return (INADDR_NONE);
	for (val = 0, i = 0; i < n; i++) {
		val <<= 8;
		val |= parts[i] & 0xff;
	}
	return ((int)val);
}



/****** socket.library/inet_ntoa ******************************************
*
*   NAME
*	inet_ntoa -- turn internet address into string
*
*   SYNOPSIS
*	#include <sys/types.h>
*	#include <sys/socket.h>
*	#include <netinet/in.h>
*
*	string = inet_ntoa( in )
*	D0                  D1
*
*	char *inet_ntoa ( struct in_addr );
*
*   FUNCTION
*	Converts an internet address to an ASCII string in the format
*	"a.b.c.d" (dotted-decimal notation).
*
*   INPUTS
*       in		- struct in_addr.
*
*	struct in_addr {
*		u_long s_addr; \* a long containing the internet address *\
*	};
*
*   RESULT
*	Pointer to a string containing the ASCII ethernet address.
*	For example, if in.s_addr = 0xc009d203 then a pointer
*	to the string "192.9.210.3" is returned.
*
*   NOTES
*	The result points to a static buffer that is overwritten
*	with each call.
*
*	'in' is *NOT* a pointer to a struct in_addr, it IS a structure.
*	It is a 4-byte structure and is a passed as a structure (rather
*	than a long or pointer to a structure) for historical reasons.
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

char * __saveds __asm inet_ntoa( register __d1 u_long l )
{
	unsigned int a, b, c, d;
	static char ntoabuf[20];

	d = l & 0xff; l >>= 8;
	c = l & 0xff; l >>= 8;
	b = l & 0xff; l >>= 8;
	a = l & 0xff;

	sprintf(ntoabuf, "%ld.%ld.%ld.%ld", a, b, c, d);

	return ( ntoabuf );
}


