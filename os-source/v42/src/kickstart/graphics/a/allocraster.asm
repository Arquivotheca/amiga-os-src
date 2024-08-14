*******************************************************************************
*
*	$Id: AllocRaster.asm,v 42.0 93/06/16 11:14:53 chrisg Exp $
*
*******************************************************************************


	section	graphics
    xdef    _AllocRaster
******* graphics.library/AllocRaster ******************************************
*
*   NAME
*	AllocRaster -- Allocate space for a bitplane.
*
*   SYNOPSIS
*	planeptr = AllocRaster( width, height )
*	   d0                    d0     d1
*
*	PLANEPTR AllocRaster(ULONG,ULONG);
*
*   FUNCTION
*	This function calls the memory allocation routines
*	to allocate memory space for a bitplane "width" bits
*	wide and "height" bits high.
*
*   INPUTS
*	width	- number of columns in bitplane
*	height	- number of rows in bitplane
*
*   RESULT
*	planeptr - pointer to first word in bitplane, or NULL if
*		   it was not possible to allocate the desired
*		   amount of memory.
*
*   NOTES
*	In order to assure proper alignment of display memory, the
*	AllocBitMap() function should be used instead of AllocRaster
*	when you wish to allocate display memory (rasters which are
*	attached to a ViewPort or Screen).
*
*   BUGS
*
*   SEE ALSO
*	FreeRaster() graphics/gfx.h
*
******************************************************************************

	include	'exec/types.i'
	include 'exec/nodes.i'
	include 'exec/lists.i'
	include 'exec/memory.i'
	include '/macros.i'
_AllocRaster:
*	        current routine calls a C subroutine to do the work
	bsr.s	rassize
	moveq.l	#MEMF_CHIP+MEMF_PUBLIC,d1
	CALLEXEC	AllocMem
    rts

	xdef  rassize
*	input d0,d1 return number of bytes for this bitplane
rassize:
	add.w	#15,d0
	asr.w	#3,d0
	and.w	#$FFFE,d0
	mulu	d1,d0
	rts

	end
