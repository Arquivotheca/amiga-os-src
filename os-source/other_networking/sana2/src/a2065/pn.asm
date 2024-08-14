**
** A2065 SANA-II Device Driver source
** Copyright (c) 1992 by Commodore-Amiga, Inc.
**
** All rights Reserved.
**


        CODE

        NOLIST

        include "includes.asm"
        include "globals.i"
        include "a2065_defs.i"

        LIST

        INT_ABLES

        XDEF    TxStats,Track,UnTrack,FreePN,TypeStats,AllocPN,LocPN
        XDEF	LocSQ,FreeSQ
        XREF    TermIO,TrmEvnt

;----------------------------------------------------------------------------
; TxStats - Called To Increment Protocol Stats Due To Successful WRITE.
;----------------------------------------------------------------------------
; IN:   A6      SysBase
;       A5      DeviceBase
;       A4      IO Request Block
;       A3      UnitBase
;----------------------------------------------------------------------------
; NOTES:
;
; 1. This routine is called from a level 2 interrupt. Be snappy.
; 2. Locate the protocol nexus corresponding to the packet type which
;    was transmitted. RAW packets are never tracked. If the protocol
;    nexus cannot be found, forget it. If it is found, and it is in
;    track mode, then bump the pointers.
; 3. No need to check the protocol type pointer against null because
;    it got past WRITE.
;----------------------------------------------------------------------------


TxStats

        ; If this is a RAW type packet, then quit immediately.

        btst    #SANA2IOB_RAW,IO_FLAGS(A4)
        bne     999$


        ; This isn't a RAW packet. Are we tracking ANY protocol
        ; types?

        tst.l   UnitTrackingCount(A3)
        beq     999$


        ; We're tracking something. Therefore, the packet type
        ; must be fished out or it must be determined that this
        ; is an 802 creenalfarb oxxi type packet.

        move.l  IOS2_PACKETTYPE(A4),D0
        cmp.l   #1500,d0
        bhi     20$


        ; This is an 802 style packet. Stick the address of the
        ; Unit 802 Packet Nexus into A0.


        lea     Unit802PN(A3),A0
        bra.s   100$

20$     ; This is NOT an 802 style packet, therefore we must attempt
        ; to locate a specific Protocol Nexus corresponding to this
        ; packet type.


        bsr     LocPN
        tst.l   D0
        beq.s   999$
        move.l  D0,A0

100$    ; Get the packet length into D0.

        move.l  IOS2_DATALENGTH(A4),D0

        ; A0 contains the address of a Protocol Nexus, see if it is
        ; in Tracking Mode. If it is, increment the number of packets
        ; transmitted on this protocol type and add the length of the
        ; packet which is now in D0.


        btst    #PNB_TRACKING,PN_FLAGS(A0)
        beq.s   999$

        ; We are tracking this protocol. All this hard work is paying
        ; off since we actually get to keep some numbers here.

        addq.l  #1,PN_TYPESTATS+S2PTS_TXPACKETS(A0)
        add.l   D0,PN_TYPESTATS+S2PTS_TXBYTES(A0)

999$    rts


;----------------------------------------------------------------------------
; LocPN - Find A Particular Protocol Nexus
;----------------------------------------------------------------------------
; IN    A6      SysBase
;       A5      DeviceBase
;       A4      IOB
;       A3      UnitBase
;       D0      Protocol Number To Look For
;----------------------------------------------------------------------------
; NOTES:
;----------------------------------------------------------------------------

LocPN   move.l  D0,D1
        and.l   #HASH_MASK,D0
        mulu.w  #MLH_SIZE,D0
        lea     UnitProtocolHashTable(A3),A0
        add.l   D0,A0
        moveq.l #0,D0

        cmp.l   LH_TAILPRED(A0),A0
        bne.s   10$

        bra.s   999$

        ; There are some protocols on this list.

10$     move.l  (A0),A0

20$     tst.l   (A0)
        beq.s   999$

        cmp.w   PN_PACKETTYPE(A0),D1
        beq.s   80$
        move.l  (A0),A0
        bra.s   20$

80$     ; We found the header we are looking for.

        move.l  A0,D0

999$    rts

