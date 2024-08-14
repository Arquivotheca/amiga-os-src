**
** A2065 SANA-II Device Driver source
** Copyright (c) 1992 by Commodore-Amiga, Inc.
**
** All rights Reserved.
**


MAIN_MODULE    EQU      1

	IFND	DEBUG_MODE
;DEBUG_MODE	EQU	1
	ENDC

TagFunctions equ 3         * CopyToBuff & CopyFromBuff & PacketFilter
FuncTableSize equ (4*(TagFunctions+1))
BaseTableSize equ (6*(TagFunctions+1))

        CODE

        NOLIST

        include "includes.asm"
        include "globals.i"
        include "a2065_defs.i"

        LIST

        INT_ABLES

        XREF    _AbsExecBase
        XREF    OpnLibs,ClsLibs
        XREF    FindBds,RemBds
        XREF    BeginIO,RemMCA,AbAll
        XREF    TermIO,HaltInterface
        XREF    CnfgBd,WrtNxt,TxStats,FreePN
        XREF    LocPN,LocMCN,AbortIO
        XREF    _intena
        XREF	LocSQ,FreeSQ
        XDEF    TheSoftInt
        XDEF    DName,TrmEvnt
        XDEF    INEAOff,INEAOn

        CNOP    2

.begin
        moveq.l #0,D0
        rts

RomTag  dc.w    RTC_MATCHWORD
        dc.l    RomTag
        dc.l    EndCode
        dc.b    RTF_AUTOINIT
        dc.b    VERSION
        dc.b    NT_DEVICE
        dc.b    DEVPRI
        dc.l    DName
        dc.l    DID
        dc.l    DInit

DInit   dc.l    MYLIB_SIZE
        dc.l    DFuncs
        dc.l    DData
        dc.l    DevInit

DFuncs  dc.l    DOpen                   ; Open
        dc.l    DClose                  ; Close
        dc.l    DExp                    ; Expunge
        dc.l    .begin                  ; Null
        dc.l    BeginIO                 ; BeginIO
        dc.l    AbortIO                 ; AbortIO
        dc.l    -1

DData   INITBYTE        LH_TYPE,NT_DEVICE
        INITLONG        LN_NAME,DName
        INITBYTE        LIB_FLAGS,LIBF_CHANGED|LIBF_SUMUSED
        INITWORD        LIB_VERSION,VERSION
        INITWORD        LIB_REVISION,REVISION
        INITLONG        LIB_IDSTRING,DID
        dc.l            0

;----------------------------------------------------------------------------
; DevInit - Called only from the first user OpenDevice
;----------------------------------------------------------------------------
; This routine is called by the system after it has loaded the device
; driver's code into memory. As documented, if the init routine fails
; it is supposed to return 0 in D0 which will cause the system to
; deallocate the memory it allocates for us. But I have found this
; feature to be broken. Therefore, this init routine will always
; report success to the system but will set a flag preventing/allowing
; OpenDevice's to actually take place. If OpenDevice's are disabled,
; then an Expunge is sure to follow.
;
; INPUT:        D0      DeviceBase
;               A0      Device Segment List
;
; OUTPUT:       D0      DeviceBase
;               BIT 0 of DeviceFlags(A5) will be set or unset depending
;                        on success of the initialization.
;----------------------------------------------------------------------------

DevInit
        move.l  A5,-(SP)
        move.l  4,A6
        move.l  D0,A5
        move.l  A0,DeviceSegmentList(A5)
        move.l  A6,SysBase(A5)

        ; Initialize various fields in the DeviceBase

        clr.b   DeviceFlags(A5)
        clr.l   UnitCount(A5)
        lea     UnitArray(A5),A0
        moveq.l #MAX_BOARDS-1,D0
1$      clr.l   (A0)+
        dbf     D0,1$

        NLIST   ActiveUnits(A5)

        ; Open the libraries we need. If any fail, then we will fail.

        bsr.s   OpnLibs
        tst.l   D0
        beq.s   990$

        ; Go look for some hardware we can control.

        bsr.s   FindBds
        tst.l   D0
        beq.s   998$

        ; Install our Interrupt Service Routine

        bsr     InstallInterrupt


        ; Setting this bit is what determines success or failure.

        bset.b  #DFB_INIT_OK,DeviceFlags(A5)
        bra.s   990$

998$    bsr.s   ClsLibs
990$    move.l  A5,D0
        move.l  (SP)+,A5
        rts

;----------------------------------------------------------------------------
; DOpen - Where OpenDevice ultimately enters the device driver
;----------------------------------------------------------------------------
; The system will call DOpen after the device has been initialized
; and after every user call to OpenDevice.
;
; The UnitOpenCount ticks with every OpenDevice, however the
; DeviceOpenCount ticks only with the OpenDevice of each then
; unused Unit.
;
;
; INPUT:        A6      DeviceBase
;               A1      IO Request Block
;               D1      OpenDevice Flags
;               D0      OpenDevice Unit Number
;
; USAGE:        A6      ExecBase
;               A5      DeviceBase
;               A4      IO Request Block
;               A3      UnitBase
;               D7      Unit Number
;----------------------------------------------------------------------------

