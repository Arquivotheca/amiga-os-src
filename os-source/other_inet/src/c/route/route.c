/* ---------------------------------------------------------------------------------
 * ROUTE.c
 *
 * $Locker:  $
 *
 * $Id: route.c,v 1.6 92/10/23 13:12:39 bj Exp $
 *
 * $Revision: 1.6 $
 *
 * $Header: Hog:Other/inet/src/c/route/RCS/route.c,v 1.6 92/10/23 13:12:39 bj Exp $
 *
 *-----------------------------------------------------------------------------------
 */




/*
 * Copyright (c) 1983 The Regents of the University of California.
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


#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/mbuf.h>

#include <net/route.h>
#include <netinet/in.h>
#include <netns/ns.h>

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>

#include <ss/socket.h>
#include <netinet/inet.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <proto/all.h>

#include "route_rev.h"

#define CMDREV		"\0$VER: " VSTRING
#define TEMPLATE	"-N=NOSYMS/S,-F=FLUSH/S,CMD/M" CMDREV
#define OPT_N		0
#define OPT_F		1
#define OPT_CMD		2
#define OPT_COUNT	3

struct Library *SockBase ;
struct rtentry route;
int	s;
int nflag;
int errno;

int flushroutes(void);
int main(void);
void routename(struct sockaddr *sa, char *string);
void netname(struct sockaddr *sa, char *string);
int newroute(short argc, char *argv[]);
int getaddr(char *s,struct sockaddr_in *sin,struct hostent **hpp,char *name,int isnet);
int mynlist(struct nlist *nlp);

int main(void)
{
	long opts[OPT_COUNT+1] ;
	struct RDargs *rdargs ;
	char **argv ;
	short argc ;
	int ret_val = RETURN_FAIL ;
	STRPTR args ;
	char buffer[256] ;
	
    memset((char *)opts, 0, sizeof(opts)) ;
	
	if(SockBase = OpenLibrary( "inet:libs/socket.library", 1L )) 
	{
		setup_sockets( 5, &errno ) ;
		if((s = socket(AF_INET, SOCK_RAW, 0)) >= 0)
		{
			if((args = GetArgStr()) && (strlen(args) > 1))
			{
			    if(rdargs = ReadArgs(TEMPLATE, opts, NULL))
				{
					if(opts[OPT_N])
					{
						nflag++ ;
					}

					if(opts[OPT_F]) 
					{
						if (flushroutes() < 0) 
						{
							ret_val = RETURN_WARN ;
							goto getout ;
						}
					}

					if (opts[OPT_CMD]) 
					{
						argv = (char **)(opts[OPT_CMD]) ;
						for(argc=0 ;argv[argc] ;argc++) 
							 ;
						if (strcmp(*argv, "add") == 0)
						{
							ret_val = newroute(argc,argv) ;
						}
						else if (strcmp(*argv, "delete") == 0)
						{
							ret_val = newroute(argc,argv) ;
						}
						else
						{
						    Printf("route: %s:  huh?\n", (LONG)*argv) ;
						}
					}
					FreeArgs(rdargs) ;
				}
				else
				{
					PrintFault(IoErr(), "route" ) ;
				}
			}
			else
			{
				PutStr("route requires an argument\n") ;
			}
getout:		
			s_close(s) ;
		}
		else
		{
			PutStr("route: socket open failed\n") ;
		}
		cleanup_sockets() ;
		CloseLibrary(SockBase) ;
	}
	else
	{
		PutStr("route: Opening of socket library failed.\n") ;
	}
	Exit(ret_val) ;
}


/*
 * Purge all entries in the routing tables not
 * associated with network interfaces.
 */
#include <nlist.h>

struct nlist nl[] = {
#define	N_RTHOST	0
	{ "_rthost" },
#define	N_RTNET		1
	{ "_rtnet" },
#define N_RTHASHSIZE	2
	{ "_rthashsize" },
	"",
};

