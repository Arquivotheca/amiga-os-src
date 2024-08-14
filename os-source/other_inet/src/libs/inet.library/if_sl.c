/*
 * Copyright (c) 1989 Regents of the University of California.
 * All rights reserved.
 *
 * Derived from: compression slip driver on Unix,
 *	Van Jacobson (van@helios.ee.lbl.gov), Dec 31, 1989:
 *	- Initial distribution.
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
#include <exec/io.h>
#include <exec/libraries.h>
#include <exec/nodes.h>
#include <exec/memory.h>
#include <devices/serial.h>

#ifdef LATTICE
	#include <proto/exec.h>
	#include <proto/expansion.h>
#endif

#ifdef INET
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <sys/protosw.h>
#endif

#include <netinet/slip.h>
#include "slcompress.h"
#include "proto.h"

#define NSL 		8	/* number of SLIP interfaces		*/

#define	SLMTU		576	/* BETA 2.1 */
#define BUFOFFSET 	(128 + sizeof(struct ifnet **))
#define SLMAX		1024 	/* max input buffer			*/

extern struct Task *netpid;
extern int slipbit;

#define OBUFSIZE	2*SLMTU
#define KBUG(x)	/*kprintf x*/
/*#define SLDEBUG*/
void kprintf(char *,...);
#include "if_slvar.h"
struct sl_softc sl_softc[NSL];

static struct MsgPort *slip_mp, *slip_w_mp;

/*
 * The following disgusting hack gets around the problem that IP TOS
 * can't be set in BSD/Sun OS yet.  We want to put "interactive"
 * traffic on a high priority queue.  To decide if traffic is
 * interactive, we check that a) it is TCP and b) one of it's ports
 * if telnet, rlogin or ftp control.
 */
static u_short interactive_ports[8] = {
	0,	513,	0,	0,
	0,	21,	0,	23,
};

#define INTERACTIVE(p) (interactive_ports[(p) & 7] == (p))

#define FRAME_END		0xc0		/* Frame End */
#define FRAME_ESCAPE		0xdb		/* Frame Esc */
#define TRANS_FRAME_END		0xdc		/* transposed frame end */
#define TRANS_FRAME_ESCAPE	0xdd		/* transposed frame esc */

/* functions */
void slattach(void);
static int slsetdevice(register struct ifnet *ifp,struct slipvars *sl);
static int slclose(register struct ifnet *ifp);
static void sloutfromq(struct sl_softc *sc);
void slpoll(void);
static int sloutput(register struct ifnet *ifp, register struct mbuf *m, struct sockaddr *dst);
static struct mbuf *sl_btom(struct sl_softc *sc, register int totlen);
static void frame_end(register struct sl_softc *sc, register int len);
static void slinput(register struct sl_softc *sc,register u_char *cp,register int len);
static int slioctl(register struct ifnet *ifp,int cmd,caddr_t data);
static void slread(struct sl_softc *sc);

/* functions in slcompress.c */
void sl_compress_init(struct slcompress *comp);
u_char sl_compress_tcp(struct mbuf *m,register struct ip *ip,struct slcompress *comp,int compress_cid);
int sl_uncompress_tcp(u_char **bufp,int len,u_int type,struct slcompress *comp);

/*
 * Called from boot code to establish sl interfaces.
 */
void slattach(void)
{
	register struct sl_softc *sc;
	register int i;

	KBUG(("slattach\n"));

	if (sl_softc[0].sc_if.if_name == NULL) {
		for (i = 0, sc = sl_softc; i < NSL; sc++) {
			bzero(sc, sizeof(struct sl_softc));
			sc->sc_if.if_name = "sl";
			sc->sc_if.if_unit = i++;
			sc->sc_if.if_mtu = SLMTU;
			sc->sc_if.if_flags = IFF_POINTOPOINT;
			sc->sc_if.if_ioctl = slioctl;
			sc->sc_if.if_output = sloutput;
			sc->sc_if.if_snd.ifq_maxlen = 50;
			sc->sc_fastq.ifq_maxlen = 32;
			if_attach(&sc->sc_if);
		}
	}
}

/*
 * setup serial variables for this SLIP
 */
static int
slsetdevice(ifp, sl)
	register struct ifnet *ifp;
	struct slipvars *sl;
{
	struct sl_softc *sc = &sl_softc[ifp->if_unit];

	KBUG(("slsetdevice\n"));

	/*
	 * if resources already allocated, remove them so that we can
	 * reset things.
	 */
	if(slip_mp || slip_w_mp || sc->sc_w || sc->sc_r){
		slclose(ifp);
	}