DOpen
        movem.l A3-A5/D7,-(SP)
        jsr     InitReg

        ; See if we are accepting Open's at all...

        btst.b  #DFB_INIT_OK,DeviceFlags(A5)
        bne.s   1$

0$      move.b  #IOERR_OPENFAIL,IO_ERROR(A4)
        bra     990$

1$
* modifications to parse a taglist that contain pointers to the
* functions CopyToBuff and CopyFromBuff
*

        tst.l    IOS2_BUFFERMANAGEMENT(a4)
        beq      175$

        movem.l d0-d7/a0-a6,-(sp)

        sub.l   #FuncTableSize,sp
        move.l  sp,a3                          *a3 points to our function
	move.l	#DummyPacketHook,8(a3)
	moveq.l	#0,d2
	move.l	IOS2_BUFFERMANAGEMENT(a4),a0
	move.l	#TAG_COPYTOBUFF,d0
	move.l	UTBase(a5),a6
	CALL	FindTagItem
	tst.l	d0
	beq.s	999$				*Oops. No CopyToBuff pointer.
	movea.l	d0,a0
	move.l	4(a0),(a3)
	move.l  IOS2_BUFFERMANAGEMENT(a4),a0
	move.l	#TAG_COPYFROMBUFF,d0
	CALL	FindTagItem
	tst.l	d0
	beq.s	999$
	movea.l	d0,a0
	move.l	4(a0),4(a3)
	move.l  IOS2_BUFFERMANAGEMENT(a4),a0
	move.l	#TAG_PACKETFILTER,d0
	CALL	FindTagItem
	tst.l	d0
	beq.s	900$
	movea.l	d0,a0
	move.l	4(a0),8(a3)
900$	move.l	SysBase(a5),a6
	move.l	#-1,FuncTableSize-4(a3)
	bra.s	904$

999$    move.l	SysBase(a5),a6
	add.l   #FuncTableSize,sp            * Free the stack function array area and exit
        movem.l (sp)+,d0-d7/a0-a6
        bra     0$

198$    move.l  (sp)+,a1                     * Free the Function jump table memory that was allocated + the stack area & exit
        move.l  #BaseTableSize,d0
        CALL    FreeMem
        bra.s   999$

904$    move.l  #BMS_SIZE,d0
        move.l  #MEMF_CLEAR+MEMF_PUBLIC,d1
        CALL    AllocMem
        tst.l   d0
        beq     999$
        movea.l	d0,a1
        move.l	(a3),BMS_CopyToBuff(a1)
        move.l	4(a3),BMS_CopyFromBuff(a1)
        move.l	8(a3),BMS_PacketHook(a1)

        move.l  a1,IOS2_BUFFERMANAGEMENT(a4)

        add.l   #FuncTableSize,sp
        movem.l (sp)+,d0-d7/a0-a6
175$

        ; Is the unit number within reason?

        cmp.l   #MAX_BOARDS,D0
        bge     0$

        ; Is the unit number valid?

        lea     UnitArray(A5),A0
        move.l  D0,D7
        asl.l   #2,D0
        move.l  0(A0,D0.l),D0
        beq     0$

        ; A valid UnitBase has been located.

        move.l  D0,A3

        ; Test for exclusive open stuff.

        btst    #UFB_MINE,UnitFlags(A3)
        bne     0$

        btst    #SANA2OPB_MINE,D1
        beq     100$

        ; User is asking for exclusive use...is unit already open?
        tst.l   UnitOpenCount(A3)
        bne     0$

        ; EXCLUSIVE USE WILL BE GRANTED.

        bset    #UFB_MINE,UnitFlags(A3)

        ; Check here for the presence of the PROM flag.
        ; Notice that this means that PROM only has effect
        ; if the device has been opened exclusively.

        btst    #SANA2OPB_PROM,D1
        beq     100$

        bset    #UFB_PROM,UnitFlags(A3)

100$    ; Unit access IS being granted. If this is the first open
        ; for this unit - then increase the device open count.

        tst.l   UnitOpenCount(A3)
        bne     110$

        ; This unit has up-to-now been unused. Increase the Device
        ; open count as well as perform other bookkeeping tasks.

        addq.w  #1,LIB_OPENCNT(A5)

        lea     ActiveUnits(A5),A0
        move.l  A3,A1
        DISABLE
        CALL    AddTail
        CALL    Enable

        ; How about this bookkeeping task: Turning the board back
        ; on if it was on once before.

        btst.b  #UFB_PAV,UnitFlags(A3)
        beq.s   110$

        ; We have to turn the hardware back on. The CnfgBd routine
        ; assumes that we have ourselves Forbidden. So do so.

        CALL    Forbid

        ; Don't configure if we are OFFLINE!

        btst.b  #UFB_OFFLINE,UnitFlags(A3)
        bne.s   109$

        bsr     CnfgBd
        bra.s   110$

        ;       THE PERMIT IS DONE BY THE CALLED ROUTINE!

