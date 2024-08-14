*
* support.asm
*
        INCLUDE "exec/types.i"
        INCLUDE "exec/nodes.i"
        INCLUDE "exec/lists.i"
        INCLUDE "exec/ports.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/tasks.i"
        INCLUDE "exec/memory.i"
        INCLUDE "exec/execbase.i"
        INCLUDE "libraries/dos.i"
        INCLUDE "libraries/dosextens.i"
        INCLUDE "libraries/dos_lib.i"

FUNCDEF	MACRO
	XREF	_LVO\1
	ENDM

        INCLUDE "exec/exec_lib.i"

callsys macro
        CALLLIB _LVO\1
        endm

        section text,code

	BASEREG	a4

	XREF _SysBase
	XREF _UtilityBase
	XREF _LVOStricmp
	XREF _LVOSMult32
	XREF _LVOSDivMod32

	XDEF @BtoC
	XDEF @CtoBcpy
	XDEF @stricmp
	XDEF @mult32
	XDEF @div32
	XDEF @rem32
	XDEF @freelist
	XDEF @NewList
	XDEF @freevec

*
* char *BtoC(char * __reg A0 objname, LONG __reg D0 bstr)
*
@BtoC:
	asl.l	#2,d0		; also return value
	move.l	d0,a1		; source (bstr)
	move.l	a0,d0		; destination, also return code
	moveq	#0,d1
	move.b	(a1)+,d1	; length
	bra.s	2$

1$:	move.b  (a1)+,(a0)+	; a1 is one char ahead of a0
2$:	dbra	d1,1$

	clr.b	(a0)		; terminate
				; d0 already has return value
	rts

*
* void CtoBcpy(char * __reg a0 dest, char * __reg a1 src)
*
@CtoBcpy:
	move.l	a0,d0
	addq.l	#1,a0

1$:	move.b	(a1)+,(a0)+	; non-overlapping
	bne.s	1$

	move.l	d0,a1
	sub.l	a0,d0
	neg.w	d0		; may be 257
	subq.w	#2,d0
	move.b	d0,(a1)
	rts

*
* stricmp
*
@stricmp:
	move.l	a6,-(sp)
	move.l	_UtilityBase,a6	; actually (a4)!!!!!!
	jsr	_LVOStricmp(a6)
	move.l	(sp)+,a6
	rts

*
* multiply d0 * d1, result in d0.
*
@mult32:
	move.l	a6,-(sp)
	move.l	_UtilityBase,a6	; actually (a4)
	jsr	_LVOSMult32(a6)
	move.l	(sp)+,a6
	rts

*
* divide d0 by d1, result in d0, remainder in d1
*
@div32:
	move.l	a6,-(sp)
	move.l	_UtilityBase,a6	; actually (a4)
	jsr	_LVOSDivMod32(a6)
	move.l	(sp)+,a6
	rts

@rem32:	jsr	@div32
	move.l	d1,d0
	rts

*
* long(d0) freelist (struct node * __reg a0 list);
* duplicate of unloadseg, mostly
* handles freelist(0);
* returns number of nodes freed
*
@freelist:
	movem.l	d2/a2/a6,-(a7)	; save
	move.l	_SysBase,a6	; actually (a4)!!!!
	moveq	#0,d2
	move.l	a0,a2
1$:	move.l	a2,a1		; freevec takes arg in a1
	move.l	a1,d0		; end on null
	beq.s	2$
	move.l	(a2),a2		; next pointer
	jsr	_LVOFreeVec(a6)
	addq.l	#1,d2
	bra.s	1$
2$:	move.l	d2,d0
	movem.l	(a7)+,d2/a2/a6
	rts


*
* Stub routine to reduce code size
* freevec(void *ptr)
*
@freevec:
	move.l	a6,-(sp)
	move.l	_SysBase,a6	; actually (a4)
	move.l	a0,a1		; freevec takes arg in a1
	jsr	_LVOFreeVec(a6)
	move.l	(sp)+,a6
	rts

*
* NewList(struct List *list)
*
@NewList:
	NEWLIST	A0
	rts

	END
