/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
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
 *	@(#)udp_usrreq.c	7.8 (Berkeley) 7/28/88
 */

#include <sys/param.h>
#include <sys/user.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <errno.h>
#include <stat.h>

#include <net/if.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_pcb.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>

#include "proto.h"

/*
 * UDP protocol implementation.
 * Per RFC 768, August, 1980.
 */

void udp_init()
{
	enter_nlist("_udb", &udb);
	enter_nlist("_udpstat", &udpstat);

	udb.inp_next = udb.inp_prev = &udb;
}

#ifndef	COMPAT_42
int	udpcksum = 1;
#else
int	udpcksum = 0;		/* XXX */
#endif
int	udp_ttl = UDP_TTL;

struct	sockaddr_in udp_in = { AF_INET };

void udp_input(m0, ifp)
	struct mbuf *m0;
	struct ifnet *ifp;
{
	register struct udpiphdr *ui;
	register struct inpcb *inp;
	register struct mbuf *m;
	struct in_addr dummy;
	int len;
	struct ip ip;

	/*
	 * Get IP and UDP header together in first mbuf.
	 * Note: IP leaves IP header in first mbuf.
	 */
	m = m0;
	ui = mtod(m, struct udpiphdr *);
	if (((struct ip *)ui)->ip_hl > (sizeof (struct ip) >> 2))
		ip_stripoptions((struct ip *)ui, (struct mbuf *)0);
	if (m->m_off > MMAXOFF || m->m_len < sizeof (struct udpiphdr)) {
		if ((m = m_pullup(m, sizeof (struct udpiphdr))) == 0) {
			udpstat.udps_hdrops++;
			return;
		}
		ui = mtod(m, struct udpiphdr *);
	}

	/*
	 * Make mbuf data length reflect UDP length.
	 * If not enough data to reflect UDP length, drop.
	 */
	len = ntohs((u_short)ui->ui_ulen);
	if (((struct ip *)ui)->ip_len != len) {
		if (len > ((struct ip *)ui)->ip_len) {
			udpstat.udps_badlen++;
			goto bad;
		}
		m_adj(m, len - ((struct ip *)ui)->ip_len);
		/* ((struct ip *)ui)->ip_len = len; */
	}
	/*
	 * Save a copy of the IP header in case we want restore it for ICMP.
	 */
	ip = *(struct ip*)ui;

	/*
	 * Checksum extended UDP header and data.
	 */
	if (udpcksum && ui->ui_sum) {
		ui->ui_next = ui->ui_prev = 0;
		ui->ui_x1 = 0;
		ui->ui_len = ui->ui_ulen;
		if (ui->ui_sum = in_cksum(m, len + sizeof (struct ip))) {
			udpstat.udps_badsum++;
			m_freem(m);
			return;
		}
	}

	/*
	 * Locate pcb for datagram.
	 */
	inp = in_pcblookup(&udb,
	    ui->ui_src, ui->ui_sport, ui->ui_dst, ui->ui_dport,
		INPLOOKUP_WILDCARD);
	if (inp == 0) {
		/* don't send ICMP response for broadcast packet */
		if (in_broadcast(ui->ui_dst))
			goto bad;
		*(struct ip *)ui = ip;
		icmp_error((struct ip *)ui, ICMP_UNREACH, ICMP_UNREACH_PORT,
		    ifp, dummy);
		return;
	}

	/*
	 * Construct sockaddr format source address.
	 * Stuff source address and datagram in user buffer.
	 */
	udp_in.sin_port = ui->ui_sport;
	udp_in.sin_addr = ui->ui_src;
	m->m_len -= sizeof (struct udpiphdr);
	m->m_off += sizeof (struct udpiphdr);
	if (sbappendaddr(&inp->inp_socket->so_rcv, (struct sockaddr *)&udp_in,
	    m, (struct mbuf *)0) == 0)
		goto bad;
	sorwakeup(inp->inp_socket);
	return;
bad:
	m_freem(m);
}

/*
 * Notify a udp user of an asynchronous error;
 * just wake up so that he can collect error status.
 */

void udp_notify(inp)
	register struct inpcb *inp;
{

	sorwakeup(inp->inp_socket);
	sowwakeup(inp->inp_socket);
}

void udp_ctlinput(cmd, sa)
	int cmd;
	struct sockaddr *sa;
{
	extern u_char inetctlerrmap[];
	struct sockaddr_in *sin;

	if ((unsigned)cmd > PRC_NCMDS)
		return;
	if (sa->sa_family != AF_INET && sa->sa_family != AF_IMPLINK)
		return;
	sin = (struct sockaddr_in *)sa;
	if (sin->sin_addr.s_addr == INADDR_ANY)
		return;

	switch (cmd) {

	case PRC_QUENCH:
		break;

	case PRC_ROUTEDEAD:
	case PRC_REDIRECT_NET:
	case PRC_REDIRECT_HOST:
	case PRC_REDIRECT_TOSNET:
	case PRC_REDIRECT_TOSHOST:
		in_pcbnotify(&udb, &sin->sin_addr, 0, in_rtchange);
		break;

	default:
		if (inetctlerrmap[cmd] == 0)
			return;		/* XXX */
		in_pcbnotify(&udb, &sin->sin_addr, (int)inetctlerrmap[cmd],
			udp_notify);
	}
}

int udp_output(inp, m0)
	register struct inpcb *inp;
	struct mbuf *m0;
{
	register struct mbuf *m;
	register struct udpiphdr *ui;
	register int len = 0;

	/*
	 * Calculate data length and get a mbuf
	 * for UDP and IP headers.
	 */
	for (m = m0; m; m = m->m_next)
		len += m->m_len;
	MGET(m, M_DONTWAIT, MT_HEADER);
	if (m == 0) {
		m_freem(m0);
		return (ENOBUFS);
	}

	/*
	 * Fill in mbuf with extended UDP header
	 * and addresses and length put into network format.
	 */
	m->m_off = MMAXOFF - sizeof (struct udpiphdr);
	m->m_len = sizeof (struct udpiphdr);
	m->m_next = m0;
	ui = mtod(m, struct udpiphdr *);
	ui->ui_next = ui->ui_prev = 0;
	ui->ui_x1 = 0;
	ui->ui_pr = IPPROTO_UDP;
	ui->ui_len = htons((u_short)len + sizeof (struct udphdr));
	ui->ui_src = inp->inp_laddr;
	ui->ui_dst = inp->inp_faddr;
	ui->ui_sport = inp->inp_lport;
	ui->ui_dport = inp->inp_fport;
	ui->ui_ulen = ui->ui_len;

	/*
	 * Stuff checksum and output datagram.
	 */
	ui->ui_sum = 0;
	if (udpcksum) {
	    if ((ui->ui_sum = in_cksum(m, sizeof (struct udpiphdr) + len)) == 0)
		ui->ui_sum = 0xffff;
	}
	((struct ip *)ui)->ip_len = sizeof (struct udpiphdr) + len;
	((struct ip *)ui)->ip_ttl = udp_ttl;
	return (ip_output(m, inp->inp_options, &inp->inp_route,
	    inp->inp_socket->so_options & (SO_DONTROUTE | SO_BROADCAST)));
}

#ifdef AMIGA
u_long	udp_sendspace = 9000;		/* really max datagram size */
u_long	udp_recvspace = 4 * (9000+sizeof(struct sockaddr_in)); /* 4 8K dgrams */
#else
u_long	udp_sendspace = 2048;		/* really max datagram size */
u_long	udp_recvspace = 4 * (1024+sizeof(struct sockaddr_in)); /* 4 1K dgrams */
#endif

int udp_usrreq(so, req, m, nam, rights)
	struct socket *so;
	int req;
	struct mbuf *m, *nam, *rights;
{
	struct inpcb *inp = sotoinpcb(so);
	int error = 0;

	if (req == PRU_CONTROL) {
		return (in_control(so, (int)m, (caddr_t)nam,(struct ifnet *)rights));
	}
	if (rights && rights->m_len) {
		error = EINVAL;
		goto release;
	}
	if (inp == NULL && req != PRU_ATTACH) {
		error = EINVAL;
		goto release;
	}
	switch (req) {

	case PRU_ATTACH:
		if (inp != NULL) {
			error = EINVAL;
			break;
		}
		error = in_pcballoc(so, &udb);
		if (error)
			break;
		error = soreserve(so, udp_sendspace, udp_recvspace);
		if (error)
			break;
		break;

	case PRU_DETACH:
		in_pcbdetach(inp);
		break;

	case PRU_BIND:
		error = in_pcbbind(inp, nam);
		break;

	case PRU_LISTEN:
		error = EOPNOTSUPP;
		break;

	case PRU_CONNECT:
		if (inp->inp_faddr.s_addr != INADDR_ANY) {
			error = EISCONN;
			break;
		}
		error = in_pcbconnect(inp, nam);
		if (error == 0)
			soisconnected(so);
		break;

	case PRU_CONNECT2:
		error = EOPNOTSUPP;
		break;

	case PRU_ACCEPT:
		error = EOPNOTSUPP;
		break;

	case PRU_DISCONNECT:
		if (inp->inp_faddr.s_addr == INADDR_ANY) {
			error = ENOTCONN;
			break;
		}
		in_pcbdisconnect(inp);
		so->so_state &= ~SS_ISCONNECTED;		/* XXX */
		break;

	case PRU_SHUTDOWN:
		socantsendmore(so);
		break;

	case PRU_SEND: {
		struct in_addr laddr;
		int s;

		if (nam) {
			laddr = inp->inp_laddr;
			if (inp->inp_faddr.s_addr != INADDR_ANY) {
				error = EISCONN;
				break;
			}
			/*
			 * Must block input while temporarily connected.
			 */
			Forbid();	/* s = splnet(); */
			error = in_pcbconnect(inp, nam);
			if (error) {
				Permit();	/* splx(s); */
				break;
			}
		} else {
			if (inp->inp_faddr.s_addr == INADDR_ANY) {
				error = ENOTCONN;
				break;
			}
		}
		error = udp_output(inp, m);
		m = NULL;
		if (nam) {
			in_pcbdisconnect(inp);
			inp->inp_laddr = laddr;
			Permit();	/* splx(s); */
		}
		}
		break;

	case PRU_ABORT:
		soisdisconnected(so);
		in_pcbdetach(inp);
		break;

	case PRU_SOCKADDR:
		in_setsockaddr(inp, nam);
		break;

	case PRU_PEERADDR:
		in_setpeeraddr(inp, nam);
		break;

	case PRU_SENSE:
		/*
		 * stat: don't bother with a blocksize.
		 */
		return (0);

	case PRU_SENDOOB:
	case PRU_FASTTIMO:
	case PRU_SLOWTIMO:
	case PRU_PROTORCV:
	case PRU_PROTOSEND:
		error =  EOPNOTSUPP;
		break;

	case PRU_RCVD:
	case PRU_RCVOOB:
		return (EOPNOTSUPP);	/* do not free mbuf's */

	default:
		panic("udp_usrreq");
	}
release:
	if (m != NULL)
		m_freem(m);
	return (error);
}
