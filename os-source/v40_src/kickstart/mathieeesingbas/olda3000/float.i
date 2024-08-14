*******************************************************************************
*
*	$Header:
*
*******************************************************************************

	section mathieeesingbas
	xref	do_trapv
	xref	do_div0

;ALLOW_TRAPS	equ 1

;	overflow trap
fptrap	macro
;	overfloat trap
	ifd ALLOW_TRAPS
		ifeq \1-2
			bsr	do_trapv
		endc
;		underfloat is ignored
		ifeq \1-1
		endc
; divide by zero
		ifeq \1-3
			bsr	do_div0
		endc
; indefinate trap
		ifeq \1-4
;			bsr	do_trapv
		endc
		ifne \1-4
			ifne \1-1
				ifne \1-2
					ifne \1-3
						fail
					endc
				endc
			endc
		endc
	endc
	endm

sdebug  macro
;	move.l	\1,-(sp)
;	move.l	\2,-(sp)
;	addq.l	#8,sp
	endm


; following special macros pack exponent and sign from d6 into d0
; The first is a new one I wrote, it takes the sign from bit 31
; and puts it in bit15. Pretty clever, eh?
DOIT	macro
	rol.l	#1,d6	; get sign bit into bit0 from bit31
	ror.w	#1,d6	; move it into bit15, along with other bits.
	or.w	d6,d0	; jam all bits into d0 now
	endm

; original code to pack sign and exp
DOIT1	macro
	eor.w	d6,d0		; Add exponent
	swap	d6		; Add sign
	and.w	#$8000,d6
	eor.w	d6,d0
	endm

