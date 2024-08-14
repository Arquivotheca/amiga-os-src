/* -----------------------------------------------------------------------
 * gethostent.c   LATTICE
 *
 * $Locker:  $
 *
 * $Id: gethostent.c,v 1.2 90/11/28 16:49:15 bj Exp $
 *
 * $Revision: 1.2 $
 *
 * $Header: HOG:Other/inet/src/lib/net2/RCS/gethostent.c,v 1.2 90/11/28 16:49:15 bj Exp $
 *
 * $Log:	gethostent.c,v $
 * Revision 1.2  90/11/28  16:49:15  bj
 * Added "__aligned" keyword to the "hostaddr[]" variable as
 * a fix to the crashes this was causing on 68000 machines.
 * Compiler was putting this on a non-word-aligned boundary.
 * The gethostent() function would then try to write a LONG
 * value into hostaddr[0] with a resulting bang.
 * 
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

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ctype.h>
#ifndef AMIGA
#include <ndbm.h>
#endif
/*
 * Internet version.
 */
#define	MAXALIASES	35
#define	MAXADDRSIZE	14

static FILE *hostf = NULL;
static char line[BUFSIZ+1];
#ifdef LATTICE
static char __aligned hostaddr[MAXADDRSIZE];
#else
static char hostaddr[MAXADDRSIZE];
#endif
static struct hostent host;
static char *host_aliases[MAXALIASES];
static char *host_addrs[] = {
	hostaddr,
	NULL
};

/*
 * The following is shared with gethostnamadr.c
 */
char	*_host_file = HOSTFILE;
int	_host_stayopen;
#ifndef AMIGA
DBM	*_host_db;	/* set by gethostbyname(), gethostbyaddr() */
#endif
static char *any();


/****** socket/sethostent ******************************************
*
*   NAME
*		sethostent -- rewind the inet:db/hosts file
*
*   SYNOPSIS
*		sethostent ( stayopen )
*
*		void sethostent ( int ); 
*
*   FUNCTION
*		Rewinds the host file if it is open.
*		If the stayopen flag is zero, the host file will be
*		closed after each call to gethostent().
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
*		gethostent(), gethostbyname(), gethostbyaddr(), endhostent()
*
******************************************************************************
*
*/

void
sethostent(f)
	int f;
{
	if (hostf != NULL)
		rewind(hostf);
	_host_stayopen |= f;
}

/****** socket/endhostent ******************************************
*
*   NAME
*		endhostent -- closes the inet:db/hosts file
*
*   SYNOPSIS
*		endhostent ()
*
*		void endhostent (void); 
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
*		gethostent(), gethostbyname(), gethostbyaddr(), sethostent()
*
******************************************************************************
*
*/

void
endhostent()
{
	if (hostf) {
		fclose(hostf);
		hostf = NULL;
	}
#ifndef AMIGA
	if (_host_db) {
		dbm_close(_host_db);
		_host_db = (DBM *)NULL;
	}
#endif
	_host_stayopen = 0;
}

/****** socket/gethostent ******************************************
*
*   NAME
*		gethostent -- get host entry from the inet:db/hosts file
*
*   SYNOPSIS
*		#include <sys/socket.h>
*		#include <netdb.h>
*
*		host = gethostent ( )
*
*		struct hostent *gethostent ( void ); 
*
*   FUNCTION
*		Opens the host file if necessary.  Returns the next entry
*		in the file in a hostent structure.
*
*		struct	hostent {
*			char	*h_name;		official name of host
*			char	**h_aliases;	alias list
*			int	h_addrtype;			host address type
*			int	h_length;			length of address
*			char	**h_addr_list;	list of addresses from name server
*		#define	h_addr	h_addr_list[0]	
*		};
*
*	INPUTS
*
*   RESULT
*		Returns NULL on EOF or failure to open hosts file.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*		sethostent(), gethostbyname(), gethostbyaddr(), endhostent()
*
******************************************************************************
*
*/

struct hostent *
gethostent()
{
	char *p;
	register char *cp, **q;

	if (hostf == NULL && (hostf = fopen(_host_file, "r" )) == NULL)
		return (NULL);
again:
	if ((p = fgets(line, BUFSIZ, hostf)) == NULL)
		return (NULL);
	if (*p == '#')
		goto again;
	cp = any(p, "#\n");
	if (cp == NULL)
		goto again;
	*cp = '\0';
	cp = any(p, " \t");
	if (cp == NULL)
		goto again;
	*cp++ = '\0';
	/* THIS STUFF IS INTERNET SPECIFIC */
	host.h_addr_list = host_addrs;
	*((u_long *)host.h_addr) = inet_addr(p);
	host.h_length = sizeof (u_long);
	host.h_addrtype = AF_INET;
	while (*cp == ' ' || *cp == '\t')
		cp++;
	host.h_name = cp;
	q = host.h_aliases = host_aliases;
	cp = any(cp, " \t");
	if (cp != NULL) 
		*cp++ = '\0';
	while (cp && *cp) {
		if (*cp == ' ' || *cp == '\t') {
			cp++;
			continue;
		}
		if (q < &host_aliases[MAXALIASES - 1])
			*q++ = cp;
		cp = any(cp, " \t");
		if (cp != NULL)
			*cp++ = '\0';
	}
	*q = NULL;
	return (&host);
}

/****** socket/sethostfile ******************************************
*
*   NAME
*		sethostfile -- changes the name of the host file
*
*   SYNOPSIS
*		sethostfile ( filename )
*
*		void sethostfile ( char * ); 
*
*   FUNCTION
*		Changes the name of the hosts file.  The default is
*		inet:db/hosts.
*
*	INPUTS
*		filename	name of the new hosts file
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
*		gethostent(), gethostbyname(), gethostent(), endhostent()
*
******************************************************************************
*
*/

void
sethostfile(file)
	char *file;
{
	_host_file = file;
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
