/*
 * if_ae.c - Commodore and Ameristar family of Ethernet controllers
 */

#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/syslog.h>
#include <sys/ioctl.h>
#include <errno.h>

#include <net/if.h>
#include <net/netisr.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <netinet/inet.h>

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/interrupts.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <hardware/intbits.h>
#include <libraries/configvars.h>
#include <ameristar.h>
#include <cbm.h>
#include <aeif/ae.h>

#ifdef LATTICE
#include <proto/exec.h>
#include <proto/expansion.h>
#endif

#ifdef AZTEC_C
#include <functions.h>
#endif

#include "proto.h"

int netisr; /* used in netisr.h */

static void aeinit (int unit);
static void aeinterrupts (int on);
static void aeread (register struct ae_softc *es, register int rbuf);
static int aeoutput (struct ifnet *ifp, struct mbuf *m0, struct sockaddr *dst);
static struct mbuf *aeget (u_char *aebuf, int totlen, int off0, struct ifnet *ifp);
static int aeput (u_char *bp, struct mbuf *m);
static void aestart (int unit);
static int aeioctl (register struct ifnet *ifp, int cmd, caddr_t data);
static int attach (register struct ae_softc *es);


extern struct ifnet loif;

/*
 * Ethernet software status per interface.
 *
 * Each interface is referenced by a network interface structure,
 * es_if, which the routing code uses to locate the interface.
 * This structure contains the output queue for the interface, its address, ...
 */

#define NAE 	5			/* max number of enet brds per amiga */
#define MAXBUF	42			/* max number of packet buffers/brd  */

#define ENETBUFSIZE	1600		/* room for more than 1 packet	     */
#define TOTSIZE(x, r) (sizeof(*es->es_iadr) + ENETBUFSIZE*((x)+(r)) + \
			sizeof(*es->es_rdra)*(r) + sizeof(*es->es_tdra)*(x))

static struct ae_softc {
	struct	arpcom es_ac;		/* common Ethernet structures 	     */
#define	es_if	es_ac.ac_if		/* network-visible interface         */
#define	es_addr	es_ac.ac_enaddr		/* hardware Ethernet address         */
	struct ae_iadr *es_iadr;	/* ptr to Init block in enet brd mem */
	struct ae_rdra *es_rdra;	/* ptr to Rcv ring in enet brd mem   */
	struct ae_tdra *es_tdra;	/* ptr to Xmit ring in enet brd memory*/
	u_char	*es_brdaddr;		/* board address		     */
	u_char	*es_buf[MAXBUF];	/* addresses of buffers              */
	u_long	es_snum;		/* serial num of board               */
	u_short	*es_csr;		/* ptr to LANCE CSR register	     */
	u_char	*es_ram;		/* ptr to start of ram		     */
	u_short	es_rcvbuf;		/* number of rcv buffers             */
	u_short	es_xmtbuf;		/* number of xmit buffers            */
	u_short	es_xmt_next;		/* next buf available for xmit	     */
	u_short es_rcv_next;		/* place where next rcvd packet is   */
	u_long	es_memsize;		/* how much memory board has	     */
	u_short	es_running;		/* interface is configured/running   */
} ae_softc[NAE];

/*
 * Initialization of interface.  format ring, build init block, instantiate
 * interrupt stub, init lance, start rcvr.  Like whew.  Although this routine
 * is made public through if_init, it is called from aeioctl() only.
 */