109$    ; If I get here, the Configuration didn't actually
        ; happen because I am offline. Therefore, PERMIT

        CALL    Permit

110$
        addq.l  #1,UnitOpenCount(A3)
        move.l  A3,IO_UNIT(A4)
        move.l  A5,IO_DEVICE(A4)
        clr.b   IO_ERROR(A4)
        bclr    #LIBB_DELEXP,LIB_FLAGS(A5)

990$    movem.l (SP)+,A3-A5/D7
        rts

;----------------------------------------------------------------------------
; DClose - Where CloseDevice ultimately enters the device driver
;----------------------------------------------------------------------------
; The system will call DClose after each user call to CloseDevice. Each
; CloseDevice causes a UnitOpenCount to drop. When that count reaches 0
; the DeviceOpenCount will be decremented. If the DeviceOpenCount drops
; to 0 then the device may be expunged.
;
; INPUT:        A6      DeviceBase
;               A1      IO Request Block
;
; USAGE:        A6      ExecBase
;               A5      DeviceBase
;               A4      IO Request Block
;               A3      UnitBase
;
; OUTPUT:       D0      SegList returned by Expunge routine or 0
;----------------------------------------------------------------------------


DClose
        movem.l A3-A5,-(SP)
        jsr     InitReg

        ; Update the appropriate fields in the IO Request Block itself.
        ; D0 now contains either 0 or the result from calling DExp.

        clr.l   IO_DEVICE(A4)
        clr.l   IO_UNIT(A4)

* Free memory allocated by the buffermanagement base
        tst.l   IOS2_BUFFERMANAGEMENT(a4)
        beq     899$
        move.l  d0,-(sp)

        move.l  IOS2_BUFFERMANAGEMENT(a4),a1
        move.l  #BMS_SIZE,d0
        CALL    FreeMem
        move.l  (sp)+,d0

899$	move.l	IOS2_BUFFERMANAGEMENT(a4),d0

	; Free any SQ's for this protocol stack

	bsr	FreeSQ

        moveq.l #0,D0
        subq.l  #1,UnitOpenCount(A3)
        bne     999$

        ; This unit is now idle. Remove it from service.

        ; TAKE BOARD OUT OF SERVICE HERE...

        move.b  UnitFlags(a3),d0
        and.b   #$ff-(UFF_MINE+UFF_PROM),d0             * Reset these flags
        move.b  d0,UnitFlags(a3)

        move.l  A3,A1
        DISABLE
        CALL    Remove

        bsr     HaltInterface

        CALL    Enable

        ; Perform Unit bookkeeping

        bsr     RemMCA

        bsr     FreePN

        ; Perform device bookkeeping, expunging the device if necessary.

        moveq.l #0,D0
        subq.w  #1,LIB_OPENCNT(A5)
        bne.s   999$

        btst    #LIBB_DELEXP,LIB_FLAGS(A5)
        beq.s   999$

        move.l  A5,A6
        bsr     DExp

999$
        movem.l (SP)+,A3-A5
        rts

;----------------------------------------------------------------------------
; DExp - Possible remove the device from memory
;----------------------------------------------------------------------------
; The system will call DExp when the device is installed and a memory
; allocation fails someplace else in the system. In this case, if the
; device is being used, then simply set the DELEXP flag so that DExp
; will be called the next time the usage count of the device drops
; to zero in CloseDevice.
;
; INPUT:        A6      DeviceBase
; USAGE:        A6      ExecBase
;               A5      DeviceBase
;               A4      UnitBase
;               A3      IO Request Block
;
; OUTPUT:       D0      SegList or 0
;----------------------------------------------------------------------------


DExp
        movem.l d2/a5/a6,-(sp)
        move.l  a6,a5

        tst.w   LIB_OPENCNT(a5)
        beq.s   1$

        bset    #LIBB_DELEXP,LIB_FLAGS(a5)
        moveq   #0,d0
        bra     99$

1$      ; Device is fully closed

        move.l  DeviceSegmentList(a5),d2

        ; Remove the device from the device list

        move.l  a5,a1
        move.l  SysBase(A5),a6
        CALL    Remove

        ; Remove the interrupt wedge

        btst.b  #INTERRUPT_LIST,d0
        beq     10$

        move.l  #INTERRUPT_LIST,d0
        lea     MyInt(a5),a1
        CALL    RemIntServer

        bsr     ClsLibs

10$     bsr     RemBds

        move.l  a5,a1
        moveq   #0,d0
        move.w  LIB_NEGSIZE(a5),d0
        sub.l   d0,a1
        add.w   LIB_POSSIZE(a5),d0
        CALL    FreeMem


        move.l  d2,d0

99$     movem.l (sp)+,d2/a5/a6

        rts

;----------------------------------------------------------------------------
; DummyWantPacketHook - WantPacket hook for older protocol stacks
;----------------------------------------------------------------------------

