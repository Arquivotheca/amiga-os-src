#ifndef	MPEG_DEV_H
#define MPEG_DEV_H	1

/*
** mpeg_device.h
**
** Internal device structures for mpeg_cdgs.device and
** mpeg_zorro.device.
**
** (C) 1993 Commodore-Amiga, Inc.
**
*/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/libraries.h>
#include <exec/ports.h>
#include <exec/devices.h>
#include <exec/semaphores.h>
#include <exec/interrupts.h>
#include <devices/timer.h>
#include <dos/dos.h>

#include "mpeg.h"
#include "cl450.h"
#include "l64111.h"

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

#define MD_MAX_UNIT	8
#define MPEG_PRI	9
/*
** Device Data Structure
*/
struct MPEGDevice
{
	struct Library	 md_Device;
	UBYTE		 md_Flags;
	UBYTE		 md_Pad1;
	struct Library	*md_SysBase;
	struct Library	*md_DOSBase;
	struct Library	*md_GfxBase;
	BPTR		 md_SegList;
	ULONG		 md_BoardType[MD_MAX_UNIT];
	struct Unit	*md_Units[MD_MAX_UNIT];
	struct ConfigDev	*md_Boards[MD_MAX_UNIT];
	struct SignalSemaphore	 md_Lock;
	struct timerequest	 md_TimerIO;
};

typedef struct MPEGDevice *MPEGDevicePtr;
typedef volatile UWORD HWREG;

