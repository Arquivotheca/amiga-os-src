/*
 * Copyright (c) 1982, 1986, 1988 Regents of the University of California.
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
 *
 *	@(#)ip_icmp.c	7.8.1.2 (Berkeley) 3/2/89
 */

#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/kernel.h>

#include <net/route.h>
#include <net/if.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp_var.h>

#include "proto.h"

#ifdef ICMPPRINTFS
/*
 * ICMP routines: error generation, receive packet processing, and
 * routines to turnaround packets back to the originator, and
 * host table maintenance routines.
 */
int	icmpprintfs = 0;
#endif

void icmp_nlist_init()
{
	enter_nlist("_icmpstat", &icmpstat);
}

/*
 * Generate an error packet of type error
 * in response to bad packet ip.
 */
/*VARARGS4*/

void icmp_error(oip, type, code, ifp, dest)
	struct ip *oip;
	int type, code;
	struct ifnet *ifp;
	struct in_addr dest;
{
	register unsigned oiplen = oip->ip_hl << 2;
	register struct icmp *icp;
	register struct mbuf *m;
	struct ip *nip;
	unsigned icmplen;

#ifdef ICMPPRINTFS
	if (icmpprintfs)
		printf("icmp_error(%x, %d, %d)\n", oip, type, code);
#endif
	if (type != ICMP_REDIRECT)
		icmpstat.icps_error++;
	/*
	 * Don't send error if not the first fragment of message.
	 * Don't error if the old packet protocol was ICMP
	 * error message, only known informational types.
	 */
	if (oip->ip_off &~ (IP_MF|IP_DF))
		goto free;
	if (oip->ip_p == IPPROTO_ICMP && type != ICMP_REDIRECT &&
	  dtom(oip)->m_len >= oiplen + ICMP_MINLEN &&
	  !ICMP_INFOTYPE(((struct icmp *)((caddr_t)oip + oiplen))->icmp_type)) {
		icmpstat.icps_oldicmp++;
		goto free;
	}

	/*
	 * First, formulate icmp message
	 */
	m = m_get(M_DONTWAIT, MT_HEADER);
	if (m == NULL)
		goto free;
	icmplen = oiplen + min(8, oip->ip_len);
	m->m_len = icmplen + ICMP_MINLEN;
	m->m_off = MMAXOFF - m->m_len;
	icp = mtod(m, struct icmp *);
	if ((u_int)type > ICMP_MAXTYPE)
		panic("icmp_error");
	icmpstat.icps_outhist[type]++;
	icp->icmp_type = type;
	if (type == ICMP_REDIRECT)
		icp->icmp_gwaddr = dest;
	else
		icp->icmp_void = 0;
	if (type == ICMP_PARAMPROB) {
		icp->icmp_pptr = code;
		code = 0;
	}
	icp->icmp_code = code;
	bcopy((caddr_t)oip, (caddr_t)&icp->icmp_ip, icmplen);
	nip = &icp->icmp_ip;
	nip->ip_len = htons((u_short)(nip->ip_len + oiplen));

	/*
	 * Now, copy old ip header (without options)
	 * in front of icmp message.
	 */
	if (m->m_len + oiplen > MLEN)
		oiplen = sizeof(struct ip);
	if (m->m_len + oiplen > MLEN)
		panic("icmp len");
	m->m_off -= sizeof(struct ip);
	m->m_len += sizeof(struct ip);
	nip = mtod(m, struct ip *);
	bcopy((caddr_t)oip, (caddr_t)nip, oiplen);
	nip->ip_len = m->m_len;
	nip->ip_hl = sizeof(struct ip) >> 2;
	nip->ip_p = IPPROTO_ICMP;
	icmp_reflect(nip, ifp);

free:
	m_freem(dtom(oip));
}

static struct sockproto icmproto = { AF_INET, IPPROTO_ICMP };
static struct sockaddr_in icmpsrc = { AF_INET };
static struct sockaddr_in icmpdst = { AF_INET };
static struct sockaddr_in icmpgw = { AF_INET };
struct in_ifaddr *ifptoia();

/*
 * Process a received ICMP message.
 */

