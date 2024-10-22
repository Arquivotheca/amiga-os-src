*******************************************************************************
*
*	$Id: mrgcop.asm,v 39.3 92/08/27 12:05:18 spence Exp $
*
*******************************************************************************

	section	graphics

	include '/monitor.i'
	include '/macros.i'
	include	'/gfxbase.i'

	xdef	_MrgCop
	xref	_mrgcop
	xref	_LVOGfxLookUp

******* graphics.library/MrgCop ************************************************
*
*   NAME
*       MrgCop -- Merge together coprocessor instructions.
*
*   SYNOPSIS
*       error = MrgCop( View )
*       d0              A1
*
*	ULONG MrgCop( struct View * );
*
*   FUNCTION
*       Merge together the display, color, sprite and user coprocessor
*       instructions into a single coprocessor instruction stream.  This
*       essentially creates a per-display-frame program for the coprocessor.
*       This function MrgCop is used, for example, by the graphics animation
*       routines which effectively add information into an essentially
*       static background display.  This changes some of the user
*       or sprite instructions, but not those which have formed the
*       basic display in the first place.  When all forms of coprocessor
*       instructions are merged together, you will have a complete per-
*       frame instruction list for the coprocessor.
*
*       Restrictions:  Each of the coprocessor instruction lists MUST be
*       internally sorted in min to max Y-X order.  The merge routines
*       depend on this! Each list must be terminated using CEND(copperlist).
*
*   INPUTS
*       View - a pointer to the view structure whose coprocessor
*              instructions are to be merged.
*
*   RESULT
*       The view structure will now contain a complete, sorted/merged
*       list of instructions for the coprocessor, ready to be used by
*       the display processor.  The display processor is told to use
*       this new instruction stream through the instruction LoadView().
*
*	From V39, MrgCop() can return a ULONG error value (previous versions
*	returned void), to indicate that either there was insufficient memory
*	to build the system copper lists, or that MrgCop() had no work to do
*	if, for example, there were no ViewPorts in the list.
*
*	You should check for these error values - they are defined in
*	<graphics/view.h>.
*
*   BUGS
*
*   SEE ALSO
*       InitVPort() MakeVPort() LoadView() graphics/view.h
*	intuition.library/RethinkDisplay()
*
******************************************************************************
_MrgCop:

* From V39, MrgCop() is vectored through the MonitorSpec. Use _mrgcop as
* a default.

	addq.l	#1,gb_SpecialCounter(a6)
	move.l	a1,-(sp)	; push view pointer on stack
	beq.s	mc_default		; MrgCopDefault

	move.l	a1,a0
	jsr	_LVOGfxLookUp(a6)
	tst.l	d0
	beq.s	mc_default
	move.l	d0,a0			; ViewExtra
	move.l	ve_Monitor(a0),d0
	beq.s	mc_default
	move.l	d0,a0
	move.l	ms_MrgCop(a0),a0	; address to call
mc_doit:
	jsr	(a0)
	addq.l	#4,sp
	rts
mc_default:
	lea	_mrgcop,a0
	bra.s	mc_doit

	end
