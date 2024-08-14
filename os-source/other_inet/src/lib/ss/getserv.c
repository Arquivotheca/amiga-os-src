/* -----------------------------------------------------------------------
 * getserv.c
 *
 * $Locker:  $
 *
 * $Id: getserv.c,v 1.3 92/07/21 16:28:11 bj Exp $
 *
 * $Revision: 1.3 $
 *
 * $Log:	getserv.c,v $
 * Revision 1.3  92/07/21  16:28:11  bj
 * 4.0 socket.library.
 * removed _asm keyword from lib function calls.
 * 
 * Revision 1.2  91/08/07  13:19:00  bj
 * rcs header added.
 * 
 *
 * $Header: AS225:src/lib/ss/RCS/getserv.c,v 1.3 92/07/21 16:28:11 bj Exp $
 *
 *------------------------------------------------------------------------
 */
/*
** getservent(), getservbyport(), getservbyname(), setservent(),
** endservent()
**
*/

#include "sslib.h"

#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>

#define any(cp,match)	strpbrk(cp,match)

BPTR servfp =NULL;
UBYTE serv_stayopen = (UBYTE)0;

/****** socket.library/getservent ******************************************
*
*   NAME
*	getservent -- Get a service entry from the services file.
*
*   SYNOPSIS
*	#include <netdb.h>
*
*	serv = getservent( void )
*	D0
*
*	struct servent *getservent( void );
*
*   FUNCTION
*	There is normally no reason to use this function.  It is used
*	internally by getservbyname() and getservbyport().  It is
*	provided only for Un*x compatibility.
*
*	Opens the services file if necessary.  Returns the next entry
*	in the file in a servent structure.
*
*	struct	servent {
*		char	*s_name;	\* official service name *\
*		char	**s_aliases;	\* alias list *\
*		int	s_port;		\* port # *\
*		char	*s_proto;	\* protocol to use *\
*	};
*
*   INPUTS
*	None.
*
*   RESULT
*	serv		- pointer to struct servent for next service.
*			  NULL on EOF or failure to open protocols file.
*
*   EXAMPLE
*
*   NOTES
*	The servent structure is returned in a buffer that will be
*	overwritten on the next call to getserv*()
*
*   BUGS
*
*   SEE ALSO
*	setservent(), getservbyname(), getservbyport(), endservent()
*
******************************************************************************
*
*/

struct servent * __saveds getservent(void)
{

	char *p;
	char *cp, **q;
	static struct servent serv;
	static char *aliases[MAXALIASES];
	static char line[128];

	/* if we haven't opened the services file, open it */
	if(servfp == NULL){
		setservent(0);
		if(servfp == NULL)
			return(NULL);
	}

again:
	/* read a line into 'line' buffer */
	if ((p = FGets(servfp, line, 127)) == NULL)
		return (NULL);

	/* check for comment */
	if (*p == '#')
		goto again;

	/* check for end of line or start of comment */
	cp = any(p, "#\n");
	if (cp == NULL)
		goto again;
	*cp = '\0';
	serv.s_name = p;

	p = any(p, " \t");
	if (p == NULL)
		goto again;
	*p++ = '\0';
	while (*p == ' ' || *p == '\t')
		p++;
	cp = any(p, ",/");
	if (cp == NULL)
		goto again;
	*cp++ = '\0';

	serv.s_port = htons((u_short)atoi(p));
	serv.s_proto = cp;

	q = serv.s_aliases = aliases;
	cp = any(cp, " \t");
	if (cp != NULL)
		*cp++ = '\0';
	while (cp && *cp) {
		if (*cp == ' ' || *cp == '\t') {
			cp++;
			continue;
		}
		if (q < &aliases[MAXALIASES-1])
			*q++ = cp;
		cp = any(cp, " \t");
		if (cp != NULL)
			*cp++ = '\0';
	}
	*q = NULL;
	return (&serv);
}

