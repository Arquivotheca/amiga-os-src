/*
 * Driver for Commodore Arcnet board
 */

#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/errno.h>

#include <net/if.h>
#include <net/netisr.h>
#include <net/route.h>

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/libraries.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <exec/interrupts.h>
#include <devices/serial.h>
#include <hardware/intbits.h>
#include <libraries/configvars.h>
#include <cbm.h>
#include <ameristar.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/inet.h>
#include <sys/protosw.h>

#ifdef LATTICE
	#include <proto/exec.h>
	#include <proto/expansion.h>
#endif

#include "proto.h"

#define AASR_RI		(1<<7)		/* Receiver inhibited		*/
#define AASR_ETS2	(1<<6)		/* Extended timeout status 2	*/
#define AASR_ETS1	(1<<5)		/* Extended timeout status 1	*/
#define AASR_POR	(1<<4)		/* Power on reset		*/
#define AASR_TEST	(1<<3)		/* Test/diag bit.  mostly zero	*/
#define AASR_RECON	(1<<2)		/* Reconfigure occured		*/
#define AASR_TMA	(1<<1)		/* transmit ack			*/
#define AASR_TA		(1<<0)		/* transmit buffer available	*/

#define AAWIM_RI	AASR_RI		/* interrupt when rcv inhibit	*/
#define AAWIM_RECON	AASR_RECON	/* interrupt when reconfig	*/
#define AAWIM_TA	AASR_TA		/* interrupt when xmit buf avl	*/

#define AACMD_DT	0x01		/* disable transmitter		*/
#define AACMD_DR	0x10		/* disable receiver		*/
#define AACMD_TRP(n)	(0x03+((n)<<3))	/* transmit page n		*/
#define AACMD_RCV(n)	(0x84+((n)<<3))	/* receive into page n		*/
#define AACMD_CONFIG	0x0d		/* receive short+long packets	*/
#define AACMD_CLEAR	0x1e		/* clear POR + RECON flags	*/
#define AACMD_CLR_POR	0x0e		/* clear POR flag		*/
#define AACMD_CLR_RECON	0x16		/* clear RECON bit		*/

#define NBUF		4		/* board has 4 buffers		*/

#define ARCTYPE_IP	0xf0		/* IP packet			*/
#define ARCTYPE_ARP	0xf1		/* ARP packet			*/
#define ARC_BROADCAST	0		/* Arcnet broadcast address	*/

#define MAXHDR		5		/* max hdr len			*/

#define IS_FREE		0		/* buffer is free		*/
#define	HAS_RCV_DATA	1		/* buffer has rcvr data ready	*/
#define IS_CURR_RCVR	2		/* buffer is current rcvr	*/
#define IS_CURR_XMIT	3		/* buffer is current xmtr	*/
#define HAS_XMIT_DATA	4		/* buffer has xmit data ready	*/
#define IS_BUSY		5		/* buffer allocated for now	*/

static struct Interrupt *aa_in;	/* Interrupt stub for arcnet boards	*/

struct aa_softc {
	struct ifnet aa_if;	/* network-visible interface 		*/
	u_char 	aa_inuse;	/* interface is allocated		*/
	u_char	*aa_base;	/* base address of board		*/
	u_char	*aa_sr;		/* pointer to status register		*/
	u_char	*aa_cmd;	/* pointer to command register		*/
	u_char	aa_myaddr;	/* network address of this board	*/
	u_char	aa_wim;		/* copy of interrupt mask		*/
	u_char	*aa_buf[NBUF]; 	/* buffer pointers, only 4 supported	*/
	u_char	aa_state[NBUF];	/* state of each buffer			*/
#define NAA 2
} aa_softc[NAA];

extern struct Library *ExpansionBase;	/* from if_ae.c		*/
extern char *explib;			/* from if_ae.c		*/
extern struct ifnet loif;

int aaattach(void);
void aapoll(void);

static int aaoutput(struct ifnet *ifp, register struct mbuf *m, struct sockaddr *dst);
static int aaoutfromq(struct aa_softc *aa);
static struct mbuf *aa_btom(struct ifnet *ifp, register u_char *cp, register int totlen);
static void aaread(register struct aa_softc *aa, int n);
static init_board(register struct ifnet *ifp);
static int aaioctl(register struct ifnet *ifp, int cmd, caddr_t data);
static void make_rcvr(register struct aa_softc *aa);
static shutdown_board(register struct ifnet *ifp);
	
/* arccopy.asm */
void __stdargs arccopyout(void *src, void *dst, long cnt);
void __stdargs arccopyin (void *src, void *dst, long cnt);


