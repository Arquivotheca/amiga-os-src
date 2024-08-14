/*
 * Definitions for SLIP interface data structures
 * 
 * (this exists so programs like slstats can get at the definition
 *  of sl_softc.)
 *
 * @(#) $Header: hognet:src/libs/inet.library/RCS/if_slvar.h,v 1.1 92/08/14 14:10:13 gregm Exp $ (LBL)
 */
struct sl_softc {
	struct ifnet sc_if;	/* network-visible interface */
	struct ifqueue sc_fastq; /* interactive output queue */
	u_char *sc_mp;		/* pointer to next available buf char */
	u_char *sc_ep;		/* pointer to last available buf char */
	u_char *sc_buf;		/* input buffer */
	u_int sc_bytessent;
	u_int sc_bytesrcvd;
	struct slcompress *sc_comp; /* tcp compression data */
	struct IOExtSer *sc_r;	/* read msg ptr */
	struct IOExtSer *sc_w;	/* write msg ptr */
	u_char	*sc_ibuf;	/* new for amiga */
	u_char  *sc_obuf;	/* output buffer */
	u_char	sc_rchar;	/* one character read buffer */
	u_char	sc_inuse;	/* interface is allocated */
	u_char	sc_escape;	/* =1 if last char input was FRAME_ESCAPE */
	u_char	sc_reserved2;
};

/*
 * There are three per-line options kept in the device specific part of
 * the interface flags word:  IFF_LINK0 enables compression; IFF_LINK1
 * enables compression if a compressed packet is received from the
 * other side; IFF_LINK2 will drop (not send) ICMP packets.
 */
#ifndef IFF_LINK0
/*
 * This system doesn't have defines for device specific interface flags,
 * Define them.
 */
#define IFF_LINK0	0x8000
#define IFF_LINK1	0x4000
#define IFF_LINK2	0x2000
#endif
