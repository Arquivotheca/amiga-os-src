
;	SECTION debug,CODE
	SECTION main,CODE

	INCLUDE "macros.i"


	IFNE	DEBUG_DETAIL

;-Bryce's debugging stuff

	XDEF	kprint_macro
	XDEF	printit
	XREF	_LVORawDoFmt
	;A0-string
	;A1-args
	;All other registers are preserved

kprint_macro:
printit:
		movem.l A6/A2/A3/D0/D1,-(SP)
		move.l	4,a6
		lea.l	PSCODE(pc),a2
		suba.l	a3,a3
		jsr	_LVORawDoFmt(a6)
		movem.l (SP)+,D0/D1/A2/A3/A6
		rts

PSCODE: 	tst.b	d0
		beq.s	ignore
		move.w	$DFF018,d1
		btst	#13,d1
		beq.s	PSCODE
		and.b	#$7f,d1
		cmp.b	#$18,d1 ;Check for CAN (^X)
		beq.s	ignore
		cmp.b	#$13,d1 ;Check for Xoff
		beq.s	PSCODE
		and.w	#$ff,d0
		or.w	#$100,d0
		move.w	d0,$DFF030
ignore: 	rts
	ENDC
	END
