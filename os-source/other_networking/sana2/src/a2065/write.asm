**
** A2065 SANA-II Device Driver source
** Copyright (c) 1992 by Commodore-Amiga, Inc.
**
** All rights Reserved.
**



;DEBUG_MODE     EQU     1
;NO_NETBUFS     EQU     1

        nolist

        include "includes.asm"
        include "globals.i"
        include "a2065_defs.i"

        list

        XDEF    Write,BCast,MCast,WrtNxt
        XREF    TermIO,INEAOff,INEAOn,_intenar

        INT_ABLES

        CNOP    2

;----------------------------------------------------------------------------
; BCast - Transmit a block over the ethernet using the Broadcast address
;----------------------------------------------------------------------------
; IN:   A6      SysBase
;       A5      DeviceBase
;       A4      IO Request Block
;       A3      UnitBase
;----------------------------------------------------------------------------
; NOTES:
;
;----------------------------------------------------------------------------

BCast   move.l  #-1,IOS2_DSTADDR(A4)
        move.w  #-1,IOS2_DSTADDR+4(A4)
        bra.s   Write


;----------------------------------------------------------------------------
; MCast - Transmit a block over the ethernet using a MultiCast address
;----------------------------------------------------------------------------
; IN:   A6      SysBase
;       A5      DeviceBase
;       A4      IO Request Block
;       A3      UnitBase
;----------------------------------------------------------------------------
; NOTES:
;
;----------------------------------------------------------------------------

MCast   btst.b  #0,IOS2_DSTADDR(A4)
        bne.s   Write

        move.b  #S2ERR_BAD_ADDRESS,IO_ERROR(A4)
        move.l  #S2WERR_BAD_MULTICAST,IOS2_WIREERROR(A4)
        bra     TermIO

;----------------------------------------------------------------------------
; Write - Transmit a block over the ethernet.
;----------------------------------------------------------------------------
; IN:   A6      SysBase
;       A5      DeviceBase
;       A4      IO Request Block
;       A3      UnitBase
;----------------------------------------------------------------------------
; NOTES:
;
; 1. I can generate the following errors:
;       BAD_STATE       NOT_CONFIGURED          if called when not configured
;       BAD_STATE       UNIT_OFFLINE            if called when offline
;       MTU_EXCEEDED                            if write length is too big
;       BAD_PROTOCOL    NULL_PTYPE              if ptype is null
;       BAD_PROTOCOL
;----------------------------------------------------------------------------

Write

        btst.b  #UFB_PAV,UnitFlags(A3)
        bne.s   10$

        move.b  #S2ERR_BAD_STATE,IO_ERROR(A4)
        move.l  #S2WERR_NOT_CONFIGURED,IOS2_WIREERROR(A4)
        bra     990$

10$     btst.b  #UFB_OFFLINE,UnitFlags(A3)
        beq.s   20$

        move.b  #S2ERR_OUTOFSERVICE,IO_ERROR(A4)
        move.l  #S2WERR_UNIT_OFFLINE,IOS2_WIREERROR(A4)
        bra     990$

20$     ; Make sure the length of this write is not too large.

        move.l  IOS2_DATALENGTH(A4),D0

        cmp.w   #1500,D0
        ble.s   30$

        ; The packet to be transmitted is too large.

        move.b  #S2ERR_MTU_EXCEEDED,IO_ERROR(A4)
        clr.l   IOS2_WIREERROR(A4)
        bra     990$

30$     ; The packet is not too big - Check Its Protocol Type
        ; Notice this is done in a way which guards against reading
        ; low memory due to a null IOS2_PACKETTYPE.

        move.l  #S2WERR_NULL_POINTER,IOS2_WIREERROR(A4) ; pessimistic
        move.l  IOS2_PACKETTYPE(A4),D0

        clr.l   IOS2_WIREERROR(A4)

33$     clr.l   IOS2_WIREERROR(A4)