	sc->sc_ibuf = 0;
	sc->sc_obuf = 0;
	slip_mp = CreatePort(0L, 0L);
	slip_w_mp = CreatePort(0L, 0L);
	sc->sc_comp = AllocMem(sizeof(struct slcompress),MEMF_PUBLIC);
	sc->sc_r = (struct IOExtSer *)CreateExtIO(slip_mp, sizeof(struct IOExtSer));
	sc->sc_w = (struct IOExtSer *)CreateExtIO(slip_w_mp, sizeof(struct IOExtSer));
	sc->sc_ibuf = AllocMem(SLMAX,MEMF_PUBLIC);
	sc->sc_obuf = AllocMem(OBUFSIZE,MEMF_PUBLIC);
	if(!sc->sc_comp
		|| !sc->sc_w || !sc->sc_r || !slip_mp || !slip_w_mp || !sc->sc_ibuf || !sc->sc_obuf){
		slclose(ifp);
		return ENOMEM;
	}

	slip_mp->mp_SigBit = slipbit;
	slip_mp->mp_SigTask = netpid;
	slip_w_mp->mp_SigBit = slipbit;
	slip_w_mp->mp_SigTask = netpid;
	sc->sc_r->io_SerFlags = SERF_SHARED | SERF_RAD_BOOGIE | SERF_XDISABLED;

	if(OpenDevice(sl->sl_devname, sl->sl_unit, (struct IORequest *)sc->sc_r, sl->sl_flags)){
		slclose(ifp);
		return ENXIO;
	}

	sc->sc_r->IOSer.io_Command = SDCMD_SETPARAMS;
	sc->sc_r->io_RBufLen = 2*SLMAX;
	sc->sc_r->io_ExtFlags = 0;
	sc->sc_r->io_Baud = sl->sl_baud;
	sc->sc_r->io_ReadLen = sc->sc_r->io_WriteLen = 8;
	sc->sc_r->io_StopBits = 1;
	sc->sc_r->io_SerFlags = SERF_RAD_BOOGIE | SERF_XDISABLED;
	if(DoIO((struct IORequest *)sc->sc_r)){
		slclose(ifp);
		return EINVAL;
	}

	*sc->sc_w = *sc->sc_r;
	sc->sc_w->IOSer.io_Message.mn_ReplyPort = slip_w_mp;
	sc->sc_inuse = 1;
	sc->sc_mp = sc->sc_buf = sc->sc_ibuf + BUFOFFSET;
	sc->sc_ep = sc->sc_ibuf + SLMAX - 1;
	sc->sc_escape = 0;
	/*sc->sc_if.if_flags |= IFF_LINK1;*/	/* allow compress */
	sl_compress_init(sc->sc_comp);
	/*
	 * Do I/O on sc_w to ensure CheckIO/WaitIO works ok
	 */
	sc->sc_w->IOSer.io_Command = SDCMD_QUERY;
	DoIO((struct IORequest *)sc->sc_w);

	sc->sc_r->IOSer.io_Command = CMD_READ;
	sc->sc_r->IOSer.io_Data = (APTR)&sc->sc_rchar;
	sc->sc_r->IOSer.io_Length = 1;
	SendIO((struct IORequest *)sc->sc_r);

	ifp->if_flags |= IFF_RUNNING;	/* this sez we've allocated resources */

	return 0;
}

/*
 * close a SLIP driver - this is designed to be safe to call at any point
 * in a failed initialization
 */
