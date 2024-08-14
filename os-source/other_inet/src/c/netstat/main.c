/*
 * Copyright (c) 1983,1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission. This software
 * is provided ``as is'' without express or implied warranty.
 */

#include <exec/types.h>

#include <sys/param.h>
#include <ss/socket.h>
#include <ss/socket_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <clib/exec_protos.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>

#include "netstat_rev.h"
static char *compiler = "MANX36 MANX36" ;
static char *vers = VERSTAG ;


#include <nlist.h>
struct nlist nl[] = {
#define	N_MBSTAT	0
	{ "_mbstat" },
#define	N_IPSTAT	1
	{ "_ipstat" },
#define	N_TCB		2
	{ "_tcb" },
#define	N_TCPSTAT	3
	{ "_tcpstat" },
#define	N_UDB		4
	{ "_udb" },
#define	N_UDPSTAT	5
	{ "_udpstat" },
#define	N_RAWCB		6
	{ "_rawcb" },
#define	N_SYSMAP	7
	{ "_Sysmap" },
#define	N_SYSSIZE	8
	{ "_Syssize" },
#define	N_IFNET		9
	{ "_ifnet" },
#define	N_IMP		10
	{ "_imp_softc" },
#define	N_RTHOST	11
	{ "_rthost" },
#define	N_RTNET		12
	{ "_rtnet" },
#define	N_ICMPSTAT	13
	{ "_icmpstat" },
#define	N_RTSTAT	14
	{ "_rtstat" },
#define	N_NFILE		15
	{ "_nfile" },
#define	N_FILE		16
	{ "_file" },
#define	N_UNIXSW	17
	{ "_unixsw" },
#define N_RTHASHSIZE	18
	{ "_rthashsize" },
#define N_IDP		19
	{ "_nspcb"},
#define N_IDPSTAT	20
	{ "_idpstat"},
#define N_SPPSTAT	21
	{ "_spp_istat"},
#define N_NSERR		22
	{ "_ns_errstat"},
#define N_NIMP		23
	{ "_nimp"},
	"",
};

/* internet protocols */
extern	int protopr();
extern	int tcp_stats(), udp_stats(), ip_stats(), icmp_stats();
/* ns protocols */
extern	int nsprotopr();
extern	int spp_stats(), idp_stats(), nserr_stats();

#define NULLPROTOX	((struct protox *) 0)
struct protox {
	u_char	pr_index;		/* index into nlist of cb head */
	u_char	pr_sindex;		/* index into nlist of stat block */
	u_char	pr_wanted;		/* 1 if wanted, 0 otherwise */
	int	(*pr_cblocks)();	/* control blocks printing routine */
	int	(*pr_stats)();		/* statistics printing routine */
	char	*pr_name;		/* well-known name */
} protox[] = {
	{ N_TCB,	N_TCPSTAT,	1,	protopr,
	  tcp_stats,	"tcp" },
	{ N_UDB,	N_UDPSTAT,	1,	protopr,
	  udp_stats,	"udp" },
	{ -1,		N_IPSTAT,	1,	0,
	  ip_stats,	"ip" },
	{ -1,		N_ICMPSTAT,	1,	0,
	  icmp_stats,	"icmp" },
	{ -1,		-1,		0,	0,
	  0,		0 }
};


char	*system = "/vmunix";
char	*kmemf = "/dev/kmem";
int	kmem;
int	kflag;
int	Aflag;
int	aflag;
int	hflag;
int	iflag;
int	mflag;
int	nflag;
int	pflag;
int	rflag;
int	sflag;
int	tflag;
int	interval;
char	*interface;
int	unit;

struct Library *SockBase;
extern struct Library *SysBase;

char	usage[] = "[ -Aaihmnrst ] [-p proto] [-I interface]";

int	af = AF_UNSPEC;
int errno;

extern	char *malloc();
extern	off_t lseek();
void myexit(int rc);

