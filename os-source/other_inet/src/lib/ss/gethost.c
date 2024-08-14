/* -----------------------------------------------------------------------
 * gethost.c
 *
 * $Locker:  $
 *
 * $Id: gethost.c,v 1.4 92/08/21 19:17:24 kcd Exp $
 *
 * $Revision: 1.4 $
 *
 * $Log:	gethost.c,v $
 * Revision 1.4  92/08/21  19:17:24  kcd
 * Modified to work with DNS.
 * 
 * Revision 1.2  91/08/07  13:12:18  bj
 * Debug() call removed.
 * RCS header added.
 *
 *
 * $Header: AS225:src/lib/ss/RCS/gethost.c,v 1.4 92/08/21 19:17:24 kcd Exp $
 *
 *------------------------------------------------------------------------
 */
#include "sslib.h"

#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

/*
 * Internet version.
 */
#define	MAXADDRSIZE	14
#define BUFLEN		127

#define any(cp,match)	strpbrk(cp,match)

static BPTR hostf =NULL;
static UBYTE host_stayopen = (UBYTE)0;
static char line[BUFLEN+1];
static char hostaddr[MAXADDRSIZE];
static struct hostent host;
static char *host_aliases[MAXALIASES];
static char *host_addrs[] = {
	hostaddr,
	NULL
};

/****** socket.library/sethostent ******************************************
*
*   NAME
*	sethostent -- Rewind the hosts file ("inet:s/hosts").
*
*   SYNOPSIS
*	sethostent( flag )
*
*	void sethostent( int );
*	                 D1
*
*   FUNCTION
*	This function is rarely useful.
*
*	Opens the host file if necessary.  Rewinds the host file if it
*	is open.
*
*   INPUTS
*	flag		- If 'flag' is 1, calls to gethostbyname() and
*			  gethostbyaddr() will not close the file between
*			  calls.  You must close the file with an
*			  endhostent().  Once set, 'flag' cannot be reset
*			  except by calling endhostent().
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
*	gethostent(), gethostbyname(), gethostbyaddr(), endhostent()
*
******************************************************************************
*
*/

void __saveds __asm _sethtent(register __d1 int flag)
{
	if (hostf == NULL)
		hostf = Open(HOSTFILE,MODE_OLDFILE);
	else
		Seek(hostf,0L,OFFSET_BEGINNING);

	host_stayopen |= (UBYTE)flag;
}


/****** socket.library/endhostent ******************************************
*
*   NAME
*	endhostent -- Closes the hosts file ("inet:s/hosts").
*
*   SYNOPSIS
*	endhostent()
*
*	void endhostent( void );
*
*   FUNCTION
*	Closes the host file if it is open.  Only needed if sethostent()
*	or gethostent() have been called.
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
*	gethostent(), gethostbyname(), gethostbyaddr(), sethostent()
*
******************************************************************************
*
*/

void __saveds _endhtent(void)
{
	if (hostf) {
		Close(hostf);
		hostf = NULL;
		host_stayopen = (UBYTE)0;
	}
}

/****** socket.library/gethostent ******************************************
*
*   NAME
*	gethostent -- Get host entry from the hosts file ("inet:s/hosts").
*
*   SYNOPSIS
*	#include <sys/types.h>
*	#include <sys/socket.h>
*	#include <netdb.h>
*
*	host = gethostent()
*	D0
*
*	struct hostent *gethostent( void );
*
*   FUNCTION
*	There is normally no reason to use this function.  It is used
*	internally by gethostbyname() and gethostbyaddr().  It is provided
*	only for Un*x compatibility and even Un*x is phasing it out in
*	favor of other methods of name resolution.
*
*	Opens the host file if necessary.  Returns the next entry
*	in the file in a hostent structure.
*
*	struct	hostent {
*	char	*h_name;	\* official name of host *\
*	char	**h_aliases;	\* alias list *\
*	int	h_addrtype;	\* host address type *\
*	int	h_length;	\* length of address *\
*	char	**h_addr_list;	\* list of addresses from name server *\
*	#define	h_addr	h_addr_list[0] \* messed up for historical reasons *\
*	};
*
*   INPUTS
*	None.
*
*   RESULT
*	host		- pointer to struct hostent for next protocol.
*			  NULL on EOF or failure to open protocols file.
*   EXAMPLE
*
*   NOTES
*	The buffer pointed to by 'host' will be overwritten by the next
*	call to gethost*().
*
*   BUGS
*
*   SEE ALSO
*	sethostent(), gethostbyname(), gethostbyaddr(), endhostent()
*
******************************************************************************
*
*/

struct hostent * __saveds gethostent(void)
{
	char *p;
	register char *cp, **q;

	if (hostf == NULL) {
		sethostent(0);
		if (hostf == NULL)
			return (NULL);
	}

again:
	/* read a line into buffer */
#define V2
#ifdef V2
	if ((p = FGets(hostf,line, BUFLEN)) == NULL)
		return (NULL);
#endif
	/* check for a comment */
	if (*p == '#')
		goto again;

	/* check for end of line or start of comment */
	cp = any(p, "#\n");
	if (cp == NULL)
		goto again;

	/* null terminate */
	*cp = '\0';

	/* look for separator */
	cp = any(p, " \t");
	if (cp == NULL)
		goto again;