/*
 * Called from boot code to establish aa interfaces.  Initialize protocol
 * stack visible structures, find boards.
 */
static struct {
	int	mfg, prod;
#define NUMTYPE 2
} dev[NUMTYPE] = {
	{COMMODORE, CBM_ARCNET}, 
	{AMERISTAR, AMER_ARCNET1}
};

int aaattach(void)
{
	register struct ConfigDev *cd;
	register struct aa_softc *aa;
	int i, type;

	if (aa_softc[0].aa_if.if_name == NULL) {
		ExpansionBase = OpenLibrary(explib, 0L);
		if(!ExpansionBase){
			return -1;
		}

		cd = 0;	type = 0;
		for (aa = aa_softc; aa < (aa_softc + NAA); aa++) {
			do {
				cd = FindConfigDev(cd, dev[type].mfg, dev[type].prod);
				if(cd == 0 && ++type >= NUMTYPE){
					break;
				}
			} while(cd == 0);

			if(cd == 0){
				break;
			}
			aa->aa_base = (u_char *)cd->cd_BoardAddr;
			aa->aa_sr = aa->aa_base + 0x4000;
			aa->aa_cmd = aa->aa_sr + 2;
			for(i = 0; i < NBUF; i++){
				aa->aa_buf[i] = aa->aa_base + 0x8000 + i*1024;
				bzero(aa->aa_buf[i], 2*512);
			}

			aa->aa_if.if_name = "aa";
			aa->aa_if.if_unit = aa - aa_softc;
			aa->aa_if.if_mtu = 512 - MAXHDR;
			aa->aa_if.if_flags = IFF_BROADCAST;
			aa->aa_if.if_ioctl = aaioctl;
			aa->aa_if.if_output = aaoutput;
			if_attach(&aa->aa_if);
		}

		CloseLibrary(ExpansionBase);
		ExpansionBase = 0;
	}

	return 0;
}

/*
 * aaoutput is called from the protocol stack to queue packets for
 * output on the interface.  The address we send to is the LSB of the
 * Internet address - done so that ARP is not required for Arcnet.
 */
#define UPF (IFF_RUNNING | IFF_UP)

static int
aaoutput(ifp, m, dst)
	struct ifnet *ifp;
	register struct mbuf *m;
	struct sockaddr *dst;
{
	register struct ifqueue *ifq;
	struct mbuf *m2, *mcopy;
	struct in_addr dest;
	register u_char *cp;
	struct aa_softc *aa;
	int s;

	if (dst->sa_family != AF_INET) {
		m_freem(m);
		return EAFNOSUPPORT;
	}

	aa = aa_softc + ifp->if_unit;
	if(aa->aa_myaddr == 0 || (ifp->if_flags & UPF) != UPF){
		m_freem(m);
		return ENETDOWN;
	}

	if(m->m_off < (MMINOFF + 3)){
		MGET(m2, M_DONTWAIT, MT_DATA);
		if(m2 == 0){
			m_freem(m);
			return ENOMEM;
		}
		m2->m_next = m;
		m = m2;
	}

	/*
	 * make room in first mbuf for SID/DID
	 */
	m->m_off -= 3;
	m->m_len += 3;

	cp = mtod(m, u_char *);
	dest = ((struct sockaddr_in *)dst)->sin_addr;
	*cp++ = aa->aa_myaddr;			/* SID */
	if(in_broadcast(dest)){			/* DID */
		*cp++ = ARC_BROADCAST;
		mcopy = m_copy(m, 0, (int)M_COPYALL);
		if(mcopy){	/* if broadcast, use loopback to hit us */
			looutput(&loif, mcopy, dst);
		}
	} else {
		*cp++ = ntohl(dest.s_addr) & 0xff;
	}
	*cp++ = ARCTYPE_IP;

	/*
	 * Queue packet for output and kick transmitter to get it going
	 */
	s = splimp();
	ifq = &ifp->if_snd;
	if (IF_QFULL(ifq)) {
		IF_DROP(ifq);
		m_freem(m);
		splx(s);
		aa->aa_if.if_oerrors++;
		return (ENOBUFS);
	}
	IF_ENQUEUE(ifq, m);
	aaoutfromq(aa);
	splx(s);

	return 0;
}

/*
 * output a packet from the send queue.  Return false if none sent.
 */
static int
aaoutfromq(aa)
	struct aa_softc *aa;
{
	register u_char *dp, *cp;
	register int n, i, cnt;
	struct mbuf *m2, *m;
	u_char *ep;
	int off;

