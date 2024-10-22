**
**      $Id:
**
**      debug print macros
**
**      (C) Copyright 1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**



**
**	Debug macros for writing text to serial port
**

* Cause a trap condition

KTRAP	MACRO
	IFNE	DEBUG
	CLR.B	$00
	ENDC
	ENDM

* Put a string lf/cr terminated to the serial port

KPUTS	MACRO
	IFNE	DEBUG

		IFND	KPutStr
		XREF	KPutStr
		ENDC

	movem.l	d0-d1/a0-a1,-(SP)
	lea	KPUTS\@(pc),a0
	jsr	KPutStr
	movem.l	(SP)+,d0-d1/a0-a1
	bra	KPUTSX\@

KPUTS\@	dc.b	\1
	dc.b	$a,$d,$0

	CNOP	0,4

KPUTSX\@	
	ENDC
	ENDM


* put a string to the console

KPUT	MACRO
	IFNE	DEBUG

		IFND	KPutStr
		XREF	KPutStr
		ENDC

	movem.l	d0-d1/a0-a1,-(SP)
	lea	KPUT\@(pc),a0
	jsr	KPutStr
	movem.l	(SP)+,d0-d1/a0-a1
	bra	KPUTX\@

KPUT\@	dc.b	\1
	dc.b	$0

	CNOP	0,4

KPUTX\@	
	ENDC
	ENDM

* put a word to the serial port - hex

KBYTE	MACRO
	IFNE	DEBUG


	movem.l	d0-d2/a0-a1,-(SP)
	move.b	\1,d2
	jsr	kbyte_out
	movem.l	(SP)+,d0-d2/a0-a1

	CNOP	0,4

	ENDC
	ENDM

* put a word to the serial port - hex

KWORD	MACRO
	IFNE	DEBUG


	movem.l	d0-d2/a0-a1,-(SP)
	move.w	\1,d2
	jsr	kword_out
	movem.l	(SP)+,d0-d2/a0-a1

	CNOP	0,4

	ENDC
	ENDM

* put a word to the serial port - hex

KLONG	MACRO
	IFNE	DEBUG

	movem.l	d0-d2/a0-a1,-(SP)
	move.l	\1,d2
	jsr	klong_out
	movem.l	(SP)+,d0-d2/a0-a1

	CNOP	0,4

	ENDC
	ENDM

* useful macros for putting out values
* string followed by a byte, word, or long word
* and a lf/cr combo

KPBYTE	MACRO
	IFNE	DEBUG

	KPUT	\1
	KBYTE	\2
	KPUTS	' '
	ENDC
	ENDM

KPWORD	MACRO
	IFNE	DEBUG

	KPUT	\1
	KWORD	\2
	KPUTS	' '
	ENDC
	ENDM

KPLONG	MACRO
	IFNE	DEBUG

	KPUT	\1
	KLONG	\2
	KPUTS	' '
	ENDC
	ENDM

*
* useful macro for displaying registers
*

KREGISTERS	MACRO
	IFNE	DEBUG

	jsr	kregs

	ENDC
        ENDM



* this code is shared by the debug macros
* conditional code to print a hex number to the serial port

	IFNE	DEBUG
		IFND	KPutChar
		XREF	KPutChar
		ENDC

kout_hex:
	move.l	a2,-(SP)

kout_loop:
	rol.l	#4,d2
	moveq	#00,d0
	move.b	d2,d0
	andi.b	#$0f,d0
	lea	kout_table(pc),a2
	move.b	0(a2,d0.w),d0
	jsr	KPutChar
	dbf	d3,kout_loop

	move.l	(SP)+,a2
	rts

	;-- ubyte value

kbyte_out:
	movem.l	d2-d3,-(SP)
	moveq	#01,d3		;2 nibbles to print
	swap	d2
	lsl.l	#8,d2		;put byte in high byte
	jsr	kout_hex
	movem.l	(SP)+,d2-d3
	rts

	;-- word value

kword_out:
	movem.l	d2-d3,-(SP)
	moveq	#03,d3		;4 nibbles to print
	swap	d2		;put word in high word
	jsr	kout_hex
	movem.l	(SP)+,d2-d3
	rts

	;-- long value

klong_out:

	movem.l	d2-d3,-(SP)
	moveq	#07,d3		;8 nibbles to print
	jsr	kout_hex
	movem.l	(SP)+,d2-d3
	rts


kout_table:
	dc.b	'0123456789ABCDEF',0

	CNOP	0,4
	ENDC



* Print registers

	IFNE	DEBUG
	
		IFND	KPutStr
		XREF	KPutStr
		ENDC

kregs:

	movem.l	d0-d7/a0-a7,-(sp)

	lea	k_pcounter,a0
	jsr	KPutStr

	move.l	64(sp),D2		;program counter on stack
	subq.l	#4,D2

	jsr	klong_out

	lea	k_pdregs,a0		;draw data registers
	jsr	KPutStr


	lea	0(sp),a2
	moveq	#07,d3

koutdx:
	move.l	(a2)+,d2
	jsr	klong_out

	lea	k_space,a0
	jsr	KPutStr

	dbf	d3,koutdx



	lea	k_paregs,a0		;address registers
	jsr	KPutStr

	lea	32(sp),a2		;a0-a7 first, followed by d0-d7

	moveq	#07,d3

koutax:
	move.l	(a2)+,d2
	jsr	klong_out

	lea	k_space,a0
	jsr	KPutStr

	dbf	d3,koutax

	lea	k_eol,a0
	jsr	KPutStr

	movem.l	(sp)+,d0-d7/a0-a7
	rts

	
	CNOP	0,4

k_pcounter:	dc.b	'PROGRAM COUNTER = ',0

k_pdregs:	dc.b	10,13,'DATA: ',0
k_paregs:	dc.b	10,13,'ADDR: ',0
k_eol:		dc.b	10,13,0
k_space:	dc.b	' ',0

	CNOP	0,4
	ENDC

		