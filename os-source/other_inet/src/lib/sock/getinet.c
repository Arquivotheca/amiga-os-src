
#include <exec/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>
#include <netdb.h>
#include <stdio.h>

#define NULLFP	(FILE *)NULL
#define NULLHOST (struct hostent *)NULL
#define NULLCHAR (char *)NULL

static FILE	*hostfp;
static 		keepopen;

#define MAXALIAS 10

/****** socket/gethostbyname ******************************************
*
*   NAME
*		gethostbyname -- get host entry from the inet:db/hosts file
*
*   SYNOPSIS
*		#include <sys/socket.h>
*		#include <netdb.h>
*
*		host = gethostbyname ( name )
*
*		struct hostent *gethostbyname ( char * ); 
*
*   FUNCTION
*		Opens the host file if necessary.  Finds the entry for "name"
*		and returns it in a hostent structure.
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
*		sethostent(), gethostent(), gethostbyaddr(), endhostent()
*
******************************************************************************
*
*/

struct hostent *
gethostbyname(name)
	char	*name;
{
	register struct hostent *hp;
	extern struct hostent *gethostent();
	static long addr, *addrs[2] = {&addr, 0};
	long inet_addr();
	int match, i;

	if((addr = inet_addr(name)) != -1L){
		static struct hostent host;

		addrs[1] = 0;
		host.h_name = name;
		host.h_addr_list = (char **)addrs;
		host.h_addrtype = AF_INET;
		host.h_length = sizeof(long);
		return &host;
	}

	sethostent(FALSE);

	for(match = 0; (hp = gethostent()) != NULLHOST;){
		if(strcasecmp(hp->h_name, name) == 0){
			match++;
		} else {
			for(i = 0; hp->h_aliases[i] != NULLCHAR; i++)
				if(strcasecmp(hp->h_aliases[i], name) == 0){
					match++;
					break;
				}
		}

		if(match)
			break;
	}

	endhostent();
	return(hp);
}

/****** socket/gethostbyaddr ******************************************
*
*   NAME
*		gethostbyaddr -- get host entry from the inet:db/hosts file
*
*   SYNOPSIS
*		#include <sys/socket.h>
*		#include <netdb.h>
*
*		host = gethostbyaddr ( addr, len, type )
*
*		struct hostent *gethostbyaddr ( char *, int, int ); 
*
*   FUNCTION
*		Opens the host file if necessary.  Finds the entry for "name"
*		and returns it in a hostent structure.
*
*		struct	hostent {
*			char	*h_name;			official name of host
*			char	**h_aliases;		alias list
*			int	h_addrtype;				host address type
*			int	h_length;				length of address
*			char	**h_addr_list;		list of addresses from name server
*		#define	h_addr	h_addr_list[0]	messed up for historical reasons
*		};
*
*	INPUTS
*		addr	internet address, in network byte order
*				this is really a long
*
*		len		sizeof(addr), usually 4
*		type	usually AF_INET
*
*   RESULT
*		Returns NULL on EOF or failure to open hosts file.
*
*   EXAMPLE
*
*   NOTES
*		The only type currently in use is AF_INET
*
*   BUGS
*
*   SEE ALSO
*		sethostent(), gethostent(), gethostbyaddr(), endhostent()
*
******************************************************************************
*
*/

struct hostent *
gethostbyaddr(addr, len, type)
	char	*addr;
	int	len, type;
{
	register struct hostent *hp;

	sethostent(FALSE);

	for(; (hp = gethostent()) != NULLHOST;){
		if(len == hp->h_length && type == hp->h_addrtype &&
					bcmp(addr, hp->h_addr, len) == 0)
			break;
	}

	endhostent();
	return(hp);
}

