*******************************************************************************
*
*	$Id: vporigin.asm,v 39.5 92/04/08 17:29:16 spence Exp $
*
*******************************************************************************

	section	graphics

	include "/view.i"

	xdef	_VPOrigin
	xref	_LVOGfxLookUp

******* graphics.library/VPOrigin *********************************************
*
*   NAME
*	VPOrigin -- Find the first visible pixel in the ViewPort (V39)
*
*   SYNOPSIS
*	VPOrigin(v, vp, origin1, origin2)
*	         a0 a1  a2       a3
*
*	void VPOrigin(struct View *, struct ViewPort *, Point *, Point *);
*
*   FUNCTION
*	To find the top left visible pixel in a viewport after accounting for
*	hardware limitations.
*
*   INPUTS
*	v       - View the ViewPort is in.
*	vp      - This ViewPort
*	origin1 - The Point structure that is to be filled for the first
*	          playfield.
*	origin2 - The Point structure that is to be filled for the second
*	          playfield, or NULL if not dualplayfield.
*
*   RESULT
*	originx - The Point structure will hold the coordinate
*	          in ViewPort resolution of the first visible pixel,
*	          relative to the View origin.
*
*   NOTES
*	The ViewPort must have a properly initialised DisplayClip in a
*	ViewPortExtra that has been Associated with the ViewPort.
*
*   SEE ALSO
*	<graphics/view.h> <graphics/gfx.h> GfxNew() GfxAssociate()
*
*******************************************************************************

_VPOrigin:
	move.l	a2,d0
	beq.s	_VPOrigin.		; Prevents silly enforcer hits.
	move.w	vp_Modes(a1),d1
	move.l	vp_ColorMap(a1),d0
	beq.s	_VPOrigin.
	move.l	d0,a1
	tst.b	cm_Type(a1)
	beq.s	_VPOrigin.
	move.l	cm_vpe(a1),d0
	beq.s	_VPOrigin.
	move.l	d0,a1
	move.w	v_DxOffset(a0),d0
	btst.b	#5,d1			; superhires?
	beq.s	1$
	add.w	d0,d0
1$:
	and.w	#V_HIRES,d1
	beq.s	2$
	add.w	d0,d0
2$:
	move.w	v_DyOffset(a1),d1
	btst.b	#2,v_Modes+1(a0)	; lace?
	beq.s	3$
	add.w	d1,d1
3$:
	move.l	vpe_Origin(a1),(a2)	; Both WORDS together.
	add.w	d0,tpt_x(a2)
	add.w	d1,tpt_y(a2)
	cmp.l	#0,a3
	beq.s	_VPOrigin.		; no dualplayfield.
	move.l	vpe_Origin+tpt_SIZEOF(a0),(a3)
	add.w	d0,tpt_x(a3)
	add.w	d1,tpt_y(a3)
_VPOrigin.:
	rts

	end