int flushroutes()
{
	struct mbuf mb;
	register struct rtentry *rt;
	register struct mbuf *m;
	struct mbuf **routehash;
	int i, doinghost = 1;
	short rthashsize;
	char string[MAXHOSTNAMELEN+1];

	if(mynlist(nl) < 0) {
		PutStr("network is not running");
		return (-1);
	}
	if (nl[N_RTHOST].n_value == 0) {
		PutStr("\"rthost\", symbol not in namelist\n");
		return(-1);
	}
	if (nl[N_RTNET].n_value == 0) {
		PutStr("\"rtnet\", symbol not in namelist\n");
		return(-1);
	}
	if (nl[N_RTHASHSIZE].n_value == 0) {
		PutStr("\"rthashsize\", symbol not in namelist\n");
		return(-1);
	}

	memcpy(&rthashsize,nl[N_RTHASHSIZE].n_value,sizeof(rthashsize));
	routehash = (struct mbuf **)AllocMem(rthashsize*sizeof(struct mbuf *),MEMF_ANY);

	memcpy(routehash,nl[N_RTHOST].n_value,rthashsize*sizeof(struct mbuf *));

	PutStr("Flushing routing tables:\n");
again:
	for (i = 0; i < rthashsize; i++) {
		if (routehash[i] == 0)
			continue;
		m = routehash[i];
		while (m) {
			memcpy(&mb,m,sizeof(mb));
			rt = mtod(&mb, struct rtentry *);
			if (rt->rt_flags & RTF_GATEWAY) {
				if(doinghost)
					routename(&rt->rt_dst,string);
				else
					netname(&rt->rt_dst,string);

				Printf("%-20.20s ", (LONG)string);
				routename(&rt->rt_gateway,string);
				Printf("%-20.20s ", (LONG)string);

				if (s_ioctl(s, SIOCDELRT, (caddr_t)rt) < 0) {
					Printf("%s\n",(LONG)strerror(errno));
				} else
					PutStr("done\n");
			}
			m = mb.m_next;
		}
	}
	if (doinghost) {
		memcpy(routehash,nl[N_RTNET].n_value,rthashsize*sizeof(struct mbuf *));
		doinghost = 0;
		goto again;
	}
	FreeMem(routehash,rthashsize*sizeof(struct mbuf *));
	return (0);
}

void routename(struct sockaddr *sa, char *string)
{
	char *cp;
	struct hostent *hp;
	char domain[MAXHOSTNAMELEN + 1];

	if (gethostname(domain, MAXHOSTNAMELEN)==0 && (cp = strchr(domain, '.')))
		(void)strcpy(domain, cp + 1);
	else
		domain[0] = 0;

	if (sa->sa_family==AF_INET) {
		struct in_addr in = ((struct sockaddr_in *)sa)->sin_addr;

		cp = 0;
		if (in.s_addr == INADDR_ANY)
			cp = "default";
		if (cp == 0 && !nflag) {
			hp = gethostbyaddr((char *)&in, sizeof (struct in_addr),AF_INET);
			if (hp) {
				if ((cp = strchr(hp->h_name, '.')) && !strcmp(cp + 1, domain))
					*cp = 0;
				cp = hp->h_name;
			}
		}
		if (cp)
			strcpy(string, cp);
		else {
#define C(x)	((x) & 0xff)
			in.s_addr = ntohl(in.s_addr);
			sprintf(string, "%lu.%lu.%lu.%lu", C(in.s_addr >> 24),
			   C(in.s_addr >> 16), C(in.s_addr >> 8), C(in.s_addr));
		}
	} else {
		u_short *s = (u_short *)sa->sa_data;
		sprintf(string, "af %ld: %lx %lx %lx %lx %lx %lx %lx",
		    sa->sa_family, s[0], s[1], s[2], s[3], s[4], s[5], s[6]);
	}
}

