**
** A2065 SANA-II Device Driver source
** Copyright (c) 1992 by Commodore-Amiga, Inc.
**
** All rights Reserved.
**



;DEBUG_MODE     EQU     1
;DEBUG_COMMANDS EQU     1

        nolist

        include "includes.asm"
        include "globals.i"
        include "a2065_defs.i"

        list
        XDEF    BeginIO,TermIO,AbortIO,AbAll
        XREF    AddMult,DelMult,GetTime,ConfigI
        XREF    Write,BCast,MCast,Read,Track,UnTrack,TypeStats
        XREF    LocPN,HaltInterface,CnfgBd,TrmEvnt

        INT_ABLES

MIN_DQ_SIZE     EQU     8

CmdTbl  dc.l    Invalid         ;       0  Invalid              DONE
        dc.l    Invalid         ;       1  Reset                DONE
        dc.l    Read            ;       2  Read                 DONE
        dc.l    Write           ;       3  Write                DONE
        dc.l    Invalid         ;       4  Update               DONE
        dc.l    Invalid         ;       5  Clear                DONE
        dc.l    Invalid         ;       6  Stop                 DONE
        dc.l    Invalid         ;       7  Start                DONE
        dc.l    Flush           ;       8  Flush                DONE
        dc.l    DvQuery         ;       9  DeviceQuery          DONE
        dc.l    GetAddr         ;       10 GetStationAddress    DONE
        dc.l    ConfigI         ;       11 ConfigInterface      DONE
        dc.l    AdAlias         ;       12 AddStationAlias      DONE
        dc.l    DlAlias         ;       13 DelStationAlias      DONE
        dc.l    AddMult         ;       14 AddMultiCastAddress  DONE
        dc.l    DelMult         ;       15 DelMultiCastAddress  DONE
        dc.l    MCast           ;       16 MultiCast            DONE
        dc.l    BCast           ;       17 BroadCast            DONE
        dc.l    Track           ;       18 TrackType            DONE
        dc.l    UnTrack         ;       19 UnTrackType          DONE
        dc.l    TypeStats       ;       20 GetTypeStats         DONE
        dc.l    Speshul         ;       21 GetSpecialStats      DONE
        dc.l    GlbStts         ;       22 GetGlobalStats       DONE
        dc.l    OnEvent         ;       23 OnEvent              DONE TESTING
        dc.l    Read            ;       24 ReadOrphan           DONE
        dc.l    OnLine          ;       25 OnLine               DONE
        dc.l    OffLine         ;       26 OffLine              DONE

        IFD     DEBUG_COMMANDS

        dc.l    Invalid         ;       27
        dc.l    Invalid         ;       28
        dc.l    Invalid         ;       29
        dc.l    Invalid         ;       30
        dc.l    Invalid         ;       31
        dc.l    Invalid         ;       32
        dc.l    Invalid         ;       33
        dc.l    Invalid         ;       34
        dc.l    Invalid         ;       35
        dc.l    Invalid         ;       36
        dc.l    TestTmr         ;       37 TestTmr              DONE
        dc.l    ReadCSR         ;       38 ReadCSR              DONE

MAXCMD  equ     38
        ENDC

        IFND    DEBUG_COMMANDS
MAXCMD  equ     26
        ENDC

        CNOP    2

;----------------------------------------------------------------------------
; BeginIO - All device COMMANDS come here for dispatching.
;----------------------------------------------------------------------------
; IN:           A6      DeviceBase
;               A1      IOB
;
; USAGE:        A6      SysBase
;               A5      DeviceBase
;               A4      IOB
;               A3      Unit
;----------------------------------------------------------------------------
;----------------------------------------------------------------------------

BeginIO movem.l A2-A6/D2-D7,-(SP)
        bsr     InitReg

        moveq   #0,D0
        move.b  D0,IO_ERROR(A4)

        IFD	DEBUG_MODE
        suba.l	a1,a1
        CALL	FindTask		;Who the hell is this, anyway?
        movea.l	d0,a0
        movea.l	LN_NAME(a0),a0
        moveq.l	#0,D0
        ENDC

        move.w  IO_COMMAND(A4),D0

	IFD	DEBUG_MODE
	KPRN2	Msg1,d0,a0
	ENDC

        move.w  IO_COMMAND(A4),D0

        cmp.w   #MAXCMD,D0
        ble.s   10$

        ;       command is not in range

        move.b  #IOERR_NOCMD,IO_ERROR(A4)
        bsr.s   TermIO
        bra.s   99$