void icmp_input(m, ifp)
	register struct mbuf *m;
	struct ifnet *ifp;
{
	register struct icmp *icp;
	register struct ip *ip = mtod(m, struct ip *);
	int icmplen = ip->ip_len, hlen = ip->ip_hl << 2;
	register int i;
	struct in_ifaddr *ia;
	void (*ctlfunc)(int, struct sockaddr *);
	int code;
	extern u_char ip_protox[];
	extern struct in_addr in_makeaddr();

	/*
	 * Locate icmp structure in mbuf, and check
	 * that not corrupted and of at least minimum length.
	 */
#ifdef ICMPPRINTFS
	if (icmpprintfs)
		printf("icmp_input from %x, len %d\n", ip->ip_src, icmplen);
#endif
	if (icmplen < ICMP_MINLEN) {
		icmpstat.icps_tooshort++;
		goto free;
	}
	i = hlen + MIN(icmplen, ICMP_ADVLENMIN);
 	if ((m->m_off > MMAXOFF || m->m_len < i) &&
 		(m = m_pullup(m, i)) == 0)  {
		icmpstat.icps_tooshort++;
		return;
	}
 	ip = mtod(m, struct ip *);
	m->m_len -= hlen;
	m->m_off += hlen;
	icp = mtod(m, struct icmp *);
	if (in_cksum(m, icmplen)) {
		icmpstat.icps_checksum++;
		goto free;
	}
	m->m_len += hlen;
	m->m_off -= hlen;

#ifdef ICMPPRINTFS
	/*
	 * Message type specific processing.
	 */
	if (icmpprintfs)
		printf("icmp_input, type %d code %d\n", icp->icmp_type,
		    icp->icmp_code);
#endif
	if (icp->icmp_type > ICMP_MAXTYPE)
		goto raw;
	icmpstat.icps_inhist[icp->icmp_type]++;
	code = icp->icmp_code;
	switch (icp->icmp_type) {

	case ICMP_UNREACH:
		if (code > 5)
			goto badcode;
		code += PRC_UNREACH_NET;
		goto deliver;

	case ICMP_TIMXCEED:
		if (code > 1)
			goto badcode;
		code += PRC_TIMXCEED_INTRANS;
		goto deliver;

	case ICMP_PARAMPROB:
		if (code)
			goto badcode;
		code = PRC_PARAMPROB;
		goto deliver;

	case ICMP_SOURCEQUENCH:
		if (code)
			goto badcode;
		code = PRC_QUENCH;
	deliver:
		/*
		 * Problem with datagram; advise higher level routines.
		 */
		icp->icmp_ip.ip_len = ntohs((u_short)icp->icmp_ip.ip_len);
		if (icmplen < ICMP_ADVLENMIN || icmplen < ICMP_ADVLEN(icp)) {
			icmpstat.icps_badlen++;
			goto free;
		}
#ifdef ICMPPRINTFS
		if (icmpprintfs)
			printf("deliver to protocol %d\n", icp->icmp_ip.ip_p);
#endif
		icmpsrc.sin_addr = icp->icmp_ip.ip_dst;
		if (ctlfunc = inetsw[ip_protox[icp->icmp_ip.ip_p]].pr_ctlinput)
			(*ctlfunc)(code, (struct sockaddr *)&icmpsrc);
		break;

	badcode:
		icmpstat.icps_badcode++;
		break;

	case ICMP_ECHO:
		icp->icmp_type = ICMP_ECHOREPLY;
		goto reflect;

	case ICMP_TSTAMP:
		if (icmplen < ICMP_TSLEN) {
			icmpstat.icps_badlen++;
			break;
		}
		icp->icmp_type = ICMP_TSTAMPREPLY;
		icp->icmp_rtime = iptime();
		icp->icmp_ttime = icp->icmp_rtime;	/* bogus, do later! */
		goto reflect;
		
	case ICMP_IREQ:
#define	satosin(sa)	((struct sockaddr_in *)(sa))
		if (in_netof(ip->ip_src) == 0 && (ia = ifptoia(ifp)))
			ip->ip_src = in_makeaddr(in_netof(IA_SIN(ia)->sin_addr),
			    in_lnaof(ip->ip_src));
		icp->icmp_type = ICMP_IREQREPLY;
		goto reflect;

	case ICMP_MASKREQ:
		if (icmplen < ICMP_MASKLEN || (ia = ifptoia(ifp)) == 0)
			break;
		icp->icmp_type = ICMP_MASKREPLY;
		icp->icmp_mask = htonl(ia->ia_subnetmask);
		if (ip->ip_src.s_addr == 0) {
			if (ia->ia_ifp->if_flags & IFF_BROADCAST)
			    ip->ip_src = satosin(&ia->ia_broadaddr)->sin_addr;
			else if (ia->ia_ifp->if_flags & IFF_POINTOPOINT)
			    ip->ip_src = satosin(&ia->ia_dstaddr)->sin_addr;
		}
reflect:
		ip->ip_len += hlen;	/* since ip_input deducts this */
		icmpstat.icps_reflect++;
		icmpstat.icps_outhist[icp->icmp_type]++;
		icmp_reflect(ip, ifp);
		return;

	case ICMP_REDIRECT:
		if (icmplen < ICMP_ADVLENMIN || icmplen < ICMP_ADVLEN(icp)) {
			icmpstat.icps_badlen++;
			break;
		}
		/*
		 * Short circuit routing redirects to force
		 * immediate change in the kernel's routing
		 * tables.  The message is also handed to anyone
		 * listening on a raw socket (e.g. the routing
		 * daemon for use in updating its tables).
		 */
		icmpgw.sin_addr = ip->ip_src;
		icmpdst.sin_addr = icp->icmp_gwaddr;
#ifdef	ICMPPRINTFS
		if (icmpprintfs)
			printf("redirect dst %x to %x\n", icp->icmp_ip.ip_dst,
				icp->icmp_gwaddr);
#endif
		if (code == ICMP_REDIRECT_NET || code == ICMP_REDIRECT_TOSNET) {
			icmpsrc.sin_addr =
			 in_makeaddr(in_netof(icp->icmp_ip.ip_dst), INADDR_ANY);
			rtredirect((struct sockaddr *)&icmpsrc,
			  (struct sockaddr *)&icmpdst, RTF_GATEWAY,
			  (struct sockaddr *)&icmpgw);
			icmpsrc.sin_addr = icp->icmp_ip.ip_dst;
			pfctlinput(PRC_REDIRECT_NET,
			  (struct sockaddr *)&icmpsrc);
		} else {
			icmpsrc.sin_addr = icp->icmp_ip.ip_dst;
			rtredirect((struct sockaddr *)&icmpsrc,
			  (struct sockaddr *)&icmpdst, RTF_GATEWAY | RTF_HOST,
			  (struct sockaddr *)&icmpgw);
			pfctlinput(PRC_REDIRECT_HOST,
			  (struct sockaddr *)&icmpsrc);
		}
		break;

	/*
	 * No kernel processing for the following;
	 * just fall through to send to raw listener.
	 */
	case ICMP_ECHOREPLY:
	case ICMP_TSTAMPREPLY:
	case ICMP_IREQREPLY:
	case ICMP_MASKREPLY:
	default:
		break;
	}

raw:
	icmpsrc.sin_addr = ip->ip_src;
	icmpdst.sin_addr = ip->ip_dst;
	raw_input(m, &icmproto, (struct sockaddr *)&icmpsrc,
	    (struct sockaddr *)&icmpdst);
	return;

free:
	m_freem(m);
}

