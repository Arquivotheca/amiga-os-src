* casm -a forbids.asm -o forbids.o -iinclude:
*
	INCLUDE "exec/types.i"
	INCLUDE "exec/resident.i"
	INCLUDE "exec/macros.i"
	INCLUDE "exec/nodes.i"
	INCLUDE "exec/execbase.i"
	INCLUDE "exec/interrupts.i"
	INCLUDE "exec/ables.i"
	INCLUDE "graphics/gfxbase.i"
	INCLUDE "hardware/intbits.i"
	XREF	_intena

COLOR0	EQU	$DFF180

bcresident:
                dc.w    RTC_MATCHWORD
                dc.l    bcresident
                dc.l    EndModule
                dc.b    RTF_COLDSTART
                dc.b    0
                dc.b    0
                dc.b    104
                dc.l    0
                dc.l    0
                dc.l    startme

startme:
	tst.b	TDNestCnt(a6)
	bmi.s	exit

		moveq	#-1,d0
loop:
		move.w	#400,d1
                move.w  d0,COLOR0
                move.w  d0,COLOR0+2
delay:
	nop
	dbf	d1,delay
	dbf	d0,loop
exit:	
	rts


EndModule:
		END