10$     ;       command is in range

        asl.l   #2,D0
        move.l  #CmdTbl,A0
        move.l  0(A0,D0.l),A0

        jsr     (a0)

99$
        movem.l (SP)+,A2-A6/D2-D7
        rts


;----------------------------------------------------------------------------
; TermIO - Terminate the IO Request Block pointed to by A4
;----------------------------------------------------------------------------
; IN:           A6      SysBase
;               A4      IO Request Block
; USAGE:        STANDARD FRAGILE REGISTERS
;----------------------------------------------------------------------------

TermIO  btst    #IOB_QUICK,IO_FLAGS(A4)
        bne.s   99$
        move.l  A4,A1
        CALL    ReplyMsg
99$     rts

;----------------------------------------------------------------------------
; Invalid - Executed when a non-existent command is requested
;----------------------------------------------------------------------------
; IN:           A6      SysBase
;               A4      IO Request Block
; USAGE:        STANDARD FRAGILE REGISTERS
;----------------------------------------------------------------------------

Invalid move.b  #IOERR_NOCMD,IO_ERROR(A4)
        bra.s   TermIO

;----------------------------------------------------------------------------
; AdAlias - AddStationAlias Command - Not Implemented
;----------------------------------------------------------------------------
; IN:           A6      SysBase
;               A5      DeviceBase
;               A4      IO Request Block
;               A3      UnitBase
; USAGE:        NO ADDITIONAL REGISTERS
;----------------------------------------------------------------------------

AdAlias move.b  #S2ERR_NO_RESOURCES,IO_ERROR(A4)
*        move.l  #S2WERR_ALIAS_LIST_FULL,IOS2_WIREERROR(A4)
        bra.s   TermIO

;----------------------------------------------------------------------------
; DlAlias - DelStationAlias Command - Not Implemented
;----------------------------------------------------------------------------
; IN:           A6      SysBase
;               A5      DeviceBase
;               A4      IO Request Block
;               A3      UnitBase
; USAGE:        NO ADDITIONAL REGISTERS
;----------------------------------------------------------------------------

DlAlias move.b  #S2ERR_UNKNOWN_ENTITY,IO_ERROR(A4)
        clr.l   IOS2_WIREERROR(A4)
        bra.s   TermIO

;----------------------------------------------------------------------------
; GetAddr - Fetches the default and current station addresses
;----------------------------------------------------------------------------
; IN:           A6      SysBase
;               A5      DeviceBase
;               A4      IO Request Block
;               A3      UnitBase
; USAGE:
;----------------------------------------------------------------------------
; Notes:
;
; This command causes the device driver to copy the current interface
; address into io_SrcAddr and to copy the factory default station
; address into io_DstAddr.
;
; If the physical address of this unit has not been defined yet (by
; call to CONFIGINTERFACE) then the current interface address will be
; returned as 0xFFFFFFFFFFFF (which is an illegal Ethernet station
; address since it corresponds to the BROADCAST address).
;
; The implementation of this command does assume that the io_Address
; fields are word aligned. This is a safe assumption since the thing
; which comes before the io_Address fields is a pointer and MAX_ADDR_BYTES
; is even.
;----------------------------------------------------------------------------

GetAddr ; I know I can copy the factory address.

        move.l  UnitDefaultAddress(A3),IOS2_DSTADDR(A4)
        move.w  UnitDefaultAddress+4(A3),IOS2_DSTADDR+4(A4)

        ; Does the unit have a current physical address?

        btst.b  #UFB_PAV,UnitFlags(A3)
        beq.s   10$

        ; Yes it does, copy that over too.

        move.l  UnitCurrentAddress(A3),IOS2_SRCADDR(A4)
        move.w  UnitCurrentAddress+4(A3),IOS2_SRCADDR+4(A4)

        bra.s   20$

10$     moveq.l #-1,D0
        move.l  D0,IOS2_SRCADDR(A4)
        move.w  D0,IOS2_SRCADDR+4(A4)

20$     bra.s   TermIO

