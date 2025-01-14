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


/****** socket/getnetbyname ******************************************
*
*   NAME
*		getnetbyname -- get net entry from the inet:db/networks file
*
*   SYNOPSIS
*		#include <netdb.h>
*
*		net = getnetbyname ( name )
*
*		struct netent *getnetbyname ( char * ); 
*
*   FUNCTION
*		Opens the networks file if necessary.  Returns the entry
*		with a matching name in a nettent structure.
*
*		struct	netent {
*			char		*n_name;		official name of net
*			char		**n_aliases;	alias list
*			int		n_addrtype;			net address type
*			unsigned long	n_net;		network #
*		};
*
*	INPUTS
*		name		network name
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
*		setnetent(), getnetbyaddr(), getnetent(), endnetent()
*
******************************************************************************
*
*/

#include <netdb.h>

extern int _net_stayopen;

struct netent *
getnetbyname(name)
	register char *name;
{
	register struct netent *p;
	register char **cp;

	setnetent(_net_stayopen);
	while (p = getnetent()) {
		if (strcmp(p->n_name, name) == 0)
			break;
		for (cp = p->n_aliases; *cp != 0; cp++)
			if (strcmp(*cp, name) == 0)
				goto found;
	}
found:
	if (!_net_stayopen)
		endnetent();
	return (p);
}