static void
aeinit(unit)
	int unit;
{
	register struct ae_softc *es = &ae_softc[unit];
	register struct ae_iadr *iadr;
	struct ifnet *ifp = &es->es_if;
	u_short *csr;
	u_char *p;
	int i;

/*	kprintf("AEINIT\n");
	kprintf("&ae_softc[0]=%lx\n",&ae_softc[0]); */

	if (ifp->if_addrlist == (struct ifaddr *)0){
		return;
	}
/*
kprintf("es_iadr=   %lx  es_rdra=%lx  es_tdra=%lx\n",es->es_iadr,es->es_rdra,es->es_tdra);
kprintf("es_brdaddr=%lx  es_buf= %lx  es_snum=%ld\n",es->es_brdaddr,es->es_buf,es->es_snum);
kprintf("es_csr=    %lx  es_ram= %lx\n",es->es_csr,es->es_ram);
kprintf("rcvbuf=    %lx  xmtbuf= %lx\n",es->es_rcvbuf,es->es_xmtbuf);
kprintf("rcv_next=  %lx  xmt_nxt=%lx\n",es->es_rcv_next,es->es_xmt_next);
kprintf("size=      %lx  running=%lx\n",es->es_memsize,es->es_running);
kprintf("ifp=%lx\n",ifp);
*/
	Forbid();	/* s = splimp(); */
	if ((ifp->if_flags & IFF_RUNNING) == 0) {
		*es->es_csr = AE_STOP;

		iadr = es->es_iadr;
		bzero(iadr, sizeof(*iadr));
		iadr->padr[1] = es->es_addr[0];	iadr->padr[0] = es->es_addr[1];
		iadr->padr[3] = es->es_addr[2];	iadr->padr[2] = es->es_addr[3];
		iadr->padr[5] = es->es_addr[4];	iadr->padr[4] = es->es_addr[5];
		iadr->rdra = (u_short)((long)es->es_rdra)&0xffff;
		iadr->rdra_hi = (u_short)(((long)es->es_rdra)>>16);
		iadr->rlen = ffs(es->es_rcvbuf) - 1;
		iadr->tdra = (u_short)((long)es->es_tdra)&0xffff;
		iadr->tdra_hi = (u_short)(((long)es->es_tdra)>>16);
		iadr->tlen = ffs(es->es_xmtbuf) - 1;

		for(i = 0; i < es->es_rcvbuf; i++){
			register struct ae_rdra *rdra;

			rdra = es->es_rdra + i;
			p = es->es_buf[i+es->es_xmtbuf];
			rdra->rbadr = (u_short) ((long)p) & 0xffff;
			rdra->ha = (u_short)(((long)p) >> 16);
			rdra->bcnt = -ENETBUFSIZE;
			rdra->mcnt = 0;
			rdra->flags = RDRA_OWN;
		}

		for(i = 0; i < es->es_xmtbuf; i++){
			register struct ae_tdra *tdra;

			tdra = es->es_tdra + i;
			p = es->es_buf[i];
			tdra->tbadr = (u_short) ((long)p) & 0xffff;
			tdra->ha = (u_short)(((long)p) >> 16);
			tdra->flags = TDRA_ENP|TDRA_STP;
		}
/* kprintf("AEINIT: calling aeinterrupts(1)\n");  */
		aeinterrupts(1);
/* kprintf("AEINIT: still here\n");                 */

		Disable();
		csr = es->es_csr;
		csr[1] = CSR3;
		*csr = CSR3_BSWAP;
		csr[1] = CSR2;
		*csr = (u_short)((long)iadr)>>16;
		csr[1] = CSR1;
		*csr = (u_short)((long)iadr)&0xffff;
		csr[1] = CSR0;
		*csr = AE_STRT | AE_INIT | AE_INEA;
		es->es_if.if_flags |= IFF_RUNNING;
		if (es->es_if.if_snd.ifq_head){
			aestart(unit);
		}
		es->es_running = 1;
		Enable();
	}
/*	kprintf("AEINIT: done\n"); */
	Permit();	/* splx(s); */
}