DummyPacketHook:
	dc.l	0	;LN_SUCC
	dc.l	0	;LN_PRED
	dc.l	1$      ;Function to call
	dc.l	0	;Not used
	dc.l	0	;Not used - I don't need no steeking context

1$	moveq.l	#1,d0
	rts


;----------------------------------------------------------------------------
; InitReg - Perform standard register initializations
;----------------------------------------------------------------------------
; Nearly every device entry point will set up the same standard
; registers. These initializations are done by InitReg as follows:
;
; INPUT:        A6      DeviceBase
;               A1      IO Request Block
;
; USAGE:        A6      SysBase
;               A5      DeviceBase
;               A4      IO Request Block
;               A3      UnitBase
;
; NOTE:         Fragile registers (like A5) must be saved first.
;----------------------------------------------------------------------------


InitReg move.l  A6,A5
        move.l  A1,A4
        move.l  IO_UNIT(A4),A3
        move.l  SysBase(A5),A6
        rts

;----------------------------------------------------------------------------
; InstallInterrupt - Installs the hardware interrupt service routine
;----------------------------------------------------------------------------
; IN:   A6      SysBase
;       A5      DeviceBase
;
;----------------------------------------------------------------------------
; Notes:
;
; 1. Called during device initialization time. There shouldn't be any
;    interrupts enabled on the device, so no interrupts should actually
;    happen until other things are set up first.
; 2. This interrupt installation sets up DeviceBase as the data passed
;    to the interrupt service routine. From there, all additional info
;    must be fished out by the interrupt service routine itself.
;----------------------------------------------------------------------------

InstallInterrupt
        movem.l d0/a1,-(sp)
        move.b  #NT_INTERRUPT,LN_TYPE+MyInt(A5)
        move.b  #100,LN_PRI+MyInt(A5)
        move.l  #DName,LN_NAME+MyInt(A5)
        move.l  A5,IS_DATA+MyInt(A5)
        move.l  #IntSvc,IS_CODE+MyInt(A5)
        lea     MyInt(A5),a1
        move.l  #INTERRUPT_LIST,d0
        CALL    AddIntServer
        movem.l (sp)+,d0/a1
        rts
;----------------------------------------------------------------------------
; TheSoftInt - A Software Interrupt Used To Launch The NEXT Transmission
;----------------------------------------------------------------------------
; IN:   A1      IO Request Block To Be Transmitted
;----------------------------------------------------------------------------
; NOTES:
;
; 1. Nearly every register must be saved.
; 2. The usual standard registers must be synthesized from the IOB.
;----------------------------------------------------------------------------

TheSoftInt
        movem.l D2-D7/A0-A6,-(SP)
        move.l  A1,A4
        move.l  IO_DEVICE(A4),A5
        move.l  IO_UNIT(A4),A3
        move.l  SysBase(A5),A6
        jsr     INEAOff

1$      move.l  UnitNWDone(a3),d2
        cmp.l   UnitNWFill(a3),d2  *If no space, exit
        beq     2$

        lea.l   UnitTxList(a3),a0      * If none waiting, exit
        cmp.l   LH_TAILPRED(a0),a0
        beq     2$
        CALL    Disable
        CALL    RemHead
        move.l  d0,a1
        lea.l   UnitTxRunningList(a3),a0
        CALL    AddTail
        CALL    Enable
        bsr     WrtNxt
        tst.l   UnitNWDone(a3)
        bge     3$
        move.l  UnitNWFill(a3),UnitNWDone(a3)
3$

        move.l  UnitNWFill(a3),d2
        addq.l  #1,d2
        and.l   #W_BUFF_MASK,d2
        move.l  d2,UnitNWFill(a3)

2$      bsr     INEAOn
        movem.l (SP)+,D2-D7/A0-A6
        rts

;----------------------------------------------------------------------------
; IntSvc - Interrupt Service Routine
;----------------------------------------------------------------------------
; IN:   A1      DeviceBase
;
; USE:  A6      SysBase
;       A5      DeviceBase
;       A3      UnitBase
;       A2      ChipBase
;       D7      LoopFlag
;       D6      Cached Interrupt Flags
;----------------------------------------------------------------------------

IntSvc  movem.l A0-A4/d2-D7,-(SP)
        move.l  A1,A5
        move.l  SysBase(A5),A6

0$      lea     ActiveUnits(A5),A3
        move.l  (A3),A3

        moveq.l #0,D7

10$     tst.l   (A3)
        beq     50$

        ; Check if this unit has an interrupt pending.

        moveq.l #0,D6
        move.l  UnitChip(A3),A2
        move.w  #CSR0,LANCE_RAP(A2)
        move.w  LANCE_RDP(A2),D6
        move.w  d6,d5
        and.w   #CSR0F_RINT|CSR0F_TINT|CSR0F_MISS|CSR0F_CERR|CSR0F_BABL|CSR0F_IDON|CSR0F_MERR,D5
        or.w    #CSR0F_INEA,d5
        move.w  d5,LANCE_RDP(a2)
        and.w   #$FF80,D6
        beq.s   40$

        ; This unit has some interrupt flags set.

        moveq.l #1,D7

        btst.l  #CSR0B_RINT,D6
        beq.s   20$
        bsr     RxDone
