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

/****** socket/getnetbyaddr ******************************************
*
*   NAME
*		getnetbyaddr -- get net entry from the inet:db/networks file
*
*   SYNOPSIS
*		#include <netdb.h>
*
*		net = getnetbyaddr ( net, type )
*
*		struct netent *getnetent ( long, int ); 
*
*   FUNCTION
*		Opens the networks file if necessary.  Returns the entry
*		with a matching address in a nettent structure.
*
*		struct	netent {
*			char		*n_name;		official name of net 
*			char		**n_aliases;	alias list
*			int			n_addrtype;		net address type
*			unsigned long	n_net;		network #
*		};
*
*	INPUTS
*		net		network number
*		type	network number type, currently AF_INET
*
*   RESULT
*		Returns NULL if no match.
*		Returns NULL on EOF or failure to open networks file.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*		setnetent(), getnetbyname(), getnetent(), endnetent()
*
******************************************************************************
*
*/


#include <netdb.h>

extern int _net_stayopen;

struct netent *
getnetbyaddr(net, type)
	register long net;
	register int type;
{
	register struct netent *p;

	setnetent(_net_stayopen);
	while (p = getnetent())
		if (p->n_addrtype == type && p->n_net == net)
			break;
	if (!_net_stayopen)
		endnetent();
	return (p);
}