	/*
	 * Run through buffer state bits to find free buffer and to ascertain
	 * how many buffers are currently free: cnt = num free, n = last free.
	 * If at least half the buffers are free then mark the free buf
	 * we found as busy, else set n to reflect that buffer really isn't
	 * available to the transmitter.  We want to preserve two free
	 * buffers for receiving to achieve a double buffering effect.
	 */
	Disable();
	n = -1; cnt = 0;
	for(i = 0; i < NBUF; i++){
		if(aa->aa_state[i] == IS_FREE){
			n = i;
			cnt++;
		}
	}
	if(cnt > 2){
		aa->aa_state[n] = IS_BUSY;
	} else {
		n = -1;
	}
	Enable();

	/*
	 * If n == -1, then we're out of buffers to use for xmit purposes
	 */
	if(n == -1){
		return 0;
	}

	/*
	 * Dequeue packet from i/f
	 */
	i = splimp();
	IF_DEQUEUE(&aa->aa_if.if_snd, m);
	splx(i);
	if (m == NULL){
		aa->aa_state[n] = IS_FREE;
		return 0;
	}

	/*
	 * get packet len
	 */
	for(cnt = 0, m2 = m; m2; m2 = m2->m_next){
		cnt += m2->m_len;
	}
	cnt -= 2;	/* account for DID/SID but not TYPE */

	/*
	 * Build arcnet physical header, ie SID/DID/CNT.
	 */
	dp = aa->aa_buf[n];
	cp = mtod(m, u_char *);
	dp[0] = cp[0];	/* SID */
	dp[2] = cp[1];	/* DID */
	if(cnt >= 253){
		if(cnt >= 253 && cnt <= 256){
			cnt = 257;
		}
		dp[4] = 0;
		off = 512 - cnt;
		dp[6] = off;
		ep = dp + 512*2;
		dp += off*2;
	} else {
		off = 256 - cnt;
		dp[4] = off;
		ep = dp + 256*2;
		dp += off*2;
	}

	/*
	 * Trim SID/DID out of packet as they've been copied already
	 */
	m->m_len -= 2;
	m->m_off += 2;

	/*
	 * copy packet onto board
	 */
	while(m){
		cnt = m->m_len;
		if((dp + cnt) < ep){
			arccopyout(mtod(m, u_char *), dp, cnt);
			dp += (cnt << 1);
		}

		MFREE(m, m2);
		m = m2;
	}

	/*
	 * Now start transmitter if necessary.  If there is a buffer marked as
	 * current, then the interrupt routine will transmit the buffer later.
	 */
	Disable();
	for(i = 0; i < NBUF; i++){
		if(aa->aa_state[i] == IS_CURR_XMIT){
			break;
		}
	}

	if(i == NBUF){
		aa->aa_state[n] = IS_CURR_XMIT;
		aa->aa_wim |= AAWIM_TA;
		*aa->aa_cmd = AACMD_TRP(n);
		*aa->aa_sr = aa->aa_wim;
	} else {
		aa->aa_state[n] = HAS_XMIT_DATA;
	}
	Enable();

	return 1;
}

/*
 * Copy data buffer to mbuf chain; add ifnet pointer.
 */
static struct mbuf *
aa_btom(ifp, cp, totlen)
	struct ifnet *ifp;
	register u_char *cp;
	register int totlen;
{
	struct mbuf *top = 0, **mp = &top;
	register struct mbuf *m;
	register int len;

	while (totlen > 0) {
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

		arccopyin(cp, mcp, len);
		cp += (len << 1);

		*mp = m;
		mp = &m->m_next;
		totlen -= len;
	}

	return top;
}

/*
 * read packet from interface:
 * deal with different size packets, complicated by memory map of
 * Amiga Arcnet board, ie bytes on word boundaries.
 * short fmt:  	<SID> <DID> <CNT> <TYPE> ... <DATA>
 * long fmt:	<SID> <DID> <0> <CNT> <TYPE> ... <DATA>
 */
static void
aaread(aa, n)
	register struct aa_softc *aa;
	int n;
{
	register int s, len, count, type;
	register u_char *cp;
	struct mbuf *m;

	cp = aa->aa_buf[n];
	if(cp[4] != 0){
		count = cp[4];
		len = 256 - count;
	} else {
		count = cp[6];
		len = 512 - count;
	}
	type = cp[count*2];

	if(type != ARCTYPE_IP){
		aa->aa_if.if_ierrors++;
		return;
	}

	m = aa_btom(&aa->aa_if, cp + ((count + 1)<<1), --len);
	if (m == NULL){
		aa->aa_if.if_ierrors++;
		return;
	}

	s = splimp();
	if (IF_QFULL(&ipintrq)) {
		IF_DROP(&ipintrq);
		aa->aa_if.if_ierrors++;
		m_freem(m);
	} else {
		IF_ENQUEUE(&ipintrq, m);
		schednetisr(NETISR_IP);
	}
	splx(s);
}

