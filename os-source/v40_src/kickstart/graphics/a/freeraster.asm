*******************************************************************************
*
*	$Id: FreeRaster.asm,v 39.0 91/08/21 17:25:26 chrisg Exp $
*
*******************************************************************************

	section	graphics
    xdef    _FreeRaster
******* graphics.library/FreeRaster ********************************************
*
*   NAME
*       FreeRaster -- Release an allocated area to the system free memory pool.
*
*
*   SYNOPSIS
*       FreeRaster( p, width, height)
*		   a0   d0:16  d1:16
*
*	void FreeRaster( PLANEPTR, USHORT, USHORT);
*
*   FUNCTION
*	Return the memory associated with this PLANEPTR of size
*	width and height to the MEMF_CHIP memory pool.
*
*   INPUTS
*       p  =  a pointer to a memory space  returned  as  a
*             result of a call to AllocRaster.
*
*	width - the width in bits of the bitplane.
*	height - number of rows in bitplane.
*
*   BUGS
*
*   NOTES
*       Width and height should be the same values with which you
*       called AllocRaster in the first place.
*
*   SEE ALSO
*	AllocRaster() graphics/gfx.h
*
******************************************************************************
	include '/macros.i'
	xref	rassize
_FreeRaster:
*               current routine calls a C subroutine to do the work
	bsr		rassize
	move.l	a0,a1
	CALLEXEC	FreeMem
    rts

	end