/*
 * aeinterrupts() - called from aeinit, etc to set up interrupts for
 *		    board.
*/
static void
aeinterrupts(int on)
{
	static struct Interrupt *ae_in;
	static	interrupts_on;
#ifdef LATTICE
	extern void __saveds ae_intr(void);
	extern void __saveds __stdargs aeintrC(void);
#else
	extern void ae_intr();
#endif

	if(on && !interrupts_on){
		ae_in = (struct Interrupt *)AllocMem(sizeof(*ae_in),
						MEMF_CLEAR|MEMF_PUBLIC);
		ae_in->is_Code = (void (*)())ae_intr;
		ae_in->is_Data = (APTR)InetBase;
		ae_in->is_Node.ln_Type = NT_INTERRUPT;
		ae_in->is_Node.ln_Pri = 100;
		ae_in->is_Node.ln_Name = "ae_intr()";
		AddIntServer(LEINTB, ae_in);
		interrupts_on++;
	}

	if(interrupts_on && !on){
		RemIntServer(LEINTB, ae_in);
		FreeMem(ae_in, sizeof(*ae_in));
		interrupts_on = 0;
	}
}

/*
 * aeintrC is called at interrupt time.  Strategy is to ack the interrupts
 * here, then signal the net process to handle the rest.
*/
int ae_ints;
short units_configured;

#define AE_EVENTS (AE_RINT|AE_TINT|AE_MISS|AE_CERR|AE_BABL|AE_IDON|AE_MERR)
#ifdef LATTICE
void __saveds __stdargs aeintrC(void)
#else
void aeintrC(void)
#endif
{
	register short callhigher, units=units_configured;
	register struct ae_softc *es;
	register u_short *csr, set;

	callhigher = 0;
	es = &ae_softc[0];

	do {
		if(es->es_running){
			csr = es->es_csr;
			if(set = (*csr & AE_EVENTS)){
				*csr = set | AE_INEA;	/* INEA playing conservative */
				if(set & AE_MISS) es->es_if.if_collisions++;
				callhigher++;
/*				ae_ints++;        */
			}
		}
		es++;
	} while(units--);

	if(callhigher)
		setsoftif();

}

/*
 * aepoll - called from net process to handle packet arrivals
*/
void aepoll()
{
	register struct ae_softc *es;
	register struct ae_rdra *rdra;
	register unit;

	es = &ae_softc[0];
	Forbid();	/* s = splimp(); */
	for(unit = 0; unit < units_configured; unit++){
		if(es->es_running){
			while(1) {
				rdra = &es->es_rdra[es->es_rcv_next];
				if(!(rdra->flags & RDRA_OWN)){
					if(!(rdra->flags & FATALERR)){
						aeread(es, es->es_rcv_next);
						es->es_if.if_ipackets++;
					} else {
						es->es_if.if_ierrors++;
					}
					rdra->flags = RDRA_OWN;
				} else {
					break;
				}
				es->es_rcv_next = (es->es_rcv_next + 1) & (es->es_rcvbuf - 1);
			}

			if(!(es->es_tdra[es->es_xmt_next].flags & TDRA_OWN)){
				if(es->es_if.if_snd.ifq_head){
					aestart(unit);
				}
			}
		}
		es++;
	}
	Permit();	/* splx(s); */
}

/*
 * Process incoming packet, handle trailers.  Read packet into m chain,
 * submit to protocol stack.
*/
static void
aeread(es, rbuf)
	register struct ae_softc *es;
	register int rbuf;
{
	register struct ether_header *ae;
	register struct ifqueue *inq;
	int len, off, resid;
	struct mbuf *m;
	u_char *aebuf;

	aebuf = es->es_buf[rbuf + es->es_xmtbuf];
	len = es->es_rdra[rbuf].mcnt - sizeof(struct ether_header) - 4;
	ae = (struct ether_header *)aebuf;
	ae->ether_type = ntohs((u_short)ae->ether_type);

#define	aedataaddr(ae, off, type)	((type)(((caddr_t)((ae)+1)+(off))))
	if (ae->ether_type >= ETHERTYPE_TRAIL &&
	    ae->ether_type < ETHERTYPE_TRAIL+ETHERTYPE_NTRAILER) {
		off = (ae->ether_type - ETHERTYPE_TRAIL) * 512;
		if (off >= ETHERMTU){
			return ;
		}
		ae->ether_type = ntohs(*aedataaddr(ae, off, u_short *));
		resid = ntohs(*(aedataaddr(ae, off+2, u_short *)));
		if (off + resid > len){
			return ;
		}
		len = off + resid;
	} else {
		off = 0;
	}

	if (len == 0){
		return;
	}

	m = aeget(aebuf, len, off, &es->es_if);
	if (m == 0){
		return;
	}
	if(off){
		struct ifnet *ifp;

		ifp = *(mtod(m, struct ifnet **));
		m->m_off += 2 * sizeof (u_short);
		m->m_len -= 2 * sizeof (u_short);
		*(mtod(m, struct ifnet **)) = ifp;
	}
	switch (ae->ether_type) {

#ifdef INET
	case ETHERTYPE_IP:
		schednetisr(NETISR_IP);
		inq = &ipintrq;
		break;

	case ETHERTYPE_ARP:
		arpinput(&es->es_ac, m);
		return ;
#endif
	default:
		m_freem(m);
		return;
	}

	if (IF_QFULL(inq)) {
		IF_DROP(inq);
		m_freem(m);
		return ;
	}
	IF_ENQUEUE(inq, m);
}

