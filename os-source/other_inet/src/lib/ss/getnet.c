/* -----------------------------------------------------------------------
 * getnet.c
 *
 * $Locker:  $
 *
 * $Id: getnet.c,v 1.3 92/07/21 16:30:04 bj Exp $
 *
 * $Revision: 1.3 $
 *
 * $Log:	getnet.c,v $
 * Revision 1.3  92/07/21  16:30:04  bj
 * Socket.library 4.0
 * removed _asm key word from functions.
 * 
 * Revision 1.2  91/08/07  13:20:05  bj
 * rcs header added.
 * 
 *
 * $Header: AS225:src/lib/ss/RCS/getnet.c,v 1.3 92/07/21 16:30:04 bj Exp $
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

#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>

#define any(cp,match)	strpbrk(cp,match)

BPTR netf =NULL;
UBYTE net_stayopen = (UBYTE)0;
static char line[128];
static struct netent net;
static char *net_aliases[MAXALIASES];

/****** socket.library/setnetent ******************************************
*
*   NAME
*	setnetent -- Open or rewind the networks file.
*
*   SYNOPSIS
*	setnetent( flag )
*                  D1
*
*	void setnetent( int );
*
*   FUNCTION
*	Rewinds the network file if it is open.
*	Opens the network file if it was not open.
*
*   INPUTS
*	flag		- if 'flag' is 1, calls to getnetbyname() and
*			  getnetbyaddr() will not close the file between
*			  calls.  You must close the file with an endnetent().
*			  Once set, 'flag' cannot be reset except by calling
*			  endnetent().
*
*   RESULT
*	None.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*	getnetent(), getnetbyname(), getnetbyaddr(), endnetent()
*
******************************************************************************
*
*/

void __saveds __asm setnetent(register __d1 int flag)
{
	if (netf == NULL)
		netf = Open(NETFILE, MODE_OLDFILE);
	else
		Seek(netf,0L,OFFSET_BEGINNING);

	net_stayopen |= (UBYTE)flag;
}

/****** socket.library/endnetent ******************************************
*
*   NAME
*	endnetent -- Closes the networks file.
*
*   SYNOPSIS
*	endnetent( )
*
*	void endnetent( void );
*
*   FUNCTION
*	Closes the host file if it is open.  This is only needed if
*	setnetent() or getnetent() has been called.
*
*   INPUTS
*	None.
*
*   RESULT
*	None.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*	getnetent(), getnetbyname(), getnetbyaddr(), setnetent()
*
******************************************************************************
*
*/

void __saveds endnetent( void )
{
	if (netf) {
		Close(netf);
		netf = NULL;
		net_stayopen = (UBYTE)0;
	}
}


/****** socket.library/getnetent ******************************************
*
*   NAME
*	getnetent -- Get net entry from the networks file.
*
*   SYNOPSIS
*	#include <netdb.h>
*
*	net = getnetent( )
*	D0
*
*	struct netent *getnetent( void );
*
*   FUNCTION
*	This function should not normally be used.  It is called internally
*	by getnetbyname() and getnetbyaddr().
*
*	Opens the networks file if necessary.  Returns the next entry
*	in the file in a nettent structure.
*
*	struct	netent {
*		char		*n_name;	\* official name of net *\
*		char		**n_aliases;	\* alias list *\
*		int		n_addrtype;	\* net address type *\
*		unsigned long	n_net;		\* network # *\
*	};
*
*   INPUTS
*	None.
*
*   RESULT
*	net		- netent structure if succesful, NULL on EOF on
*			  failure to open networks file.
*
*   EXAMPLE
*
*   NOTES
*	The netent structure is returned in a buffer that will be
*	overwritten on the next call to getnet*().
*
*   BUGS
*
*   SEE ALSO
*	setnetent(), getnetbyname(), getnetbyaddr(), endnetent()
*
******************************************************************************
*
*/


struct netent * __saveds getnetent( void )
{
	char *p, *cp, **q;

	/* if we haven't opened the networks file, open it */
	if(netf == NULL){
		setnetent(0);
		if(netf == NULL)
			return(NULL);
	}

again:
	/* read a line into buffer 3 */
	if ((p = FGets(netf, line, 127)) == NULL)
		return (NULL);

	/* check for comment */
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
		if (q < &net_aliases[MAXALIASES-1])
			*q++ = cp;
		cp = any(cp, " \t");
		if (cp != NULL)
			*cp++ = '\0';
	}
	*q = NULL;
	return (&net);
}

/****** socket.library/getnetbyaddr ******************************************
*
*   NAME
*	getnetbyaddr -- Get net entry from the networks file.
*
*   SYNOPSIS
*	#include <netdb.h>
*
*	net = getnetbyaddr( net, type )
*	D0                  D0   D1
*
*	struct netent *getnetbyaddr( long, int );
*
*   FUNCTION
*	Opens the networks file if necessary.  Returns the entry
*	with a matching address in a nettent structure.
*
*	struct	netent {
*		char		*n_name;	\* official name of net *\
*		char		**n_aliases;	\* alias list *\
*		int		n_addrtype;	\* net address type *\
*		unsigned long	n_net;		\* network # *\
*	};
*
*   INPUTS
*	net		- network number.
*	type		- network number type, currently AF_INET.
*
*   RESULT
*	net		- netent structure if succesful, NULL on EOF on
*			  failure to open networks file.
*
*   EXAMPLE
*
*   NOTES
*	The netent structure is returned in a buffer that will be
*	overwritten on the next call to getnet*().
*
*   BUGS
*
*   SEE ALSO
*	getnetbyname()
*
******************************************************************************
*
*/


struct netent * __saveds __asm getnetbyaddr(
	register __d0 long net,
	register __d1 int type)
{
	struct netent *p;

	setnetent(0);

	while (p = getnetent()) {
		if (p->n_addrtype == type && p->n_net == net)
			break;
    }

    if(!net_stayopen)
		endnetent();
	return (p);
}


/****** socket.library/getnetbyname ******************************************
*
*   NAME
*	getnetbyname -- Get net entry from the networks file.
*
*   SYNOPSIS
*	#include <netdb.h>
*
*	net = getnetbyname( name )
*	D0                  A0
*
*	struct netent *getnetbyname ( char * );
*
*   FUNCTION
*	Opens the networks file if necessary.  Returns the entry
*	with a matching name in a nettent structure.
*
*	struct	netent {
*		char		*n_name;	\* official name of net *\
*		char		**n_aliases;	\* alias list *\
*		int		n_addrtype;	\* net address type *\
*		unsigned long	n_net;		\* network # *\
*	};
*
*   INPUTS
*	name		- network name.
*
*   RESULT
*	net		- netent structure if succesful, NULL on EOF on
*			  failure to open networks file.
*
*   EXAMPLE
*
*   NOTES
*	The netent structure is returned in a buffer that will be
*	overwritten on the next call to getnet*().
*
*   BUGS
*
*   SEE ALSO
*	getnetbyaddr()
*
******************************************************************************
*
*/


struct netent * __saveds __asm getnetbyname(
	register __a0 char *name)
{
	struct netent *p;
	char **cp;

	setnetent(0);

	while (p = getnetent()) {
		if (strcmp(p->n_name, name) == 0)
			break;
		for (cp = p->n_aliases; *cp != 0; cp++)
			if (strcmp(*cp, name) == 0)
				goto found;
	}
found:
	if(!net_stayopen)
		endnetent();
	return (p);
}
