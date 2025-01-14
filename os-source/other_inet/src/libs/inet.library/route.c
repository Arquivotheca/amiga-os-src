/*
 * Copyright (c) 1980, 1986 Regents of the University of California.
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
 *	@(#)route.c	7.4 (Berkeley) 6/27/88
 */

#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/user.h>
#include <sys/ioctl.h>
#include <errno.h>

#include <net/if.h>
#include <net/af.h>
#include <net/route.h>

#include "proto.h"

/* these declarations removed from net/route.h */
struct	mbuf *rthost[RTHASHSIZ];
struct	mbuf *rtnet[RTHASHSIZ];
struct	rtstat	rtstat;

int	rttrash;		/* routes not in table but not freed */
struct	sockaddr wildcard;	/* zero valued cookie for wildcard searches */
short	rthashsize = RTHASHSIZ;	/* for netstat, etc. */

void route_nlist_init()
{
	enter_nlist("_rthashsize", &rthashsize);
	enter_nlist("_rthost", &rthost);
	enter_nlist("_rtnet", &rtnet);
	enter_nlist("_rtstat", &rtstat);
}

/*
 * Packet routing routines.
 */

void rtalloc(ro)
	register struct route *ro;
{
	register struct rtentry *rt;
	register struct mbuf *m;
	register u_long hash;
	struct sockaddr *dst = &ro->ro_dst;
	int (*match)(struct sockaddr *, struct sockaddr *); 
	int doinghost, s;
	struct afhash h;
	u_int af = dst->sa_family;
	struct mbuf **table;

	if (ro->ro_rt && ro->ro_rt->rt_ifp && (ro->ro_rt->rt_flags & RTF_UP))
		return;				 /* XXX */
	if (af >= AF_MAX)
		return;
	(*afswitch[af].af_hash)(dst, &h);
	match = afswitch[af].af_netmatch;
	hash = h.afh_hosthash, table = rthost, doinghost = 1;
	s = splnet();
again:
	for (m = table[RTHASHMOD(hash)]; m; m = m->m_next) {
		rt = mtod(m, struct rtentry *);
		if (rt->rt_hash != hash)
			continue;
		if ((rt->rt_flags & RTF_UP) == 0 ||
		    (rt->rt_ifp->if_flags & IFF_UP) == 0)
			continue;
		if (doinghost) {
			if (bcmp((caddr_t)&rt->rt_dst, (caddr_t)dst,
			    sizeof (*dst)))
				continue;
		} else {
			if (rt->rt_dst.sa_family != af ||
			    !(*match)(&rt->rt_dst, dst))
				continue;
		}
		rt->rt_refcnt++;
		splx(s);
		if (dst == &wildcard)
			rtstat.rts_wildcard++;
		ro->ro_rt = rt;
		return;
	}
	if (doinghost) {
		doinghost = 0;
		hash = h.afh_nethash, table = rtnet;
		goto again;
	}
	/*
	 * Check for wildcard gateway, by convention network 0.
	 */
	if (dst != &wildcard) {
		dst = &wildcard, hash = 0;
		goto again;
	}
	splx(s);
	rtstat.rts_unreach++;
}

void rtfree(rt)
	register struct rtentry *rt;
{

	if (rt == 0)
		panic("rtfree");
	rt->rt_refcnt--;
	if (rt->rt_refcnt == 0 && (rt->rt_flags&RTF_UP) == 0) {
		rttrash--;
		(void) m_free(dtom(rt));
	}
}

/*
 * Force a routing table entry to the specified
 * destination to go through the given gateway.
 * Normally called as a result of a routing redirect
 * message from the network layer.
 *
 * N.B.: must be called at splnet or higher
 *
 */

void rtredirect(dst, gateway, flags, src)
	struct sockaddr *dst, *gateway, *src;
	int flags;
{
	struct route ro;
	register struct rtentry *rt;

	/* verify the gateway is directly reachable */
	if (ifa_ifwithnet(gateway) == 0) {
		rtstat.rts_badredirect++;
		return;
	}
	ro.ro_dst = *dst;
	ro.ro_rt = 0;
	rtalloc(&ro);
	rt = ro.ro_rt;
#define	equal(a1, a2) \
	(bcmp((caddr_t)(a1), (caddr_t)(a2), sizeof(struct sockaddr)) == 0)
	/*
	 * If the redirect isn't from our current router for this dst,
	 * it's either old or wrong.  If it redirects us to ourselves,
	 * we have a routing loop, perhaps as a result of an interface
	 * going down recently.
	 */
	if ((rt && !equal(src, &rt->rt_gateway)) || ifa_ifwithaddr(gateway)) {
		rtstat.rts_badredirect++;
		if (rt)
			rtfree(rt);
		return;
	}
	/*
	 * Create a new entry if we just got back a wildcard entry
	 * or the the lookup failed.  This is necessary for hosts
	 * which use routing redirects generated by smart gateways
	 * to dynamically build the routing tables.
	 */
	if (rt &&
	    (*afswitch[dst->sa_family].af_netmatch)(&wildcard, &rt->rt_dst)) {
		rtfree(rt);
		rt = 0;
	}
	if (rt == 0) {
		rtinit(dst, gateway, (int)SIOCADDRT,
		    (flags & RTF_HOST) | RTF_GATEWAY | RTF_DYNAMIC);
		rtstat.rts_dynamic++;
		return;
	}
	/*
	 * Don't listen to the redirect if it's
	 * for a route to an interface. 
	 */
	if (rt->rt_flags & RTF_GATEWAY) {
		if (((rt->rt_flags & RTF_HOST) == 0) && (flags & RTF_HOST)) {
			/*
			 * Changing from route to net => route to host.
			 * Create new route, rather than smashing route to net.
			 */
			rtinit(dst, gateway, (int)SIOCADDRT,
			    flags | RTF_DYNAMIC);
			rtstat.rts_dynamic++;
		} else {
			/*
			 * Smash the current notion of the gateway to
			 * this destination.
			 */
			rt->rt_gateway = *gateway;
			rt->rt_flags |= RTF_MODIFIED;
			rtstat.rts_newgateway++;
		}
	} else
		rtstat.rts_badredirect++;
	rtfree(rt);
}