/*
 * Reflect the ip packet back to the source
 */

void icmp_reflect(ip, ifp)
	register struct ip *ip;
	struct ifnet *ifp;
{
	register struct in_ifaddr *ia;
	struct in_addr t;
	struct mbuf *opts = 0, *ip_srcroute();
	int optlen = (ip->ip_hl << 2) - sizeof(struct ip);

	t = ip->ip_dst;
	ip->ip_dst = ip->ip_src;
	/*
	 * If the incoming packet was addressed directly to us,
	 * use dst as the src for the reply.  Otherwise (broadcast
	 * or anonymous), use the address which corresponds
	 * to the incoming interface.
	 */
	for (ia = in_ifaddr; ia; ia = ia->ia_next) {
		if (t.s_addr == IA_SIN(ia)->sin_addr.s_addr)
			break;
		if ((ia->ia_ifp->if_flags & IFF_BROADCAST) &&
		    t.s_addr == satosin(&ia->ia_broadaddr)->sin_addr.s_addr)
			break;
	}
	if (ia == (struct in_ifaddr *)0)
		ia = ifptoia(ifp);
	if (ia == (struct in_ifaddr *)0)
		ia = in_ifaddr;
	t = IA_SIN(ia)->sin_addr;
	ip->ip_src = t;
	ip->ip_ttl = MAXTTL;

