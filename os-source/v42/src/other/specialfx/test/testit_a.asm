* Interrupt code for the copper interrupt.

	incdir	"include:"
	include	'graphics/gfxbase.i'

	xdef	_CopServer
	xdef	_PortServer

	xref	_LVOVBeamPos
_LVOFindVP EQU	-60

_CopServer:
	move.l	a1,a5
	move.l	4(a5),a6	; GfxBase
	jsr	_LVOVBeamPos(a6) ; line in d0
	move.l	gb_ActiView(a6),a0
	move.l	8(a5),a6	; SpecialFXBase
	move.w	d0,-(sp)
	move.l	sp,a1
	jsr	_LVOFindVP(a6)
	move.w	(sp)+,d1	; line in the ViewPort of the interrupt
	move.l	d0,(a5)		; resulting ViewPort
	beq.s	1$
	subq.w	#1,d1		; count on the VBeamPos being 1 line out.
	cmp.w	12(a5),d1	; is this the first line?
	bgt.s	2$
	move.w	#$0f0,$dff180
	bra.s	1$
2$:
	move.w	#$00f,$dff180

1$:
	moveq	#0,d0
	rts

_PortServer:
	move.l	a1,a5
	move.l	4(a5),a6	; GfxBase
	jsr	_LVOVBeamPos(a6) ; line in d0
	move.l	gb_ActiView(a6),a0
	move.l	8(a5),a6	; SpecialFXBase
	jsr	_LVOFindVP(a6)
	tst.l	d0
	beq.s	10$
	move.w	#$0F0,$dff180	; colour 0 -> green
	moveq	#0,d0
	rts
10$:
	move.w	#$00f,$dff180
	moveq	#0,d0
	rts
