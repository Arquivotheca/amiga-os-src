/*
 * rs485 device specs
 */

#ifndef RS485_H
#define RS485_H

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/resident.h>
#include <exec/libraries.h>
#include <exec/devices.h>
#include <exec/memory.h>
#include <exec/semaphores.h>
#include <exec/errors.h>
#include <exec/interrupts.h>
#include <hardware/intbits.h>
#include <libraries/configvars.h>
#include <libraries/expansion.h>
#include <libraries/dos.h>

#define ABSEXECBASE (*(struct Library **)4L)
#define SysBase ABSEXECBASE

#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>
#include <clib/expansion_protos.h>
#include <pragmas/expansion_pragmas.h>
#include <clib/utility_protos.h>
#include <pragmas/utility_pragmas.h>
#include <clib/alib_protos.h>

#include <string.h>

#include <devices/sana2.h>

/*
 * rs485 device structure - basic management variables.
 */
struct rs485Device {
        struct       Library ml_Lib;
        ULONG        ml_SegList;
        ULONG        ml_Flags;
        APTR         ml_ExecBase;
        LONG         ml_Data;
};

/*
 * rs485Unit structure - one allocated per board.
 */

#define UNIT(iob) ((struct rs485Unit *) ((iob)->ios2_Req.io_Unit))
#define NBUF		4		/* number of com20020 buffers	*/

struct rs485Unit
{
	struct Node node;		/* for internal unit list	*/
	struct rs485Device *libbase;	/* for interrupt/task		*/

	struct Task *task;		/* unit task pointer		*/
	struct Interrupt *intr;		/* int server			*/
	char   temp[NBUF][512];		/* mirrors on-chip buffers	*/
	UBYTE  	netbit;			/* signal to send to task	*/
	UBYTE	state;			/* initialized			*/
#define ONLINE		1		/* board is active		*/
#define OFFLINE		2		/* software ignores board	*/
#define INITIAL		3		/* board in initial state	*/

	struct SignalSemaphore sem;	/* protects addr/data port 	*/
	struct Sana2DeviceStats stats;	/* device statistics of unit	*/
	struct List trackedtype;	/* list of tracked types	*/
	struct List read_q;		/* read/readorphan requests	*/
	struct List write_q;		/* write/broadcast requests	*/
	struct List misc_q;		/* misc blockable requests	*/

	UBYTE 	*base;			/* base address of board	*/
	ULONG	flags;			/* flags from OpenDevice()	*/
	UBYTE	unitnum;		/* unit number, for LibOpen()	*/
	UBYTE	opencnt;		/* number of processes using us	*/
	UBYTE	intmask;		/* copy of chip interrupt mask	*/
	UBYTE	myaddr;			/* address from application	*/

	UBYTE	bstate[NBUF];		/* state of each com20020 buffer*/
#define IS_FREE		0		/* buffer is free		*/
#define HAS_XMIT_DATA	1		/* buffer has pending xmit data	*/
#define HAS_RCV_DATA	2		/* has rcvr data for copyout	*/
#define IS_CURR_XMIT	3		/* is current transmit buffer	*/
#define IS_CURR_RCVR	4		/* is current receiver buffer	*/
#define IS_BUSY		5		/* is busy building buffer	*/
};

/*
 * chip related definitions
 */
#define SR(up) 		((up)->base[0])	/* status/command register	*/
#define DSR(up)		((up)->base[4])	/* diagnostic status register	*/
#define ADDRH(up)	((up)->base[8])	/* addr high/control bits	*/
#define ADDRL(up)	((up)->base[12])/* low order addr bits		*/
#define DATA(up)	((up)->base[16])/* data register		*/
#define CFG(up)		((up)->base[24])/* configuration register	*/
#define SUB_REG(up)	((up)->base[28])/* nodeid/ten nodeid/control	*/
#define CMD(up)		DSR(up)		/* command reg is alias of DSR()*/
#define SETINTMASK(np, v) SR(np) = np->intmask = (v)

/* address of registers that appear in SUB_REG */
#define SUB_TENATIVE_ID	0		/* sub addr of tenative node ID	*/
#define SUB_NODE_ID	1		/* sub addr of node ID		*/
#define SUB_SETUP	2		/* sub addr of setup register	*/

/* COMMANDS */
#define C_SEND(n)	(3 | ((n)<<3))	/* send page n			*/
#define C_RECV(n)	(0x84|((n)<<3))	/* recv to page n		*/
#define C_CLR_RECON	(0x16)		/* clear recon status bit	*/
#define C_CLR_POR	(0x0e)		/* clear por status bit		*/
#define C_CONFIG	(0x0d)		/* configure for long packets	*/
#define C_DIS_XMIT	(0x01)		/* disable transmitter		*/
#define C_DIS_RCVR	(0x02)		/* disable receiver		*/
#define C_CLR_FLAGS	(0x1e)		/* clear RECON/POR flags	*/

