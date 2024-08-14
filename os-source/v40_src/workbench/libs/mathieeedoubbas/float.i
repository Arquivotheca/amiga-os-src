*******************************************************************************
*
*	$Header:
*
*******************************************************************************

	section mathieeedoubbas
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