/*
 * Read packet off ethernet board into mchain.
 *
 * FIX trailer processing on this procedure
*/
static struct mbuf *
aeget(aebuf, totlen, off0, ifp)
	u_char *aebuf;
	int totlen, off0;
	struct ifnet *ifp;
{
	struct mbuf *top = 0, **mp = &top;
	register int len;
	register struct mbuf *m;
	u_char *cp;

	cp = aebuf + sizeof (struct ether_header);
	while (totlen > 0) {
/*		register int words; */
		u_char *mcp;

		MGET(m, M_DONTWAIT, MT_DATA);
		if(m == 0){
			m_freem(top);
			return 0;
		}

		len = totlen;
		if (ifp){
			len += sizeof(ifp);
		}

		len = m->m_len = MIN(MLEN, len);
		m->m_off = MMINOFF;
		mcp = mtod(m, u_char *);
		if (ifp) {
			*(mtod(m, struct ifnet **)) = ifp;
			mcp += sizeof(ifp);
			len -= sizeof(ifp);
			ifp = (struct ifnet *)0;
		}

/*** changed by Dale and Martin 2 Apr 90 ***/
		bcopy(cp,mcp,len);
		cp += len;
		mcp += len;

/*** original Ameristar code follows
		if (words = (len >> 1)) {
			register u_short *to, *from;

			to = (u_short *)mcp;
			from = (u_short *)cp;
			do {
				*to++ = *from++;
			} while (--words > 0);
			mcp = (u_char *)to;
			cp = (u_char *)from;
		}

		if (len & 01){
			*mcp++ = *cp++;
		}
***********************************/

		*mp = m;
		mp = &m->m_next;
		totlen -= len;
	}

	return top;
}

/*
 * Ethernet output routine.
 * Encapsulate a packet of type family for the local net.
 * Use trailer local net encapsulation if enough data in first
 * packet leaves a multiple of 512 bytes of data in remainder.
 * If destination is this address or broadcast, send packet to
 * loop device to kludge around the fact that Ameristar interfaces can't
 * talk to themselves.
 */
static int
aeoutput(ifp, m0, dst)
	struct ifnet *ifp;
	struct mbuf *m0;
	struct sockaddr *dst;
{
	int type, error;
 	u_char edst[6];
	struct in_addr idst;
	register struct ae_softc *es = &ae_softc[ifp->if_unit];
	register struct mbuf *m = m0;
	register struct ether_header *ae;
	struct mbuf *mcopy = (struct mbuf *)0;
	int usetrailers;
	register int off;