#ifndef BIT
#define BIT(n) (1<<(n))
#endif

#define SR_RI		BIT(7)
#define SR_POR		BIT(4)
#define SR_TEST		BIT(3)
#define SR_RECON	BIT(2)
#define SR_TMA		BIT(1)
#define SR_TA		BIT(0)
#define DSR_MYRECON	BIT(7)
#define DSR_DUPID	BIT(6)
#define DSR_RCVACT	BIT(5)
#define DSR_TOKEN	BIT(4)
#define DSR_EXCNAK	BIT(3)
#define DSR_TENTID	BIT(2)
#define ADH_RDDATA	BIT(7)
#define ADH_AUTOINC	BIT(6)
#define CFG_RESET	BIT(7)
#define CFG_CCHEN	BIT(6)
#define CFG_TXEN	BIT(5)
#define CFG_ET1		BIT(4)
#define CFG_ET2		BIT(3)
#define CFG_BACKPLANE	BIT(2)
#define CFG_SUBAD1	BIT(1)
#define CFG_SUBAD0	BIT(0)
#define NDID_CKP1	BIT(2)
#define NDID_CKP0	BIT(1)
#define NDID_ARBSEL	BIT(0)

/*
 * Amiga list handling support
 */
#define ISEMPTY(lp) ((lp) == (struct List *)(lp)->lh_TailPred)
#define FORALL(lp, np, t)\
		for((np)=(t)((lp)->lh_Head); \
			((struct Node *)(np))->ln_Succ; \
			(np)=(t)((struct Node *)(np))->ln_Succ)

/*
 * constants used for determining location of board in address space
 */
#define CBM_PROD	514		/* Commodore MFG number		*/
#define RS485_PROD	117		/* product number of A2000 board*/
#define A500_BASE	0xd90001	/* base address of A500 board	*/

/*
 * command processing
 */
#define QUEUED	1			/* no reply			*/
#define COMPLETE 2			/* operation completed		*/

/*
 * event support
 */
#define WAKEUP(up, events) \
	if(!ISEMPTY(&(up)->misc_q)) wakeup((up), (events));

/*
 * Packet formatting support:
 * Max packet size is 508 bytes: 512 - (DID + SID + zero byte + count byte)
 * To allow for protocol multiplexing, there must be a type field in the
 * packet - this is normally one byte, but the spec calls for two byte codes
 * in certain cases.  We thus advertise an MTU of 508 - 2 = 506 bytes to the
 * protocol stacks.
 */
#define RS485_MTU 	504
#define BROADCASTADDR	0
#define TWOBYTECODE(php) (((php)->type[0] >= 128) && ((php)->type[0] <= 191))
struct pkthdr {
	UBYTE	sid;			/* source identifier of packet	*/
	UBYTE	did;			/* dest identifier of packet	*/
	UBYTE	type[2];		/* packet type data		*/
};

/*
 * TrackedType structure - one allocated per tracked type
 */
struct TrackedType {
	struct Node node;		/* for internal unit list	*/
	ULONG type;			/* packet type			*/
	struct Sana2PacketTypeStats stats;/* per type statistics	*/
};

/*
 * functions.h - prototypes for all external functions.
 */
#include "functions.h"

/*
 * global variables defined in global.c
 */
extern struct Library *UtilityBase;
extern struct Library *ExpansionBase;
extern struct List rs485Units;

/*
 * blink defined variables
 */
extern char __far _LibName[];
extern char __far _LibID[];

/*
**  Buffer Management Functions
*/
typedef BOOL (*buffnc)(VOID *, VOID *, ULONG);
struct buffmgmt
{
	buffnc CopyToBuff;
	buffnc CopyFromBuff;
};
BOOL __saveds __asm CopyToBuff(register __a0 VOID *to,
				register __a1 VOID *from,
				register __d0 ULONG n);
BOOL __saveds __asm CopyFromBuff(register __a0 VOID *to,
				register __a1 VOID *from,
				register __d0 ULONG n);
#define CopyToBuff(x,y,z)   ( (* ((struct buffmgmt *)iob->ios2_BufferManagement)->CopyToBuff)(x, y, z) )
#define CopyFromBuff(x,y,z) ( (* ((struct buffmgmt *)iob->ios2_BufferManagement)->CopyFromBuff)(x, y, z) )

/*
 * misc useful stuff
 */
#ifndef min
#define min(a,b) ((a) < (b)? (a):(b))
#endif

#ifdef DEBUG
#include <clib/alib_protos.h>
#define DPRINT(X) KPrintF X
#else
#define DPRINT(X)
#endif

#endif
