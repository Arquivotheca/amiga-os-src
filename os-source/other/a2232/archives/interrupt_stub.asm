
	INCLUDE "exec/types.i"
	INCLUDE "exec/interrupts.i"
	INCLUDE "hardware/intbits.i"

JSRLIB	MACRO
	XREF	_LVO\1
	JSR	_LVO\1(A6)
	ENDM


		move.l	4,a6
		lea.l	md_PORTS_IS(pc),a1
		move.b	#NT_INTERRUPT,LN_TYPE(a1)
		lea.l	PORTS_IRQ(pc),a0
		move.l	a0,IS_CODE(a1)
		clr.l	IS_DATA(a1)
		lea.l	nameText(pc),a0
		move.l	a0,LN_NAME(a1)
		move.b	#-20,LN_PRI(a1)
		moveq	#INTB_PORTS,d0
		JSRLIB	AddIntServer

		move.l	#1<<12,d0
		JSRLIB	Wait

		lea.l	md_PORTS_IS(pc),a1
		moveq	#INTB_PORTS,d0
		JSRLIB	RemIntServer
		moveq	#0,d0
		rts

*
* A0 = Custom
* A1 = IS_DATA
* D0/D1,A0/A1,A5 are scratch
*

		CNOP	0,4 ;Be nice to 68020's
;PORTS_IRQ:
;		 tst.b	 $EA0000+$000F
;		 bne.s	 ForUs
;		 rts	 ;(exit Z=true=don't terminate chain)
;
;ForUs:
PORTS_IRQ:
		;tst.w	 $E90000+$4000
		bchg.b	#1,$bfe001
		moveq	#0,d0	;Z=flase. Flag "terminate chain"
		rts	;(exit)


md_PORTS_IS:
	dcb.b	IS_SIZE,0

nameText:
	dc.b	'stub interrupt',0
	ds.w	0

