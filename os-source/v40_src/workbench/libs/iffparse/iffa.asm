        OPTIMON

;---------------------------------------------------------------------------

        NOLIST

        INCLUDE "exec/types.i"
        INCLUDE "utility/hooks.i"
        INCLUDE "iffparsebase.i"
        INCLUDE "iffprivate.i"

        LIST

;---------------------------------------------------------------------------

	XDEF	_CallAHook
	XDEF	_ListAction

;---------------------------------------------------------------------------

; CallAHook(REG(a0) struct Hook *hook,
;          REG(a2) VOID *object,
;          REG(a1) LONG *message)
;
; Calls the h_Entry vector in the hook structure. If h_Entry is NULL,
; 1 is returned (signalling an error).
;
_CallAHook:
	move.l	a0,d0			; set flags
	beq.s	1$                      ; no hook, so split
	move.l	h_Entry(a0),d0		; check if there's a function
	beq.s	1$			; no function, so split
	move.l	d0,-(sp)		; put function address on stack
	rts				; go to it, and don't come back

1$:	moveq	#IFFERR_NOHOOK,d0
	rts

;---------------------------------------------------------------------------

; ListAction(REG(a0) struct IFFHandleP *iffp,
;            REG(a1) LONG *list,
;            REG(d0) LONG n,
;            REG(a2) ULONG (*func)())
;
; Takes an array of LONGs and passes them to the function.  I had to do
; it this way because I don't know how to make Lattice do __asm stuff from
; a function pointer.  Besides, I want to jump through the library base
; at some future date.
;
_ListAction:
	movem.l	d2/a2-a4,-(sp)	; Preserve registers that will change
	movea.l	a2,a4		; Copy parameters up to safe regs.
	movea.l	a1,a3
	movea.l	a0,a2
	move.l	d0,d2
	moveq	#0,d0
	bra.s	2$
1$:	move.l	(a3)+,d0	; Get type
	move.l	(a3)+,d1	; Get ID
	movea.l	a2,a0
	jsr	(a4)
	tst.l	d0
	bne.s	xit
2$:	dbra	d2,1$
xit:
	movem.l	(sp)+,d2/a2-a4
	rts

;---------------------------------------------------------------------------

	END
