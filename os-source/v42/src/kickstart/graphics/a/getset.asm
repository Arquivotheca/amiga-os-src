*******************************************************************************
*
*	$Id: GetSet.asm,v 42.0 93/06/16 11:14:24 chrisg Exp $
*
*******************************************************************************


	SECTION	graphics

	include	'/rastport.i'

; this module contains functions to get and set the values of rastport attributes
; that had no Set or Get function before V38. These are provided in order to make
; the transition to RTG smoother.
;

	xdef	_GetAPen,_GetBPen,_GetDrMd,_GetOPen
	xdef	_SetOutlinePen,_SetWriteMask,_SetMaxPen

	
******* graphics.library/SetOutlinePen ******************************************
*
*   NAME
*       SetOutlinePen -- Set the Outline Pen value for a RastPort (V39).
*
*
*   SYNOPSIS
*       old_pen=SetOutlinePen  ( rp, pen )
*	  d0	                 a0   d0
*
*	ULONG SetOutlinePen(struct RastPort *,ULONG)
*
*   FUNCTION
*	Set the current value of the O pen for the rastport and turn on area outline
*	mode. This function should be used instead of poking the structure directly,
*	because future graphics devices may store it differently, for instance,
*	using more bits.
*
*   INPUTS
*       rp  =  a pointer to a valid RastPort structure.
*	pen =  a longword pen number
*
*	returns the previous outline pen
*   BUGS
*
*   NOTES
*
*   SEE ALSO
*	GetOutlinePen() graphics/gfxmacros.h
*
*********************************************************************************
_SetOutlinePen:
	bset	#RPB_AREAOUTLINE,rp_Flags+1(a0)
	move.b	rp_AOLPen(a0),d1
	move.b	d0,rp_AOLPen(a0)
	moveq	#0,d0
	move.b	d1,d0
	rts

******* graphics.library/SetWriteMask *******************************************
*
*   NAME
*       SetWriteMask -- Set the pixel write mask value for a RastPort (V39).
*
*
*   SYNOPSIS
*       success=SetWriteMask ( rp, msk )
*	  d0	               a0   d0
*
*	ULONG SetWriteMask(struct RastPort *,ULONG)
*
*   FUNCTION
*	Set the current value of the bit write mask for the rastport.
*	bits of the pixel with zeros in their mask will not be modified by
*	subsequent drawing operations.
*
*   INPUTS
*       rp  =  a pointer to a valid RastPort structure.
*	msk =  a longword mask value. 
*
*	Graphics devices which do not support per-bit masking will
*	return 0 (failure).
*   BUGS
*
*   NOTES
*
*   SEE ALSO
*	graphics/gfxmacros.h
*
*********************************************************************************
_SetWriteMask:
	move.b	d0,rp_Mask(a0)
	moveq	#-1,d0
	rts

******* graphics.library/SetMaxPen *********************************************
*
*   NAME
*       SetMaxPen -- set maximum pen value for a rastport (V39).
*
*
*   SYNOPSIS
*       SetMaxPen ( rp, maxpen)
*   		    a0   d0
*
*	void SetMaxPen(struct RastPort *,ULONG)
*
*   FUNCTION
*	This will instruct the graphics library that the owner of the rastport
*	will not be rendering in any colors whose index is >maxpen. If there
*	are any speed optimizations which the graphics device can make based
*	on this fact (for instance, setting the pixel write mask), they will
*	be done.
*
*	Basically this call sets the rastport mask, if this would improve
*	speed. On devices where masking would slow things down (like with
*	chunky pixels), it will be a no-op.
*
*   INPUTS
*       rp  =  a pointer to a valid RastPort structure.
*	maxpen =  a longword pen value. 
*
*   BUGS
*
*   NOTES
*	The maximum pen value passed must take into account not only which
*	colors you intend to render in the future, but what colors you will
*	be rendering on top of.
*	SetMaxPen(rp,0) doesn't make much sense.
*
*   SEE ALSO
*	SetWriteMask()
*
*********************************************************************************
_SetMaxPen:
	st	d1
	bra.s	2$
1$:	lsr.b	#1,d1
2$:	add.b	d0,d0
	bhi.s	1$
	move.b	d1,rp_Mask(a0)
	rts

_GetAPen:
******* graphics.library/GetAPen ************************************************
*
*   NAME
*       GetAPen -- Get the A Pen value for a RastPort (V39).
*
*
*   SYNOPSIS
*       pen = GetAPen  ( rp )
*	 d0		 a0
*
*	ULONG GetAPen(struct RastPort *)
*
*   FUNCTION
*	Return the current value of the A pen for the rastport. This function 
*	should be used instead of peeking the structure directly, because future
*	graphics devices may store it differently, for instance, using more bits.
*
*   INPUTS
*       rp  =  a pointer to a valid RastPort structure.
*
*   BUGS
*
*   NOTES
*
*   SEE ALSO
*	SetAPen() graphics/gfx.h
*
*********************************************************************************
	moveq	#0,d0
	move.b	rp_FgPen(a0),d0
	rts

_GetBPen:
******* graphics.library/GetBPen ************************************************
*
*   NAME
*       GetBPen -- Get the B Pen value for a RastPort (V39).
*
*
*   SYNOPSIS
*       pen = GetBPen  ( rp )
*	d0	   	 a0
*
*	ULONG GetBPen(struct RastPort *)
*
*   FUNCTION
*	Return the current value of the B pen for the rastport. This function 
*	should be used instead of peeking the structure directly, because future
*	graphics devices may store it differently, using more bits.
*
*   INPUTS
*       rp  =  a pointer to a valid RastPort structure.
*
*   BUGS
*
*   NOTES
*
*   SEE ALSO
*	SetBPen() graphics/gfx.h
*
*********************************************************************************
	moveq	#0,d0
	move.b	rp_BgPen(a0),d0
	rts

_GetDrMd:
******* graphics.library/GetDrMd *************************************************
*
*   NAME
*       GetDrMd -- Get the draw mode value for a RastPort (V39).
*
*
*   SYNOPSIS
*       mode = GetDrMd  ( rp )
*	d0                a0
*
*	ULONG GetDrMd(struct RastPort *)
*
*   FUNCTION
*	Return the current value of the draw mode for the rastport. This function 
*	should be used instead of peeking the structure directly, because future
*	graphics devices may store it differently.
*
*   INPUTS
*       rp  =  a pointer to a valid RastPort structure.
*
*   BUGS
*
*   NOTES
*
*   SEE ALSO
*	SetDrMd() graphics/gfx.h
*
*********************************************************************************
	moveq	#0,d0
	move.b	rp_DrawMode(a0),d0
	rts

_GetOPen:
******* graphics.library/GetOutlinePen ******************************************
*
*   NAME
*       GetOutlinePen -- Get the Outline-Pen value for a RastPort (V39).
*
*
*   SYNOPSIS
*       pen = GetOutlinePen  ( rp )
*	d0		 a0
*
*	ULONG GetOutlinePen(struct RastPort *)
*
*   FUNCTION
*	Return the current value of the O pen for the rastport. This function 
*	should be used instead of peeking the structure directly, because future
*	graphics devices may store it differently, for instance, using more bits.
*
*   INPUTS
*       rp  =  a pointer to a valid RastPort structure.
*
*   BUGS
*
*   NOTES
*
*   SEE ALSO
*	SetOutlinePen() graphics/gfx.h
*
*********************************************************************************
	moveq	#0,d0
	move.b	rp_AOLPen(a0),d0
	rts

