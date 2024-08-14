*******************************************************************************
*
*	$Id: WaitBOVP.asm,v 42.0 93/06/16 11:12:52 chrisg Exp $
*
*******************************************************************************

	include 'exec/types.i'
	include '/gfxbase.i'
	include '/view.i'
	include '/monitor.i'

	section	graphics

    xdef    _WaitBOVP
******* graphics.library/WaitBOVP *******************************************
*                                                      
*   NAME
*	WaitBOVP -- Wait till vertical beam reached bottom of
*		    this viewport.
*
*   SYNOPSIS
*	WaitBOVP( vp )
*		  a0
*
*	void WaitBOVP( struct ViewPort * );
*
*   FUNCTION
*	Returns when the vertical beam has reached the bottom of this viewport
*
*   INPUTS
*	vp - pointer to ViewPort structure
*
*   RESULT
*	This function will return sometime after the beam gets beyond
*	the bottom of the viewport.  Depending on the multitasking load
*	of the system, the actual beam position may be different than
*	what would be expected in a lightly loaded system.
*
*   BUGS
*	Horrors! This function currently busy waits waiting for the
*	beam to get to the right place.  It should use the copper
*	interrupt to trigger and send signals like WaitTOF does.
*
*   SEE ALSO
*	WaitTOF() VBeamPos()
*
*********************************************************************
_WaitBOVP:
	move.w	vp_DHeight(a0),d0
	bra.s	_WaitTOVP

	xref	_LVOVBeamPos
	xref	_LVOGfxLookUp

	xdef	_waittovp
_waittovp:
* c interface for WaitTOVP
	move.l	4(sp),a0	* get vp
	move.w	8+2(sp),d0	* get offset
*	fall into code

_WaitTOVP:
*  a0 = viewport ptr, d0.w = offset into viewport
* this should really wait for a signal
	move.l	gb_ActiView(a6),d1		* get current actiview
	beq 	tvprts
	movem.l	d2/d3/d4/d5/a2,-(sp)
	move.w	d0,d2				* preserve offset   in d2
	move.l	d1,d4				* preserve actiview in d4
	moveq.l	#0,d5				* clear interlace shift

	move.l	a0,-(sp)			* store viewport on stack
	xref	_new_mode
	jsr	_new_mode			* new viewport modes
	btst	#3,d0				* double scan?
	beq.s	no_dbscan
	add.w	d2,d2

no_dbscan:
	and.b	#V_LACE,d0			* test for interlace and
	if <>
		moveq.l	#1,d5			* set interlace shift
	endif
	
	move.l	gb_natural_monitor(a6),a2	* preliminary mspc in a2
	move.l	d4,a0				* actiview in a0
	move.w	v_Modes(a0),d1			* view->Modes in d1
	and.w	#EXTEND_VSTRUCT,d1		* check for possible real mspc
	beq.s	not_xtnd			* and either skip or
	jsr 	_LVOGfxLookUp(a6)		* return viewextra in d0.l
	tst.l	d0			       
	beq.s	not_xtnd
        move.l  d0,a1
	move.l	ve_Monitor(a1),d0	
	beq.s	not_xtnd
        move.l  d0,a2 				* monitorspec pointer in a2
not_xtnd:
	move.l	(sp)+,a0			* restore viewport to a0
	move.l	d4,a1				* actiview in a1
	moveq.l	#0,d4				* use d4 for a2024 request 

	move.w	ms_total_rows(a2),d3
	btst.b	#MSB_REQUEST_A2024,ms_Flags+1(a2)	* request a2024?
	beq.s	localmax_found
	move.w	#$2C,d4
	bra.s	localmax_found
no_mspc:
	move.w	gb_MaxDisplayRow(a6),d3
localmax_found:
	subq.w	#1,d3				* localmax in d3

	repeat
		move.w	vp_DyOffset(a0),d1	* in case this changes
		add.w	d2,d1
		asr.w	d5,d1			* scale to account for lace
		tst.w	d4			* if d4 is NULL then use v_dy
		bne.s	fixed_voffset		* else use fixed offset
		add.w	v_DyOffset(a1),d1 	* in case this changes
fixed_voffset:  add.w	d4,d1		
		if d3<d1.w	* too many rows?
			move.w	d3,d1
		endif
		jsr _LVOVBeamPos(a6)			* returns in d0.l
	until d0>=d1.w
	movem.l	(sp)+,d2/d3/d4/d5/a2
tvprts: rts

	end
