/* -----------------------------------------------------------------------
 * ARP.C
 *
 * $Locker:  $
 *
 * $Id: arp.c,v 1.3 93/01/29 21:16:34 kcd Exp $
 *
 * $Revision: 1.3 $
 *
 * $Header: AS225:src/c/arp/RCS/arp.c,v 1.3 93/01/29 21:16:34 kcd Exp $
 *
 *------------------------------------------------------------------------
 */



/*
 * Copyright (c) 1984 Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Sun Microsystems, Inc.
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

/*
 * arp - display, set, and delete arp table entries
 */

#include <exec/types.h>

#include <ss/socket.h>
#include <ss/socket_pragmas.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include <sys/param.h>
#include <sys/file.h>
#include <sys/ioctl.h>

#include <netdb.h>
#include <net/if.h>
#include <netinet/if_ether.h>

#include <errno.h>
#include <nlist.h>
#include <stdio.h>

#include "arp_rev.h"

#define TEMPLATE "Name,Ether_Addr,-A=All/S,-D=Del/S,-S=Set/S,Temp/S,Pub/S,Trail/S,-F=File/S"
#define TEMPLATE_FILE "Name,Ether_Addr,Temp/S,Pub/S,Trail/S"

#define ARG_NAME	0
#define ARG_ETHER	1
#define ARG_ALL		2
#define ARG_DEL		3
#define ARG_SET		4
#define ARG_TEMP	5
#define ARG_PUB		6
#define ARG_TRAIL	7
#define ARG_FILE	8

static char *vers = VERSTAG ;

struct Library *SockBase;
extern struct Library *SysBase;
extern struct Library *DOSBase;

char UsageStr[]="Usage: arp Hostname\n       arp -a/ALL\n       arp -d/DEL Hostname\n       arp -s/SET Hostname Ether_Addr [TEMP] [PUB] [TRAIL]\n       arp -f/FILE Filename\n";

struct RDArgs *rdargs;
LONG args[9];
BOOL freeargs=FALSE;
LONG myTags[1]={TAG_DONE};

void myexit(int rc);
int file(char *name);
void get(char *host);
void delete(char *host);
void dump(char *kernel, char *mem);
int kread(int fd, void *buf, int size);
void klseek(int fd, long off, int how);
void ether_print(u_char *cp);

int errno;
int rc;
static int kflag;

main(argc, argv)
	int argc;
	char **argv;
{

	if(SockBase = OpenLibrary("socket.library",0L))
	{
	    setup_sockets(10, &errno);

	    if(rdargs=AllocDosObject(DOS_RDARGS,(struct TagItem *)myTags))
	    {
		rdargs->RDA_DAList = NULL;
		rdargs->RDA_Source.CS_Buffer = NULL;
		rdargs->RDA_Source.CS_Length = 0;
		rdargs->RDA_Source.CS_CurChr = 0;
		rdargs->RDA_ExtHelp = UsageStr;

		if(ReadArgs(TEMPLATE,args,rdargs))
		{
		    freeargs=TRUE;

		    if(args[ARG_ALL])
		    {
			    int res=0;

                            if (argc > 2)
                            {
                            	PutStr("wrong number of arguments\n");
                            	res = 1;
                            }
                            else
                                dump("foo","foo");

                            myexit(res);
                    }
                    if(args[ARG_DEL])
                    {
                    	    int res=0;

                            if (!args[ARG_NAME])
                            {
                            	PutStr("Hostname required with -d or DEL.\n");
                            	res = 1;
                            }
                            else
                                delete((char *)args[ARG_NAME]);

                            myexit(res);
                    }
                    if(args[ARG_SET])
                    {
                    	    int res;

                            if (argc < 4 || argc > 7)
                            {
                            	PutStr("wrong number of arguments\n");
                            	res=1;
                            }
                            else
                               res = set(args[ARG_NAME],args[ARG_ETHER],args[ARG_TEMP],args[ARG_PUB],args[ARG_TRAIL]);
                            myexit(res);

                    }
                    if(args[ARG_FILE])
                    {
			int res;

                        if (!args[ARG_NAME])
                        {
                            PutStr("Filename required with -f or FILE.\n");
                            res = 1;
                        }
                        else
                            res = file(args[ARG_NAME]);

                        myexit(res);
                    }
                    if(args[ARG_NAME])
                    {
                    	get(args[ARG_NAME]);
                    	myexit(0);
                    }
                    PutStr("arp: required argument missing\n");
                }
            }
        }
        else
            PutStr("Couldn't open socket.library");

        myexit(1);
}

void myexit(rc)
	int rc;
{
	if(SockBase)
	{
	    cleanup_sockets();
	    CloseLibrary(SockBase);
	}
	if(freeargs)
	    FreeArgs(rdargs);

	if(rdargs)
	    FreeDosObject(DOS_RDARGS,rdargs);

	exit(rc);
}