/*
 * Return the name of the network whose address is given.
 * The address is assumed to be that of a net or subnet, not a host.
 */

void netname(struct sockaddr *sa, char *string)
{
	char *cp = 0;
	struct netent *np;
	u_long net, mask;
	register u_long i;
	int subnetshift;

	if (sa->sa_family==AF_INET) {
		struct in_addr in = ((struct sockaddr_in *)sa)->sin_addr;

		i = in.s_addr = ntohl(in.s_addr);
		if (in.s_addr == 0)
			cp = "default";
		else if (!nflag) {
			if (IN_CLASSA(i)) {
				mask = IN_CLASSA_NET;
				subnetshift = 8;
			} else if (IN_CLASSB(i)) {
				mask = IN_CLASSB_NET;
				subnetshift = 8;
			} else {
				mask = IN_CLASSC_NET;
				subnetshift = 4;
			}
			/*
			 * If there are more bits than the standard mask
			 * would suggest, subnets must be in use.
			 * Guess at the subnet mask, assuming reasonable
			 * width subnet fields.
			 */
			while (in.s_addr &~ mask)
				mask = (long)mask >> subnetshift;
			net = in.s_addr & mask;
			while ((mask & 1) == 0)
				mask >>= 1, net >>= 1;
			np = getnetbyaddr(net, AF_INET);
			if (np)
				cp = np->n_name;
		}
		if (cp)
			strcpy(string, cp);
		else if ((in.s_addr & 0xffffff) == 0)
			(void)sprintf(string, "%lu", C(in.s_addr >> 24));
		else if ((in.s_addr & 0xffff) == 0)
			(void)sprintf(string, "%lu.%lu", C(in.s_addr >> 24),
			    C(in.s_addr >> 16));
		else if ((in.s_addr & 0xff) == 0)
			(void)sprintf(string, "%lu.%lu.%lu", C(in.s_addr >> 24),
			    C(in.s_addr >> 16), C(in.s_addr >> 8));
		else
			(void)sprintf(string, "%lu.%lu.%lu.%lu", C(in.s_addr >> 24),
			    C(in.s_addr >> 16), C(in.s_addr >> 8),
			    C(in.s_addr));
	} else {	
		u_short *s = (u_short *)sa->sa_data;

		sprintf(string, "af %ld: %lx %lx %lx %lx %lx %lx %lx",
			sa->sa_family, s[0], s[1], s[2], s[3], s[4], s[5], s[6]);
	}
}

int newroute(short argc, char *argv[])
{
	struct sockaddr_in *sin;
	char *cmd, dest[MAXHOSTNAMELEN+1], gateway[MAXHOSTNAMELEN+1];
	int ishost, metric = 0, ret, attempts, oerrno;
	short forcehost = 0;
	short forcenet = 0;
	struct hostent *hp;

	cmd = argv[0];
	if (argc > 2) {
		if ((strcmp(argv[1], "host")) == 0) {
			forcehost++;
			argc--;
			argv++;
		} else if ((strcmp(argv[1], "net")) == 0) {
			forcenet++;
			argc--;
			argv++;
		}
	}

	if (*cmd == 'a') {
		if (argc != 4) {
			Printf("usage: %s destination gateway metric\n", (LONG)cmd);
			PutStr("(metric of 0 if gateway is this host)\n");
			return(RETURN_WARN);
		}
		metric = atoi(argv[3]);
	} else {
		if (argc < 3) {
			Printf("usage: %s destination gateway\n", (LONG)cmd);
			return(RETURN_WARN);
		}
	}

//	sin = (struct sockaddr_in *)&route.rt_dst;
	ishost = getaddr(argv[1], (struct sockaddr_in *)&route.rt_dst, &hp, dest, forcenet);
	if(ishost==-1)
		return(RETURN_ERROR);
	if (forcehost)
		ishost = 1;
	if (forcenet)
		ishost = 0;
	sin = (struct sockaddr_in *)&route.rt_gateway;
	if(getaddr(argv[2], (struct sockaddr_in *)&route.rt_gateway, &hp, gateway, 0)==-1)
		return(RETURN_ERROR);
	route.rt_flags = RTF_UP;
	if (ishost)
		route.rt_flags |= RTF_HOST;
	if (metric > 0)
		route.rt_flags |= RTF_GATEWAY;
	for (attempts = 1; ; attempts++) {
		errno = 0;
		if ((ret = s_ioctl(s, *cmd == 'a' ? SIOCADDRT : SIOCDELRT,
		     (caddr_t)&route)) == 0)
			break;
		if (errno != ENETUNREACH && errno != ESRCH)
			break;
		if (hp && hp->h_addr_list[1]) {
			hp->h_addr_list++;
			bcopy(hp->h_addr_list[0], (caddr_t)&sin->sin_addr,
			    hp->h_length);
		} else
			break;
	}
	oerrno = errno;
	Printf("%s %s %s: gateway %s", (LONG)cmd, ishost? "host" : "net",dest,gateway);
	if (attempts > 1 && ret == 0)
	    Printf(" (%s)",(LONG)inet_ntoa(((struct sockaddr_in *)&route.rt_gateway)->sin_addr));
	if (ret == 0)
		PutStr("\n");
	else {
		PutStr(": ");
		Printf("%s\n",(LONG)strerror(oerrno));
	}
	return(0);
}


