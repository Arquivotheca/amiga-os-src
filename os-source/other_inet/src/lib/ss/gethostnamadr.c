/*
 * Copyright (c) 1985, 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)gethostnamadr.c	6.45 (Berkeley) 2/24/91";
#endif /* LIBC_SCCS and not lint */

#include "sslib.h"
#include <sys/param.h>
#include <sys/socket.h>
/* #include <netinet/in.h> */
/* #include <arpa/inet.h> */
#include <arpa/nameser.h>
#include <netdb.h>
#include <resolv.h>
#include <stdio.h>
/* #include <ctype.h> */
#include <errno.h>
#include <string.h>
#include <exec/tasks.h>
#include <exec/memory.h>

extern struct ExecBase *SysBase;

#define	MAXALIASES	35
#define	MAXADDRS	35

static char *h_addr_ptrs[MAXADDRS + 1];

static struct hostent host;
static char *host_aliases[MAXALIASES];
static char hostbuf[BUFSIZ+1];
static struct in_addr host_addr;
static FILE *hostf = NULL;
static char hostaddr[MAXADDRS];
static char *host_addrs[2];
static int stayopen = 0;
char *strpbrk();

extern void kprintf(STRPTR fmt, ...);

extern struct SocketLibrary *SockBase;

#if PACKETSZ > 1024
#define	MAXPACKET	PACKETSZ
#else
#define	MAXPACKET	1024
#endif

typedef union {
    HEADER hdr;
    u_char buf[MAXPACKET];
} querybuf;

typedef union {
    long al;
    char ac;
} align;

extern int h_errno;

static struct hostent * __saveds __asm getanswer(register __a0 querybuf *answer,
	register __d0 int anslen,
	register __d1 int iquery);

static struct hostent * __saveds __asm getanswer(register __a0 querybuf *answer,
	register __d0 int anslen,
	register __d1 int iquery)
{
	register HEADER *hp;
	register u_char *cp;
	register int n;
	u_char *eom;
	char *bp, **ap;
	int type, class, buflen, ancount, qdcount;
	int haveanswer, getclass = C_ANY;
	char **hap;

	eom = answer->buf + anslen;
	/*
	 * find first satisfactory answer
	 */
	hp = &answer->hdr;
	ancount = ntohs(hp->ancount);
	qdcount = ntohs(hp->qdcount);
	bp = hostbuf;
	buflen = sizeof(hostbuf);
	cp = answer->buf + sizeof(HEADER);
	if (qdcount) {
		if (iquery) {
			if ((n = dn_expand((u_char *)answer->buf,
			    (u_char *)eom, (u_char *)cp, (u_char *)bp,
			    buflen)) < 0) {
				h_errno = NO_RECOVERY;
				return ((struct hostent *) NULL);
			}
			cp += n + QFIXEDSZ;
			host.h_name = bp;
			n = strlen(bp) + 1;
			bp += n;
			buflen -= n;
		} else
			cp += __dn_skipname(cp, eom) + QFIXEDSZ;
		while (--qdcount > 0)
			cp += __dn_skipname(cp, eom) + QFIXEDSZ;
	} else if (iquery) {
		if (hp->aa)
			h_errno = HOST_NOT_FOUND;
		else
			h_errno = TRY_AGAIN;
		return ((struct hostent *) NULL);
	}
	ap = host_aliases;
	*ap = NULL;
	host.h_aliases = host_aliases;
	hap = h_addr_ptrs;
	*hap = NULL;
#if BSD >= 43 || defined(h_addr)	/* new-style hostent structure */
	host.h_addr_list = h_addr_ptrs;
#endif
	haveanswer = 0;
	while (--ancount >= 0 && cp < eom) {
		if ((n = dn_expand((u_char *)answer->buf, (u_char *)eom,
		    (u_char *)cp, (u_char *)bp, buflen)) < 0)
			break;
		cp += n;
		type = _getshort(cp);
 		cp += sizeof(u_short);
		class = _getshort(cp);
 		cp += sizeof(u_short) + sizeof(u_long);
		n = _getshort(cp);
		cp += sizeof(u_short);
		if (type == T_CNAME) {
			cp += n;
			if (ap >= &host_aliases[MAXALIASES-1])
				continue;
			*ap++ = bp;
			n = strlen(bp) + 1;
			bp += n;
			buflen -= n;
			continue;
		}
		if (iquery && type == T_PTR) {
			if ((n = dn_expand((u_char *)answer->buf,
			    (u_char *)eom, (u_char *)cp, (u_char *)bp,
			    buflen)) < 0) {
				cp += n;
				continue;
			}
			cp += n;
			host.h_name = bp;
			return(&host);
		}
		if (iquery || type != T_A)  {
#ifdef DEBUG
			if (_res.options & RES_DEBUG)
				printf("unexpected answer type %d, size %d\n",
					type, n);
#endif
			cp += n;
			continue;
		}
		if (haveanswer) {
			if (n != host.h_length) {
				cp += n;
				continue;
			}
			if (class != getclass) {
				cp += n;
				continue;
			}
		} else {
			host.h_length = n;
			getclass = class;
			host.h_addrtype = (class == C_IN) ? AF_INET : AF_UNSPEC;
			if (!iquery) {
				host.h_name = bp;
				bp += strlen(bp) + 1;
			}
		}

		bp += sizeof(align) - ((u_long)bp % sizeof(align));

		if (bp + n >= &hostbuf[sizeof(hostbuf)]) {
#ifdef DEBUG
			if (_res.options & RES_DEBUG)
				printf("size (%d) too big\n", n);
#endif
			break;
		}
		bcopy(cp, *hap++ = bp, n);
		bp +=n;
		cp += n;
		haveanswer++;
	}
	if (haveanswer) {
		*ap = NULL;
#if BSD >= 43 || defined(h_addr)	/* new-style hostent structure */
		*hap = NULL;
#else
		host.h_addr = h_addr_ptrs[0];
#endif
		return (&host);
	} else {
		h_errno = TRY_AGAIN;
		return ((struct hostent *) NULL);
	}
}

