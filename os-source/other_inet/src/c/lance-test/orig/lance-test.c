/*
 * LANCE diagnostics. 
 *
 * Original code Copyright 1989 Ameristar Technology, Inc
 *
 * Modified by Robert Baker, Commodore QA Jan 1990 for Lattice C
 * 07-Feb-1990
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <exec/types.h>
#include <exec/interrupts.h>
#include <exec/memory.h>
#include <hardware/intbits.h>
#include <libraries/configvars.h>
#include <libraries/dos.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <string.h>
#include <proto/expansion.h>

#include <ameristar.h>
#include <cbm.h>
#include "lance.h"
#include "version.h"

#define	PASS	1		/* test passed			*/
#define FAIL	2		/* test failed - abort diags	*/
#define WARN	3		/* test failed - proceed diags	*/

#define	CUR_UP	"[A"		/* cursor up escape sequence	*/

#define	XMTCHK	1		/* check for errors on transmit	*/
#define	NOXCHK	0		/* don't check for send error	*/
#define MAXBUF 64
#define ENETBUFSIZE	1600		/* room for more than 1 packet  */
#define TOTSIZE(x, r) (sizeof(*es->es_iadr) + ENETBUFSIZE*((x)+(r)) + \
			sizeof(*es->es_rdra)*(r) + sizeof(*es->es_tdra)*(x))

struct board {
	UBYTE es_enaddr[6];		/* ethernet address of board	     */
	struct	ae_iadr *es_iadr;	/* ptr to Init block in enet brd mem */
	struct	ae_rdra *es_rdra;	/* ptr to Rcv ring in enet brd mem   */
	struct	ae_tdra *es_tdra;	/* ptr to Xmit ring in enet brd memory*/
	UBYTE	*es_brdaddr;		/* board address		     */
	UBYTE	*es_buf[MAXBUF];	/* addresses of buffers              */
	ULONG	es_snum;		/* serial num of board               */
	UWORD	*es_csr;		/* ptr to LANCE CSR register	     */
	UBYTE	*es_ram;		/* ptr to start of ram		     */
	UWORD	es_rcvbuf;		/* number of rcv buffers             */
	UWORD	es_xmtbuf;		/* number of xmit buffers            */
	UWORD	es_xmt_next;		/* next buf available for xmit	     */
	UWORD	es_rcv_next;		/* place where next rcvd packet is   */
	ULONG	es_memsize;		/* how much memory board has	     */
} boards[8];

int	maxboard, fcc, errno;
UBYTE	am_enet_mfg[3] = {0x00, 0x00, 0x9f};
UBYTE	co_enet_mfg[3] = {0x00, 0x80, 0x10};
UWORD	errbits, errcsr;
char	*eformat = "%02x:%02x:%02x:%02x:%02x:%02x";

struct boardtype {
	char	*name;
	long	lance_offset;
	long	ram_offset;
	long	ram_size;
	short	prod_num;
	short	mfg_num;
	UBYTE	*enet_num;
} ae_types[] = {
	{"AE-2000", 0x4000, 0x8000, 32768, AMER_ENET1, AMERISTAR, am_enet_mfg},
	{"CBM-1",   0x4000, 0x8000, 32768, CBM_ENET1,  COMMODORE, co_enet_mfg},
	{"AEM-500", 0x20000,0x60000,32768, AMER_AEM500,AMERISTAR, am_enet_mfg},
	0
};

struct Library *ExpansionBase;

static	void	main	(int argc, char *argv[]);
static	long	le_send (int brd, char *p, int len, int check);
static	long	le_recv (int brd, char *p, int len);
static	int	init_board(int board, int mode);
static	int	build_board(struct ConfigDev *cd, struct BoardType *ap);
static	int	findboards(void);
static	int	printboards(void);
static	void	print_ether(char *p);
static	int	mscanf(char *fmt, void *a, ...);
static	int	get_ether(char *p);
static	long	lsend(int board);
static	long	lrecv(int board, int rmode);
static	int	lance_test(int board);
static	int	memory_test(int board);
static	int	__interrupt __saveds int_service(void);
static	int	int_test(int board);
static	int	internal_loop_test(int board);
static	int	lance_coll(int board);
static	int	xloop(int board);
static	int	diags(int board);
static	void	usage(void);
static	void	send_err(long cnt);
static	void	recv_err(long cnt);
static	void	send_num(long cnt, long error);
static	void	recv_num(long cnt, long error);
	