/*
 * Interpret an argument as a network address of some kind,
 * returning 1 if a host address, 0 if a network address.
 */
getaddr(s, sin, hpp, name, isnet)
	char *s;
	struct sockaddr_in *sin;
	struct hostent **hpp;
	char *name;
	int isnet;
{
	struct hostent *hp;
	struct netent *np;
	u_long val;

	*hpp = 0;
	if (strcmp(s, "default") == 0) {
		sin->sin_family = AF_INET;
		sin->sin_addr = inet_makeaddr(0, INADDR_ANY);
		strcpy(name,"default");
		return(0);
	}
	sin->sin_family = AF_INET;
	if (isnet == 0) {
		val = inet_addr(s);
		if (val != -1) {
			sin->sin_addr.s_addr = val;
			strcpy(name,s);
			return(inet_lnaof(sin->sin_addr) != INADDR_ANY);
		}
	}
	val = inet_network(s);
	if (val != -1) {
		sin->sin_addr = inet_makeaddr(val, INADDR_ANY);
		strcpy(name,s);
		return(0);
	}
	np = getnetbyname(s);
	if (np) {
		sin->sin_family = np->n_addrtype;
		sin->sin_addr = inet_makeaddr(np->n_net, INADDR_ANY);
		strcpy(name,np->n_name);
		return(0);
	}
	hp = gethostbyname(s);
	if (hp) {
		*hpp = hp;
		sin->sin_family = hp->h_addrtype;
		bcopy(hp->h_addr, &sin->sin_addr, hp->h_length);
		strcpy(name,hp->h_name);
		return(1);
	}
	Printf("%s: bad value\n", (LONG)s);
	return(-1);
}


/*
** Simple nlist of inet.library
*/

extern struct ExecBase *SysBase;

int mynlist(struct nlist *nlp)
{
	register struct InetNode *ip;
	register struct nlist *kl;
	register i;

	Forbid();
	ip = (struct InetNode *)FindName(&SysBase->LibList, "inet.library");
	if(!ip){
		Permit();
		return -1;
	}

	for(; nlp->n_name && nlp->n_name[0]; nlp++){
		nlp->n_value = 0l;
		kl = ip->names;
		for(i = 0; i < ip->nlsize; i++, kl++){
			if(kl->n_name && (strcmp(nlp->n_name, kl->n_name) == 0)){
				nlp->n_value = kl->n_value;
				nlp->n_type = 1;
				break;
			}
		}
	}
	Permit();

	return 0;
}