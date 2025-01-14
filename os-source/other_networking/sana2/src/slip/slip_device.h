/*
** $Source: HOG:Other/networking/sana2/src/slip/RCS/slip_device.h,v $
** $State: Exp $
** $Revision: 38.1 $
** $Date: 94/02/17 14:17:15 $
** $Author: kcd $
**
** SANA-II Example device driver C include file
**
** (C) Copyright 1992 Commodore-Amiga, Inc.
*/

#include <exec/types.h>
#include <exec/devices.h>
#include <exec/ports.h>
#include <exec/semaphores.h>
#include <exec/memory.h>
#include <dos.h>
#include <devices/sana2.h>
#include <devices/serial.h>
#include <utility/hooks.h>

#ifdef	OWNDEVUNIT_SUPPORT
#include <libraries/owndevunit.h>
#endif

/* Berkely Stuff */

#include "param.h"
#include "mbuf.h"
#include "in.h"
#include "in_systm.h"
#include "ip.h"
#include "tcp.h"
#include "string.h"
#include "slcompress.h"

/*
** Stacksize and Priority of the Unit Process
*/
#define SLIP_STACKSIZE 4000
#define SLIP_PRI 5

/*
** Maximum Transmission Unit
*/
#define DEFAULT_SLIP_MTU 1006

/*
** Max # of Units allowed
*/
#define SD_MAXUNITS 8

/*
** Message passed to the Unit Process at
** startup time.
*/
struct StartupMessage
{
	struct Message	Msg;
	struct Unit	*Unit;
	struct Device	*Device;
};

/*
** Device Data Structure
*/
struct SLIPDevice
{
    struct Library	 sd_Device;
    UBYTE		 sd_Flags;
    UBYTE		 sd_Pad1;
    struct Library	*sd_SysBase;
    struct Library	*sd_DOSBase;
    ULONG		 sd_DosTag[10];
    BPTR		 sd_SegList;
    struct Unit		*sd_Units[SD_MAXUNITS];
    struct SignalSemaphore sd_Lock;
    struct StartupMessage sd_Startup;
    struct Hook		 sd_DummyPFHook;
};

/*
** Typedef's for the SANA-II callback functions.
*/
typedef BOOL (*SANA2_CFB)(APTR to, APTR from, LONG length);
typedef BOOL (*SANA2_CTB)(APTR to, APTR from, LONG length);

struct BufferManagement
{
    struct MinNode	bm_Node;
    SANA2_CFB		bm_CopyFromBuffer;
    SANA2_CTB		bm_CopyToBuffer;
    struct Hook	       *bm_PacketFilterHook;	/* New SANA-II V2 Callback Hook */
    struct MinList	bm_RxQueue;		/* Pending CMD_READS */
};

struct SuperS2PTStats
{
    struct MinNode		ss_Node;
    ULONG			ss_PType;
    struct Sana2PacketTypeStats	ss_Stats;
};

struct DQInfo
{
    UWORD	Type;
    UWORD	Size;
    ULONG	Data;
};

#define DQ_CWORD	1
#define DQ_CLONG	2
#define DQ_MTU		3
#define DQ_BPS		4

