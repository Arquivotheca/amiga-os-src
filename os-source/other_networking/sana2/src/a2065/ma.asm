**
** A2065 SANA-II Device Driver source
** Copyright (c) 1992 by Commodore-Amiga, Inc.
**
** All rights Reserved.
**


;DEBUG_MODE     EQU     1

        CODE

        NOLIST

        include "includes.asm"
        include "globals.i"
        include "a2065_defs.i"

        LIST

        INT_ABLES

        XDEF    AddMult,DelMult,RemMCA,LocMCN
        XREF    TermIO,CnfgBd,TrmEvnt

;----------------------------------------------------------------------------
;AddMult - Enables A MultiCast Address
;----------------------------------------------------------------------------
;IN:    A6      SysBase
;       A5      DeviceBase
;       A4      IO Request Block
;       A3      Unit
;USAGE: A2      Pointer To A MultiCast Address Node
;       D7      The Hashed Bit
;----------------------------------------------------------------------------
;NOTES:
;
;1. Looks for address to already exist in list. If it's there...then add
;   to its open count.  If it isn't there, then allocate a list node and
;   add it to the list, then hash it to see which bit it corresponds to.
;   Then, look the bit up in the list,  if it is found,  then do nothing
;   more. If it isn't found, then diddle the hardware to enable the bit.
;
;2. Needs to be single threaded.
;
;3. This command touches the hardware, therefore...it will reject calls
;   made to it while the unit is offline.
;
;4. We can perform a basic check of the multicast address...the 0 bit of
;   the first address byte must be a 1. If not, it isn't a multicast
;   address.
;
;5. If we can't allocate a multicast addressing node, then return
;   the NO_RESOURCES error and wake up anyone waiting on the SOFTWARE
;   error event.
;----------------------------------------------------------------------------

AddMult movem.l A2/D7/D6,-(SP)

        moveq.l #0,D6

        CALL    Forbid

        btst.b  #UFB_OFFLINE,UnitFlags(A3)
        beq.s   0$

        ; Unit is offline right now - reject this command

        move.b  #S2ERR_OUTOFSERVICE,IO_ERROR(A4)
        move.l  #S2WERR_UNIT_OFFLINE,IOS2_WIREERROR(A4)
        bra     999$

0$      ; Unit is online - has it been configured?

        btst.b  #UFB_PAV,UnitFlags(A3)
        bne.s   1$

        move.b  #S2ERR_BAD_STATE,IO_ERROR(A4)
        move.l  #S2WERR_NOT_CONFIGURED,IOS2_WIREERROR(A4)
        bra     999$

1$      ; Unit is configured - do simple multicast address check

        lea     IOS2_SRCADDR(A4),A0
        btst.b  #0,(A0)
        bne.s   5$

        move.b  #S2ERR_BAD_ADDRESS,IO_ERROR(A4)
        move.l  #S2WERR_BAD_MULTICAST,IOS2_WIREERROR(A4)
        bra     999$

5$      ; Look for this address already in the list

        bsr     LocMCN
        tst.l   D0
        beq.s   10$

        ; The address is already enabled as a multicast address.

        move.l  D0,A0
        addq.w  #1,MCN_OpenCount(A0)
        bra     998$

10$     ; The address is not in the table...allocate a node for it.

        move.l  #MCN_SIZE,D0
        move.l  #MEMF_PUBLIC|MEMF_CLEAR,D1
        CALL    AllocMem
        tst.l   D0
        bne.s   20$

        ; Allocation of a node failed, report an error.

        move.b  #S2ERR_NO_RESOURCES,IO_ERROR(A4)
        clr.l   IOS2_WIREERROR(A4)

        move.l  #s2eF_SOFTWARE|s2eF_ERROR,D0
        bsr     TrmEvnt

        bra     999$