20$     btst.l  #CSR0B_TINT,D6
        beq.s   21$
        bsr     TxDone
21$     btst.l  #CSR0B_IDON,D6
        beq.s   22$
        bsr     IDone
22$     btst.l  #CSR0B_MERR,D6
        beq.s   23$
        bsr     MErr
23$     btst.l  #CSR0B_MISS,D6
        beq.s   24$
        bsr     MissPkt
24$     btst.l  #CSR0B_CERR,D6
        beq.s   25$
        bsr     CErr
25$     btst.l  #CSR0B_BABL,D6
        beq.s   40$
        bsr     Babbler

40$     move.l  (A3),A3
        bra     10$

50$     tst.l   D7
        bne     0$

        movem.l (SP)+,A0-A4/d2-D7
        moveq.l #0,D0
        rts

;----------------------------------------------------------------------------
; Interrupt Function Routines
;----------------------------------------------------------------------------
; IN:   A6      SysBase
;       A5      DeviceBase
;       A3      UnitBase
;       A2      ChipBase
;       D7      LoopFlag For Parent Routine
;       D6      Cached Interrupt Pending Bits
;----------------------------------------------------------------------------
;----------------------------------------------------------------------------

IDone   lea     UnitConfigList(A3),A0
        CALL    RemHead
        tst.l   D0
        beq.s   10$

        move.l  D0,A4
        bsr     TermIO

10$

20$     move.w  #CSR0,LANCE_RAP(A2)
        move.w  #CSR0F_IDON|CSR0F_INEA,LANCE_RDP(A2)
        move.w  #CSR0F_STRT|CSR0F_INEA,LANCE_RDP(A2)

        ; Does the transmitter need kickstarting? This will happen
        ; when a multicast address happens to be added when more than
        ; one transmit request is pending.

        btst.b  #UFB_WRITE_NEEDS_KICKSTART,UnitFlags(A3)
        beq.s   30$

        ; Yes - it does.

        bclr.b  #UFB_WRITE_NEEDS_KICKSTART,UnitFlags(A3)

        lea     UnitTxList(A3),A0
        cmp.l   LH_TAILPRED(A0),A0
        beq.s   30$

        lea     UnitSoftInt(A3),A1
        move.l  (A0),IS_DATA(A1)
        CALL    Cause

30$

        rts

;----------------------------------------------------------------------------
; TxDone - Called When A Transmission Completes
;----------------------------------------------------------------------------
; IN:   A6      SysBase
;       A5      DeviceBase
;       A3      UnitBase
;       A2      ChipBase
;       D7      LoopFlag For Parent Routine
;       D6      Cached Interrupt Pending Bits
;----------------------------------------------------------------------------
; NOTES:
;
;----------------------------------------------------------------------------

TxDone

        move.l  UnitNWDone(a3),d0
        bge     100$
        bra     200$
100$

        move.l  UnitTxDescriptor(a3),a4
        move.l  d0,d1
        asl.l   #3,d1
        add.l   d1,a4
        move.w  TMD1(a4),d0
        btst    #TMD1B_OWN,d0
        bne     200$

        move.l  UnitNWDone(a3),d0
        addq.l  #1,d0
        and.l   #W_BUFF_MASK,d0
        move.l  d0,UnitNWDone(a3)

        lea     UnitTxRunningList(A3),A0
        CALL    RemHead
        tst.l   D0
        beq.s   10$

        move.l  D0,A4

        ; The IORequest Which Just Completed Is In A4.
        ; Add to all the appropriate statistics blocks.

        add.l   #1,UnitStats+S2DS_TXPACKETS(A3)

        bsr     TxStats                         ; to handle protocol stats

        bsr     TermIO

10$
        move.l  UnitNWDone(a3),d2
        cmp.l   UnitNWFill(a3),d2
*        bne     100$
        bne     TxDone
        move.l  #-1,UnitNWDone(a3)
200$

        ; If there is another packet waiting to be transmitted, then
        ; invoke the software interrupt to cause that transmission to
        ; occur.

        lea     UnitTxList(A3),A0
        cmp.l   LH_TAILPRED(A0),A0
        beq.s   30$

        move.l  UnitNWFill(a3),d2
        cmp.l   UnitNWDone(a3),d2
        beq     30$

        ; There is a transmit waiting, but does someone need us to
        ; stop transmitting for a while?

        btst.b  #UFB_HALT_WRITES,UnitFlags(A3)
        beq.s   25$

        ; Yes, we are being asked to stop transmitting with a transmit
        ; still in the queue. Therefore set the NEEDS_KICKSTART flag
        ; and DONT start the next write.

        bset.b  #UFB_WRITE_NEEDS_KICKSTART,UnitFlags(A3)
        bra.s   30$

25$     lea     UnitSoftInt(A3),A1
        move.l  (A0),IS_DATA(A1)
        CALL    Cause

30$     bclr.b  #UFB_HALT_WRITES,UnitFlags(A3)
        rts


