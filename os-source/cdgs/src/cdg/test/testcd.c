	INCLUDE "exec/types.i"
	INCLUDE "exec/macros.i"
	INCLUDE "exec/ables.i"

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
	
        btst    #6,$bfe001
        beq.s   noreport

	move.l	counter(pc),$0

noreport:
	move.l	save68,a0
	jmp	(a0)
	


counter:
	dc.l	0

save68:
	dc.l	0


EndModule:
		END

