**
** A2065 SANA-II Device Driver source
** Copyright (c) 1992 by Commodore-Amiga, Inc.
**
** All rights Reserved.
**

;DEBUG_MODE     EQU     1

        nolist

        include "includes.asm"
        include "globals.i"
        include "a2065_defs.i"

        list

        XREF    TermIO
        XREF    GetTime
        XDEF    ConfigI,HaltInterface,CnfgBd

        INT_ABLES

        CNOP    2

;----------------------------------------------------------------------------
; ConfigI - Completely configures the hardware and gets things going.
;----------------------------------------------------------------------------
; IN:   A6      SysBase
;       A5      DeviceBase
;       A4      IOB
;       A3      Unit
;       A2      DeviceMemory
;
;----------------------------------------------------------------------------
; Notes:
;
; 1. This routine creates the memory architecture in A2065 ram including
;    the initialization block which includes the physical address. This
;    routine can assume that NO MULTICAST ADDRESSES ARE DEFINED since
;    this ADDMULTICASTADDRESS cannot be called until after this function
;    in called.
;
; 2. Make sure you remember to take predefined multicast addressess into
;    account when doing a UNIT_ONLINE which also reinitializes the hardware.
;----------------------------------------------------------------------------

ConfigI

        CALL    Forbid

        ; If the physical address is already configured, then we
        ; should not be here.

        btst    #UFB_PAV,UnitFlags(A3)
        beq.s   10$

        move.b  #S2ERR_BAD_STATE,IO_ERROR(A4)
        move.l  #S2WERR_IS_CONFIGURED,IOS2_WIREERROR(A4)
        bra.s   999$

10$     ; Make sure we aren't passing a multicast address off as
        ; a physical address.

        lea     IOS2_SRCADDR(A4),A0

        btst.b  #0,(A0)
        beq.s   20$

15$     move.b  #S2ERR_BAD_ADDRESS,IO_ERROR(A4)
        clr.l   IOS2_WIREERROR(A4)
        bra.s   999$

20$     ; Make sure we aren't trying to pass a broadcast address
        ; off as a physical address

        move.l  (A0),D0
        move.w  4(A0),D1

        cmp.l   #-1,D0
        bne.s   30$

        cmp.w   #-1,D1
        beq.s   15$

30$     ; Make sure we are online.

        btst    #UFB_OFFLINE,UnitFlags(A3)
        beq.s   40$

        move.b  #S2ERR_OUTOFSERVICE,IO_ERROR(A4)
        move.l  #S2WERR_UNIT_OFFLINE,IOS2_WIREERROR(A4)
        bra.s   999$

40$     ; D0.L and D1.W contain a valid Ethernet Address

* Welcome to the WILD WILD world of ethernet.
* Because of popular support, the following two lines
* prevent you from setting the ethernet address to anything
* other than the default address.  Hurrah!
***
*        move.l  UnitDefaultAddress(a3),d0
*        move.w  UnitDefaultAddress+4(a3),d1
        move.l  d0,IOS2_SRCADDR(a4)
        move.w  d1,IOS2_SRCADDR+4(a4)

***


        move.l  D0,UnitCurrentAddress(A3)
        move.w  D1,UnitCurrentAddress+4(A3)

        ; Set the Configuration Time

        addq.l  #1,UnitStats+S2DS_RECONFIGURATIONS(A3)
        lea     UnitStats+S2DS_LASTSTART(A3),A0
        bsr     GetTime

        ; Everything's Set. Call the routine to poke hardware!

        bra.s   CnfgBd

999$
        CALL    Permit
        bra     TermIO

;----------------------------------------------------------------------------
; CnfgBd - The Routine Which Actually Pokes Hardware To Config The Board
;----------------------------------------------------------------------------
; IN:   A6      SysBase
;       A5      DeviceBase
;       A4      IO Request Block
;       A3      Unit
; USE   A2      MemoryBase Or ChipBase
;----------------------------------------------------------------------------
; NOTES:
;
; 1. This routine will be called from two places. First, it will be called
;    from  SANA2CMD_CONFIGINTERFACE.   Second,  it  will  be  called  from
;    SANA2CMD_ONLINE.  In both cases,  the hardware is assumed to be in an
;    unknown state and all aspects of the hardware will be reprogrammed.
;
; 2. THIS ROUTINE ASSUMES ENTRY IN A FORBIDDEN STATE!!!!!!!!!!!!!!!!!!!!!!
;
;----------------------------------------------------------------------------


CnfgBd  move.l  A2,-(SP)

        ; Clear the QUICK bit from the IOB's flags if it's there
        ; as this IO will not be complete when this function returns.

        bclr.b  #IOB_QUICK,IO_FLAGS(A4)
        move.b  #0,LN_TYPE(A4)

        ; Lets Start Poking!

        DISABLE

        ; A2 Has ChipBase Now

        move.l  UnitChip(A3),A2

        ; STOP The LANCE

        move.w  #CSR0,LANCE_RAP(A2)
        move.w  #CSR0F_STOP,LANCE_RDP(A2)

        CALL    Enable

        ; Enable BYTE Swapping Of Data

        move.w  #CSR3,LANCE_RAP(A2)
        move.w  #CSR3F_BSWP,LANCE_RDP(A2)

        ; Specify The Address Of The InitBlock (MEMORY_OFFSET)

        move.w  #CSR1,LANCE_RAP(A2)

        move.w  MemoryOffset+2(a3),LANCE_RDP(a2)