/*
** Unit Data Structure
*/
struct MPEGUnit
{
	struct Unit	 mu_Unit;		/* Message Port, etc. */
	UBYTE		 mu_UnitNum;		/* Unit Number */
	UBYTE		 mu_ProductID;		/* Board Type */
	struct MPEGDevice *mu_Device;		/* Pointer to Device Base */
	HWREG		*mu_CL450Fifo;		/* CL450 Fifo Address */
	CL450Regs	*mu_CL450;		/* CL450 Base Address */
	L64111Regs	*mu_L64111;		/* L64111 Base Address */
	volatile UWORD	*mu_Control;		/* MPEG Board Control register */
	UWORD		 mu_ControlVal;         /* MPEG Board Control register value */
	UBYTE		 mu_HandleVidUnderflow; /* BOOL */
	UBYTE		 mu_HandleAudUnderflow;	/* BOOL */
	UWORD		 mu_CL450IntMask;	/* CL450 Interrupt Bit */
	UWORD		 mu_L64111IntMask;	/* L64111 Interrupt Bit */
	UWORD		*mu_CL450Dram;		/* CL450 DRAM location */
	UWORD		 mu_AudioID;		/* Audio stream ID */
	UWORD		 mu_PadZ;		/* Padding */
	UWORD		 mu_PlayFlags;		/* Current Play State */
	UWORD		 mu_SlowSpeed;		/* Slow Motion Speed */
	UWORD		 mu_StreamType;         /* Type of stream we're playing. */
	UBYTE		 mu_CL450IntAble;	/* Used by the interrupt routines */
	UBYTE		 mu_L64111IntAble;	/* Used by the interrupt routines */
	ULONG		 mu_CL450SigMask;	/* Used to inform the unit process of a CL450 interrupt  */
	ULONG		 mu_L64111SigMask;	/* Used to inform the unit process of a L64111 interrupt */
	ULONG		 mu_AbortSigMask;		/* Used to abort the current play command */
	ULONG		 mu_SyncSigMask;	/* Used to inform the unit process that sync has occured */
	ULONG		 mu_WaitMask;		/* Unit process wait mask */
	ULONG		 mu_PortSigMask;	/* Unit port signal mask */
	ULONG		 mu_CDSigMask;		/* cd.device signal mask */
	ULONG		 mu_DPSigMask;		/* internal data pump signal mask */
	UBYTE		 mu_CL450SigBit;	/* CL450 Signal bit number */
	UBYTE		 mu_L64111SigBit;	/* L64111 Signal bit number */
	UWORD		 mu_CFLevelBit;		/* CL450 CFLevel bit mask */
	UWORD		 mu_L64111Int1Bits;	/* L64111 Int1 Enable Bits */
	UWORD		 mu_L64111Int2Bits;	/* L64111 Int2 Enable Bits */
	UWORD		 mu_VideoSize;		/* Amount of data in current packet */
	UWORD		 mu_AudioSize;		/* Amount of data in current packet */
	UWORD		*mu_VideoData;		/* Video data */
	UWORD		*mu_AudioData;		/* Audio data */
	UBYTE		 mu_VidOverflowByte;	/* Extra Byte */
	UBYTE		 mu_IsVidOverflow;	/* Flag */
	UBYTE		 mu_AudOverflowByte;	/* Extra Byte */
	UBYTE		 mu_IsAudOverflow;	/* Flag */
	UBYTE		 mu_DPReqs;		/* Number of pending cd.device I/O requests */
	UBYTE		 mu_ValidPTS;
	UBYTE		 mu_PendingScan;
	UBYTE		 mu_DPState;		/* State of the cd.device data pump */
	UBYTE		 mu_CL450CmdSem;	/* CL450 Is being issued a command */
	UBYTE		 mu_SyncSigBit;		/* Signal Bit number for sync */
	UWORD		 mu_CL450IntBits;	/* CL450 Interrupt Enable Bits */
	LONG		 mu_ScanSpeed;		/* Slow motion/Scan speed */
	ULONG		 mu_StreamBegin;	/* Beginning if stream/track */
	ULONG		 mu_StreamOffset;	/* Where to play */
	ULONG		 mu_StreamEnd;		/* End of stream/track */
	ULONG		 mu_SectorSize;		/* Sector size to use */
	ULONG		 mu_CDOffset;		/* Current read offset */
	struct Device	*mu_CDDevice;		/* cd.device device base */
	struct Unit	*mu_CDUnit;		/* cd.device unit base */
	struct Library	*mu_SysBase;		/* Used by the interrupt routines */
	struct MinList	 mu_VideoStream;	/* Demux'd video data */
	struct MinList	 mu_AudioStream;	/* Demux'd audio data */
	struct MinList	 mu_CMDQueue;		/* Queued Microcode CMD requeusts */
	struct MinList	 mu_PlayQueue;		/* Queued Play requests */
	struct MinList	 mu_CDIOList;		/* Queue CD_READ commands */
	struct MinList	 mu_SeqHeadQueue;	/* Queued MPEGCMD_GETVIDEOPARAMS requests */
	UWORD		 mu_VPackets;		/* Number of packets in mu_VideoStream */
	UWORD		 mu_APackets;		/* Number of packets in mu_AudioStream */
	struct MsgPort	*mu_DPReplyPort;	/* Internal data pump reply port  */
	struct MsgPort	*mu_CDReplyPort;	/* cd.device reply port */
	struct IOMPEGReq *mu_VideoIO;		/* Currently processing IO request */
	struct IOMPEGReq *mu_AudioIO;		/* Currently processing IO request */
	struct IOMPEGReq *mu_ScanIO;		/* Currently executing MPEGCMD_SCAN request */
	struct IOStdReq  *mu_CDCmdIO;		/* cd.device command IO request */
	struct IOMPEGReq *mu_CurrentPlayCmd;	/* Currently executing MPEGCMD_PLAY request */
	struct SignalSemaphore	mu_Lock;	/* Semaphore for mu_VideoStream and mu_AudioStream */
	struct Task	*mu_Task;		/* Unit Process */
	ULONG		 mu_PicCount;		/* CL450 Picture Interrupt Count */
	struct Interrupt mu_Interrupt;		/* Unit Interrupt Routine */
	UBYTE		 mu_VPTS[128];		/* Timestamp Valid Flags  */
	ULONG		 mu_PTS[128];		/* Timestamps */
	UWORD		 mu_PTSHi;
	UWORD		 mu_PTSMid;
	UWORD		 mu_PTSLo;
	UWORD		 mu_PacketSize;
	UWORD		 mu_PacketBytes;
//	UWORD		 mu_LastPTSHi;
//	UWORD		 mu_LastPTSMid;
	UWORD		 mu_LastPTSLo;
	ULONG		 mu_LastPTSCheck;
	ULONG		 mu_LastEHigh;
	ULONG		 mu_LastELow;
	UBYTE		 mu_AbortSigBit;
	UBYTE		 mu_VEnable;
	UBYTE		 mu_AEnable;
};

#define MPEG_MANUFACTURER	0x202

#define MPEG_ZORRO2_ID		0x68
#define MPEG_CDGS_ID		0x6A

/*
** Device States
**
*/

#define MPEGSTATE_IDLE		0
#define MPEGSTATE_PLAYING	1
#define MPEGSTATE_SLOW		2
#define MPEGSTATE_SCANNING	3
#define MPEGSTATE_PAUSED	4
#define MPEGSTATE_STEPPING	5

#define MPEGPF_PLAY		1
#define MPEGPF_PAUSE		2
#define MPEGPF_SCAN		4
#define MPEGPF_SLOW		8
#define MPEGPF_SYNC		16
#define MPEGPF_INTMUTE		32
#define MPEGPF_EXTMUTE		64
#define MPEGPF_WAITSYNC		128

#define DPSTATE_IDLE		0
#define DPSTATE_FINDSEQHEADER	1
#define DPSTATE_RUNNING		2
#define DPSTATE_SHUTTLE		3
#define DPSTATE_HOLDING		4
#define DPSTATE_STOPPING	5

