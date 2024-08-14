*******************************************************************************
*
*	$Id: areamove.asm,v 39.0 91/08/21 17:23:51 chrisg Exp $
*
*******************************************************************************

	section	graphics
    xdef    _AreaMove
******* graphics.library/AreaMove ********************************************
*
*   NAME
*	AreaMove -- Define a new starting point for a new
*	            shape in the vector list.
*
*
*   SYNOPSIS
*	error =  AreaMove( rp,   x,     y)
*	 d0                a1  d0:16  d1:16
*
*	LONG AreaMove( struct RastPort *, SHORT, SHORT );
*
*   FUNCTION
*	Close  the last polygon and start another polygon
*	at  (x,y). Add the necessary  points  to  vector
*	buffer. Closing a polygon may result in the generation
*	of another AreaDraw() to close previous polygon.
*	Remember to have an initialized AreaInfo structure attached
*	to the RastPort.
*
*   INPUTS
*	rp  - points to a RastPort structure
*	x,y - positions in the raster
*
*   RETURNS
*	error - zero for success, or -1 if there is no space left in the
*	vector list
*
*   BUGS
*
*   SEE ALSO
*	InitArea() AreaDraw() AreaEllipse() AreaEnd() graphics/rastport.h
*
*
******************************************************************************
    include 'exec/types.i'
    include 'graphics/rastport.i'
    include '/c/areafill.i'

	xref	_LVOAreaDraw

_AreaMove:
	move.w	d7,-(sp)
	move.l	rp_AreaInfo(a1),a0	*	get pointer to AreaInfo structure
	move.w	ai_Count(a0),d7
	addq	#1,d7
	if	ai_MaxCount(a0)<d7.w
bad:
		moveq	#-1,d0
	else
		move.w	d7,ai_Count(a0)
		if #1<>d7.w
			move.l	a2,-(sp)	* need another register
			move.l	ai_FlagPtr(a0),a2
			if -(a2).b=#0		* was previous point a move?
				sub.w	#1,ai_Count(a0)	* yes, lets just backup
				move.l	a2,ai_FlagPtr(a0)
				subq.l	#4,ai_VctrPtr(a0)
			else
				move.l	ai_VctrPtr(a0),a2	* now check if previous
*											* polygon was closed properly
				move.w	-4(a2),d7			* get old x
				if ai_FirstX(a0)=d7
					move.w -2(a2),d7		* get old y
					cmp.w ai_FirstY(a0),d7
					beq.s closed
				endif
*				We have to call AreaDraw now
				move.l	a1,a2				* lets save RastPort here
				movem.w	d0/d1,-(sp)			* save these guys also
				movem.w	ai_FirstX(a0),d0/d1	* get original start
				jsr	_LVOAreaDraw(a6)		* close it up, sets cc
				movem.w	(sp)+,d0/d1			* restore d0/d1
				move.l	a2,a1				* get RastPort back
				if <						* now test the cc return
					move.l	(sp)+,a2
					bra bad
				endif
			endif
closed:
			move.l	(sp)+,a2
		endif
		movem.w	d0/d1,ai_FirstX(a0)	* we have a new beginning of polygon
		move.l	ai_VctrPtr(a0),a1	* get pointer to next pair
		move.w	d0,(a1)+			* stash x
		move.w	d1,(a1)+			* stash y
		move.l	a1,ai_VctrPtr(a0)	* save new pointer
		move.l	ai_FlagPtr(a0),a1	* get pointer to flag matrix
		moveq	#0,d0				* this one is a MOVE
		move.b	d0,(a1)+			* also return(0)
		move.l	a1,ai_FlagPtr(a0)	* new flag pointer
	endif
	move.w	(sp)+,d7
	tst.l	d0						* set up correct condition codes
	rts

	end