main(argc, argv)
	int argc;
	char *argv[];
{
	char *cp, *name;
	register struct protoent *p;
	register struct protox *tp;	/* for printing cblocks & stats */
	struct protox *name2protox();	/* for -p */

	if(SockBase = OpenLibrary("socket.library",0L))
	{
	    setup_sockets(10, &errno);

            name = argv[0];
            argc--, argv++;

            while (argc > 0 && **argv == '-') {
                    for (cp = &argv[0][1]; *cp; cp++)
                    switch(*cp) {

                    case 'A':
                            Aflag++;
                            break;

                    case 'a':
                            aflag++;
                            break;

                    case 'h':
                            hflag++;
                            break;

                    case 'i':
                            iflag++;
                            break;

                    case 'm':
                            mflag++;
                            break;

                    case 'n':
                            nflag++;
                            break;

                    case 'r':
                            rflag++;
                            break;

                    case 's':
                            sflag++;
                            break;

                    case 't':
                            tflag++;
                            break;

                    case 'u':
                            af = AF_UNIX;
                            break;

                    case 'p':
                            argv++;
                            argc--;
                            if (argc == 0)
                                    goto use;
                            if ((tp = name2protox(*argv)) == NULLPROTOX) {
                                    fprintf(stderr, "%s: unknown or uninstrumented protocol\n",
                                            *argv);
                                    myexit(10);
                            }
                            pflag++;
                            break;

                    case 'f':
                            argv++;
                            argc--;
                            if (strcmp(*argv, "ns") == 0)
                                    af = AF_NS;
                            else if (strcmp(*argv, "inet") == 0)
                                    af = AF_INET;
                            else if (strcmp(*argv, "unix") == 0)
                                    af = AF_UNIX;
                            else {
                                    fprintf(stderr, "%s: unknown address family\n",
                                            *argv);
                                    myexit(10);
                            }
                            break;

                    case 'I':
                            iflag++;
                            if (*(interface = cp + 1) == 0) {
                                    if ((interface = argv[1]) == 0)
                                            break;
                                    argv++;
                                    argc--;
                            }
                            for (cp = interface; isalpha(*cp); cp++)
                                    ;
                            unit = atoi(cp);
                            *cp-- = 0;
                            break;

                    default:
    use:
                            printf("usage: %s %s\n", name, usage);
                            myexit(1);
                    }
                    argv++, argc--;
            }
            if (argc > 0 && isdigit(argv[0][0])) {
                    interval = atoi(argv[0]);
                    if (interval <= 0)
                            goto use;
                    argv++, argc--;
                    iflag++;
            }
            if (argc > 0) {
                    system = *argv;
                    argv++, argc--;
            }
            nlist(system, nl);
            if (nl[0].n_type == 0) {
                    fprintf(stderr, "network not running\n");
                    myexit(1);
            }
            if (mflag) {
                    mbpr((off_t)nl[N_MBSTAT].n_value);
                    myexit(0);
            }
            if (pflag) {
                    if (tp->pr_stats)
                            (*tp->pr_stats)(nl[tp->pr_sindex].n_value,
                                    tp->pr_name);
                    else
                            printf("%s: no stats routine\n", tp->pr_name);
                    myexit(0);
            }
            /*
             * Keep file descriptors open to avoid overhead
             * of open/close on each call to get* routines.
             */
            sethostent(1);
            setnetent(1);
            if (iflag) {
                    intpr(interval, nl[N_IFNET].n_value);
                    myexit(0);
            }
            if (rflag) {
                    if (sflag)
                            rt_stats((off_t)nl[N_RTSTAT].n_value);
                    else
                            routepr((off_t)nl[N_RTHOST].n_value,
                                    (off_t)nl[N_RTNET].n_value,
                                    (off_t)nl[N_RTHASHSIZE].n_value);
                    myexit(0);
            }
        if (af == AF_INET || af == AF_UNSPEC) {
            setprotoent(1);
            setservent(1);
            while (p = getprotoent()) {

                    for (tp = protox; tp->pr_name; tp++)
                            if (strcmp(tp->pr_name, p->p_name) == 0)
                                    break;
                    if (tp->pr_name == 0 || tp->pr_wanted == 0)
                            continue;
                    if (sflag) {
                            if (tp->pr_stats)
                                    (*tp->pr_stats)(nl[tp->pr_sindex].n_value,
                                            p->p_name);
                    } else
                            if (tp->pr_cblocks)
                                    (*tp->pr_cblocks)(nl[tp->pr_index].n_value,
                                            p->p_name);
            }
            endprotoent();
        }
    }
    else
        fprintf(stderr,"Couldn't open socket.library");

    myexit(0);
}

void myexit(int rc)
{
	if(SockBase)
	{
		cleanup_sockets();
		CloseLibrary(SockBase);
	}
	exit(rc);
};

static long addr;

kread(fd, buf, size)
{
	bcopy((caddr_t)addr, buf, size);
	addr += size;
}

klseek(fd, off, how)
	int	fd;
	long	off;
	int	how;
{
	addr = off;
}


char *
plural(n)
	int n;
{

	return (n != 1 ? "s" : "");
}

/*
 * Find the protox for the given "well-known" name.
 */
struct protox *
knownname(name)
	char *name;
{
	struct protox *tp;

	for (tp = protox; tp->pr_name; tp++)
		if (strcmp(tp->pr_name, name) == 0)
			return(tp);
	return(NULLPROTOX);
}

/*
 * Find the protox corresponding to name.
 */
struct protox *
name2protox(name)
	char *name;
{
	struct protox *tp;
	char **alias;			/* alias from p->aliases */
	struct protoent *p;

	/*
	 * Try to find the name in the list of "well-known" names. If that
	 * fails, check if name is an alias for an Internet protocol.
	 */
	if (tp = knownname(name))
		return(tp);

	setprotoent(1);			/* make protocol lookup cheaper */
	while (p = getprotoent()) {
		/* assert: name not same as p->name */
		for (alias = p->p_aliases; *alias; alias++)
			if (strcmp(name, *alias) == 0) {
				endprotoent();
				return(knownname(p->p_name));
			}
	}
	endprotoent();
	return(NULLPROTOX);
}