*        move.w  #MEMORY_OFFSET,LANCE_RDP(A2)
        move.w  #CSR2,LANCE_RAP(A2)
        move.w  MemoryOffset(a3),LANCE_RDP(a2)
*        move.w  #0,LANCE_RDP(A2)

        ; Specify The Mode Field Of The InitBlock
        ; Here is where PROMISCUOUS MODE might be set.

        moveq.l #0,D0
        btst    #UFB_PROM,UnitFlags(A3)
        beq.s   40$
        bset    #MODEB_PROM,D0

40$     ; Debugging Lines

        ; A2 Changes To MemoryBase Now

        move.l  UnitMemory(A3),A2
        move.w  D0,LANCE_MODE(A2)

        ; Specify The Physical Address Field

        lea     UnitCurrentAddress(A3),A0

        move.b  1(A0),LANCE_PA+0(A2)
        move.b  0(A0),LANCE_PA+1(A2)
        move.b  3(A0),LANCE_PA+2(A2)
        move.b  2(A0),LANCE_PA+3(A2)
        move.b  5(A0),LANCE_PA+4(A2)
        move.b  4(A0),LANCE_PA+5(A2)

        ; Specify The Logical Address Field

        move.l  UnitLogicalAddress(A3),LANCE_LA(A2)
        move.l  UnitLogicalAddress+4(A3),LANCE_LA+4(A2)

        ; Specify The Address Of The Receive Descriptor Ring
        ; See Memory Map in A2065.i.

        move.w  #RXRING_0,LANCE_RX_RING_POINTER(A2)
        move.w  #RXRING_2,LANCE_RX_RING_POINTER+2(A2)

        ; Specify The Address Of The Transmit Descriptor Ring
        ; See Memory Map in A2065.i.

        move.w  #TXRING_0,LANCE_TX_RING_POINTER(A2)
        move.w  #TXRING_2,LANCE_TX_RING_POINTER+2(A2)

        ; Now initialize the one Transmit Descriptor

        lea     LANCE_TX_RING(A2),A0

        ; A0 now points to the first entry in the transmit
        ; descriptor ring.

        move.w  #TXBUF_COUNT-1,d1
        move.l  MemoryOffset(a3),a1
        add.l   #LANCE_TX_BUFFER,a1
*        move.l  #MEMORY_OFFSET+LANCE_TX_BUFFER,a1
        move.w  #-TXBUFLEN,d0
        or.w    #$F000,d0

99$     move.w  a1,(a0)+
        move.w  #TMD1F_ENP|TMD1F_STP,(A0)+
        move.w  d0,(a0)+
        clr.w   (A0)+
        lea.l   TXBUFLEN(a1),a1
        dbra    d1,99$

        ; A0 now points to the first Receive Descriptor. Loop
        ; through all of the receive descriptors, initializing
        ; them as you go.

        move.w  #RXBUF_COUNT-1,D1
        move.l  MemoryOffset(a3),a1
        add.l   #LANCE_RX_BUFFER,a1
*        move.l  #MEMORY_OFFSET+LANCE_RX_BUFFER,A1
        move.w  #-RXBUFLEN,D0
        or.w    #$F000,D0

100$    move.w  A1,(A0)+
        move.w  #RMD1F_OWN,(A0)+
        move.w  D0,(A0)+
        clr.w   (A0)+
        lea     RXBUFLEN(A1),A1

        dbf     D1,100$

        DISABLE

        ; If IOS2_UNIT doesn't match A3 then this is being called from
        ; the OPEN routine. Therefore, don't put the IOB on the special
        ; list so that is DOESN't get TermIO'd

        cmp.l   IO_UNIT(A4),A3
        bne.s   150$

        ; Everything is ready. Stick this IO Request Block onto
        ; the special list kept by the Unit structure. When the
        ; IDONE interrupt occurs, this IOB will be terminated.

        lea     UnitConfigList(A3),A0
        move.l  A4,A1
        CALL    AddTail

        ; Set the physical address valid flag!

150$    bset    #UFB_PAV,UnitFlags(A3)

        ; And tell the read interrupt service routine to start from
        ; the very first descriptor.

        clr.l   UnitNRD(A3)
        clr.l   UnitNWFill(A3)
        move.l  #-1,UnitNWDone(a3)

        ; OK - Tell the damn LANCE we're ready to rock and roll.

        move.l  UnitChip(A3),A2
        move.w  #CSR0,LANCE_RAP(A2)
        move.w  #CSR0F_INIT|CSR0F_INEA,LANCE_RDP(A2)

        CALL    Enable

        CALL    Permit

        move.l  (SP)+,A2
        rts

;----------------------------------------------------------------------------
; HaltInterface - A way of stopping the network interface cold.
;----------------------------------------------------------------------------
; IN:   A3      UnitBase
;----------------------------------------------------------------------------
; NOTES:
;
; 1. This routine is called from the CloseDevice function when a unit
;    becomes completely idle. This function is intended to stop the
;    network interface dead in its tracks so that it may be removed.
;----------------------------------------------------------------------------

HaltInterface
        move.l  UnitChip(A3),A0
        move.w  #CSR0,LANCE_RAP(A0)
        move.w  #CSR0F_STOP,LANCE_RDP(A0)
        rts

        END