/****** socket.library/getservbyname ****************************************
*
*   NAME
*	getservbyname -- Find a service entry by name.
*
*   SYNOPSIS
*	#include <netdb.h>
*
*	serv = getservbyname( name, proto )
*	D0                    A0    A1
*
*	struct servent *getservbyname( char *, char * );
*
*   FUNCTION
*	Opens the services file and finds the service with the matching
*	name and protocol.
*
*	struct	servent {
*		char	*s_name;	\* official service name *\
*		char	**s_aliases;	\* alias list *\
*		int	s_port;		\* port # *\
*		char	*s_proto;	\* protocol to use *\
*	};
*
*
*   INPUTS
*	name		- name of service to look up.
*	proto		- protocol of service to look up.
*
*   RESULT
*	serv		- pointer to struct servent for service 'name.'
*			  NULL on EOF or failure to open protocols file.
*
*   EXAMPLE
*
*   NOTES
*	The servent structure is returned in a buffer that will be
*	overwritten on the next call to getserv*().
*
*   BUGS
*
*   SEE ALSO
*	getservent(), getservbyport()
*
******************************************************************************
*
*/

struct servent * __saveds __asm getservbyname(register __a0 char *name,
	register __a1 char *proto)
{
	register struct servent *p;
	register char **cp;

    /* open or rewind file */
	setservent(0);

	while (p = getservent()) {
		if (strcmp(name, p->s_name) == 0)
			goto gotname;
		for (cp = p->s_aliases; *cp; cp++)
			if (strcmp(name, *cp) == 0)
				goto gotname;
		continue;
gotname:
		if (proto == 0 || strcmp(p->s_proto, proto) == 0)
			break;
	}

    if (!serv_stayopen)
		endservent();

	return (p);
}

/****** socket.library/setservent ******************************************
*
*   NAME
*	setservent -- Open or rewind the services file.
*
*   SYNOPSIS
*	setservent( flag )
*	            D1
*
*	void setservent( int );
*
*   FUNCTION
*	This function is rarely useful.
*
*	Opens the services file if necessary.
*	Rewinds the services file if it is open.
*
*   INPUTS
*	flag		- if 'flag' is 1, calls to getservbyport() and
*			  getservbyname() will not close the file between
*			  calls.  You must close the file with an
*			  endservent().  Once set, 'flag' cannot be reset
*			  except by calling endservent().
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*	getservent(), endservent()
*
******************************************************************************
*
*/


void __saveds __asm setservent(register __d1 int flag)
{
	if(servfp == NULL)
		servfp = Open(SERVFILE,MODE_OLDFILE);
	else
		Seek(servfp,0L,OFFSET_BEGINNING);

	serv_stayopen |= (UBYTE)flag;
}


/****** socket.library/endservent ******************************************
*
*   NAME
*	endservent -- Closes the services file.
*
*   SYNOPSIS
*	endservent()
*
*	void endservent( void );
*
*   FUNCTION
*	Closes the services file if it is open.  This function is needed
*	only if setservent() or getservent() have been called.
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
*	setservent(), getservent()
*
******************************************************************************
*
*/

void __saveds endservent( void )
{
	if(servfp != NULL){
		Close(servfp);
		servfp = NULL;
		serv_stayopen = (UBYTE)0;
	}
}


/****** socket.library/getservbyport ******************************************
*
*   NAME
*	getservbyport -- Find a service entry by port.
*
*   SYNOPSIS
*	#include <netdb.h>
*
*	serv = getservbyport( port, proto )
*	D0                    D0    A0
*
*	struct servent *getservbyport( u_short, char * );
*
*   FUNCTION
*	Opens the services file and finds the service with the matching
*	port and protocol.
*
*	struct	servent {
*		char	*s_name;	\* official service name *\
*		char	**s_aliases;	\* alias list *\
*		int	s_port;		\* port # *\
*		char	*s_proto;	\* protocol to use *\
*	};
*
*   INPUTS
*
*   RESULT
*	serv		- pointer to struct servent for service 'name.'
*			  NULL on EOF or failure to open protocols file.
*
*   EXAMPLE
*
*   NOTES
*	The servent structure is returned in a buffer that will be
*	overwritten on the next call to getserv*().
*
*   BUGS
*
*   SEE ALSO
*	getservent(), getservbyname()
*
******************************************************************************
*
*/


struct servent * __saveds __asm getservbyport( register __d0 u_short port,
	register __a0 char *proto)
{
	struct servent *p;

	/* open or rewind file */
	setservent(0);

	while (p = getservent()) {
		if (p->s_port != port)
			continue;
		if (proto == 0 || strcmp(p->s_proto, proto) == 0)
			break;
	}

	if(!serv_stayopen)
		endservent();
	return (p);
}
