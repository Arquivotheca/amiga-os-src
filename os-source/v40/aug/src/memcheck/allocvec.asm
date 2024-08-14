;*************************************************************************************
; allocvec.asm
;*************************************************************************************

	INCLUDE "exec/types.i"
	INCLUDE "globaldata.i"

;*************************************************************************************

	XDEF	_nAllocVec

;*************************************************************************************

	XREF	_LVOForbid
	XREF	_LVOPermit
	XREF	_newAllocMem
	XREF	_ShowProcessInfo
	XREF	KPutStr

;*************************************************************************************

	XREF	_gd
	XREF	alloc0

;*************************************************************************************

_nAllocVec:
	move.l	a5,-(a7)
	lea	4(a7),a5			; Get caller address
	tst.l	d0				; See if size is zero
	beq.s	_nAllocVec_Zero			; it is, so give error message
	addq.l	#4,d0				; Need space to remember the size.
	move.l	d0,-(a7)			; Save the size on the stack
	jsr	_newAllocMem			; Call our AllocMem(size,attrs)
	tst.l	d0				; See if the allocation failed
	beq.s	ac_fail				; It failed, so exit
	move.l	d0,a0				; Copy the address
	move.l	(a7),(a0)+			; Save the size in the memory block
	move.l	a0,d0				; Return the adjusted memory block
ac_fail:
	addq.l	#4,a7				; Drop size off the stack
ac_exit:
	move.l	(a7)+,a5
	rts

_nAllocVec_Zero
	move.l	a2,-(a7)			; Save the registers
	jsr	_LVOForbid(a6)			; forbid a task switch

	; Output the size of zero error message
	lea	alloc0,a0
	jsr	KPutStr

	; Show the process information.
	move.l	_gd,a0				; GlobalData
	move.l	a5,a1				; stack pointer
	sub.l	a2,a2				; clear mc
	jsr	_ShowProcessInfo		; a0=GlobalData a1=stackptr a2=mc

	jsr	_LVOPermit(a6)			; permit a task switch
	move.l	(a7)+,a2			; restore the registers
	moveq.l	#0,d0				; clear the return value
	bra	ac_exit
