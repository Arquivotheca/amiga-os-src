;*****************************************************************************/
; freevec.asm
;*****************************************************************************/

	INCLUDE "globaldata.i"

;*****************************************************************************/

	XDEF	_nFreeVec

;*************************************************************************************

	XREF	_LVOForbid
	XREF	_LVOPermit
	XREF	_newFreeMem

;*************************************************************************************
; void ASM nFreeVec (REG (a1) UBYTE * memb, REG (a6) struct Library *);
;*************************************************************************************
_nFreeVec:
	jsr	_LVOForbid(a6)
	movem.l	a2,-(sp)
	lea	4(sp),a2

	move.l	a1,d1				; Test for NULL
	beq.s	_nFreeVec_RTS

	move.l	-(a1),d0			; decrement the address and move the size to D0
	jsr	_newFreeMem			; call our FreeMem

_nFreeVec_RTS:
	movem.l	(sp)+,a2
	jmp	_LVOPermit(a6)

;*************************************************************************************

	IFD	BIG_SLOW

;*************************************************************************************

	XREF	_LVOTypeOfMem
	XREF	_ShowProcessInfo
	XREF	_nFreeMem
	XREF	_kprintf

;*************************************************************************************

	XREF	_gd

;
;	D3	ULONG size
;
;	A2	temporary
;	A3	struct GlobalData *
;	A4	UBYTE *memb
;	A5	ULONG *stackptr

_dFreeVec:
	movem.l	d3/a2-a5,-(sp)			; save the registers
	lea	24(sp),a5			; get the stack pointer

	; Test for NULL memory address
	move.l	a1,d1
	beq.s	_Exit

	; Get the size of memory block
	move.l	-(a1),d3			; decrement the address & get the size
	move.l	a1,a4				; save the memory address

	; Get the GlobalData
	move.l	_gd,a3

	; Are we supposed to be active?
	btst.b	#4,gd_Flags(a3)
	beq.b	_FreeMem

	; See if we have a valid memory address
	move.l	a4,a1
	jsr	_LVOTypeOfMem(a6)
	tst.l	d0
	bne.b	_FreeMem

	; Don't let anything interrupt our error message
	jsr	_LVOForbid(a6)

	; Show the error message
	move.l	a4,-(a7)
	pea	err1(pc)
	jsr	_kprintf
	lea	8(a7),a7

	; need to jsr to ShowProcessInfo
	move.l	a3,a0				; GlobalData
	move.l	a5,a1				; stack pointer
	sub.l	a2,a2				; clear mc
	jsr	_ShowProcessInfo		; a0=GlobalData a1=stackptr a2=mc

	; Done with the error message
	jsr	_LVOPermit(a6)

	bra	_Exit

_FreeMem:
	; Call the system FreeMem function
	move.l	a4,a1				; address
	move.l	gd_FreeMem(a3),a0
	jsr	(a0)

_Exit
	movem.l	(a7)+,d3/a2-a5		; restore the registers
	rts

;*************************************************************************************

err1	dc.b	'Free: (%08lx) Invalid memory pointer.',13,10,0

;*************************************************************************************

	ENDC
