* casm -a fixbarney.asm -o fixbarney.o -iinclude:
*
	INCLUDE "exec/types.i"
	INCLUDE "exec/resident.i"
	INCLUDE "exec/macros.i"
	INCLUDE "exec/nodes.i"
	INCLUDE "exec/interrupts.i"
	INCLUDE "exec/ables.i"
	INCLUDE "graphics/gfxbase.i"
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
	move.w	#$91,$00
	move.w	#$00,$00

	move.l	a6,-(sp)
	lea	gfxname(pc),a1
	moveq	#00,d0
	JSRLIB	OpenLibrary
	move.l	d0,a1
	and.b	#(~(GFXF_HR_AGNUS!GFXF_HR_DENISE)),gb_ChipRevBits0(a1)
	JSRLIB	CloseLibrary
	
	move.l	$4,a6
	move.l	$6c,save6c
	move.l	#amosfix,$6c

	move.l	(sp)+,a6

	rts


amosfix:

	move.w	#$0000,$B80C80	;reset joy data

	dc.w	$4ef9
save6c:
	dc.l	0


gfxname:
	dc.b	'graphics.library',0
EndModule:
		END

