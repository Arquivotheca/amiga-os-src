**
** A2065 SANA-II Device Driver source
** Copyright (c) 1992 by Commodore-Amiga, Inc.
**
** All rights Reserved.
**



        include "includes.asm"
        include "globals.i"
        include "a2065_defs.i"

        include "libraries/configregs.i"
        include "libraries/configvars.i"

        list

        XDEF    FindBds,RemBds
        XREF    DName,TheSoftInt

;
;       FindBds
;
;       On Entry:
;               A5 contains pointer to device
;
;       Register Usage:
;               D4      Counter of boards found
;               A3      Pointer to current ConfigDev structure
;               A2      Pointer To Newly Allocated Unit Structure
;
;       OUTPUT: D0      Count of the number of boards found and configured.

FindBds movem.l d1-d4/a0-A3/a6,-(sp)
        moveq   #0,D4
        move.l  D4,A3

10$     moveq.l #0,d3
        move.l  A3,A0
        move.l  #-1,D0
        moveq.l #-1,D1
        move.l  ExpansionBase(A5),A6
        CALL    FindConfigDev
        tst.l   D0
        beq     99$
        move.l  d0,a3

        move.w  cd_Rom+er_Manufacturer(a3),d1
        cmp.w   #MANUFACTURER,d1                    * C= A2065 board
        bne     17$
        move.b  cd_Rom+er_Product(a3),d1
        cmp.b   #PRODUCT,d1
        beq     150$

17$     move.w  cd_Rom+er_Manufacturer(a3),d1
        cmp.w   #MAN_AMERISTAR,d1
        bne     10$                                 * Ameristar
        move.b  cd_Rom+er_Product(a3),d1
        cmp.b   #PROD_AMERISTAR,d1
        bne     10$

150$
        btst    #CDB_CONFIGME,cd_Flags(a3)   * If already configured, don't do it again.
        beq     99$
        bclr    #CDB_CONFIGME,cd_Flags(a3)

        ; Board matching our requirements has been found.

        move.l  #UNIT_SIZE_,D0
        move.l  #MEMF_PUBLIC|MEMF_CLEAR,D1
        move.l  SysBase(A5),A6
        CALL    AllocMem
        tst.l   d0
        beq     99$

        ; A Unit data structure has been allocated. Now initialize it.

        move.l  D0,A2


        tst.b   d3
        beq     603$
        move.l  #MEMORY_OFFSET_500,MemoryOffset(a2)
        move.l  #CHIP_OFFSET_500,ChipOffset(a2)
        bra     604$
603$    move.l  #MEMORY_OFFSET,MemoryOffset(a2)
        move.l  #CHIP_OFFSET,ChipOffset(a2)
604$

        clr.l   UnitStats+S2DS_RECONFIGURATIONS(A2)

        move.l  A3,UnitConfigDev(a2)
        move.l  cd_BoardAddr(A3),A1
        move.l  A1,UnitBoard(A2)
        move.l  A1,A0

*        lea     CHIP_OFFSET(A0),A0
        add.l   ChipOffset(a2),a0
        move.l  A0,UnitChip(A2)

        add.l   MemoryOffset(a2),a1
*        add.l   #MEMORY_OFFSET,A1
        move.l  A1,UnitMemory(A2)

        move.l  A1,A0
        lea     LANCE_TX_BUFFER(A0),A0
        move.l  A0,UnitTxBuffer(A2)

        move.l  A1,A0
        lea     LANCE_RX_BUFFER(A0),A0
        move.l  A0,UnitRxBuffer(A2)

        move.l  A1,A0
        lea     LANCE_TX_RING(A0),A0
        move.l  A0,UnitTxDescriptor(A2)

        move.l  A1,A0
        lea     LANCE_RX_RING(A0),A0
        move.l  A0,UnitRxDescriptor(A2)

        ; Construct the default Ethernet address for this unit.

        move.w  cd_Rom+er_Manufacturer(a3),d0
        cmp.w   #MAN_AMERISTAR,d0
        bne     439$
        move.l  #AMERISTAR,UnitDefaultAddress(a2)       ; first three bytes
        bra     440$
439$    move.l  #COMMODORE,UnitDefaultAddress(A2)       ; first three bytes
440$    lea     UnitDefaultAddress+3(A2),A1
        lea     cd_Rom+er_SerialNumber+1(A3),A0
        move.b  (A0)+,(A1)+                             ; fourth byte
        move.b  (A0)+,(A1)+                             ; fifth byte
        move.b  (A0)+,(A1)+                             ; sixth byte

        ; Initialize This Units SoftInterrupt

        lea     UnitSoftInt(A2),A0
        move.b  #0,LN_PRI(A0)
        move.b  #NT_INTERRUPT,LN_TYPE(A0)
        move.l  #DName,LN_NAME(A0)
        clr.l   IS_DATA(A0)
        move.l  #TheSoftInt,IS_CODE(A0)

        ; Initialize The Protocol Nexus For 802 Style Packets and Orphans

        lea     Unit802PN(A2),A0
        NLIST   PN_RXQ(A0)

        lea     UnitOrfPN(A2),A0
        NLIST   PN_RXQ(A0)

        ; Initialize the MultiCast Address List

        NLIST   UnitMultiCastList(A2)

        ; Initialize the Transmission List

        NLIST   UnitTxList(A2)
        NLIST   UnitTxRunningList(A2)

        ; Initialize the OnEvent List

        NLIST    UnitEventList(A2)

        ; Initialize the ConfigIOB List

        NLIST   UnitConfigList(A2)

        ; Initialize the Protocol Hash Table

        bsr.s   InitPH

        ; Place this unit into the Unit Array

        lea     UnitArray(A5),A0
        move.l  D4,D0
        asl.l   #2,D0
        move.l  A2,0(A0,D0.l)

        ; Increment the count of the boards found

        addq.l  #1,d4
        cmp.l   #MAX_BOARDS,D4

        ; And loop again if we haven't found a box full of Ethernet boards.

        bne     10$

99$     move.l  D4,D0
        move.l  D4,UnitCount(a5)
        movem.l (sp)+,a0-A3/a6/d1-d4
        rts

;---------------------------------------------------------------------------
; InitPH - Initializes a Unit's ProtocolHashTable.
;---------------------------------------------------------------------------
; INPUT:        A2      Pointer To A Newly Allocated Unit Structure
;
;---------------------------------------------------------------------------

InitPH  movem.l A2/D2,-(SP)
        lea     UnitProtocolHashTable(A2),A2
        move.w  #HASH_SIZE-1,D2
100$    NEWLIST A2
        lea     MLH_SIZE(A2),A2
        dbf     D2,100$
        movem.l (SP)+,A2/D2
        rts

RemBds  movem.l D2/A2,-(sp)

        lea     UnitArray(A5),A2
        move.l  SysBase(A5),A6
        moveq.l #MAX_BOARDS,D2

0$      tst.l   (A2)
        beq.s   99$

        move.l  (a2),a1
        move.l  UnitConfigDev(a1),a1

        bset    #CDB_CONFIGME,cd_Flags(a1)     * Allow others to config it

        move.l  (A2),A1
        move.l  #UNIT_SIZE_,d0
        CALL    FreeMem

        clr.l   (A2)+
        subq.l  #1,D2
        bne.s   0$

99$     movem.l (sp)+,D2/A2
        rts

        end
