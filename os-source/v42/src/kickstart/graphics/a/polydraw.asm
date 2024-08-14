*******************************************************************************
*
*	$Id: PolyDraw.asm,v 42.0 93/06/16 11:12:34 chrisg Exp $
*
*******************************************************************************

	section	graphics
    xdef    _PolyDraw
    xref    _LVODraw
******* graphics.library/PolyDraw *******************************************
*
*   NAME
*	PolyDraw -- Draw lines from table of (x,y) values.
*
*   SYNOPSIS
*	PolyDraw( rp, count , array )
*		  a1   d0      a0
*
*	void PolyDraw( struct RastPort *, WORD, WORD * );
*
*   FUNCTION
*	starting with the first pair in the array, draw connected lines to
*	it and every successive pair.
*
*   INPUTS
*	rp - pointer to RastPort structure
*	count -  number of (x,y) pairs in the array
*	array - pointer to first (x,y) pair
*
*   BUGS
*
*   SEE ALSO
*	Draw() Move() graphics/rastport.h
*
*********************************************************************
_PolyDraw:
    movem.l d2/a2/a3,-(sp)
    move.w  d0,d2
    move.l  a0,a2
    move.l  a1,a3
    bra.s	endloop
loop:
	move.w  (a2)+,d0
	move.w  (a2)+,d1
	move.l  a3,a1   * make sure he gets RastPort
	jsr _LVODraw(a6)	* changed by Dale 11/4/85
endloop:
    dbra d2,loop
    movem.l (sp)+,d2/a2/a3
    rts

	end