20$     ; Allocation of a node succeded - Initialize It

        move.l  D0,A2

        ; First time through, put the bytes into the MCNode
        ; swapped so that the HASH function can work properly.

        move.b  IOS2_SRCADDR+1(A4),MCN_Address+0(A2)
        move.b  IOS2_SRCADDR+0(A4),MCN_Address+1(A2)
        move.b  IOS2_SRCADDR+3(A4),MCN_Address+2(A2)
        move.b  IOS2_SRCADDR+2(A4),MCN_Address+3(A2)
        move.b  IOS2_SRCADDR+5(A4),MCN_Address+4(A2)
        move.b  IOS2_SRCADDR+4(A4),MCN_Address+5(A2)

        addq.w  #1,MCN_OpenCount(A2)

        ; Now find out which bit needs to be set on the hardware.

        lea     MCN_Address(A2),A0
        bsr     Hash
        move.l  D0,D7
        move.b  D0,MCN_Bit(A2)

        ; Set the address up properly now.

        move.l  IOS2_SRCADDR(A4),MCN_Address(A2)
        move.w  IOS2_SRCADDR+4(A4),MCN_Address+4(A2)

        ; Look through the list for this bit

        move.l  D7,D0
        bsr     LocMCBt
        tst.l   D0
        bne.s   30$

        ; The bit was not found in the list. Therefore, it needs to
        ; be enabled in the hardware.

        moveq.l #1,D6

        move.b  D7,D0
        lsr.b   #3,D0
        and.l   #7,D0

        ; D0 now contains the byte in which the bit should be set.
        ; But, the logical address fields on the AMD are byteswapped.
        ; Therefore, if D0 is even, add one. If it is odd - subtract 1.

        btst    #0,D0
        beq.s   21$

        ; D0 is Odd. Subtract 1.

        subq.l  #1,D0
        bra.s   22$

21$     ; D0 is Even. Add 1.

        addq.l  #1,D0

22$     lea     UnitLogicalAddress(A3),A0
        add.l   D0,A0
        and.l   #7,D7
        bset.b  D7,(A0)

30$     ; Add the node to the list and we're done.

        CALL    Disable
        lea     UnitMultiCastList(A3),A0
        move.l  A2,A1
        CALL    AddTail
        CALL    Enable


998$

999$    tst.l   D6
        bne.s   AdMult2

        ; This is the exit case when a bit didn't get twiddled.

        bsr     TermIO
        CALL    Permit
        movem.l (SP)+,A2/D7/D6
        rts


AdMult2 ; A bit has been twiddled. We have to do some grody stuff
        ; on the hardware since the AMD only reads the logical address
        ; block at initialization time.


        CALL    Disable

        lea     UnitTxList(A3),A0
        cmp.l   LH_TAILPRED(A0),A0
        beq.s   10$

        ; The device is currently transmitting.

        bset.b  #UFB_HALT_WRITES,UnitFlags(A3)
        CALL    Enable

5$      btst.b  #UFB_HALT_WRITES,UnitFlags(A3)
        bne.s   5$
        bra.s   20$

10$     ; The device is not currently writing.

        CALL    Enable

20$     movem.l (SP)+,A2/D7/D6

        ; We jump to CnfgBd WHILE FORBIDDEN!

        bra     CnfgBd

;----------------------------------------------------------------------------
;Hash - Ethernet Address Hashing (Fast CRC-32)
;----------------------------------------------------------------------------
;IN:    A0      Points to the first byte of the address to be hashed.
;OUT:   D0      The hash result.
;USAGE: D1      Loop Counter (Each Byte)
;       D3      Loop Counter (Each Bit)
;       D4      Each Byte To Be CRC'ed
;       D5      Flag (MSB ON Or OFF)
;       A0      Is updated as bytes are hashed.
;----------------------------------------------------------------------------

POLYNOMIAL      EQU     $4C11DB6

; USE:  D0      Accumulator (AX,DX)
;       D1      Word Counter (CH)
;       D2      Bit Counter (CL)
;       D3      A Word From The Address To Hash (BP)
;       D4      Miscellaneous (BX)
;       D7      Polynomial
;       A0      Pointer To Address To Hash (S1)
;

Hash    movem.l A0-A6/D1-D7,-(SP)       ; save all registers
        moveq.l #-1,D0                  ; preset crc accumulator to all 1's
        move.l  #POLYNOMIAL,D7          ;
        moveq.l #2,D1                   ; initialize word counter (CH)

SETH10  move.w  (A0)+,D3                ; mov   BP,[S1]
                                        ; add   S1,1
        move.w  #15,D2                  ; initialize bit counter (CL)

