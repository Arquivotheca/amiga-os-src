*******************************************************************************
*
*	$Id: AreaCircle.asm,v 42.0 93/06/16 11:14:09 chrisg Exp $
*
*******************************************************************************

******* graphics.library/AreaCircle ********************************************
*
*    NAME
*	AreaCircle -- add a circle to areainfo list for areafill.
*
*
*    SYNOPSIS
*	error = (int) AreaCircle( rp,  cx,  cy, radius)
*	D0			  A1   D0   D1	D2
*
*	ULONG AreaCircle(struct RastPort *, WORD, WORD, UWORD);
*
*    FUNCTION
*	Add circle to the vector buffer. It will be drawn to the rastport when
*	AreaEnd is executed.
*
*    INPUTS
*	rp	 - pointer to a RastPort structure
*
*	cx, cy   - the coordinates of the center of the desired circle.
*
*	radius	 - is the radius of the circle to draw around the centerpoint.
*
*    RESULTS
*	0 if no error
*	-1 if no space left in vector list
*
*    NOTES
*	This function is actually a macro which calls 
*	    AreaEllipse(rp,cx,cy,radius,radius).
*
*    SEE ALSO
*	AreaMove() AreaDraw() AreaCircle() InitArea() AreaEnd()
*	graphics/rastport.h graphics/gfxmacros.h
*
********************************************************************************


	include 'exec/types.i'
	include 'graphics/rastport.i'
	include '/c/areafill.i'

AREACIRCLE_MACRO equ 1

	ifnd AREACIRCLE_MACRO

	section	graphics
	xdef    _AreaCircle

	xref	_LVOAreaDraw

_AreaCircle:
*				use a0 as scratch pointer
* this could be speeded up by using upper 16 bits of d0/d1 as scratch
	movem.l	d2/d7,-(sp)
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
		btst    #EXTENDEDn,-(a2)	* was an extended instruction ?
		blt.s	not_extended
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
			bra bad
		    endif
		endif
	    endif
closed:
	    move.l	(sp)+,a2		* restore the saved register
	endif

*	bart - 05.01.86 end routine  - close last polygon if still open

	move.w	ai_Count(a0),d7			* get current count of vectors
	addq.w	#2,d7				* circle takes two points
	if ai_MaxCount(a0)>=d7.w
		move.w	d7,ai_Count(a0)		* store new count

*		always draw boundary for circles
		moveq	#MDFLAG2,d7

*		overwrite boundary with outline -- bart 88.10.04 
		btst #RPB_AREAOUTLINE,rp_Flags+1(a1)
		if <>
			or.b	#DRWBNDRY,d7
		endif

*		circle is extended instuction
		or.b	#EXTENDED,d7

*		no longer need RastPort pointer so we can use a1 now
		move.l	ai_VctrPtr(a0),a1

*		don't need to look at previous instruction - bart - 04.28.86
*		if -2(a1)<>d1.w
			or.b	#MDFLAG1,d7
*		endif

		move.w	d0,(a1)		* stash away xcenter - radius
		sub.w	d2,(a1)+
		move.w	d1,(a1)		* stash away ycenter - radius
		sub.w	d2,(a1)+
		move.w	d0,(a1)		* stash away xcenter + radius
		add.w	d2,(a1)+
		move.w	d1,(a1)		* stash away ycenter + radius
		add.w	d2,(a1)+

		move.l	a1,ai_VctrPtr(a0)	* new pointer

		move.l	ai_FlagPtr(a0),a1	* get pointer to flags
		move.b	d7,(a1)+		* stash first flag btye
		moveq	#CIRCLE,d7		* extended command
		move.b	d7,(a1)+		* stash extended flag btye
		move.l	a1,ai_FlagPtr(a0)
		
		moveq	#0,d0			* no error
	else
*		too many vectors
bad:
		moveq	#-1,d0			* return error
	endif
	movem.l	(sp)+,d2/d7			* does not affect ccr
	rts

	endc

	end