35$     ; Clear The QUICK Flag As We Are Accepting This Transmission.

        bclr    #IOB_QUICK,IO_FLAGS(A4)
        move.b  #0,LN_TYPE(A4)

        CALL    Forbid              * Don't allow parallel tasks to fumble with the queue while we are.
        jsr     INEAOff
        move.l  UnitNWDone(a3),d0
        cmp.l   UnitNWFill(a3),d0
        bne     400$
* The buffers are full, if Fill = Done.  Queue it on the list, and it'll
* get transmitted when a buffer frees up.

        CALL    Disable             * Don't allow IntSvc's TxDone to munge the list
        lea     UnitTxList(A3),A0
        move.l  A4,A1
        CALL    AddTail
        CALL    Enable
        jsr     INEAOn
        CALL    Permit
        bra.s   999$

400$
        tst.l   d0
        bge     410$
        move.l  UnitNWFill(a3),UnitNWDone(a3)
410$    jsr     WrtNxt

        CALL    Disable
        lea.l   UnitTxRunningList(a3),a0
        move.l  a4,a1
        CALL    AddTail
        CALL    Enable

        move.l  UnitNWFill(a3),d0
        addq.l  #1,d0
        and.l   #W_BUFF_MASK,d0           * wraparound
        move.l  d0,UnitNWFill(a3)
        jsr     INEAOn
        CALL    Permit
        bra.s   999$


990$    bsr     TermIO
999$    rts

;----------------------------------------------------------------------------
; WrtNxt - Cause The IOB Pointed To By A4 To Be Transmitted
;----------------------------------------------------------------------------
; IN:   A6      SysBase
;       A5      DeviceBase
;       A4      IOB
;       A3      UnitBase
; USE:  D7      Buffers Size
;----------------------------------------------------------------------------
; NOTES:
;
; 1. This routine is called from the user level write routine as well as
;    from the software interrupt called from the interrupt (txdone) level.
; 2. I am really pleased with the fact that this function can be executed
;    without FORBIDDING or DISABLING! That is, the time consuming chore
;    of filling the hardware for transmission can be done without impacting
;    reads from the Ethernet as well as the system general. Oxxi bergnippit.
;----------------------------------------------------------------------------

WrtNxt  movem.l A2/D7/D6,-(SP)

        move.l  UnitChip(a3),a2
        move.w  #CSR0,LANCE_RAP(a2)
        move.w  LANCE_RDP(a2),d0
        move.w  _intenar,d1

        move.l  UnitTxDescriptor(A3),A1
        move.l  UnitNWFill(a3),d0
        asl.l   #3,d0
        add.l   d0,a1
        move.l  a1,d6         * save
        move.l  UnitBoard(a3),a2
        moveq.l #0,d0
        move.w  TMD0(a1),d0
        add.l   d0,a2
        move.l  a2,a1

        moveq.l #0,D7

        ; Get Transmit Length Into D7

        move.l  IOS2_DATALENGTH(A4),D7

        ; Packet Length Is Now Buffered In D7

        ; Before checking on the packet type, see if this IOB is supposed
        ; to be sent ``raw''. If so, everything is supposed to be in the
        ; user's data area already including the data link layer header.

        btst    #SANA2IOB_RAW,IO_FLAGS(A4)
        bne.s   5$

        ; This is not a ``raw'' packet. Therefore, we must assemble
        ; the data link layer header.

        move.l  IOS2_PACKETTYPE(A4),D0
        cmp.l   #1500,d0
        bhi     0$

        ; This is an 802 type packet. Instead of packet type as
        ; given by MATCH, use the length as an Ethernet packet
        ; type. Notice we move in the packet length WITHOUT the
        ; addition of the Ethernet Header length.

        move.w  D7,EP_PTYPE(A1)
        bra.s   1$

0$      move.w  d0,EP_PTYPE(a1)

1$
        move.l  UnitCurrentAddress(A3),EP_SRCADDR(A1)
        move.w  UnitCurrentAddress+4(A3),EP_SRCADDR+4(A1)
        move.l  IOS2_DSTADDR(A4),EP_DSTADDR(A1)
        move.w  IOS2_DSTADDR+4(A4),EP_DSTADDR+4(A1)

        ; Make A1 a pointer to the first data byte location

        lea     EP_SIZE(A1),A1

        ; Now transfer the packet to the hardware.