static int
slclose(ifp)
	register struct ifnet *ifp;
{
	register struct sl_softc *sc = &sl_softc[ifp->if_unit];
	int s;

	s = splimp();
	if_down(&sc->sc_if);
	ifp->if_flags &= ~IFF_RUNNING;

	if(sc->sc_w){
		AbortIO((struct IORequest *)sc->sc_w);
		WaitIO((struct IORequest *)sc->sc_w);
		DeleteExtIO((struct IORequest *)sc->sc_w);
		sc->sc_w = 0;
	}

	if(sc->sc_r){
		AbortIO((struct IORequest *)sc->sc_r);
		WaitIO((struct IORequest *)sc->sc_r);
		CloseDevice((struct IORequest *)sc->sc_r);
		DeleteExtIO((struct IORequest *)sc->sc_r);
		sc->sc_r = 0;
	}

	sc->sc_inuse = 0; sc->sc_ep = sc->sc_mp = sc->sc_buf = 0;

	if (sc->sc_ibuf)
	{
		FreeMem(sc->sc_ibuf,SLMAX);
		sc->sc_ibuf = 0;
	}

	if (sc->sc_obuf)
	{
		FreeMem(sc->sc_obuf,OBUFSIZE);
		sc->sc_obuf = 0;
	}
	if (sc->sc_comp)
	{
		FreeMem(sc->sc_comp,sizeof(struct slcompress));
		sc->sc_comp = 0;
	}

	/*
 	 * Check if all slip drivers are idle; if so, free resources
	 */
	for(sc = sl_softc; sc < (sl_softc + NSL); sc++){
		if(sc->sc_inuse){
			break;
		}
	}

	if(sc == (sl_softc + NSL)){
		/*
		 * Don't use deleport to do this since it will
		 * free the network slipbit.
		 */
		if(slip_mp){
			slip_mp->mp_Node.ln_Type = -1;
			slip_mp->mp_MsgList.lh_Head = (struct Node *)-1;
			FreeMem(slip_mp, sizeof(*slip_mp));
			slip_mp = 0;
		}
		if(slip_w_mp){
			slip_w_mp->mp_Node.ln_Type = -1;
			slip_w_mp->mp_MsgList.lh_Head = (struct Node *)-1;
			FreeMem(slip_w_mp, sizeof(*slip_w_mp));
			slip_w_mp = 0;
		}
	}
	splx(s);

	return 0;
}

/*
 * sloutput is called from the protocol stack to queue packets for
 * output on the interface.
 */
#define UPF (IFF_RUNNING | IFF_UP)

static int
sloutput(ifp, m, dst)
	register struct ifnet *ifp;
	register struct mbuf *m;
	struct sockaddr *dst;
{
	register struct sl_softc *sc;
	register struct ifqueue *ifq;
	int s;
	register struct ip *ip;

	KBUG(("sloutput\n"));

	if (dst->sa_family != AF_INET) {
		m_freem(m);
		return (EAFNOSUPPORT);
	}

	sc = &sl_softc[ifp->if_unit];
	if(!sc->sc_w || (ifp->if_flags & UPF) != UPF){
		m_freem(m);
		return ENETDOWN;
	}

	/* need to have a way to test if the CARRIER is on */
	/* the serial/modem line */
	/* if not return EHOSTUNREACH */

	ifq = &ifp->if_snd;

	if ((ip = mtod(m, struct ip *))->ip_p == IPPROTO_TCP) {
		register int p = ((int *)ip)[ip->ip_hl];

		if (INTERACTIVE(p & 0xffff) || INTERACTIVE(p >> 16)) {
			ifq = &sc->sc_fastq;
			p = 1;
		} else {
			p = 0;
		}

		if (sc->sc_if.if_flags & IFF_LINK0) {
			/*
			 * The last parameter turns off connection id
			 * compression for background traffic:  Since
			 * fastq traffic can jump ahead of the background
			 * traffic, we don't know what order packets will
			 * go on the line.
			 */
			p = sl_compress_tcp(m, ip, sc->sc_comp, p);
			*mtod(m, u_char *) |= p;
		}
	} else if ((sc->sc_if.if_flags & IFF_LINK2) &&
		 ip->ip_p == IPPROTO_ICMP) {
		m_freem(m);
		return (0);
	}

	s = splimp();
	if (IF_QFULL(ifq)) {
		IF_DROP(ifq);
		m_freem(m);
		splx(s);
		sc->sc_if.if_oerrors++;
		return (ENOBUFS);
	}

	IF_ENQUEUE(ifq, m);
	if(CheckIO((struct IORequest *)sc->sc_w)){
		sloutfromq(sc);
	}
	splx(s);

	return 0;
}

/*
 * output a packet from the send queue.  Called only whenever there are
 * no serial output in process.
 */
static void
sloutfromq(sc)
	struct sl_softc *sc;
{
	register u_char *dp, *cp;
	u_char *sp,*ep;
	struct mbuf *m2, *m;
	register int len;
	int s;
	int cfreecount = OBUFSIZE;