/*
 * Routing table ioctl interface.
 */

int rtioctl(cmd, data)
	int cmd;
	caddr_t data;
{

	if (cmd != SIOCADDRT && cmd != SIOCDELRT)
		return (EINVAL);
	return (rtrequest(cmd, (struct rtentry *)data));
}

/*
 * Carry out a request to change the routing table.  Called by
 * interfaces at boot time to make their ``local routes'' known,
 * for ioctl's, and as the result of routing redirects.
 */

int rtrequest(req, entry)
	int req;
	register struct rtentry *entry;
{
	register struct mbuf *m, **mprev;
	struct mbuf **mfirst;
	register struct rtentry *rt;
	struct afhash h;
	int s, error = 0, (*match)(struct sockaddr *, struct sockaddr *);
	u_int af;
	u_long hash;
	struct ifaddr *ifa;
	struct ifaddr *ifa_ifwithdstaddr();

	af = entry->rt_dst.sa_family;
	if (af >= AF_MAX)
		return (EAFNOSUPPORT);
	(*afswitch[af].af_hash)(&entry->rt_dst, &h);
	if (entry->rt_flags & RTF_HOST) {
		hash = h.afh_hosthash;
		mprev = &rthost[RTHASHMOD(hash)];
	} else {
		hash = h.afh_nethash;
		mprev = &rtnet[RTHASHMOD(hash)];
	}
	match = afswitch[af].af_netmatch;
	s = splimp();
	for (mfirst = mprev; m = *mprev; mprev = &m->m_next) {
		rt = mtod(m, struct rtentry *);
		if (rt->rt_hash != hash)
			continue;
		if (entry->rt_flags & RTF_HOST) {
			if (!equal(&rt->rt_dst, &entry->rt_dst))
				continue;
		} else {
			if (rt->rt_dst.sa_family != entry->rt_dst.sa_family ||
			    (*match)(&rt->rt_dst, &entry->rt_dst) == 0)
				continue;
		}
		if (equal(&rt->rt_gateway, &entry->rt_gateway))
			break;
	}
	switch (req) {

	case SIOCDELRT:
		if (m == 0) {
			error = ESRCH;
			goto bad;
		}
		*mprev = m->m_next;
		if (rt->rt_refcnt > 0) {
			rt->rt_flags &= ~RTF_UP;
			rttrash++;
			m->m_next = 0;
		} else
			(void) m_free(m);
		break;

	case SIOCADDRT:
		if (m) {
			error = EEXIST;
			goto bad;
		}
		if ((entry->rt_flags & RTF_GATEWAY) == 0) {
			/*
			 * If we are adding a route to an interface,
			 * and the interface is a pt to pt link
			 * we should search for the destination
			 * as our clue to the interface.  Otherwise
			 * we can use the local address.
			 */
			ifa = 0;
			if (entry->rt_flags & RTF_HOST) 
				ifa = ifa_ifwithdstaddr(&entry->rt_dst);
			if (ifa == 0)
				ifa = ifa_ifwithaddr(&entry->rt_gateway);
		} else {
			/*
			 * If we are adding a route to a remote net
			 * or host, the gateway may still be on the
			 * other end of a pt to pt link.
			 */
			ifa = ifa_ifwithdstaddr(&entry->rt_gateway);
		}
		if (ifa == 0) {
			ifa = ifa_ifwithnet(&entry->rt_gateway);
			if (ifa == 0) {
				error = ENETUNREACH;
				goto bad;
			}
		}
		m = m_get(M_DONTWAIT, MT_RTABLE);
		if (m == 0) {
			error = ENOBUFS;
			goto bad;
		}
		m->m_next = *mfirst;
		*mfirst = m;
		m->m_off = MMINOFF;
		m->m_len = sizeof (struct rtentry);
		rt = mtod(m, struct rtentry *);
		rt->rt_hash = hash;
		rt->rt_dst = entry->rt_dst;
		rt->rt_gateway = entry->rt_gateway;
		rt->rt_flags = RTF_UP |
		    (entry->rt_flags & (RTF_HOST|RTF_GATEWAY|RTF_DYNAMIC));
		rt->rt_refcnt = 0;
		rt->rt_use = 0;
		rt->rt_ifp = ifa->ifa_ifp;
		break;
	}
bad:
	splx(s);
	return (error);
}

/*
 * Set up a routing table entry, normally
 * for an interface.
 */

void rtinit(dst, gateway, cmd, flags)
	struct sockaddr *dst, *gateway;
	int cmd, flags;
{
	struct rtentry route;

	bzero((caddr_t)&route, sizeof (route));
	route.rt_dst = *dst;
	route.rt_gateway = *gateway;
	route.rt_flags = flags;
	(void) rtrequest(cmd, &route);
}