struct hostent * __saveds __asm
_ghbyname(register __a0 char *name);

struct hostent * __saveds __asm
_ghbyname(register __a0 char *name)
{
	querybuf buf;
	register char *cp;
	struct hostent *hent;
	int n;

	/*
	 * disallow names consisting only of digits/dots, unless
	 * they end in a dot.
	 */
	if (isdigit(name[0]))
		for (cp = name;; ++cp) {
			if (!*cp) {
				if (*--cp == '.')
					break;
				/*
				 * All-numeric, no dot at the end.
				 * Fake up a hostent as if we'd actually
				 * done a lookup.  What if someone types
				 * 255.255.255.255?  The test below will
				 * succeed spuriously... ???
				 */
				if ((host_addr.s_addr = inet_addr(name)) == -1) {
					h_errno = HOST_NOT_FOUND;
					return((struct hostent *) NULL);
				}
				host.h_name = name;
				host.h_aliases = host_aliases;
				host_aliases[0] = NULL;
				host.h_addrtype = AF_INET;
				host.h_length = sizeof(u_long);
				h_addr_ptrs[0] = (char *)&host_addr;
				h_addr_ptrs[1] = (char *)0;
#if BSD >= 43 || defined(h_addr)	/* new-style hostent structure */
				host.h_addr_list = h_addr_ptrs;
#else
				host.h_addr = h_addr_ptrs[0];
#endif
				return (&host);
			}
			if (!isdigit(*cp) && *cp != '.')
				break;
		}

        if(hent = _gethtbyname(name))
        	return(hent);

	if(SockBase->ml_origbase->ml_config->usedns)
        {
            if ((n = res_search(name, C_IN, T_A, buf.buf, sizeof(buf))) < 0) {
#ifdef DEBUG
                    if (_res.options & RES_DEBUG)
                            printf("res_search failed\n");
#endif
                    if (errno == ECONNREFUSED)
                            return (_gethtbyname(name));
                    else
                            return ((struct hostent *) NULL);
            }
            return (getanswer(&buf, n, 0));
        }
        else
            return((struct hostent *) NULL);
}

struct hostent * __saveds __asm _ghbyaddr(
	register __a0 char *addr,
	register __d0 int len,
	register __d1 int type);

struct hostent * __saveds __asm _ghbyaddr(
	register __a0 char *addr,
	register __d0 int len,
	register __d1 int type)
{
	int n;
	querybuf buf;
	register struct hostent *hp;
	char qbuf[MAXDNAME];

	if(hp = _gethtbyaddr(addr, len, type))
		return(hp);

	if (type != AF_INET)
		return ((struct hostent *) NULL);
	(void)sprintf(qbuf, "%lu.%lu.%lu.%lu.in-addr.arpa",
		((unsigned)addr[3] & 0xff),
		((unsigned)addr[2] & 0xff),
		((unsigned)addr[1] & 0xff),
		((unsigned)addr[0] & 0xff));

	if(SockBase->ml_origbase->ml_config->usedns)
	{

            n = res_query(qbuf, C_IN, T_PTR, (char *)&buf, sizeof(buf));
            if (n < 0) {
#ifdef DEBUG
                    if (_res.options & RES_DEBUG)
                            printf("res_query failed\n");
#endif
//              kprintf("Query failed!\n");
                    if (errno == ECONNREFUSED)
                            return (_gethtbyaddr(addr, len, type));

                    return ((struct hostent *) NULL);
            }
            hp = getanswer(&buf, n, 1);
            if (hp == NULL)
            {
//              kprintf("NULL hostent!\n");
                    return ((struct hostent *) NULL);
            }
            hp->h_addrtype = type;
            hp->h_length = len;

//      kprintf("h_name: %s",hp->h_name);

            h_addr_ptrs[0] = (char *)&host_addr;
            h_addr_ptrs[1] = (char *)0;
            host_addr = *(struct in_addr *)addr;
#if BSD < 43 && !defined(h_addr)        /* new-style hostent structure */
            hp->h_addr = h_addr_ptrs[0];
#endif
        }
        return(hp);
}