	if (optlen > 0) {
		register u_char *cp;
		int opt, cnt;
		u_int len;

		/*
		 * Retrieve any source routing from the incoming packet;
		 * add on any record-route or timestamp options.
		 */
		cp = (u_char *) (ip + 1);
		if ((opts = ip_srcroute()) == 0 &&
		    (opts = m_get(M_DONTWAIT, MT_HEADER))) {
			opts->m_len = sizeof(struct in_addr);
			mtod(opts, struct in_addr *)->s_addr = 0;
		}
		if (opts) {
#ifdef ICMPPRINTFS
		    if (icmpprintfs)
			    printf("icmp_reflect optlen %d rt %d => ",
				optlen, opts->m_len);
#endif
		    for (cnt = optlen; cnt > 0; cnt -= len, cp += len) {
			    opt = cp[IPOPT_OPTVAL];
			    if (opt == IPOPT_EOL)
				    break;
			    if (opt == IPOPT_NOP)
				    len = 1;
			    else {
				    len = cp[IPOPT_OLEN];
				    if (len <= 0 || len > cnt)
					    break;
			    }
			    /*
			     * should check for overflow, but it "can't happen"
			     */
			    if (opt == IPOPT_RR || opt == IPOPT_TS) {
				    bcopy((caddr_t)cp,
					mtod(opts, caddr_t) + opts->m_len, len);
				    opts->m_len += len;
			    }
		    }
		    if (opts->m_len % 4 != 0) {
			    *(mtod(opts, caddr_t) + opts->m_len) = IPOPT_EOL;
			    opts->m_len++;
		    }
#ifdef ICMPPRINTFS
		    if (icmpprintfs)
			    printf("%d\n", opts->m_len);
#endif
		}
		/*
		 * Now strip out original options by copying rest of first
		 * mbuf's data back, and adjust the IP length.
		 */
		ip->ip_len -= optlen;
		ip->ip_hl = sizeof(struct ip) >> 2;
/* XXXX WAS, BUT DIDN'T COMPILE AS:
		m->m_len -= optlen;
		optlen += sizeof(struct ip);
		bcopy((caddr_t)ip + optlen, (caddr_t)(ip + 1),
		    m->m_len - sizeof(struct ip));
*/
		opts->m_len -= optlen;
		optlen += sizeof(struct ip);
		bcopy((caddr_t)ip + optlen, (caddr_t)(ip + 1),
					    opts->m_len - sizeof(struct ip));
	}

	icmp_send(ip, opts);
	if (opts)
		(void)m_free(opts);
}

struct in_ifaddr *
ifptoia(ifp)
	struct ifnet *ifp;
{
	register struct in_ifaddr *ia;

	for (ia = in_ifaddr; ia; ia = ia->ia_next)
		if (ia->ia_ifp == ifp)
			return (ia);
	return ((struct in_ifaddr *)0);
}

/*
 * Send an icmp packet back to the ip level,
 * after supplying a checksum.
 */

void icmp_send(ip, opts)
	register struct ip *ip;
	struct mbuf *opts;
{
	register int hlen;
	register struct icmp *icp;
	register struct mbuf *m;

	m = dtom(ip);
	hlen = ip->ip_hl << 2;
	m->m_off += hlen;
	m->m_len -= hlen;
	icp = mtod(m, struct icmp *);
	icp->icmp_cksum = 0;
	icp->icmp_cksum = in_cksum(m, ip->ip_len - hlen);
	m->m_off -= hlen;
	m->m_len += hlen;
#ifdef ICMPPRINTFS
	if (icmpprintfs)
		printf("icmp_send dst %x src %x\n", ip->ip_dst, ip->ip_src);
#endif
	(void) ip_output(m, opts, (struct route *)0, 0);
}

/************FIX THIS********************/

n_time
iptime()
{
/*
	struct timeval atv;
	u_long t;

	microtime(&atv); 
	t = (atv.tv_sec % (24*60*60)) * 1000 + atv.tv_usec / 1000;
	return (htonl(t));
*/
	return 0;
}
