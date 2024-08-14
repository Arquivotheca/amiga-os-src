/* -----------------------------------------------------------------------
 * IFCONFIG.C  (manx 36)
 *
 * $Locker:  $
 *
 * $Id: ifconfig.c,v 1.2 93/03/23 11:54:04 kcd Exp $
 *
 * $Revision: 1.2 $
 *
 * $Header: AS225:src/c/ifconfig/RCS/ifconfig.c,v 1.2 93/03/23 11:54:04 kcd Exp $
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

#include <exec/types.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>
#include <ss/socket.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <ss/socket_pragmas.h>
#include <dos/rdargs.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <errno.h>
#include <ctype.h>
#include <netdb.h>

#include "ifconfig_rev.h"
static char *vers = VERSTAG ;

#define	TEMPLATE	"Interface/A,Address/M,Up/S,Down/S,Netmask/K,Metric/K/N,Arp/S,-Arp=NoArp/S,Trailers/S,-Trailers=NoTrailers/S,PointToPoint/S,NoPointToPoint/S,Broadcast/K"

#define ARG_Interface		0
#define ARG_Address		1
#define ARG_Addr		1
#define	ARG_Up			2
#define	ARG_Down		3
#define	ARG_Mask		4
#define	ARG_Metric		5
#define	ARG_Arp			6
#define	ARG_NoArp		7
#define	ARG_Trailers		8
#define ARG_NoTrailers		9
#define	ARG_PointToPoint	10
#define ARG_NoPointToPoint	11
#define ARG_Broadcast		12
int errno;
struct	ifreq ifr;
struct	sockaddr_in sin = { AF_INET };
struct	sockaddr_in broadaddr;
struct	sockaddr_in netmask = { AF_INET };
struct	sockaddr_in ipdst = { AF_INET };
char	name[30];
int	flags;
int	metric;
int	setaddr;
int	setmask;
int	setbroadaddr;
int	setipdst;
int	s;
LONG 	args[13];
BPTR	Stdout;
extern char *sys_errlist[];
extern int sys_nerr, errno;
void myexit();

#define	SysBase	(*(struct Library **)4L)
struct Library *DOSBase;
struct Library *SockBase;
struct Library *UtilityBase;

void	setifflags(), setifaddr(), setifdstaddr(), setifnetmask();
void	setifmetric(), setifbroadaddr(), setifipdst(), setpointtopoint();
void	perror(char *);

#define	NEXTARG		0xffffff
#ifdef FOO
struct	cmd {
	char	*c_name;
	int	c_parameter;		/* NEXTARG means next argv */
	void	(*c_func)();
} cmds[] = {
	{ "up",		IFF_UP,		setifflags } ,
	{ "down",	-IFF_UP,	setifflags },
	{ "trailers",	-IFF_NOTRAILERS,setifflags },
	{ "-trailers",	IFF_NOTRAILERS,	setifflags },
	{ "arp",	-IFF_NOARP,	setifflags },
	{ "-arp",	IFF_NOARP,	setifflags },
	{ "debug",	IFF_DEBUG,	setifflags },
	{ "-debug",	-IFF_DEBUG,	setifflags },
	{ "pointtopoint",TRUE,		setpointtopoint },
	{ "-pointtopoint",FALSE,	setpointtopoint },
	{ "netmask",	NEXTARG,	setifnetmask },
	{ "metric",	NEXTARG,	setifmetric },
	{ "broadcast",	NEXTARG,	setifbroadaddr },
	{ "ipdst",	NEXTARG,	setifipdst },
	{ 0,		0,		setifaddr },
	{ 0,		0,		setifdstaddr },
};
#endif
int	in_status(), in_getaddr();

/* Known address families */
struct afswtch {
	char *af_name;
	short af_af;
	int (*af_status)();
	int (*af_getaddr)();
} afs[] = {
	{ "inet",	AF_INET,	in_status,	in_getaddr },
	{ 0,		0,		0,		0 }
};
#define MAXSOCKS 10

struct afswtch *afp;	/*the address family being set or asked about*/
struct RDArgs *myargs;
LONG returncode;

