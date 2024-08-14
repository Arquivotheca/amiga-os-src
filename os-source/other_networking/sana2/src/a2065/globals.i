**
** A2065 SANA-II Device Driver source
** Copyright (c) 1992 by Commodore-Amiga, Inc.
**
** All rights Reserved.
**


        include "utility/tagitem.i"
        include "utility/hooks.i"
        include "devices/sana2.i"
        include "devices/sana2specialstats.i"
        include "a2065stats.i"
        include "a2065.i"
        include "lvos.i"

        xref _LVONextTagItem
        xref _LVOMakeFunctions
	xref _LVOFindTagItem

Ether2 equ      ((((S2WIRETYPE_ETHERNET)&$ffff)<<16)|$0000)

_LVOCopyToBuff equ -12
_LVOCopyFromBuff equ -18
_LVOWantPacket equ -24

* temporary

TAG_COPYTOBUFF   equ (TAG_USER+$B0000+1)
TAG_COPYFROMBUFF equ (TAG_USER+$B0000+2)
TAG_PACKETFILTER equ (TAG_USER+$B0000+3)

	IFND DEBUG_MODE
;DEBUG_MODE	EQU	1
	ENDC

        TRASHREG        OFF

DEVPRI          EQU     0
MAX_BOARDS      EQU     5
HASH_SIZE       EQU     256
HASH_MASK       EQU     255
DELREG          EQU     $E80000

        IFD     MAIN_MODULE
        XDEF    InitReg
        ELSE
        XREF    InitReg
        ENDC

;----------------------------------------------------------------------------
;                               DEVICE_BASE
;----------------------------------------------------------------------------

        STRUCTURE MYLIB,LIB_SIZE
        ULONG   DeviceSegmentList
        ULONG   SysBase
        ULONG   ExpansionBase
        ULONG   UTBase
        ULONG   UnitCount
        STRUCT  MyInt,IS_SIZE
        STRUCT  UnitArray,MAX_BOARDS*4
        STRUCT  ActiveUnits,MLH_SIZE
        UBYTE   DeviceFlags
        UBYTE   DevicePadding
        LABEL   MYLIB_SIZE

        BITDEF  DF,INIT_OK,0    ; If set, means device is ``initialized''

;----------------------------------------------------------------------------
;                              UNIT STRUCTURE
;----------------------------------------------------------------------------

        STRUCTURE       MY_UNIT,MLN_SIZE
        APTR            UnitBoard
        APTR            UnitChip
        APTR            UnitMemory
        APTR            UnitRxDescriptor
        APTR            UnitTxDescriptor
        APTR            UnitTxBuffer
        APTR            UnitRxBuffer
        ULONG           UnitOpenCount
        ULONG           UnitTrackingCount
        ULONG           UnitNRD
        ULONG           UnitNWFill
        ULONG           UnitNWDone
        ULONG           UnitMCastRejects
        STRUCT          UnitCurrentAddress,6
        STRUCT          UnitDefaultAddress,6
        STRUCT          UnitLogicalAddress,8
        UBYTE           UnitFlags
        UBYTE           UnitINEACount
        STRUCT		UnitBMList,MLH_SIZE
        STRUCT          UnitProtocolHashTable,HASH_SIZE*MLH_SIZE
        STRUCT          Unit802PN,PN_SIZE
        STRUCT          UnitOrfPN,PN_SIZE
        STRUCT          UnitMultiCastList,MLH_SIZE
        STRUCT          UnitConfigList,MLH_SIZE
        STRUCT          UnitTxList,MLH_SIZE
        STRUCT          UnitTxRunningList,MLH_SIZE
        APTR            UnitConfigDev
        STRUCT          UnitEventList,MLH_SIZE

        LONG            MemoryOffset
        LONG            ChipOffset

        STRUCT          UnitSoftInt,IS_SIZE

        STRUCT          UnitStats,S2DS_SIZE
        LABEL           UNIT_SIZE_

        BITDEF          UF,PAV,7
        BITDEF          UF,OFFLINE,6
        BITDEF          UF,HALT_WRITES,5
        BITDEF          UF,WRITE_NEEDS_KICKSTART,4

        BITDEF          UF,PROM,1
        BITDEF          UF,MINE,0

;----------------------------------------------------------------------------
;                      MultiCast Address List Entry
;----------------------------------------------------------------------------

        STRUCTURE       MCN,MLN_SIZE
        STRUCT          MCN_Address,6
        ULONG           MCN_RxCount
        UBYTE           MCN_Flags
        UBYTE           MCN_Bit
        UWORD           MCN_OpenCount
        LABEL           MCN_SIZE