static shutdown_board(ifp)
	register struct ifnet *ifp;
{
	register struct aa_softc *aa;
	int s;

	aa = aa_softc + ifp->if_unit;
	if(ifp->if_unit < 0 || ifp->if_unit >= NAA || !aa->aa_base){
		return ENXIO;
	}

	s = splimp();

	if_down(&aa->aa_if);
	ifp->if_flags &= ~IFF_RUNNING;

	Disable();
	aa->aa_wim = 0;
	*aa->aa_sr = aa->aa_wim;
	*aa->aa_cmd = AACMD_DR;
	*aa->aa_cmd = AACMD_DT;
	*aa->aa_base = 0;		/* put 9026 in POR state */
	aa->aa_inuse = 0;
	aa->aa_myaddr = 0;
	Enable();

	for(aa = aa_softc; aa < (aa_softc + NAA); aa++){
		if(aa->aa_inuse){
			break;
		}
	}

	if(aa_in && (aa - aa_softc) == NAA){	/* free interrupt server */
		RemIntServer(INTB_PORTS, aa_in);
		aa_in->is_Code = (void (*)())-1;
		aa_in->is_Node.ln_Type = -1;
		aa_in->is_Node.ln_Pri = -129;
		FreeMem(aa_in, sizeof(*aa_in));
		aa_in = 0;
	}
	splx(s);

	return 0;
}

static init_board(ifp)
	register struct ifnet *ifp;
{
	register struct aa_softc *aa;
	extern void *AllocMem();
	extern void aa_intr();

	aa = aa_softc + ifp->if_unit;
	if(ifp->if_unit < 0 || ifp->if_unit >= NAA || !aa->aa_base){
		return ENXIO;
	}

	if(aa_in == 0){
		aa_in = (struct Interrupt *)AllocMem(sizeof(*aa_in),MEMF_PUBLIC);
		if(aa_in == 0){
			return ENOMEM;
		}

		aa_in->is_Code = aa_intr;
		aa_in->is_Data = (APTR)InetBase;
		aa_in->is_Node.ln_Type = NT_INTERRUPT;
		aa_in->is_Node.ln_Pri = 100;
		aa_in->is_Node.ln_Name = "aa_intr()";
		AddIntServer(INTB_PORTS, aa_in);
	}

	aa->aa_if.if_flags |= IFF_RUNNING;
	aa->aa_inuse = 1;
	aa->aa_myaddr = 0;

	/*
	 * The next insn takes the 9026 out of its reset state.  This should
	 * generate an immediate POR interrupt.
	 */
	*aa->aa_base = 0xff;

	return 0;
}

/*
 * Process an ioctl request.
 */
static int
aaioctl(ifp, cmd, data)
	register struct ifnet *ifp;
	int cmd;
	caddr_t data;
{
	register struct ifaddr *ifa = (struct ifaddr *)data;
	int s = splimp(), error = 0;

	switch (cmd) {
	case SIOCSIFADDR:
	case SIOCSIFDSTADDR:
		if (ifa->ifa_addr.sa_family != AF_INET){
			error = EAFNOSUPPORT;
		} else {
			if(!(ifp->if_flags & IFF_RUNNING)){
				error = init_board(ifp);
			}
			if(!error){
				ifp->if_flags |= IFF_UP;
			}
		}
		break;

	case SIOCSIFFLAGS:
		if((ifp->if_flags & (IFF_UP|IFF_RUNNING)) != (IFF_UP|IFF_RUNNING)){
			shutdown_board(ifp);
		}
		break;

	default:
		error = EINVAL;
	}
	splx(s);

	return error;
}

/*
 * setup current receiver buffer, if none was found
 */
static void
make_rcvr(aa)
	register struct aa_softc *aa;
{
	register int curr, free, i;

	/*
	 * Run through buffers looking for current rcvr buffer,
	 * and also find free buffer to replace one with data
	 * in it.
	 */
	curr = free = -1;
	for(i = 0; i < NBUF; i++){
		switch(aa->aa_state[i]){
		case IS_CURR_RCVR:
			curr = i;
			break;

		case IS_FREE:
			free = i;
			break;
		}
	}

	if(curr >= 0){	/* rcvr buff already set */
		return ;
	}

	/*
	 * If found free buffer, notify 9026 it is current
 	 * rcvr buffer, set int mask for rcvr ints, and mark
	 * buf as current rcvr.
	 */
	if(free >= 0){
		*aa->aa_cmd = AACMD_RCV(free);
		aa->aa_wim |= AAWIM_RI;
		aa->aa_state[free] = IS_CURR_RCVR;
	}
}