int _main(void)
{
	int af = AF_INET;
	UBYTE	**addr;
	BOOL	show_status = TRUE;
	UBYTE i;

	returncode = 0;

	if(DOSBase = OpenLibrary("dos.library",37L))
	{
	    Stdout = Output();

	    if(SockBase = OpenLibrary("socket.library",0))
	    {
	    	setup_sockets(MAXSOCKS, &errno);

	    	if(UtilityBase = OpenLibrary("utility.library",37L))
	    	{
	    	    for(i=0;i<13;i++)
	    	    	args[i]=0L;

	    	    if(myargs = ReadArgs(TEMPLATE,args,NULL))
	    	    {
	    	    	stccpy(ifr.ifr_name,args[0],sizeof ifr.ifr_name);

	    	    	if(args[ARG_Address])
	    	    	{
	    	    	    struct afswtch *myafp;

	    	    	    addr = (UBYTE **)args[ARG_Address];

                            for (myafp = afp = afs; myafp->af_name; myafp++)
                                    if (!Stricmp(myafp->af_name, *addr)) {
                                            afp = myafp; addr++;
                                            break;
                                    }
                            af = ifr.ifr_addr.sa_family = afp->af_af;
                       }
                       s = socket(af, SOCK_DGRAM, 0);
                       if (s >= 0)
                       {
	    	    	   stccpy(ifr.ifr_name,args[0],sizeof ifr.ifr_name);
                           if(s_ioctl(s, SIOCGIFFLAGS, (caddr_t)&ifr) >= 0)
                           {
		    	    	stccpy(ifr.ifr_name,args[0],sizeof ifr.ifr_name);
		    	    	flags = ifr.ifr_flags;

		    	    	if(s_ioctl(s, SIOCGIFMETRIC, (caddr_t)&ifr) >= 0)
		    	    	{
		    	    	    metric = ifr.ifr_metric;

				    if(args[ARG_PointToPoint])
				    {
				    	show_status = FALSE;
				    	setpointtopoint("pointtopoint",TRUE);
				    }
				    if(args[ARG_NoPointToPoint])
				    {
				    	show_status = FALSE;
				    	setpointtopoint("pointtopoint",FALSE);
				    }
				    if(args[ARG_Arp])
				    {
				    	show_status = FALSE;
				    	setifflags("arp",-IFF_NOARP);
				    }
				    if(args[ARG_NoArp])
				    {
				    	show_status = FALSE;
				    	setifflags("-arp",IFF_NOARP);
				    }
				    if(args[ARG_Trailers])
				    {
				    	show_status = FALSE;
				    	setifflags("trailers",-IFF_NOTRAILERS);
				    }
				    if(args[ARG_NoTrailers])
				    {
				    	show_status = FALSE;
				    	setifflags("-trailers",IFF_NOTRAILERS);
				    }
				    if(args[ARG_Metric])
				    {
				    	show_status = FALSE;
                                        stccpy(ifr.ifr_name, args[0], sizeof (ifr.ifr_name));
                                        ifr.ifr_metric = (*(ULONG *)args[ARG_Metric]);
                                        if (s_ioctl(s, SIOCSIFMETRIC, (caddr_t)&ifr) < 0)
                                                perror("ioctl (set metric)");
                                    }
				    if(args[ARG_Mask] && (af == AF_INET))
		    	    	    {
				    	show_status = FALSE;
                                        in_getaddr(args[ARG_Mask], &netmask);
                                        ifr.ifr_addr = *(struct sockaddr *)&netmask;
                                        if (s_ioctl(s, SIOCSIFNETMASK, (caddr_t)&ifr) < 0)
                                                Perror("ioctl (SIOCSIFNETMASK)");
                                    }
                                    if(args[ARG_Addr] && (af == AF_INET))
                                    {
                                    	if(*addr)
                                    	{
					    show_status = FALSE;
                                    	    (*afp->af_getaddr)(*addr, &sin);
                                            ifr.ifr_addr = *(struct sockaddr *) &sin;
                                            if (s_ioctl(s, SIOCSIFADDR, (caddr_t)&ifr) < 0)
                                                    Perror("ioctl (SIOCSIFADDR)");
                                            addr++;
                                        }
                                    }
                                    if(args[ARG_Addr] && (af == AF_INET))
                                    {
                                    	if(*addr)
                                    	{
				    	    show_status = FALSE;
                                    	    (*afp->af_getaddr)(*addr, &ifr.ifr_addr);
                                    	    if (s_ioctl(s, SIOCSIFDSTADDR, (caddr_t)&ifr) < 0)
						Perror("ioctl (SIOCSIFDSTADDR)");
                                        }
                                    }
                                    if(args[ARG_Broadcast])
                                    {
				    	show_status = FALSE;
					in_getaddr(args[ARG_Broadcast], &broadaddr);
                                        ifr.ifr_addr = *(struct sockaddr *)&broadaddr;
                                        if (s_ioctl(s, SIOCSIFBRDADDR, (caddr_t)&ifr) < 0)
                                                Perror("ioctl (SIOCSIFBRDADDR)");
                                    }
				    if(args[ARG_Up])
				    {
				    	show_status = FALSE;
				    	setifflags("up",IFF_UP);
				    }
				    if(args[ARG_Down])
				    {
				    	show_status = FALSE;
				    	setifflags("down",-IFF_UP);
				    }
				    if(show_status)
				    	status();
                                }
                                else
                                {
                                    Perror("ioctl (SIOCGIFMETRIC)",errno);
                                    returncode = 1;
                                }
                            }
                            else
                            {
                            	returncode = 10;
                            	Perror("ioctl (SIOCGIFFLAGS)");
                            }
                        }
                        else
                        {
                            PutStr("ifconfig: Make sure that inet.library is located in inet:libs.\n");
                            returncode = 1;
                        }
                        FreeArgs(myargs);
                    }
                    else
                    {
                        Printf("usage: ifconfig interface\n%s%s%s%s%s%s",
                            "\t[ af [ address [ dest_addr ] ] [ up ] [ down ]",
                                    "[ netmask mask ] ]\n",
                            "\t[ metric n ]\n",
                            "\t[ trailers | -trailers ]\n",
                            "\t[ arp | -arp ]\n",
                            "\t[ pointtopoint | -pointtopoint ]\n");
                        returncode = 1;
                    }
                    CloseLibrary(UtilityBase);
                }
                else
                {
                    Printf("ifconfig: Couldn't open utility.library.\n");
                    returncode = 1;
                }
                cleanup_sockets();
                CloseLibrary(SockBase);
            }
            else
            {
            	Printf("ifconfig: Couldn't open socket.library.\n");
                returncode = 1;
            }
            CloseLibrary(DOSBase);
        }
        exit(returncode);
}