	*cp++ = '\0';

	/* THIS STUFF IS INTERNET SPECIFIC */

	/* the current host file format only allows
	one address per line, so that's all we will allow */
	host.h_addr_list = host_addrs;
	*((u_long *)host.h_addr) = inet_addr(p);

	/* always for Inet numbers */
	host.h_length = sizeof (u_long);
	host.h_addrtype = AF_INET;

	/* skip white space */
	while (*cp == ' ' || *cp == '\t')
		cp++;

	host.h_name = cp;

	/* now read in all aliases */
	q = host.h_aliases = host_aliases;
	cp = any(cp, " \t");
	if (cp != NULL)
		*cp++ = '\0';
	while (cp && *cp) {
		if (*cp == ' ' || *cp == '\t') {
			cp++;
			continue;
		}
		if (q < &host_aliases[MAXALIASES-1])
			*q++ = cp;
		cp = any(cp, " \t");
		if (cp != NULL)
			*cp++ = '\0';
	}
	*q = NULL;
	return (&host);
}


#define NULLFP	(FILE *)NULL
#define NULLHOST (struct hostent *)NULL
#define NULLCHAR (char *)NULL

/****** socket.library/gethostbyname ******************************************
*
*   NAME
*	gethostbyname -- Get host entry from the hosts file.
*
*   SYNOPSIS
*	#include <sys/socket.h>
*	#include <netdb.h>
*
*	host = gethostbyname( name )
*	D0                    A0
*
*	struct hostent *gethostbyname( char * );
*
*   FUNCTION
*	Opens the host file if necessary.  Finds the entry for "name"
*	and returns it in a hostent structure.  In the future, this function
*	will look beyond just the hosts file.
*
*	struct	hostent {
*	char	*h_name;	\* official name of host *\
*	char	**h_aliases;	\* alias list *\
*	int	h_addrtype;	\* host address type *\
*	int	h_length;	\* length of address *\
*	char	**h_addr_list;	\* list of addresses from name server *\
*	#define	h_addr	h_addr_list[0] \* messed up for historical reasons *\
*	};
*
*   INPUTS
*
*   RESULT
*	host		- pointer to struct hostent for protocol 'name.'
*			  NULL on EOF or failure to open protocols file.
*
*   EXAMPLE
*
*   NOTES
*	The buffer pointed to by 'host' will be overwritten by the next
*	call to gethost*().
*
*   BUGS
*
*   SEE ALSO
*	gethostbyaddr()
*
******************************************************************************
*
*/

struct hostent * __saveds __asm
_gethtbyname(register __a0 char *name)
{
	register struct hostent *hp;
	static long addr, *addrs[2]={&addr,0};
	int match, i;

	_sethtent(0);	/* open or rewind file */

	for(match = 0; (hp = gethostent()) != NULLHOST;){
		if(stricmp(hp->h_name, name) == 0){
			match++;
		} else {
			for(i = 0; hp->h_aliases[i] != NULLCHAR; i++)
				if(stricmp(hp->h_aliases[i], name) == 0){
					match++;
					break;
				}
		}

		if(match)
			break;
	}

    if(!host_stayopen)
		_endhtent();	/* close file */

	return(hp);
}

/****** socket.library/gethostbyaddr ******************************************
*
*   NAME
*	gethostbyaddr -- Get host entry from the hosts file.
*
*   SYNOPSIS
*	#include <sys/socket.h>
*	#include <netdb.h>
*
*	host = gethostbyaddr( addr, len, type )
*	D0                    A0    D0   D1
*
*	struct hostent *gethostbyaddr( char *, int, int );
*
*   FUNCTION
*	Opens the host file if necessary.  Finds the entry for 'addr'
*	and returns it in a hostent structure.  In the future, this
*	function will look beyond just the hosts file.
*
*	struct	hostent {
*	char	*h_name;	\* official name of host *\
*	char	**h_aliases;	\* alias list *\
*	int	h_addrtype;	\* host address type *\
*	int	h_length;	\* length of address *\
*	char	**h_addr_list;	\* list of addresses from name server *\
*	#define	h_addr	h_addr_list[0]	\* messed up for historical reasons *\
*	};
*
*   INPUTS
*	addr		- internet address, in network byte order.
*			  (This is really a long.)
*	len		- sizeof(addr), usually 4
*	type		- usually AF_INET
*
*   RESULT
*	host		- pointer to struct hostent for protocol 'addr.'
*			  NULL on EOF or failure to open protocols file.
*
*   EXAMPLE
*
*   NOTES
*	The only type currently in use is AF_INET.
*
*	The buffer pointed to by 'host' will be overwritten by the next
*	call to gethost*().
*
*   BUGS
*
*   SEE ALSO
*	gethostbyname()
*
******************************************************************************
*
*/

struct hostent * __saveds __asm _gethtbyaddr(
	register __a0 char *addr,
	register __d0 int len,
	register __d1 int type)
{
	register struct hostent *hp;

	_sethtent(0);

	for(; (hp = gethostent()) != NULLHOST;){
		if(len == hp->h_length && type == hp->h_addrtype &&
					bcmp(addr, hp->h_addr, len) == 0)
			break;
	}

    if(!host_stayopen)
		_endhtent();	/* close file */

	return(hp);
}