;----------------------------------------------------------------------------
; DvQuery - DeviceQuery gets information about this network type
;----------------------------------------------------------------------------
; IN:           A6      SysBase
;               A5      DeviceBase
;               A4      IO Request Block
;               A3      UnitBase
; USAGE:        A2      IOW_STATS(A4)
;               D2      Size Of Buffer
;----------------------------------------------------------------------------
; Notes:
;
; This command is supposed to return a lot of neato information in the
; memory pointed to by IOW_STATS. The user tells us how much room he
; has set aside for us by filling in DQ_SIZE_AVAILABLE. When we are
; done, we tell him how much we filled in by setting DQ_SIZE_SUPPLIED.
; Under no circumstances should DQ_SIZE_SUPPLIE exceed DQ_SIZE_AVAILABLE.
;----------------------------------------------------------------------------

DvQuery movem.l A2/D2,-(SP)
        move.l  IOS2_STATDATA(A4),D0
        bne.s   10$

        move.l  #S2WERR_NULL_POINTER,IOS2_WIREERROR(A4)

0$      move.b  #S2ERR_BAD_ARGUMENT,IO_ERROR(A4)
        bsr.s   TermIO
        bra.s   99$

10$     ; There is something in IOW_STATS...do some sanity checks on it.

        move.l  #S2WERR_BAD_STATDATA,IOS2_WIREERROR(A4)

        move.l  D0,A2
        move.l  S2DQ_SIZEAVAILABLE(A2),D0
        cmp.l   #MIN_DQ_SIZE,D0
        blt.s   0$
        cmp.l   #S2DQ_SIZE,D0
        bgt.s   0$

        ; The S2DQ_SIZEAVAILABLE (still in D0) seems fine.

        clr.l   IOS2_WIREERROR(A4)

        move.l  D0,D2
        subq.l  #MIN_DQ_SIZE,D0
        beq.s   30$

        lea     DevData,A0
        lea     S2DQ_FORMAT(A2),A1
        CALL    CopyMem

30$     move.l  D2,S2DQ_SIZESUPPLIED(A2)

99$     movem.l (SP)+,A2/D2
        rts

DevData dc.l    0                       ; DQ_FORMAT
        dc.l    0                       ; DQ_LEVEL
        dc.w    48                      ; DQ_ADDRESS_SIZE
        dc.l    1500                    ; DQ_MTU
        dc.l    10000000                ; DQ_BPS
        dc.l    S2WIRETYPE_ETHERNET     ; DQ_HARDWARE_TYPE


;----------------------------------------------------------------------------
; OnEvent - Queue Up An IOB For Some Future Error Notification
;----------------------------------------------------------------------------
; IN:   A6      SysBase
;       A5      DeviceBase
;       A4      IO Request Block
;       A3      Unit
;
;       IOS2_WIREERROR(A4)      Which Event To Respond To
;----------------------------------------------------------------------------
; NOTES:
;----------------------------------------------------------------------------

OnEvent
        CALL    Disable
        move.l  IOS2_WIREERROR(A4),D0

        tst.l   d0
        bne     100$

99$     move.b  #S2ERR_BAD_ARGUMENT,IO_ERROR(A4)
        move.l  #S2WERR_BAD_EVENT,IOS2_WIREERROR(A4)
        bra     TermIO

100$

* Special case for online/offline
        move.l  d0,d1
        and.l   #S2EVENT_ONLINE,d1                 * If already online ..
        beq     101$
        btst.b  #UFB_OFFLINE,UnitFlags(a3)
        bne     101$
        move.l  #S2EVENT_ONLINE,IOS2_WIREERROR(a4)
104$    move.b  #0,IO_ERROR(a4)
        bclr    #IOB_QUICK,IO_FLAGS(a4)
        move.b  #0,LN_TYPE(a4)
        move.l  a4,a1
        CALL    ReplyMsg
        CALL    Enable
        rts

101$    move.l  d0,d1                              * If already offline ..
        and.l   #S2EVENT_OFFLINE,d1
        beq     102$
        btst.b  #UFB_OFFLINE,UnitFlags(a3)
        beq     102$
        move.l  #S2EVENT_OFFLINE,IOS2_WIREERROR(a4)
        bra     104$
102$

* Otherwise, add it to the list, and wait ...

        lea.l   UnitEventList(a3),a0
        move.l  A4,A1

        bclr    #IOB_QUICK,IO_FLAGS(A4)
        move.b  #0,LN_TYPE(A4)
        CALL    AddTail
        CALL    Enable
        rts

        IFD     DEBUG_COMMANDS

