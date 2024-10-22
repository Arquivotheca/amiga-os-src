********************************************************************************
*
*	$Id: NewMode.asm,v 39.0 93/06/30 13:36:54 spence Exp $
*
********************************************************************************

* This is a downcoded version of the new_mode() function in V39
* graphics.library. 

	include	'graphics/view.i'
	include 'internal/displayinfo_internal.i'

	xdef	new_mode

;ULONG new_mode(struct ViewPort *vp)
;       d0                       a0
; does not corrupt a0

new_mode:
	movem.l	a2/a3,-(sp)
	moveq	#0,d0
	move.w	vp_Modes(a0),d0
	move.l	vp_ColorMap(a0),d1
	beq.s	1$
	move.l	d1,a2
	tst.b	cm_Type(a2)
	beq.s	1$
	move.l	cm_CoerceDisplayInfo(a2),d1
	bne.s	2$
	move.l	cm_NormalDisplayInfo(a2),d1
	beq.s	1$
2$:
	move.l	d1,a3
	move.l	rec_Control(a3),d0
	bra.s	new_mode.

1$:
	move.l	d0,d1
	and.l	#V_EXTENDED_MODE,d1
	beq.s	3$
	move.w	vp_Modes(a0),d1
	and.w	#V_DUALPF,d1
	beq.s	4$
	move.l	#$8c44,d1
	bra.s	5$
4$:
	move.l	#$8c04,d1
	bra.s	5$
3$:
	move.w	vp_Modes(a0),d1
	and.w	#V_SUPERHIRES,d1
	beq.s	new_mode.
	move.l	#(~V_HIRES),d1
5$:
	and.l	d1,d0

new_mode.
	movem.l	(sp)+,a2/a3
	rts