	if ((ifp->if_flags & (IFF_UP|IFF_RUNNING)) != (IFF_UP|IFF_RUNNING)) {
		error = ENETDOWN;
		goto bad;
	}
	switch (dst->sa_family) {

#ifdef INET
	case AF_INET:
		idst = ((struct sockaddr_in *)dst)->sin_addr;
		if (!arpresolve(&es->es_ac, m, &idst, edst, &usetrailers)){
			return (0);
		}
		if(!bcmp((caddr_t)edst, (caddr_t)etherbroadcastaddr, sizeof(edst))){
			mcopy = m_copy(m, 0, (int)M_COPYALL);
		}
		off = ntohs((u_short)mtod(m, struct ip *)->ip_len) - m->m_len;

		if (usetrailers && off > 0 && (off & 0x1ff) == 0 &&
		    m->m_off >= MMINOFF + 2 * sizeof (u_short)) {
			type = ETHERTYPE_TRAIL + (off>>9);
			m->m_off -= 2 * sizeof (u_short);
			m->m_len += 2 * sizeof (u_short);
			*mtod(m, u_short *) = ntohs((u_short)ETHERTYPE_IP);
			*(mtod(m, u_short *) + 1) = ntohs((u_short)m->m_len);
			goto gottrailertype;
		}
		type = ETHERTYPE_IP;
		off = 0;
		goto gottype;
#endif

	case AF_UNSPEC:
		ae = (struct ether_header *)dst->sa_data;
 		bcopy((caddr_t)ae->ether_dhost, (caddr_t)edst, sizeof (edst));
		type = ae->ether_type;
		goto gottype;

	default:
		printf("ae%d: bad address family af%d\n",
				ifp->if_unit, dst->sa_family);
		error = EAFNOSUPPORT;
		goto bad;
	}

gottrailertype:
	while (m->m_next){
		m = m->m_next;
	}
	m->m_next = m0;
	m = m0->m_next;
	m0->m_next = 0;
	m0 = m;

gottype:
	if (m->m_off > MMAXOFF ||
	    MMINOFF + sizeof (struct ether_header) > m->m_off) {
		m = m_get(M_DONTWAIT, MT_HEADER);
		if (m == 0) {
			error = ENOBUFS;
			goto bad;
		}
		m->m_next = m0;
		m->m_off = MMINOFF;
		m->m_len = sizeof (struct ether_header);
	} else {
		m->m_off -= sizeof (struct ether_header);
		m->m_len += sizeof (struct ether_header);
	}
	ae = mtod(m, struct ether_header *);
 	bcopy((caddr_t)edst, (caddr_t)ae->ether_dhost, sizeof (edst));
	bcopy((caddr_t)es->es_addr, (caddr_t)ae->ether_shost,
					    sizeof(ae->ether_shost));
	ae->ether_type = htons((u_short)type);

	Forbid();	/* s = splimp(); */
	if (IF_QFULL(&ifp->if_snd)) {
		IF_DROP(&ifp->if_snd);
		error = ENOBUFS;
		goto qfull;
	}
	IF_ENQUEUE(&ifp->if_snd, m);

	aestart(ifp->if_unit);

	Permit();	/* splx(s); */
	return (mcopy ? looutput(&loif, mcopy, dst) : 0);

qfull:
	m0 = m;
	Permit();	/* splx(s); */
bad:
	m_freem(m0);
	if (mcopy){
		m_freem(mcopy);
	}
	return (error);
}

/*
 * Start output on interface.  Get another datagram to send
 * off of the interface queue, and copy it to the interface
 * before starting the output.
 */
static void
aestart(unit)
{
	register struct ae_softc *es = &ae_softc[unit];
	register int curr, len;
	struct mbuf *m;

	Forbid();	/* s = splimp(); */
	if ((es->es_if.if_flags & IFF_RUNNING) == 0){
		Permit();	/* splx(s); */
		return;
	}

	/* now do all packets we can - Dale */
	while(1) {
		curr = es->es_xmt_next;
		if (es->es_tdra[curr].flags & TDRA_OWN) {
			/* next one still in use */
			Permit();	/* splx(s); */
			return;
		}

		IF_DEQUEUE(&es->es_if.if_snd, m);
		if (m == 0){
			Permit();	/* splx(s); */
			return;
		}
		es->es_xmt_next = (curr + 1) & (es->es_xmtbuf - 1);
		len = aeput(es->es_buf[curr], m);
		if(len < 64){
			len = 64;
		}

		/* Disable(); */	/* Dale */
		es->es_tdra[curr].bcnt = -len;
		es->es_tdra[curr].flags = TDRA_OWN | TDRA_STP | TDRA_ENP;
		*es->es_csr = AE_TDMD | AE_INEA;
		es->es_if.if_opackets++;
		/* Enable(); */		/* Dale */
	}
	Permit();	/* splx(s); */
	return;
}

