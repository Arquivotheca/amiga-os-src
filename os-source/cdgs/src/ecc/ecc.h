/* ================================================================================= */
/*
 * $Id: ecc.h,v 1.2 93/02/10 09:37:43 havemose Exp Locker: havemose $
 *
 * Decoding of Cross Interleaved Reed Solomon codes as used in CDROM.
 * Created: Dec 1, 1992, Allan Havemose
 * See ecc.doc for technical documentation.
 */
/* ================================================================================= */
/* ecc.h  */

#include	<exec/types.h>

/* --- misc defines related to the decoding algorithm -- */
#define	ROWNUM	28		/* number of rows in data matrix. Only used in main.c  */
#define	COLNUM	43		/* ditto colums. only used in main.c		 */

#define	IRRPOL	0x1C		/* irriducible polynomial */

#define	P_ECC_PROB	0x100	/* P-ECC not able to correct data */
#define	Q_ECC_PROB	0x100	/* Q-ECC not able to correct data */

#define	TRY_MAX	5		/* maximum number of ECC tries */

#define	DOWNCODEx

/* --- return values from Decode_CDECC() function --- */

#define	ECC_OK		0	/* No errors or all errors corrected */
#define	ECC_ERROR	1	/* Unable to correct all errors */
#define	ECC_HEADER_ERROR 2	/* uncorrecable error in CDROM header */

/* --- Layout of data in Buffer --- */
/* Layout:
 *	Addr	Size	Descript
 *	x000	12	Status Word
 *	x00C	4	CDROM header.
 *	x010	2336	Start of CDROM data.
 *			DMA'ed into 0x10 through 0x92F
 *
 *	xC00		Start of C2P0 data.
 *
 *	At the same time that the CDROM data is read in, the C2P0 information
 * 	is also read into a bit map starting at xC00. The low order bit
 *	at xC00 corresponds to the first sync byte, and bits continue
 *	upwards in 32 bit chunks. There is one bit for each CDROM byte,
 *	and therefore there should be 2352/32 = 73.5 words of C2P0
 *	information. Only the 73 whole words are DMAd here.  Note that
 *	the alignement has been carefully controlled to cause bit positions
 *	in this region to naturally point to byte positions at an offset
 *	of 0 from the start of the sector buffer. ( 0x00 ).
 *
 */

#define	SYNC_START		0x0	/* byte offset */
#define	HEADER_START		0xC
#define	DATA_START		0x10
#define	PARITYBITS_START	0x80c
#define	C2PO_START		0xc04
#define	C2PO_LEN		(4*74)	/* size of C2PO-data in bytes */
#define	DATA_SIZE		2352	/* size of all ecc data excl c2ptr */

#define	SECTOR_LEN		4096	/* total sector length incl. c2po's*/

#define	STATUS_START	0x2

#define	ECCDATA_START	HEADER_START	/* No ECC on the 12 sync bytes */

/* SYNC Mark is 12 bytes long with first and last byte '0' and
 * all other bytes 0xFF.
 */

#define	SYNC_MARK_0	0x00FFFFFF	/* first sync-mark long word */


/* --- run time statistics --- */
/* when RSTAT is defines runtime statistics in collected */
#define	RSTATx


/* --- cached version of algorithm --- */
/* use the new cache version of the algorithm --- */
#define	CACHE_CALC


/* --- debugging --- */
/* defining DEBA activates detailed diagnostics in 'ecc.c'. */

#define	DEBAx

#ifdef	DEBA
#define	DEB(x)	(x);
#else
#define	DEB(x)
#endif

#define	DEB2x		/* DEB2 given more detailed information */

/* --- include files --- */
#include	"stdio.h"
#include	"ecc_protos.h"