void setifflags(vname, value)
	char *vname;
	short value;
{
 	if (s_ioctl(s, SIOCGIFFLAGS, (caddr_t)&ifr) < 0) {
 		Perror("ioctl (SIOCGIFFLAGS)");
 		myexit(1);
 	}
	strncpy(ifr.ifr_name, args[0], sizeof (ifr.ifr_name));
 	flags = ifr.ifr_flags;

	if (value < 0) {
		value = -value;
		flags &= ~value;
	} else
		flags |= value;
	ifr.ifr_flags = flags;
	if (s_ioctl(s, SIOCSIFFLAGS, (caddr_t)&ifr) < 0) {
		Perror(vname);
	}
}

void setpointtopoint(vname, value)
	char *vname;
	short value;
{
 	if (s_ioctl(s, SIOCGIFFLAGS, (caddr_t)&ifr) < 0) {
 		Perror("ioctl (SIOCGIFFLAGS)");
 		myexit(1);
 	}
	strncpy(ifr.ifr_name, args[0], sizeof (ifr.ifr_name));
 	flags = ifr.ifr_flags;

	if(value) {
		flags &= ~IFF_BROADCAST;
		flags |= IFF_POINTOPOINT|IFF_NOARP;
	}
	else {
		flags |= IFF_BROADCAST;
		flags &= ~(IFF_POINTOPOINT|IFF_NOARP);
	}

	ifr.ifr_flags = flags;
	if (s_ioctl(s, SIOCSIFFLAGS, (caddr_t)&ifr) < 0) {
		Perror(vname);
	}
}


#define	IFFBITS \
"\020\1UP\2BROADCAST\3DEBUG\4LOOPBACK\5POINTOPOINT\6NOTRAILERS\7RUNNING\10NOARP\
"

/*
 * Print the status of the interface.  If an address family was
 * specified, show it and it only; otherwise, show them all.
 */
status()
{
	register struct afswtch *p = afp;
//	short af = ifr.ifr_addr.sa_family;

	Printf("%s: ", args[0]);
	printb("flags", flags, IFFBITS);
	if (metric)
		Printf(" metric %d", metric);
	PutStr("\n");
	if ((p = afp) != NULL) {
		(*p->af_status)(1);
	} else for (p = afs; p->af_name; p++) {
		ifr.ifr_addr.sa_family = p->af_af;
		(*p->af_status)(0);
	}
	return(0);
}

