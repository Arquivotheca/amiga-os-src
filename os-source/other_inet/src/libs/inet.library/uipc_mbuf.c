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
 *	@(#)uipc_mbuf.c	7.4.1.3 (Berkeley) 2/15/89
 */

#include <sys/param.h>
#include <sys/user.h>
#include <sys/mbuf.h>
#include <sys/kernel.h>
#include <syslog.h>
#include <sys/domain.h>
#include <sys/protosw.h>

#ifdef AMIGA
#include <exec/types.h>
#include <exec/memory.h>
#ifdef LATTICE
#include <proto/exec.h>
#endif
#endif

#ifdef AZTEC_C
#include <functions.h>
#endif

#include "proto.h"

/* these declarations from sys/mbuf.h */
struct	mbstat mbstat;
struct	mbuf *mfree, *mclfree;
int	m_want;

void mbinit()
{
	int s;

	s = splimp();
	if (m_clalloc(1, MPG_MBUFS, M_DONTWAIT) == 0){
		panic("mbinit");
	}
	splx(s);

	enter_nlist("_mbstat", &mbstat);

	return;
}

#define MBUFS_AT_INIT	128	/* number of buffer available at init time */
static char *mbuf_base;		/* memory allocated to mbufs	*/
static int mbuf_size;		/* size of mbuf allocation	*/

caddr_t
m_clalloc(ncl, how, canwait)
	register int ncl;
	int how;
{
	register struct mbuf *m;
	register int num_bufs;
	void *AllocMem();

/************************************************
check if already allocated. commented out by MMH
as a temporary fix to mbuf problem

	if(mbuf_base){
		return 0;
	}
************************************************/

	num_bufs = MBUFS_AT_INIT;
	mbuf_size = (num_bufs + 1)*MSIZE;
	mbuf_base = (char *)AllocMem(mbuf_size, MEMF_PUBLIC|MEMF_CLEAR);
	if(!mbuf_base){
		return 0;
	}
	m = dtom(mbuf_base + MSIZE - 1);
	switch (how) {
	case MPG_MBUFS:
		while(num_bufs-- > 0) {
			m->m_off = 0;
			m->m_type = MT_DATA;
			m->m_next = 0;
			mbstat.m_mtypes[MT_DATA]++;
			mbstat.m_mbufs++;
			(void) m_free(m);
			m++;
		}
		break;

	case MPG_SPACE:
		mbstat.m_space++;
		break;
	}
	return (caddr_t)mbuf_base;
}

/*
 * Must be called at splimp.
 */

int m_expand(canwait)
	int canwait;
{
	register struct domain *dp;
	register struct protosw *pr;
	int tries;

	tries = 0;
	for (;;){
		if (m_clalloc(1, MPG_MBUFS, canwait)){
			return (1);
		}
		if (canwait == 0 || tries++){
			return (0);
		}

		/* ask protocols to free space */
		for (dp = domains; dp; dp = dp->dom_next){
			for (pr = dp->dom_protosw; pr < dp->dom_protoswNPROTOSW; pr++){
				if (pr->pr_drain){
					(*pr->pr_drain)();
				}
			}
		}
		mbstat.m_drain++;
	}
}

/*
 * Space allocation routines.
 * These are also available as macros
 * for critical paths.
 */

struct mbuf *
m_get(canwait, type)
	int canwait, type;
{
	register struct mbuf *m;

	Forbid();	/* s = splimp(); */
	if(m = mfree){
		if (m->m_type != MT_FREE){
#ifdef MBTRACER
printf("m_get: panic, got mbuf type %d, wanted %d, m = %x\n", type, MT_FREE, m);
			m_dump(m, 10000);
#endif
			panic("mget");
		}
		m->m_type = type;
		mbstat.m_mtypes[MT_FREE]--;
		mbstat.m_mtypes[type]++;
		mfree = m->m_next;
		m->m_next = 0;
		m->m_off = MMINOFF;
	} else {
		m = m_more(canwait, type);
	}

#ifdef MBTRACER
	m_record("m_get", ((u_long *)&canwait)[-1], m);
#endif
	Permit();	/* splx(s); */

	return m;
}

struct mbuf *
m_getclr(canwait, type)
	int canwait, type;
{
	register struct mbuf *m;

	MGET(m, canwait, type);
	if (m == 0){
		return (0);
	}
	bzero(mtod(m, caddr_t), MLEN);
#ifdef MBTRACER
	m_record("m_getclr", ((u_long *)&canwait)[-1], m);
#endif
	return (m);
}

struct mbuf *
m_free(m)
	struct mbuf *m;
{
	register struct mbuf *n;

	Forbid();	/* s = splimp(); */
#ifdef MBTRACER
	m_record("m_free", ((u_long *)&m)[-1], m);
#endif
	if (m->m_type == MT_FREE){
#ifdef MBTRACER
printf("mfree panic: called from %06x (m=%06x), dump:\n", ((u_long *)&m)[-1],m);
		m_dump(m, 100000);
#endif
		panic("mfree");
		return (struct mbuf *)0;
	}

	mbstat.m_mtypes[m->m_type]--;
	mbstat.m_mtypes[MT_FREE]++;

	m->m_type = MT_FREE;
	n = m->m_next;
	m->m_next = mfree;
	m->m_act = 0;
	m->m_off = 0;
	mfree = m;
	if(m_want){
		m_want = 0;
		wakeup((ulong)&mfree);
	}

	Permit();	/* splx(s); */

	return n;
}

/*
 * Get more mbufs; called from MGET macro if mfree list is empty.
 * Must be called at splimp.
 */
/*ARGSUSED*/
struct mbuf *
m_more(canwait, type)
	int canwait, type;
{
	register struct mbuf *m;

	while (m_expand(canwait) == 0) {
		if (canwait == M_WAIT) {
			mbstat.m_wait++;
			m_want++;
			(void)sleep((ulong)&mfree, PZERO - 1);
			if (mfree){
				break;
			}
		} else {
			mbstat.m_drops++;
			return (NULL);
		}
	}
#define m_more(x,y) (panic("m_more"), (struct mbuf *)0)
	MGET(m, canwait, type);
#undef m_more
	return m;
}

void m_freem(m)
	struct mbuf *m;
{
	register struct mbuf *n;

#ifdef MBTRACER
	m_record("m_freem", ((u_long *)&m)[-1], m);
#endif
	if (m == NULL){
		return;
	}
	Forbid();	/* s = splimp(); */
	do {
		MFREE(m, n);
	} while (m = n);
	Permit();	/* splx(s); */
}

/*
 * Mbuffer utility routines.
 */

/*
/*
 * Make a copy of an mbuf chain starting "off" bytes from the beginning,
 * continuing for "len" bytes.  If len is M_COPYALL, copy to end of mbuf.
 * Should get M_WAIT/M_DONTWAIT from caller.
 */
struct mbuf *
m_copy(m, off, len)
	register struct mbuf *m;
	int off;
	register int len;
{
	register struct mbuf *n, **np;
	struct mbuf *top, *p;

	if (len == 0){
		return (0);
	}
	if (off < 0 || len < 0){
		panic("m_copy");
	}
	while (off > 0) {
		if (m == 0){
			panic("m_copy");
		}
		if (off < m->m_len){
			break;
		}
		off -= m->m_len;
		m = m->m_next;
	}
	np = &top;
	top = 0;
	while (len > 0) {
		if (m == 0) {
			if (len != M_COPYALL){
				panic("m_copy");
			}
			break;
		}
		MGET(n, M_DONTWAIT, m->m_type);
		*np = n;
		if (n == 0){
#ifdef MBTRACER
printf("m_copy: nospace\n");
#endif
			goto nospace;
		}
		n->m_len = MIN(len, m->m_len - off);
		if (m->m_off > MMAXOFF) {
			p = mtod(m, struct mbuf *);
			n->m_off = ((int)p - (int)n) + off;
		} else {
			bcopy(mtod(m, caddr_t)+off, mtod(n, caddr_t),
			    (unsigned)n->m_len);
		}
		if (len != M_COPYALL){
			len -= n->m_len;
		}
		off = 0;
		m = m->m_next;
		np = &n->m_next;
	}
	return (top);

nospace:
	m_freem(top);
	return (0);
}

/*
 * Copy data from an mbuf chain starting "off" bytes from the beginning,
 * continuing for "len" bytes, into the indicated buffer.
 */

/*
 *void m_copydata(m, off, len, cp)
 *	register struct mbuf *m;
 *	register int off;
 *	register int len;
 *	caddr_t cp;
 *{
 *	register unsigned count;
 *
 *	if (off < 0 || len < 0){
 *		panic("m_copydata");
 *	}
 *	while (off > 0) {
 *		if (m == 0){
 *			panic("m_copydata");
 *		}
 *		if (off < m->m_len){
 *			break;
 *		}
 *		off -= m->m_len;
 *		m = m->m_next;
 *	}
 *	while (len > 0) {
 *		if (m == 0){
 *			panic("m_copydata");
 *		}
 *		count = MIN(m->m_len - off, len);
 *		bcopy(mtod(m, caddr_t) + off, cp, count);
 *		len -= count;
 *		cp += count;
 *		off = 0;
 *		m = m->m_next;
 *	}
 *}
 *
 */

void m_cat(m, n)
	struct mbuf *m, *n;
{
	while (m->m_next){
		m = m->m_next;
	}
	while(n){
		if (m->m_off >= MMAXOFF ||
		    m->m_off + m->m_len + n->m_len > MMAXOFF) {
			/* just join the two chains */
			m->m_next = n;
			return;
		}
		/* splat the data from one into the other */
		bcopy(mtod(n, caddr_t), mtod(m, caddr_t) + m->m_len,
		    (u_int)n->m_len);
		m->m_len += n->m_len;
		n = m_free(n);
	}
}

void m_adj(mp, len)
	struct mbuf *mp;
	register int len;
{
	register struct mbuf *m;
	register count;

	if ((m = mp) == NULL){
		return;
	}
	if (len >= 0) {
		while (m != NULL && len > 0) {
			if (m->m_len <= len) {
				len -= m->m_len;
				m->m_len = 0;
				m = m->m_next;
			} else {
				m->m_len -= len;
				m->m_off += len;
				break;
			}
		}
	} else {
		/*
		 * Trim from tail.  Scan the mbuf chain,
		 * calculat    its length and finding the last mbuf.
		 * If the adjustment only affects this mbuf, then just
		 * adjust and return.  Otherwise, rescan and truncate
		 * after the remaining size.
		 */
		len = -len;
		count = 0;
		for (;;) {
			count += m->m_len;
			if (m->m_next == (struct mbuf *)0){
				break;
			}
			m = m->m_next;
		}
		if (m->m_len >= len) {
			m->m_len -= len;
			return;
		}
		count -= len;
		/*
		 * Correct length for chain is "count".
		 * Find the mbuf with last data, adjust its length,
		 * and toss data from remaining mbufs on chain.
		 */
		for (m = mp; m; m = m->m_next) {
			if (m->m_len >= count) {
				m->m_len = count;
				break;
			}
			count -= m->m_len;
		}
		while (m = m->m_next){
			m->m_len = 0;
		}
	}
}

/*
 * Rearange an mbuf chain so that len bytes are contiguous
 * and in the data area of an mbuf (so that mtod and dtom
 * will work for a structure of size len).  Returns the resulting
 * mbuf chain on success, frees it and returns null on failure.
 * If there is room, it will add up to MPULL_EXTRA bytes to the
 * contiguous region in an attempt to avoid being called next time.
 */
struct mbuf *
m_pullup(n, len)
	register struct mbuf *n;
	int len;
{
	register struct mbuf *m;
	register int count;
	int space;

	if (n->m_off + len <= MMAXOFF && n->m_next) {
		m = n;
		n = n->m_next;
		len -= m->m_len;
	} else {
		if (len > MLEN){
			goto bad;
		}
		MGET(m, M_DONTWAIT, n->m_type);
		if (m == 0){
			goto bad;
		}
		m->m_len = 0;
	}
	space = MMAXOFF - m->m_off;
	do {
		count = MIN(MIN(space - m->m_len, len + MPULL_EXTRA), n->m_len);
		bcopy(mtod(n, caddr_t), mtod(m, caddr_t)+m->m_len,
				  (unsigned)count);
		len -= count;
		m->m_len += count;
		n->m_len -= count;
		if (n->m_len){
			n->m_off += count;
		} else {
			n = m_free(n);
		}
	} while (len > 0 && n);
	if (len > 0) {
		(void) m_free(m);
		goto bad;
	}
	m->m_next = n;
	return (m);
bad:
	m_freem(n);
	return (0);
}

#ifdef MBTRACER

#define TRACESIZE 2048
struct mbtrace {
	long	caller;		/* who called			*/
	struct mbuf *m;		/* affected mbuf		*/
	char	*name;		/* name of routine		*/
} mbtrace[TRACESIZE];
static mb_next = 0, mb_total;

m_record(name, caller, m)
	char 	*name;
	long	caller;
	struct mbuf *m;
{
	mb_total++;
	mbtrace[mb_next].m = m;
	mbtrace[mb_next].caller = caller;
	mbtrace[mb_next].name = name;
	if(++mb_next >= TRACESIZE){
		mb_next = 0;
	}
}

/*
** dump last <num> allocates/frees of <m>
*/
int m_dump(m, num)
	register struct mbuf *m;
	int num;
{
	register int mb, i, initial;

	initial = mb = mb_next;
	if(num > TRACESIZE){
		num = TRACESIZE;
	}
	printf("total logged: %d\n", mb_total);
	while(num--){
		for(i = 0; i < TRACESIZE; i++){
			--mb;
			if(mb < 0){
				mb = TRACESIZE-1;
			}
			if(mb == initial){
				goto done;
			}
			if(m == 0 || m == mbtrace[mb].m){
				printf("%04d: %06x called %s on mbuf %x\n",
					mb,
					mbtrace[mb].caller,
					mbtrace[mb].name,
					mbtrace[mb].m);
				break;
			}
		}
		if(i == TRACESIZE){
			break;
		}
	}
done:	return 0;
}
#endif
