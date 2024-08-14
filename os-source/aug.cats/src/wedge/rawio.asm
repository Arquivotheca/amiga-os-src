;
;Raw I/O routines for parallel version of kprintf, kputchar, etc.
;

;***** Include Files *****
	INCLUDE	'exec/types.i'
	INCLUDE 'hardware/cia.i'

;***** Imported Globals *****
	XREF	_ciaa
	XREF	_ciab
	XREF	_AbsExecBase

;***** Imported Functions *****
*	XREF	_LVORawDoFmt
******  Use the one in dofmt.asm
	XREF	RawDoFmt

;***** Exported Functions *****
	XDEF	PRawPutChar
	XDEF	PRawMayGetChar
	XDEF	PRawDoFmt

;
; Put a character out to the parallel port.
; - expands lf to cr/lf.
;
_PRawPutChar:
	move.l	4(sp),d0	; get char to send
PRawPutChar:
	move.b	d0,-(sp)	; save char to send
	cmp.b	#10,d0		; is it a lf?
	bne.s	10$		; no, so just output it
	moveq	#13,d0		; expand lf to cr/lf
	bsr.s	20$		; output the cr
	tst.b	_ciab+ciapra
10$	move.b	(sp)+,d0	; get char to send
;
;	Put a character to the port.  Do some throw-away reads of the register
;	before checking the BUSY bit (gives printer over 1us to stabilize
;	BUSY).
;
;	The 8520 asserts PC three cycles after a write to port B.
;
20$	tst.b	_ciab+ciapra			;
	tst.b	_ciab+ciapra
30$	btst.b	#CIAB_PRTRBUSY,_ciab+ciapra	; check parallel port status
	bne.s	30$				; yes, so wait

	move.b	#$ff,_ciaa+ciaddrb	; set port to output
	move.b	d0,_ciaa+ciaprb		; write data
	tst.b	_ciab+ciapra
	tst.b	_ciab+ciapra
;
;	Post-data hold time is guaranteed by the throw-away reads above.
;
	rts

;
; Format a c type format string to the parallel port
;
PRawDoFmt:
	movem.l	a2/a6,-(sp);		; save a2 & a6
	lea	PRawPutChar(pc),a2	; get address of output routine in a2
	move.l	_AbsExecBase,a6		; get execbase in a6
*	jsr	_LVORawDoFmt(a6)	; do the formatting
	jsr	RawDoFmt		; do the formatting
	movem.l	(sp)+,a2/a6		; restore a2 & a6
	rts

;
; Attempt to return a char from the parallel port.
;
_PRawMayGetChar:
PRawMayGetChar:
	moveq	#-1,d0		; assume no data
	rts

	END

