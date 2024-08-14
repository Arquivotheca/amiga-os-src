*******************************************************************************
*
*	$Id: setrgb4.asm,v 38.8 92/08/17 13:37:50 chrisg Exp $
*
*******************************************************************************

	section graphics
	include	'/macros.i'
	include	'/view.i'
	include	'/gfxbase.i'
	include	'hardware/custom.i'

	xref	_custom
    xdef    _SetRGB4
    xref    _pokecolors,_LVOSetRGB32

******* graphics.library/SetRGB4 ***********************************************
*
*    NAME
*       SetRGB4 -- Set one color register for this viewport.
*
*    SYNOPSIS
*       SetRGB4(  vp, n,   r,    g,    b)
*                 a0  d0  d1:4  d2:4  d3:4
*
*	void SetRGB4( struct ViewPort *, SHORT, UBYTE, UBYTE, UBYTE );
*
*    FUNCTION
*	Change the color look up table so that this viewport displays
*	the color (r,g,b) for pen number n.
*
*    INPUTS
*	vp - pointer to  viewport structure
*       n - the color number (range from 0 to 31)
*       r - red level (0-15)
*       g - green level (0-15)
*       b - blue level (0-15)
*
*    RESULT
*	If there is a ColorMap for this viewport, then the value will
*	be stored in the ColorMap.
*       The selected color register is changed to match your specs.
*	If the color value is unused then nothing will happen.
*
*    BUGS
*
*	NOTE: Under V36 and up, it is not safe to call this function
*	from an interrupt, due to semaphore protection of graphics
*	copper lists.
*
*    SEE ALSO
*       LoadRGB4() GetRGB4() graphics/view.h
******************************************************************************

	xref	_SetRGB32

_SetRGB4:
	swap	d0
	clr.w	d0
	swap	d0
	cmp.w	#0,a0
	beq.s	no_vp_hack
	tst.l	vp_ColorMap(a0)
	beq.s	no_cm_hack
	movem.l	d2/d3/d4,-(a7)
	swap	d1
	swap	d2
	swap	d3
	lsl.l	#8,d1
	lsl.l	#8,d2
	lsl.l	#8,d3
	move.l	d1,d4
	lsl.l	#4,d4
	or.l	d4,d1
	move.l	d2,d4
	lsl.l	#4,d4
	or.l	d4,d2
	move.l	d3,d4
	lsl.l	#4,d4
	or.l	d4,d3
	jsr	_LVOSetRGB32(a6)
	movem.l	(a7)+,d2/d3/d4
	rts
no_vp_hack:
	bsr.s	get_12bit_color	;d1=12bit(d1,d2,d3)
	add.w	d0,d0
	move.l	#_custom+color,a0
	move.w	d1,0(a0,d0.w)
	rts

get_12bit_color:
	move.l	d0,-(a7)
	and.w	#$f,d1
	lsl.w	#8,d1
	move	d2,d0
	and.w	#$f,d0
	lsl.w	#4,d0
	or.w	d0,d1
	move	d3,d0
	and.w	#$f,d0
	or.w	d0,d1
	move.l	(a7)+,d0
	rts

no_cm_hack:
	move.l	d2,-(a7)
	bsr	get_12bit_color	; d1=12bit(d1,d2,d3)
	add.w	d0,d0
	add.w	#color,d0
	move.w	#$fff,d2
; no point in holding the semaphore for a hack like this!
	bsr.s	_pokevp
	move.l	(a7)+,d2
	rts

; void __asm pokevp(register __a0 struct ViewPort *vp,register __d0 UWORD regnum, register __d1 UWORD value, 
;					register __d2 UWORD vmask); /* __asm */

; must be called with GfxBase->ActiViewCprSemaphore held!
_pokevp::
	movem.l	d2/d3/d4/a2,-(a7)
	and.w	d2,d1
	not.w	d2
	move.w	vp_Modes(a0),d3
	and.w	#V_VP_HIDE,d3
	bne.s	done_pokevp
	move.l	vp_DspIns(a0),d3
	beq.s	done_pokevp
	move.l	d3,a1
	move.l	cl_CopLStart(a1),a2
	bsr.s	pokevp_hw
	move.l	cl_CopSStart(a1),a2
	bsr.s	pokevp_hw
; now, try intermediate copper list
	move.l	cl_CopIns(a1),a1
; now, do intermediate copper list
	cmp.w	#0,a1
	beq.s	done_pokevp
pokevp_intermed:
	move.w	(a1)+,d3	; opcode
	and.w	#3,d3		; filter lof/shf bits
	beq.s	is_move
	subq.w	#1,d3		; wait?
	beq.s	is_wait
; it's a cprnxtbuf!
	move.l	(a1),a1
	bra.s	pokevp_intermed
is_wait:
	cmp.w	#255,2(a1)	; end of list?
	beq.s	done_pokevp
	lea	4(a1),a1
	bra.s	pokevp_intermed
is_move:
	cmp.w	(a1),d0
	bne.s	skipback
	move.w	2(a1),d4
	and.w	d2,d4
	or.w	d1,d4
	move.w	d4,2(a1)
skipback:
	lea	4(a1),a1
	bra.s	pokevp_intermed
done_pokevp:
	movem.l	(a7)+,d2/d3/d4/a2
	rts


pokevp_hw:
; poke all register values between the first and last WAIT's
; trashes a2,d3/d4
	cmp.w	#0,a2
	beq.s	done_list
skip_waits:
	btst	#0,1(a2)	; 2nd wait because of line 255?
	beq.s	1$
	lea	4(a2),a2		; skip 2nd wait
	bra.s	skip_waits
1$:	move.w	(a2)+,d3
	move.w	(a2)+,d4
	btst	#0,d3		; move or wait?
	bne.s	done_list	; it's a WAIT
	cmp.w	d0,d3		; right register?
	bne.s	1$		; nope
	and.w	d2,d4
	or.w	d1,d4
	move.w	d4,-2(a2)
	bra.s	1$
done_list:
	rts


	end
