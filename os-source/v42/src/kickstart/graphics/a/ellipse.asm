*******************************************************************************
*
*	$Id: Ellipse.asm,v 42.0 93/06/16 11:14:02 chrisg Exp $
*
*******************************************************************************

	section graphics
	xdef	_DrawEllipse
	xref	_draw_ellipse

******* graphics.library/DrawEllipse *******************************************
*
*    NAME
*	DrawEllipse -- Draw an ellipse centered at cx,cy with vertical
*	   and horizontal radii of a,b respectively.
*
*    SYNOPSIS
*	DrawEllipse( rp, cx, cy, a, b )
*		     a1  d0  d1  d2 d3 
*
*	void DrawEllipse( struct RastPort *, SHORT, SHORT, SHORT, SHORT);
*
*    FUNCTION
*       Creates an elliptical outline within the rectangular region
*	specified by the parameters, using the current foreground pen color.
*
*    INPUTS
*	rp - pointer to the RastPort into which the ellipse will be drawn.
*	cx - x coordinate of the centerpoint relative to the rastport.
*	cy - y coordinate of the centerpoint relative to the rastport.
*	a - the horizontal radius of the ellipse (note: a must be > 0)
*	b - the vertical radius of the ellipse (note: b must be > 0)
*
*    BUGS
*
*    NOTES
*	this routine does not clip the ellipse to a non-layered rastport.
*
*    SEE ALSO
*	DrawCircle(), graphics/rastport.h
*
*******************************************************************************/
_DrawEllipse:
*               current routine calls a C subroutine to do the work
	move.l  d3,-(sp)    * b
	move.l  d2,-(sp)    * a
	move.l  d1,-(sp)    * cy
	move.l  d0,-(sp)    * cx
	move.l  a1,-(sp)    * RastPort
	jsr     _draw_ellipse
	lea     20(sp),sp   * reset stack
	rts


	end