/*
 * Routine to copy from mbuf chain to transmit buffer in enet memory.
 */
static int
aeput(bp, m)
	u_char *bp;
	struct mbuf *m;
{
	register struct mbuf *mp;
	register int totlen;
/*	register int off;	*/	/* Martin */

	totlen = 0;
	for (mp = m; mp; mp = mp->m_next) {
		register unsigned len = mp->m_len;
		u_char *mcp;

		totlen += len;
		if (len == 0){
			continue;
		}
		mcp = mtod(mp, u_char *);
/*** changed by Dale and Martin 2 Apr 90 ***/
		bcopy(mcp,bp,len);
		bp += len;
		mcp += len;

/*** original Ameristar code
		if ((unsigned)bp & 01) {
			*bp++ = *mcp++;
			len--;
		}

		if (off = (len >> 1)) {
			register u_short *to, *from;

			to = (u_short *)bp;
			from = (u_short *)mcp;
			do {
				*to++ = *from++;
			} while (--off > 0);
			bp = (u_char *)to,
			mcp = (u_char *)from;
		}
		if (len & 01){
			*bp++ = *mcp++;
		}
*****************************/
	}
	m_freem(m);

	return totlen;
}

/*
 * Process an ioctl request.
 */
static int
aeioctl(ifp, cmd, data)
	register struct ifnet *ifp;
	int cmd;
	caddr_t data;
{
	register struct ifaddr *ifa = (struct ifaddr *)data;
	struct ae_softc *es = &ae_softc[ifp->if_unit];
	int error;

	error = 0;
	Forbid();	/* s = splimp(); */
	switch (cmd) {
	case SIOCSIFADDR:
		ifp->if_flags |= IFF_UP;

		switch (ifa->ifa_addr.sa_family) {
#ifdef INET
		case AF_INET:
			aeinit(ifp->if_unit);
			((struct arpcom *)ifp)->ac_ipaddr =
						IA_SIN(ifa)->sin_addr;
			arpwhohas((struct arpcom *)ifp, &IA_SIN(ifa)->sin_addr);
			break;
#endif
		default:
			aeinit(ifp->if_unit);
			break;
		}
		break;

	case SIOCSIFFLAGS:
		if((ifp->if_flags & IFF_UP)==0 && ifp->if_flags & IFF_RUNNING){
			*es->es_csr = AE_STOP;
			ifp->if_flags &= ~IFF_RUNNING;
		} else if(ifp->if_flags&IFF_UP && (ifp->if_flags&IFF_RUNNING)==0){
			aeinit(ifp->if_unit);
		}
		break;

	default:
		error = EINVAL;
	}

	Permit();	/* splx(s); */
	return (error);
}

/*
 * Configuration system for Amiga.
*/

static u_char am_enet_mfg[3] = {0x00, 0x00, 0x9f};
static u_char co_enet_mfg[3] = {0x00, 0x80, 0x10};
char *explib = "expansion.library";
struct Library *ExpansionBase;

struct boardtype {
	char	*name;
	long	lance_offset;
	long	ram_offset;
	long	ram_size;
	short	prod_num;
	short	mfg_num;
	u_char	*enet_num;
} ae_types[] = {
	{"AE-2000", 0x4000, 0x8000, 32768, AMER_ENET1, AMERISTAR, am_enet_mfg},
	{"CBM-1",   0x4000, 0x8000, 32768, CBM_ENET1,  COMMODORE, co_enet_mfg},
	{"AEM-500", 0x20000,0x60000,32768, AMER_AEM500,AMERISTAR, am_enet_mfg},
	0
};