/*
 * aa_intrC is called from interrupt level.  Its mission is just to clear
 * the interrupting condition and then wakeup the polling task.  Higher
 * level is called if a packet is received, or a buffer freed.
 */
void __saveds __stdargs aaintrC(void)
{
	register struct aa_softc *aa;
	register u_char *state;
	register int i;
	int poll;

	poll = 0;
	for(aa = aa_softc; aa < (aa_softc + NAA); aa++){
		if(!aa->aa_inuse){
			continue;
		}

		/*
		 * The only way this board can be interrupting is by setting
		 * one or more of RI, TA, POR, RECON.  If none set, then move
		 * on to checking next board.
		 */
		if(!(*aa->aa_sr & (AASR_RI | AASR_TA | AASR_POR | AASR_RECON))){
			continue;
		} 
		state = aa->aa_state;

		/*
		 * If POR int, then clear POR flag & get address from packet ram
		 */
		if(*aa->aa_sr & AASR_POR){
			aa->aa_myaddr = 0;
			if(aa->aa_buf[0][0] == 0xd1){
				aa->aa_myaddr = aa->aa_buf[0][2];
			}
			*aa->aa_cmd = AACMD_CLR_POR;
			aa->aa_wim |= AAWIM_RECON;
		}

		/*
		 * Note that a network where only one board is configured
		 * will continuously RECON once every 780 mS.  It is possible
		 * to handle this differently, eg using a polled system, but
		 * it probably isn't worth giving up the user level feedback
		 * of incrementing if_collisions during recons.
		 *
		 * Note that the 9026 is apparently sensitive to the order
		 * in which AACMD_CLR_RECON and AACMD_CONFIG are issued.  The
		 * way that seems to work is to give CONFIG first, then
		 * CLR_RECON.
		 */
		if(*aa->aa_sr & AASR_RECON){
			aa->aa_wim |= AAWIM_RECON;
			aa->aa_if.if_collisions++;
			*aa->aa_cmd = AACMD_CONFIG;
			*aa->aa_cmd = AACMD_CLR_RECON;
		}

		/*
		 * Check & handle Receive Inhibit (RI) conditions
		 */
		if(*aa->aa_sr & AASR_RI){
			aa->aa_wim &= ~AAWIM_RI; 
			for(i = 0; i < NBUF; i++){
				if(state[i] == IS_CURR_RCVR){
					state[i] = HAS_RCV_DATA;
				}
			}
			make_rcvr(aa);
			poll++;
		}

		/*
		 * Check for transmit done
		 */
		if(*aa->aa_sr & AASR_TA){
			aa->aa_wim &= ~AAWIM_TA;
			for(i = 0; i < NBUF; i++){
				if(state[i] == IS_CURR_XMIT){
					state[i] = IS_FREE;
					aa->aa_if.if_opackets++;
					poll++;
				}
				if(state[i] == HAS_XMIT_DATA){
					state[i] = IS_CURR_XMIT;
					*aa->aa_cmd = AACMD_TRP(i);
					aa->aa_wim |= AAWIM_TA;
					break;
				}
			}
		}

		/*
		 * update interrupt mask
		 */
		*aa->aa_sr = aa->aa_wim;
	}

	if(poll){
		setsoftifaa();
	}
}

/*
 * aapoll is called from inet task to handle signals from the arcnet
 * driver.  This code runs at splimp() from inside inet process,
 * so almost anything goes...
 */
void aapoll(void)
{
	register struct aa_softc *aa;
	register int i, free;

	for (aa = aa_softc; aa < (aa_softc + NAA); aa++) {
		if(!aa->aa_inuse){
			continue;
		}

		for(free = i = 0; i < NBUF; i++){
			switch(aa->aa_state[i]){
			case HAS_RCV_DATA:
				aaread(aa, i);
				aa->aa_if.if_ipackets++;
				aa->aa_state[i] = IS_FREE;
			case IS_FREE:
				free++;
				break;
			
			case HAS_XMIT_DATA:
			case IS_CURR_XMIT:
			case IS_CURR_RCVR:
			case IS_BUSY:
			default:
				/* NOP */
				break;
			}
		}

		if(free > 0){
			Disable();
			make_rcvr(aa);
			*aa->aa_sr = aa->aa_wim;
			Enable();
			while(aaoutfromq(aa)) {
			}
		}
	}
}

