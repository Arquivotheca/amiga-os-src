/*
 *  Ethernet defines...
 *	07-Feb-1990
 */

#include <sys/types.h>

extern	int	inet_up(void);
extern	int	ffs(long);

#define AEBIT(n) (1<<n)		/* set bit <n>				*/
#define LEINTB	INTB_PORTS	/* Lance pulls level 2 interrupt	*/

/*  MODE Register  */

#define	MODE_PROM	AEBIT(15)	/* Promiscuous Mode	*/
#define MODE_INTL	AEBIT(6)	/* Internal Loopback	*/
#define MODE_DRTY	AEBIT(5)	/* Disable Retry	*/
#define MODE_COLL	AEBIT(4)	/* Force Collision	*/
#define MODE_DTCR	AEBIT(3)	/* Disable Transmit CRC	*/
#define MODE_LOOP	AEBIT(2)	/* Loopback		*/
#define MODE_DTX	AEBIT(1)	/* Disable Transmitter	*/
#define MODE_DRX	AEBIT(0)	/* Disable Reveiver	*/

/*  LANCE Initialization Block  */

struct ae_iadr {
	u_short	mode;		/* Mode Register  */
	u_char	padr[6];	/* Physical Address of Lance (48 bits)  */
	u_char	ladr[8];	/* Logical Descriptor Ring Pointer  */
	u_int	rdra:16, rlen:3, rdra_res:5, rdra_hi:8;  /* Rcv Desc Ring */
	u_int	tdra:16, tlen:3, tdra_res:5, tdra_hi:8;  /* Xmt Desc Ring */
};


/* Receive Message Descriptors (RMD0-RMD3)  */ 
 
#define RDRA_OWN	AEBIT(7)	/* Onwed by Lance	*/
#define RDRA_ERR	AEBIT(6)	/* Error Summary	*/
#define RDRA_FRAM	AEBIT(5)	/* Framing Error	*/
#define RDRA_OFLO	AEBIT(4)	/* Overflow		*/
#define RDRA_CRC	AEBIT(3)	/* CRC Error		*/
#define RDRA_BUFF	AEBIT(2)	/* Buffer Error		*/
#define RDRA_STP	AEBIT(1)	/* Start of Packet	*/
#define RDRA_ENP	AEBIT(0)	/* End of Packet	*/

#define FATALERR (RDRA_OFLO | RDRA_CRC | RDRA_FRAM)

struct ae_rdra {
	u_int	rbadr:16, flags:8, ha:8;	/* Buffer Adr & Status */
	short	bcnt;				/* Buffer  Byte Count (RMD3) */
	short	mcnt;				/* Message Byte Count (RMD2) */
};

/*  Transmit Message Descriptors (TMD0-TMD3)  */

#define TDRA_ENP	AEBIT(0)	/* End of Packet	*/
#define TDRA_STP	AEBIT(1)	/* Start of Packet	*/
#define TDRA_DEF	AEBIT(2)	/* Deferred		*/
#define TDRA_ONE	AEBIT(3)	/* ONE retry needed	*/
#define TDRA_MORE	AEBIT(4)	/* MORE than one retry	*/
#define TDRA_RES	AEBIT(5)	/* RESERVED		*/
#define TDRA_ERR	AEBIT(6)	/* ERROR summary	*/
#define TDRA_OWN	AEBIT(7)	/* Owned by LANCE	*/

#define	TDRA_RTRY	AEBIT(10)	/* Retry Error		*/
#define	TDRA_LCAR	AEBIT(11)	/* Loss of Carrier	*/
#define	TDRA_LCOL	AEBIT(12)	/* Late Collision	*/
#define	TDRA_UFLO	AEBIT(14)	/* Underflow Error	*/
#define	TDRA_BUFF	AEBIT(15)	/* Buffer Error		*/
#define	TDRA_TDRMSK	0x03ff		/* mask for 10 bit tdr  */

struct ae_tdra {	/* Transmit Descriptor Table Entry  */
	UWORD	tbadr;	/* buffer address (low 16 bits)     */
	UBYTE	flags;	/* status bits                      */
	UBYTE	ha;	/* buffer address (high 8 bits)     */
	short	bcnt;	/* buffer byte count                */
	UWORD	errors;	/* error flags & 10 bit tdr value   */
};

/*  Control & Status Register 0 (CSR0)  */

#define CSR0		0

#define AE_INIT		AEBIT(0)	/* Initialize		*/
#define AE_STRT		AEBIT(1)	/* START (enable LANCE)	*/
#define AE_STOP 	AEBIT(2)	/* STOP (disable LANCE)	*/
#define AE_TDMD 	AEBIT(3)	/* Transmit Demand	*/
#define AE_TXON 	AEBIT(4)	/* Transmitter ON	*/
#define AE_RXON 	AEBIT(5)	/* Receiver ON		*/
#define AE_INEA 	AEBIT(6)	/* Interrupt Enable	*/
#define AE_INTR 	AEBIT(7)	/* Interrupt Flag	*/
#define AE_IDON 	AEBIT(8)	/* Initialization Done	*/
#define AE_TINT 	AEBIT(9)	/* Transmitter Intr.	*/
#define AE_RINT 	AEBIT(10)	/* Reveiver Interrupt	*/
#define AE_MERR 	AEBIT(11)	/* Memory Error		*/
#define AE_MISS 	AEBIT(12)	/* Missed Packet	*/
#define AE_CERR 	AEBIT(13)	/* Collision Error	*/
#define AE_BABL 	AEBIT(14)	/* Babble		*/
#define AE_ERR  	AEBIT(15)	/* Error Summary	*/

#define	AE_CLEAR	0xff00		/* mask to clear error & intr bits */

#define CSR1		1	/* IADR: Init. Block Adr (low 15 bits) */
#define CSR2		2	/* IADR: Init. Block Adr  (hi  8 bits) */

#define CSR3		3	/* Control & Status Register 3 (CSR3)  */

#define CSR3_BSWAP	AEBIT(2)	/* Byte Swap	*/
#define	CSR3_ACON	AEBIT(1)	/* ALE  Control	*/
#define	CSR3_BCON	AEBIT(0)	/* Byte Control	*/

/* Error codes */

#define	EINTR		1		/* Control-C entered	*/
#define	ETIMEDOUT	2		/* basic timeout 	*/
#define	ENETERR		3		/* error on send/recv	*/