5$

        IFD     NO_NETBUFS

        ; Compute the number of 256 byte blocks to be sent

        move.l  D7,D0
        add.w   #255,D0
        lsr.w   #8,D0
        subq.w  #1,D0
        and.w   #7,D0

        ; Get a pointer to the user data into A0

        move.l  IOS2_STATDATA(A4),A0

        ;       Actually transfer data to the board.

        movem.l A2-A6/D2-D7,-(SP)

10$     bsr.s   MoveBlock
        dbf     D0,10$

        movem.l (SP)+,A2-A6/D2-D7

        ENDC

        IFND    NO_NETBUFS

        ; A1 points to where the bytes have to go.
        ; D7 has the number of bytes to transfer

        move.l    d7,d0
        move.l    a1,a0              * dest
        move.l    IOS2_DATA(a4),a1   * src
        move.l    IOS2_BUFFERMANAGEMENT(a4),a6
        movea.l	  BMS_CopyFromBuff(a6),a6
        movem.l   a2-a6/d2-d7,-(sp)
        jsr	  (a6)
        movem.l   (sp)+,a2-a6/d2-d7

        move.l    SysBase(A5),A6

        ENDC

        ; Data is now on the board all ready to go. Locate
        ; the transmit buffer ring and fill in the one
        ; descriptor.

        move.l  d6,a0

        ; Get the length of the packet into a register again.
        ; And adjust for the Ethernet Protocol Header Length.

        move.l  D7,D0
        btst    #SANA2IOB_RAW,IO_FLAGS(A4)
        bne.s   19$
        add.l   #EP_SIZE,D0

19$     cmp.w   #64,D0
        bge.s   20$
        move.w  #64,D0
20$     and.w   #$0FFF,D0
        neg.w   D0
        move.w  D0,4(A0)
        clr.w   6(A0)
        move.w  #TMD1F_ENP|TMD1F_STP|TMD1F_OWN,2(A0)
        move.l  UnitChip(a3),a2
        CALL    Disable
        move.w  #CSR0,LANCE_RAP(a2)
        move.w  #CSR0F_INEA|CSR0F_TDMD,LANCE_RDP(a2)
        CALL    Enable
        sub.l   a0,a0         * If someone's using these, cause ugly enforcer hits
        sub.l   a2,a2         *
        movem.l (SP)+,D7/A2/D6
        rts


        IFD     NO_NETBUFS

;       MoveBlock       Move a 256 byte block quickly
;
;       Copyright 1987 by ASDG Incorporated.
;
;       ASDG Recoverable Ram Disk Release 4
;
;       A0      Source Address
;       A1      Destination Address
;
;       Preserves No Registers
;
;       Change History
;
;       1.      File Created.
;       2.      Changed From 512 to 256 Bytes For The EtherNet Driver
;               And Made NOT To Save ALL Registers!
;       3.      Changed adda.l to lea with A1.
;

        XDEF    MoveBlock

MoveBlock

        movem.l D0/A2,-(SP)

        movem.l (a0)+,a2-a6/d0-d7
        movem.l a2-a6/d0-d7,(a1)        ; moves to 0 from original A1
        lea     52(A1),a1

        movem.l (a0)+,a2-a6/d0-d7       ; moves to 52 from original A1
        movem.l a2-a6/d0-d7,(a1)
        lea     52(A1),a1

        movem.l (a0)+,a2-a6/d0-d7       ; moves to 104 from original A1
        movem.l a2-a6/d0-d7,(a1)
        lea     52(A1),a1

        movem.l (a0)+,a2-a6/d0-d7       ; moves to 156 from original A1
        movem.l a2-a6/d0-d7,(a1)
        lea     52(A1),a1

        movem.l (a0)+,a2-a6/d0-d6       ; moves to 208 from original A1
        movem.l a2-a6/d0-d6,(a1)
        lea     48(A1),A1

        movem.l (SP)+,D0/A2
        rts

        ENDC

        END
