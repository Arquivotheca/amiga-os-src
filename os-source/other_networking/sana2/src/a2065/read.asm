**
** A2065 SANA-II Device Driver source
** Copyright (c) 1992 by Commodore-Amiga, Inc.
**
** All rights Reserved.
**


        nolist

        include "includes.asm"
        include "globals.i"
        include "a2065_defs.i"

        list

        XDEF    Read

        XREF    TermIO,AllocPN,LocPN,LocSQ

        INT_ABLES

        CNOP    2

;----------------------------------------------------------------------------
; Read - Read a packet of a specific type.
;----------------------------------------------------------------------------
; IN:   A6      SysBase
;       A5      DeviceBase
;       A4      IO Request Block
;       A3      UnitBase
; USE:  D7.w    Protocol Number
;       A2      Pointer To Protocol Nexus
;----------------------------------------------------------------------------
; NOTES:
;
; 1. The read routine doesn't do much execpt check for errors in the IOB
;    and then figure out which protocol nexus to hang this read request
;    from. If an protocol nexus is not found, one is created.
; 2. ERRORS:    UNIT_NOT_CONFIGURED             no physical address yet
;               UNIT_OFFLINE                    unit is offline
;               BAD_PROTOCOL_TYPE               protocol type pointer is null
;               BAD_PROTOCOL_TYPE               protocol length isn't 2
;               BAD_PROTOCOL_TYPE               unkown magic value
;               NO_RESOURCES                    no memory to allocate a PN
;                                               THIS GENERATES AN ONEVENT CLASS
;----------------------------------------------------------------------------

Read    movem.l D7/A2,-(SP)

        ; First, let's try to get some ram back into the NetBuffLib free
        ; pool. Even if we can't service this read request, we'll still
        ; benefit from grabbing back some memory.

        ; Now, if the unit has not been assigned a physical address...
        ; how can we read anything?

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

20$     ; Make sure the packet type being requested is a valid pointer.

        move.l  IOS2_PACKETTYPE(A4),D0
40$     ; Pointer is valid, make sure the length is 2.

        clr.l   IOS2_WIREERROR(A4)

        move.l  #S2WERR_NULL_POINTER,IOS2_WIREERROR(A4)

50$     ; Clear The QUICK Flag As We Are Accepting This Reception.

        clr.l   IOS2_WIREERROR(A4)

        CALL    Forbid

        bclr    #IOB_QUICK,IO_FLAGS(A4)
        move.b  #0,LN_TYPE(A4)

        ; OK - now which list do I put it on?
        ; Is this a read oprhpan?

        cmp.w   #S2_READORPHAN,IO_COMMAND(A4)
        bne.s   55$

        lea     UnitOrfPN(A3),A2
        bra.s   110$

55$     ; Is this an 802?

        cmp.l   #1500,d0
        bhi     60$

        ; Yes, this is an 802 type read request.

        lea     Unit802PN(A3),A2
        bra.s   110$

60$     ; This is a non-802 type read request, I must figure out
        ; which protocol nexus this should go on.

        move.l  d0,d7
        bsr     LocPN
        tst.l   D0
        bne.s   100$

        bsr     AllocPN
        tst.l   D0
        bne.s   100$

        ; Could not allocate a protocol nexus.

        CALL    Permit
        bra.s   990$


100$    ; D0 now points to a protocol nexus which this read will
        ; be appended to.

        move.l  D0,A2

	; Find the stack queue for this IO Request. LocSQ will
	; allocate one if need be.

110$	bsr	LocSQ
	tst.l	D0
	bne.s	115$
	CALL	Permit
	bra.s	990$

115$	CALL	Disable
	movea.l	d0,a0
	lea.l	SQ_LIST(A0),A0
	movea.l	A4,A1
	CALL	AddTail
	CALL	Enable
	CALL	Permit
	bra.s	999$

990$    bsr     TermIO
999$    movem.l (SP)+,D7/A2
        rts

        END