/*
 * Send a packet to network.  Return -1 if didn't send packet.
 */

#define	SENDERR	(AE_BABL | AE_CERR | AE_MERR) 

long	le_send (brd, p, len, check)
char	*p;
int	brd, len, check;
{
	register struct board *es;
	register int pos, cnt;

	es = &boards[brd];
	pos = es->es_xmt_next;
	es->es_xmt_next = (pos + 1) & (es->es_xmtbuf - 1);
	errno = 0;

	for (cnt = 50; cnt > 0; cnt--) {
		if (SetSignal(0L, SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C) {
			errno = EINTR;
			return -1;
		}
		if ((es->es_tdra[pos].flags & TDRA_OWN) != TDRA_OWN) {	
			break;
		}
		Delay(1);
	}

	if (cnt == 0) {			/* no buffer avail */
		errno = ETIMEDOUT;
		return -1;
	}

	memcpy(es->es_buf[pos], p, len);
	es->es_tdra[pos].bcnt = -len;

/* flag msg descriptor as owned by Lance,
 * then clear Lance Xmt/Rcv intr flags and Xmit Demand
 * to start transmission of test packet.
 */
	es->es_tdra[pos].flags = TDRA_OWN | TDRA_STP | TDRA_ENP;
	*es->es_csr = AE_CLEAR | AE_TDMD;	 

	if (!check) {
		return len;
	}

	for (cnt = 50; cnt > 0; cnt--) {	/* wait for own flag to clear */
		if (SetSignal(0L, SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C) {
			errno = EINTR;
			return -1;
		}
		if ((es->es_tdra[pos].flags & TDRA_OWN) != TDRA_OWN) {	
			break;
		}
		Delay(1);
	}

	if (cnt == 0) {			/* lance did not send packet */
		errno = ETIMEDOUT;
		return -1;
	}

	errcsr = *es->es_csr;		/* save CSR0 state */
	errbits = es->es_tdra[pos].errors;	/* and error flags */
	*es->es_csr = AE_CLEAR;		/* clear intr & error bits */

	if ((errcsr & SENDERR) || (es->es_tdra[pos].flags & TDRA_ERR)) {
		errno = ENETERR;	/* check for error conditions */
		return -1;
	}
	return len;
}

/* Transmit error display */

#define	TERRMSK	(TDRA_UFLO | TDRA_LCOL | TDRA_LCAR | TDRA_RTRY)

void	send_err(cnt)
long	cnt;
{
	printf("Transmit failed after %d packets\n", cnt);
	if ((errcsr & SENDERR) || (errbits & TERRMSK)) {
		printf("Error Flags = ");
		if (errcsr & AE_BABL) printf("BABL ");
		if (errcsr & AE_CERR) printf("CERR ");
		if (errcsr & AE_MERR) printf("MERR ");
		if (errbits & TDRA_BUFF) printf("BUFF ");
		if (errbits & TDRA_UFLO) printf("UFLO ");
		if (errbits & TDRA_LCOL) printf("LCOL ");
		if (errbits & TDRA_LCAR) printf("LCAR ");
		if (errbits & TDRA_RTRY) {
			printf("RTRY\n");
			printf("TDR indicates cable open at %d nanoseconds",
			    (errbits & TDRA_TDRMSK) * 100);
		}
		printf("\n");
	}
	printf("\n");
}


/*
 * Receive a packet from net.  Return 0 if no packets waiting, otherwise
 * actual length of packet is returned.
 */

#define RECVERR	(AE_MISS | AE_MERR)	/* receive errors */

long	le_recv (brd, p, len)
char	*p;
int	brd, len;
{
	register struct board *es;
	register int pos, cnt;

	es = &boards[brd];
	pos = es->es_rcv_next;
	errno = 0;

	for (cnt = 100; cnt > 0; cnt--) {
		if (SetSignal(0L, SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C) {
			errno = EINTR;
			return -1;
		}
		if ((es->es_rdra[pos].flags & RDRA_OWN) != RDRA_OWN) {
			break;
		}
		Delay(1);
	}

	if (cnt == 0) {			/* no packet avail */
		errno = ETIMEDOUT;
		return -1;
	}
	errcsr = *es->es_csr;			/* save CSR0 state */
	errbits = es->es_rdra[pos].flags;	/* and error flags */
	*es->es_csr = AE_CLEAR;		/* clear intr & error bits */

	if ((errcsr & RECVERR) || (errbits & RDRA_ERR)) {
		errno = ENETERR;
	}
	es->es_rcv_next = (pos + 1) & (es->es_rcvbuf - 1);
	if (errno == 0) {
		len = min(es->es_rdra[pos].mcnt, len);
		memcpy(p, es->es_buf[pos + es->es_xmtbuf], len); 
	}
	es->es_rdra[pos].flags = RDRA_OWN;
	return (errno == 0) ? len : -1;
}

/* Receive error display */

void	recv_err(cnt)
long	cnt;
{
	printf("Receive failed after %d packets\n", cnt);
	if ((errcsr & RECVERR) || (errbits & RDRA_ERR)) {
		printf("Error Flags = ");
		if (errcsr & AE_MISS) printf("MISS ");
		if (errcsr & AE_MERR) printf("MERR ");
		if (errbits & RDRA_FRAM) printf("FRAM ");
		if (errbits & RDRA_OFLO) printf("OFLO ");
		if (errbits & RDRA_CRC) printf("CRC ");
		if (errbits & RDRA_BUFF) printf("BUFF ");
		printf("\n");
	}
	printf("\n");
}

/*
 * Initialize board for send/recv tests...
 */

int	init_board(board, mode)
int	board, mode;
{
	register struct ae_iadr *iadr;
	register struct board *es;
	UWORD *csr;
	UBYTE *p;
	int i;

	es = &boards[board];
	if (es->es_csr == 0) {		/* LANCE CSR should never = 0  */
		return -1;
	}

	csr = es->es_csr;
	iadr = es->es_iadr;
	*csr = AE_STOP;			/* STOP Lance chip */

	memset(es->es_ram, 0, es->es_memsize);		
	memset(iadr, 0, sizeof(struct ae_iadr));

	iadr->mode = mode;	/* set up initialization block */
	iadr->padr[1] = es->es_enaddr[0]; iadr->padr[0] = es->es_enaddr[1];
	iadr->padr[3] = es->es_enaddr[2]; iadr->padr[2] = es->es_enaddr[3];
	iadr->padr[5] = es->es_enaddr[4]; iadr->padr[4] = es->es_enaddr[5];
	iadr->rdra = (UWORD)((long)es->es_rdra) & 0xffff;
	iadr->rdra_hi = (UWORD)(((long)es->es_rdra) >> 16);
	iadr->rlen = ffs(es->es_rcvbuf) - 1;
	iadr->tdra = (UWORD)((long)es->es_tdra) & 0xffff;
	iadr->tdra_hi = (UWORD)(((long)es->es_tdra) >> 16);
	iadr->tlen = ffs(es->es_xmtbuf) - 1;

	for (i = 0; i < es->es_rcvbuf; i++) {	/* init receive buffer */
		register struct ae_rdra *rdra;

		rdra = es->es_rdra + i;
		p = es->es_buf[i+es->es_xmtbuf];
		rdra->rbadr = (UWORD) ((long)p) & 0xffff;
		rdra->ha = (UWORD)(((long)p) >> 16);
		rdra->bcnt = -ENETBUFSIZE;
		rdra->mcnt = 0;
		rdra->flags = RDRA_OWN;
	}	

	for (i = 0; i < es->es_xmtbuf; i++) {	/* init transmit buffer */
		register struct ae_tdra *tdra;

		tdra = es->es_tdra + i;
		p = es->es_buf[i];
		tdra->tbadr = (UWORD) ((long)p) & 0xffff;
		tdra->ha = (UWORD)(((long)p) >> 16);
		tdra->flags = TDRA_ENP | TDRA_STP;
	}	
	es->es_xmt_next = es->es_rcv_next = 0;

	csr = es->es_csr;
	csr[1] = CSR3;
	*csr = CSR3_BSWAP;	/* byte swap on bus accesses */
	csr[1] = CSR2;
	*csr = (UWORD)((long)iadr) >> 16;	/* init block adr */
	csr[1] = CSR1;
	*csr = (UWORD)((long)iadr) & 0xffff;
	csr[1] = CSR0;
	*csr = AE_INIT;		/* initialize Lance chip */

	for (i = 100; i > 0; i--) {
		if (*csr & AE_IDON) {		/* wait for done init */
			*csr = (AE_STRT | AE_IDON);	/* start LANCE */
			break;
		}
		Delay(1);		/* error time-out */
	}

	return (i == 0) ? -1:0;
}


int	build_board(cd, ap)
struct	ConfigDev *cd;
struct	boardtype *ap;
{
	register int size, total_bufs;
	register struct board *es;
	int i;

	es = &boards[maxboard++];

	es->es_snum = cd->cd_Rom.er_SerialNumber;
	memcpy(es->es_enaddr, ap->enet_num, 3);
	es->es_enaddr[3] = (es->es_snum >> 16) & 0xff; 
	es->es_enaddr[4] = (es->es_snum >> 8) & 0xff; 
	es->es_enaddr[5] = es->es_snum & 0xff;

	es->es_brdaddr = (UBYTE *)cd->cd_BoardAddr;
	es->es_ram = es->es_brdaddr + ap->ram_offset;
	es->es_csr = (UWORD *)(es->es_brdaddr + ap->lance_offset);
	es->es_memsize = ap->ram_size;

	es->es_xmtbuf = es->es_rcvbuf = 4; size = 0;
	do {
		es->es_rcvbuf <<= 1;
		size = TOTSIZE(es->es_xmtbuf, es->es_rcvbuf);
		total_bufs = es->es_xmtbuf + es->es_rcvbuf;
	} while ((size <= es->es_memsize) && (total_bufs <= MAXBUF));
	es->es_rcvbuf >>= 1;

	es->es_iadr = (struct ae_iadr *)es->es_ram;
	es->es_tdra = (struct ae_tdra *)(es->es_iadr + 1); 
	es->es_rdra = (struct ae_rdra *)(es->es_tdra + es->es_xmtbuf);
	es->es_buf[0] = (UBYTE *)(es->es_rdra + es->es_rcvbuf);
	for (i = 1; i < (es->es_xmtbuf + es->es_rcvbuf); i++) {
		es->es_buf[i] = es->es_buf[0] + i*ENETBUFSIZE;
	}

	return 0;
}


int	findboards()
{
	struct ConfigDev *cd;
	int i;

	if ((ExpansionBase = OpenLibrary("expansion.library", 0)) == 0) {
		printf("Could not open expansion.library\n");
		return -1;
	}

	maxboard = 0;
	for (i = 0; ae_types[i].name; i++) {
		cd = 0;
		do {
			cd = FindConfigDev((ULONG) cd, ae_types[i].mfg_num, ae_types[i].prod_num);
			if (cd) {
				build_board(cd, &ae_types[i]);
			}
		} while (cd);
	}
	CloseLibrary(ExpansionBase);

	if (maxboard == 0) {
		return -1;
	}
	return 0;
}


int	printboards()
{
	int 	i;

	printf("Ethernet boards in system:\n");
	for (i = 0; i < maxboard; i++) {
		printf("Board %d at addr %06x;  serial num %08x;  ether addr ",
			1 + i, boards[i].es_brdaddr, boards[i].es_snum);
		print_ether(boards[i].es_enaddr);
		printf("\n");
	}
	return 0;
}


void	print_ether(p)
char	*p;
{
	printf(eformat, p[0], p[1], p[2], p[3], p[4], p[5]);
}

mscanf(fmt, a, b, c, d, e, f)
char	*fmt;
void	*a, *b, *c, *d, *e, *f;
{
	char 	line[80];

	do {
		gets(line);
	} while (!line[0]);
	return sscanf(line, fmt, a, b, c, d, e, f);
}


int	get_ether(p)
char	*p;
{
	unsigned int buf[6], i, num;

	num = mscanf(eformat, &buf[0],&buf[1],&buf[2],&buf[3],&buf[4],&buf[5]);
	if (num != 6) {
		return -1;
	}
	for (i = 0; i < 6; i++) {
		if (buf[i] > 0xff) {
			return -1;
		}
		p[i] = buf[i];
	}
	return 0;
}

/*
 * Packet SEND Test
 * Expects corresponding receive test running on another system
 */

#define	GSIZE	10000		/* FCC Packet Group Size  */
#define	PSIZE	1500		/* Individual Packet Size */
#define	MSIZE	100		/* Packet count for stats */

long	lsend(board)
int	board;
{
	long	num, cnt, tcnt, packgrp, error;
	int	len, valid;
	static	char buf[1518];

	if (init_board(board, 0) < 0) {
		printf("Board %d didn't initialize properly\n", board+1);
		return 1;
	}

	for (len = 0; len < sizeof(buf); len++) {
		if (fcc) {
			buf[len] = 'H';    /* H packets for FCC testing */
		} else {
			buf[len] = len;    /* else variable data */
		}
	}
	memcpy(&buf[6], boards[board].es_enaddr, 6);

	do {
		valid = 0;

		printf("Enter ethernet address to send to: (XX:XX:XX:XX:XX:XX) ");
		if (get_ether(&buf[0]) < 0) {
			continue;
		}
	
		if (!fcc) {
			printf("Enter number of packets to send: ");
			if (mscanf("%d", &num) != 1) {
				continue;
			}

			printf("Enter packet length [60..1500] ");
			if (mscanf("%d", &len) != 1 || len < 60 || len > 1500) {
				continue;
			}
		} else {
			printf("FCC testing uses %d byte H-packets,\n", PSIZE);
			printf("in groups of %d packets, by default\n", GSIZE);
			num = GSIZE;
			len = PSIZE;
		}

		valid++;

	} while (!valid);

	printf("Starting:  SEND TEST,  type ^C to abort\n\n");
	packgrp = error = 0;

	for (cnt = 1, tcnt = 0; tcnt < num; cnt++) {
		if (le_send(board, buf, len, XMTCHK) < 0) {
			switch (errno) {
			case EINTR:		/* abort transmit test */
			    if (fcc) {
				printf("Total H-packet groups sent this group: %d\n", cnt);
			    } else {
				send_num(cnt, error);
			    }
			    return 0;
			case ENETERR:		/* send problem */
				if (!fcc) send_err(cnt);
				break;
			case ETIMEDOUT:
				printf("Transmit time-out after packet %d\n\n", cnt);
				break;
			default:
				printf("Internal error - could not send packet %d\n", cnt);
				return 1;
			}
			error++;
		} 
		if (fcc) {			/* indicate progress */
        	    if (cnt >= GSIZE) {
			cnt = 1;
			packgrp++;	/* FCC packet/group counts */
        		printf("%sTotal H-packet groups sent: %d\n", CUR_UP, packgrp);
		    }
		} else {
		    tcnt++;
		    if ((cnt % MSIZE) == 0) send_num(cnt, error);
		}
	}
	send_num(--cnt, error);
	return (error == 0) ? 0:1;
}

/* Indicate number of packets sent */

void	send_num(cnt, error)
long	cnt, error;
{
	printf("%sTotal data packets sent: %d,  errors: %d\n", CUR_UP, cnt, error);
}

/*
 *  Packet RECEIVE Test
 *  Expects corresponding send test running on another system
 */

long	lrecv(board, rmode)
int	board, rmode;
{
	static	unsigned char lasts[6], lastr[6], buf[64];
	long	cnt, packgrp, error;

	if (init_board(board, rmode) < 0) {
		printf("Board didn't initialize properly\n");
		return 1;
	}

	printf("Starting:  RECEIVE TEST,  type ^C to abort\n");
	if (rmode & MODE_PROM) {
		printf("Receiver operating in PROMISCUOUS mode\n");
	}
	if ((rmode & MODE_DTCR) == 0) {
		printf("Receiver CRC checks disabled\n"); 
	}
	if (fcc) {
		printf("Expecting H-packets for FCC testing\n");
	}
	memset(lasts, 0, sizeof(lasts));
	memset(lastr, 0, sizeof(lastr));
	cnt = packgrp = error = 0;

	while (1) {
	    if (le_recv(board, buf, sizeof(buf)) < 0) {
		switch (errno) {
		case EINTR:		/* abort receive test */
			if (fcc) {
			    printf("Total H-packets received this group: %d\n", cnt);	
			} else {
			    recv_num(cnt, error);
 			}
			return 0;
		case ETIMEDOUT:		/* no message avail - continue */
			continue;
		case ENETERR:		/* receive problem */
			if (!fcc) recv_err(cnt);
			break;
		default:
			printf("Internal error - could not receive packet %d\n", cnt);
			return 1;
		}
		error++;
	    } else {		/* test progress & status */
		cnt++;
		if ((memcmp(lasts, &buf[6], 6) != 0) ||
		   ((rmode & MODE_PROM) && (memcmp(lastr, &buf[0], 6) != 0))) {
			error = packgrp = 0;
			cnt = 1;
			memcpy(lasts, &buf[6], sizeof(lasts));
			printf("Receiving packets from ");
			print_ether(&buf[6]);
			printf("\n");
			if (rmode & MODE_PROM) {
			    memcpy(lastr, &buf[0], sizeof(lastr));
			    printf("          Addressed to ");
			    print_ether(&buf[0]);
			    printf("\n");
			}
			printf("\n");
		}

		if (!fcc) {			/* indicate progress */
			if ((cnt % MSIZE) == 0) recv_num(cnt, error);
		} else {
			if (cnt >= GSIZE) {
			    cnt = 0;
			    packgrp++;
			    printf("%sTotal H-packet groups received: %d\n", CUR_UP, packgrp);
			}
		}
	    }
	}
}

/* Indicate number of packets received */

void	recv_num(cnt, error)
long	cnt, error;
{
	printf("%sTotal data packets received: %d,  errors: %d\n", CUR_UP, cnt, error);
}

/*
 * Try to initialize LANCE buffers + chip.
 */

int	lance_test(board)
int	board;
{
	if (init_board(board, 0) < 0) {
		return FAIL;
	}
	return PASS;
}


/*
 * memory_test - lance buffer memory test.  Checks for stuck address
 *		 lines, and does checkerboard test.
 */

int	memory_test(board)
int	board;
{
	register UWORD *p, *limit, patt, patt1, *rm;
	struct board *es;
	register int cnt;

	es = &boards[board];
	*es->es_csr = AE_STOP;

	rm = (UWORD *)es->es_ram;
	limit = (UWORD *)(es->es_ram + es->es_memsize);
	for (cnt = 0; cnt < 5; cnt++) {

		patt = 0; patt1 = 0x1001;
		for (p = rm; p < limit; patt += patt1) {
			*p++ = patt;
		}
		patt = 0; 
		for (p = rm; p < limit; patt += patt1) {
			if (*p++ != patt) {
				return FAIL;
			}
		}
		patt = 0x5555; patt1 = 0xffff;
		for (p = rm; p < limit; patt ^= patt1) {
			*p++ = patt;
		}
		patt = 0x5555;
		for (p = rm; p < limit; patt ^= patt1) {
			if (*p++ != patt) {
				return FAIL;
			}
		}
		patt = 0xAAAA;
		for (p = rm; p < limit; patt ^= patt1) {
			*p++ = patt;
		}
		patt = 0xAAAA;
		for (p = rm; p < limit; patt ^= patt1) {
			if (*p++ != patt) {
				return FAIL;
			}
		}
	}
	return PASS;
}


/*
 * int_test - check that interrupt jumper is in the "correct" place, and
 *	      that the board can cause an interrupt.  Returns WARN if could
 *	      not perform test due to low memory condition.  This test is
 *	      actually very weak since there may be others on the same
 *	      int chain the board is on.
 */

#define	AE_INTS (AE_BABL | AE_CERR | AE_MISS | AE_MERR | AE_RINT | AE_TINT | AE_IDON)

static UWORD *int_csr;
static saw_int;

int	__interrupt __saveds	int_service()
{
	UWORD ints;

	if ((ints = *int_csr & AE_INTS) != 0) {
		*int_csr = ints;
		if (ints & AE_IDON) {
			saw_int++;
		}
		return 1;
	}
	return 0;
}


int	int_test(board)
int	board;
{
	extern void *AllocMem();
	struct Interrupt *in;
	struct board *es;
	int k, i;

	es = &boards[board];
	int_csr = es->es_csr;
	saw_int = 0;
	in = (struct Interrupt *)AllocMem(sizeof(struct Interrupt), MEMF_PUBLIC);
	if (!in) {
		return WARN;
	}
	in->is_Node.ln_Type = NT_INTERRUPT;
	in->is_Node.ln_Pri = -127;
	in->is_Node.ln_Name = "LANCE interrupt";
	in->is_Code = (VOID (*)())int_service;
	in->is_Data = (APTR)0;
	AddIntServer(INTB_PORTS, in);
	for (k = 0; k < 10; k++) {
		Disable();
		saw_int = 0;
		*es->es_csr = AE_STOP;
		*es->es_csr = AE_INIT | AE_INEA;
		Enable();
		for (i = 20; i > 0 && saw_int == 0; i--) {
			Delay(10);
		}
		if (saw_int == 0) {
			break;
		}
	}
	RemIntServer(INTB_PORTS, in);
	return (saw_int==0 ? FAIL:PASS);
}


/*
 * loop_test - local loopback test.
 */
#define LSIZE	24	/* 24 byte loopback packets */

int	internal_loop_test(board)
int	board;
{
	static char buf1[128], buf[256];
	struct board *es;
	int cnt, len;

	if (init_board(board, MODE_LOOP | MODE_INTL) < 0) {
		return FAIL;
	}
	es = &boards[board];
	memset(buf1, 0, sizeof(buf1));
	memcpy(buf1, es->es_enaddr, sizeof(es->es_enaddr));

	for (cnt = 0; cnt < 100; cnt++) {
		if (le_send(board, buf1, LSIZE, XMTCHK) < 0) {
			if (errno == ETIMEDOUT) {
				return WARN;
			}
			return FAIL;
		}
		len = le_recv(board, buf, sizeof(buf));
		if (len < 0) {
			if (errno == ETIMEDOUT) {
				return WARN;
			}
			return FAIL;
		} else if (len != LSIZE+4) {
			return FAIL; 
		}
	}
	return PASS;
}

/* External loopback test checks Ethernet cable connection */

#define	XLP_CNT		200	/*  #packets to send   */	
#define	XLP_SIZE	24	/*  #bytes per packet  */

int	xloop(board)
int	board;
{
	static	char	buf1[128], buf[256];
	struct	board	*es;
	int	len;
	long	cnt;

	if (init_board(board, MODE_LOOP) < 0) {
		printf("Board didn't initialize properly\n");
		return 1;
	}
	es = &boards[board];
	for (len = 0; len < LSIZE; len++) {
		buf1[len] = len;
	}
	memcpy(buf1, es->es_enaddr, sizeof(es->es_enaddr));

	printf("Starting:  External Loopback test,  Type ^C to abort\n");

	for (cnt = 1; cnt <= XLP_CNT; cnt++) {
		if (le_send(board, buf1, XLP_SIZE, XMTCHK) < 0) {
			switch (errno) {
			case EINTR:
				printf("Test Aborted after %d packets\n", cnt);
				return 0;
			case ENETERR:		/* send problem */
				send_err(cnt);
				return 1;
			case ETIMEDOUT:
				printf("Transmit time-out after %d packets, check cable connections.\n", cnt);
				return 1;
			default:
				printf("Internal error - count not send packet %d\n", cnt);
				return 1;
			}
		}
		len = le_recv(board, buf, sizeof(buf));
		if (len < 0) {
			switch (errno) {
			case EINTR:
				printf("Test aborted after %d packets\n", cnt);
				return 0;
			case ENETERR:		/* receive problem */
				recv_err(cnt);
				return 1;
			case ETIMEDOUT:
				printf("Receive time-out after %d packets\n", cnt);
				return 1;
			default:
				printf ("Internal error - cound not receive packet %d\n", cnt);
				return 1;
			}	
		} else if (len != LSIZE+4) {
			printf("Incorrect received packet size after %d packets\n", cnt);
			return 1;
		}
	}
	printf("External Loopback test passed.\n");
	return 0;
}


/*
 * lance_coll - test lance collision logic.
 */

#define COLLS 10

int	lance_coll(board)
int	board;
{
	int cnt, len, got_coll;
	static char buf[128];
	struct board *es;

	if (init_board(board, MODE_COLL | MODE_INTL | MODE_LOOP) < 0) {
		return FAIL;
	}

	es = &boards[board];
	memset(buf, 0, sizeof(buf));
	memcpy(buf, es->es_enaddr, sizeof(es->es_enaddr));
	for (got_coll = cnt = 0; cnt < COLLS; cnt++) {
		len = le_send(board, buf, 32, XMTCHK); 
		if ((len < 0) && (errbits & TDRA_RTRY)) {
			got_coll++;
		}
	}
	return (got_coll == COLLS)? PASS:FAIL;
}


static struct {
	int	(*test_proc)(int board);
	char	*test_name;
	int	do_test;	/* set to 0 to bypass test */
} test[] = {
	{memory_test,	"Buffer memory test..............", 1},
	{lance_test,	"LANCE configuration test........", 1},
	{int_test,	"Interrupt test..................", 1},	
	{lance_coll,	"LANCE collision logic test......", 1},
	{internal_loop_test,	
			"Internal loopback test..........", 1},
	0
};

#define	DLIST	99
#define	SKIPPED	98	

int	diags(board)
int	board;
{
	int i, stat;

	printf("\n\t    Ethernet Controller Diagnostics");
	if (board == DLIST) {
		printf(" Include:");
	}
	printf("\n\n");
	for (i = 0; test[i].test_proc != (int (*)())0; i++) {
		printf("\t\t%s ", test[i].test_name);
		if ((board != DLIST) && test[i].do_test) {
			stat = ((*test[i].test_proc)(board));
		} else if (test[i].do_test == 0) {
			stat = SKIPPED;
		} else {
			stat = DLIST;
		}
		switch (stat) {
		case PASS:
			printf("PASS\n");
			break;

		case FAIL:
			printf("FAIL\n");
			return 1;
			break;

		case WARN:
			printf("WARN\n");
			break;

		case SKIPPED:
			printf("BYPASSED\n");
			break;

		case DLIST:
			printf("\n");
			break;
		}
	}
	printf("\n");
	if (board != DLIST) {
		printf("\t    Controller PASSED diagnostics.\n\n");
	}
	return 0;
}


void	usage()
{
	printf("\nUsage:  Lance-Test  addrs | diags | loop | recv [fcc][any] | send [fcc] | ?\n\n");
	exit (1);
}


void	main (argc, argv)
int	argc;
char	*argv[];
{
	int	d, s, r, a, l, rtn, board;
	int	rmode;

	printf("%s\n", version);
	if (argc == 1) {
		usage();
	}
	
	if (inet_up()) {
		printf("Network already running.  Please reboot before running this test!\n");
		exit (1);
	}

	rmode = MODE_DTCR;	/* crc check on for recv test */
	d = s = r = a = l = fcc = 0;
	for (argv++; --argc > 0; argv++) {
		if (stricmp(*argv, "diags") == 0) {
			d++;
		} else if (stricmp(*argv, "?") == 0) {
			printf("\n\tAddrs = Display board addresses\n");
			printf("\tSend  = Transmit packets      (fcc = H-packets)\n");
			printf("\tRecv  = Receive  packets      (fcc = H-packets)\n");
			printf("\t                              (any = promiscious mode)\n");
			printf("\tLoop  = External loopback test\n");
			printf("\tDiags = Run all diagnostics...\n");
			diags(DLIST);
			exit (0);
		} else	if (stricmp(*argv, "send") == 0) {
			s++;
		} else	if (stricmp(*argv, "recv") == 0) {
			r++;
		} else	if (stricmp(*argv, "loop") == 0) {
			l++;
		} else	if (stricmp(*argv, "addrs") == 0) {
			a++;
		} else	if (stricmp(*argv, "any") == 0) {
			if (r) {
				rmode = MODE_PROM;
			} else {
				usage();
			}
		} else	if (stricmp(*argv, "fcc") == 0) {
			if (r || s) {
				fcc++;
			} else {
				usage();
			}
		} else {
			usage();
		}
	}
	if (d+s+r+a+l > 1) {
		usage();
	}

	findboards();
	if (maxboard == 0) {
		printf("\nNo Ethernet boards found in system!\n\n");
		exit (1);
	}

	if(!a) {
		if (maxboard > 1) {
			printboards();
			printf("\n");
			do {
				printf("Select board [1..%d]: ", maxboard);
				board = -1;
				mscanf("%d", &board);
			} while (board < 1 || board > maxboard);
		} else {
			board = 1;
		}
		--board;

		printf("Ethernet address of selected board is:  ");
		print_ether(boards[board].es_enaddr);
		printf("\n");
	}

	rtn = 0;
	if (a) { rtn |= printboards(); }
	if (d) { rtn |= diags(board); } 
	if (r) { rtn |= lrecv(board, rmode); }
	if (s) { rtn |= lsend(board); }
	if (l) { rtn |= xloop(board); }

	if (rtn != 0) rtn = 1;
	exit (rtn);
}
