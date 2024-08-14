*******************************************************************************
*
*	$Id: Circle.asm,v 39.0 91/08/21 17:24:33 chrisg Exp $
*
*******************************************************************************

****** graphics.library/DrawCircle ********************************************
*
*    NAME
*	DrawCircle -- draw a circlular outline into a rastport using the
*		      current drawing pen color.
*
*    SYNOPSIS
*	DrawCircle( rp, cx, cy, radius)
*		    A1  D0  D1  D2
*
*	void DrawCircle(struct RastPort *, SHORT, SHORT, SHORT );
*
*    FUNCTION
*	Create a circular outline in the rastport, centered at cx,cy 
*	with the given radius, using the current foreground pen color
*	and draw mode.
*
*    INPUTS
*	rp - pointer to the RastPort into which the circle will be drawn.
*	cx- x coordinate of the centerpoint relative to the rastport.
*	cy- y coordinate of the centerpoint relative to the rastport.
*	radius - the radius of the circle (note: the radius must be > 0)
*
*	Note: this routine does not clip the circle to a non-layered rastport.
*
*    NOTES
*	This routine is a macro which calls DrawEllipse(rp,cx,cy,radius,radius).
*
*    BUGS
*
*    SEE ALSO
*	DrawEllipse() graphics/rastport.h, graphics/gfxmacros.h
*
*******************************************************************************/

	
CIRCLE_IS_MACRO	equ 1

	ifnd	CIRCLE_IS_MACRO

	section graphics
	xdef	_DrawCircle
	xref	_draw_circle

_DrawCircle:
*	        current routine calls a C subroutine to do the work
	move.l  d2,-(sp)    * radius
	move.l  d1,-(sp)    * ycenter
	move.l  d0,-(sp)    * xcenter
	move.l  a1,-(sp)    * RastPort
	jsr     _draw_circle
	lea     16(sp),sp   * reset stack
	rts

	endc

	end

