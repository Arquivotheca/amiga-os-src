*******************************************************************************
*
*	$Id: Flood.asm,v 39.0 91/08/21 17:25:10 chrisg Exp $
*
*******************************************************************************

	section	graphics
    xdef    _Flood
    xref    _flood
******* graphics.library/Flood *************************************************
*
*   NAME
*	Flood -- Flood rastport like areafill.
*
*   SYNOPSIS
*	error = Flood( rp, mode, x, y)
*        d0            a1   d2  d0  d1
*
*	BOOL Flood(struct RastPort *, ULONG, SHORT, SHORT);
*
*   FUNCTION
*	Search the BitMap starting at (x,y).
*	Fill all adjacent pixels if they are:
*	    Mode 0: not the same color as AOLPen
*	    Mode 1: the same color as the pixel at (x,y)
*
*	When actually doing the fill use the modes that apply to
*	standard areafill routine such as drawmodes and patterns.
*
*   INPUTS
*	rp - pointer to RastPort
*	(x,y) - coordinate in BitMap to start the flood fill at.
*	mode -  0 fill all adjacent pixels searching for border.
*		1 fill all adjacent pixels that have same pen number
*		as the one at (x,y).
*
*   NOTES
*	In order to use Flood, the destination RastPort must 
*	have a valid TmpRas raster whose size is as large as 
*	that of the RastPort.
*
*   SEE ALSO
*	AreaEnd() InitTmpRas() graphics/rastport.h
*
*********************************************************************
_Flood:
*               current routine calls a C subroutine to do the work
    move.l  d1,-(sp)    * y
    move.l  d0,-(sp)    * x
    move.l  d2,-(sp)    * mode
    move.l  a1,-(sp)    * RastPort
    jsr     _flood
    lea     16(sp),sp   * reset stack
    rts


	end