;----------------------------------------------------------------------------
; ReadCSR - Returns the CSR of your choice
;----------------------------------------------------------------------------
; IN:   A6      SysBase
;       A5      DeviceBase
;       A4      IO Request Block
;       A3      Unit
;
;       IOS2_DTLLENGTH(A4)      Which CSR To Read
;
; USED: A2      ChipBase
;       D7      Which Register We Want To Read
;       D6      Buffer a copy of CSR0
;
; OUT:  IOS2_WIREERROR(A4)      The Returned CSR
;----------------------------------------------------------------------------

ReadCSR movem.l A2/D7/D6,-(SP)
        move.l  UnitChip(A3),A2
        move.l  IOS2_DTLLENGTH(A4),D7
        clr.l   IOS2_WIREERROR(A4)
        cmpi.b  #3,D7
        ble.s   0$

        move.b  #S2ERR_BAD_ARGUMENT,IO_ERROR(A4)
        clr.l   IOS2_WIREERROR(A4)

        bra     999$

0$      DISABLE

        tst.b   D7
        bne.s   10$

        ; This is a read of CSR 0, this is an easy one.

        move.w  D7,LANCE_RAP(A2)
        move.w  LANCE_RDP(A2),IOS2_WIREERROR+2(A4)

        CALL    Enable

        bra.s   999$

10$     ; This is a read of CSR 1, 2, or 3. This is harder...
        ; To access these registers, the LANCE must be in a STOPPED
        ; state. Therefore, we buffer CSR0 and STOP the LANCE. Then
        ; we read the value from the requested CSR and then, if needed,
        ; START the LANCE.

        move.w  #CSR0,LANCE_RAP(A2)
        move.w  LANCE_RDP(A2),D6

        move.w  #CSR0F_STOP,LANCE_RDP(A2)

        ; Read the requested register

        move.w  D7,LANCE_RAP(A2)
        move.w  LANCE_RDP(A2),IOS2_WIREERROR+2(A4)

        ; Do we need to START the LANCE again?

        btst    #CSR0B_STRT,D6
        beq.s   20$

        ; Yes, we do. But first...

        ; If we needed to START the LANCE again, then we need to
        ; reset the BSWP bit which is cleared by the STOP bit.

        move.w  #CSR3,LANCE_RAP(A2)
        move.w  #CSR3F_BSWP,LANCE_RDP(A2)

        move.w  #CSR0,LANCE_RAP(A2)
        move.w  #CSR0F_STRT,LANCE_RDP(A2)

20$     CALL    Enable

999$    movem.l (SP)+,A2/D7/D6
        bra     TermIO

;----------------------------------------------------------------------------
; TestTmr - Tests my timer related code
;----------------------------------------------------------------------------
; IN:   A6      SysBase
;       A5      DeviceBase
;       A4      IO Request Block
;       A3      Unit
; OUT:  IOS2_WIREERROR(A4)      Points to UnitLastConfigTime
;
;----------------------------------------------------------------------------
; Notes:
;
;----------------------------------------------------------------------------

TestTmr lea     UnitStats(A3),A0
        lea     S2DS_LASTSTART(A0),A0
        move.l  A0,IOS2_WIREERROR(A4)
        bsr     GetTime
        bra     TermIO

        ENDC    DEBUG_COMMANDS

;----------------------------------------------------------------------------
; GlbStts - Get Global Statistics About This Unit
;----------------------------------------------------------------------------
; IN:   A6      SysBase
;       A5      DeviceBase
;       A4      IO Request Block
;       A3      Unit
;----------------------------------------------------------------------------
; NOTES:
;----------------------------------------------------------------------------

GlbStts move.l  IOS2_STATDATA(A4),D0
        bne.s   10$

        ; We don't want to write into low memory - after all, we aren't OXXI

        move.b  #S2ERR_BAD_ARGUMENT,IO_ERROR(A4)
        move.l  #S2WERR_NULL_POINTER,IOS2_WIREERROR(A4)
        bra.s   999$

10$     move.l  D0,A1
        move.l  #S2DS_SIZE,D0
        lea     UnitStats(A3),A0
        CALL    CopyMem

999$    bra     TermIO

