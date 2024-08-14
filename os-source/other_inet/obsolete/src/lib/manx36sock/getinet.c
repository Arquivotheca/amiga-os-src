/*
** gethostent(), gethostbyname(), sethostent(), endhostent().
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>
#include <netdb.h>
#include <stdio.h>
#include <bstr.h>

#define NULLFP	(FILE *)NULL
#define NULLHOST (struct hostent *)NULL
#define NULLCHAR (char *)NULL
#define FALSE (1==0)

static FILE	*hostfp;
static 		keepopen;

#define MAXALIAS 10

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