;----------------------------------------------------------------------------
; RxDone - Called When A Packet Arrives From The Wire
;----------------------------------------------------------------------------
; IN:   A6      SysBase
;       A5      DeviceBase
;       A3      UnitBase
;USE:   D6      Flag Bits To Set In The IOB's Flag Byte
;       D5      Number Of Descriptors In A Row Which We Didn't Own
;       D4      The Descriptor Number We Are Looking At Now.
;       A2      Pointer To The Descriptor In Question
;       A4      Miscellaneous
;----------------------------------------------------------------------------
;----------------------------------------------------------------------------


RxDone:

        movem.l D2-D7/A2,-(SP)
        move.l  UnitNRD(A3),D4
        moveq.l #0,D5

0$      move.l  D4,D0
        asl.l   #3,D0
        move.l  UnitRxDescriptor(A3),A2
        adda.l  D0,A2

        ; A2 now points to the descriptor in question

        move.w  RMD1(A2),D0

        ; Do We Own This Descriptor? D0 now has the flag word

        btst    #RMD1B_OWN,D0
        beq.s   100$

        ; No We Don't - Look At The Next Descriptor...
        ; Look at up to RXBUF_COUNT descriptors UNsuccesfully.

5$      addq.l  #1,D4
        cmp.b   #RXBUF_COUNT,D4
        bne.s   10$
        moveq.l #0,D4
10$     move.l  D4,UnitNRD(A3)
        addq.l  #1,D5
        cmp.b   #RXBUF_COUNT,D5
        bne.s   0$

        addq.l  #1,D4
        cmp.b   #RXBUF_COUNT,D4
        bne.s   20$
        moveq.l #0,D4
20$     move.l  D4,UnitNRD(A3)

        bra     999$

100$    ; We Own This Descriptor - Clear The UnSuccesful-In-A-Row Count

        moveq.l #0,D5
        move.l  D5,D6

        ; Is There An Error?

        btst    #RMD1B_ERR,D0
        beq     200$

        ; Yes, There Is An Error - Figure Out Which

        ; Is ENP Set? This Affects Error Code Validity

        btst    #RMD1B_ENP,D0
        beq.s   150$

        ; Yes - ENP Is Set...This Could Be FRAM or CRC

        btst    #RMD1B_FRAM,D0
        beq.s   120$

        add.l   #1,UnitStats+S2DS_FRAMINGERRORS(A3)

120$    btst    #RMD1B_CRC,D0
        beq.s   170$

        add.l   #1,UnitStats+S2DS_BADDATA(A3)
        bra.s   170$

150$    ; ENP Is Not Set, Therefore This Can Be A OFLO

        btst    #RMD1B_OFLO,D0
        beq.s   170$

        add.l   #1,UnitStats+S2DS_FIFOOVERRUNS(A3)

170$    ; All Receives In Error Come Here...Set Things Right

        move.w  #0,RMD3(A2)
        move.w  #RMD1F_OWN,RMD1(A2)
        bra     5$

200$    ; This Packet Has No Error

        add.l   #1,UnitStats+S2DS_RXPACKETS(A3)
        moveq.l #0,D3
        move.w  RMD3(A2),D3

        ; D3 now has the physical packet length
        ; Is This A MultiCast? If Yes, Then Validate It

        moveq.l #0,D0
        move.l  UnitBoard(A3),A1
        move.w  RMD0(A2),D0
        adda.l  D0,A1                   ; A1 now points to the packet

        btst.b  #0,EP_DSTADDR(A1)
        beq.s   250$

        ; This may be a multicast or a broadcast

        moveq.l #-1,D0
        cmp.l   EP_DSTADDR(A1),D0
        bne.s   210$
        cmp.w   EP_DSTADDR+4(A1),D0
        bne.s   210$

        ; This is a BroadCast

        moveq.l #SANA2IOF_BCAST,D6
        bra.s   250$

210$    ; This is a multicast packet

        btst.b  #UFB_PROM,UnitFlags(a3)             * If in prom mode, you should receive all
        bne     250$                                * destination multicasts

        moveq.l #SANA2IOF_MCAST,D6

        movem.l A1,-(SP)
        lea     EP_DSTADDR(A1),A0
        bsr     LocMCN
        movem.l (SP)+,A1
        tst.l   D0
        bne.s   250$

        addq.l  #1,UnitMCastRejects(A3)
        bra     170$

250$    ; This packet is for us. Fish Out The Packet Type

        moveq.l #0,D0
        move.w  EP_PTYPE(A1),D0

        movem.l A1,-(SP)
        bsr     LocPN
        movem.l (SP)+,A1
        move.l  D0,A4
        tst.l   D0
        bne.s   500$

        ; No packet of this specific type waiting...If it is an 802
        ; packet then is there a read for one of those?

        move.w  EP_PTYPE(A1),D0
        cmp.w   #1500,D0
        bgt.s   260$

        lea     Unit802PN(A3),A4
        lea     PN_RXQ(A4),A0
        cmp.l   LH_TAILPRED(A0),A0
        bne.s   500$

