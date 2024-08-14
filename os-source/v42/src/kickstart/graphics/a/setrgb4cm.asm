*******************************************************************************
*
*	$Id: SetRGB4CM.asm,v 42.0 93/06/16 11:13:58 chrisg Exp $
*
*******************************************************************************

	section graphics
    xdef    _SetRGB4CM,_SetRGB32CM

******* graphics.library/SetRGB4CM ********************************************
*
*   NAME
*       SetRGB4CM -- Set one color register for this ColorMap.
*
*   SYNOPSIS
*       SetRGB4CM(  cm,  n,   r,    g,    b)
*                   a0  d0  d1:4  d2:4  d3:4
*
*       void SetRGB4CM( struct ColorMap *, SHORT, UBYTE, UBYTE, UBYTE );
*
*   INPUTS
*	cm = colormap
*       n = the number of the color register to set. Ranges from 0 to 31
*	    on current Amiga displays.
*       r = red level (0-15)
*       g = green level (0-15)
*       b = blue level (0-15)
*
*   RESULT
*	Store the (r,g,b) triplet at index n of the ColorMap structure.
*       This function can be used to set up a ColorMap before before
*	linking it into a viewport.
*
*   BUGS
*
*   SEE ALSO
*       GetColorMap() GetRGB4() SetRGB4() graphics/view.h
******************************************************************************

		include 'exec/types.i'
		include 'graphics/view.i'

_SetRGB4CM:
	movem.l	d2/d3/d7,-(a7)
	cmp	#0,a0
	beq.s	no_low
	cmp.w	cm_Count(a0),d0
	bhs.s	no_low
	lsl.l	#8,d1
	lsl.l	#4,d2
	or.l	d2,d1
	or.l	d3,d1
	add.l	d0,d0
	move.l	cm_ColorTable(a0),a1
	move.w	0(a1,d0.w),d2
	and.w	#$8000,d2
	or.w	d1,d2
	move.w	d2,0(a1,d0.w)
	tst.b	cm_Type(a0)
	beq.s	no_low
	tst.l	cm_LowColorBits(a0)
	beq.s	no_low
	move.l	cm_LowColorBits(a0),a1
	move.w	d1,0(a1,d0.w)
no_low:
	movem.l	(a7)+,d2/d3/d7
	rts


******* graphics.library/SetRGB32CM ********************************************
*
*   NAME
*       SetRGB32CM -- Set one color register for this ColorMap. (V39)
*
*   SYNOPSIS
*       SetRGB32CM(  cm,  n,   r,    g,    b)
*                    a0  d0   d1    d2    d3
*
*       void SetRGB4CM( struct ColorMap *, ULONG, ULONG, ULONG , ULONG);
*
*   INPUTS
*	cm = colormap
*       n = the number of the color register to set. Must not exceed the number of colors
*	    allocated for the colormap.
*       r = red level (32 bit unsigned left justified fraction)
*       g = green level
*       b = blue level
*
*   RESULT
*	Store the (r,g,b) triplet at index n of the ColorMap structure.
*       This function can be used to set up a ColorMap before before
*	linking it into a viewport.
*
*   BUGS
*
*   SEE ALSO
*       GetColorMap() GetRGB32() SetRGB32() SetRGB4CM() graphics/view.h
******************************************************************************
_SetRGB32CM:
	cmp.w	cm_Count(a0),d0
	bge.s	skip
	move.l	d4,-(a7)
	move.l	cm_ColorTable(a0),a1
	tst.b	cm_Type(a0)
	move.l	cm_LowColorBits(a0),a0	; doesn't set condition bits
	bne.s	new_style
	move.l	a1,a0
new_style:
; first, get high word
	add.w	d0,d0			; cvt to index
	add.w	d0,a0
	add.w	d0,a1

	swap	d1
	swap	d2
	swap	d3

	move.w	d1,d0	
	lsr.w	#4,d0
	and.w	#$0f00,d0

	move.w	d2,d4
	lsr.w	#8,d4
	and.w	#$00f0,d4
	or.w	d4,d0

	move.w	d3,d4
	rol.w	#4,d4
	and.w	#$000f,d4
	or.w	d4,d0
	move.w	(a1),d4			; fetch old data
	and.w	#$8000,d4
	or.w	d4,d0
	move.w	d0,(a1)			; set high

	move.w	d1,d0
	and.w	#$0f00,d0
	move.w	d2,d4
	lsr.w	#4,d4
	and.w	#$00f0,d4
	or.w	d4,d0
	move.w	d3,d4
	and.w	#$000f,d4
	or.w	d4,d0
	move.w	d0,(a0)
	move.l	(a7)+,d4
	swap	d2
	swap	d3
skip:
	rts



	end
