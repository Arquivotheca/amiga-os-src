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

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ctype.h>

#define	MAXALIASES	35

#ifdef NETFILE
static char NETDB[] = NETFILE;
#else
static char NETDB[] = "/etc/networks";
#endif

static FILE *netf = NULL;
static char line[BUFSIZ+1];
static struct netent net;
static char *net_aliases[MAXALIASES];
int _net_stayopen;
static char *any();

/****** socket/setnetent ******************************************
*
*   NAME
*		setnetent -- open or rewind the inet:db/networks file
*
*   SYNOPSIS
*		setnetent ( stayopen )
*
*		void setnetent ( int ); 
*
*   FUNCTION
*		Rewinds the network file if it is open.
*		If the stayopen flag is zero, the network file will be
*		closed after each call to getnetent().
*
*	INPUTS
*		stayopen	integer flag
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
*		getnetent(), getnetbyname(), getnetbyaddr(), endnetent()
*
******************************************************************************
*
*/

setnetent(f)
	int f;
{
	if (netf == NULL)
		netf = fopen(NETDB, "r" );
	else
		rewind(netf);
	_net_stayopen |= f;
}

/****** socket/endnetent ******************************************
*
*   NAME
*		endnetent -- closes the inet:db/networks file
*
*   SYNOPSIS
*		endnetent ()
*
*		void endnetent (void); 
*
*   FUNCTION
*		Closes the host file if it is open.
*
*	INPUTS
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
*		getnetent(), getnetbyname(), getnetbyaddr(), setnetent()
*
******************************************************************************
*
*/

void
endnetent()
{
	if (netf) {
		fclose(netf);
		netf = NULL;
	}
	_net_stayopen = 0;
}


/****** socket/getnetent ******************************************
*
*   NAME
*		getnetent -- get net entry from the inet:db/networks file
*
*   SYNOPSIS
*		#include <netdb.h>
*
*		net = getnetent ( )
*
*		struct netent *getnetent ( void ); 
*
*   FUNCTION
*		Opens the networks file if necessary.  Returns the next entry
*		in the file in a nettent structure.
*
*		struct	netent {
*			char		*n_name;		official name of net
*			char		**n_aliases;	alias list
*			int		n_addrtype;			net address type
*			unsigned long	n_net;		network #
*		};
*
*	INPUTS
*
*   RESULT
*		Returns NULL on EOF or failure to open networks file.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*		setnetent(), getnetbyname(), getnetbyaddr(), endnetent()
*
******************************************************************************
*
*/


struct netent *
getnetent()
{
	char *p;
	register char *cp, **q;

	if (netf == NULL && (netf = fopen(NETDB, "r" )) == NULL)
		return (NULL);
again:
	p = fgets(line, BUFSIZ, netf);
	if (p == NULL)
		return (NULL);
	if (*p == '#')
		goto again;
	cp = any(p, "#\n");
	if (cp == NULL)
		goto again;
	*cp = '\0';
	net.n_name = p;
	cp = any(p, " \t");
	if (cp == NULL)
		goto again;
	*cp++ = '\0';
	while (*cp == ' ' || *cp == '\t')
		cp++;
	p = any(cp, " \t");
	if (p != NULL)
		*p++ = '\0';
	net.n_net = inet_network(cp);
	net.n_addrtype = AF_INET;
	q = net.n_aliases = net_aliases;
	if (p != NULL) 
		cp = p;
	while (cp && *cp) {
		if (*cp == ' ' || *cp == '\t') {
			cp++;
			continue;
		}
		if (q < &net_aliases[MAXALIASES - 1])
			*q++ = cp;
		cp = any(cp, " \t");
		if (cp != NULL)
			*cp++ = '\0';
	}
	*q = NULL;
	return (&net);
}

static char *
any(cp, match)
	register char *cp;
	char *match;
{
	register char *mp, c;

	while (c = *cp) {
		for (mp = match; *mp; mp++)
			if (*mp == c)
				return (cp);
		cp++;
	}
	return ((char *)0);
}