static int
attach(es)
	register struct ae_softc *es;
{
	register int size, total_bufs, i;
	register struct ifnet *ifp;

	bzero(es->es_ram, es->es_memsize);
	es->es_xmtbuf = es->es_rcvbuf = 4; size = 0;
	do {
		es->es_rcvbuf <<= 1;
		size = TOTSIZE(es->es_xmtbuf, es->es_rcvbuf);
		total_bufs = es->es_xmtbuf + es->es_rcvbuf;
	} while((size <= es->es_memsize) && (total_bufs <= MAXBUF));
	es->es_rcvbuf >>= 1;

	es->es_iadr = (struct ae_iadr *)es->es_ram;
	es->es_tdra = (struct ae_tdra *)(es->es_iadr + 1);
	es->es_rdra = (struct ae_rdra *)(es->es_tdra + es->es_xmtbuf);
	es->es_buf[0] = (u_char *)(es->es_rdra + es->es_rcvbuf);
	for(i = 1; i < (es->es_xmtbuf + es->es_rcvbuf); i++){
		es->es_buf[i] = es->es_buf[0] + i*ENETBUFSIZE;
	}

	ifp = &es->es_if;
	ifp->if_unit = es - ae_softc;
	ifp->if_mtu = ETHERMTU;
	ifp->if_flags = IFF_BROADCAST;
	ifp->if_init = aeinit;
	ifp->if_ioctl = aeioctl;
	ifp->if_output = aeoutput;

	ifp->if_name = "ae";	/* setting name actually allocates device */
	if_attach(ifp);

	return 0;
}

/*
 * Attach any ethernet interface cards we find.
 */
int aeattach()
{
	register struct ae_softc *es;
	register struct ConfigDev *cd;
	register int unit, s, i;

	ExpansionBase = OpenLibrary(explib, 0L);
	if(!ExpansionBase){
		return -1;
	}

	s = splimp();
	cd = 0;
	i = 0;
	unit = 0;
	while(1){
		while(ae_types[i].name){
			cd = FindConfigDev(cd, ae_types[i].mfg_num, ae_types[i].prod_num);
			if(cd){
				break;
			}
			cd = 0;	i++; 	/* look for next board type */
		}
		if(!cd || !ae_types[i].name){
			break;
		}

		es = &ae_softc[unit++];
		bzero((caddr_t)es, sizeof(*es));
		es->es_brdaddr = (u_char *)cd->cd_BoardAddr;
		es->es_csr = (u_short *)(es->es_brdaddr + ae_types[i].lance_offset);
		es->es_ram = es->es_brdaddr + ae_types[i].ram_offset;
		es->es_memsize = ae_types[i].ram_size;
		es->es_snum = cd->cd_Rom.er_SerialNumber;
		bcopy(ae_types[i].enet_num, es->es_addr, 3);
		es->es_addr[3] = (es->es_snum >> 16) & 0xff;
		es->es_addr[4] = (es->es_snum >> 8) & 0xff;
		es->es_addr[5] = es->es_snum & 0xff;
		attach(es);
	}
        units_configured = unit;
	splx(s);

	CloseLibrary(ExpansionBase);
	ExpansionBase = NULL;
	return(0);

}

/***********
#ifdef DEBUG
void m_dump(m)
	register struct mbuf *m;
{
	for(; m; m = m->m_next){
		register char *p = mtod(m, char *);
		register int i;

		printf("m=%x, mbuf (%d bytes)\n", m, m->m_len);
		for(i = 0; i < (m->m_len)>>2; i++){
			printf((i&7) == 7 ? "%08x\n":"%08x ", *(u_long *)p);
			p += sizeof(long);
		}
		printf("\n");
	}
}
#endif
***********/