;----------------------------------------------------------------------------
; LocSQ - Find A Particular Stack Queue
;----------------------------------------------------------------------------
; IN    A6      SysBase
;       A5      DeviceBase
;       A4      IOB
;       A3      UnitBase
;       A2	Protocol Nexus
;----------------------------------------------------------------------------
; NOTES:
;
; 1. This routine will try to find an existing StackQueue that matches
;    the buffermanagement pointer.
; 2. If none exist, a new one will be created.
;
;----------------------------------------------------------------------------

LocSQ   lea.l	PN_RXQ(a2),a0
	move.l	IOS2_BUFFERMANAGEMENT(a4),d1

	; Look for SQ_FUNCTABLE(a0) to match d1

10$	move.l	(a0),a0
	tst.l	(a0)			; Don't process the tail node!
	beq.s	30$
	cmp.l	SQ_FUNCTABLE(a0),d1
	bne.s	10$

	move.l	a0,-(sp)
	move.l	(sp)+,d0
	rts

	; Need to allocate a new StackQueue

30$	move.l	#SQ_SIZE,d0
	move.l	#MEMF_CLEAR,d1
	CALL	AllocMem
	tst.l	d0
	bne.s	34$
	rts

34$     ; Initialize the new StackQueue

	movea.l	d0,a1
	move.l	d0,-(sp)
	move.l	IOS2_BUFFERMANAGEMENT(a4),SQ_FUNCTABLE(a1)
	NLIST	SQ_LIST(a1)

	; Add it to the protocol nexus StackQueue list
	movea.l	(sp),a1
	lea.l	PN_RXQ(a2),a0

	DISABLE
	CALL	AddTail
	CALL	Enable

	move.l	(sp)+,d0
	rts