;----------------------------------------------------------------------------
; AbortIO - Abort The Supplied Request If Possible
;----------------------------------------------------------------------------
; IN:           A6      DeviceBase
;               A1      IOB
;
; USAGE:        A6      SysBase
;               A5      DeviceBase
;               A4      IOB
;               A3      Unit
;               D7      Return Value
;               D6      Write Flag (1 == Yes)
;               A2      IOB Pointer
;----------------------------------------------------------------------------
; NOTES:
;
; 1. The only things we can AbortIO are:
;       CMD_WRITE
;       CMD_READ
;       S2_ONEVENT
;       S2_READORPHAN     (Same handling as CMD_READ)
;       S2_BROADCAST      (Same handling as CMD_WRITE)
;       S2_MULTICAST      (Same handling as CMD_WRITE)
;
;----------------------------------------------------------------------------

AbortIO movem.l D6/D7/A2-A5,-(SP)
        bsr     InitReg

        moveq.l #1,D7           ; Return Value
        moveq.l #0,D6           ; Is This A Write? 0 Means No.

        clr.b   IO_ERROR(A4)

        ; If LN_TYPE is not zero, then this request is not pending.

        tst.b   LN_TYPE(A4)
        bne     999$

        move.w  IO_COMMAND(A4),D0

        cmp.w   #CMD_READ,D0
        beq     200$
        cmp.w   #S2_READORPHAN,D0
        beq     200$
        cmp.w   #CMD_WRITE,D0
        beq     600$
        cmp.w   #S2_BROADCAST,D0
        beq     600$
        cmp.w   #S2_MULTICAST,D0
        beq     600$
        cmp.w   #S2_ONEVENT,D0
        bne     999$

100$    ; Attempting To Abort An ONEVENT Request

        move.l  IOS2_WIREERROR(A4),D0

        cmp.b   #S2EVENT_ERROR,D0       ; Don't assume EVENT_ERROR is zero.
        blt     999$
        cmp.b   #S2EVENT_SOFTWARE,D0
        bgt     999$

        lea     UnitEventList(A3),A2

        CALL    Forbid
        bra     800$

200$    ; Process a read type AbortIO

        CALL    Forbid

        ; Is this a read orphan request?

        cmp.w   #S2_READORPHAN,D0
        bne.s   210$

        ; Attempting To Abort A ReadOrphan

        lea     UnitOrfPN(A3),A2
        lea     PN_RXQ(A2),A2
        bra.s   800$

210$    move.l  IOS2_PACKETTYPE(A4),D0
        cmp.l   #1500,d0
        bhi     220$

        ; Attempting To Abort An 802 Request

        lea     Unit802PN(A3),A2
        lea     PN_RXQ(A2),A2
        bra.s   800$

220$    ; Attempting To Abort An Ordinary Read

        bsr     LocPN
        tst.l   D0
        beq     990$
        move.l  D0,A2
        lea     PN_RXQ(A2),A2
        bra.s   800$

600$    ; Process a Write Type Request

        CALL    Forbid
        moveq.l #1,D6
        lea     UnitTxList(A3),A2

800$    ; Common Aborting Function
        ;
        ; A2 contains the list header address for the list to search.
        ;

        CALL    Disable

        cmp.l   LH_TAILPRED(A2),A2
        beq.s   980$

        ; This list is non-empty. Check the first member.

        move.l  (A2),A2
        cmp.l   A2,A4
        bne.s   820$

        ; The node we want is the first one on the list.

        tst.b   D6
        bne.s   980$

810$    ; Abort the request pointed to by A2

        move.l  A2,A1
        CALL    Remove
        move.b  #IOERR_ABORTED,IO_ERROR(A2)
        moveq.l #0,D7
        move.l  A2,A1
        bsr     TermIO

        bra.s   980$

820$    ; Search for the node we want.

        move.l  (A2),A2
        tst.l   (A2)
        beq.s   980$
        cmp.l   A2,A4
        beq.s   810$
        bra.s   820$

900$	; New Common aborting function for read queues <sigh>
	;
	; A2 contains the protocol nexus list header we want
	;
	; Find the matching StackQueue list header and branch
	; to 800$ which will then walk the list for us. :)
	;
        move.l	IOS2_BUFFERMANAGEMENT(a2),d0
901$	move.l	(a2),a2
	tst.l	(a2)
	beq.s	980$		;No matching StackQueue (weird)

	cmp.l	SQ_FUNCTABLE(a2),d0
	bne.s	901$
	lea.l	SQ_LIST(a2),a2
	bra.s	800$

