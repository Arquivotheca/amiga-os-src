*******************************************************************************
*
*	$Id: AreaEllipse.asm,v 42.0 93/06/16 11:14:11 chrisg Exp $
*
*******************************************************************************

	section	graphics
    xdef    _AreaEllipse

******* graphics.library/AreaEllipse ********************************************
*
*    NAME
*	AreaEllipse -- add a ellipse to areainfo list for areafill.
*
*
*    SYNOPSIS
*	error = AreaEllipse( rp, cx,   cy,   a,    b    )
*	d0		     a1  d0:16 d1:16 d2:16 d3:16
*
*	LONG AreaEllipse( struct RastPort *, SHORT, SHORT, SHORT, SHORT)
*
*    FUNCTION
*	Add an ellipse to the vector buffer. It will be draw when AreaEnd() is
*	called.
*
*    INPUTS
*	rp - pointer to a RastPort structure
*	cx - x coordinate of the centerpoint relative to the rastport.
*	cy - y coordinate of the centerpoint relative to the rastport.
*	a  - the horizontal radius of the ellipse (note: a must be > 0)
*	b  - the vertical radius of the ellipse (note: b must be > 0)
*
*    RESULT
*	error - zero for success, or -1 if there is no space left in the
*		vector list
*
*    SEE ALSO
*	AreaMove() AreaDraw() AreaCircle() InitArea() AreaEnd()
*	graphics/rastport.h
*
********************************************************************************

	include 'exec/types.i'
	include 'graphics/rastport.i'
	include '/c/areafill.i'

	xref	_LVOAreaDraw

_AreaEllipse:
*				use a0 as scratch pointer
* this could be speeded up by using upper 16 bits of d0/d1 as scratch
	movem.l	d2/d3/d7,-(sp)
	move.l	rp_AreaInfo(a1),a0		*get pointer to AreaInfo structure

*	bart - 05.01.86 begin routine - close last polygon if still open

	move.w	ai_Count(a0),d7			* copy current count
	if #0<>d7.w
	    move.l	a2,-(sp)		* need another register
	    move.l	ai_FlagPtr(a0),a2
	    if -(a2).b=#0			* was previous point a move?
		sub.w	#1,ai_Count(a0)		* yes, lets just backup
		move.l	a2,ai_FlagPtr(a0)
		subq.l	#4,ai_VctrPtr(a0)
	    else
		cmp.w	#2,d7			* at least two instructions ?
		blt.s	not_extended		* no, could not be extended...
		btst    #EXTENDEDn,-(a2)	* was an extended instruction ?
		if =
not_extended:
		    move.l	ai_VctrPtr(a0),a2 
*		    				* now check if previous polygon 
*						* was closed properly
		    move.w	-4(a2),d7	* get old x
		    if ai_FirstX(a0)=d7
			move.w -2(a2),d7	* get old y
			cmp.w ai_FirstY(a0),d7
			beq.s closed
		    endif
		    move.l	a1,a2			* save RastPort here
		    movem.w	d0/d1,-(sp)		* save these guys also
		    movem.w	ai_FirstX(a0),d0/d1	* get original start
		    jsr	_LVOAreaDraw(a6)		* close it up, sets cc
		    movem.w	(sp)+,d0/d1		* restore d0/d1
		    move.l	a2,a1			* get RastPort back
		    if <				* now test the cc return
			move.l	(sp)+,a2
			bra.s bad
		    endif
		endif
	    endif	
closed:
	    move.l	(sp)+,a2		* restore the saved register
	endif

*	bart - 05.01.86 end routine  - close last polygon if still open

	move.w	ai_Count(a0),d7			* get current count of vectors
	addq.w	#2,d7				* ellipse takes two points
	if ai_MaxCount(a0)>=d7.w
		move.w	d7,ai_Count(a0)		* store new count

*		always draw boundary for ellipses
		moveq	#MDFLAG2,d7

*		overwrite boundary with outline -- bart 88.10.04 
		btst #RPB_AREAOUTLINE,rp_Flags+1(a1)
		if <>
			or.b	#DRWBNDRY,d7
		endif

*		ellipse is extended instuction
		or.b	#EXTENDED,d7

*		no longer need RastPort pointer so we can use a1 now
		move.l	ai_VctrPtr(a0),a1

*		don't need to look at previous instruction - bart - 04.28.86
*		if -2(a1)<>d1.w
			or.b	#MDFLAG1,d7
*		endif

		move.w	d0,(a1)		
		sub.w	d2,(a1)+		* stash away xmin
		move.w	d1,(a1)		
		sub.w	d3,(a1)+		* stash away ymin
		move.w	d0,(a1)		
		add.w	d2,(a1)+		* stash away xmax
		move.w	d1,(a1)		
		add.w	d3,(a1)+		* stash away ymax

		move.l	a1,ai_VctrPtr(a0)	* new pointer

		move.l	ai_FlagPtr(a0),a1	* get pointer to flags
		move.b	d7,(a1)+		* stash first flag btye
		moveq	#ELLIPSE,d7		* extended command
		move.b	d7,(a1)+		* stash extended flag btye
		move.l	a1,ai_FlagPtr(a0)
		
		moveq	#0,d0			* no error
	else
*		too many vectors
bad:
		moveq	#-1,d0				* return error
	endif
	movem.l	(sp)+,d2/d3/d7				* does not affect ccr
	rts

	end