SETH20  move.l  D0,D4                   ; mov   BX,DX
        swap    D4                      ;
        and.l   #$FFFF,D4               ;
        rol.w   #1,D4                   ; rol   BX,1
        eor.w   D3,D4                   ; xor   BX,BP
        lsl.l   #1,D0                   ; sal   AX,1
                                        ; rcl   DX,1
        and.w   #1,D4                   ; and   BX,0001H
        beq.s   SETH30                  ; jz    SETH30

        eor.l   D7,D0                   ; xor   AX,POLYL
                                        ; xor   DX,POLYH

SETH30  or.w    D4,D0                   ; or    AX,BX
        ror.w   #1,D3                   ; ror   BP,1
        dbf     D2,SETH20               ; dec   CL
                                        ; jnz   SETH20
        dbf     D1,SETH10               ; dec   CH
                                        ; jnz   SETH10

        ; D0(0:7) now contains the hash code but the bits are in
        ; reverse order?

        and.l   #$3F,D0
        lsl.b   #2,D0
        moveq.l #5,D1
        moveq.l #0,D2

10$     lsl.b   #1,D0
        bcc.s   20$
        bset.b  #7,D2
20$     lsr.b   #1,D2
        dbf     D1,10$
        lsr.b   #1,D2
        move.l  D2,D0

        movem.l (SP)+,A0-A6/D1-D7
        rts

;----------------------------------------------------------------------------
; DelMult - Remove A MultiCast Address From The Current List
;----------------------------------------------------------------------------
; IN:   A6      SysBase
;       A5      DeviceBase
;       A4      IO Request Block
;       A3      UnitBase
; OUT:
; USED: A2      Pointer to the node we are deleting.
;       D6      Flag specifying if a bit was twiddled.
;       D7      Not really used but must be saed.
;----------------------------------------------------------------------------
; NOTES:
;
; 1. Look the given address up in the list. If it isn't there, then report
;    an error. If it is there, decrement its open count. If the count has
;    reached 0, then remove it from the list and deallocate it while
;    remembering which bit it used. Look this bit up elsewhere in the list.
;    If the bit is found, then exit. Else, (if the bit is not found elsewhere
;    in the list) then remove it from the hardware.
;----------------------------------------------------------------------------

DelMult movem.l A2/D6/D7,-(SP)
        moveq.l #0,D6

        CALL    Forbid

        btst.b  #UFB_OFFLINE,UnitFlags(A3)
        beq.s   0$

        ; Unit is offline right now - reject this command

        move.b  #S2ERR_OUTOFSERVICE,IO_ERROR(A4)
        move.l  #S2WERR_UNIT_OFFLINE,IOS2_WIREERROR(A4)
        bra     999$

0$      ; Unit is online - but is it configured yet?

        btst.b  #UFB_PAV,UnitFlags(A3)
        bne.s   1$

        move.b  #S2ERR_BAD_STATE,IO_ERROR(A4)
        move.l  #S2WERR_NOT_CONFIGURED,IOS2_WIREERROR(A4)
        bra     999$

1$      ; Unit is configured - do simple multicast address check

        lea     IOS2_SRCADDR(A4),A0
        btst.b  #0,(A0)
        bne.s   5$

        move.b  #S2ERR_BAD_ADDRESS,IO_ERROR(A4)
        move.l  #S2WERR_BAD_MULTICAST,IOS2_WIREERROR(A4)
        bra     999$

5$      ; Look for this address already in the list

        bsr     LocMCN
        tst.l   D0
        bne.s   10$

        ; The address is not enabled as a multicast address.

        move.b  #S2ERR_UNKNOWN_ENTITY,IO_ERROR(A4)
        move.l  #S2WERR_BAD_MULTICAST,IOS2_WIREERROR(A4)
        bra     999$