/*
** Unit Data Structure
*/
struct SLIPUnit
{
    struct Unit			 su_Unit;		/* Standard Unit Structure */
    UBYTE			 su_UnitNum;		/* Unit number */
    UBYTE			 su_Reserved0;		/* Padding */
    struct Device		*su_Device;		/* Pointer to our device node */
    ULONG			 su_StAddr;		/* Our current "hardware" address */
    ULONG			 su_DefaultAddr;	/* Our default "hardware" address */
    ULONG			 su_BaudRate;		/* Serial baud rate */
    ULONG			 su_MTU;		/* Maximum Transmission Unit */
    LONG			 su_RBufLen;		/* Serial receive buffer size */
    ULONG			 su_SerUnitNum;		/* Serial driver unit number */
    BOOL			 su_Continue;
    BOOL			 su_Escape;		/* SLIP Decode state */
    ULONG			 su_State;		/* Various state information */
    struct IOExtSer 		*su_SerRx;		/* Serial IORequest for CMD_READ's */
    struct IOExtSer 		*su_SerTx;		/* Serial IORequest for CMD_WRITE's */
    struct MsgPort 		*su_RxPort;		/* Serial CMD_READ IORequest reply port */
    struct MsgPort 		*su_TxPort;		/* Serial CMD_WRITE IORequest reply port */
    UBYTE			*su_RxBuff;		/* Buffer for holding decoded packets */
    UBYTE			*su_RxBuffPtr;		/* Current place in decode buffer */
    UBYTE			*su_TxBuff;		/* Temporary buffer for hold unencoded outgoing packets */
    UBYTE			*su_RxSLIP;		/* Two buffers that hold packets encoded */
    UBYTE			*su_TxSLIP;		/* in SLIP format. */
    struct Process		*su_Proc; 		/* NB: This points to the Task, not the MsgPort */
    struct SignalSemaphore	 su_ListLock;		/* A Semaphore for access to all of our queues. */
    struct MsgPort		*su_Tx;			/* Pending CMD_WRITE's */
    struct MsgPort		*su_TxFast;		/* Low-Delay CMD_WRITE's */
    struct MinList	 	 su_Events;		/* Pending S2_ONEVENT's */
    struct SuperS2PTStats	*su_IPTrack;		/* For tracking IP packets */
    struct MinList	 	 su_Track;		/* List of packet types being tracked */
    struct MinList		 su_BuffMgmt;		/* List of Callback routines */
    struct Sana2DeviceStats	 su_Stats;		/* Global device statistics */
    UBYTE		 	 su_SerDevName[32];	/* Name of the serial driver to use */
#ifdef OWNDEVUNIT_SUPPORT
    struct Library		*su_ODUBase;		/* OwnDevUnit library base */
    UBYTE			 su_OwnerName[32];	/* name reported to ODU as owner of serial device */
#endif
    struct mbuf			 su_MBuff;
    struct slcompress		 su_SLCompress;
};

/*
** State bits for sdu_State
*/

#define SLIPUB_CONFIG		0
#define SLIPUB_ONLINE		1
#define SLIPUB_SETCONFIG	2
#define SLIPUB_CD		3
#define SLIPUB_EXCLUSIVE	4
#define SLIPUB_7WIRE		5
#define SLIPUB_MAYCOMPRESS	6
#define SLIPUB_AUTOCOMP		7

#define SLIPUF_CONFIG		(1<<SLIPUB_CONFIG)
#define SLIPUF_ONLINE		(1<<SLIPUB_ONLINE)
#define SLIPUF_SETCONFIG	(1<<SLIPUB_SETCONFIG)
#define SLIPUF_CD		(1<<SLIPUB_CD)
#define SLIPUF_EXCLUSIVE	(1<<SLIPUB_EXCLUSIVE)
#define SLIPUF_7WIRE		(1<<SLIPUB_7WIRE)
#define SLIPUF_MAYCOMPRESS	(1<<SLIPUB_MAYCOMPRESS)
#define SLIPUF_AUTOCOMP		(1<<SLIPUB_AUTOCOMP)

/*
** Type of Service Flags
*/

#define TOSF_LOWDELAY	0x10
#define TOSF_HITHRUPUT	0x04
#define TOSF_HIRELIABLE	0x02

/*
** Device Name
*/

#define SLIPDEVNAME "slip.device"

/*
** Packet Encoding Bytes
*/

#define SLIP_END     192
#define SLIP_ESC     219
#define SLIP_ESC_END 220
#define SLIP_ESC_ESC 221

/*
** Useful define
*/
#ifndef min
#define min(a,b) ((a)<=(b)?(a):(b))
#endif

/*
** Compiler Magic
**
*/

#define SysBase		(slipDevice->sd_SysBase)
#define DOSBase		(slipDevice->sd_DOSBase)
#define OwnDevUnitBase	(slipUnit->su_ODUBase)

#if 1
#define ASM           __asm
#define REG(x)        register __ ## x
#else
#define ASM
#define REG(x)
#endif

typedef ULONG (* ASM HOOK_FUNC)(REG(a0) struct Hook *hook, REG(a2) APTR object, REG(a1) APTR message);

#define STDSLIPARGS REG(a2) struct IOSana2Req *ios2, REG(a4) struct SLIPUnit *slipUnit, REG(a6) struct SLIPDevice *slipDevice