	/*
	 * Get a packet from output queue
	 */
    dp = sc->sc_obuf;
    /*end = dp + OBUFSIZE - 5;*/
	/* send as many packets at once as possible */
	/* leave space for 5 additional chars */
    while (cfreecount > SLMTU+5) {
	s = splimp();
	IF_DEQUEUE(&sc->sc_fastq, m);
	if (m == NULL){
		IF_DEQUEUE(&sc->sc_if.if_snd, m);
	}
	splx(s);

	if (m == NULL){
		break;
	}

#ifdef SLDEBUG
	if (cfreecount != OBUFSIZE)
	    kprintf("sloutfromq: sent=%ld\n",OBUFSIZE-cfreecount);
#endif

	sp = dp;	/* save pckt start ptr */

	/*
	 * The extra FRAME_END will start up a new packet, and thus
	 * will flush any accumulated garbage.  We do this whenever
	 * the line may have been idle for some time.
	 */
	if (dp == sc->sc_obuf) *dp++ = FRAME_END;

	while (m) {
		cp = mtod(m, u_char *);
		ep = cp + m->m_len;
		while (cp < ep) {
			switch(*dp++ = *cp++){
			case FRAME_ESCAPE:
				*dp++ = TRANS_FRAME_ESCAPE;
				break;
			case FRAME_END:
				dp[-1] = FRAME_ESCAPE;
				*dp++ = TRANS_FRAME_END;
				break;
			}
		}

		MFREE(m, m2);
		m = m2;
	}
	*dp++ = FRAME_END;

	sc->sc_if.if_opackets++;
	len = dp - sp;
	cfreecount -= len;
    }
	len = dp - sc->sc_obuf;
	if (len)
	{
	    if (len > OBUFSIZE)
	    {
		printf("sloutput PANIC: output buffer overflow, excess=%ld\n",
			len-OBUFSIZE);
	    }
	    sc->sc_bytessent += len;

	    sc->sc_w->IOSer.io_Command = CMD_WRITE;
	    sc->sc_w->IOSer.io_Data = (APTR)sc->sc_obuf;
	    sc->sc_w->IOSer.io_Length = len;
	    SendIO((struct IORequest *)sc->sc_w);
	}
}

/*
 * Copy data buffer to mbuf chain; add ifnet pointer.
 */
static struct mbuf *
sl_btom(sc, totlen)
	struct sl_softc *sc;
	register int totlen;
{
	struct mbuf *top = 0, **mp = &top;
	struct ifnet *ifp = &sc->sc_if;
	register struct mbuf *m;
	register int len;
	u_char *cp;

	cp = sc->sc_buf;
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
		bcopy(cp, mcp, len);
		cp += len;
		*mp = m;
		mp = &m->m_next;
		totlen -= len;
	}

	return top;
}

static void
frame_end(sc, len)
	register struct sl_softc *sc;
	register int len;
{
	struct mbuf *m;
	int s;
	register u_char c;

	KBUG(("frame_end len=%ld\n",len));

	if (len < 3){
		return;
	}

	if ((c = (*sc->sc_buf & 0xf0)) != (IPVERSION << 4)) {
		KBUG(("checking for compressed c=%lx flgs=%lx\n",c,sc->sc_if.if_flags));
		if (c & 0x80){
			c = TYPE_COMPRESSED_TCP;
		} else if (c == TYPE_UNCOMPRESSED_TCP){
			*sc->sc_buf &= 0x4f;
		}

		KBUG(("now c=%lx\n",c));

		if (sc->sc_if.if_flags & IFF_LINK0) {
			KBUG(("Calling sl_uncompress0\n"));
			len = sl_uncompress_tcp(&sc->sc_buf,len,c,sc->sc_comp);
			KBUG(("len=%ld\n",len));
			if (len <= 0)
				goto error;
		} else if ((sc->sc_if.if_flags & IFF_LINK1) &&
				   c == TYPE_UNCOMPRESSED_TCP && len >= 40) {
			KBUG(("Calling sl_uncompress1\n"));
			len = sl_uncompress_tcp(&sc->sc_buf,len,c,sc->sc_comp);
			KBUG(("len=%ld\n",len));
			if (len <= 0)
				goto error;
			sc->sc_if.if_flags |= IFF_LINK0;
		} else
			goto error;
	}
	m = sl_btom(sc, len);
	if (m == NULL)
		goto error;

	sc->sc_if.if_ipackets++;

	s = splimp();
	if (IF_QFULL(&ipintrq)) {
		IF_DROP(&ipintrq);
		sc->sc_if.if_ierrors++;
		m_freem(m);
	} else {
		IF_ENQUEUE(&ipintrq, m);
		schednetisr(NETISR_IP);
	}
	splx(s);
	return;

error:
	sc->sc_if.if_ierrors++;
	return;
}

/*
 * slinput - process a batch of characters that were rcvd from serial device
 */