;----------------------------------------------------------------------------
; Track - Starts taking statistics on a particular protocol nexus.
;----------------------------------------------------------------------------
; IN    A6      SysBase
;       A5      DeviceBase
;       A4      IO Request Block
;       A3      UnitBase
; USE:  D7      Buffers packet type
;----------------------------------------------------------------------------
; NOTES:
; 1. A packettype is passed in, check to see if it already has a protocol
;    nexus. If it does, then zero the counts and bookkeep. If it doesn't
;    type to allocate a protocol nexus -- if you can't it's an error. If
;    you can, zero it out and add it to the list, bookkeep, return.
; 2. Can generate error of BAD_PROTOCOL_TYPE if protocol type pointer is
;    NULL.
; 3. Can generate error of NO_RESOURCES if we can't allocate a protocol
;    nexus (user is in deep doodoo if we can't allocate these few bytes).
; 4. Can generate error of TYPE_ALREADY_TRACKED if the protocol nexus
;    shows that it is already being tracked.
;----------------------------------------------------------------------------

Track   movem.l A2/D7,-(SP)
        move.l  IOS2_PACKETTYPE(A4),D0

0$      cmp.l   #1500,d0
        bhi     5$

2$      ; The user wants to track 802 style packets. This is easy.

        lea     Unit802PN(A3),A2
        bra.s   110$

5$      ; The user wants to track a NON 802 style packet.

         move.l   d0,d7

        ; Look for a pre-existing protocol nexus

        move.l  D7,D0
        bsr     LocPN
        tst.l   D0
        bne.s   100$

        bsr.s   AllocPN
        tst.l   D0
        beq.s   999$

100$    ; Protocol nexus exists and is pointed to by D0
        ; zero it out and bookkeep

        move.l  D0,A2

110$    btst    #PNB_TRACKING,PN_FLAGS(A2)
        beq.s   150$

        ; This protocol type is already being tracked.

        move.b  #S2ERR_BAD_STATE,IO_ERROR(A4)
        move.l  #S2WERR_ALREADY_TRACKED,IOS2_WIREERROR(A4)
        bra.s   999$

150$    move.l  A2,A0
        lea     PN_TYPESTATS(A0),A0
        moveq.l #0,D0
        move.l  D0,(A0)+        ; TX_PACKETS
        move.l  D0,(A0)+        ; RX_PACKETS
        move.l  D0,(A0)+        ; TX_BYTES
        move.l  D0,(A0)+        ; RX_BYTES
        move.l  D0,(A0)+        ; PACKETSDROPPED
        bset    #PNB_TRACKING,PN_FLAGS(A2)
        addq.l  #1,UnitTrackingCount(A3)

999$    movem.l (SP)+,D7/A2
        bra     TermIO


;----------------------------------------------------------------------------
; AllocPN - Allocate and Initialize a Protocol Nexus
;----------------------------------------------------------------------------
; IN:   A6      SysBase
;       A5      DeviceBase
;       A4      IOB
;       A3      UnitBase
;       D7.w    Protocol Number
; OUT:  D0      Allocated Protocol Nexus Or Zero
;----------------------------------------------------------------------------
; NOTES:
;
; 1. Can be called by TRACK and READ.
; 2. If we can't create a protocol nexus, then wake up anyone sleeping
;    on a SOFTWARE ONEVENT.
;----------------------------------------------------------------------------

AllocPN move.l  A2,-(SP)

        ; A protocol nexus does not exist for this protocol type.
        ; Attempt to create one.

        CALL    Forbid

        move.l  #PN_SIZE,D0
        move.l  #MEMF_PUBLIC|MEMF_CLEAR,D1
        CALL    AllocMem
        tst.l   D0
        bne.s   10$

        ; Could not allocate a protocol nexus.

        move.b  #S2ERR_NO_RESOURCES,IO_ERROR(A4)
        clr.l   IOS2_WIREERROR(A4)

        move.l  #s2eF_SOFTWARE|s2eF_ERROR|s2eF_RX,D0
        bsr     TrmEvnt

        CALL    Permit
        moveq.l #0,D0
        bra     999$

10$     ; Protocol nexus has been allocated...initialize it.

        move.l  D0,A2
        move.w  D7,PN_PACKETTYPE(A2)

        NLIST   PN_RXQ(A2)

        ; Now it must be added to the right list.

        move.l  D7,D0
        and.l   #HASH_MASK,D0
        mulu.w  #MLH_SIZE,D0
        lea     UnitProtocolHashTable(A3),A0
        add.l   D0,A0
        move.l  A2,A1
        DISABLE
        CALL    AddTail
        CALL    Enable
        CALL    Permit
        move.l  A2,D0

999$    move.l  (SP)+,A2
        rts


;----------------------------------------------------------------------------
; UnTrack - Stops taking statistics on a particular protocol nexus.
;----------------------------------------------------------------------------
; IN    A6      SysBase
;       A5      DeviceBase
;       A4      IO Request Block
;       A3      UnitBase
; USE:
;----------------------------------------------------------------------------
; NOTES:
;
; 1. Can generate an error of BAD_PROTOCOL_TYPE if the protocol type
;    pointer is null.
; 2. Can generate an error of TYPE_NOT_TRACKED if the protocol nexus
;    cannot be found or the type is not being tracked.
;----------------------------------------------------------------------------

UnTrack move.l  IOS2_PACKETTYPE(A4),D0

        cmp.l   #1500,d0
        bhi     100$

52$     ; The user wants to stop tracking 802 style packets. This is easy.

        lea     Unit802PN(A3),A0
        bra.s   200$

100$    ; The user wants to stop tracking a NON 802 style packet.

        ; Look for an existing protocol nexus

        bsr     LocPN
        tst.l   D0
        bne.s   190$

        ; Protocol Nexus for this type was not found.

150$    move.b  #S2ERR_BAD_STATE,IO_ERROR(A4)
        move.l  #S2WERR_NOT_TRACKED,IOS2_WIREERROR(A4)
        bra.s   999$

        ; Found the protocol nexus...clear the flag.

190$    move.l  D0,A0
200$    btst.b  #PNB_TRACKING,PN_FLAGS(A0)
        beq.s   150$
        bclr.b  #PNB_TRACKING,PN_FLAGS(A0)
999$    bra     TermIO


;----------------------------------------------------------------------------
; FreePN - Free All Of The Protocol Nexae Allocated To This Unit
;----------------------------------------------------------------------------
; IN:   A6      SysBase
;       A5      DeviceBase
;       A4      ---
;       A3      UnitBase
; USE:  D7      Loop Counter
;----------------------------------------------------------------------------
; Notes:
;
; 1. For each of the HASH_SIZE list headers, go through each list
;    deallocating any protocol nexus structures you might find.
; 2. Do not have to FORBID or DISABLE since this routine is called
;    only at CloseDevice time.
;----------------------------------------------------------------------------

FreePN
        movem.l D7/A2,-(SP)
        move.l  #HASH_SIZE-1,D7
        lea     UnitProtocolHashTable(A3),A2

0$      move.l  A2,A0
        CALL    RemHead
        tst.l   D0
        beq.s   500$

        move.l  D0,A1
        move.l  #PN_SIZE,D0
        CALL    FreeMem
        bra.s   0$

500$    lea     MLH_SIZE(A2),A2
        dbf     D7,0$

        movem.l (SP)+,D7/A2
        rts

;----------------------------------------------------------------------------
; FreeSQ - Free All Of The Stack Queues associated with a protocol stack
;----------------------------------------------------------------------------
; IN:   A6      SysBase
;       A5      DeviceBase
;       A4      ---
;       A3      UnitBase
;	D0	FuncTable pointer
; USE:  D7      Loop Counter
;----------------------------------------------------------------------------
; Notes:
;
; 1. For each of the HASH_SIZE list headers, go through each list
;    deallocating any protocol nexus structures you might find.
; 2. Do not have to FORBID or DISABLE since this routine is called
;    only at CloseDevice time.
;----------------------------------------------------------------------------

FreeSQ
        movem.l D7/A2,-(SP)
        move.l  #HASH_SIZE-1,D7
        lea     UnitProtocolHashTable(A3),A2

	;Outer Loop - Walk Hash Table
	CALL	Forbid			; make things really safe

0$      move.l  A2,A0			; a0 points to the list header

	; Mid Loop - Walk Protocol Nexus Chain

1$	move.l	(a0),a0			; get next PN on chain
	tst.l	(a0)			; don't do the tail node
	beq.b	500$
	lea.l	PN_RXQ(a0),a1
	pea	1$			; Return trick

	; Inner Loop - Walk Stack Queue Chain
2$	move.l	(a1),a1
	tst.l	(a1)
	bne.s	3$			;Next PN?
	rts				;Back to mid loop

3$	cmp.l	SQ_FUNCTABLE(a1),d0	;Match?
	bne.b	2$			;Nope

	; Remove this stack queue

	movem.l	d0/a0/a1,-(sp)
	DISABLE				;Protect from device driver
	CALL	Remove
	CALL	Enable
	movem.l	(sp),d0/a0/a1		;Restore registers

	move.l	#SQ_SIZE,d0
	CALL	FreeMem
	movem.l	(sp)+,d0/a0/a1
	bra.b	2$

500$    lea     MLH_SIZE(A2),A2
        dbf     D7,0$

	lea	Unit802PN(a3),a1	;Check 802 Nexus
	bsr.b	2$
	lea	UnitOrfPN(a3),a1	;Check Orphan Nexus
	bsr.b	2$

	CALL	Permit			;All done

        movem.l (SP)+,D7/A2
        rts

;----------------------------------------------------------------------------
; TypeStats - Called By the GETTYPESTATS User Command
;----------------------------------------------------------------------------
; IN:   A6      SysBase
;       A5      DeviceBase
;       A4      IOB
;       A3      UnitBase
;----------------------------------------------------------------------------
; NOTES:
;----------------------------------------------------------------------------

TypeStats
        tst.l   IOS2_STATDATA(A4)
        bne.s   10$

        ; We don't want to write into low memory - after all, we aren't OXXI

        move.b  #S2ERR_BAD_ARGUMENT,IO_ERROR(A4)
        move.l  #S2WERR_NULL_POINTER,IOS2_WIREERROR(A4)
        bra.s   999$

10$     ; Get the protocol type

        move.l  IOS2_PACKETTYPE(A4),D0
        cmp.l   #1500,d0
        bhi     100$


22$     ; The user wants the 802 statistics

        lea     Unit802PN(A3),A0
        bra.s   200$

100$    ; The user wants to stop tracking a NON 802 style packet.

        ; Look for an existing protocol nexus

        bsr     LocPN
        tst.l   D0
        bne.s   190$

        ; Protocol Nexus for this type was not found.

150$    move.b  #S2ERR_BAD_STATE,IO_ERROR(A4)
        move.l  #S2WERR_NOT_TRACKED,IOS2_WIREERROR(A4)
        bra.s   999$

        ; Found the protocol nexus...clear the flag.

190$    move.l  D0,A0
200$    lea     PN_TYPESTATS(A0),A0
        move.l  IOS2_STATDATA(A4),A1
        move.l  #S2PTS_SIZE,D0
        CALL    CopyMem

999$    bra     TermIO

EndCode END