/*
 * Process a file to set standard arp entries
 */
int file(name)
	char *name;
{
	BPTR file;
	UBYTE line[256];
	int i, retval;

	LONG args[5];
	struct RDArgs *loc_rdargs;

	retval = 0;

        if(loc_rdargs=AllocDosObject(DOS_RDARGS,(struct TagItem *)myTags))
        {
            if(file = Open(name,MODE_OLDFILE))
            {
                while(FGets(file,line,255))
                {
                    for(i=0;(i<256 && line[i]);i++);
                    line[i]='\n';

                    for(i=0;i<5;i++)
                    	args[i]=0;

                    loc_rdargs->RDA_DAList = NULL;
                    loc_rdargs->RDA_Source.CS_Buffer = line;
                    loc_rdargs->RDA_Source.CS_Length = 255;
                    loc_rdargs->RDA_Source.CS_CurChr = 0;

                    if(ReadArgs(TEMPLATE_FILE,args,loc_rdargs))
                    {
                        if(set(args[0],args[1],args[2],args[3],args[4]))
                            retval = 1;

                        FreeArgs(loc_rdargs);
                    }
                    else
                    {
                    	Printf("arp: bad line: %s\n",line);
                    	retval = 1;
                    }
                }
                Close(file);
            }
            else
            {
            	Printf("arp: cannot open %s\n",name);
            }
            FreeDosObject(DOS_RDARGS,loc_rdargs);
        }
        return(retval);
}

/*
 * Set an individual arp entry
 */
int set(char *host, char *eaddr, LONG temp, LONG pub, LONG trail)
{
	struct arpreq ar;
	struct hostent *hp;
	struct sockaddr_in *sin;
	u_char *ea;
	int s;

	bzero((caddr_t)&ar, sizeof ar);
	sin = (struct sockaddr_in *)&ar.arp_pa;
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = inet_addr(host);
	if (sin->sin_addr.s_addr == -1) {
		if (!(hp = gethostbyname(host))) {
			Printf("arp: %s: Unknown host\n", host);
			return (1);
		}
		bcopy((char *)hp->h_addr, (char *)&sin->sin_addr,
		    sizeof sin->sin_addr);
	}
	ea = (u_char *)ar.arp_ha.sa_data;
	if (ether_aton(eaddr, ea))
		return (1);
	ar.arp_flags = ATF_PERM;

	if(temp)
	    ar.arp_flags &= ~ATF_PERM;

	if(pub)
	    ar.arp_flags |= ATF_PUBL;

	if(trail)
	    ar.arp_flags |= ATF_USETRAILERS;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		PutStr("arp: socket");
		return(1);
	}
	if (s_ioctl(s, SIOCSARP, (caddr_t)&ar) < 0) {
		PutStr(host);
		return(1);
	}
	s_close(s);
	return (0);
}

/*
 * Display an individual arp entry
 */
void get(host)
	char *host;
{
	struct arpreq ar;
	struct hostent *hp;
	struct sockaddr_in *sin;
	u_char *ea;
	int s;
	char *inet_ntoa();

	bzero((caddr_t)&ar, sizeof ar);
	ar.arp_pa.sa_family = AF_INET;
	sin = (struct sockaddr_in *)&ar.arp_pa;
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = inet_addr(host);
	if (sin->sin_addr.s_addr == -1) {
		if (!(hp = gethostbyname(host))) {
			Printf("arp: %s: Unknown host\n", host);
			myexit(1);
		}
		bcopy((char *)hp->h_addr, (char *)&sin->sin_addr,
		    sizeof sin->sin_addr);
	}
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		PutStr("arp: socket");
		myexit(1);
	}
	if (s_ioctl(s, SIOCGARP, (caddr_t)&ar) < 0) {
		if (errno == ENXIO)
			Printf("%s (%s) -- no entry\n",
			    host, inet_ntoa(sin->sin_addr.s_addr));
		else
			PutStr("SIOCGARP");
		myexit(1);
	}
	s_close(s);
	ea = (u_char *)ar.arp_ha.sa_data;
	Printf("%s (%s) at ", host, inet_ntoa(sin->sin_addr.s_addr));
	if (ar.arp_flags & ATF_COM)
		ether_print(ea);
	else
		PutStr("(incomplete)");
	if (ar.arp_flags & ATF_PERM)
		PutStr(" permanent");
	if (ar.arp_flags & ATF_PUBL)
		PutStr(" published");
	if (ar.arp_flags & ATF_USETRAILERS)
		PutStr(" trailers");
	PutStr("\n");
}

/*
 * Delete an arp entry
 */