static void
slinput(sc, cp, len)
	register u_char *cp;
	register struct sl_softc *sc;
	register int len;
{
	register int plen;
	register u_char *mp;
	register u_char esc;

	if (sc == 0)	return;

	sc->sc_bytesrcvd += len;
	mp = sc->sc_mp;
	esc = sc->sc_escape;

	KBUG(("slinput len=%ld\n",len));
	/* This code is not in the 2.1 beta distribution */
	if((mp + len) > sc->sc_ep){
		len = sc->sc_ep - mp;
		KBUG(("sender MTU too large\n"));
		sc->sc_if.if_collisions++; /* sender MTU too large for us */
	}

	while(len-- > 0){
		switch (*mp++ = *cp++) {
		default:
			if(esc){ /* lost data or out of sync */
				sc->sc_if.if_ierrors++;
				KBUG(("lost data\n"));
				len = 0;
				esc = 0;
			}
			break;
		case TRANS_FRAME_ESCAPE:
			if(esc){
				mp[-1] = FRAME_ESCAPE;
				esc = 0;
			}
			break;
		case TRANS_FRAME_END:
			if(esc){
				mp[-1] = FRAME_END;
				esc = 0;
			}
			break;
		case FRAME_ESCAPE:
			mp--;
			esc = 1;
			break;
		case FRAME_END:
			KBUG(("Frame end\n"));
			mp--;
			plen = mp - sc->sc_buf;
			if(plen > 0){
				frame_end(sc, plen);
			}
			mp = sc->sc_buf = sc->sc_ibuf + BUFOFFSET;
			esc = 0;
			break;
		}
	}

	sc->sc_mp = mp;
	sc->sc_escape = esc;
}

/*
 * Process an ioctl request.
 */
static int
slioctl(ifp, cmd, data)
	register struct ifnet *ifp;
	int cmd;
	caddr_t data;
{
	register struct ifaddr *ifa = (struct ifaddr *)data;
	int s = splimp(), error = 0;

	switch (cmd) {
	case SIOCSIFADDR: /* iff IFF_RUNNING is set, then mark the I/F up */
		if (ifa->ifa_addr.sa_family != AF_INET){
			error = EAFNOSUPPORT;
		} else if(ifp->if_flags & IFF_RUNNING){
			ifp->if_flags |= IFF_UP;
		}
		break;

	case SIOCSIFDSTADDR:
		if (ifa->ifa_addr.sa_family != AF_INET){
			error = EAFNOSUPPORT;
		}
		break;

	case SIOCSSLIPDEV:
		error = slsetdevice(ifp, (struct slipvars *)data);
		break;

	case SIOCSIFFLAGS:
		if(!(ifp->if_flags & IFF_RUNNING) || !(ifp->if_flags & IFF_UP)){
			return slclose(ifp);
		}
		break;

	default:
		error = EINVAL;
	}
	splx(s);
	return (error);
}

static void
slread(sc)
	struct sl_softc *sc;
{
	register struct IOExtSer *mp;
	u_char rbuf[SLMAX];
	register int len;

	mp = sc->sc_r;
	slinput(sc, (u_char *)mp->IOSer.io_Data, mp->IOSer.io_Actual);

	do {
		mp->IOSer.io_Command = SDCMD_QUERY;
		DoIO((struct IORequest *)mp);
		if((len = mp->IOSer.io_Actual) > 0){
			if(len > sizeof(rbuf)){
				len = sizeof(rbuf);
			}

			mp->IOSer.io_Command = CMD_READ;
			mp->IOSer.io_Data = (APTR)rbuf;
			mp->IOSer.io_Length = len;
			DoIO((struct IORequest *)mp);
			slinput(sc,(u_char *)rbuf, mp->IOSer.io_Actual);
		}
	} while(mp->IOSer.io_Actual > 0);

	mp->IOSer.io_Command = CMD_READ;
	mp->IOSer.io_Data = (APTR)&sc->sc_rchar;
	mp->IOSer.io_Length = 1;
	SendIO((struct IORequest *)mp);
}

/*
 * slpoll is called from inet task to handle signals from the serial
 * port drivers.  This code runs at splimp() from inside inet process,
 * so almost anything goes...
 */
void slpoll(void)
{
	register struct sl_softc *sc;

	for (sc = sl_softc; sc < (sl_softc + NSL); sc++) {
		if(!sc->sc_inuse){
			continue;
		}

		if(CheckIO((struct IORequest *)sc->sc_r)){
		    	WaitIO((struct IORequest *)sc->sc_r);
			slread(sc);
		}

		if(CheckIO((struct IORequest *)sc->sc_w)){
			WaitIO((struct IORequest *)sc->sc_w);
			sloutfromq(sc);
		}
	}
}
