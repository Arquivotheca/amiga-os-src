* casm -a fixamos.asm -o fixamos.o -iinclude:
*
	INCLUDE "exec/types.i"
	INCLUDE "exec/resident.i"
	INCLUDE "exec/macros.i"
	INCLUDE "exec/nodes.i"
	INCLUDE "exec/interrupts.i"
	INCLUDE "exec/ables.i"
	INCLUDE "hardware/intbits.i"
	XREF	_intena

bcresident:
                dc.w    RTC_MATCHWORD
                dc.l    bcresident
                dc.l    EndModule
                dc.b    RTF_COLDSTART
                dc.b    0
                dc.b    0
                dc.b    7
                dc.l    0
                dc.l    0
                dc.l    startme

startme:

	move.l	4,a6
	lea	interrupt(pc),a1
	moveq	#INTB_VERTB,d0
	JSRLIB	AddIntServer
	rts


amosfix:

	move.w	#$8080,$B80C80	;reset joy data
	moveq	#00,d0
	rts


interrupt:
	dc.l	0
	dc.l	0
	dc.b	-100
	dc.b	NT_INTERRUPT
	dc.l	0
	dc.l	0
	dc.l	amosfix

EndModule:
		END

