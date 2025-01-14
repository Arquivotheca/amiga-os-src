**
** mpeg_device.i
**
** Internal device structures for mpeg_cdgs.device and
** mpeg_zorro.device.
**
** (C) 1993 Commodore-Amiga, Inc.
**
*/

		INCDIR	"include:"

		INCLUDE "exec/types.i"
		INCLUDE "exec/nodes.i"
		INCLUDE "exec/libraries.i"
		INCLUDE "exec/ports.i"
		INCLUDE "exec/devices.i"
		INCLUDE "exec/semaphores.i"
		INCLUDE "exec/interrupts.i"
		INCLUDE "devices/timer.i"
		INCLUDE "mpeg.i"

MD_MAX_UNIT	EQU	8

**
** Device Data Structure
**
**
	STRUCTURE MPEGDevice,LIB_SIZE
		UBYTE	md_Flags
		UBYTE	md_Pad1
		APTR	md_SysBase
		APTR	md_DOSBase
		APTR	md_GfxBase
		APTR	md_SegList
		STRUCT	md_BoardType,4*MD_MAX_UNIT
		STRUCT	md_Units,4*MD_MAX_UNIT
		STRUCT	md_Boards,4*MD_MAX_UNIT
		STRUCT  md_Lock,SS_SIZE
		STRUCT  md_TimerIO,IOTV_SIZE

		LABEL	MPEGDev_Sizeof

**
** Unit Data Structure
**
**
	STRUCTURE MPEGUnit,UNIT_SIZE
		UBYTE	mu_UnitNum
		UBYTE	mu_ProductID
		APTR	mu_Device
		APTR	mu_CL450Fifo
		APTR	mu_CL450
		APTR	mu_L64111
		APTR	mu_Control
		UWORD	mu_ControlVal
		UBYTE	mu_HandleVidUnderflow;
		UBYTE	mu_HandleAudUnderflow;
		UWORD	mu_CL450IntMask
		UWORD	mu_L64111IntMask
		APTR	mu_CL450Dram
		ULONG	mu_State
		UWORD	mu_PlayFlags
		UWORD	mu_SlowSpeed
		UWORD	mu_StreamType
		UBYTE	mu_CL450IntAble
		UBYTE	mu_L64111IntAble
		ULONG	mu_CL450SigMask
		ULONG	mu_L64111SigMask
		ULONG	mu_AbortMask
		ULONG	mu_SyncSigMask
		ULONG	mu_WaitMask
		ULONG	mu_PortSigMask
		ULONG	mu_CDSigMask
		ULONG	mu_DPSigMask
		UBYTE	mu_CL450SigBit
		UBYTE	mu_L64111SigBit
		UWORD	mu_CFLevelBit
		UWORD	mu_L64111Int1Bits
		UWORD	mu_L64111Int2Bits
		UWORD	mu_VideoSize
		UWORD	mu_AudioSize
		APTR	mu_VideoData
		APTR	mu_AudioData
		UBYTE	mu_VidOverflowByte
		UBYTE	mu_IsVidOverflow
		UBYTE	mu_AudOverflowByte
		UBYTE	mu_IsAudOverflow
		UBYTE	mu_DPReqs
		UBYTE	mu_ValidPTS
		UBYTE	mu_PendingScan
		UBYTE	mu_DPState
		UBYTE	mu_CL450CmdSem
		UBYTE	mu_SyncSigBit
		UWORD	mu_CL450IntBits
		LONG	mu_ScanSpeed
		ULONG	mu_StreamBegin
		ULONG	mu_StreamOffset
		ULONG	mu_StreamEnd
		ULONG	mu_SectorSize
		ULONG	mu_CDOffset;
		APTR	mu_CDDevice
		APTR	mu_CDUnit
		APTR	mu_SysBase
		STRUCT	mu_VideoStream,MLH_SIZE
		STRUCT	mu_AudioStream,MLH_SIZE
		STRUCT	mu_CMDQueue,MLH_SIZE
		STRUCT  mu_PlayQueue,MLH_SIZE
		STRUCT  mu_CDIOList,MLH_SIZE
		STRUCT  mu_SeqHeadQueue,MLH_SIZE
		UWORD	mu_VPackets
		UWORD	mu_APackets
		APTR	mu_DPReplyPort
		APTR	mu_CDReplyPort
		APTR	mu_VideoIO
		APTR	mu_AudioIO
		APTR	mu_ScanIO
		APTR	mu_CDCmdIO
		APTR	mu_CurrentPlayCmd
		STRUCT	mu_Lock,SS_SIZE
		APTR	mu_Task
		ULONG	mu_PicCount
		STRUCT  mu_Interrupt,IS_SIZE
		STRUCT	mu_VPTS,128
		STRUCT	mu_PTS,128*4
		UWORD	mu_PTSHi
		UWORD	mu_PTSMid
		UWORD	mu_PTSLo
		UWORD	mu_PacketSize
		UWORD	mu_PacketBytes
;		UWORD	mu_LastPTSHi
;		UWORD	mu_LastPTSMid
		UWORD	mu_LastPTSLo
		ULONG	mu_LastPTSCheck
		ULONG	mu_LastEHigh
		ULONG	mu_LastELow
		UBYTE	mu_AbortSigBit
		UBYTE	mu_VEnable
		UBYTE	mu_AEnable
		LABEL	MPEGUnit_Sizeof

iomr_DataStart	EQU	iomr_Private1
iomr_DataSize	EQU	iomr_Private2
iomr_SCRHigh	EQU	iomr_Private3
iomr_SCRMid	EQU	iomr_Private4
iomr_SCRLow	EQU	iomr_Private5

**
** Flag definitions
**
	BITDEF	MPEG,VALID_SCR,30

**
** Device Name Macros
**

MPEGDEVNAME	MACRO
		DC.B	'cd32mpeg.device',0
		ENDM

**
** Handy system call macro
**

jsrlib		MACRO
		IFND	_LVO\1
		XREF	_LVO\1
		ENDC
		jsr	_LVO\1(a6)
		ENDM

