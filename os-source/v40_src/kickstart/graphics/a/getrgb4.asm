*******************************************************************************
*
*	$Id: getrgb4.asm,v 39.1 92/08/20 14:46:55 chrisg Exp $
*
*******************************************************************************

	section	graphics
    xdef    _GetRGB4
******* graphics.library/GetRGB4 ************************************************
*
*   NAME
*       GetRGB4 -- Inquire value of entry in ColorMap.
*
*   SYNOPSIS
*       value = GetRGB4( colormap, entry )
*          d0              a0       d0
*
*	ULONG GetRGB4(struct ColorMap *, LONG);
*
*   FUNCTION
*	Read and format a value from the ColorMap.
*
*   INPUTS
*	colormap - pointer to ColorMap structure
*	entry - index into colormap
*
*   RESULT
*	returns -1 if no valid entry
*	return UWORD RGB value 4 bits per gun right justified
*
*   NOTE
*	Intuition's DisplayBeep() changes color 0. Reading Color 0 during a
*	DisplayBeep() will lead to incorrect results.
*
*   BUGS
*
*   SEE ALSO
*       SetRGB4() LoadRGB4() GetColorMap() FreeColorMap() graphics/view.h
******************************************************************************

	include	'exec/types.i'
	include	'graphics/view.i'

_GetRGB4:
*               current routine calls a C subroutine to do the work
	if cm_Count(a0)<=d0.w
		moveq	#-1,d0
		rts					* return error index out of range
	endif
	move.l	cm_ColorTable(a0),a0
	add.w	d0,d0			* convert to byte offset, word per entry
	move.w	0(a0,d0.w),d0		* get rgb data
	and.l	#$fff,d0		* mask any genlock bits set
    rts

	end