void delete(host)
	char *host;
{
	struct arpreq ar;
	struct hostent *hp;
	struct sockaddr_in *sin;
	int s;

	bzero((caddr_t)&ar, sizeof ar);
	ar.arp_pa.sa_family = AF_INET;
	sin = (struct sockaddr_in *)&ar.arp_pa;
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = inet_addr(host);
	if (sin->sin_addr.s_addr == -1) {
		if (!(hp = gethostbyname(host))) {
			Printf("arp: %s: Unknown host\n", host);
			myexit(1);
		}
		bcopy((char *)hp->h_addr, (char *)&sin->sin_addr,
		    sizeof sin->sin_addr);
	}
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		PutStr("arp: socket");
		myexit(1);
	}
	if (s_ioctl(s, SIOCDARP, (caddr_t)&ar) < 0) {
		if (errno == ENXIO)
			Printf("%s (%s) -- no entry\n",
			    host, inet_ntoa(sin->sin_addr.s_addr));
		else
			PutStr("SIOCDARP");
		myexit(1);
	}
	s_close(s);
	Printf("%s (%s) deleted\n", host, inet_ntoa(sin->sin_addr.s_addr));
}

struct nlist nl[] = {
#define	X_ARPTAB	0
	{ "_arptab" },
#define	X_ARPTAB_SIZE	1
	{ "_arptab_size" },
	{ "" },
};


/*
 * Dump the entire arp table
 */
void dump(kernel, mem)
	char *kernel, *mem;
{
	extern int h_errno;
	struct arptab *at;
	struct hostent *hp;
	int bynumber, mf, sz;
	short arptab_size;
	char *host, *malloc();
	off_t lseek();
	int num_printed;
	mf=0;

	if (nlist(kernel, nl) < 0 || nl[X_ARPTAB_SIZE].n_type == 0) {
		PutStr("arp: network is not running\n");
		myexit(1);
	}
	klseek(mf, (long)nl[X_ARPTAB_SIZE].n_value, L_SET);
	kread(mf, &arptab_size, sizeof arptab_size);
	if (arptab_size <= 0 || arptab_size > 1000) {
		Printf("arp: %s: namelist wrong\n", kernel);
		myexit(1);
	}
	sz = arptab_size * sizeof (struct arptab);
	at = (struct arptab *)malloc((u_int)sz);
	if (at == NULL) {
		PutStr("arp: can't get memory for arptab.\n");
		myexit(1);
	}
	klseek(mf, (long)nl[X_ARPTAB].n_value, L_SET);
	if (kread(mf, (char *)at, sz) != sz) {
		PutStr("arp: error reading arptab");
		myexit(1);
	}
	num_printed = 0;
	for (bynumber = 0; arptab_size-- > 0; at++) {
		if (at->at_iaddr.s_addr == 0 || at->at_flags == 0)
			continue;
		if (bynumber == 0)
			hp = gethostbyaddr((caddr_t)&at->at_iaddr,
			    sizeof at->at_iaddr, AF_INET);
		else
			hp = 0;
		if (hp)
			host = hp->h_name;
		else {
			host = "?";
		}
		Printf("%s (%s) at ", host, inet_ntoa(at->at_iaddr.s_addr));
		num_printed++;
		if (at->at_flags & ATF_COM)
			ether_print(at->at_enaddr);
		else
			PutStr("(incomplete)");
		if (at->at_flags & ATF_PERM)
			PutStr(" permanent");
		if (at->at_flags & ATF_PUBL)
			PutStr(" published");
		if (at->at_flags & ATF_USETRAILERS)
			PutStr(" trailers");
		PutStr("\n");
	}
	if(num_printed == 0){
		PutStr("arp table has no entries\n");
	}
}

/*
 * Seek into the kernel for a value.
 */

static long addr;

int kread(fd, buf, size)
	int fd;
	void *buf;
	int size;
{
	bcopy((caddr_t)addr, buf, size);
	addr += size;
	return(size);
}

void klseek(fd, off, how)
	int	fd;
	long	off;
	int	how;
{
	addr = off;
}

void ether_print(cp)
	u_char *cp;
{
	Printf("%lx:%lx:%lx:%lx:%lx:%lx", cp[0], cp[1], cp[2], cp[3], cp[4], cp[5]);
}

int ether_aton(a, n)
	char *a;
	u_char *n;
{
	int i, o[6];

	i = sscanf(a, "%x:%x:%x:%x:%x:%x", &o[0], &o[1], &o[2],
					   &o[3], &o[4], &o[5]);
	if (i != 6) {
		Printf("arp: invalid Ethernet address '%s'\n", a);
		return (1);
	}
	for (i=0; i<6; i++)
		n[i] = o[i];
	return (0);
}

