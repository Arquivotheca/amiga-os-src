*******************************************************************************
*
*	$Id: EraseRect.asm,v 42.0 93/06/16 11:14:20 chrisg Exp $
*
*******************************************************************************

	xdef	_EraseRect

******* graphics.library/EraseRect *******************************************
*
*   NAME
*
*       EraseRect -- Fill a defined rectangular area using the current
*		     	BackFill hook. (V36)
*
*   SYNOPSIS
*	EraseRect( rp, xmin, ymin, xmax, ymax)
*                  a1  d0:16 d1:16 d2:16 d3:16
*
*	void EraseRect(struct RastPort *, SHORT, SHORT, SHORT, SHORT);
*
*   FUNCTION
*	Fill the rectangular region specified by the parameters with the
*	BackFill hook. If non-layered, the rectangular region specified by
*	the parameters is cleared. If layered the Layer->BackFill Hook is used.
*
*   INPUTS
*	rp	- pointer to a RastPort structure
*	xmin	- x coordinate of the upper left corner of the region to fill.
*	ymin	- y coordinate of the upper left corner of the region to fill.
*	xmax	- x coordinate of the lower right corner of the region to fill.
*	ymax	- y coordinate of the lower right corner of the region to fill.
*
*   BUGS
*
*   NOTES
*	The following relation MUST be true:
*	(xmax >= xmin) and (ymax >= ymin)
*
*   SEE ALSO
*	graphics/rastport.h graphics/clip.h
*
******************************************************************************

	section	graphics

	include 'exec/types.i'
	include 'graphics/rastport.i'
	include 'graphics/clip.i'
	include	'/gfxbase.i'
	include 'submacs.i'

	xref    _LVOClipBlit
	xref    _rastrect
	xref	_LVODoHookClipRects


; void __asm EraseRect(register __a1 struct RastPort *rp, register __d0 short xmin, register __d1 short ymin,
;			register __d2 short xmax, register __d3 short ymax); /* __asm */

_EraseRect:

	tst.l	rp_Layer(a1)
	beq.s	non_layered
	movem.l	a2/a5,-(sp)
	move.l	rp_Layer(a1),a5
	LOCKLAYER
	move.l	lr_BackFill(a5),a0 * hook routine is protected by layer lock
	movem.w	d0/d1/d2/d3,-(sp)	
	move.l	a7,a2
	move.l	a6,-(a7)
	move.l	gb_LayersBase(a6),a6
	jsr	_LVODoHookClipRects(a6)
	move.l	(a7)+,a6
	lea		4*2(a7),a7	; pop rect
	UNLOCKLAYER
	movem.l	(a7)+,a5/a2
	rts

non_layered:

	movem.l d4/d5/d6,-(sp)
	move.l	a1,a0		* src rp
	sub.l	d0,d2	
	addq.l  #1,d2
	move.l	d2,d4		* xsize
	move.l	d0,d2		* destx
	sub.l	d1,d3
	addq.l  #1,d3
	move.l	d3,d5		* ysize
	move.l	d1,d3		* desty
	moveq.l	#0,d6		* clear
	jsr	_LVOClipBlit(a6)
	movem.l (sp)+,d4/d5/d6
	rts
	
	end
