*******************************************************************************
*
*	$Id: Move.asm,v 39.0 91/08/21 17:26:41 chrisg Exp $
*
*******************************************************************************

    include 'exec/types.i'
    include 'graphics/rastport.i'

	section	graphics
    xdef    _Move
    xref    getcode
******* graphics.library/Move **********************************************
*
*   NAME
*	Move -- Move graphics pen position.
*
*   SYNOPSIS
*	Move( rp,   x,    y)
*	      a1  d0:16 d1:16
*
*	void Move( struct RastPort *, SHORT, SHORT );
*
*   FUNCTION
*	Move graphics pen position to (x,y) relative to upper left (0,0)
*	of RastPort. This sets the starting point for subsequent Draw()
*	and Text() calls.
*
*   INPUTS
*	rp - pointer to a RastPort structure
*	x,y - point in the RastPort
*
*   RESULTS
*
*   BUGS
*
*   SEE ALSO
*	Draw graphics/rastport.h
*
*****************************************************************************
_Move:
    movem.w d0/d1,rp_cp_x(a1)   * store new x position and y pos
    or.w    #FRST_DOT,rp_Flags(a1)  * set first dot flag
    move.b  #15,rp_linpatcnt(a1)    * set correct shift count
    rts

	end
