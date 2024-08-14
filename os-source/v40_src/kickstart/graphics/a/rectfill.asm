*******************************************************************************
*
*	$Id: rectfill.asm,v 39.1 92/03/20 10:02:11 chrisg Exp $
*
*******************************************************************************

    xdef    _RectFill
******* graphics.library/RectFill *******************************************
*
*   NAME
*       RectFill -- Fill a rectangular region in a RastPort.
*
*   SYNOPSIS
*
*	RectFill( rp, xmin, ymin, xmax, ymax)
*                 a1  d0:16 d1:16 d2:16 d3:16
*
*	void RectFill( struct RastPort *, SHORT, SHORT, SHORT, SHORT );
*
*   FUNCTION
*	Fills  the  rectangular  region  specified  by  the
*	parameters  with the chosen pen  colors,  areafill
*	pattern, and drawing mode. If no areafill pattern is
*	specified, fill the rectangular region with the FgPen
*	color, taking into account the drawing mode.
*
*   INPUTS
*	rp - pointer to a RastPort structure
*	(xmin,ymin) (xmax,ymax) are the coordinates of the upper
*		left corner and the lower right corner, respectively, of the
*	        rectangle.
*   NOTE
*
*	The following relation MUST be true:
*		(xmax >= xmin) and (ymax >= ymin)  
*
*   BUGS
*	Complement mode with FgPen complements all bitplanes.
*
*   SEE ALSO
*	AreaEnd() graphics/rastport.h
*
******************************************************************************

	include		'exec/types.i'
	include		'exec/nodes.i'
	include		'exec/lists.i'
	include		'graphics/rastport.i'
	include		'submacs.i'

	section	graphics
	xref	_LVOMove
	xref	_LVODraw
	xref	_LVOBltPattern
	xref	GenMinTerms

*LAYERNEEDED	equ 1

; void __asm RectFill(register __a1 struct RastPort *r,register __d0 int x1,register __d1 int y1,
; 		      register __d2 int x2,register __d3 int y2); /* __asm */

_RectFill:
	movem.l	d4/d5/a2,-(sp)
	move.l	a1,a2			* save RastPort ptr in a2

	ifd		LAYERNEEDED
	move.l	rp_Layer(a1),d4	* is there a Layer for this RastPort?
	if	<>
		exg.l	d4,a5
		LOCKLAYER
		exg.l	d4,a5
	endif
	endc

	clr.l	d4
	move.l	d4,a0		* clear a0
	btst	#RPB_AREAOUTLINE,rp_Flags+1(a2)
	if =
	    jsr	_LVOBltPattern(a6)		* use current rp_Mask
	else	.extend
		movem.l	d0/d1,-(sp)

		btst	#1,rp_DrawMode(a2) 		* bart - 05.07.86
		if <>					* COMPLEMENT mode ?
			movem.l	d2/d3,-(sp)		* save regs
			addq.w	#1,d0			* blit rectangle
			addq.w	#1,d1			* smaller in each
			subq.w	#1,d2			* dimension if not
			subq.w	#1,d3			* "negative"
			cmp.w	d0,d2
			if >=
			    cmp.w	d1,d3
			    if >=
					jsr	_LVOBltPattern(a6)		* use current rp_Mask
			    endif
			endif
			movem.l	(sp)+,d2/d3		* restore regs
		else
			jsr	_LVOBltPattern(a6)	* not COMPLEMENT mode
		endif					* end bart - 05.07.86

		movem.l	(sp)+,d4/d5
		move.l	rp_minterms(a2),-(sp)
		move.l	rp_minterms+4(a2),-(sp)
		move.b	rp_FgPen(a2),-(sp)
		move.b	rp_DrawMode(a2),-(sp)
		move.b	rp_AOLPen(a2),rp_FgPen(a2)
		clr.b	rp_DrawMode(a2)
		move.l	a2,a1
		GENMINTERMS
		move.l	a2,a1
		move.l	d4,d0		*xmin,ymin
		move.l	d5,d1
		jsr		_LVOMove(a6)
		move.l	a2,a1
		move.l	d4,d0		*xmin,ymax
		move.l	d3,d1
		jsr		_LVODraw(a6)
		move.l	a2,a1
		move.l	d2,d0		*xmax,ymax
		move.l	d3,d1
		jsr		_LVODraw(a6)
		move.l	a2,a1
		move.l	d2,d0		*xmax,ymin
		move.l	d5,d1
		jsr		_LVODraw(a6)
		move.l	a2,a1
		move.l	d4,d0		*xmin,ymin
		move.l	d5,d1
		jsr		_LVODraw(a6)
		move.b	(sp)+,rp_DrawMode(a2)
		move.b	(sp)+,rp_FgPen(a2)
		move.l	(sp)+,rp_minterms+4(a2)
		move.l	(sp)+,rp_minterms(a2)
	endif

	ifd	LAYERNEEDED
	move.l	rp_Layer(a2),d4		* on the way out check for layer again
	if <>
		exg.l	d4,a5
		UNLOCKLAYER
		exg.l	d4,a5
	endif
	endc

	movem.l	(sp)+,d4/d5/a2
    rts

	end