980$    CALL    Enable

990$    CALL    Permit

999$    move.l  D7,D0
        movem.l (SP)+,A2-A5/D7/D6
        rts

;----------------------------------------------------------------------------
;OnLine - Put This Unit Back OnLine
;----------------------------------------------------------------------------
;USAGE: IN      A6      SysBase
;               A5      DeviceBase
;               A4      IOB
;               A3      UnitBase
;----------------------------------------------------------------------------
;NOTES:
;
;1. If the unit is not OUT of service, it is an error.
;----------------------------------------------------------------------------

OnLine  movem.l A2,-(SP)
        btst.b  #UFB_OFFLINE,UnitFlags(A3)
        bne.s   10$

        move.b  #S2ERR_BAD_STATE,IO_ERROR(A4)
        move.l  #S2WERR_UNIT_ONLINE,IOS2_WIREERROR(A4)
        bra     999$

10$     ; This unit is really offline. This means I do not have to check
        ; to make sure the unit has been initialized, since if it weren't
        ; it couldn't be offline.
        ;
        ; We now reinitialize the hardware again. Make this call from
        ; a DOUBLE FORBIDDEN state.

        CALL    Forbid
        CALL    Forbid
        bsr     CnfgBd

        ; ONE PERMIT IS DONE BY CnfgBd - So we are still forbidden here.

        bclr.b  #UFB_OFFLINE,UnitFlags(A3)

        addq.l  #1,UnitStats+S2DS_RECONFIGURATIONS(A3)
        lea     UnitStats+S2DS_LASTSTART(A3),A0
        bsr     GetTime

        ; Wake up anyone sleeping on an ONLINE event.

        move.l  #s2eF_ONLINE,D0
        bsr     TrmEvnt

        ; Everyone should be woken up now.

        CALL    Permit

999$    movem.l (SP)+,A2
        bra     TermIO

;----------------------------------------------------------------------------
;OffLine - Take This Unit Offline
;----------------------------------------------------------------------------
;USAGE: IN      A6      SysBase
;               A5      DeviceBase
;               A4      IOB
;               A3      UnitBase
;----------------------------------------------------------------------------
;NOTES:
;
;----------------------------------------------------------------------------

OffLine CALL    Forbid

        btst.b  #UFB_PAV,UnitFlags(A3)
        bne.s   10$

        move.b  #S2ERR_BAD_STATE,IO_ERROR(A4)
        move.l  #S2WERR_NOT_CONFIGURED,IOS2_WIREERROR(A4)
        bra     999$

10$     btst.b  #UFB_OFFLINE,UnitFlags(A3)
        beq.s   20$

        move.b  #S2ERR_OUTOFSERVICE,IO_ERROR(A4)
        move.l  #S2WERR_UNIT_OFFLINE,IOS2_WIREERROR(A4)
        bra     999$

20$     ; The unit is online and configured. Take it offline.

        bsr     HaltInterface
        bset.b  #UFB_OFFLINE,UnitFlags(A3)

        ; Now start the arduous task of aborting anything that
        ; is pending!

	move.l	#S2WERR_UNIT_OFFLINE,d0
        bsr.s   AbAll

150$    ; Wake up anyone sleeping on an OFFLINE event.

        move.l  #s2eF_OFFLINE,D0
        bsr     TrmEvnt

999$    CALL    Permit
        bra     TermIO

;----------------------------------------------------------------------------
;Flush - AbortIO everything in sight
;----------------------------------------------------------------------------
;USAGE: IN      A6      SysBase
;               A5      DeviceBase
;               A4      IOB
;               A3      UnitBase
;----------------------------------------------------------------------------
;NOTES:
;
;----------------------------------------------------------------------------


Flush:
        bclr    #IOB_QUICK,IO_FLAGS(a4)
	move.l	#S2WERR_GENERIC_ERROR,d0
        CALL    Disable
        bsr     AbAll
        CALL    Enable
        bra     TermIO


;----------------------------------------------------------------------------
; AbAll - Cause everything to be aborted.
;----------------------------------------------------------------------------
; IN:   A6      SysBase
;       A3      UnitBase
; USE:  A2      Pointer To Things
;       D7      Loop Counter
;       D6      Temporary Buffer Of A2
;	D2	Type of error to place in IOS2_WIREERROR
;----------------------------------------------------------------------------


