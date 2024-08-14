*******************************************************************************
*
*	$Id: AreaDraw.asm,v 42.0 93/06/16 11:12:59 chrisg Exp $
*
*******************************************************************************

	section	graphics
    xdef    _AreaDraw
******* graphics.library/AreaDraw *******************************************
*
*   NAME
*	AreaDraw -- Add a point to a list of end points for areafill.
*
*
*   SYNOPSIS
*	error = AreaDraw( rp,  x,     y)
*	  d0	          A1 D0:16 D1:16
*
*	ULONG AreaDraw( struct RastPort *, SHORT, SHORT);
*
*   FUNCTION
*	Add point to the vector buffer.
*
*
*   INPUTS
*	rp	- points to a RastPort structure.
*	x,y	- are coordinates of a point in the raster.
*
*   RESULT
*	error	- zero for success, else -1 if no there was no space
*		  left in the vector list.
*
*   BUGS
*
*   SEE ALSO
*	AreaMove() InitArea() AreaEnd() graphics/rastport.h
*
*
******************************************************************************
	include 'exec/types.i'
	include 'graphics/rastport.i'
	include '/c/areafill.i'

_AreaDraw:
*				use a0 as scratch pointer
* this could be speeded up by using upper 16 bits of d0/d1 as scratch
	move.l	d7,-(sp)
	move.l	rp_AreaInfo(a1),a0		*get pointer to AreaInfo structure
	move.w	ai_Count(a0),d7			* get current count of vectors
	addq.w	#1,d7
	if ai_MaxCount(a0)>=d7.w
		move.w	d7,ai_Count(a0)		* store new count
*		is boundary mode on?
		moveq	#MDFLAG2,d7
		btst #RPB_AREAOUTLINE,rp_Flags+1(a1)
		if <>
			or.b	#DRWBNDRY,d7
		endif
*		no longer need RastPort pointer so we can use a1 now
		move.l	ai_VctrPtr(a0),a1
*	assume that no one does a draw first
		if -2(a1)<>d1.w
			or.b	#MDFLAG1,d7
		endif
		move.w	d0,(a1)+	* stash away x
		move.w	d1,(a1)+	* stash away y
		move.l	a1,ai_VctrPtr(a0)	* new pointer
		move.l	ai_FlagPtr(a0),a1	* get pointer to flags
		move.b	d7,(a1)+	* stash flags for this point
		move.l	a1,ai_FlagPtr(a0)
		moveq	#0,d0
	else
*		too many vectors
		moveq	#-1,d0				* return error
	endif
	movem.l	(sp)+,d7				* does not affect ccr
	rts

	end