/*
** Control Register Bits
**
*/
#define Z2_USE_ADSP2105   0x8000
#define Z2_LSTARTI        0x4000
#define Z2_LHDMUTE        0x2000
#define Z2_MUTE           0x1000
#define Z2_DACINIT        0x0800
#define Z2_DACLATCH       0x0400
#define Z2_DACATT         0x0200
#define Z2_DACSHIFT       0x0100
#define Z2_DACSOC_MASK    0x00C0
#define Z2_CINTACK_N      0x0010
#define Z2_VIDEO_DISABLE  0x0004		/* When this bit is set MPEG video will be passed when
						   the output color is in the following color space
						   11xxxxxx111xxxxx011xxxxx. */
#define Z2_KEY1           0x0002
#define Z2_KEY0           0x0001



#define Z2_COND_GENLOCK   0x0003		/* The output genlock signal is the Amiga Pixel Switch
						   ANDed with the NOT of the lowest order bit of the high
						   high order nybble of the Amiga blue. */
#define Z2_PASS_GENLOCK   0x0002		/* The output genlock signal is the Amiga Pixel Switch. */
#define Z2_ALWAYS_GENLOCK 0x0001		/* The genlock signal is always asserted. */
#define Z2_NEVER_GENLOCK  0x0000		/* The genlock signal is never asserted. */

#define CD32_LHDMUTE	  0x0400
#define CD32_MUTE	  0x0200
#define CD32_DACEMPH	  0x0100
#define CD32_GENLOCK	  0x8000
#define CD32_MPEG_ENABLE  0x4000
#define CD32_CL450_RESET  0x2000
#define CD32_MAP_UMPEG	  0x1000

#define CD32_CINT_N	  0x8000
#define CD32_LINT_N	  0x4000
#define CD32_CFLEVEL	  0x0800
#define CD32_LFALF_N	  0x0400
#define CD32_LFALE_N	  0x0200
#define CD32_LFEMPTY_N	  0x0100

/*
** Internal Flags for IO requests
*/

#define MPEGB_VALID_SCR		30	/* Valid SCR value to be used? */

#define MPEGF_VALID_SCR		(1L << MPEGB_VALID_SCR)

/*
** Useful Macros for register manipulation.
*/

/* Used when you only have the unit pointer handy */

#define WriteCL450Register(reg, val)	mpegUnit->mu_CL450->reg=val
#define ReadCL450Register(reg)		mpegUnit->mu_CL450->reg
#define WriteL64111Register(reg, val)   mpegUnit->mu_L64111->reg=val
#define ReadL64111Register(reg)		mpegUnit->mu_L64111->reg

/* Used when you have the chip's base pointer handy */

#define WrCL450Register(reg, val)	cl450Regs->reg=val
#define RdCL450Register(reg)		cl450Regs->reg
#define WrL64111Register(reg, val)	l64111Regs->reg=val
#define RdL64111Register(reg)		l64111Regs->reg

/* Other misc macros */

#define iomr_DataStart	iomr_Private1
#define	iomr_DataSize	iomr_Private2

#define iomr_SCRHigh	iomr_Private3
#define iomr_SCRMid	iomr_Private4
#define iomr_SCRLow	iomr_Private5

#define SysBase	mpegDevice->md_SysBase

#define ResetL64111	InitL64111
#define SoftResetL64111	InitL64111

#define WriteCL450Dram(addr, val)	*(UWORD *)((ULONG)mpegUnit->mu_CL450Dram+addr)=val
#define ReadCL450Dram(addr)	*(volatile UWORD *)((ULONG)mpegUnit->mu_CL450Dram+addr)
#define PerformIO(ioReq) (*mpegCmdDispatch[((struct IORequest *)ioReq)->io_Command])(ioReq,(struct MPEGUnit *)((struct IORequest *)ioReq)->io_Unit);

/* Keep the interrupt handler and other tasks from playing with the CL450 */

#if 0
#define StartCL450Cmd()		Forbid(); mpegUnit->mu_CL450CmdSem++;
#define EndCL450Cmd()		Permit(); mpegUnit->mu_CL450CmdSem--;
#endif
#if 1
#define StartCL450Cmd()		Disable(); mpegUnit->mu_CL450CmdSem++;
#define EndCL450Cmd()		Enable();  mpegUnit->mu_CL450CmdSem--;
#endif
/* Rude versions */
// #define StartCL450Cmd()		Disable();
// #define EndCL450Cmd()		Enable();

#define ASM __asm
#define REG(x)        register __ ## x

extern int __builtin_min(int, int);

#ifndef min
#define min(a,b) __builtin_min(a,b)
#endif

extern VOID kprintf(STRPTR fmt,	...);

#endif	/* MPEG_DEV_H */