AbAll   movem.l A2/D7/D6/D2,-(SP)
	move.l	d0,d2
        ; Abort All 802 Style Read Requests

        lea     Unit802PN+PN_RXQ(A3),A0
        bsr     AbrtAllSQ

        ; Abort All ReadOrphan Requests

        lea     UnitOrfPN+PN_RXQ(A3),A0
        bsr.s   AbrtAllSQ

        ; Abort All Write Requests

        lea     UnitTxList(A3),A0
        bsr.s   AbrtAll

        lea.l   UnitTxRunningList(A3),a0
        bsr.s   AbrtAll

        ; Loop Through All Protocol Nexa Aborting Anything I Find

        lea     UnitProtocolHashTable(A3),A2
        move.l  #HASH_SIZE-1,D7

50$     cmp.l   LH_TAILPRED(A2),A2
        beq.s   70$

        ; This Hash List Is NOT Empty ... Run Through Each Element

        move.l  A2,D6
        move.l  (A2),A2

55$     tst.l   (A2)
        beq.s   60$

        lea     PN_RXQ(A2),A0
        bsr.s   AbrtAllSQ
        move.l  (A2),A2
        bra.s   55$

60$     move.l  D6,A2

70$     add.l   #MLH_SIZE,A2
        dbf     D7,50$

150$    movem.l (SP)+,A2/D7/D6/D2
        rts

;----------------------------------------------------------------------------
;AbrtAll - Strip an entire list of requests, aborting each one.
;----------------------------------------------------------------------------
;IN:    A0      A List To Strip
;----------------------------------------------------------------------------
;----------------------------------------------------------------------------

AbrtAll movem.l A2/A4,-(SP)
        move.l  A0,A2
10$     move.l  A2,A0
        CALL    RemHead
        tst.l   D0
        beq.s   999$

        move.l  D0,A4
        move.b  #IOERR_ABORTED,IO_ERROR(A4)
        move.l  d2,IOS2_WIREERROR(A4)
        bsr     TermIO
        bra.s   10$

999$    movem.l (SP)+,A2/A4
        rts

;----------------------------------------------------------------------------
;AbrtAllSQ - Strip an entire list of StackQueue's
;----------------------------------------------------------------------------
;IN:    A0      The StackQueue List To Strip
;----------------------------------------------------------------------------
;----------------------------------------------------------------------------

AbrtAllSQ
	move.l  A2,-(sp)
        move.l  A0,A2		; PN_RXQ List Header
10$     move.l  (a2),a2
	tst.l	(a2)
	beq.s	999$
	lea.l	SQ_LIST(a2),a0
	bsr	AbrtAll
	bra.s	10$

999$    movea.l	(sp)+,A2
        rts

;----------------------------------------------------------------------------
;Speshul - Handle a request for device specific statistics.
;----------------------------------------------------------------------------
;IN:    A6      SysBase
;       A5      DeviceBase
;       A4      IOB
;       A3      UnitBase
;----------------------------------------------------------------------------
;----------------------------------------------------------------------------

Speshul move.l  IOS2_STATDATA(A4),D0
        bne.s   10$

0$      move.b  #S2ERR_BAD_ARGUMENT,IO_ERROR(A4)
        move.l  #S2WERR_NULL_POINTER,IOS2_WIREERROR(A4)
        bra.s   999$

10$     ; statdata is nonnull at least.

        move.l  D0,A0
        move.l  S2SSH_RECORDCOUNTMAX(A0),D0
        bne.s   20$

        ; He can accept zero arguments? No Way!

        move.b  #S2ERR_BAD_ARGUMENT,IO_ERROR(A4)
        move.l  #S2WERR_BAD_STATDATA,IOS2_WIREERROR(A4)
        bra.s   999$

20$     ; the number of records he can take is non-null.

        move.l  #1,S2SSH_RECORDCOUNTSUPPLIED(A0)
        lea     S2SSH_SIZE(A0),A0

        move.l  #S2SSH_MCR_ID,S2SSR_TYPE(A0)
        move.l  UnitMCastRejects(A3),S2SSR_COUNT(A0)
        move.l  #MCR_TXT,S2SSR_STRING(A0)

999$    bra     TermIO




MCR_TXT S2SSH_MCR_TXT
Msg1	dc.b	'BeginIO Command=%ld from %s',10,0

        END