10$     ; The address is in the table...decrement its OpenCount.

        move.l  D0,A2
        subq.w  #1,MCN_OpenCount(A2)

        bne.s   999$

        ; The OpenCount of this node has reached zero. See if the bit
        ; is still being used. If it isn't then remove the bit. The
        ; node will be removed and deallocated regardless.

        DISABLE
        move.l  A2,A1
        CALL    Remove
        CALL    Enable

        move.b  MCN_Bit(A2),D0

        ; Look through the list for this bit

        bsr     LocMCBt
        tst.l   D0
        bne.s   30$

        ; The bit was not found in the list. Therefore, it needs to
        ; be disabled in the hardware.

        moveq.l #1,D6

        move.b  MCN_Bit(A2),D1
        move.b  D1,D0
        lsr.b   #3,D0
        and.l   #7,D0

        ; D0 now contains the byte in which the bit should be set.
        ; But, the logical address fields on the AMD are byteswapped.
        ; Therefore, if D0 is even, add one. If it is odd - subtract 1.

        btst    #0,D0
        beq.s   21$

        ; D0 is Odd. Subtract 1.

        subq.l  #1,D0
        bra.s   22$

21$     ; D0 is Even. Add 1.

        addq.l  #1,D0

22$     lea     UnitLogicalAddress(A3),A0
        add.l   D0,A0
        and.l   #7,D1
        bclr.b  D1,(A0)

30$     ; Deallocate this node, and we're done!

        move.l  A2,A1
        move.l  #MCN_SIZE,D0
        CALL    FreeMem

999$    tst.l   D6
        bne     AdMult2

        bsr     TermIO
        CALL    Permit
        movem.l (SP)+,A2/D6/D7
        rts

;----------------------------------------------------------------------------
; RemMCA - Remove and Deallocate an entire MCA List
;----------------------------------------------------------------------------
; IN:   A6      SysBase
;       A5      DeviceBase
;       A4      IO Request Block
;       A3      UnitBase
; OUT:
; USED:
;----------------------------------------------------------------------------

RemMCA  lea     UnitMultiCastList(A3),A0
        CALL    RemHead
        tst.l   D0
        beq.s   999$
        move.l  D0,A1
        move.l  #MCN_SIZE,D0
        CALL    FreeMem
        bra.s   RemMCA
999$    rts

;----------------------------------------------------------------------------
;LocMCN - Find A Given Address Already In The MC List
;----------------------------------------------------------------------------
;IN:    A6      SysBase
;       A5      DeviceBase
;       A4      IO Request Block
;       A3      UnitBase
;       A0      Pointer to address to look for.
;OUT:   D0      Address of located node or 0
;USED:  A2      Buffer for the passed in pointer (to the address to look for)
;       D0      The value to return
;       A3      Used to step through the list of MCNodes
;----------------------------------------------------------------------------

LocMCN  movem.l A2/A3,-(SP)
        move.l  A0,A2
        CALL    Forbid
        lea     UnitMultiCastList(A3),A3
        move.l  (A3),A3
        moveq.l #0,D0

0$      ; Any more nodes in the list?

        tst.l   (A3)
        beq.s   998$

        ; Yes, there are more nodes. Does this one match?

        move.l  A2,A0
        lea     MCN_Address(A3),A1
        cmp.l   (A0)+,(A1)+
        bne.s   10$
        cmp.w   (A0)+,(A1)+
        bne.s   10$

        ; We found the node we were looking for!

        move.l  A3,D0
        bra.s   999$

10$     move.l  (A3),A3
        bra.s   0$

998$

999$    move.l  D0,-(SP)
        CALL    Permit
        move.l  (SP)+,D0
        movem.l (SP)+,A2/A3
        rts

;----------------------------------------------------------------------------
;LocMCBt - Find Any Other Node With The Same Bit Set
;----------------------------------------------------------------------------
;IN:    A6      SysBase
;       A5      DeviceBase
;       A4      IO Request Block
;       A3      UnitBase
;       D0      Which Bit To Look For
;OUT:   D0      Address of located node or 0
;----------------------------------------------------------------------------

LocMCBt move.l  D7,-(SP)
        move.l  D0,D7
        CALL    Forbid
        lea     UnitMultiCastList(A3),A0
        move.l  (A0),A0
        moveq.l #0,D0

0$      ; Any more nodes in the list?

        tst.l   (A0)
        beq.s   998$

        ; Yes, there are more nodes. Does this one match?

        cmp.b   MCN_Bit(A0),D7
        bne.s   10$

        ; We found the node we were looking for!

        move.l  A0,D0
        bra.s   999$

10$     move.l  (A0),A0
        bra.s   0$

998$

999$    move.l  D0,-(SP)
        CALL    Permit
        move.l  (SP)+,D0
        move.l  (SP)+,D7
        rts

        END
