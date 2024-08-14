/*
 * AE2000-100 Ethernet defines
 */

#ifndef AEIF_AE_H
#define AEIF_AE_H
#define AEBIT(n) (1<<n)		/* set bit <n>				*/
#define LEINTB	INTB_PORTS	/* Lance pulls level 2 interrupt	*/

/*
 * Format of Init block in memory.
*/
#define	MODE_PROM	AEBIT(15)
#define MODE_INTL	AEBIT(6)
#define MODE_DRTY	AEBIT(5)
#define MODE_COLL	AEBIT(4)
#define MODE_DTCR	AEBIT(3)
#define MODE_LOOP	AEBIT(2)
#define MODE_DTX	AEBIT(1)
#define MODE_DRX	AEBIT(0)

struct ae_iadr {
	u_short	mode;
	u_char	padr[6];
	u_char	ladr[8];
#ifdef LATTICE
	u_int	rdra:16, rlen:3, rdra_res:5, rdra_hi:8;
	u_int	tdra:16, tlen:3, tdra_res:5, tdra_hi:8;
#elif AZTEC_C
	u_int	rdra_hi:8,
			rdra_res:5,
			rlen:3,
			rdra:16;
	u_int	tdra_hi:8,
			tdra_res:5,
			tlen:3,
			tdra:16;	
#else /* manx 3.6 */
	u_short	rdra;
	u_short	rdra_hi:8, rdra_res:5, rlen:3;
	u_short	tdra;
	u_short tdra_hi:8, tdra_res:5, tlen:3;
#endif
};

/*
 * Format of receive buffer descriptor in memory.  
 */
#define RDRA_OWN	AEBIT(7)
#define RDRA_ERR	AEBIT(6)
#define RDRA_FRAM	AEBIT(5)
#define RDRA_OFLO	AEBIT(4)
#define RDRA_CRC	AEBIT(3)
#define RDRA_BUFF	AEBIT(2)
#define RDRA_STP	AEBIT(1)
#define RDRA_ENP	AEBIT(0)
#define FATALERR (RDRA_OFLO|RDRA_CRC|RDRA_FRAM)

struct ae_rdra {
#ifdef LATTICE
	u_int	rbadr:16, flags:8, ha:8;
#elif AZTEC_C
	u_int ha:8, flags:8, rbadr:16;
#else /* manx 3.6 */
	u_short	rbadr;
	u_short ha:8, flags:8;
#endif
	short	bcnt;
	short	mcnt;
};

/*
 * Format of transmit buffer descriptor in memory.  
 */
#define TDRA_ENP	AEBIT(0)
#define TDRA_STP	AEBIT(1)
#define TDRA_DEF	AEBIT(2)
#define TDRA_ONE	AEBIT(3)
#define TDRA_MORE	AEBIT(4)
#define TDRA_RES	AEBIT(5)
#define TDRA_ERR	AEBIT(6)
#define TDRA_OWN	AEBIT(7)

struct ae_tdra {
#ifdef LATTICE
	u_int	tbadr:16, flags:8, ha:8;
	int	bcnt:16, buff:1, uflo:1, res:1, lcol:1, lcar:1, rtry:1, tdr:10;
#elif AZTEC_C
	u_int 	ha:8, 
			flags:8,
			tbadr:16;
	u_int	tdr:10, rtry:1, lcar:1, lcol:1, res:1, uflo:1, buff:1, bcnt:16;
#else /* manx 3.6 */
	u_short	tbadr;
	u_short ha:8, flags:8;
	short	bcnt;
	u_short	tdr:10, rtry:1, lcar:1, lcol:1, res:1, uflo:1, buff:1;
#endif
};

#define CSR0		0
#define AE_INIT		AEBIT(0)
#define AE_STRT		AEBIT(1)
#define AE_STOP 	AEBIT(2)
#define AE_TDMD 	AEBIT(3)
#define AE_TXON 	AEBIT(4)
#define AE_RXON 	AEBIT(5)
#define AE_INEA 	AEBIT(6)
#define AE_INTR 	AEBIT(7)
#define AE_IDON 	AEBIT(8)
#define AE_TINT 	AEBIT(9)
#define AE_RINT 	AEBIT(10)
#define AE_MERR 	AEBIT(11)
#define AE_MISS 	AEBIT(12)
#define AE_CERR 	AEBIT(13)
#define AE_BABL 	AEBIT(14)
#define AE_ERR  	AEBIT(15)

#define CSR1		1
#define CSR2		2
#define CSR3		3
#define CSR3_BSWAP	AEBIT(2)

#endif
