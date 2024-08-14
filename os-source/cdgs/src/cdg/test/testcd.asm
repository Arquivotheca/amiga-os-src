* casm -a testcd.asm -o testcd.o -iinclude:
*
	INCLUDE "exec/types.i"
	INCLUDE "exec/resident.i"
	INCLUDE "exec/macros.i"
	INCLUDE "exec/ables.i"

	XREF	_intena

bcresident:
                dc.w    RTC_MATCHWORD
                dc.l    bcresident
                dc.l    EndModule
                dc.b    RTF_COLDSTART
                dc.b    0
                dc.b    0
                dc.b    126
                dc.l    0
                dc.l    0
                dc.l    startme

startme:
	clr.l	counter(pc)
	clr.l	save68(pc)

	move.l	4,a6
	DISABLE	

	move.l	$68,a0
	lea	countem(pc),a1
	move.l	a1,$68
	move.l	a0,save68(pc)

	ENABLE

	rts

countem:
	tst.b	$B80c56
	beq.s	notme

	addq.l	#1,counter(pc)

notme:
	
	move.l	counter(pc),$0

noreport:

	dc.w	$4ef9
	
save68:
	dc.l	0


counter:
	dc.l	0



EndModule:
		END

