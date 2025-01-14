*******************************************************************************
*
*	$Id: WaitTOF.asm,v 42.0 93/06/16 11:12:28 chrisg Exp $
*
*******************************************************************************

    include 'exec/types.i'
    include 'exec/nodes.i'
    include 'exec/lists.i'
    include 'exec/interrupts.i'
    include 'exec/libraries.i'
    include 'exec/ables.i'
    include 'exec/execbase.i'

    section	graphics
    include 'graphics/gfxbase.i'
    include 'submacs.i'

    xdef    _WaitTOF
******* graphics.library/WaitTOF ***********************************************
*
*   NAME
*       WaitTOF -- Wait for the top of the next video frame.
*
*   SYNOPSIS
*       WaitTOF()
*
*	void WaitTOF( void );
*
*   FUNCTION
*       Wait  for vertical blank to occur and all vertical blank
*       interrupt routines to complete before returning to caller.
*
*   INPUTS
*       none
*
*   RESULT
*	Places this task on the TOF wait queue. When the vertical blank
*	interrupt comes around, the interrupt service routine will fire off
*	signals to all the tasks doing WaitTOF. The highest priority task
*	ready will get to run then.
*
*   BUGS
*
*   SEE ALSO
*	exec.library/Wait() exec.library/Signal()
*
******************************************************************************
* now downcoded from waittof() in waitbovp.c

TOF_SIGNAL equ 4		/* stolen SIGF_BLIT & SIGF_SINGLE */
	xref	_LVOFindTask
	xref	_LVOSetSignal
	xref	_LVOAddTail
	xref	_LVOWait
	xref	_LVORemove
	xref	_LVODisable
	xref	_LVOEnable
	xref	_intena

*	xref	_waittof
_WaitTOF:
*	jmp		_waittof
	movem.l	a5-a6,-(sp)
	sub.w	#LN_SIZE+2,sp	; build a <longword aligned> node on the stack
	lea	gb_TOF_WaitQ(a6),a5
	move.l	gb_ExecBase(a6),a6		; get execbase
	move.l	ThisTask(a6),LN_NAME(sp)	; cheat!! (It's ok -Bryce)
	move.b	#TOF_SIGNAL,LN_PRI(sp)		; now stores sigbit in LN_PRI!
	moveq	#0,d0
	move.b	d0,LN_TYPE(sp)
	moveq	#1<<TOF_SIGNAL,d1
	jsr	_LVOSetSignal(a6)		; Clear TOF signal bit
	move.l	a5,a0				; gb_TOF_WaitQ(GfxBase)
	move.l	sp,a1
	DISABLE
	ADDTAIL
	moveq	#1<<TOF_SIGNAL,d0
	jsr	_LVOWait(a6)
	move.l	sp,a1
	REMOVE
	ENABLE
	add.w	#LN_SIZE+2,sp
	movem.l	(sp)+,a5-a6
	rts

	end