260$    ; Is there a read orphan pending? If so, make it that.

        lea     UnitOrfPN(A3),A4
        lea     PN_RXQ(A4),A0
        cmp.l   LH_TAILPRED(A0),A0
        bne.s   500$

270$    ; I don't know what to do with this packet. Check again against
        ; the 802 style bounds. Because, if it is an 802 packet, we
        ; can register that it is a Dropped 802.

        cmp.w   #1500,D0
        bgt.s   275$
        lea     Unit802PN(A3),A4
272$    btst    #PNB_TRACKING,PN_FLAGS(A4)
        beq.s   275$
        addq.l  #1,PN_TYPESTATS+S2PTS_PACKETSDROPPED(A4)
        bra     170$

275$    add.l   #1,UnitStats+S2DS_RXUNKNOWNTYPES(A3)
        bra     170$


500$    ; A4 contains the appropriate protocol nexus

        lea     PN_RXQ(A4),A0
        cmp.l   LH_TAILPRED(A0),A0
        beq.b   272$

        ;This next part is for SANA-II V2 compliance.  We maintain a list
        ;of StackQueue's, each queue holding the read requests from a
        ;different device opener.  We attempt to hand this packet off to
        ;everyone on the list that wants a copy.

	;At this point we have:
	;
	; a0  -  Pointer to the StackQueue list header
	; a1  -  Pointer to the packet data
	; a2  -  Descriptor
	; a3  -  Unit Pointer
	; a4  -  Protocol Nexus
	; a5  -  Device Base
	; a6  -  SysBase

	; d0  -  Scratch
	; d1  -  Scratch
	; d2  -  Scratch
	; d3  -  physcial length
	; d4  -  Descriptor Number
	; d5  -  Descriptor Didn't Own Count
	; d6  -  IOB Flags
	; d7  -  Scratch

550$	; Use PN_PADDING(a4) as a temporary flag for whether the packet gets dropped.

	move.b	#1,PN_PADDING(a4)	; TRUE = Packet dropped

	move.l	a4,-(sp)		; Save the PN Address for later
	move.l	a1,d7			; d7 will hold original packet data pointer

560$	movea.l	d7,a1			; Reset packet data pointer
	move.l	d3,d2			; Reset packet length

	move.l	(a0),a0			; Get next stack queue
	tst.l	(a0)			; Are we finished?
	beq	680$			; Yes...

        move.l	a0,-(sp)		; Save pointer to StackQueue
        lea.l	SQ_LIST(a0),a0		; Get IOReq list
        cmp.l	LH_TAILPRED(a0),a0	; Empty list?
        bne.b	570$			; Nope...

        movea.l	(sp)+,a0		; Restore StackQueue pointer
        bra.b	560$

        ; Get pointer to the first IO request...

570$    move.l	LH_HEAD(a0),a4

	; Set up the IO Request fields to look like they will if they say "yes".
	; Note: This doesn't really waste any significant amount of time, and
	; it may be useful for packet sniffers that want to do high-speed filtering
	; up front.

        move.l  EP_SRCADDR(A1),IOS2_SRCADDR(A4)
        move.w  EP_SRCADDR+4(A1),IOS2_SRCADDR+4(A4)

        move.l  EP_DSTADDR(A1),IOS2_DSTADDR(A4)
        move.w  EP_DSTADDR+4(A1),IOS2_DSTADDR+4(A4)

	; Set flags correctly

        or.b    D6,IO_FLAGS(A4)		; Set flags as they would be...

	; Is this a RAW packet?

        btst	#SANA2IOB_RAW,IO_FLAGS(a4)
        bne.b	602$

        ; This is NOT a RAW packet, therefore adjust offset and length

        sub.l	#18,D2			; dec sizeof(hdr & crc)
        add.l	#14,a1			; Skip past hdr

        ; See if they are interested in this packet or not...

602$    move.l	a1,-(sp)		; save for later...

        movea.l	IOS2_BUFFERMANAGEMENT(a4),a6

        movem.l	d2-d7/a2-a6,-(sp)	; Save registers

        move.l	d2,IOS2_DATALENGTH(a4)	; Packet length in the IO Request itself

	movea.l	a4,a2			; The "Object" is the IO Request we'll use
					; The "Message" is the Packet Data in a1

	movea.l	BMS_PacketHook(a6),a0	; The Hook itself
	movea.l	h_Entry(a0),a6		; Fetch entry point
	jsr	(a6)			; Call it.

        movem.l	(sp)+,d2-d7/a2-a6	; Restore our registers
        move.l  SysBase(A5),A6
        tst.l	d0
        beq	670$			; They're not interested

	; Okay, now remove the IORequest from the list since we know
	; that they really want this packet.

	movea.l	a4,a1
	CALL	Remove

        ; A4 contains the IOB
        ; D3 contains the physical length
        ; A2 contains the Descriptor

	movea.l	(sp)+,a1		; Real packet data to use

	; RAW packets already taken into account (see above)

        move.l  IOS2_DATA(a4),a0                        * dest
        move.l  d2,d0                                   * count
        move.l  IOS2_BUFFERMANAGEMENT(a4),a6
        movea.l	BMS_CopyToBuff(a6),a6
        movem.l d2-d7/a2-a6,-(sp)
        jsr	(a6)
        movem.l (sp)+,d2-d7/a2-a6

        move.l  SysBase(A5),A6
        bsr     TermIO