;----------------------------------------------------------------------------
;                             Protocol Nexus
;----------------------------------------------------------------------------

        STRUCTURE       PN,MLN_SIZE
        USHORT          PN_PACKETTYPE
        UBYTE           PN_FLAGS
        UBYTE           PN_PADDING
        STRUCT          PN_TYPESTATS,S2PTS_SIZE
        STRUCT          PN_RXQ,MLH_SIZE
        LABEL           PN_SIZE

        BITDEF          PN,TRACKING,0

;----------------------------------------------------------------------------
;                Bit definitions for the ON_EVENT classes
;----------------------------------------------------------------------------

s2eF_ERROR    equ S2EVENT_ERROR
s2eF_TX       equ S2EVENT_TX
s2eF_RX       equ S2EVENT_RX
s2eF_ONLINE   equ S2EVENT_ONLINE
s2eF_OFFLINE  equ S2EVENT_OFFLINE
s2eF_NETBUF   equ S2EVENT_BUFF
s2eF_HARDWARE equ S2EVENT_HARDWARE
s2eF_SOFTWARE equ S2EVENT_SOFTWARE

* a rig because we changed the include files again, and I'm tired
* of changing all the source constantly:
S2DS_RXPACKETS equ S2DS_PACKETSRECEIVED
S2DS_TXPACKETS equ S2DS_PACKETSSENT
S2DS_FRAMINGERRORS equ S2DS_BADDATA
S2DS_FIFOOVERRUNS equ  S2DS_OVERRUNS
S2DS_RXUNKNOWNTYPES equ S2DS_UNKNOWNTYPESRECEIVED
S2DS_HARDMISSES equ S2DS_OVERRUNS

S2ERR_UNKNOWN_ENTITY equ S2ERR_BAD_ARGUMENT





;----------------------------------------------------------------------------
;       Debugging Stuff
;       Comment out DEBUG_MODE to disable all debugging print outs.
;----------------------------------------------------------------------------
;

        IFD     DEBUG_MODE
        XREF    _kputstr,_kprintf
        ENDC

KPUTSTR MACRO
        IFD     DEBUG_MODE
        movem.l a0-a1/d0-d1,-(sp)
        move.l  #\1,-(sp)
        jsr     _kputstr
        addq.w  #4,sp
        movem.l (sp)+,a0-a1/d0-d1
        ENDC
        ENDM

KPRN1   MACRO
        IFD     DEBUG_MODE
        movem.l a0-a1/d0-d1,-(sp)
        move.l  \2,-(SP)
        pea     \1
        jsr     _kprintf
        add.w   #8,sp
        movem.l (sp)+,a0-a1/d0-d1
        ENDC
        ENDM

KPRN2   MACRO
        IFD     DEBUG_MODE
        movem.l a0-a1/d0-d1,-(sp)
        move.l  \3,-(SP)
        move.l  \2,-(SP)
        pea     \1
        jsr     _kprintf
        add.w   #12,sp
        movem.l (sp)+,a0-a1/d0-d1
        ENDC
        ENDM

KPRN3   MACRO
        IFD     DEBUG_MODE
        movem.l a0-a1/d0-d1,-(sp)
        move.l  \4,-(SP)
        move.l  \3,-(SP)
        move.l  \2,-(SP)
        pea     \1
        jsr     _kprintf
        add.w   #16,sp
        movem.l (sp)+,a0-a1/d0-d1
        ENDC
        ENDM


;----------------------------------------------------------------------------
;                           Miscellaneous Stuff
;----------------------------------------------------------------------------

NLIST   MACRO
        LEA     \1,A0
        NEWLIST A0
        ENDM

DELAY   MACRO
        tst.w   DELREG
        ENDM

;----------------------------------------------------------------------------
;                           New Stuff for SANA-II V2 Compliance
;----------------------------------------------------------------------------

    STRUCTURE	SQ,MLN_SIZE
	STRUCT		SQ_LIST,MLH_SIZE
	APTR		SQ_FUNCTABLE
	IFD	DEBUG_MODE
	APTR		SQ_TASKNAME
	ENDC
	LABEL		SQ_SIZE

    STRUCTURE	BMSTRUCT,0
	APTR		BMS_CopyToBuff
	APTR		BMS_CopyFromBuff
	APTR		BMS_PacketHook
	LABEL		BMS_SIZE