int in_status(force)
	int force;
{
	struct sockaddr_in *sin;
	char *inet_ntoa();

	strncpy(ifr.ifr_name, args[0], sizeof (ifr.ifr_name));
	if (s_ioctl(s, SIOCGIFADDR, (caddr_t)&ifr) < 0) {
		if (errno == EADDRNOTAVAIL || errno == EAFNOSUPPORT) {
			if (!force)
				return(0);
			bzero((char *)&ifr.ifr_addr, sizeof(ifr.ifr_addr));
		} else
			perror("ioctl (SIOCGIFADDR)");
	}
	sin = (struct sockaddr_in *)&ifr.ifr_addr;
	Printf("\tinet %s ", inet_ntoa(sin->sin_addr.s_addr));
	strncpy(ifr.ifr_name, args[0], sizeof (ifr.ifr_name));
	if (s_ioctl(s, SIOCGIFNETMASK, (caddr_t)&ifr) < 0) {
		if (errno != EADDRNOTAVAIL)
			perror("ioctl (SIOCGIFNETMASK)");
		bzero((char *)&ifr.ifr_addr, sizeof(ifr.ifr_addr));
	} else
		netmask.sin_addr =
		    ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;
	if (flags & IFF_POINTOPOINT) {
		if (s_ioctl(s, SIOCGIFDSTADDR, (caddr_t)&ifr) < 0) {
			if (errno == EADDRNOTAVAIL)
			    bzero((char *)&ifr.ifr_addr, sizeof(ifr.ifr_addr));
			else
			    perror("ioctl (SIOCGIFDSTADDR)");
		}
		strncpy(ifr.ifr_name, args[0], sizeof (ifr.ifr_name));
		sin = (struct sockaddr_in *)&ifr.ifr_dstaddr;
		Printf("--> %s ", inet_ntoa(sin->sin_addr.s_addr));
	}
	Printf("netmask %lx ", ntohl(netmask.sin_addr.s_addr));
	if (flags & IFF_BROADCAST) {
		if (s_ioctl(s, SIOCGIFBRDADDR, (caddr_t)&ifr) < 0) {
			if (errno == EADDRNOTAVAIL)
			    bzero((char *)&ifr.ifr_addr, sizeof(ifr.ifr_addr));
			else
			    perror("ioctl (SIOCGIFADDR)");
		}
		strncpy(ifr.ifr_name, args[0], sizeof (ifr.ifr_name));
		sin = (struct sockaddr_in *)&ifr.ifr_addr;
		if (sin->sin_addr.s_addr != 0)
			Printf("broadcast %s", inet_ntoa(sin->sin_addr.s_addr));
	}
	PutStr("\n");
	return(0);
}

Perror(cmd)
	char *cmd;
{
	extern int errno;

	PutStr("ifconfig: ");
	switch (errno) {
	case ENXIO:
		Printf("%s: no such interface\n", cmd);
		PutStr("Barf!\n");
		break;

	case EPERM:
		Printf("%s: permission denied\n", cmd);
		break;

	default:
		perror(cmd);
	}
	return(0);
}

void myexit(rcode)
	int rcode;
{
	cleanup_sockets();
	CloseLibrary(SockBase);
	CloseLibrary(UtilityBase);
	if(myargs)
		FreeArgs(myargs);
	CloseLibrary(DOSBase);
	exit(rcode);
}

struct	in_addr inet_makeaddr();

in_getaddr(s, saddr)
	char *s;
	struct sockaddr *saddr;
{
	register struct sockaddr_in *sin = (struct sockaddr_in *)saddr;
	struct netent *np, *getnetbyname();
	struct hostent *hp;
	int val;

	sin->sin_family = AF_INET;
	val = inet_addr(s);
	if (val != -1) {
		sin->sin_addr.s_addr = val;
		return(0);
	}
	hp = gethostbyname(s);
	if (hp) {
		sin->sin_family = hp->h_addrtype;
		bcopy(hp->h_addr, (char *)&sin->sin_addr, hp->h_length);
		return(0);
	}
	np = getnetbyname(s);
	if (np) {
		sin->sin_family = np->n_addrtype;
		sin->sin_addr = inet_makeaddr(np->n_net, INADDR_ANY);
		return(0);
	}
	Printf("%s: bad value\n", s);
	myexit(1);
}

/*
 * Print a value a la the %b format of the kernel's printf
 */
printb(s, v, bits)
	char *s;
	register char *bits;
	register unsigned short v;
{
	register int i, any = 0;
	register char c;

//	if (bits && *bits == 8)
//		Printf("%s=%lo", s, v);
//	else
		Printf("%s=%lx", s, v);
	Flush(Stdout);
	bits++;
	if (bits) {
		FPutC(Stdout,'<');
		while (i = *bits++) {
			if (v & (1 << (i-1))) {
				if (any)
					FPutC(Stdout,',');
				any = 1;
				for (; (c = *bits) > 32; bits++)
					FPutC(Stdout,c);
			} else
				for (; *bits > 32; bits++)
					;
		}
		FPutC(Stdout,'>');
	}
	Flush(Stdout);
	return(0);
}

void perror(s)
	char *s;
{
	if (s)
		Printf ("%s: ", s);

	if (errno < 0 || errno > sys_nerr)
		PutStr("unknown error\n");
	else
		Printf ("%s\n", sys_errlist[errno]);

}