660$    movea.l	(sp)+,a0		; Restore StackQueue pointer
	move.l	(sp),a4			; Restore PN Address
	clr.b	PN_PADDING(a4)		; Somone got the packet
        bra	560$			; Try next Stack

	;Remove any flags we might have set (that might not be true later)

670$    and.b	#255-(SANA2IOF_BCAST+SANA2IOF_MCAST),IO_FLAGS(a4)
	addq.l	#4,sp			; pop a1
	movea.l	(sp)+,a0		; restore stackqueue pointer
	bra	560$			; Try next stack

680$	; Put PN Address back in A4

        move.l  (SP)+,A4
        tst.b	PN_PADDING(a4)
        bne	272$

        add.l   D3,PN_TYPESTATS+S2PTS_RXBYTES(A4)
        addq.l  #1,PN_TYPESTATS+S2PTS_RXPACKETS(A4)

        bra     170$

999$    movem.l (SP)+,D2-D7/A2
        rts

;----------------------------------------------------------------------------
; TrmEvnt - Terminate/Signal An Event
;----------------------------------------------------------------------------
; IN:   A6      SysBase
;       A3      UnitBase
;       D0      Bit Mask Describing Which Lists To Access
;----------------------------------------------------------------------------
;----------------------------------------------------------------------------

TrmEvnt movem.l A2/A4/D7/D6,-(SP)

        move.l  D0,D7
        lea     UnitEventList(A3),A2
        cmp.l   MLH_TAILPRED(a2),a2       * list empty
        beq     99$
        move.l  MLH_HEAD(a2),a2
10$     move.l  LN_SUCC(a2),a4
        tst.l   LN_SUCC(a2)
        beq     19$
        move.l  IOS2_WIREERROR(a2),d6     * Clear the status
        and.l   d7,d6
        tst.l   d6
        beq     15$
        move.l  d6,IOS2_WIREERROR(a2)

* This one is done.  Remove it, and replymsg it.
        move.l  a2,a1
        CALL    Remove

        move.l  a2,A1
        clr.b   IO_ERROR(A1)
        CALL    ReplyMsg

15$     move.l  a4,a2
        bra     10$
19$


99$

        movem.l (SP)+,A2/A4/D7/D6
        rts

;----------------------------------------------------------------------------
; MissPkt - Routine to handle the case of the hardware missing a packet.
;----------------------------------------------------------------------------
; IN    A6      SysBase
;       A3      UnitBase
;       A2      ChipBase
;----------------------------------------------------------------------------
;----------------------------------------------------------------------------

MissPkt move.l  #s2eF_ERROR|s2eF_RX|s2eF_HARDWARE,D0
        bsr     TrmEvnt
        add.l   #1,UnitStats+S2DS_HARDMISSES(A3)
        bra.s   ErrReset

;----------------------------------------------------------------------------
; MErr - Handle the case of the hardware getting genuinely hosed.
;----------------------------------------------------------------------------
; IN    A6      SysBase
;       A3      UnitBase
;       A2      ChipBase
;----------------------------------------------------------------------------
;----------------------------------------------------------------------------

MErr    move.l  #s2eF_OFFLINE|s2eF_ERROR|s2eF_HARDWARE,D0
        bsr     TrmEvnt

        bsr     HaltInterface
        bset.b  #UFB_OFFLINE,UnitFlags(A3)
        bsr     AbAll
        bra.s   ErrReset


CErr
Babbler NOP

ErrReset
        move.w  #CSR0,LANCE_RAP(A2)
        move.w  #$7F00|CSR0F_INEA,LANCE_RDP(A2)
        rts

;----------------------------------------------------------------------------
; INEAOff
;----------------------------------------------------------------------------
; Turns the LANCE's interrupts OFF - a nice alternative to Disable()
;----------------------------------------------------------------------------
; A3    pointer to Unit Base
;----------------------------------------------------------------------------
INEAOff:
         move.w      #$0008,_intena
         addq.b      #1,UnitINEACount(a3)
         rts


;----------------------------------------------------------------------------
; INEAOn
;----------------------------------------------------------------------------
; Turns the LANCE's interrupts ON - a nice alternative to Enable()
;----------------------------------------------------------------------------
; A3    pointer to Unit Base
;----------------------------------------------------------------------------
INEAOn:
         subq.b      #1,UnitINEACount(a3)
         bne         1$
         move.w      #$8008,_intena
1$       rts


DName:
        dc.b         'a2065.device',0
        cnop    0,2
        VERS

DID:
        VSTRING
        cnop    0,2

        VERSTAG
        cnop    0,2

EndCode END
